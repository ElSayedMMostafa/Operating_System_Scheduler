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

void run_process(char *PS, int ppid, int PS_ProcessingTime){
	char pt_string[10]; char ppid_string[10];
	sprintf(pt_string, "%d", PS_ProcessingTime); //convert to string
        sprintf(ppid_string, "%d", ppid);            //convert to string
        char *argv[] = { PS, ppid_string, pt_string };
        execve(argv[0], &argv[0], NULL); //start the new process
}

void MyHandler(){
 int status;
 printf("HERE \n");
  int pid = wait(&status);
	    if(!(status & 0x00FF)) //if exited normally
            printf("\nA child with pid %d terminated with exit code %d\n", pid, status>>8);  
}
void handler(){
 printf("handler\n");
}
int main(int argc, char * argv[])
{
pid_t my_pid = getpid();
printf("%d\n",my_pid);
 //The handler done after each child-exit
 signal (SIGCHLD, MyHandler); signal (SIGINT, handler);
  // For IPC
  key_t msgqid; int msgbuf_key=2550; struct msgbuff message; int recieving_status;
  msgqid = msgget(msgbuf_key,0644);

    initClk(); //to intialize the clock
  // Intialize the priority queue
  heap_t *p_queue = (heap_t *)calloc(1, sizeof (heap_t));	

  
//This queue will contain the processes recieved from the process generator
   // Assume the processes are just data (number) inside an array recieved>>
      // It'll be recieved from the IPC message [ Messege Queue ]

  // To recieve: (( THIS MUST BE INSIDE THE HANDLER OF SOME SIGNAL ))
    recieving_status = msgrcv(msgqid, &message, sizeof(message.data),    message.mtype, IPC_NOWAIT);

struct Process_data my_data = {'A', 3, 5, 12};
push(p_queue, my_data.priority, my_data); 
struct Process_data my_data2 = {'B', 9, 15, 15};
push(p_queue, my_data2.priority, my_data2); 

  /*  for (int i = 0; i < 2; i++) {
        printf("%i\n", peak_time(p_queue)); 
        printf("%i\n", pop(p_queue).priority);
    }*/
     int next_st = peak_time(p_queue);
   struct Process_data Popped = pop(p_queue);
   char Process_String[20]="./process$.out";  
   Process_String[9]=Popped.name; //Now the process_string is ready to pass to execve
   printf("The next process to run: %s\n",Process_String);
	        int pid=fork();   
		printf("PID = %d\n",pid);
		if (pid == -1) perror("error in fork");    
		else if (pid == 0) run_process(Process_String,my_pid,Popped.processing_time);
   		else {
		//the parent
	    recieving_status = msgrcv(msgqid, &message, sizeof(message.data),    message.mtype, IPC_NOWAIT);
	}

   // destroyClk(true);
}


