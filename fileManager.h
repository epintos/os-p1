#ifndef _fileManager_h
#define _fileManager_h

#include "graph.h"
#include "companyReader.h"

int openAndReadCityFile(char *path, graph * map);
int readAmountOfCities(FILE * file);
int readCityName(FILE *file, int index, graph *map);
int readMedicines(FILE *file, int index, graph *map);
char * readString(FILE * file, char cutCondition);
int readConnections(FILE *file, graph *map);
int readBlankLine(FILE * file);
void printData(graph *map);
void printMedsList(medList * list);
void printDistance(graphNode *node);
int parserError(graph *map, company *cmp, int error,FILE *file);

#endif
