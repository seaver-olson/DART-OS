#include "sim.h"
//hey Neil if your looking for my struct i moved it to sim.h
//im kinda losing my mind with scheduling algorithms so I wanted to make a quick demo without worrying about the hardware or IRQs
//I'm using setjmp and longjmp to simulate context switching
//I will also use this to test the EDF algorithm 

task tasks[MAX_TASKS];
int num_tasks = 0;
int curr_task = 0;
char task_stack[MAX_TASKS][tSTACK_SIZE];

void task_master(int x){
    //creates x number of tasks for cpu util testing
    for (int i = 0; i < x; i++){
        create_task(task1, rand() % 1000);
    }
}

int create_task(void (*func)(void), int deadline){
    if (num_tasks >= MAX_TASKS){
        printf("Error: Max number of tasks reached\n");
        return -1;
    }
    int id = num_tasks;
    task *t = &tasks[id];
    t->id = id;
    t->state = READY;
    t->func = func;
    t->deadline = deadline;

    // Set the stack pointer in the context to point to the task's stack
    t->context[5] = (int)(task_stack[id] + tSTACK_SIZE - 1); // Set the stack pointer to the end of the task's stack

    if (setjmp(t->context) == 0){
        num_tasks++;
        printf("Task %d created\n", id);
    } else {
        // Task is running, call the function and mark as finished
        t->func();
        t->state = FINISHED;
    }
    return 0;
}



void schedule(){
    if (num_tasks == 0){
        printf("Error: No tasks to schedule\n Please create a task before calling schedule\n This can be done with the create_task function\n");
        exit(1);
    }
    edf();
}

int main(){
    srand(time(0));
    create_task(idle, 100000);//.1 seconds 
    task_master(10);
    printf("task count: %d\n", num_tasks);
    if (setjmp(tasks[curr_task].context) == 0) {
        printf("Starting scheduler\n");
        tasks[curr_task].state = RUNNING;
        longjmp(tasks[curr_task].context, 1);
    }
    while (1){
        schedule();
    }
    return 0;
}

//from the current m tasks, select earliest deadline task if not blocked or finished
//if blocked or finished, select next task with earliest deadline
void edf() {
    int i, min = -1;//i is current task, min is the task with the earliest deadline
    // Find the task with the earliest deadline
    for (i = 0; i < num_tasks; i++) {
        if (tasks[i].state == 0) {
            if (min == -1 || tasks[i].deadline < tasks[min].deadline) {
                min = i;
            }
        } else {
            printf("Task %d is %d\n", i, tasks[i].state);
        }
    }
    //doesnt switch if the current task is the one with the earliest deadline
    if (tasks[curr_task].state == FINISHED || (min != -1 && min != curr_task)) { 
        if (setjmp(tasks[curr_task].context) == 0) {
            if (tasks[curr_task].state != READY && tasks[curr_task].state != RUNNING) {
                printf("Error: Attempted to switch to uninitialized task %d\n", curr_task);
                return;
            }
            tasks[curr_task].state = READY;
            printf("Switching from task %d to task %d\n", curr_task, min);
            curr_task = min;
            tasks[curr_task].state = RUNNING;
            longjmp(tasks[curr_task].context, 1);
        }
    } else if (min == -1) {
        printf("No tasks available, entering idle state.\n");
        curr_task = 0;
        longjmp(tasks[curr_task].context, 1);
    }
}


void idle(){
    while(1){
        printf("Idle\n");
        sleep(1);
    }
}
void task1(){
    int i = 0;
    for (i = 0; i < 10; i++){
        printf("Task 1 {Count: %d}\n", i);
        sleep(1);
    }
    tasks[curr_task].state = FINISHED;
}

void task2(){
    int i = 0;
    for (i = 0; i < 10; i++){
        printf("Task 2 {Count: %d}\n", i);
        sleep(1);
    }
    tasks[curr_task].state = FINISHED;
}