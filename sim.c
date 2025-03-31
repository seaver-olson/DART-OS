#include "sim.h"

task tasks[MAX_TASKS];
int num_tasks = 0;
int curr_task = 0;
scheduler_stats stats = {0};
int current_time = 0;

char task_stack[MAX_TASKS][tSTACK_SIZE];

// Initialize task stack and context
void init_task_context(task *t, int id) {
    // Initialize stack with a pattern to help debug stack issues
    memset(t->stack, 0xAA, tSTACK_SIZE);
    
    // Set up the stack pointer to point to the top of the stack
    // Align to 16 bytes for better performance
    t->context[5] = (uintptr_t)(t->stack + tSTACK_SIZE - 16);
    
    // Initialize other context registers
    t->context[0] = 0;  // Program counter
    t->context[1] = 0;  // Stack pointer
    t->context[2] = 0;  // Frame pointer
    t->context[3] = 0;  // General purpose registers
    t->context[4] = 0;
}

void task_master(int x){
    //creates x number of tasks for cpu util testing
    for (int i = 0; i < x; i++){
        if ((create_task(task1, 
                        rand() % 1000,  // deadline
                        rand() % 2000 + 500,  // period
                        rand() % 5 + 1,  // execution time
                        rand() % 10 + 1  // priority
                        )) != 0){
            printf("Error creating task %d\n", i);
        }
    }
}

int create_task(void (*func)(void), int deadline, int period, int execution_time, int priority) {
    if (num_tasks >= MAX_TASKS) {
        printf("Error: Max number of tasks reached\n");
        return -1;
    }

    int id = num_tasks;
    task *t = &tasks[id];
    t->id = id;
    t->state = READY;
    t->func = func;
    t->deadline = deadline;
    t->period = period;
    t->execution_time = execution_time;
    t->remaining_time = execution_time;
    t->priority = priority;
    t->arrival_time = current_time;
    t->missed_deadlines = 0;

    // Initialize the task's context and stack
    init_task_context(t, id);

    // Set up the initial context
    if (setjmp(t->context) == 0) {
        num_tasks++;
        stats.total_tasks++;
        printf("Task %d created: deadline=%d, period=%d, exec_time=%d, priority=%d\n", 
               id, deadline, period, execution_time, priority);
        return 0;
    }
    return 1;
}

void schedule(){
    if (num_tasks == 0){
        printf("Error: No tasks to schedule\n Please create a task before calling schedule\n This can be done with the create_task function\n");
        exit(1);
    }
    edf();
}

void task_wrapper(void) {
    // Get the current task
    task *t = &tasks[curr_task];
    
    // Call the task function
    t->func();
    
    // Task completed
    t->state = FINISHED;
    stats.completed_tasks++;
    printf("Task %d completed at time %d\n", t->id, current_time);
    
    // Return to scheduler
    schedule();
}

int main(){
    srand(time(0));
    create_task(idle, 100000, 100000, 1, 0);  // Idle task
    task_master(10);
    printf("task count: %d\n", num_tasks);
    if (setjmp(tasks[curr_task].context) == 0) {
        printf("Starting scheduler\n");
        tasks[curr_task].state = RUNNING;
        task_wrapper();
    }
    while (1){
        update_scheduler_stats();
        print_scheduler_stats();
        schedule();
        sleep(1);
    }
    return 0;
}

//Earliest Deadline First Scheduler
void edf() {
    int min_deadline = INT_MAX;
    int next_task = -1;

    // Find task with earliest deadline
    for (int i = 0; i < num_tasks; i++) {
        if (tasks[i].state == READY && tasks[i].deadline < min_deadline) {
            min_deadline = tasks[i].deadline;
            next_task = i;
        }
    }

    // Check for deadline misses
    for (int i = 0; i < num_tasks; i++) {
        if (tasks[i].state == READY && current_time > tasks[i].deadline) {
            tasks[i].missed_deadlines++;
            stats.missed_deadlines++;
            printf("Task %d missed deadline at time %d\n", i, current_time);
        }
    }

    // Switch to next task if needed
    if (next_task != -1 && next_task != curr_task) {
        if (setjmp(tasks[curr_task].context) == 0) {
            tasks[curr_task].state = READY;
            printf("Time %d: Switching from task %d to task %d (deadline: %d)\n", 
                   current_time, curr_task, next_task, tasks[next_task].deadline);
            curr_task = next_task;
            tasks[curr_task].state = RUNNING;
            tasks[curr_task].remaining_time--;
            
            if (tasks[curr_task].remaining_time <= 0) {
                tasks[curr_task].state = FINISHED;
                stats.completed_tasks++;
                printf("Task %d completed at time %d\n", curr_task, current_time);
            }

            if (tasks[curr_task].context[0] != 0) {
                longjmp(tasks[curr_task].context, 1);
            }
        }
    }
}

//Rate Monotonic Scheduler
void rm_schedule() {
    int highest_priority = -1;
    int min_period = INT_MAX;

    for (int i = 0; i < num_tasks; i++) {
        if (tasks[i].state == READY && tasks[i].period < min_period) {
            min_period = tasks[i].period;
            highest_priority = i;
        }
    }

    if (highest_priority != -1 && highest_priority != curr_task) {
        if (setjmp(tasks[curr_task].context) == 0) {
            tasks[curr_task].state = READY;
            printf("Time %d: RM switching to task %d (period: %d)\n", 
                   current_time, highest_priority, tasks[highest_priority].period);
            curr_task = highest_priority;
            tasks[curr_task].state = RUNNING;
            tasks[curr_task].remaining_time--;
            
            if (tasks[curr_task].remaining_time <= 0) {
                tasks[curr_task].state = FINISHED;
                stats.completed_tasks++;
            }

            if (tasks[curr_task].context[0] != 0) {
                longjmp(tasks[curr_task].context, 1);
            }
        }
    }
}

//First In First Out Scheduler
void fifo_schedule() {
    int earliest_arrival = INT_MAX;
    int next_task = -1;

    for (int i = 0; i < num_tasks; i++) {
        if (tasks[i].state == READY && tasks[i].arrival_time < earliest_arrival) {
            earliest_arrival = tasks[i].arrival_time;
            next_task = i;
        }
    }

    if (next_task != -1 && next_task != curr_task) {
        if (setjmp(tasks[curr_task].context) == 0) {
            tasks[curr_task].state = READY;
            printf("Time %d: FIFO switching to task %d (arrival: %d)\n", 
                   current_time, next_task, tasks[next_task].arrival_time);
            curr_task = next_task;
            tasks[curr_task].state = RUNNING;
            tasks[curr_task].remaining_time--;
            
            if (tasks[curr_task].remaining_time <= 0) {
                tasks[curr_task].state = FINISHED;
                stats.completed_tasks++;
            }

            if (tasks[curr_task].context[0] != 0) {
                longjmp(tasks[curr_task].context, 1);
            }
        }
    }
}

void update_scheduler_stats() {
    // Calculate CPU utilization
    int total_execution = 0;
    for (int i = 0; i < num_tasks; i++) {
        total_execution += tasks[i].execution_time;
    }
    stats.cpu_utilization = (double)total_execution / current_time * 100;
}

void print_scheduler_stats() {
    printf("\n=== Scheduler Statistics ===\n");
    printf("Total Tasks: %d\n", stats.total_tasks);
    printf("Completed Tasks: %d\n", stats.completed_tasks);
    printf("Missed Deadlines: %d\n", stats.missed_deadlines);
    printf("CPU Utilization: %.2f%%\n", stats.cpu_utilization);
    printf("Current Time: %d\n", current_time);
    printf("========================\n\n");
}

void idle() {
    while (1) {
        printf("Idle at time %d\n", current_time);
        current_time++;
        sleep(1);
    }
}

void task1() {
    printf("Task 1 started at time %d\n", current_time);
    for (int i = 0; i < 3; i++) {
        printf("Task 1 running at time %d\n", current_time);
        sleep(1);
        current_time++;
    }
    tasks[curr_task].state = FINISHED;
}

void task2() {
    printf("Task 2 started at time %d\n", current_time);
    for (int i = 0; i < 4; i++) {
        printf("Task 2 running at time %d\n", current_time);
        sleep(1);
        current_time++;
    }
    tasks[curr_task].state = FINISHED;
}