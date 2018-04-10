FROM fsync_error_base
RUN echo "deb http://apt.postgresql.org/pub/repos/apt/ stretch-pgdg main" > /etc/apt/sources.list.d/pgdg.list
RUN wget --quiet -O - https://www.postgresql.org/media/keys/ACCC4CF8.asc | apt-key add -
RUN apt-get -y update \
    && apt-get -y install flex bison zlib1g-dev git libreadline6-dev python3-psycopg2 sudo ccache

RUN mkdir /pgbuild && \
    adduser --system --home /pgbuild --no-create-home pgbuild && \
    chown pgbuild /pgbuild

#
# Bind mount target
#   use -v /path/to/postgres/source:/postgres
# at "docker run" time
#
#
RUN mkdir -p /postgres /mnt/tmp

USER root
ADD postgres/fsync_test_postgres.sh postgres/pg_fsync_error.py ./
RUN chmod a+x fsync_test_postgres.sh

# Use docker run -P to get these on some high host ports:
EXPOSE 5432/tcp 

#
# ccache is installed, so you can map a host directory to /ccache at run time
# and set the env var CCACHE_DIR=/ccache e.g.
#
#   -v $HOME/.ccache:/ccache --env CCACHE_DIR=/ccache
#
# Ownership will be set by the script, so you need to mount there not somewhere
# else.
#

ENTRYPOINT ["./fsync_test_postgres.sh"]
CMD ["xfs","",""]
