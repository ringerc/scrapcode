#!/usr/local/bin/python
#
# See http://blog.ringerc.id.au/2010/07/how-to-make-sco-openserver-505-printing.html

spoolhost = "10.0.0.10"
spoolport = 6668
defaultqueue = "iprint"

import sys
import os
import socket
import string

def syslog(facility, message):
        cmd = "/usr/bin/logger '%s' \"exprint: %s\"" % (facility, message)
        os.system(cmd)

def fatal(errmsg, exc):
        syslog("local1.error", "%s (%s)" % (errmsg, exc))
        sys.exit(1)

class Client:

        def __init__(self):
                pass

        def spool_socket(self, queue, spoolfile):
                self.spoolfile = spoolfile
                self.jobsize = os.stat(spoolfile)[6]
                self.queue = queue
                self.sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                self.sock.connect((spoolhost, spoolport))
                self.send_handshake()
                self.expect_handshake_reply()
                self.send_job_data()
                ok = self.expect_job_ack()
                self.shutdown_connection()
                return ok

        def _recv_fixed_msg(self,length_bytes):
                bytes_remaining = length_bytes
                msg = ""
                while bytes_remaining > 0:
                        frag = self.sock.recv(bytes_remaining)
                        if (frag == ""):
                                raise Exception("Unexpected EOF in fixed msg recv")
                        bytes_remaining = bytes_remaining - len(frag)
                        msg = msg + frag
                return msg

        def _send_fixed_msg(self,msg):
                totalsent = 0
                msglen = len(msg)
                while totalsent < msglen:
                        sent = self.sock.send(msg[totalsent:])
                        if sent == 0:
                                raise Exception("Unexpected EOF in fixed msg send")
                        totalsent = totalsent + sent

        def send_handshake(self):
                self._send_fixed_msg("SPOOL%30s%10s" % (self.queue, self.jobsize))

        def expect_handshake_reply(self):
                if self._recv_fixed_msg(5) != "READY":
                        raise Exception("Unexpected handshake reply")

        def send_job_data(self):
                # Hack: just slurp the job into RAM
                data = open(self.spoolfile, "rb").read(self.jobsize)
                if len(data) != self.jobsize:
                        raise Exception("Job size mismatch: expected %s read %s" % (self.jobsize, len(data)))
                self._send_fixed_msg(data)
                self.sock.shutdown(1)

        def expect_job_ack(self):
                ack = self._recv_fixed_msg(7)
                return ack == "SUCCESS"

        def shutdown_connection(self):
                self.sock.shutdown(0)
                self.sock.close()

def spool_lpd(queue, spoolfile):
        print_cmd = "/usr/bin/lp '-d%s' '%s' >/dev/null 2>/dev/null" % (queue, spoolfile)
        ret = os.system(print_cmd)
        if (ret >> 8) == 0 or (ret >> 8) == 1:
                syslog("local1.info", "spooled %s to %s" % (spoolfile, queue))
        else:
                syslog("local1.error", "spooler error sending %s to %s (ret: %s)" % (spoolfile, queue, ret >> 8))

def spool(queue, spoolfile):
        Client().spool_socket(queue, spoolfile)

def main():
        spoolfile = None
        queue = defaultqueue

        # Test for args
        if len(sys.argv) < 2:
                print "Usage: %s [-dPRINTERNAME] /path/to/printfile" % sys.argv[0]
                sys.exit(1)

        # Test for printer override
        if sys.argv[1][:2] == "-d":
                queue = sys.argv[1][2:]
                spoolfile = sys.argv[2]
        else:
                spoolfile = sys.argv[1]

        # check print file
        try:
                finfo = os.stat(spoolfile)
                if finfo[5] == 0:
                        fatal("Zero size spool file %s" % spoolfile, e)
        except OSError, e:
            fatal("Spool file %s missing or inaccessible" % spoolfile, e)

        # spool print file
        spool(queue, spoolfile)

        # and move the tempfile into the PRINTED directory
        os.rename(spoolfile, os.path.join("/usr/tmp/PRINTED", string.split(spoolfile, "/")[-1]))

if __name__ == '__main__':
        main()
