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

int main(int argc, char * argv[])
{
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

// Uncomment the next 2 lines when the IPC part is ready..
  //recieving_status = msgrcv(msgqid, &message, sizeof(pg_message.data),pg_message.mtype,!IPC_NOWAIT);
  //push(p_queue, pg_message.data.priority, pg_message.data); 

//Test Data [ Untill we finish the IPC Part ]
struct Process_data my_data = {'A', 3, 5, 12}; //SOMETHING WEIRD HERE IN THE NAMING.
push(p_queue, my_data.priority, my_data); 
struct Process_data my_data2 = {'B', 4, 6, 12};
push(p_queue, my_data2.priority, my_data2); 
struct Process_data my_data3 = {'C', 6, 7, 12};
push(p_queue, my_data3.priority, my_data3);

// The main functionality
while(1){
printf("====SCHEDULER PID==== %d\n",getpid());
process_name='$';
// Uncomment the next 2 lines when the IPC part is ready.. [ Continuous Recieving ]
  //recieving_status = msgrcv(msgqid, &message, sizeof(pg_message.data),pg_message.mtype,IPC_NOWAIT);
  //push(p_queue, pg_message.data.priority, pg_message.data); 
Popped = pop(p_queue);
process_name=Popped.name; //Now the process_string is ready to pass to execve
printf("The next process to run: %c\n",process_name);
  if (process_name=='$') {
                   printf("NO NEW processes to be run --> Gonna wait\n");  
	           recieving_status = msgrcv(msgqid, &pg_message, sizeof(pg_message.data),pg_message.mtype,!IPC_NOWAIT);
		printf("%d\n",recieving_status); sleep(5);
                   push(p_queue, pg_message.data.priority, pg_message.data); 
                    }
  else{
                   int pid=fork();   
		   if (pid == -1) perror("error in fork");    
		   else if (pid == 0) run_process(Popped.processing_time);
   		   else {  //the scheduler
		      pid_back= wait(&status);
			if(!(status & 0x00FF)) //if exited normally
 			 printf("Process %c with pid %d terminated with exit code %d\n", process_name, pid_back, status>>8);
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


