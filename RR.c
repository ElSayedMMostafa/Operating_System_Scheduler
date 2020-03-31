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

struct msgbuff
{
    long mtype;
    struct Process_data data;
};
int quantum = 4;
int start_now,stop_now; //Global Variables
// For IPC
key_t msgqid;
int msg_key = 12345;
int recieving_status;
struct msgbuff pg_message; //Instance of the buffer (Sent by the process generator)

void run_process(int PS_ProcessingTime);
void sigusr1_handler(int signum);
void child_handler(int signum);
int pid;

int main(int argc, char * argv[])
{
    //signal(SIGCHLD,child_handler);
	signal(SIGUSR1,sigusr1_handler);
//Variables
    struct Process_data Popped;
    int process_name;
    int status,pid_back;
    int I_got_one=0;

// For IPC
    int recieving_status;
    pg_message.mtype=0; //to recieve on all the tags.
    msgqid = msgget(msg_key,0644);
    if(msgqid == -1) perror("Error in recieving, No msg_queue exits");

// Intialize the priority queue
    heap_t *p_queue = (heap_t *)calloc(1, sizeof (heap_t));

//To intialize the clock
    initClk();

   /*
    //Test Data [ Untill we finish the IPC Part ]
    struct Process_data my_data = {1, 1, 5, 5, 2}; //SOMETHING WEIRD HERE IN THE NAMING.
    push(p_queue, my_data.arrival_time, my_data);
    struct Process_data my_data2 = {2, 2, 7, 7, 3};
    push(p_queue, my_data2.arrival_time, my_data2);
    struct Process_data my_data3 = {3, 3, 8, 8, 4};
    push(p_queue, my_data3.arrival_time, my_data3);
    */


    while(getClk()<1); //Start form clock=1;
// The main functionality
    while(1) {

        printf("====SCHEDULER PID============================== %d\n",getpid());
        if(I_got_one==0) {
            // [ Continuous Recieving ]
            recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
            if(recieving_status != -1) {push(p_queue, getClk(), pg_message.data); printf("I recieved process %d\n",pg_message.data.name);}
        } else printf("NO I HAVE ONE\n");
// [ Pop a process ]
        Popped = pop(p_queue);
        process_name=Popped.name; //Now the process_string is ready to pass to execve
        printf("The next process to run: %d\n",process_name);
        if (process_name==-1) {
            printf("NO NEW processes to be run --> Gonna wait\n");
            recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,!IPC_NOWAIT);
            push(p_queue, getClk(), pg_message.data);
            I_got_one=1;
        }
        else { 
            I_got_one=0;
            if(Popped.processing_time != Popped.remaining_time) { 
	        printf("GONNNA CONTINUE A PROCESS NOW..\n"); 
                printf("Process %d with pid %d to be continued, re_time= %d at %d \n", process_name, Popped.process_pid, Popped.remaining_time,getClk());
                kill(Popped.process_pid,SIGCONT);
                start_now=getClk();
	        printf("%d\n",quantum);
                sleep(quantum);
                if(Popped.remaining_time > quantum) { //The quantum ended
	        printf("[Continue] At %d woke up Because of QUANTUM..\n",getClk());
                    kill(Popped.process_pid,SIGSTOP);
                    Popped.remaining_time = Popped.remaining_time - quantum;
                    push(p_queue, getClk(), Popped);
                } else {
                    pid_back= wait(&status);
	        printf("[Continue] At %d woke up Because of EXIT_CODE..\n",getClk());
                    if(!(status & 0x00FF))  printf("Process %d with pid %d finished\n", process_name, pid_back);
                }

            } 

            else { 
	        printf("GONNNA FORK NOW..\n");
                pid=fork();
                if (pid == -1) perror("error in fork");
                else if (pid == 0) run_process(Popped.processing_time);
                //////===========================================================/////////
                else {  //the scheduler  ///-->%
                    Popped.process_pid = pid;
                    start_now=getClk();
                    printf("At time %d Process %d started arr %d total %d remain %d wait %d\n",start_now,process_name,Popped.arrival_time,Popped.processing_time,Popped.processing_time,start_now-Popped.arrival_time); //Move this line to OUTPUT_FILE
                    sleep(quantum);
                    if(Popped.remaining_time > quantum) { //The quantum ended
	        printf("[Fork] At %d woke up Because of QUANTUM..\n",getClk());
                        kill(pid,SIGSTOP);
                        Popped.remaining_time = Popped.remaining_time - quantum;
                        push(p_queue, getClk(), Popped);
                    } else {
	        printf("[Fork] At %d woke up Because of EXIT_CODE..\n",getClk());
                        pid_back= wait(&status);
                        if(!(status & 0x00FF))  printf("Process %d with pid %d finished with exit code %d\n", process_name, pid_back, status>>8);
                    }
                }   
                //////=========================================/////////
            } 

        }
    } //End of while loop
    destroyClk(true);
}

void run_process(int PS_ProcessingTime) {
    char pt_string[9]; //SOMETHING WEIRD HERE... less than 9 cause error
    sprintf(pt_string, "%d", PS_ProcessingTime); //convert to string
    char *argv[] = { "./process.out", pt_string, 0 };
    execve(argv[0], &argv[0], NULL); //start the new process
    printf("NOOOOOOOOOOOOOOOOOOOOOOOO\n");
}
void child_handler(int signum) {
    printf("CHILD \n");
}

void sigusr1_handler(int signum){
// Empty
 //printf("SIGUSR1 RECIEVED..\n");
}
