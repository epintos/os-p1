#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <math.h>
#include "medList.h"
#include "defines.h"
#include "fileManager.h"
#include "company.h"
#include "free.h"
#include "companyReader.h"


int openAndReadCompanyFile(char * path, company ** cmp, int index, graph * map) {

	FILE * file = fopen(path, "r");
	char * num = NULL;
	int airplaneQty = 0;
	int counter = 0;
	cmp[index] = NULL;
	char * str = NULL;

	if (file == NULL)
		return parserError(NULL, cmp[index], FILE_ERROR, file);

	num = readString(file, '\n');
	if (!isdigit(*num)) {
		free(num);
		return parserError(NULL, cmp[index], FILE_ERROR, file);
	}
	airplaneQty = atoi(num);
	cmp[index] = malloc(sizeof(company));

	if (cmp[index] == NULL)
		return parserError(NULL, cmp[index], MALLOC_ERROR, file);

	if (initializeCompany(cmp[index], airplaneQty) != EXIT_SUCCESS)
		return parserError(NULL, cmp[index], MALLOC_ERROR, file);

	if (readBlankLine(file) != EXIT_SUCCESS)
		return parserError(NULL, cmp[index], FILE_ERROR, file);

	for (counter = 0; counter < airplaneQty; counter++) {

		medList * list = NULL;

		int read = 0;

		str = readString(file, '\n');

		if (str == NULL)
			return parserError(NULL, cmp[index], FILE_ERROR, file);
		if (existsCity(str, map) == -1)
			return parserError(NULL, cmp[index], FILE_ERROR, file);

		if (addActualCity(cmp[index], str, counter) != EXIT_SUCCESS)
			return parserError(NULL, cmp[index], FILE_ERROR, file);

		list = createList();

		while ((read = fgetc(file)) != '\n' && read != EOF) {
			if (read != '\r') {
				char * medName = NULL;
				char * amount = NULL;

				fseek(file, -1, SEEK_CUR);

				medName = readString(file, ' ');
				amount = readString(file, '\n');

				if (medName == NULL || amount == NULL || !isdigit(*amount)) {
					if (amount != NULL)
						free(amount);
					if (medName != NULL)
						free(medName);
					if (list != NULL)
						freeMedList(list);
					return parserError(NULL, cmp[index], FILE_ERROR, file);
				}

				addMed(medName, atoi(amount), list);
				free(amount);
			}
		}
		addMedListToCompany(cmp[index], list, counter);
	}

	fclose(file);
	free(num);
	return EXIT_SUCCESS;
}

int initializeCompany(company * cmp, int airplaneQty) {

	int i = 0;
	cmp->airplaneQty = airplaneQty;
	cmp->airplaneList = malloc(airplaneQty * sizeof(airplane*));

	if (cmp->airplaneList == NULL)
		return MALLOC_ERROR;

	for (i = 0; i < airplaneQty; i++) {
		cmp->airplaneList[i] = malloc(sizeof(airplane));

		if (cmp->airplaneList[i] == NULL)
			return MALLOC_ERROR;
		cmp->airplaneList[i]->actualCity = NULL;
		cmp->airplaneList[i]->medsList = NULL;
	}

	return EXIT_SUCCESS;
}

int addActualCity(company *cmp, char *path, int index) {

	if (cmp == NULL || path == NULL)
		return INVALID_ARGUMENTS;

	cmp->airplaneList[index]->actualCity = path;

	return EXIT_SUCCESS;
}

void addMedListToCompany(company * cmp, medList * list, int index) {

	if (cmp == NULL || list == NULL)
		return;

	cmp->airplaneList[index]->medsList = list;

}

void printCompany(company * cmp) {
	int i = 0;
	printf("Compania:\n");
	for (i = 0; i < cmp->airplaneQty; ++i) {
		printf("Avion numero %d empieza en %s\n", i + 1,
				cmp->airplaneList[i]->actualCity);
	}
}
