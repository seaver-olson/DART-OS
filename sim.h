#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>

typedef enum {READY, RUNNING, BLOCKED, SUSPENDED, FINISHED} state_t;
#define MAX_TASKS 10
#define tSTACK_SIZE 4096 // 4KB 
#define hSTACK_SIZE 8192 // 8KB

typedef struct {
    jmp_buf context;
    int id;
    state_t state;
    char stack[tSTACK_SIZE];
    void (*func)(void);
} task;


