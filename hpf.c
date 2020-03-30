#include "headers.h"
#include "priority_queue.h"
// The coming "includes" for IPC
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <math.h>

#define MAXSIZE 100
struct msgbuff
{
    long mtype;
    struct Process_data data;
};

void int_handler();
void sigusr1_handler();

void run_process(int PS_ProcessingTime);
int pid; //The pid of the process..

FILE *f;
int counter=-1;
int s_TA=0;
int s_WTA=0;
double arr_WTA[MAXSIZE];

int start_now,end_now;
int main(int argc, char * argv[])
{
    signal(SIGINT,int_handler);
    signal(SIGUSR1,sigusr1_handler);
// FILE

    f = fopen("output.log", "w");

//Variables
    struct Process_data Popped;
    int process_name=-1;
    int status,pid_back;
    int TA;
    double WTA;
    int I_got_one=0;

// For IPC
    key_t msgqid;
    int msg_key = 15200;
    int recieving_status;
    struct msgbuff pg_message; //Instance of the buffer (Sent by the process generator)
    pg_message.mtype=0; //to recieve on all the tags.
    msgqid = msgget(msg_key,0644);

    if(msgqid == -1) perror("Error in recieving, No msg_queue exits");

// Intialize the priority queue
    heap_t *p_queue = (heap_t *)calloc(1, sizeof (heap_t));

//To intialize the clock
    initClk();

    /*
    //Test Data [ Untill we finish the IPC Part ]
    struct Process_data my_data = {10, 3, 5, 12, 1}; //SOMETHING WEIRD HERE IN THE NAMING.
    push(p_queue, my_data.priority, my_data);
    struct Process_data my_data2 = {20, 4, 6, 12, 4};
    push(p_queue, my_data2.priority, my_data2);
    struct Process_data my_data3 = {30, 6, 7, 12, 6};
    push(p_queue, my_data3.priority, my_data3);
    */

    while(getClk()<1); //Start form clock=1;
// The main functionality
    while(1) {
        printf("====SCHEDULER PID==== %d\n",getpid());
        process_name=-1;

        // [ Continuous Recieving ]
        if(I_got_one==0) {
            recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),14,IPC_NOWAIT);
            if(recieving_status != -1) push(p_queue, pg_message.data.priority, pg_message.data);
            else printf("Recieved NOTHING..\n");
        }

// [ Pop a process ]
        Popped = pop(p_queue);
        process_name=Popped.name; //Now the process_string is ready to pass to execve
//printf("%d\n",process_name);

        if (process_name==-1) {
            printf("NO NEW processes to be run --> Gonna wait\n");
            recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),14,!IPC_NOWAIT);
            printf("I recieved.. %d\n",pg_message.data.name);
            push(p_queue, pg_message.data.priority, pg_message.data);
            I_got_one=1;
        }
        else {
            I_got_one=0;
            //printf("HERE RIGHT?\n");
            printf("The next process to run: %d for %d seconds \n",process_name,Popped.processing_time);
            pid=fork();
            if (pid == -1) perror("error in fork");
            else if (pid == 0) run_process(Popped.processing_time);
            ////=================================/////
            else {  //the scheduler
                start_now=getClk();
                fprintf(f,"At time %d Process %d started arr %d total %d remain %d wait %d\n",start_now,process_name,Popped.arrival_time,Popped.processing_time,Popped.processing_time,start_now-Popped.arrival_time);
                pid_back= wait(&status);
                printf("HERE..\n");
                if(!(status & 0x00FF)) { //if exited normally
                    end_now=getClk();
                    counter++;
                    TA=end_now-Popped.arrival_time;
                    s_TA+=TA;
                    WTA=TA/Popped.processing_time;
                    arr_WTA[counter]=WTA;
                    s_WTA+=WTA;
                    fprintf(f,"At time %d Process %d finished arr %d total %d remain 0 wait %d TA %d WTA %f\n",end_now,process_name,Popped.arrival_time,Popped.processing_time,start_now-Popped.arrival_time,TA,WTA);
                }
            }
            ////=================================/////

        }
    } //End of while loop
    destroyClk(true);
}

void run_process(int PS_ProcessingTime) {
    char pt_string[9]; //SOMETHING WEIRD HERE... less than 9 cause error
    sprintf(pt_string, "%d", PS_ProcessingTime); //convert to string
    char *argv[] = { "./process.out", pt_string , 0 }; //ZERO IS SHHHHHIIIIIIT
    execve(argv[0], &argv[0], NULL); //start the new process
}

void int_handler() {

    double std_WTA=0;
    double avg_TA = (double)s_TA/counter;
    double avg_WTA = (double)s_WTA/counter;
    fprintf(f,"Average TA = %f\n",avg_TA);
    fprintf(f,"Average WTA = %f\n",avg_WTA);
    /*for(int i=0; i<counter; i++){
      std_WTA += pow((arr_WTA[i]-avg_WTA),2);
     }
     fprintf(f,"std WTA = %f\n",sqrt(std_WTA/counter));*/
    exit(0);
}

void sigusr1_handler(int signum){
// Empty
 //printf("SIGUSR1 RECIEVED..\n");
}

