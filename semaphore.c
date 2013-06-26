/* Implementation of semaphore.h with System V functions */

#include <sys/sem.h>
#include "semaphore.h"
#include "defines.h"
#include <stdio.h>
#define P_RDWR 0600
#define UP 1
#define DOWN -1

struct sem_values {
	int value;
};

int createSem(key_t key, int sem_qty) {

	int sem_id;
	if ((sem_id = semget(key, sem_qty, IPC_CREAT | P_RDWR)) == ERROR) {
		return ERROR;
	}

	//setAllSem(sem_id, sem_qty, 0);
	return sem_id;
}

int setSem(int sem_id, int sem_num, int value) {

	struct sem_values sem_value;
	sem_value.value = value;

	if (semctl(sem_id, sem_num, SETVAL, sem_value) == ERROR) {
		return ERROR;
	}

	return EXIT_SUCCESS;
}

int setAllSem(int sem_id, int sem_qty, int value) {
	int i = 0;
	for (i = 0; i < sem_qty; i++) {
		setSem(sem_id, i, value);
	}
	return EXIT_SUCCESS;
}

int upSem(int sem_id, int sem_num) {

	struct sembuf op;
	op.sem_num = sem_num;
	op.sem_op = UP;
	op.sem_flg = 0;

	if (semop(sem_id, &op, 1) == ERROR) {
		return ERROR;
	}
	return EXIT_SUCCESS;
}

int downSem(int sem_id, int sem_num) {

	struct sembuf op;
	op.sem_num = sem_num;
	op.sem_op = DOWN;
	op.sem_flg = 0;

	if (semop(sem_id, &op, 1) == ERROR) {
		return ERROR;
	}
	return EXIT_SUCCESS;
}

int destroySem(long sem_id) {
	semctl(sem_id, IPC_RMID, 0);
	return EXIT_SUCCESS;

}
