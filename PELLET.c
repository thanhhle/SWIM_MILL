//   CECS 326 - Operating System
//   Insturctor: Anthony Giacalone
//
//   PELLET.c
//   Lab 3 - Semaphores and Message Passing
//
//   Author: Thanh Le
//   Deadline: November 27, 2018

#include "Header.h"
#include <time.h>

void pelletInterruption(int ignored);
void pelletTermination(int ignored);

int main()
{
    // Setup signal intercepts from SwimMill
    signal(SIGINT, pelletInterruption);
    signal(SIGTERM, pelletTermination);
    
    // Attach process to shared memory
    attachSharedMemory();
    
    // Create a semaphore
    createSemaphore();
    
    // Generate random location for the new pellet to drop where no other pellet currently locates
    srand((int)time(NULL));
    int x, y;
    do
    {
        x = rand()%(row-2);
        y = rand()%(col-2);
    }
    while (*pool[x][y] == 'P');
    
    // Print the new generated pellet to the pool at [x][y]
    *pool[x][y] = 'P';
    printf("PELLET %d was DROPPED at [%d,%d]\n", getpid(), x, y);
    sleep(1);
    bool wasEaten = false;
    
    // Execute the pellet
    while(x < (row - 1))
    {
        // Lock the critical region
        sem_op.sem_num = 0;
        sem_op.sem_op = -1;
        sem_op.sem_flg = 0;
        semop(semID, &sem_op, 1);
        
        // Move the pellet down 1 unit
        x++;
        
        // Update the symbol of water for previous position
        if(*pool[x-1][y] != 'F')
        {
            *pool[x-1][y] = '~';
        }
        
        // Check if pellet was eaten
        if(*pool[x][y] == 'F')
        {
            *pool[x][y] |= 'P';         // Allow the pellet and the fish coincide
            wasEaten = true;
            sleep(1);
            *pool[x][y] = 'F';
            break;
        }
        else if(*pool[x][y] == 86)      // 86 is when the fish is eating another pellet
        {
            wasEaten = true;
            sleep(1);
            *pool[x][y] = 'F';
            break;
        }
        
        *pool[x][y] = 'P';              // If the pellet was not eaten, print the pellet in the new location
        
        // Unlock the critical region
        sem_op.sem_num = 0;
        sem_op.sem_op = 1;
        sem_op.sem_flg = 0;
        semop(semID, &sem_op, 1);
        
        sleep(1);
    }
    
    // Update the symbol at last row and print out if the pellet was eaten or missed
    if(wasEaten)
    {
        printf("PELLET %d was EATEN at column %d!\n", getpid(), y);
    }
    else
    {
        *pool[x][y] = '~';        // Update the symbol to '~' (water) if the pellet was missed
        printf("PELLET %d was MISSED at column %d!\n", getpid(), y);
    }
    
    printf("---------------------------------------------------------------------------\n");
    
    // Write the result into a file after each child finished
    FILE *fp;
    fp = fopen("/Users/admin/Documents/CSULB/Classes/Fall 2018/CECS 326/Labs/Lab 3 - Semaphores and Message Passing/output.txt", "a");
    // Print message to notify if the file cannot be opened
    if(fp == NULL)
    {
        printf("PELLET %d failed to write result into the file\n", getpid());
        exit(1);
    }
    else
    {
        if(wasEaten)    fprintf(fp, "PELLET %d was EATEN at column %d!\n", getpid(), y);
        else            fprintf(fp, "PELLET %d was MISSED at column %d\n", getpid(), y);
        fclose(fp);
    }
    
    kill(getpid(), SIGKILL);
    return 0;
}

// Kill the PELLET process by interrupt signal ^C
void pelletInterruption(int ignored)
{
    fprintf(stderr, "\n PELLET %d was killed by interrupt signal ^C", getpid());
    exit(0);
}

// Kill the PELLET process when the time limit reached
void pelletTermination(int ignored)
{
    fprintf(stderr, "\n PELLET %d was killed because time limit reached", getpid());
    exit(0);
}
