/* Message queue IPC using System V functions.
 * There is only one message queue with key QUEUE_KEY for all the processes*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include "ipcs.h"
#include "semaphore.h"

#define MSG_MAX_LENGTH 1024
#define QUEUE_KEY 0xFACEB00C
#define MSGQUEUE_SEM_KEY 0xdeadfe0
#define P_RDWR 0600
#define MALLOC_ERROR -2
#define READ 0
#define WRITE 1

struct mult_msg {
	int multiple;
	char msg[MSG_MAX_LENGTH];
};
struct msg_buffer {
	long mtype;
	struct mult_msg ms;
};

void static initialize_msg_str(struct mult_msg *msg, int dest_id,
		struct msg_buffer *msg_str);
int writeMsg(char * source, char ** dest, int * index);

static int myQueue_id = 0;
static int *msgQueuesID = NULL;
static int sem_id = 0;

int createIPC(int id, int ipcs_qty) {

	int i = 0;
	msgQueuesID = malloc(ipcs_qty * sizeof(int));
	for (i = 0; i < ipcs_qty; i++)
		msgQueuesID[i] = -1;

	myQueue_id = msgget(id + 50, P_RDWR | IPC_CREAT);
	sem_id = createSem(MSGQUEUE_SEM_KEY, ipcs_qty);
	setSem(sem_id, id, 1);

	return EXIT_SUCCESS;

}
int sendMessage(char *msg, int id, int dest_id, int size) {
	int msgQty = 1, i = 0;
	downSem(sem_id, dest_id);

	if (msgQueuesID[dest_id] == -1) {
		msgQueuesID[dest_id] = msgget(dest_id + 50, P_RDWR);
	}

	if (MSG_MAX_LENGTH < size) {
		msgQty = size / MSG_MAX_LENGTH + 1;
	}
	for (i = 0; i < msgQty; i++) {
		int j = 0, flag = 1;
		struct msg_buffer msg_str;
		struct mult_msg ms;
		ms.multiple = msgQty - i - 1;
		for (j = 0; j < MSG_MAX_LENGTH && flag; j++) {
			ms.msg[j] = msg[j + MSG_MAX_LENGTH * i];
			if (ms.msg[j] == '\0') {
				flag = 0;
			}
		}
		initialize_msg_str(&ms, dest_id, &msg_str);
		if (msgsnd(msgQueuesID[dest_id], &msg_str, sizeof(struct mult_msg), 0)
				== ERROR
				) {
			return ERROR;
		}
	}
	upSem(sem_id, dest_id);
	return EXIT_SUCCESS;

}
int getMessage(char **msg, int id) {
	int rbytes = 0, index = 0;

	struct msg_buffer* msg_buf = (struct msg_buffer*) malloc(
			sizeof(struct msg_buffer));
	do {
		if (msg_buf == NULL
		)
			return MALLOC_ERROR;
		if ((rbytes = msgrcv(myQueue_id, msg_buf, sizeof(struct mult_msg), id,
				0)) == ERROR) {
			return ERROR;
		}
		writeMsg(msg_buf->ms.msg, msg, &index);
	} while (msg_buf->ms.multiple != 0);

	return rbytes;
}

int disconnectFromIPC(int id) {
	msgctl(myQueue_id, IPC_RMID, NULL);
	free(msgQueuesID);
	return ERROR;
}

int destroyIPC(int id) {
	destroySem(sem_id);
	return EXIT_SUCCESS;
}

void static initialize_msg_str(struct mult_msg *msg, int dest_id,
		struct msg_buffer *msg_str) {
	msg_str->mtype = dest_id;
	msg_str->ms.multiple = msg->multiple;
	memcpy(msg_str->ms.msg, msg->msg, MSG_MAX_LENGTH);
}

int writeMsg(char * source, char ** dest, int * index) {
	int i = 0, flag = 1;
	char * aux;

	for (i = 0; i < MSG_MAX_LENGTH && flag; i++) {
		if (*index % MSG_MAX_LENGTH == 0) {
			aux = realloc((*dest), ((*index) + MSG_MAX_LENGTH) * sizeof(char));
			if (aux == NULL
			)
				return ERROR;
			(*dest) = aux;
		}
		(*dest)[(*index)++] = source[i];
		if (source[i] == '\0')
			flag = 0;
	}

	return EXIT_SUCCESS;
}
