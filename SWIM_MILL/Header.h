//   CECS 326 - Operating System
//   Insturctor: Anthony Giacalone
//
//   Header.h
//   Lab 3 - Semaphores and Message Passing
//
//   Author: Thanh Le
//   Deadline: November 27, 2018

#ifndef Header_h
#define Header_h

#include <stdlib.h>         // Standard library
#include <stdio.h>          // Standard I/O

#include <unistd.h>         // For fork
#include <signal.h>         // For signal
#include <stdbool.h>        // For bool
#include <sys/types.h>      // For pid_t
#include <sys/ipc.h>        // For interprocess communication
#include <sys/shm.h>        // For shared memory
#include <sys/wait.h>       // For wait
#include <sys/sem.h>        // For semaphore

int sMemID;
const key_t mkeyVal = 1998;
const int row = 10;
const int col = 10;
char (*pool)[row][col];

const int skeyVal = 1234;
struct sembuf sem_op;
int semID;


void attachSharedMemory()
{
    // Create shared memory segment. Exit on error.
    sMemID = shmget(mkeyVal, sizeof(pool), IPC_CREAT|0666);
    if(sMemID < 0)
    {
        perror("Could not create memory segment! - shmget error\n");
        exit(1);
    }
    
    // Attach the shared memory segment. Exit on error.
    pool = (char(*)[10][10])shmat(sMemID, NULL, 0);
    if(pool == (char(*)[row][col])-1) {
        perror("Could not attach shared memory segment! - shmat error\n");
        exit(1);
    }
}

void createSemaphore()
{
    /// Create a semaphore
    semID = semget(skeyVal, 1, IPC_CREAT|0666);
    if (semID == -1)
    {
        perror("Could not create semaphore");
        exit(1);
    }
    
    // Set the semaphore to 0
    if(semctl(semID, 0, SETVAL, 1) == -1)
    {
        perror("Could not initialize semaphore");
    }
}
#endif /* Header_h */
