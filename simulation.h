#ifndef SIMULATION_H_
#define SIMULATION_H_

/* Parses the map and the companies */
int parseFiles(int argc, char *argv[], graph * map, company ** cmp);

typedef struct {
	int* pids;
	int cantPids;
} pids;

#endif

