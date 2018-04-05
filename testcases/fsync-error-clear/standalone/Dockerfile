FROM fsync_error_base:latest
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ stretch-pgdg main" > /etc/apt/sources.list.d/pgdg.list
ADD standalone/Makefile.standalone standalone/fsync-error-clear.c standalone/fsync_test_standalone.sh ./
RUN chmod a+x fsync_test_standalone.sh
RUN make -f Makefile.standalone
ENTRYPOINT ["./fsync_test_standalone.sh"]
CMD ["xfs","",""]
