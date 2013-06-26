#ifndef _ipcs_h
#define _ipcs_h

#define ERROR -1
#define EXIT_SUCCESS 0

/* Creates or connects a process with id to an IPC */
int createIPC(int id, int ipcs_qty);

/* Sends msg of size to a process with dest_id. Returns the total bytes sent */
int sendMessage(char *msg, int id, int dest_id, int size);

/* Process with id receives msg from another process. Returns the total bytes read */
int getMessage(char **msg, int id);

/* Disconnects, erases if necessary, the ipc of a process with id */
int disconnectFromIPC(int id);

/* Erases the ipc with id */
int destroyIPC(int id);


#endif
