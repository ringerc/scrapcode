FROM debian:stretch
LABEL description="fsync test image" \
      version="1.0"

# Use docker run -P to get these on some high host ports:
EXPOSE 5432/tcp 

RUN apt-get -y update \
    && apt-get -y install build-essential dmsetup xfsprogs e2fsprogs jfsutils dosfstools btrfs-progs gcc make bash sudo wget ca-certificates
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ stretch-pgdg main" > /etc/apt/sources.list.d/pgdg.list
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add -
RUN apt-get -y update \
    && apt-get -y install postgresql-10
ADD Makefile.test fsync-error-clear.c fsync_test.sh ./
RUN chmod a+x fsync_test.sh
RUN make -f Makefile.test
ENTRYPOINT ["./fsync_test.sh"]
CMD ["xfs","",""]
