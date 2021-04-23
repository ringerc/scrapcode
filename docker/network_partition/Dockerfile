FROM debian:10
ENV DEBIAN_FRONTEND noninteractive
RUN apt-get -y update && \
    apt-get -y install iproute2 socat iptables
ADD server /usr/local/bin/server
ADD client /usr/local/bin/client
RUN chmod a+x /usr/local/bin/server /usr/local/bin/client
