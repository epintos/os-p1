#ifndef _graph_h
#define _graph_h

#include "medList.h"

typedef struct {

	char * cityName;
	medList * medicineList;

} city;


typedef struct {
	int weight;
	city * neighbour;
} arc;

typedef struct {

	city *info;
	arc ** adjList;
	int arcQty;

} graphNode;

typedef struct {

	graphNode ** nodes;
	int nodeCount;

} graph;

/* Initializes the map, and asigns nodeCount representing the city quantity */
int initializeGraph(int nodeCount, graph *map);

/* Asigns the name to a city */
int addNameToCity(int cityIndex, char *name, graph *map);

/* Returns the index in the city list of a city named "cityName", if it doesn't exits,
 * returns -1 */
int existsCity(char * cityName, graph *map);

/* Adds a connection between a city with index "originIndex" and another city with index
 * "destinyIndex", then assigns the weigth to that connection */
int addConnection(graph *map, int originIndex, int destinyIndex, int weight);

/* Returns FALSE if exists a connection between a city with index "originIndex"
 * and another city with index "destinyIndex", if the connection doesn't exists,
 * returns TRUE */
int arcDoesntExist(int originIndex, int destinyIndex, graph *map);

/* Addds medicine with name "medName" to a city with index "cityIndex" */
int addMedToCity(int cityIndex, char *medName, int amount, graph * map);

#endif
