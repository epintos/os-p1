#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

/* Creates a semaphore */
int createSem(key_t key, int sem_qty);

/* Sets a value to a sem with sem_id */
int setSem(int sem_id, int sem_num, int value);

/* Sets al the the semaphores */
int setAllSem(int sem_id, int sem_qty, int value);

/* Increases the semaphore */
int upSem(int sem_id, int sem_num);

/* Decreases the semaphore */
int downSem(int sem_id, int sem_num);

/* Destroys the sempahore */
int destroySem(long sem_id);

#endif /* SEMAPHORE_H_ */

