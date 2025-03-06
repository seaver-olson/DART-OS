#include "sim.h"
//hey neil if your looking for my struct i moved it to sim.h

task tasks[MAX_TASKS];
int num_tasks = 0;
int curr_task = 0;

void task1(){
    printf("Task 1\n");
}

void task2(){
    printf("Task 2\n");
}

int create_task(void (*func)(void)){
    if (num_tasks >= MAX_TASKS) return -1;
    int id = num_tasks;
    task *t = &tasks[id];
    t->id = id;
    t->state = READY;
    t->func = func;
    num_tasks++;
    return 0;
}



int main(){
    create_task(task1);
    create_task(task2);
    tasks[0].func();
    tasks[1].func();
    return 0;
}