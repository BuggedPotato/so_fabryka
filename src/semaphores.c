#include<sys/ipc.h>
#include<sys/sem.h>
#include<stdlib.h>
#include<errno.h>
#include<stdio.h>

/*
* Functions Names:	semLower
*
* Function:			lower semaphore
*
* Arguments:		id - semaphore set id,
                    semNum - semaphore number in set
*
* Return:			0->success, 1->failure/error with errno set
*/
int semLower( int id, int semNum ){
    struct sembuf op;
    op.sem_num = semNum;
    op.sem_op = -1;
    op.sem_flg = 0;
    if( semop( id, &op, 1 ) == -1 ){
        return 1;
    }
    return 0;
}

/*
* Functions Names:	semRaise
*
* Function:			raise semaphore
*
* Arguments:		id - semaphore set id,
                    semNum - semaphore number in set
*
* Return:			0->success, 1->failure/error with errno set
*/
int semRaise( int id, int semNum ){
    struct sembuf op;
    op.sem_num = semNum;
    op.sem_op = 1;
    op.sem_flg = 0;
    if( semop( id, &op, 1 ) == -1 ){
        return 1;
    }
    return 0;
}