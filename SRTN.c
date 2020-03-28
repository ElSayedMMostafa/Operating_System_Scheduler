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

void run_process(int PS_ProcessingTime);
void sig_handler(); int pid;

int main(int argc, char * argv[])
{
signal(SIGUSR1,sig_handler);
//Variables
struct Process_data Popped;
char process_name;
int status,pid_back;
// For IPC
 key_t msgqid; int msg_key = 12500; int recieving_status;
 struct msgbuff pg_message; //Instance of the buffer (Sent by the process generator)
 pg_message.mtype=0; //to recieve on all the tags.
 msgqid = msgget(msg_key,0644);
 if(msgqid == -1) perror("Error in recieving, No msg_queue exits");
    
// Intialize the priority queue
  heap_t *p_queue = (heap_t *)calloc(1, sizeof (heap_t));

//To intialize the clock
initClk(); 

//Test Data [ Untill we finish the IPC Part ]
struct Process_data my_data = {'A', 3, 5, 12, 1}; //SOMETHING WEIRD HERE IN THE NAMING.
push(p_queue, my_data.remaining_time, my_data); 
struct Process_data my_data2 = {'B', 4, 6, 12, 4};
push(p_queue, my_data2.remaining_time, my_data2); 
struct Process_data my_data3 = {'C', 6, 7, 12, 6};
push(p_queue, my_data3.remaining_time, my_data3);

while(getClk()<1); //Start form clock=1;
// The main functionality
while(1){
printf("====SCHEDULER PID==== %d\n",getpid());
process_name='$';
  // [ Continuous Recieving ]
  recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
  if(recieving_status != -1) push(p_queue, pg_message.data.priority, pg_message.data); 
 // [ Pop a process ]
 Popped = pop(p_queue);
 process_name=Popped.name; //Now the process_string is ready to pass to execve
 printf("The next process to run: %c\n",process_name);
  if (process_name=='$') {
                   printf("NO NEW processes to be run --> Gonna wait\n");  
	           recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,!IPC_NOWAIT);
                   push(p_queue, pg_message.data.priority, pg_message.data); 
                    }
  else{
                   pid=fork();   
		   if (pid == -1) perror("error in fork");    
		   else if (pid == 0) run_process(Popped.processing_time);
   		   else {  //the scheduler
                   int now=getClk();
		  printf("At time %d Process %c started arr %d total %d remain %d wait %d\n",now,process_name,Popped.arrival_time,Popped.processing_time,Popped.processing_time,now-Popped.arrival_time); //Move this line to OUTPUT_FILE
		   sleep(1);
                        pid_back= wait(&status);
			if(!(status & 0x00FF)) //if exited normally
 			 printf("Process %c with pid %d finished with exit code %d\n", process_name, pid_back, status>>8);
			else {//The process is terminated
		int termination_time = getClk();
  printf("Process %c with pid %d terminated at %d\n", process_name, pid_back, termination_time);
  Popped.processing_time=termination_time-now;
  sleep(1);
   printf("Process %c Remainaing_time = %d\n",Popped.name,Popped.processing_time);
   push(p_queue, Popped.processing_time, Popped);
   
//recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,!IPC_NOWAIT);
//push(p_queue, pg_message.data.remaining_time, pg_message.data);

			}
                        }
     }
} //End of while loop
    destroyClk(true);
}

void run_process(int PS_ProcessingTime){
	char pt_string[9]; //SOMETHING WEIRD HERE... less than 9 cause error
        sprintf(pt_string, "%d", PS_ProcessingTime); //convert to string
        char *argv[] = { "./process.out", pt_string };
        execve(argv[0], &argv[0], NULL); //start the new process
}
void sig_handler(){
 kill(pid,SIGINT);
}
