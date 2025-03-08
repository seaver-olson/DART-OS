#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <time.h>

typedef enum {READY, RUNNING, BLOCKED, SUSPENDED, FINISHED} state_t;
#define MAX_TASKS 2000
#define tSTACK_SIZE 4096 // 4KB 
#define hSTACK_SIZE 8192 // 8KB

void task1();
void task2();
void idle();

void schedule();
int create_task(void (*func)(void), int deadline);

void edf();


typedef struct {
    jmp_buf context;
    int id;
    state_t state;
    int deadline; // in microseconds
    char stack[tSTACK_SIZE];
    void (*func)(void);
} task;


