.phony all:
all: main

main: main.c
	gcc list.c main.c -lreadline -lhistory -o PMan

.PHONY clean:
clean:
	-rm -rf *.o *.exe
