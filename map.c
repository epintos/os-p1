#include <stdio.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "marshalling.h"
#include "defines.h"
#include "fileManager.h"
#include "graph.h"
#include "map.h"
#include "simulation.h"
#include "semaphore.h"

static void initializeSigHandler();
static void getMedsFromUpdate(airlineUpdate * au, graph * map, int index);
static int finished(graph * map);

static struct sigaction act;
static struct sigaction act2;
static void catchint(int);
static void catchterm(int);
static int sim_finished = 0;
static int sem_id = 0;
static graph *map = NULL;
static int cmpQty=0;

int main(int argc, char **argv) {

	initializeSigHandler();

	int pidsQty = 0;
	sem_id = atoi(argv[0]);
	int ipcsQty = atoi(argv[1]);
	int* pids = malloc(sizeof(int) * (ipcsQty - ID_QTY));
	airlineUpdate * au = NULL;
	char option;
	char *msg = NULL;
	cmpQty = ipcsQty - ID_QTY;
	int i = 0;

	if (createIPC(MAP_ID, ipcsQty) == ERROR
	)
		return ERROR;

	upSem(sem_id, MAP_ID);

	getUpdate(MAP_ID, &msg);
	map = deserializeMap(msg);
	upSem(sem_id, MAP_ID);
	for (i = 0; i < cmpQty; i++) {
		upSem(sem_id, CMP_ID + i);
	}
	msg = NULL;

	sendMapNecessities(map);
	downSem(sem_id, cmpQty+ID_QTY+2);

	while (TRUE) {
		option = getUpdate(MAP_ID, &msg);
		switch (option) {
		case NEWTURN:
			sendMapNecessities(map);
			break;
		case PID:
			pids[pidsQty++] = receivePID(msg);
			break;
		case INITIALIZE:
			map = deserializeMap(msg);
			break;
		case AIRLINE:
			au = getAirlineUpdate(msg);
			processUpdate(au, map, ipcsQty - ID_QTY, pids);
			break;
		}
	}

	return EXIT_SUCCESS;
}

void processUpdate(airlineUpdate * au, graph* map, int cmpQty, int *pids) {
	if (sim_finished != TRUE) {
		int index = 0;

		if (au == NULL || map == NULL) {
			return;
		}

		index = existsCity(au->cityName, map);
		getMedsFromUpdate(au, map, index);

		if (finished(map) != TRUE) {
			sendNeighboursAndCityInfo(au, index, map);
		} else {
			int j;
			for (j = 0; j < cmpQty; j++) {
				kill(pids[j], SIGTERM);
			}
		}
	}

}

static void getMedsFromUpdate(airlineUpdate * au, graph * map, int index) {
	medNode * mn = map->nodes[index]->info->medicineList->first;
	int medCount = map->nodes[index]->info->medicineList->nodesCount;
	medNode * auMedNode = au->list->first;
	int auCount = au->list->nodesCount;
	int i = 0, j = 0, flag = 1;

	for (i = 0; i < auCount && flag; i++) {
		for (j = 0; j < medCount && flag; j++) {
			if (strcmp(auMedNode->med->medicineName, mn->med->medicineName)
					== 0) {
				flag = 0;
				if (auMedNode->med->amountNeeded > mn->med->amountNeeded) {
					auMedNode->med->amountNeeded -= mn->med->amountNeeded;
					mn->med->amountNeeded = 0;
				} else if (auMedNode->med->amountNeeded
						< mn->med->amountNeeded) {
					mn->med->amountNeeded -= auMedNode->med->amountNeeded;
					auMedNode->med->amountNeeded = 0;
				} else {
					auMedNode->med->amountNeeded = 0;
					mn->med->amountNeeded = 0;
				}

			} else
				mn = mn->next;

		}
		mn = map->nodes[index]->info->medicineList->first;
		auMedNode = auMedNode->next;
		flag = 1;

	}
}

static int finished(graph * map) {

	int i = 0, j = 0, flag = 1;

	for (i = 0; i < map->nodeCount && flag; i++) {
		medList * ml = map->nodes[i]->info->medicineList;
		medNode * med = ml->first;
		for (j = 0; j < ml->nodesCount && flag; j++) {
			medicine * m = med->med;
			if (m->amountNeeded > 0) {
				flag = 0;
			} else
				med = med->next;
		}

	}
	if (flag) {
		sim_finished = TRUE;
		sendMapNecessities(map);
		downSem(sem_id, cmpQty+SEM_TURN);
	}
	return flag;
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

	disconnectFromIPC(MAP_ID);
	exit(1);

}

static void catchterm(int sig) {
	disconnectFromIPC(MAP_ID);
	exit(0);
}
