#ifndef COMPANY_H_
#define COMPANY_H_

#include <pthread.h>

typedef struct {
	airplane * plane;
	int tid;
} parameter;

typedef struct {
	char * cityName;
	int travelTime;
	medList * deliveredMeds;
	int modif;
} communication;

typedef struct {
	char * cityName;
	int travelTime;
	struct cityList * next;
} cityList;

typedef struct {
	char * cityName;
	medList * list;
	cityList * first;
	int cityQty;
} mapAnswer;

void * airplaneThread(void *threadid);

/* Decreases the time travel to each airplane in the company with "my_id" and if the
 * aiplane reaches the destiny, the map updates the necessities and then assigns a
 * new destiny to the plane */
int processAllPlanes(int time, int my_id);

/* Calculates the minimum time of the planes in the company with "my_id" and sends it
 * to the multitasker */
void sendTravelTime(int my_id);

/* Choose randomly a destiny city */
int choseCity(mapAnswer * ma, char ** cityName);

#endif
