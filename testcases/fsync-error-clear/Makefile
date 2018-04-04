default: run

build: Dockerfile fsync-error-clear.c fsync_test.sh Makefile.test
	docker build -t fsync_error_test .

run: build
	docker run --rm -it --privileged -t fsync_error_test
