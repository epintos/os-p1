#ifndef _companyReader_h
#define _companyReader_h

typedef struct {
	char *actualCity;
	medList *medsList;
} airplane;

typedef struct {
	airplane **airplaneList;
	int airplaneQty;
} company;


int initializeCompany(company * cmp, int airplaneQty);
int addActualCity(company *cmp, char *path, int index);
void addMedListToCompany(company * cmp, medList * list, int index);
int openAndReadCompanyFile(char * path, company ** cmp, int index, graph * map);
void printCompany(company * cmp);

#endif
