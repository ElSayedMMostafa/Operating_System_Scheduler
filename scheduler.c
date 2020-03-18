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

void run_process(char *PS, int PS_ProcessingTime);
void sigchild_handler();


int main(int argc, char * argv[])
{
// Intialize the priority queue
  heap_t *p_queue = (heap_t *)calloc(1, sizeof (heap_t));
 signal(SIGCHLD,sigchild_handler);

  // For IPC
  key_t msgqid; int msgbuf_key=2550; struct msgbuff message; int recieving_status;
  msgqid = msgget(msgbuf_key,0644);

    initClk(); //to intialize the clock
  	


      // It'll be recieved from the IPC message [ Messege Queue ]

  // To recieve: (( THIS MUST BE INSIDE THE HANDLER OF SOME SIGNAL ))
    recieving_status = msgrcv(msgqid, &message, sizeof(message.data),    message.mtype, IPC_NOWAIT);

struct Process_data my_data = {'A', 3, 5, 12};
push(p_queue, my_data.priority, my_data); 
struct Process_data my_data2 = {'B', 4, 7, 12};
push(p_queue, my_data2.priority, my_data2); 

char Process_String[20]="./process$.out";

while(1){
printf("LOL, I'm %d\n",getpid());
printf("Start the loop now\n");
 //int next_st = peak_time(p_queue);
   struct Process_data Popped = pop(p_queue);
   printf("%c\n",Popped.name);
   Process_String[9]=Popped.name; //Now the process_string is ready to pass to execve
   printf("The next process to run: %s\n",Process_String);
	      if (Popped.name =='$') {
                   printf("NO NEW processes to be run\n");
		    raise(SIGSTOP);
                    }
                      else{
                           int pid=fork();   
		           if (pid == -1) perror("error in fork");    
		           else if (pid == 0) run_process(Process_String,Popped.processing_time);
   		            else {
	 	             //the parent
                               printf("I'm %d after else: \n",getpid());
	                       printf("I'll be stopped now & myPID= %d\n",getpid());
		               raise(SIGTSTP);
                     	       printf("CONTINUED..\n");
                    		 printf("I'm %d after continue: \n",getpid()); 
                     }
                        printf("I'm %d after else tany: \n",getpid()); 
    		    }
}
    destroyClk(true);
}


void sigchild_handler(){

int status;
  int pid = wait(&status);
	    if(!(status & 0x00FF)) //if exited normally
            printf("\nA child with pid %d terminated with exit code %d\n", pid, status>>8); 
   
}

void run_process(char *PS, int PS_ProcessingTime){
	char pt_string[10]; 
	printf("I'm %d & I'll start PROCESS ""%s"" now\n",getpid(),PS);
	sprintf(pt_string, "%d", PS_ProcessingTime); //convert to string
        char *argv[] = { PS, pt_string };
        execve(argv[0], &argv[0], NULL); //start the new process
        printf("That's my end %d\n",getpid());
}

