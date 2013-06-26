#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "graph.h"
#include "free.h"
#include "medList.h"
#include "defines.h"
#include "fileManager.h"
#include "companyReader.h"

int openAndReadCityFile(char *path, graph * map) {

	FILE * file = fopen(path, "r");
	int num = 0;
	int character = 0, counter = 0;


	if (map == NULL)
		return MALLOC_ERROR;

	if (file == NULL)
		return parserError(map, NULL, FILE_ERROR, file);

	num = readAmountOfCities(file);

	if (num == FILE_ERROR)
		return parserError(map, NULL, FILE_ERROR, file);

	initializeGraph(num, map);

	for (counter = 0; counter < num; counter++) {
		//read blank line
		if (fgetc(file) != '\n') {
			return parserError(map, NULL, FILE_ERROR, file);
		}

		/*Reads and saves the city name*/
		if (readCityName(file, counter, map) != EXIT_SUCCESS)
			return parserError(map, NULL, FILE_ERROR, file);

		/*Reads and saves all medicines that the city needs*/
		if (readMedicines(file, counter, map) != EXIT_SUCCESS)
			return parserError(map, NULL, FILE_ERROR, file);
	}

	if (fgetc(file) != '\n')
		return parserError(map, NULL, FILE_ERROR, file);

	while ((character = fgetc(file)) != EOF) {
		/*Reads connections between cities and builds what's left
		 from the graph*/
		if (character != '\n') {
			fseek(file, -1, SEEK_CUR);
			if (readConnections(file, map) != EXIT_SUCCESS)
				return parserError(map, NULL, FILE_ERROR, file);
		}

	}

	fclose(file);
	return EXIT_SUCCESS;

}

int readAmountOfCities(FILE * file) {

	int num = 0;
	int character;

	while ((character = fgetc(file)) != '\n') {

		if (!isdigit(character))
			return FILE_ERROR;
		num *= 10;
		num += character - '0';
	}

	return num;
}

int readCityName(FILE *file, int index, graph *map) {

	char * name;

	name = readString(file, '\n');

	if (name == NULL)
		return FILE_ERROR;

	if (addNameToCity(index, name, map) != EXIT_SUCCESS)
		return FILE_ERROR;

	return EXIT_SUCCESS;
}

int readMedicines(FILE *file, int index, graph *map) {

	map->nodes[index]->info->medicineList = createList();
	char actual;

	while ((actual = fgetc(file)) != '\n') {
		fseek(file, -1, SEEK_CUR);
		char * medName = readString(file, ' ');
		char * amount = readString(file, '\n');
		if (medName == NULL || amount == NULL || !isdigit(*amount)) {
			free(medName);
			free(amount);
			return FILE_ERROR;
		}

		addMed(medName, atoi(amount), map->nodes[index]->info->medicineList);
		free(amount);
	}
	fseek(file, -1, SEEK_CUR);
	return EXIT_SUCCESS;

}

int readConnections(FILE *file, graph *map) {

	char * origin = readString(file, ' ');
	char * destiny = NULL;
	char * weight = NULL;
	int dest = 0, index = 0;

	if (origin == NULL || (index = existsCity(origin, map)) == -1) {
		return FILE_ERROR;
	}

	destiny = readString(file, ' ');

	if (destiny == NULL || (dest = existsCity(destiny, map)) == -1) {
		return FILE_ERROR;
	}

	weight = readString(file, '\n');

	if (!isdigit(*weight)) {
		return FILE_ERROR;
	}

	if (addConnection(map, index, dest, atoi(weight)) != EXIT_SUCCESS) {
		return INTERNAL_ERROR;
	}
	free(origin);
	free(destiny);
	free(weight);
	return EXIT_SUCCESS;

}

/*Reads a string until the cutCondition is reached.

 For instance, if the user wants to read a string that's
 terminated with a blank space, then cutCondition must be ' '*/
char * readString(FILE * file, char cutCondition) {

	char * name = NULL;
	char * aux = NULL;
	int i = 0, read;

	while ((read = fgetc(file)) != cutCondition && read != EOF) {
		if (cutCondition == '\n' && read == ' ')
			return NULL;

		if (cutCondition == ' ' && read == '\n')
			return NULL;

		if (read != '\r') {
			if (i % BLOCK == 0) {
				aux = realloc(name, (i + BLOCK) * sizeof(char));
				if (aux == NULL)
					return NULL;
				name = aux;
			}
			name[i++] = read;
		}
	}
	aux = realloc(name, (i + 1) * sizeof(char)); // adds one for the final 0.
	if (aux == NULL)
		return NULL;
	aux[i] = 0;
	name = aux;
	return name;

}

int readBlankLine(FILE * file) {
	int read = 0;

	if ((read = fgetc(file)) == '\n')
		return EXIT_SUCCESS;
	if (read == '\r')
		if (fgetc(file) == '\n')
			return EXIT_SUCCESS;

	return FILE_ERROR;
}

void printData(graph *map) {

	int nodeCount = map->nodeCount;
	int i = 0;

	for (i = 0; i < nodeCount; i++) {
		city * city = map->nodes[i]->info;
		printf("Nombre ciudad %d: %s\n", i, city->cityName);
		printMedsList(city->medicineList);
		//printDistance(map->nodes[i]);
	}
}

void printMedsList(medList * list) {
	int count = list->nodesCount;
	medNode * actual = list->first;

	while (count-- > 0) {
		printf("Necesita %d de %s\n", actual->med->amountNeeded,
				actual->med->medicineName);
		actual = (medNode*) (actual->next);
	}

}

void printDistance(graphNode *node) {
	int arcQty = node->arcQty;
	int i = 0;
	for (i = 0; i < arcQty; ++i) {
		printf("Distancia de %s a %s: %d\n", node->info->cityName,
				node->adjList[i]->neighbour->cityName, node->adjList[i]->weight);
	}
}

int parserError(graph *map, company *cmp, int error, FILE *file) {
	if (cmp != NULL)
		freeCompany(cmp);
	if (map != NULL)
		freeMap(map);
	switch (error) {
	case FILE_ERROR:
		printf("Error en el archivo\n");
		break;
	case MALLOC_ERROR:
		printf("Error al alocar memoria\n");
		break;
	case INVALID_ARGUMENTS:
		printf("Error en los argumentos ingresados\n");
		break;
	}
	if (file != NULL)
		fclose(file);
	return error;
}
