#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include "ipcs.h"
#include "semaphore.h"

#define STR_SIZE sizeof(struct sockaddr_un)
#define MAX_PATH_STR 108
#define MALLOC_ERROR -2
#define ERROR -1
#define SEM_KEY 0x200

static void initialize_socket_addr(int id, struct sockaddr_un * socket_addr);
static int sem_id=0;
static int fd = 0;

int createIPC(int id, int sockets_qty) {


	if ((fd = socket(AF_UNIX, SOCK_DGRAM, 0)) == ERROR
		)
		return ERROR;

	struct sockaddr_un socket_addr;
	initialize_socket_addr(id, &socket_addr);

	if ((bind(fd, (struct sockaddr *) &socket_addr, STR_SIZE) == ERROR))
		return ERROR;

	sem_id = createSem(SEM_KEY, sockets_qty);
	setAllSem(sem_id, sockets_qty, 1);

	return EXIT_SUCCESS;

}

int sendMessage(char *msg, int id, int dest_id, int size) {

	int sbytes = 0;

	downSem(sem_id, dest_id);

	struct sockaddr_un socket_addr;
	initialize_socket_addr(dest_id, &socket_addr);

	if (sendto(fd, &size, sizeof(int), 0, (struct sockaddr *) &socket_addr,
			STR_SIZE) == ERROR
		)
		return ERROR;

	if ((sbytes = sendto(fd, msg, size, 0, (struct sockaddr *) &socket_addr,
			STR_SIZE)) == ERROR
		)
		return ERROR;

	upSem(sem_id, dest_id);

	return sbytes;
}

int getMessage(char **msg, int unused) {

	int size = 0;
	int rbytes = 0;

	if (recvfrom(fd, &size, sizeof(int), 0, NULL, NULL) == ERROR
		)
		return ERROR;

	(*msg) = malloc(size);
	if ((*msg) == NULL
		)
		return MALLOC_ERROR;

	if ((rbytes = recvfrom(fd, *msg, size, 0, NULL, NULL)) == ERROR
		)
		return ERROR;

	return rbytes;
}

int disconnectFromIPC(int id) {
	return destroyIPC(id);
}

int destroyIPC(int id) {

	close(fd);
	char *path = malloc(sizeof(char) * MAX_PATH_STR);
	sprintf(path, "/tmp/socket_id%d", id);
	unlink(path);
	free(path);
	destroySem(sem_id);
	return EXIT_SUCCESS;
}

static void initialize_socket_addr(int id, struct sockaddr_un * socket_addr) {

	socket_addr->sun_family = AF_UNIX;
	char *path = malloc(sizeof(char) * MAX_PATH_STR);
	sprintf(path, "/tmp/socket_id%d", id);
	strcpy(socket_addr->sun_path, path);
	free(path);
}

