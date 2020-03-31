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


int start_now,stop_now; //Global Variables
// For IPC
key_t msgqid;
int msg_key = 12345;
int recieving_status;
struct msgbuff pg_message; //Instance of the buffer (Sent by the process generator)

void run_process(int PS_ProcessingTime);
void sigusr1_handler(int signum);
void sigusr2_handler(int signum);
int pid;

int main(int argc, char * argv[])
{
    printf("%d\n",getpid());
    // signal(SIGUSR1,sigusr1_handler); ---> I don't need it is SRTN
    signal(SIGUSR2,sigusr2_handler);
//Variables
    struct Process_data Popped;
    int process_name;
    int status,pid_back;

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
        struct Process_data my_data = {'A', 1, 4, 4, 2}; //SOMETHING WEIRD HERE IN THE NAMING.
        push(p_queue, my_data.remaining_time, my_data);
        struct Process_data my_data2 = {'B', 2, 5, 5, 3};
        push(p_queue, my_data2.remaining_time, my_data2);
        struct Process_data my_data3 = {'C', 3, 6, 6, 4};
        push(p_queue, my_data3.remaining_time, my_data3);
    */


    while(getClk()<1); //Start form clock=1;
// The main functionality
    while(1) {
        printf("====SCHEDULER PID==== %d\n",getpid());
        // [ Continuous Recieving ]
        recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
        if(recieving_status != -1) {
            push(p_queue, pg_message.data.remaining_time, pg_message.data);
            printf("I recieved %d\n",pg_message.data.name);
        }
// [ Pop a process ]
        Popped = pop(p_queue);
        process_name=Popped.name; //Now the process_string is ready to pass to execve
        printf("The next process to run: %d, re_time = %d\n",process_name, Popped.remaining_time);
        if (process_name==-1) {
            printf("NO NEW processes to be run --> Gonna wait\n");
            recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,!IPC_NOWAIT);
            printf("I recieved %d\n",pg_message.data.name);
            push(p_queue, pg_message.data.remaining_time, pg_message.data);
        }
        else {
            //////////////////////////////////////////////////////////////////////////////////
            if(Popped.processing_time != Popped.remaining_time) {
                printf("At %d Process %d with pid %d to be continued, re_time= %d \n",getClk(),process_name, Popped.process_pid, Popped.remaining_time);
                kill(Popped.process_pid,SIGCONT);
                start_now=getClk();
                sleep(Popped.remaining_time); //Terminated by [ TIME ] OR [ SIGUSR2 ]
                recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
                if(recieving_status != -1) {
                    push(p_queue, pg_message.data.priority, pg_message.data);
                    printf("I recieved %d\n",pg_message.data.name);
                    if((Popped.remaining_time - (getClk()-start_now))>pg_message.data.remaining_time) {
                        printf("[ SIGUSR 2 ]\n");
                        kill(pid,SIGSTOP);
                        stop_now=getClk();
                        Popped.remaining_time = Popped.remaining_time - (stop_now - start_now);
                        printf("At %d Process %d with pid %d is stopped, re_time= %d \n",getClk(),process_name, Popped.process_pid, Popped.remaining_time);
                        push(p_queue, Popped.remaining_time, Popped);
                    } else {// (Popped.remaining_time - (getClk()-start_now)) < message.data.remaining_time)
                        printf("[ SIGUSR2-->TIME ]\n");
                        pid_back= wait(&status);
                        if(!(status & 0x00FF))  printf("At %d Process %d with pid %d finished with exit code %d\n",getClk(), process_name, pid_back, status>>8);
                    }
                } else {// recieving_status == -1
                    printf("[ TIME ]\n");
                    pid_back= wait(&status);
                    if(!(status & 0x00FF))  printf("At %d Process %d with pid %d finished with exit code %d\n", getClk(),process_name, pid_back, status>>8);
                }
            }
            //////////////////////////////////////////////////////////////////////////////////
            else {
                pid=fork();
                if (pid == -1) perror("error in fork");
                else if (pid == 0) run_process(Popped.remaining_time);
                else {  //the scheduler
                    Popped.process_pid = pid;
                    start_now=getClk();
                    printf("At %d Process %d started arr %d total %d remain %d wait %d\n",start_now,process_name,Popped.arrival_time,Popped.processing_time,Popped.processing_time,start_now-Popped.arrival_time); //Move this line to OUTPUT_FILE
                    printf("I SHOULD SLEEP HERE FOR %d\n", Popped.remaining_time);
                    sleep(Popped.remaining_time); //Terminated by [ TIME ] OR [ SIGUSR2 ]
                    recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
                    if(recieving_status != -1) {
                        push(p_queue, pg_message.data.priority, pg_message.data);
                        printf("I recieved %d\n",pg_message.data.name);
                        if((Popped.remaining_time - (getClk()-start_now))>pg_message.data.remaining_time) {
                            printf("[ SIGUSR 2 ]\n");
                            kill(pid,SIGSTOP);
                            stop_now=getClk();
                            Popped.remaining_time = Popped.remaining_time - (stop_now - start_now);
                            printf("At %d Process %d with pid %d is stopped, re_time= %d \n",getClk(),process_name, Popped.process_pid, Popped.remaining_time);
                            push(p_queue, Popped.remaining_time, Popped);
                        } else {// (Popped.remaining_time - (getClk()-start_now)) < message.data.remaining_time)
                            printf("[ SIGUSR2-->TIME ]\n");
                            pid_back= wait(&status);
                            if(!(status & 0x00FF))  printf("At %d Process %d with pid %d finished with exit code %d\n",getClk(), process_name, pid_back, status>>8);
                        }
                    } else {// recieving_status==-1
                        printf("[ TIME ]\n");
                        pid_back= wait(&status);
                        if(!(status & 0x00FF))  printf("At %d Process %d with pid %d finished with exit code %d\n", getClk(),process_name, pid_back, status>>8);
                    }
                }//Scheduler else
            }//Forking or Continue else
        }//Process Existance else
    } //End of while loop
    destroyClk(true);
}

void run_process(int PS_ProcessingTime) {
    char pt_string[9]; //SOMETHING WEIRD HERE... less than 9 cause error
    sprintf(pt_string, "%d", PS_ProcessingTime); //convert to string
    char *argv[] = { "./process.out", pt_string, 0 };
    execve(argv[0], &argv[0], NULL); //start the new process
}
void sigusr2_handler(int signum) {
    printf("SIGUSR2 RECIEVED..\n");
}
void sigusr1_handler(int signum) {
// Empty
    printf("SIGUSR1 RECIEVED..\n");
}
