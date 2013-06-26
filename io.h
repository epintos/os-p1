#ifndef _io_h
#define _io_h

/* Prints information about an airplane arrived, the new destiny, and the travel time */
void printLand(Land* land);

/* Writes in the airplane log information about an airplane arrived, the new destiny,
 *  and the travel time */
void writeLand(Land* land);

/* Prints information about an airplane destiny and travel time  */
void printPlan(Plan* plan);

/* Writes in the airplane log information about an airplane destiny and travel time */
void writePlan(Plan* plan);

/* Writes in the map log the medicines needed in every city */
void writeNecessity(Necessity* necesity);

/* Prints the total simulation time in terms of the distance unit */
void printSimulTime(int simulTime);

/* Writes in the map log the total simulation time in terms of the distance unit */
void writeSimulTime(int simulTime);

/* Prints a turn title indicating a new simulation turn  */
void printStartTurn();

/* Writes in the airplane log a turn title indicating a new simulation turn  */
void writeStartTurn();

/*Writes in the airplane log that the airplane left medicines in the starting city*/
void setInitialMessage(int cmpID);

/* Closes the logs files */
void closeCompanyLogs();

/* Writes first turn in the correct companyLog*/
void writeFirstTurn(int id);

#endif
