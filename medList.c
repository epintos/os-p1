#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "graph.h"
#include "medList.h"
#include "defines.h"

medList * createList() {

	medList * list = malloc(sizeof(medList));

	if (list == NULL)
		return NULL;

	list->nodesCount = 0;
	list->first = NULL;

	return list;
}


int addMed(char *name, int amountNeeded, medList * list) {

	medNode *node = malloc(sizeof(medNode));
	medNode *aux;

	if (node == NULL)
		return MALLOC_ERROR;

	node->med = malloc(sizeof(medicine));

	if (node->med == NULL)
		return MALLOC_ERROR;

	node->med->medicineName = name;
	node->med->amountNeeded = amountNeeded;

	if (list->first == NULL) {
		aux = NULL;
	} else {
		aux = list->first;
	}

	node->next = aux;
	list->first = node;

	list->nodesCount++;

	return EXIT_SUCCESS;
}
