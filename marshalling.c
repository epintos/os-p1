#include "ipcs.h"
#include "graph.h"
#include "medList.h"
#include "marshalling.h"
#include "companyReader.h"
#include "defines.h"
#include "medList.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <unistd.h>

static int serializeGraphNode(char ** msg, int * counter, graphNode * node,
		graph * map);
static int serializeArcs(char ** msg, int *counter, arc * arc, graph * map);
static int serializeCity(char ** msg, int * counter, city* city);
static int addMedList(char **msg, int *counter, medList * list);
static int addMedListRec(char **msg, int *counter, medNode *node);
static int addString(char **msg, int *counter, char * string);
static int addArcsToMap(graph* map, arcStruct** arcs, int arcsCant);
static int
getArcs(char* msg, int* index, arcStruct*** arcs, int* pos, int from);
static arcStruct* createArc(int from, int to, int weight);
static int deserializeMedicines(int i, char* msg, int* index, graph* map);
static char* readStringDes(char* msg, int* index);
static int readNumber(char *msg, int *index);
static int serializeAirplane(char ** msg, int * counter, airplane * airplane);
static void getAirplanes(char* msg, int * index, company* cmp);
static airplane* getAirplane(char* msg, int * index);
static medList* getMeds(char* msg, int *index);
static int sendNecessities(city* city, int id, int dest_id);

char getUpdate(int id, char **msg) {
	if (getMessage(msg, id) == ERROR) {
		return ERROR;
	}
	return (*msg)[0];
}

char* receiveMsg(int from_id) {
	char *msg = NULL;
	getMessage(&msg, from_id);
	return msg;
}

int initializeMap(graph *map, int id, int dest_id) {

	char * msg = malloc(sizeof(char) * BLOCK);
	int counter = 0;
	int i = 0;

	if (msg == NULL
	)
		return ERROR;

	msg[counter++] = INITIALIZE;
	msg[counter++] = ' ';
	addNumber(&msg, &counter, map->nodeCount);

	for (i = 0; i < map->nodeCount; i++) {
		serializeGraphNode(&msg, &counter, map->nodes[i], map);
	}

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, id, dest_id, counter + 1);
	free(msg);

	return EXIT_SUCCESS;
}

static int serializeGraphNode(char ** msg, int * counter, graphNode * node,
		graph * map) {

	int i = 0;

	serializeCity(msg, counter, node->info);

	addNumber(msg, counter, node->arcQty);

	for (; i < node->arcQty; i++) {
		serializeArcs(msg, counter, node->adjList[i], map);
	}

	return EXIT_SUCCESS;
}

static int serializeArcs(char ** msg, int *counter, arc * arc, graph * map) {

	addNumber(msg, counter, arc->weight);
	addNumber(msg, counter, existsCity(arc->neighbour->cityName, map));

	return EXIT_SUCCESS;
}

static int serializeCity(char ** msg, int * counter, city* city) {

	addString(msg, counter, city->cityName);
	addMedList(msg, counter, city->medicineList);

	return EXIT_SUCCESS;
}

static int addMedList(char **msg, int *counter, medList * list) {

	if (list == NULL) {
		addNumber(msg, counter, -1);
	} else {
		addNumber(msg, counter, list->nodesCount);

		addMedListRec(msg, counter, list->first);

	}

	return EXIT_SUCCESS;
}

static int addMedListRec(char **msg, int *counter, medNode *node) {
	if (node == NULL) {
		return EXIT_SUCCESS;
	}

	addString(msg, counter, node->med->medicineName);
	addNumber(msg, counter, node->med->amountNeeded);

	addMedListRec(msg, counter, (medNode *) node->next);

	return EXIT_SUCCESS;
}

static int addString(char **msg, int *counter, char * string) {

	int i = 0;

	while (string[i] != '\0') {

		if ((*counter) % BLOCK == 0) {
			char * aux = realloc(*msg, ((*counter) + BLOCK) * sizeof(char));
			if (aux == NULL
			)
				return ERROR;
			*msg = aux;
		}

		(*msg)[(*counter)++] = string[i++];
	}

	if ((*counter) % BLOCK == 0) {
		char * aux = realloc(*msg, ((*counter) + BLOCK) * sizeof(char));
		if (aux == NULL
		)
			return ERROR;
		*msg = aux;
	}

	(*msg)[(*counter)++] = ' ';

	return EXIT_SUCCESS;
}

int addNumber(char ** msg, int *counter, int number) {

	char * num = NULL;
	int i = 0;

	if (number < 0) {
		if ((*counter) % BLOCK == 0) {
			char * aux = realloc(*msg, ((*counter) + BLOCK) * sizeof(char));
			if (aux == NULL
			)
				return ERROR;
			*msg = aux;
		}
		(*msg)[(*counter)++] = '-';
		number *= -1;
	}

	if (number == 0) {

		if ((*counter) % BLOCK == 0) {
			char * aux = realloc(*msg, ((*counter) + BLOCK) * sizeof(char));
			if (aux == NULL
			)
				return ERROR;
			*msg = aux;
		}

		(*msg)[(*counter)++] = '0';
	}

	while (number != 0) {

		if (i % BLOCK == 0) {
			char * aux = realloc(num, (i + BLOCK) * sizeof(char));
			if (aux == NULL
			)
				return ERROR;
			num = aux;
		}
		num[i++] = number % 10 + '0';
		number /= 10;
	}

	i--;
	for (; i >= 0; i--) {

		if ((*counter) % BLOCK == 0) {
			char * aux = realloc(*msg, ((*counter) + BLOCK) * sizeof(char));
			if (aux == NULL
			)
				return ERROR;
			*msg = aux;
		}

		(*msg)[(*counter)++] = num[i];
	}
	if ((*counter) % BLOCK == 0) {
		char * aux = realloc(*msg, ((*counter) + BLOCK) * sizeof(char));
		if (aux == NULL
		)
			return ERROR;
		*msg = aux;
	}
	(*msg)[(*counter)++] = ' ';
	free(num);

	return EXIT_SUCCESS;
}

graph * deserializeMap(char *msg) {
	graph * map = malloc(sizeof(graph));
	int index = 2, i = 0;
	int cityQty = readNumber(msg, &index);
	int pos = 0;
	arcStruct** arcs = NULL;

	initializeGraph(cityQty, map); //Se crea el mapa con capacidad para las ciudades pero vacio.

	for (i = 0; i < cityQty; i++) {
		char* cityName = readStringDes(msg, &index); //Se levanta el nombre de la ciudad
		if (cityName == NULL
		)
			return NULL;
		addNameToCity(i, cityName, map); //se agrega la ciudad al mapa.

		deserializeMedicines(i, msg, &index, map); // se agregan las medicinas que necesita esa ciudad.

		getArcs(msg, &index, &arcs, &pos, i); // se agregan los caminos entre las ciudades.

	}
	if (msg[index] != '\0')
		return NULL;

	addArcsToMap(map, arcs, pos);

	return map;

}

static int addArcsToMap(graph* map, arcStruct** arcs, int arcsCant) {
	int i = 0;

	for (; i < arcsCant; i++) {
		if (addConnection(map, arcs[i]->from, arcs[i]->to, arcs[i]->weight)
				!= EXIT_SUCCESS) {
			return ERROR;
		}
	}
	return EXIT_SUCCESS;
}

static int getArcs(char* msg, int* index, arcStruct*** arcs, int* pos, int from) {

	int arcQty = readNumber(msg, index);
	int to = 0;
	int weight = 0;
	int i;
	for (i = 0; i < arcQty; i++) {
		if (*pos % BLOCK == 0) {
			arcStruct** aux = realloc(*arcs,
					(*pos + BLOCK) * sizeof(arcStruct*));
			if (aux == NULL
			)
				return MALLOC_ERROR;
			*arcs = aux;
		}
		weight = readNumber(msg, index);
		to = readNumber(msg, index);
		(*arcs)[*pos] = createArc(from, to, weight);

		(*pos)++;
	}
	return EXIT_SUCCESS;
}

static arcStruct* createArc(int from, int to, int weight) {
	arcStruct* resp = malloc(sizeof(arcStruct));
	resp->from = from;
	resp->to = to;
	resp->weight = weight;
	return resp;
}

static int deserializeMedicines(int i, char* msg, int* index, graph* map) {

	int medsNumber = readNumber(msg, index);
	int it = 0;

	map->nodes[i]->info->medicineList = createList();

	for (it = 0; it < medsNumber; it++) {
		char* medName = readStringDes(msg, index);
		int medsQty = readNumber(msg, index);
		addMedToCity(i, medName, medsQty, map);
	}
	return EXIT_SUCCESS;
}

static char* readStringDes(char* msg, int* index) {

	char* name = NULL;
	int i = 0;
	while (msg[*index] != ' ') {
		if (i % BLOCK == 0) {
			char* aux = realloc(name, (i + BLOCK) * sizeof(char));
			if (aux == NULL
			)
				return NULL;
			name = aux;
		}
		name[i++] = msg[(*index)++];
	}

	if (i % BLOCK == 0) {
		char* aux = realloc(name, (i + BLOCK) * sizeof(char));
		if (aux == NULL
		)
			return NULL;
		name = aux;
	}
	name[i] = '\0';
	(*index)++;

	return name;
}

static int readNumber(char *msg, int *index) {

	int num = 0;
	int minusFlag = 0;
	while (msg[*index] != ' ') {
		if (msg[*index] == '-') {
			minusFlag = 1;
			(*index)++;
		} else {
			num *= 10;
			num += msg[(*index)++] - '0';
		}
	}
	(*index)++;

	if (minusFlag)
		num *= -1;
	return num;
}

int initializeCmp(company* cmp, int id, int dest_id) {

	char * msg = NULL;
	int counter = 0;
	int i = 0;

	addNumber(&msg, &counter, cmp->airplaneQty);

	for (i = 0; i < cmp->airplaneQty; i++) {
		serializeAirplane(&msg, &counter, cmp->airplaneList[i]);
	}

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;
	msg[counter] = '\0';

	sendMessage(msg, id, dest_id, counter + 1);
	free(msg);

	return EXIT_SUCCESS;

}

static int serializeAirplane(char ** msg, int * counter, airplane * airplane) {

	addString(msg, counter, airplane->actualCity);
	addMedList(msg, counter, airplane->medsList);

	return EXIT_SUCCESS;
}

company* deserializeCompany(char* msg) {

	int index = 0;
	company* cmp = malloc(sizeof(company));

	cmp->airplaneQty = readNumber(msg, &index);

	cmp->airplaneList = malloc(cmp->airplaneQty * sizeof(airplane*));
	if (cmp->airplaneList == NULL
	)
		return NULL;

	getAirplanes(msg, &index, cmp);
	if (msg[index] != '\0')
		return NULL;

	return cmp;
}

static void getAirplanes(char* msg, int * index, company* cmp) {

	int i = 0;
	for (i = 0; i < cmp->airplaneQty; i++) {
		cmp->airplaneList[i] = getAirplane(msg, index);
	}

}

static airplane* getAirplane(char* msg, int * index) {

	airplane* plane = malloc(sizeof(airplane));

	plane->actualCity = readStringDes(msg, index);
	plane->medsList = getMeds(msg, index);

	return plane;

}

static medList* getMeds(char* msg, int *index) {

	medList* list = malloc(sizeof(medList));
	list->first = NULL;
	list->nodesCount = 0;
	int medsQty = readNumber(msg, index);

	if (medsQty <= 0)
		return NULL;
	int i;
	for (i = 0; i < medsQty; i++) {
		char* medName = readStringDes(msg, index);
		int medNeeded = readNumber(msg, index);
		addMed(medName, medNeeded, list);
	}
	return list;
}

int sendUpdate(communication * c, int id, int dest_id) {

	int index = 0, auxIndex = 0;
	char * msg = malloc(sizeof(char) * BLOCK);

	if (msg == NULL
	)
		return MALLOC_ERROR;

	msg[index++] = AIRLINE;
	msg[index++] = ' ';
	while (c->cityName[auxIndex] != '\0') {

		if (index % BLOCK == 0) {
			char* aux = realloc(msg, (index + BLOCK) * sizeof(char));
			if (aux == NULL
			)
				return ERROR;
			msg = aux;
		}
		msg[index++] = c->cityName[auxIndex++];

	}
	if (index % BLOCK == 0) {
		char* aux = realloc(msg, (index + BLOCK) * sizeof(char));
		if (aux == NULL
		)
			return ERROR;
		msg = aux;
	}
	msg[index++] = ' ';

	addMedList(&msg, &index, c->deliveredMeds);
	addNumber(&msg, &index, id);

	char *aux = realloc(msg, (index + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;
	msg[index] = '\0';

	sendMessage(msg, id, dest_id, index + 1);

	return EXIT_SUCCESS;
}

/**********DESERIALIZE INFO SENT FROM AIRLINE**********/

airlineUpdate * getAirlineUpdate(char *msg) {

	int index = 2;
	airlineUpdate * au = malloc(sizeof(airlineUpdate));

	au->cityName = readStringDes(msg, &index);
	au->list = getMeds(msg, &index);
	au->aid = readNumber(msg, &index);

	return au;
}

/*************SEND CITY INFO TO AIRLINE***************/

int sendCityInfo(airlineUpdate * au, int index, graph * map) {

	char * msg = malloc(sizeof(char) * BLOCK);
	int counter = 0;
	graphNode * city = map->nodes[index];

	if (msg == NULL || map == NULL
	)
		return ERROR;

	msg[counter++] = MAP_REPLY_NO_MODIF;
	msg[counter++] = ' ';

	addString(&msg, &counter, city->info->cityName);
	addMedList(&msg, &counter, city->info->medicineList);

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';

	sendMessage(msg, MAP_ID, au->aid, counter + 1);

	return EXIT_SUCCESS;
}

int sendNeighboursAndCityInfo(airlineUpdate * au, int index, graph * map) {

	char * msg = malloc(sizeof(char) * BLOCK);
	int counter = 0;
	int arcQty = 0, it = 0;
	graphNode * city = map->nodes[index];

	if (msg == NULL || map == NULL
	)
		return ERROR;

	msg[counter++] = MAP_REPLY_NO_MODIF;
	msg[counter++] = ' ';

	/*msg will now contain the city in which the plane is located
	 ,the medicines it gave to it and the neighbour cities*/
	addString(&msg, &counter, city->info->cityName);
	addMedList(&msg, &counter, au->list);

	arcQty = city->arcQty;
	addNumber(&msg, &counter, arcQty);

	for (it = 0; it < arcQty; it++) {
		addString(&msg, &counter, city->adjList[it]->neighbour->cityName);
		addNumber(&msg, &counter, city->adjList[it]->weight);
	}

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, MAP_ID, au->aid, counter + 1);

	return EXIT_SUCCESS;
}

int sendSmallestTravelTime(int time, int id, int dest_id) {
	char * msg = malloc(sizeof(char) * BLOCK);
	int index = 0;
	msg[index++] = MULTITASKER;
	msg[index++] = ' ';
	addNumber(&msg, &index, time);

	msg[index] = '\0';
	sendMessage(msg, id, dest_id, index + 1);

	return EXIT_SUCCESS;
}

int getTravelTime(char * msg) {
	int index = 2;

	return readNumber(msg, &index);
}

mapAnswer * buildCityInfo(char * msg) {
	mapAnswer * ma = malloc(sizeof(mapAnswer));
	int index = 2, it = 0;
	char * name = readStringDes(msg, &index);
	medList * list = getMeds(msg, &index);
	int neighQty = readNumber(msg, &index);
	cityList * actual = NULL;
	ma->cityName = name;
	ma->list = list;
	ma->cityQty = neighQty;
	ma->first = malloc(sizeof(cityList));
	actual = ma->first;
	for (it = 0; it < neighQty; it++) {
		actual->cityName = readStringDes(msg, &index);
		actual->travelTime = readNumber(msg, &index);
		if (it != neighQty - 1) {
			actual->next = malloc(sizeof(cityList));
			actual = actual->next;
		} else
			actual->next = NULL;
	}
	return ma;
}

int sendPID(int id, int dest_id, int pid) {
	char* msg = malloc(sizeof(char) * 3);
	int counter = 2;
	msg[0] = PID;
	msg[1] = ' ';

	addNumber(&msg, &counter, pid);

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, id, dest_id, counter + 1);
	return EXIT_SUCCESS;
}

int receivePID(char* msg) {
	int counter = 2;
	int pid = readNumber(msg, &counter);

	return pid;
}

int sendPlan(int planeNum, communication* c, int id, int dest_id) {
	char* msg = malloc(sizeof(char) * 2);
	int counter = 2;
	msg[0] = PLAN;
	msg[1] = ' ';
	addNumber(&msg, &counter, planeNum);
	addString(&msg, &counter, c->cityName);
	addNumber(&msg, &counter, c->travelTime);
	addNumber(&msg, &counter, id);

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, id, dest_id, counter + 1);

	return EXIT_SUCCESS;

}

int sendLand(int planeNum, char* cityNow, communication* c, int id, int dest_id) {
	char* msg = malloc(sizeof(char) * 2);
	int counter = 2;
	msg[0] = LAND;
	msg[1] = ' ';
	addNumber(&msg, &counter, planeNum);
	addString(&msg, &counter, cityNow);
	addString(&msg, &counter, c->cityName);
	addNumber(&msg, &counter, c->travelTime);
	addNumber(&msg, &counter, id);

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, id, dest_id, counter + 1);

	return EXIT_SUCCESS;

}

static int sendNecessities(city* city, int id, int dest_id) {
	char*msg = malloc(sizeof(char) * 2);
	int counter = 2;
	msg[0] = NECESITY;
	msg[1] = ' ';
	serializeCity(&msg, &counter, city);

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, id, dest_id, counter + 1);
	return EXIT_SUCCESS;

}

Land* getLand(char* msg) {

	Land* resp = malloc(sizeof(Land));
	int counter = 2;
	resp->planeNum = readNumber(msg, &counter);
	resp->cityNow = readStringDes(msg, &counter);
	resp->cityThen = readStringDes(msg, &counter);
	resp->travelTime = readNumber(msg, &counter);
	resp->cmpID = readNumber(msg, &counter);

	return resp;

}

Plan* getPlan(char* msg) {

	Plan* resp = malloc(sizeof(Plan));
	int counter = 2;
	resp->planeNum = readNumber(msg, &counter);
	resp->cityName = readStringDes(msg, &counter);
	resp->travelTime = readNumber(msg, &counter);
	resp->cmpID = readNumber(msg, &counter);

	return resp;
}

Necessity* getNecessity(char* msg) {
	int counter = 2;
	Necessity* resp = malloc(sizeof(Necessity));
	resp->cityName = readStringDes(msg, &counter);
	resp->necesity = getMeds(msg, &counter);

	return resp;
}

int sendSimulTime(int simulTime, int id, int dest_id) {
	char* msg = malloc(sizeof(char) * 2);
	int counter = 2;
	msg[0] = SIMULTIME;
	msg[1] = ' ';
	addNumber(&msg, &counter, simulTime);

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, id, dest_id, counter + 1);

	return EXIT_SUCCESS;

}

int getSimulTime(char* msg) {

	int counter = 2;
	long long int simulTime = readNumber(msg, &counter);
	return simulTime;
}

int sendStartTurn(int id, int dest_id) {
	char* msg = malloc(sizeof(char) * 3);
	msg[0] = NEWTURN;
	msg[1] = ' ';
	msg[2] = '\0';

	sendMessage(msg, id, dest_id, 3);

	return EXIT_SUCCESS;

}

int sendFirstTurn(int id, int dest_id) {
	int counter = 2;
	char* msg = malloc(sizeof(char) * 3);
	msg[0] = FIRSTTURN;
	msg[1] = ' ';
	addNumber(&msg, &counter, id);

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, id, dest_id, counter + 1);

	return EXIT_SUCCESS;
}

int getID(char* msg) {
	int counter = 2;
	int ID = readNumber(msg, &counter);
	return ID;
}

int sendMapNecessities(graph* map) {
	int i = 0;

	for (i=0; i < map->nodeCount; i++) {
		sendNecessities(map->nodes[i]->info, MAP_ID, IO_ID);
	}
	return EXIT_SUCCESS;
}

int sendInitialMed(int id, int dest_id) {
	int counter = 2;
	char* msg = malloc(sizeof(char) * 3);
	msg[0] = INITIAL;
	msg[1] = ' ';
	addNumber(&msg, &counter, id);

	char *aux = realloc(msg, (counter + 1) * sizeof(char));
	if (aux == NULL
	)
		return ERROR;
	msg = aux;

	msg[counter] = '\0';
	sendMessage(msg, id, dest_id, counter + 1);

	return EXIT_SUCCESS;
}
