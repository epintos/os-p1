#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "medList.h"
#include "graph.h"
#include "defines.h"


int initializeGraph(int nodeCount, graph *map) {

	int i = 0;
	int size = nodeCount * sizeof(graphNode*);
	map->nodes = malloc(size);
	map->nodeCount = nodeCount;

	if (map->nodes == NULL)
		return MALLOC_ERROR;

	for (i = 0; i < nodeCount; i++) {
		map->nodes[i] = malloc(sizeof(graphNode));

		if (map->nodes[i] == NULL)
			return MALLOC_ERROR;

		map->nodes[i]->info = malloc(sizeof(city));
		map->nodes[i]->adjList = malloc(sizeof(int));
		map->nodes[i]->arcQty = 0;

		if (map->nodes[i]->info == NULL)
			return MALLOC_ERROR;
	}

	return EXIT_SUCCESS;
}

int addNameToCity(int cityIndex, char *name, graph *map) {

	if (name == NULL || map == NULL || cityIndex < 0)
		return INVALID_ARGUMENTS;

	map->nodes[cityIndex]->info->cityName = name;

	return EXIT_SUCCESS;
}

int addMedToCity(int cityIndex, char *medName, int amount, graph * map) {

	graphNode * node = map->nodes[cityIndex];

	if (addMed(medName, amount, node->info->medicineList) != EXIT_SUCCESS)
		return INTERNAL_ERROR;

	return EXIT_SUCCESS;

}

int existsCity(char * cityName, graph *map) {

	int cityQty = map->nodeCount;
	int i = 0;
	int found = -1;

	for (i = 0; i < cityQty && found == -1; i++) {
		if (strcmp(cityName, map->nodes[i]->info->cityName) == 0)
			found = i;
	}

	return found;
}

int addConnection(graph *map, int originIndex, int destinyIndex, int weight) {

	graphNode * origin = map->nodes[originIndex];
	graphNode * destiny = map->nodes[destinyIndex];
	arc * newArc = NULL;
	arc * destinyArc = NULL;

	if (!arcDoesntExist(originIndex, destinyIndex, map))
		return EXIT_SUCCESS;

	if (origin->arcQty % BLOCK == 0) {

		arc ** aux = realloc(origin->adjList, (origin->arcQty + BLOCK)
				* sizeof(int));

		if (aux == NULL) {
			return MALLOC_ERROR;
		}

		origin->adjList = aux;
	}

	if (destiny->arcQty % BLOCK == 0) {

		arc ** aux = realloc(destiny->adjList, (destiny->arcQty + BLOCK)
				* sizeof(int));

		if (aux == NULL) {
			return MALLOC_ERROR;
		}

		destiny->adjList = aux;
	}

	newArc = malloc(sizeof(arc));
	destinyArc = malloc(sizeof(arc));

	if (newArc == NULL || destinyArc == NULL)
		return MALLOC_ERROR;

	newArc->neighbour = map->nodes[destinyIndex]->info;
	newArc->weight = weight;
	destinyArc->neighbour = map->nodes[originIndex]->info;
	destinyArc->weight = weight;

	origin->adjList[origin->arcQty++] = newArc;
	destiny->adjList[destiny->arcQty++] = destinyArc;

	return EXIT_SUCCESS;
}

int arcDoesntExist(int originIndex, int destinyIndex, graph *map) {

	graphNode * origin = map->nodes[originIndex];
	graphNode * destiny = map->nodes[destinyIndex];
	int nodeQty = origin->arcQty;

	while (nodeQty > 0) {
		arc * actual = origin->adjList[nodeQty - 1];

		if (strcmp(actual->neighbour->cityName, destiny->info->cityName) == 0)
			return FALSE;

		nodeQty--;
	}

	return TRUE;
}
