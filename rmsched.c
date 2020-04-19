/**
*  @author: Travis Mayer
*     ID: 002273275
*     Email: mayer129@mail.chapman.edu
*     Course: CPSC 353-01
*     Sources:    
*       Provided course material
*       Operating System Concepts - Ninth Edition
*       Multithread solution for checking a Sudoku puzzle example
*       https://www.youtube.com/watch?v=l6i67U1dJqs
*       https://www.sciencedirect.com/topics/computer-science/rate-monotonic-scheduling
*       https://github.com/raymo116/rmsched
*       https://www.embedded.com/introduction-to-rate-monotonic-scheduling/
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define TRUE 1
#define FALSE 0

typedef struct  { // struct for paramaters
    char* name;
    int wcet;
    int period;
    int remaining;
} parameters;

typedef struct { // container for paramaters struct
    parameters* params;
    int val;
} parametersContainer;

// variables
void *saveThread(void*);
int checkArray(int, parametersContainer*);
sem_t* sem1;
sem_t* sem2;
parameters* params;
parametersContainer paramsContainer;
int status = 1;
int l;
FILE* filePointer;
pthread_t* tid;


int main(int argc, char *argv[]) {
    if (argc != 4) { // basic error checking
        printf("usage: ./rmsched <nPeriods> <taskSet file> <schedule file>\n");
        return -1;
    }
    // initialize and assign variables
    int nPeriods = atoi(argv[1]);
    char* taskSet = argv[2];
    char* scheduler = argv[3];
    // checking for file
    if ((filePointer = fopen(taskSet, "r+")) == NULL) {
        printf("Unable to find file\n");
        return -1;
    }
    // intialize and assign variables
    char temp1[32];
    int wcet, period;
    paramsContainer.val = 0;
    int fileCheck;
    // checking for how much data to allocate
    while (TRUE) {
        fileCheck = fscanf(filePointer, "%s %d %d", temp1, &wcet, &period);
        if (fileCheck == 3) ++paramsContainer.val;
        else if (errno != 0) {
            perror("Error: scanf");
            break;
        } else if (fileCheck == EOF) break;
        else printf("No Match Found\n");
    }
    // restarting file and allocating data
    rewind(filePointer);
    paramsContainer.params = malloc(paramsContainer.val * sizeof(parameters));
    paramsContainer.val = 0;
    // returning data from file
    while (TRUE) {
        char* temp2 = malloc(32 * sizeof(char));
        fileCheck = fscanf(filePointer, "%s %d %d", temp2, &wcet, &period);
        if(fileCheck == 3) {
            int index = paramsContainer.val++;
            paramsContainer.params[index].name = temp2;
            paramsContainer.params[index].wcet = wcet;
            paramsContainer.params[index].period = period;
            paramsContainer.params[index].remaining = 0;
        } else if (errno != 0) {
            perror("Error: scanf");
            break;
        } else if (fileCheck == EOF) {
            break;
        } else printf("No Match Found\n");
    }
    // error checking
    fclose(filePointer);
    int test = paramsContainer.val;
    sem2 = malloc(sizeof(sem2));
    if (sem_init(sem2, 0, 0) == -1) printf("%s\n",strerror(errno));
    sem1 = malloc(paramsContainer.val * sizeof(sem1));
    int k;
    for (k = 0; k < paramsContainer.val; k++) {
        if(sem_init(&sem1[k], 0, 0) == -1) printf("%s\n",strerror(errno));
    }
    // initialize and assign variables, error checking, finding max period
    tid = malloc(paramsContainer.val * sizeof(tid));
    int max = 0;
    int j;
    for (j = 0; j < paramsContainer.val; j++) if(paramsContainer.params[j].period > max) max = paramsContainer.params[j].period;
    l = max;
    while (checkArray(l, &paramsContainer) != 1) ++l;
    int sum = 0;
    int a;
    for (a = 0; a < paramsContainer.val; a++) sum += paramsContainer.params[a].wcet / paramsContainer.params[a].period;
    if (sum <= 1) {
        int i;
        for (i = 0; i < paramsContainer.val; i++) {
            int* temp = malloc(sizeof(temp));
            *temp = i;
            pthread_create(&tid[i], NULL, saveThread, temp);
        }
        
        if ((filePointer = fopen(scheduler, "w+")) == NULL) {
            printf("Unable to open file\n");
            return -1;
        }
        // initialize and assign variables
        int* stack = malloc(paramsContainer.val * sizeof(stack));
        int topOfStack= -1;
        int j;
        for (j = 0; j < l; j++) fprintf(filePointer, "%d  ", j);
        fprintf(filePointer, "\n");
        // runs hyperperiods
        while (nPeriods > 0) {
            nPeriods--;
            for (j = 0; j < l; j++) {
                int i;
                for (i = 0; i < paramsContainer.val; i++) {
                    if (j % paramsContainer.params[i].period == 0) {
                        if (paramsContainer.params[i].remaining == 0) {
                            stack[++topOfStack] = i;
                            paramsContainer.params[i].remaining = paramsContainer.params[i].wcet;
                            int k;
                            for (k = topOfStack; k > 0; k--) {
                                if (paramsContainer.params[stack[k]].period > paramsContainer.params[stack[k - 1]].period) {
                                    int temp = stack[k];
                                    stack[k] = stack[k - 1];
                                    stack[k - 1] = temp;
                                }
                            }
                        } else { // error checking
                            printf("Unable to schedule task set\n");
                            status = 0;
                            remove(scheduler);
                        }
                    }
                } // check top of stack
                if (topOfStack != -1) {
                    sem_post(&sem1[stack[topOfStack]]);
                    sem_wait(sem2);
                    paramsContainer.params[stack[topOfStack]].remaining--;
                    if (paramsContainer.params[stack[topOfStack]].remaining == 0) topOfStack--;
                } else fprintf(filePointer, "__ ");
            }
            fprintf(filePointer, "\n");
        }
        // initialize and assign variables, closes filePointer, frees memory
        // sem_post, pthread_join
        fclose(filePointer);
        status = 0;
        int c;
        for (c = 0; c < paramsContainer.val; c++) sem_post(&sem1[c]);
        free(stack);
        for (i = 0; i < paramsContainer.val; i++) pthread_join(tid[i],NULL);
    } else printf("Unable to schedule task set\n");
    int b;
    for (b = 0; b < paramsContainer.val; b++) free(paramsContainer.params[b].name);
    free(paramsContainer.params);
    free(tid);
    free(sem1);
    return 0;
}
    // checks array each period
int checkArray(int lcm, parametersContainer* paramsContainer) {
    int k;
    for (k = 0; k < paramsContainer->val; k++) if (lcm % (paramsContainer->params[k].period != 0)) return 0;
    return 1;
}
    // writes thread name to file
void *saveThread(void* param) {
    int id = *((int *) param);
    while (status == TRUE) {
        sem_wait(&sem1[id]);
        if (status == TRUE) {
			fprintf(filePointer, "%s ", paramsContainer.params[id].name);
            sem_post(sem2);
        }
	}
    free(param);
    pthread_exit(0);
}