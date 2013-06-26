#include "marshalling.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include "defines.h"
#include "fileManager.h"
#include "semaphore.h"
#include "graph.h"
#include <stdlib.h>

static void initializeSigHandler();

static struct sigaction act;
static struct sigaction act2;
static void catchint(int);
static void catchterm(int);
static long long int simulTime = 0;
static int sem_id = 0;
int main(int argc, char ** argv) {

	initializeSigHandler();

	int i = 0;
	char option;
	char * msg = NULL;
	int travelTime = UNDEF_TIME;
	int aux = 0;
	sem_id = atoi(argv[0]);
	int ipcsQty = atoi(argv[1]);
	int cmpQty = ipcsQty - ID_QTY;
	sigset_t * blockMask = 0;
	sigset_t * oldMask=NULL;
	sigemptyset(blockMask);
	sigaddset(blockMask, SIGTERM);
	simulTime = 0;

	if (createIPC(MULTITASKER_ID, ipcsQty) == ERROR
	)
		return ERROR;

	upSem(sem_id, MULTITASKER_ID);

	while (TRUE) {

		sigprocmask(SIG_BLOCK, blockMask, oldMask);
		for (i = 0; i < cmpQty; i++) {
			option = getUpdate(MULTITASKER_ID, &msg);

			switch (option) {
			case MULTITASKER:
				aux = getTravelTime(msg);
				if (aux < travelTime || travelTime == UNDEF_TIME
				)
					travelTime = aux;
				break;
			}

		}
		sendStartTurn(MULTITASKER_ID, IO_ID); //Writes Turn
		downSem(sem_id, cmpQty + SEM_START);

		sendStartTurn(MULTITASKER_ID, MAP_ID); //Writes necessities
		downSem(sem_id, cmpQty + SEM_TURN);

		simulTime += travelTime;
		sendSimulTime(simulTime, MULTITASKER_ID, IO_ID); //Sends simulation time
		downSem(sem_id, cmpQty + SEM_TIME);

		for (i = 0; i < cmpQty; i++) {
			sendSmallestTravelTime(travelTime, MULTITASKER_ID, CMP_ID + i);
		}
		for (i = 0; i < cmpQty; i++) {
			downSem(sem_id, cmpQty + ID_QTY);
		}
		for (i = 0; i < cmpQty; i++) {
			upSem(sem_id, cmpQty + SEM_SINC);
		}
		sigprocmask(SIG_UNBLOCK, oldMask, NULL);
		travelTime = UNDEF_TIME;
	}

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

static void catchint(int sig) {
	disconnectFromIPC(MULTITASKER_ID);
	exit(1);
}

static void catchterm(int sig) {
	sendSimulTime(simulTime, MULTITASKER_ID, IO_ID);
	disconnectFromIPC(MULTITASKER_ID);
	exit(0);
}

