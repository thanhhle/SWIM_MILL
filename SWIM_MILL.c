//   CECS 326 - Operating System
//   Insturctor: Anthony Giacalone
//
//   SWIM_MILL.c
//   Lab 3 - Semaphores and Message Passing
//
//   Author: Thanh Le
//   Deadline: November 27, 2018

#include "Header.h"
#include <time.h>
#include <pthread.h>

int numPro = 0;                         // number of running processes
const int maxPro = 20;                  // maximum number of running process is set to 20

void swimMillInterruption(int ignored);
void swimMillTermination(int ignored);
void *printPool (void *ignored);

int main(int argc, char *argv[])
{
    // Setup signal intercepts from SWIM MILL
    signal(SIGINT, swimMillInterruption);
    signal(SIGALRM, swimMillTermination);
    alarm(30);
    
    // Create shared memory segment. Exit on error.
    attachSharedMemory();
    
    // Generate the pool
    for (int i = 0; i < row; i++)
    {
        for (int j = 0; j < col; j++)
        {
            *pool[i][j] = '~';
        }
    }
    
    // Create the Fish process
    pid_t fish = fork();
    if(fish == 0)
    {
        // FISH process
        numPro++;                                         // Increase the number of total running processes
        execv("FISH", argv);                              // Execute the FISH process
    }
    else
    {
        // Create a thread to print the pool for every 1 second
        pthread_t printPool_thread;
        pthread_create(&printPool_thread, NULL, printPool, NULL);
        
        while(1)
        {
            if(numPro <= maxPro)                          // Check if the total running process exceed the limit
            {
                pid_t pellet = fork();                    // Create PELLET process
                if(pellet == 0)
                {
                    // PELLET process
                    numPro++;                                 // Increase the number of total running processes
                    execv("PELLET", argv);                    // Execute the PELLET process
                }
                
                // Sleep for random time interval[1, 5) before creating new PELLET process
                srand((int)time(NULL));
                int sec = ((rand() % 4) + 1);
                sleep(sec);
            }
            else
            {
                wait(NULL);                                    // wait until a FISH exits
            }
        }
    }
    return 0;
}

// Print the Swim mill pool
void *printPool (void *ignored)
{
    while(1)
    {
        for(int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                printf("%d\t\t", *pool[i][j]);
            }
            printf("\n");
        }
        printf("---------------------------------------------------------------------------\n");
        sleep(1);
    }
    return NULL;
}

// Kill the SWIM_MILL process by interrupt signal ^C
void swimMillInterruption(int ignored)
{
    shmdt(pool);
    shmctl(sMemID, IPC_RMID, 0);
    
    printf("SWIM_MILL %d was kill by interrupt signal ^C\n", getpid());
    kill(0, SIGKILL);
    exit(0);
}

// Kill the SWIM_MILL process when the time limit reached
void swimMillTermination(int ignored)
{
    shmdt(pool);
    shmctl(sMemID, IPC_RMID, 0);
    
    printf("SWIM_MILL %d was kill because time limit reached\n", getpid());
    kill(0, SIGKILL);
    exit(0);
}
