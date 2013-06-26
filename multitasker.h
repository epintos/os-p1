#ifndef MARSHALLING_H_
#define MARSHALLING_H_

#include "graph.h"
#include "medList.h"
#include <unistd.h>
#include "companyReader.h"
#include "company.h"
#include "ipcs.h"

typedef struct{
	int planeNum;
	char* cityName;
	int travelTime;
	int cmpID;
}Plan;

typedef struct{
	char* cityName;
	medList * necesity;

}Necesity;

typedef struct{
	int planeNum;
	char* cityNow;
	char* cityThen;
	int travelTime;
	int cmpID;
}Land;

typedef struct arcStruct{
	int from;
	int to;
	int weight;

}arcStruct;

typedef struct {
    char * cityName;
    medList * list;
    int aid;
} airlineUpdate;

/* Gets a message from another process, copies it in msg, and returns the first char,
 * which indicates an action option  */
char getUpdate(int id,char **msg);

/* Same as getUpdate, but does not return an option, it returns the message */
char *receiveMsg(int from_id);

/* Makes a map serialization in an string format and sends it to a process with
 * "dest_id" */
int initializeMap(graph *map, int id, int dest_id);

/* Adds "number" to msg, and returns in counter the final size of msg */
int addNumber(char ** msg, int *counter, int number);

/* Deserializes msg into a map in a graph format */
graph * deserializeMap(char *msg);

/* Same as initializeMap, but with a company */
int initializeCmp(company* comp,  int id, int dest_id) ;

/* Deserializes msg into a company format */
company* deserializeCompany(char* msg);

/* Serializes information for the company to send to the map, then sends it */
int sendUpdate(communication * c, int id, int dest_id);

/* Deserializes information sent by the airplane in msg */
airlineUpdate * getAirlineUpdate(char *msg);

int sendCityInfo(airlineUpdate * au, int index, graph * map);

/* Serializes neighbours and city necessities and sends it to the company with au->aid */
int sendNeighboursAndCityInfo(airlineUpdate * au, int index, graph * map);

/* Serializes time and sends it to dest_id */
int sendSmallestTravelTime(int time, int id, int dest_id);

/* Deserializes msg and returns a travel time */
int getTravelTime(char * msg);

/* Deserializes the information in msg, and returns a mapAnswer with a new destiny for
 * an airplane  */
mapAnswer * buildCityInfo(char * msg);

/* Serializes a pid and sends it to dest_id */
int sendPID(int id, int dest_id, int pid);

/* Deserializes msg and returns a pid */
int receivePID(char* msg);

/* Serializes airplane travel information and sends it to dest_id */
int sendPlan(int planeNum, communication* c, int id, int dest_id);

/* Serializes airplane arrive information and sends it to dest_id */
int sendLand(int planeNum, char* cityNow ,communication* c, int id, int dest_id);

int sendNecesities(city* city, int id, int dest_id);

/* Deserializes msg and returns landing information */
Land* getLand(char* msg);

/* Deserializes msg and returns and airplane travel information */
Plan* getPlan(char* msg);

Necesity* getNecesity(char* msg);

/* Serializes the current simluation time in a turn and sends it to dest_id */
int sendSimulTime(int simulTime, int id, int dest_id);

/* Deserializes msg and returns a simulation time */
int getSimulTime(char* msg);

/* Sends to dest_id a msg indicating a new turn started */
int sendStartTurn(int id, int dest_id);

/* Sends to dest_id a msg indicating the company with id equals to id that is ready to start */
int sendFirstTurn(int id, int dest_id);

int getID(char* msg);

#endif
