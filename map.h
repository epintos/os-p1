#ifndef MAP_H_
#define MAP_H_

#include "graph.h"
#include "marshalling.h"

/* Updates the airplane stock and the destiny city necessities, then analizes
 * if all the cities have been satisfied, and if this happens, the map sends a signal
 * to the companies notifying this, if not, the simulation continues */
void processUpdate(airlineUpdate * au, graph* map, int cmpQty, int *pids);


#endif

