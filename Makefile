srcDir = ./src
includeDir = ./include
all: manager storage director worker delivery
manager: $(srcDir)/manager.c utils.o $(includeDir)/constants.h
	gcc -o manager $(srcDir)/manager.c utils.o
director: $(srcDir)/director.c utils.o $(includeDir)/types.h $(includeDir)/constants.h
	gcc -o director $(srcDir)/director.c utils.o
storage: $(srcDir)/storage.c utils.o semaphores.o $(includeDir)/types.h $(includeDir)/constants.h
	gcc -o storage $(srcDir)/storage.c utils.o semaphores.o
worker: $(srcDir)/worker.c utils.o semaphores.o $(includeDir)/types.h $(includeDir)/constants.h
	gcc -o worker $(srcDir)/worker.c utils.o semaphores.o
delivery: $(srcDir)/delivery.c utils.o semaphores.o $(includeDir)/types.h $(includeDir)/constants.h
	gcc -o delivery $(srcDir)/delivery.c utils.o semaphores.o
semaphores.o: $(srcDir)/semaphores.c
	gcc -c $(srcDir)/semaphores.c
utils.o: $(srcDir)/utils.c
	gcc -c $(srcDir)/utils.c
clean_log:
	rm ./logs/*.log
clean: clean_log
	rm *.o