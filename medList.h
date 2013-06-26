#ifndef _structs_h
#define _structs_h

typedef struct {
	char * medicineName;
	int amountNeeded;
} medicine;

typedef struct {
	medicine * med;
	struct medNode * next;
} medNode;

typedef struct {
	int nodesCount;
	medNode * first;
} medList;

/*
 The list starts with an initial space.
 The user can add elements to the list, and in case it runs out of space,
 more space is allocated.
 */

/* Creates a medicine list */
medList * createList();

/* Adds a medicine to the list */
int addMed(char *name, int amountNeeded, medList * list);

#endif
