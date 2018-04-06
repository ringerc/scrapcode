FROM fsync_error_base
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ stretch-pgdg main" > /etc/apt/sources.list.d/pgdg.list
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add -
RUN apt-get -y update \
    && apt-get -y install postgresql-10 python3-psycopg2 sudo
ADD postgres/fsync_test_postgres.sh postgres/pg_fsync_error.py ./
RUN chmod a+x fsync_test_postgres.sh

# Use docker run -P to get these on some high host ports:
EXPOSE 5432/tcp 

ENTRYPOINT ["./fsync_test_postgres.sh"]
CMD ["xfs","",""]
