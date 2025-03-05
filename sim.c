#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>
#include <unistd.h>

#define MAX_TASKS 10
#define QUANTUM_US 500000 // 0.5 seconds in microseconds

struct task {
    int id;
    int arrival_time;
    jmp_buf context;
    int priority;
    void (*task_func)();
};

struct task tasks[MAX_TASKS] = {0};
int current_task = -1;
int num_tasks = 0;
int system_time = 0; // Simulated system time

void task1(); // Forward declarations
void task2();
void task3();

void task1() {
    while (1) {
        printf("Task 1 executing at time %d\n", system_time);
        usleep(QUANTUM_US / 2);
        system_time += QUANTUM_US / 2 / 1000;
        if (setjmp(tasks[current_task].context) == 0) return;
    }
}

void task2() {
    while (1) {
        printf("Task 2 executing at time %d\n", system_time);
        usleep(QUANTUM_US / 2);
        system_time += QUANTUM_US / 2 / 1000;
        if (setjmp(tasks[current_task].context) == 0) return;
    }
}

void task3() {
    while (1) {
        printf("Task 3 executing at time %d\n", system_time);
        usleep(QUANTUM_US / 2);
        system_time += QUANTUM_US / 2 / 1000;
        if (setjmp(tasks[current_task].context) == 0) return;
    }
}

void task_create(int id, int arrival_time, int priority, void (*task_func)()) {
    if (num_tasks >= MAX_TASKS) return;

    tasks[num_tasks].id = id;
    tasks[num_tasks].arrival_time = arrival_time;
    tasks[num_tasks].priority = priority;
    tasks[num_tasks].task_func = task_func;

    num_tasks++; // Increment num_tasks here, before the setjmp
}

void schedule() {
    if (num_tasks == 0) {
        printf("No tasks to schedule\n");
        return;
    }

    current_task = 0;

    for (int i = 0; i < num_tasks; i++) {
        if(setjmp(tasks[i].context) == 0){
            tasks[i].task_func();
        }
    }

    while (1) {
        if (setjmp(tasks[current_task].context) == 0) {
            usleep(QUANTUM_US);
            system_time += QUANTUM_US / 1000;
            current_task = (current_task + 1) % num_tasks;
            longjmp(tasks[current_task].context, 1);
        }
    }
}

int main() {
    printf("Starting scheduler\n");

    task_create(1, 0, 1, task1);
    task_create(2, 0, 2, task2);
    task_create(3, 0, 3, task3);

    schedule();

    return 0;
}