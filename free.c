#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "medList.h"
#include "defines.h"
#include "fileManager.h"
#include "graph.h"
#include "companyReader.h"
#include "free.h"


static void freeMedNodeRec(medNode* medNode);

void freeMap(graph* map) {
	if (map == NULL)
		return;
	int i = 0;
	for (i = 0; i < map->nodeCount; ++i) {
		if (map->nodes[i] != NULL)
			freeGraphNode(map->nodes[i]);
	}
	if (map->nodes != NULL)
		free(map->nodes);
	free(map);
}

void freeGraphNode(graphNode* node) {
	int i = node->arcQty;
	if (node->info != NULL)
		freeCity(node->info);
	while (i-- > 0) {
		if (node->adjList[i] != NULL)
			free(node->adjList[i]);
	}
	if (node->adjList != NULL)
		free(node->adjList);
	free(node);
}

void freeCity(city *city) {
	if (city->cityName != NULL)
		free(city->cityName);
	if (city->medicineList != NULL)
		freeMedList(city->medicineList);
	free(city);
}

void freeMedList(medList* medList) {
	if (medList->first != NULL)
		freeMedNodeRec(medList->first);
	if (medList != NULL)
		free(medList);
}

static void freeMedNodeRec(medNode* medNode) {
	if (medNode == NULL) {
		return;
	}
	if (medNode->next == NULL) {
		freeMedNode(medNode);
		return;
	}
	freeMedNodeRec(medNode->next);
	freeMedNode(medNode);
}
void freeMedNode(medNode* medNode) {
	if (medNode != NULL) {
		freeMedicine(medNode->med);
		free(medNode);
	}
}
void freeMedicine(medicine* medicine) {
	if (medicine != NULL) {
		free(medicine->medicineName);
		free(medicine);
	}
}

void freeCompany(company * cmp) {
	if (cmp == NULL)
		return;
	int i = 0;
	for (i = 0; i < cmp->airplaneQty; ++i) {
		if (cmp->airplaneList[i] != NULL)
			freeAirplane(cmp->airplaneList[i]);
	}
	if (cmp->airplaneList != NULL)
		free(cmp->airplaneList);
	free(cmp);
}

void freeAirplane(airplane * airplane) {
	if (airplane->actualCity != NULL)
		free(airplane->actualCity);
	if (airplane->medsList != NULL)
		freeMedList(airplane->medsList);
	if (airplane != NULL)
		free(airplane);
}
