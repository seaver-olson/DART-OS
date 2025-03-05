#include <stdio.h>
#include <setjmp.h>
#include <unistd.h>

#define MAX_TASKS 10  // Tasks are FreeRTOS threads

struct task {
    int id;
    int arrival_time;  // Time at which the task arrives
    jmp_buf context;
    int priority;
};

struct task tasks[MAX_TASKS] = {0};
int current_task = 0;  // Start from the first task
int num_tasks = 0;


void task1() {
    while (1) {
        printf("Task 1 executing\n");
        if (setjmp(tasks[current_task].context) == 0) return;
    }
}

void task2() {
    while (1) {
        printf("Task 2 executing\n");
        if (setjmp(tasks[current_task].context) == 0) return;
    }
}

void task3() {
    while (1) {
        printf("Task 3 executing\n");
        if (setjmp(tasks[current_task].context) == 0) return;
    }
}

void task_create(int id, int arrival_time, int priority, void (*task_func)()) {
    if (num_tasks >= MAX_TASKS) return;  // Prevent overflow

    tasks[num_tasks].id = id;
    tasks[num_tasks].arrival_time = arrival_time;
    tasks[num_tasks].priority = priority;

    if (setjmp(tasks[num_tasks].context) == 0) {
        num_tasks++;
        return;  
    }
    task_func(); // Task runs when switched to
}

void schedule() {
    if (num_tasks == 0) {
        printf("No tasks to schedule\n");
        return;
    }
    current_task = 0;
    longjmp(tasks[current_task].context, 1);

    while (1) {
        if (setjmp(tasks[current_task].context) == 0) {
            
            usleep(1000000);  // Sleep for 1 second
            printf("here");
            current_task = (current_task + 1) % num_tasks;
            printf("g");
            longjmp(tasks[current_task].context, 1);
        }
    }
}

int main() {
    printf("Starting scheduler\n");

    task_create(1, 0, 1, task1);
    task_create(2, 0, 2, task2);
    task_create(3, 0, 3, task3);

    schedule(); // Start the scheduler

    return 0;
}
