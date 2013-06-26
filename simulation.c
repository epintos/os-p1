#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "defines.h"
#include "ipcs.h"
#include "marshalling.h"
#include "defines.h"
#include "graph.h"
#include "fileManager.h"
#include "company.h"
#include "companyReader.h"
#include "simulation.h"
#include "semaphore.h"
#include "free.h"
#include <time.h>

#define SEM_CREAT_KEY 0xCA6E

static void initializeSigHandler();
static void initializeArray(char **msg, int number);
static void catchint(int);
static void catchterm(int);
static void freeAll();
static void freePid();
static int addPid(int pid);
static int initPIDS();

static pids* processID;
static struct sigaction act;
static struct sigaction act2;
static int sem_id = 0;
static char *semMsg = NULL;
static char *procQty = NULL;
static int cmpQty = 0;
static graph * map = NULL;
static company **cmp = NULL;

int main(int argc, char *argv[]) {

	initializeSigHandler();

	setpgid(0, 0);
	int i = 0;
	int ret = 0;
	pid_t pid;
	cmpQty = argc - 2;
	cmp = malloc(sizeof(company*) * cmpQty);
	map = malloc(sizeof(graph));
	sem_id = createSem(SEM_CREAT_KEY, cmpQty + SEM_QTY);
	setAllSem(sem_id, cmpQty + SEM_QTY, 0);

	if (initPIDS() == MALLOC_ERROR
	)
		return MALLOC_ERROR;

	/***PARSER***/
	if (cmp == NULL || map == NULL
	)
		return parserError(NULL, NULL, MALLOC_ERROR, NULL);
	if ((ret = parseFiles(argc, argv, map, cmp)) != EXIT_SUCCESS
	)
		return ret;

	/***INITIALIZES ARRAYS FOR THE EXECL***/

	initializeArray(&semMsg, sem_id);
	initializeArray(&procQty, ID_QTY + cmpQty);

	/***MAP AND COMPANIES CREATION***/
	if (createIPC(MAIN_ID, cmpQty + ID_QTY) == ERROR
	)
		return ERROR;

	int map_pid = 0;
	int io_pid = 0;
	int mult_pid = 0;
	if ((pid = fork()) == 0) {
		execl("io", semMsg, procQty, NULL);
		exit(1);
	} else {
		io_pid = pid;
		addPid(io_pid);
		if ((pid = fork()) == 0) {
			execl("map", semMsg, procQty, NULL);
			exit(1);
		} else {
			map_pid = pid;
			addPid(map_pid);
			if ((pid = fork()) == 0) {
				execl("multitasker", semMsg, procQty, NULL);
				exit(1);
			} else {
				mult_pid = pid;
				addPid(mult_pid);
				for (i = 0; i < cmpQty; i++) {
					if ((pid = fork()) == 0) {
						char *buf = NULL;
						int id = CMP_ID + i;
						initializeArray(&buf, id);
						execl("company", buf, procQty, semMsg, NULL);
					} else {
						addPid(pid);
					}
				}
			}
		}
	}
	upSem(sem_id, MAIN_ID);
	downSem(sem_id, IO_ID);
	downSem(sem_id, MULTITASKER_ID);
	for (i = 0; i < cmpQty; i++) {
		downSem(sem_id, CMP_ID + i);
		if (initializeCmp(cmp[i], MAIN_ID, CMP_ID + i) == ERROR
		)
			return ERROR;
	}
	downSem(sem_id, MAP_ID);
	if (initializeMap(map, MAIN_ID, MAP_ID) == ERROR) {
		return ERROR;
	}
	downSem(sem_id, MAP_ID);

	for (i = 0; i < cmpQty; i++) {
		wait(0); //Waits for all the companies to finish
	}
	kill(mult_pid, SIGTERM);
	waitpid(mult_pid, 0, 0);

	kill(map_pid, SIGTERM);
	waitpid(map_pid, 0, 0);

	kill(io_pid, SIGTERM);
	waitpid(io_pid, 0, 0);

	if (disconnectFromIPC(MAIN_ID) == ERROR) {
		destroyIPC(MAIN_ID);
	}
	destroySem(sem_id);

	/***FREES***/
	freeAll();

	return EXIT_SUCCESS;

}

static void freeAll() {
	int i = 0;
	for (i = 0; i < cmpQty; ++i) {
		freeCompany(cmp[i]);
	}
	free(cmp);
	freeMap(map);
	freePid();
	free(semMsg);
	free(procQty);
}

int parseFiles(int argc, char *argv[], graph * map, company ** cmp) {
	int i = 0;
	int ret = 0;
	if (argc < 3)
		return parserError(NULL, NULL, INVALID_ARGUMENTS, NULL);
	if ((ret = openAndReadCityFile(argv[1], map)) != EXIT_SUCCESS
	)
		return ret;
	for (i = 0; i < argc - 2; i++) {
		cmp[i] = NULL;
	}
	for (i = 2; i < argc; ++i) {
		if ((ret = openAndReadCompanyFile(argv[i], cmp, i - 2, map))
				!= EXIT_SUCCESS
				)
			return ret;
	}
	return EXIT_SUCCESS;
}

static int addPid(int pid) {

	if (processID->cantPids % BLOCK == 0) {
		int*aux = realloc(processID->pids,
				(processID->cantPids + BLOCK) * sizeof(int));
		if (aux == NULL
		)
			return MALLOC_ERROR;
		processID->pids = aux;
	}
	(processID->pids)[processID->cantPids] = pid;
	(processID->cantPids)++;
	return EXIT_SUCCESS;
}

static int initPIDS() {
	processID = malloc(sizeof(pids));
	if (processID == NULL
	)
		return MALLOC_ERROR;
	processID->cantPids = 0;
	processID->pids = NULL;
	return EXIT_SUCCESS;
}

static void initializeSigHandler() {
	act.sa_handler = catchint;
	sigfillset(&(act.sa_mask));
	sigaction(SIGINT, &act, NULL);

	act2.sa_handler = catchterm;
	sigfillset(&(act2.sa_mask));
	sigaction(SIGTERM, &act2, NULL);
}

static void catchint(int signal) {
	int i;
	for (i = 0; i < processID->cantPids; i++) {
		wait(0); // Waits for child processes to abort
	}

	printf("Aborting Simulation\n");
	freeAll();
	if (disconnectFromIPC(MAIN_ID) == ERROR) {
		destroyIPC(MAIN_ID);
	}
	destroySem(sem_id);

	exit(1);

}

static void catchterm(int sig) {
	if (disconnectFromIPC(MAIN_ID) == ERROR) {
		destroyIPC(MAIN_ID);
	}
	destroySem(sem_id);
	freeAll();
	exit(0);
}

static void freePid() {
	free(processID->pids);
	free(processID);
}

static void initializeArray(char **msg, int number) {
	int index = 0;
	*msg = malloc(sizeof(char) * BLOCK);
	addNumber(msg, &index, number);
	(*msg)[index - 1] = '\0';
}
