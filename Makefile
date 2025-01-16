all: storage director manager
manager: manager.c utils.o constants.h
	gcc -o manager manager.c utils.o
director: director.c utils.o types.h constants.h
	gcc -o director director.c utils.o
storage: storage.c utils.o types.h constants.h
	gcc -o storage storage.c utils.o
utils.o: utils.c
	gcc -c utils.c
clean:
	rm *.o