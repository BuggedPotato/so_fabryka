all: storage director worker
director: director.c utils.o types.h constants.h
	gcc -o director director.c utils.o
storage: storage.c utils.o types.h constants.h
	gcc -o storage storage.c utils.o
worker: worker.c utils.o semaphores.o types.h constants.h
	gcc -o worker worker.c utils.o semaphores.o
semaphores.o: semaphores.c
	gcc -c semaphores.c
utils.o: utils.c
	gcc -c utils.c
clean:
	rm *.o