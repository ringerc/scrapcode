#!/usr/bin/env python
#
# Install this script as `/usr/bin/lp', renaming the old lp
# command. Make sure printserver.py is running on your CUPS
# server and ready to accept data.
#
# You need Python (1.3.x) on your SCO box to run this.
#
#
# This spooler uses the following very simple network protocol:
# First, a handshake string is sent. This is a fixed-size message
# of the form:
#
#  SPOOL queuename job-bytes
#
# "SPOOL" is a literal five byte string.
#
# queuename is a string exactly thirty bytes long that encodes the
# name of the print queue, with a null byte terminating the name.
# The remainder of the string is padding and is ignored.
#
# job-bytes is a string exactly 10 bytes long. It is a number in ASCII
# representation, padded by spaces with no sign, that indicates the size
# of the file that will be sent by the client.
#
# Total message size is 45 bytes. There is no terminator, and no
# separator between fields.
#
#
#
# The server replies to this message with another fixed size message,
# simply the five-byte string:
#
#   READY
#
# without any terminator.
#
#
#
# The client proceeds to send the print job data. When it's sent the last
# byte it waits for a reply from the server.
#
#
# The server reads client data until the last byte is received. It spools
# it, then informs the client of the result with a simple fixed-length
# 7 byte message, either:
#
#   SUCCESS
#
# or
#
#   FAILURE
#
# (unterminated strings). No additional error info is provided; that'll be
# logged server side.
#
# After sending status, the server closes its socket. The client closes
# its socket when it receives the ack message.
#

#
import os
import sys
import socket
import threading
import tempfile
import syslog

printer_map = {
        'hp' : 'iprint',
        'P1' : 'iprint',
        'P2' : 'accounts'
}

listen_host = "10.0.0.10"
spoolport = 6668

class ClientThread(threading.Thread):
        
        def __init__(self, sock_info):
                threading.Thread.__init__(self)
                (self.sock, self.client_address) = sock_info

        def _log(self, prio, msg):
                syslog.syslog(syslog.LOG_DAEMON|prio, "exprintserver (%s:%s): " % (self.client_address) + msg)

        def run(self):
                try:
                        self._log(syslog.LOG_DEBUG, "connect")
                        # A 30-second timeout is set on socket I/O operations
                        # so that protocol issues or unexpected remote end death
                        # don't cause our threads to hang around forever.
                        self.sock.settimeout(30)
                        self.expect_handshake()
                        self.send_handshake_reply()
                        self.read_job_data()
                        if self._should_ignore_job():
                                status_ok = True
                        else:
                                status_ok = self.spool_job()
                        self.send_job_ack(status_ok)
                        self.shutdown_connection()
                        self._log(syslog.LOG_DEBUG, "disconnect")
                except Exception, e:
                        self._log(syslog.LOG_ERR, "exception during client communication: " + str(e))

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

        def expect_handshake(self):
                msg = self._recv_fixed_msg(45)
                if not msg.startswith("SPOOL"):
                        raise Exception("Invalid handshake")
                self.queuename = msg[5:35].strip()
                jobsize_str = msg[35:45].strip()
                self.jobsize = int(jobsize_str)
                if self.jobsize == 0:
                        raise Exception("Zero sized job proposed")

        def send_handshake_reply(self):
                self._send_fixed_msg("READY")
        
        def read_job_data(self):
                # spool the fucker into RAM for now
                jobdata = self._recv_fixed_msg(self.jobsize)
                self.f = tempfile.NamedTemporaryFile()
                self.f.write(jobdata)
                self.jobfilename = self.f.name
                self.f.flush()

        def _should_ignore_job(self):
                # Return true if this job should be discarded unprinted.
                # this lets us filter jobs from the system based on their contents.
                self.f.seek(0)
                if self.f.read().find("Cannot get TTY") != -1:
                        # Ignore "Cannot get TTY" error messages instead of printing them.
                        self._log(syslog.LOG_INFO, "ignoring job '%s' to '%s' - Cannot get TTY error" % (self.jobfilename, self.queuename))
                        return True
                return False

        def spool_job(self):
                """Note: this function may be skipped by the server, so it must not be involved
                in any I/O with the client lest the client get confused if it's skipped."""
                print "before", self.queuename
                self.queuename = printer_map.get(self.queuename, self.queuename)
                print "after", self.queuename
                self._log(syslog.LOG_INFO, "spooling job '%s' to '%s'" % (self.jobfilename, self.queuename))
                lpcmd = "lp '-d%s' '%s' >/dev/null 2>/dev/null" % (self.queuename, self.jobfilename)
                print lpcmd
                ret = os.system(lpcmd)
                return ret >> 8 == 0
        
        def send_job_ack(self,ok):
                if ok:
                        msg = "SUCCESS"
                else:
                        msg = "FAILURE"
                self._send_fixed_msg(msg)
                self.sock.shutdown(1)

        def shutdown_connection(self):
                self.sock.shutdown(0)
                self.sock.close()

def main():
        try:
                s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
                s.bind((listen_host, spoolport))
                s.listen(5)
                syslog.syslog(syslog.LOG_DAEMON|syslog.LOG_INFO, "exprintserver: startup complete")
        except Exception, e:
                syslog.syslog(syslog.LOG_DAEMON|syslog.LOG_ERR, "exprintserver: startup failed: " + str(e))
                sys.exit(1)
        try:
                while 1:
                        try:
                                ct = ClientThread(s.accept())
                                ct.start()
                        except socket.timeout:
                                syslog.syslog(syslog.LOG_DAEMON|syslog.LOG_ERR, "exprintserver: unexpected timeout on listening socket")
        except KeyboardInterrupt:
                return
        except Exception, e:
                syslog.syslog(syslog.LOG_DAEMON|syslog.LOG_ERR, "exprintserver: unexpected exception during client thread setup: " + str(e))
        
def close_stdio():
        #sys.stdout.close()
        #sys.stdin.close()
        #sys.stderr.close()
        #os.close(0)
        #os.close(1)
        #os.close(2)
        pass

if __name__ == '__main__':
        close_stdio()
        main()

