#ifndef _FREE_H_
#define _FREE_H_

#include "companyReader.h"
/* Frees map represented as a graph*/
void freeMap(graph* map);

/* Frees each graph node*/
void freeGraphNode(graphNode* node);

/* Frees city*/
void freeCity(city *city);

/* Frees a graph arc*/
void freeArc(arc *arc);

/* Frees a medicine list */
void freeMedList(medList* medsList);

/* Frees a medicine list node */
void freeMedNode(medNode* medNode);

/* Frees a medicine from the list */
void freeMedicine(medicine* medicine);

/* Frees a company */
void freeCompany(company * cmp);

/* Frees an airplane */
void freeAirplane(airplane * airplane);


#endif
