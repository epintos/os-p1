#include "marshalling.h"
#include <stdio.h>
#include <arpa/inet.h>
#include <signal.h>
#include "companyReader.h"
#include "fileManager.h"
#include "company.h"
#include <unistd.h>
#include "semaphore.h"
#include "defines.h"
#include <pthread.h>
#include <stdlib.h>

static void initializeSigHandler();
static void initializeThreads(pthread_attr_t *attr, pthread_t **threads,
		int airplaneQty);
static void sincronizeThreads();
static void catchint(int);
static void catchterm(int);

pthread_mutex_t planeMutex, cmpMutex;
pthread_cond_t planeVar, cmpVar;
pthread_t *threads;
static int counter = 0;
static int threadsFlag = 1;
static int aux = 0;
static communication ** shmem;
static int threadQty = 0;
static struct sigaction act;
static struct sigaction act2;
static int id = 0;

int main(int argc, char **argv) {

	srand(getpid());
	initializeSigHandler();

	pthread_attr_t attr;
	company * cmp;
	char* msg;
	int i = 0;
	id = atoi(argv[0]);
	int ipcsQty = atoi(argv[1]);
	int sem_id = atoi(argv[2]);
	int cmpQty = ipcsQty - ID_QTY;
	int mainFlag = 1;

	if (createIPC(id, ipcsQty) == ERROR
	)
		return ERROR;

	upSem(sem_id, id);

	/***INITIALIZE COMPANY***/
	msg = receiveMsg(id);
	cmp = deserializeCompany(msg);
	downSem(sem_id, id);

	sendPID(id, MAP_ID, getpid());

	initializeThreads(&attr, &threads, cmp->airplaneQty);

	/***STARTS PLANES MOVEMENTS***/

	sendInitialMed(id, IO_ID);
	for (i = 0; i < cmp->airplaneQty; i++) {
		mapAnswer * ma = NULL;
		communication * c = malloc(sizeof(communication));
		parameter * param = malloc(sizeof(parameter));
		if (param == NULL || c == NULL
		)
			exit(1);
		c->cityName = cmp->airplaneList[i]->actualCity; //plane starting city
		c->deliveredMeds = cmp->airplaneList[i]->medsList; //plane medicine stock
		c->modif = 1; //plane has medicines left to give out

		sendUpdate(c, id, MAP_ID); // sends to the map, starting city, stock of the plane and cmp id, the map updates the necessities
		char * msg = receiveMsg(id);
		ma = buildCityInfo(msg);
		c->travelTime = choseCity(ma, &(c->cityName)); //Assigns new city where is going and the travel time
		c->deliveredMeds = ma->list;

		shmem[i] = c;
		param->plane = cmp->airplaneList[i];
		param->tid = i;
		pthread_create(&threads[i], NULL, airplaneThread, (void*) param);
	}
	sendFirstTurn(id, IO_ID);
	for (i = 0; i < threadQty; i++) {
		communication * c = shmem[i];

		sendPlan(i + 1, c, id, IO_ID);

	}

	while (mainFlag) {
		int travelTime = 0;
		char * msg = NULL;
		char option;

		sendTravelTime(id);
		option = getUpdate(id, &msg);
		switch (option) {
		case MULTITASKER:
			travelTime = getTravelTime(msg); // Receives the min time the multitasker calculated from all the companies
			mainFlag = processAllPlanes(travelTime, id);
			break;
		}
		upSem(sem_id, cmpQty + ID_QTY);
		downSem(sem_id, cmpQty + SEM_SINC);
		sincronizeThreads();

	}
	threadsFlag = 0; //With this the while ends, so the planes end.
	pthread_mutex_lock(&planeMutex);
	pthread_cond_broadcast(&planeVar); //Wakes up the planes, so the join finishes OK
	pthread_mutex_unlock(&planeMutex);

	for (i = 0; i < cmp->airplaneQty; i++) {
		pthread_join(threads[i], NULL); //Company waits for the planes to finish
	}

	disconnectFromIPC(id);

	return EXIT_SUCCESS;
}

void sendTravelTime(int my_id) {
	int index = 0, travelTime = -1;

	for (index = 0; index < threadQty; index++) {
		communication * c = shmem[index];
		if (c->travelTime < travelTime || travelTime == -1) {
			travelTime = c->travelTime;
		}
	}

	sendSmallestTravelTime(travelTime, my_id, MULTITASKER_ID);
}

int processAllPlanes(int time, int my_id) {

	int index = 0, flag = 0;

	for (index = 0; index < threadQty; index++) {
		if (shmem[index]->modif) {
			flag = 1;
			communication * c = shmem[index];
			c->travelTime -= time;
			if (c->travelTime != 0) {
				sendPlan(index + 1, c, my_id, IO_ID);
			}

			//Airplane arrives
			if (c->travelTime == 0) {
				mapAnswer * ma = NULL;
				char * msg = NULL;
				sendUpdate(c, my_id, MAP_ID);
				msg = receiveMsg(my_id);
				char *auxCity = c->cityName;

				//Assigns new destiny
				ma = buildCityInfo(msg);
				c->travelTime = choseCity(ma, &(c->cityName));
				c->deliveredMeds = ma->list;
				sendLand(index + 1, auxCity, c, my_id, IO_ID);

			}
		}

	}

	return flag;
}

int choseCity(mapAnswer * ma, char ** cityName) {
	int i = (int) ((double) ma->cityQty * rand() / (RAND_MAX + 1.0));
	cityList * list = ma->first;
	while (i-- > 0) {
		if (list->next != NULL
		)
			list = list->next;
	}
	*cityName = (list->cityName);
	return ma->first->travelTime;
}

static void sincronizeThreads() {
	while (aux < threadQty)
		;
	pthread_mutex_lock(&planeMutex);
	aux = 0;
	pthread_cond_broadcast(&planeVar); //Wakes planes
	pthread_mutex_lock(&cmpMutex);
	pthread_mutex_unlock(&planeMutex);
	pthread_cond_wait(&cmpVar, &cmpMutex); //Sleeps company
	pthread_mutex_unlock(&cmpMutex);
}

void * airplaneThread(void *threadid) {

	parameter * param = malloc(sizeof(parameter));
	param = (parameter *) threadid;

	while (threadsFlag) {
		int i = 0, flag = 1;
		medNode * node = shmem[param->tid]->deliveredMeds->first;
		pthread_mutex_lock(&planeMutex);
		aux++;
		pthread_cond_wait(&planeVar, &planeMutex); //sleeps plane
		for (i = 0; i < shmem[param->tid]->deliveredMeds->nodesCount && flag;
				i++) {
			if (node->med->amountNeeded != 0)
				flag = 0;
		}
		//There are meds to be delivered
		if (flag == 0)
			shmem[param->tid]->modif = 1;
		//The plane has no meds left
		else
			shmem[param->tid]->modif = 0;
		counter++;
		if (counter == threadQty) {
			counter = 0;
			pthread_mutex_lock(&cmpMutex);
			pthread_cond_signal(&cmpVar); //wakes cmp
			pthread_mutex_unlock(&cmpMutex);
		}
		pthread_mutex_unlock(&planeMutex);
	}

	pthread_exit(NULL);

	return (void *) EXIT_SUCCESS;
}

static void catchint(int sig) {
	int i = 0;
	threadsFlag = 0; //With this the while ends, so the planes end.
	pthread_mutex_lock(&planeMutex);
	pthread_cond_broadcast(&planeVar); //Wakes up the planes, so the join finishes OK
	pthread_mutex_unlock(&planeMutex);

	for (i = 0; i < threadQty; i++) {
		pthread_join(threads[i], NULL); //Company waits for the planes to finish
	}
	disconnectFromIPC(id);
	exit(1);

}

static void catchterm(int sig) {
	int i = 0;
	threadsFlag = 0; //With this the while ends, so the planes end.
	pthread_mutex_lock(&planeMutex);
	pthread_cond_broadcast(&planeVar); //Wakes up the planes, so the join finishes OK
	pthread_mutex_unlock(&planeMutex);

	for (i = 0; i < threadQty; i++) {
		pthread_join(threads[i], NULL); //Company waits for the planes to finish
	}
	disconnectFromIPC(id);
	exit(0);
}

static void initializeSigHandler() {
	act.sa_handler = catchint;
	sigfillset(&(act.sa_mask));
	sigaction(SIGINT, &act, NULL);

	act2.sa_handler = catchterm;
	sigfillset(&(act2.sa_mask));
	sigaction(SIGTERM, &act2, NULL);
}

static void initializeThreads(pthread_attr_t *attr, pthread_t **threads,
		int airplaneQty) {
	pthread_attr_init(attr);
	pthread_attr_setdetachstate(attr, PTHREAD_CREATE_JOINABLE);

	pthread_mutex_init(&planeMutex, NULL);
	pthread_mutex_init(&cmpMutex, NULL);
	pthread_cond_init(&planeVar, NULL);
	pthread_cond_init(&cmpVar, NULL);

	*threads = malloc(sizeof(pthread_t) * airplaneQty);
	shmem = malloc(sizeof(communication) * airplaneQty);
	threadQty = airplaneQty;
}

