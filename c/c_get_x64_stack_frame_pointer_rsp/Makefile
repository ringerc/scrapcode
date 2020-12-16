run-unwind: unwind
	rm -f ~/core/*.core
	./unwind foobarbaz 420 1

run-main: main
	rm -f ~/core/*.core
	#./main foobarbaz 420 0 ||true
	./main foobarbaz 420 1 ||true

main: main.c
	gcc -fPIE -Wall -Wextra -O0 -ggdb3 -fomit-frame-pointer -fno-inline main.c -o main

unwind: unwind.c
	gcc -fPIE -Wall -Wextra -O0 -ggdb3 -fomit-frame-pointer -fno-inline unwind.c -lunwind -o unwind

clean:
	rm -f main unwind
