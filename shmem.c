#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include "ipcs.h"
#include "semaphore.h"
#include <string.h>

#define SHMEM_KEY 0x50045612
#define SHMEM_SIZE 1024*30
#define SEM_RD_KEY 0xDEAD3453
#define SEM_WR_KEY 0xDEADBEAF
#define SEM_T_KEY 0x12344446
#define MSG_MAX_LENGTH 256

static void printShMem();
static int writeMsg(char * source, char ** dest, int * index);

typedef struct {
	int next;
	int multMsg;
	char msg[MSG_MAX_LENGTH];
	int unused;
} memBlock;

/*
 * Memory block result from shmget.
 * */
static void * mem = NULL;
/*
 * The first ipcs_qty bytes from mem will be used as an array to indicate the index of the first message for each ipc in "blocks"
 * */
static int * unused = NULL;
/*
 * The second ipcs_qty bytes from mem will be used as an array to indicate the index of the last message for each ipc in "blocks"
 * */
static int * last = NULL;
static memBlock * blocks = NULL;
static int blockQty = 0;
static int shmid = 0;
static int ipcs_Qty = 0;
static int rdSemID = 0;
static int wrSemID = 0;
static int semID = 0;

int searchFirstFreeBlock();
void initialize();

int createIPC(int id, int ipcs_qty) {

	ipcs_Qty = ipcs_qty;

	semID = createSem(SEM_T_KEY, 1);

	rdSemID = createSem(SEM_RD_KEY, ipcs_Qty);

	wrSemID = createSem(SEM_WR_KEY, 1);

	if ((shmid = shmget(SHMEM_KEY, SHMEM_SIZE, 0600 | IPC_CREAT | IPC_EXCL))
			< 0) {
		if ((shmid = shmget(SHMEM_KEY, SHMEM_SIZE, IPC_CREAT | 0600)) < 0) {
			exit(1);
		} else {
			mem = shmat(shmid, NULL, 0);
			unused = mem;
			last = (int *) ((unsigned int) mem + ipcs_Qty * sizeof(int));
			blocks = (memBlock*) ((unsigned int) mem
					+ 2 * ipcs_Qty * sizeof(int));
			blockQty = (SHMEM_SIZE - 2 * ipcs_Qty * sizeof(int))
					/ sizeof(memBlock);
			while(unused[0] != -1);
		}
	} else {
		initialize();

	}

	if (mem == (void*) -1)
		return ERROR;

	return EXIT_SUCCESS;
}

void initialize() {
	setAllSem(rdSemID, ipcs_Qty, 0);
	setSem(semID, 0, 1);
	setSem(wrSemID, 0, 1);

	mem = shmat(shmid, NULL, 0);
	blockQty = (SHMEM_SIZE - 2 * ipcs_Qty * sizeof(int)) / sizeof(memBlock);
	int j = 0;

	unused = mem;
	last = (int *) ((unsigned int) mem + ipcs_Qty * sizeof(int));
	blocks = (memBlock*) ((unsigned int) mem + 2 * ipcs_Qty * sizeof(int));

	for (j = 0; j < ipcs_Qty; j++) {
		unused[j] = -1;
		last[j] = -1;
	}
	for (j = 0; j < blockQty; j++) {
		blocks[j].unused = 1;
	}

}

int searchFirstFreeBlock() {
	int i = 0;

	for (i = 0; i < blockQty; i++) {
		if (blocks[i].unused)
			return i;
	}

	return ERROR;
}

int sendMessage(char *msg, int id, int dest_id, int size) {

	int offset = 0, msgQty = 1, i = 0;
	downSem(semID, 0);

	if (strlen(msg) > MSG_MAX_LENGTH) {
		msgQty = strlen(msg) / MSG_MAX_LENGTH + 1;
	}

	for (i = 0; i < msgQty; i++) {
		int j = 0, flag = 1;
		do {
			offset = searchFirstFreeBlock();
		} while (offset == ERROR);

		downSem(wrSemID, 0);
		if (unused[dest_id] == -1) {
			unused[dest_id] = offset;
			last[dest_id] = offset;
		} else {
			blocks[last[dest_id]].next = offset;
			last[dest_id] = offset;
		}
		upSem(wrSemID, 0);

		blocks[offset].next = -1;
		blocks[offset].unused = 0;
		for (j = 0; j < MSG_MAX_LENGTH && flag; j++) {
			blocks[offset].msg[j] = msg[j + MSG_MAX_LENGTH * i];
			if (blocks[offset].msg[j] == '\0')
				flag = 0;
		}
		blocks[offset].multMsg = msgQty - i - 1;
	}
	upSem(rdSemID, dest_id);
	upSem(semID, 0);

	return EXIT_SUCCESS;
}

int getMessage(char **msg, int id) {

	int aux = 0;
	int i = 0, flag = 1;
	downSem(rdSemID, id);
	if (unused[id] == -1)
		printf(
				"El ID %d esta intentando recibir pero no tiene mensajes disponibles\n",
				id);
	(*msg) = malloc(10);

	while (flag) {
		writeMsg(blocks[unused[id]].msg, msg, &i);
		downSem(wrSemID, 0);
		aux = unused[id];

		unused[id] = blocks[aux].next;
		if (blocks[aux].next == -1) {
			last[id] = -1;
		}
		if (blocks[aux].multMsg == 0)
			flag = 0;
		blocks[aux].unused = 1;
		upSem(wrSemID, 0);

	}
	return EXIT_SUCCESS;
}

static int writeMsg(char * source, char ** dest, int * index) {
	int i = 0;
	char * aux;

	for (i = 0; i < MSG_MAX_LENGTH && source[i] != '\0'; i++) {
		if (*index % MSG_MAX_LENGTH == 0) {
			aux = realloc((*dest), ((*index) + MSG_MAX_LENGTH) * sizeof(char));
			if (aux == NULL
			)
				return ERROR;
			(*dest) = aux;
		}
		(*dest)[(*index)++] = source[i];
	}
	return EXIT_SUCCESS;
}

int disconnectFromIPC(int id) {
	return ERROR;
}

int destroyIPC(int id) {
	downSem(semID, 0);
	downSem(wrSemID, 0);
	shmdt(mem);
	shmctl(shmid, IPC_RMID, 0);
	destroySem(rdSemID);
	destroySem(semID);
	destroySem(wrSemID);
	return EXIT_SUCCESS;
}

static void printShMem() {
	int i = 0;
	for (i = 0; i < ipcs_Qty; i++) {
		printf("Unused %d vale %d y last vale %d\n", i, unused[i], last[i]);
	}
}
