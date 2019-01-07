//   CECS 326 - Operating System
//   Insturctor: Anthony Giacalone
//
//   FISH.c
//   Lab 3 - Semaphores and Message Passing
//
//   Author: Thanh Le
//   Deadline: November 27, 2018

#include "Header.h"
#include <pthread.h>

// makeup position for the closest pellet, make sure it is further than any pellet that will be generated later
int closestPellet[2] = {-1, col/2};

// the position the Fish at the begining is in the center of the last row in the pool
int currentPos = col/2;

// thread to run currently with the process to detect pellet
pthread_t child_thread;

void fishInterruption(int ignored);
void fishTermination(int ignored);

static void *updateClosestPellet(void *ignored);
bool checkIfCloser(int *location);
bool checkIfEatable(int *location);

int main()
{
    // Setup signal intercepts from SwimMill
    signal(SIGINT, fishInterruption);
    signal(SIGTERM, fishTermination);
    
    // Attach to the shared memory
    attachSharedMemory();
    
    // Create a semaphore
    createSemaphore();
    
    // Initialize the initial the position of the fish which is in the center of the last row
    *pool[row-1][currentPos] = 'F';
    
    // Create child thread to keep looking for the closest pellet
    pthread_create(&child_thread, NULL, updateClosestPellet, (void*)&child_thread);
    
    while(1)
    {
        // Move the fish to the column of the closest pellet
        while(closestPellet[1] != currentPos)
        {
            // Lock the critical region
            sem_op.sem_num = 0;
            sem_op.sem_op = -1;
            sem_op.sem_flg = 0;
            semop(semID, &sem_op, 1);
            
            if(closestPellet[1] > currentPos)
            {
                // Move right
                *pool[row-1][currentPos] = '~';
                currentPos += 1;
                *pool[row-1][currentPos] = 'F';
            }
            else
            {
                //Move left
                *pool[row-1][currentPos] = '~';
                currentPos -= 1;
                *pool[row-1][currentPos] = 'F';
            }
            
            // Unlock the critical region
            sem_op.sem_num = 0;
            sem_op.sem_op = 1;
            sem_op.sem_flg = 0;
            semop(semID, &sem_op, 1);
            
            sleep(1);
        }
        
        //Update the closest pellet after the PELLET was eaten
        closestPellet[0] = -1;
        closestPellet[1] = col/2;
        sleep(1);
    }
    return 0;
}

// Use the thread to look for the closest pellet in the pool
static void *updateClosestPellet()
{
    while(1)
    {
        for(int i = row-2; i >= 0; i--)
        {
            for(int j = col-2; j >= 0; j--)
            {
                if(*pool[i][j] == 'P')
                {
                    int location[2] = {i, j};
                    if(checkIfEatable(location) && checkIfCloser(location))
                    {
                        closestPellet[0] = i;
                        closestPellet[1] = j;
                    }
                }
            }
        }
    }
}

// Check if the new detected pellet is closer than the closestPellet
bool checkIfCloser(int* location)
{
    if (closestPellet[0] == -1)        return true;
    else                               return (abs(currentPos - location[1]) < abs(currentPos - closestPellet[1]));
}

// Check if the new detected pellet is eatable
bool checkIfEatable(int* location)
{
    return (abs(currentPos - location[1]) <= (row - location[0] - 1));
}

// Kill the FISH process by interrupt signal ^C
void fishInterruption(int ignored)
{
    pthread_cancel(child_thread);
    fprintf(stderr, "\n FISH %d was killed by interrupt signal ^C", getpid());
    exit(0);
}

// Kill the FISH process when the time limit reached
void fishTermination(int ignored)
{
    pthread_cancel(child_thread);
    fprintf(stderr, "\n FISH %d was killed because time limit reached", getpid());
    exit(0);
}
