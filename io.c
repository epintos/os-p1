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
#include <string.h>
#include "io.h"

static char* getFileName(int number);
static void initializeSigHandler();

static struct sigaction act;
static struct sigaction act2;

static void catchint(int);
static void catchterm(int);

static FILE** files;
static int cmpQty = 0;
static FILE* mapFile;
static int simulTime = 0;

int main(int argc, char ** argv) {

	initializeSigHandler();
	Land* land;
	Plan* plan;
	Necessity* necessity;

	char* msg = NULL;
	char option = 0;
	int i = 0;
	int id = 0;

	int sem_id = atoi(argv[0]);
	int ipcsQty = atoi(argv[1]);

	cmpQty = ipcsQty - ID_QTY;

	mapFile = fopen("mapLog.txt", "w");
	if (mapFile == NULL
	)
		return ERROR;

	files = malloc(sizeof(FILE*) * (cmpQty));
	if (files == NULL
	)
		return MALLOC_ERROR;

	for (i = 0; i < cmpQty; i++) {
		char* filename = getFileName(i);
		files[i] = fopen(filename, "w");
		if (files[i] == NULL
		)
			return ERROR;
		free(filename);
	}

	if (createIPC(IO_ID, ipcsQty) == ERROR
	)
		return ERROR;

	upSem(sem_id, IO_ID);

	while (TRUE) {
		option = getUpdate(IO_ID, &msg);

		switch (option) {
		case INITIAL:
			id = getID(msg);
			setInitialMessage(id);
			break;
		case FIRSTTURN:
			id = getID(msg);
			writeFirstTurn(id);
			break;
		case NEWTURN:
			printStartTurn();
			writeStartTurn();
			upSem(sem_id, cmpQty + SEM_START);
			break;
		case SIMULTIME:
			simulTime = getSimulTime(msg);
			upSem(sem_id, cmpQty + SEM_TIME);
			break;
		case NECESITY:
			necessity = getNecessity(msg);
			writeNecessity(necessity);
			free(necessity);
			upSem(sem_id, cmpQty + SEM_TURN);
			break;
		case LAND:
			land = getLand(msg);
			printLand(land);
			writeLand(land);
			free(land);
			break;
		case PLAN:
			plan = getPlan(msg);
			printPlan(plan);
			writePlan(plan);
			free(plan);
			break;

		}

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
	closeCompanyLogs();
	fclose(mapFile);
	disconnectFromIPC(IO_ID);
	exit(1);
}

static void catchterm(int sig) {
	printSimulTime(simulTime);
	writeSimulTime(simulTime);
	closeCompanyLogs();
	fclose(mapFile);
	disconnectFromIPC(IO_ID);
	exit(0);
}

void printPlan(Plan* plan) {

	printf("El avion %d va hacia %s, tiempo de vuelo %d ( Compania ID: %d)\n",
			plan->planeNum, plan->cityName, plan->travelTime, plan->cmpID);

	return;
}

void printLand(Land* land) {

	printf(
			"Llego avion %d a %s, ahora va a %s, tiempo de vuelo %d (Compania ID: %d)\n",
			land->planeNum, land->cityNow, land->cityThen, land->travelTime,
			land->cmpID);
	return;
}

static char* getFileName(int number) {
	char* name = "aero";
	char* resp = malloc(sizeof(char) * 20);
	int counter = 4;
	strcpy(resp, name);
	addNumber(&resp, &counter, number);
	resp[counter - 1] = '\0';
	strcat(resp, "Log.txt");
	return resp;
}

void writePlan(Plan* plan) {

	fprintf(files[plan->cmpID - ID_QTY],
			"El avion %d va hacia %s, tiempo de vuelo %d \n", plan->planeNum,
			plan->cityName, plan->travelTime);
	return;
}

void writeLand(Land* land) {

	fprintf(files[land->cmpID - ID_QTY],
			"Llego avion %d a %s, ahora va a %s, tiempo de vuelo %d \n",
			land->planeNum, land->cityNow, land->cityThen, land->travelTime);
	return;
}

void writeNecessity(Necessity* necessity) {

	int it = 0;
	int imp = 0;
	medNode* cur = necessity->necesity->first;
	medNode* prev = NULL;
	fprintf(mapFile, "A %s le falta	", necessity->cityName);
	for (it = 0; it < necessity->necesity->nodesCount; it++) {
		prev = cur;
		if (cur->med->amountNeeded != 0) {
			fprintf(mapFile, "%d de %s, ", cur->med->amountNeeded,
					cur->med->medicineName);
		} else
			imp++;
		cur = (medNode*) (cur->next);
	}
	if (imp == necessity->necesity->nodesCount)
		fprintf(mapFile, "Nada \n");
	else
		fprintf(mapFile, "\n");

}

void closeCompanyLogs() {

	int i = 0;
	for (i = 0; i < cmpQty; i++) {
		fclose(files[i]);
	}
}

void setInitialMessage(int cmpID) {

	fprintf(files[cmpID - CMP_ID],
			"Dejando medicamentos en ciudades de salida \n");
	printf(
			"Aviones dejando medicamentos en ciudades de salida ( Compania ID: %d)\n",
			cmpID);
}

void printSimulTime(int simulTime) {
	printf("\nLa Simulacion ha finalizado, tardo %d unidades de tiempo\n",
			simulTime);
}

void writeSimulTime(int simulTime) {
	fprintf(mapFile,
			"\nLa Simulacion ha finalizado, tardo %d unidades de tiempo\n",
			simulTime);
}

void printStartTurn() {
	printf("\n--------------TURNO-----------------\n");
}

void writeStartTurn() {
	int i = 0;
	for (; i < cmpQty; i++) {
		fprintf(files[i], "\n--------------TURNO-----------------\n");
	}
	fprintf(mapFile, "\n--------------TURNO-----------------\n");
}

void writeFirstTurn(int id) {
	if (id == MAP_ID
	)
		fprintf(mapFile, "\n--------------TURNO-----------------\n");

	else
		fprintf(files[id - CMP_ID], "\n--------------TURNO-----------------\n");

}
