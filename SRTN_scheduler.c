#include "headers.h"
#include "priority_queue.h"
#include <math.h>
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

FILE *f_log;
FILE *f_perf;
int start_now,stop_now; //Global Variables
// For IPC
key_t msgqid,msgqid2;
int msg_key = 12345;
int msg_key2 = 12346;
int recieving_status;
struct msgbuff pg_message;//Instance of the buffer (Sent by the process generator)
int s_TA=0, s_PT=0;
double s_WTA=0;
double WTA_arr[100];
int count=0, start, finish;

void int_handler();
void run_process(int PS_ProcessingTime);
void sigusr1_handler(int signum);
void sigusr2_handler(int signum);
int pid;
int first_start;
int main(int argc, char * argv[])
{
    printf("%d\n",getpid());
    signal(SIGUSR1,sigusr1_handler); //---> I don't need it is SRTN
    signal(SIGUSR2,sigusr2_handler);
    signal(SIGINT, int_handler);
    f_log=fopen("scheduler.log", "w");
    f_perf=fopen("scheduler.perf", "w");
//Variables
    struct Process_data Popped;
    int process_name;
    int status,pid_back;
    int to_finish,out;
    int TA;
    double WTA;
// For IPC
    int recieving_status;
    pg_message.mtype=0; //to recieve on all the tags.
    msgqid = msgget(msg_key,0644);
    msgqid2 = msgget(msg_key2,0644);
    if(msgqid == -1) perror("Error in recieving, No msg_queue exits");

// Intialize the priority queue
    heap_t *p_queue = (heap_t *)calloc(1, sizeof (heap_t));

//To intialize the clock
    initClk();

    while(getClk()<1); //Start form clock=1;
// The main functionality
    while(1) {
        // [ Continuous Recieving ]
        recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
        if(recieving_status != -1)  push(p_queue, pg_message.data.remaining_time, pg_message.data);
           
      
// [ Pop a process ]
        Popped = pop(p_queue);
        process_name=Popped.name; //Now the process_string is ready to pass to execve
        //  printf("The next process to run: %d, re_time = %d\n",process_name, Popped.remaining_time);
        if (process_name==-1) {
            recieving_status = msgrcv(msgqid2, &pg_message, sizeof(pg_message.data),14,IPC_NOWAIT);
            if(recieving_status != -1) break;
            printf("NO NEW processes to be run --> Gonna wait\n");
            recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,!IPC_NOWAIT);
            //  printf("I recieved %d\n",pg_message.data.name);
            push(p_queue, pg_message.data.remaining_time, pg_message.data);
        }
        else {
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
            if(Popped.processing_time != Popped.remaining_time) {

                kill(Popped.process_pid,SIGCONT);
                start_now=getClk();
                Popped.waiting_time= Popped.waiting_time + Popped.stop_time - start_now;
                fprintf(f_log,"At time %d process %d resumed arr %d total %d remain %d wait %d\n", start_now, Popped.name, Popped.arrival_time, Popped.processing_time, Popped.remaining_time, Popped.waiting_time);
                out=0;
                to_finish=start_now+Popped.remaining_time;


                while( out==0 && getClk() < to_finish ) {
                    sleep(2*to_finish-getClk()); //Terminated by [ TIME ] OR [ SIGUSR2 ]
                    recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
                    while(recieving_status != -1) {

                        push(p_queue, pg_message.data.remaining_time, pg_message.data);
                        if((Popped.remaining_time - (getClk()-start_now))>pg_message.data.remaining_time) out=1;
                        recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
                    }
                }

                if(out==1) {
                    kill(pid,SIGSTOP);
                    stop_now=getClk();
                    Popped.remaining_time = Popped.remaining_time - (stop_now - start_now);
                    push(p_queue, Popped.remaining_time, Popped);

                    Popped.stop_time=stop_now;
                    fprintf(f_log,"At time %d process %d stopped arr %d total %d remain %d wait %d\n", stop_now, Popped.name, Popped.arrival_time, Popped.processing_time, Popped.remaining_time, Popped.waiting_time);
                } else {
                    pid_back= wait(&status);
                    if(!(status & 0x00FF))  printf("At %d Process %d with pid %d finished with exit code %d\n",getClk(), process_name, pid_back, status>>8);
                    stop_now=getClk();
                    TA= Popped.waiting_time+Popped.processing_time;
                    WTA= (double) TA/Popped.processing_time;
                    s_TA=s_TA + TA;
                    s_WTA=s_WTA + WTA;
                    WTA_arr[count]=WTA;
                    count++;
                    stop_now=getClk();

                    fprintf(f_log,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", stop_now, Popped.name, Popped.arrival_time, Popped.processing_time, 0, Popped.waiting_time, TA, WTA);

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
                    s_PT=s_PT+Popped.processing_time;
                    Popped.waiting_time=start_now-Popped.arrival_time;
                    if(process_name==1)first_start=Popped.arrival_time;
                    fprintf(f_log,"At time %d Process %d started arr %d total %d remain %d wait %d\n",start_now,process_name,Popped.arrival_time,Popped.processing_time,Popped.processing_time,Popped.waiting_time);

                    out=0;
                    to_finish=start_now+Popped.remaining_time;

                    while( out==0 && getClk() < to_finish ) {
                        sleep(2*to_finish-getClk()); //Terminated by [ TIME ] OR [ SIGUSR2 ]
                        recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
                        while(recieving_status != -1) {

                            push(p_queue, pg_message.data.remaining_time, pg_message.data);
                            if((Popped.remaining_time - (getClk()-start_now))>pg_message.data.remaining_time) out=1;
                            recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
                        }
                    }

                    if(out==1) {
                        kill(pid,SIGSTOP);
                        stop_now=getClk();
                        Popped.stop_time=stop_now;
                        Popped.remaining_time = Popped.remaining_time - (stop_now - start_now);
                        fprintf(f_log,"At time %d process %d stopped arr %d total %d remain %d wait %d\n", stop_now, Popped.name, Popped.arrival_time, Popped.processing_time, Popped.remaining_time, Popped.waiting_time);
                        push(p_queue, Popped.remaining_time, Popped);

                    } else {
                        pid_back= wait(&status);
                        if(!(status & 0x00FF))  printf("FINISHED PROCESS\n");

                        stop_now=getClk();
                        TA= Popped.waiting_time+Popped.processing_time;
                        WTA= (double) TA/Popped.processing_time;
                        s_TA=s_TA + TA;
                        s_WTA=s_WTA + WTA;
                        WTA_arr[count]=WTA;
                        count++;
                        stop_now=getClk();
                        fprintf(f_log,"At time %d process %d finished arr %d total %d remain %d wait %d TA %d WTA %.2f \n", stop_now, Popped.name, Popped.arrival_time, Popped.processing_time, 0, Popped.waiting_time, TA, WTA);
                    }


                }//Scheduler else
            }//Forking or Continue else
            ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        }//Process Existance else
    } //End of while loop
    raise(SIGINT);
    //destroyClk(true);
}

void run_process(int PS_ProcessingTime) {
    char pt_string[9]; //SOMETHING WEIRD HERE... less than 9 cause error
    sprintf(pt_string, "%d", PS_ProcessingTime); //convert to string
    char *argv[] = { "./process.out", pt_string, 0 };
    execve(argv[0], &argv[0], NULL); //start the new process
}
void sigusr2_handler(int signum) {
    // From the process generator
    // printf("SIGUSR2 RECIEVED..\n");
}
void sigusr1_handler(int signum) {
// Empty
//   printf("SIGUSR1 RECIEVED..\n");
}

void int_handler()
{
    double std_WTA=0;
    double avg_WAITING = (double)((s_TA-s_PT)/count);
    double avg_WTA = (double)s_WTA/count;
    double ut_time= (double) (s_PT*100)/(stop_now-first_start);
    fprintf(f_perf,"Utilization time =%.2f %%\n", ut_time);
    fprintf(f_perf,"Average WTA = %.2f\n",avg_WTA);
    fprintf(f_perf,"Average Waiting = %.2f\n",avg_WAITING);


    for(int i=0; i<count; i++) {
        std_WTA += pow((WTA_arr[i]-avg_WTA),2);
    }
    fprintf(f_perf,"std WTA = %f\n",sqrt(std_WTA/count));
    fclose(f_log);
    fclose(f_perf);
    exit(0);
}
