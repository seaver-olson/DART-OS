#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <setjmp.h>
#include <time.h>
#include <string.h>
#include <limits.h>  // For INT_MAX

typedef enum {READY, RUNNING, BLOCKED, SUSPENDED, FINISHED} state_t;
#define MAX_TASKS 200
#define tSTACK_SIZE 4096 // 4KB 
#define hSTACK_SIZE 8192 // 8KB

// Task characteristics
typedef struct {
    int id;
    state_t state;
    int deadline;      // in microseconds
    int period;        // for periodic tasks
    int execution_time; // estimated execution time
    int priority;      // task priority
    int arrival_time;  // when the task arrives
    int remaining_time; // remaining execution time
    int missed_deadlines; // counter for missed deadlines
    jmp_buf context;
    char stack[tSTACK_SIZE];
    void (*func)(void);
} task;

// Scheduling statistics
typedef struct {
    int total_tasks;
    int completed_tasks;
    int missed_deadlines;
    double cpu_utilization;
    double average_response_time;
    double average_waiting_time;
} scheduler_stats;

// Function declarations
void task1();
void task2();
void idle();
void schedule();
int create_task(void (*func)(void), int deadline, int period, int execution_time, int priority);
void edf();
void rm_schedule(); // Rate Monotonic scheduling
void fifo_schedule(); // First In First Out scheduling
void print_scheduler_stats();
void update_scheduler_stats();
void init_task_context(task *t, int id);
void task_wrapper(void);


