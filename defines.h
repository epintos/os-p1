#ifndef _defines_h
#define _defines_h

/* Defines for type of exits */
#define FILE_ERROR -1
#define MALLOC_ERROR -2
#define EXIT_SUCCESS 0
#define INVALID_ARGUMENTS -3
#define INTERNAL_ERROR -4
#define ERROR -1

/* Defines for mallocs */
#define BLOCK 5

/* Defines for options */
#define AIRLINE 'A'
#define INITIALIZE 'I'
#define MAP_REPLY_NO_MODIF 'M'
#define MULTITASKER 'T'
#define PID 'P'

/* Defines for IO options */
#define NEWTURN 'T'
#define FIRSTTURN 'F'
#define LAND 'L'
#define PLAN 'P'
#define NECESITY 'N'
#define SIMULTIME 'S'
#define INITIAL 'I'

/* Defines for process id */
#define MAIN_ID 0
#define MAP_ID 1
#define MULTITASKER_ID 2
#define IO_ID 3
#define CMP_ID 4
#define ID_QTY 4

/* Defines for booleans */
#define TRUE 1
#define FALSE 0

/* Define for multitasker */
#define UNDEF_TIME -1

/* Define for semaphores*/
#define SEM_QTY ID_QTY*2+1
#define SEM_SINC ID_QTY+1
#define SEM_TURN ID_QTY+2
#define SEM_TIME ID_QTY+3
#define SEM_START ID_QTY+4

#endif
