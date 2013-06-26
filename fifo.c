#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include "ipcs.h"
#include "semaphore.h"

#define MAX_PATH 14
#define MALLOC_ERROR -2
#define UNUSED_FD -3
#define SEM_KEY 0xFEED

static int *fds = NULL;
static int fd = 0;
static int fifosQty = 0;
static int sem_id = 0;


int createIPC(int id, int fifos_qty) {
	int i = 0;
	char *path = malloc(sizeof(char) * MAX_PATH);
	if (path == NULL
		)
		return MALLOC_ERROR;

	sprintf(path, "/tmp/fifo_id%d", id);
	if (mkfifo(path, 0600 | IPC_CREAT) == ERROR) {
		return ERROR;
	}
	if ((fd = open(path, O_RDWR)) == ERROR) {
		return ERROR;
	}

	fds = malloc(sizeof(int) * fifos_qty);
	if (fds == NULL
		)
		return MALLOC_ERROR;

	for (i = 0; i < fifos_qty; i++) {
		fds[i] = UNUSED_FD;
	}

	fifosQty = fifos_qty;
	free(path);

	sem_id = createSem(SEM_KEY, fifos_qty);
	setAllSem(sem_id, fifos_qty, 1);

	return EXIT_SUCCESS;
}

int sendMessage(char *msg, int id, int dest_id, int size) {

	downSem(sem_id, dest_id);
	int wbytes = 0;
	int dest_fd = fds[dest_id];
	char *path = malloc(sizeof(char) * MAX_PATH);
	if (dest_fd == UNUSED_FD) {
		sprintf(path, "/tmp/fifo_id%d", dest_id);
		if ((dest_fd = open(path, O_RDWR)) == ERROR) {
			perror("Open error:");
			return ERROR;
		}
		fds[dest_id] = dest_fd;
	}

	if (write(dest_fd, &size, sizeof(int)) == ERROR
		)
		return ERROR;

	if ((wbytes = write(dest_fd, msg, size)) == ERROR
		)
		return ERROR;
	free(path);
	upSem(sem_id, dest_id);
	return wbytes;
}

int getMessage(char **msg, int id) {

	int size = 0;
	int rbytes = 0;

	if (read(fd, &size, sizeof(int)) == ERROR
		)
		return ERROR;

	(*msg) = malloc(size);
	if ((*msg) == NULL
		)
		return MALLOC_ERROR;
	if ((rbytes = read(fd, *msg, size)) == ERROR
		)
		return ERROR;
	return rbytes;
}

int disconnectFromIPC(int id) {
	return destroyIPC(id);
}

int destroyIPC(int id) {
	int i = 0;
	for (i = 0; i < fifosQty; i++) {
		if (fds[i] != UNUSED_FD
			)
			close(fds[i]);
	}
	close(fd);
	free(fds);
	char *path = malloc(sizeof(char) * MAX_PATH);
	sprintf(path, "/tmp/fifo_id%d", id);
	unlink(path);
	destroySem(sem_id);
	free(path);

	return EXIT_SUCCESS;
}
