
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

int main(int argc, char * argv[]){
// For IPC
 key_t msgqid; int msg_key = 12500; int recieving_status;
 struct msgbuff pg_message; //Instance of the buffer (Sent by the process generator)
 msgqid = msgget(msg_key,IPC_CREAT|0644);
 if(msgqid == -1) perror("Error in intializing");
 
//The message
  struct Process_data my_Data = {'S', 1, 7, 10, 3};
   // OR like
 /* struct Process_data my_Data;
  my_Data.name='R';my_Data.processing_time=12;
  my_Data.arrival_time=2; my_Data.starting_time=3;
  my_Data.priority=1; */

  pg_message.mtype=getpid(); //Some sychronization..
  pg_message.data = my_Data;
 
  int sending_status = msgsnd(msgqid,&pg_message,sizeof(pg_message.data),!IPC_NOWAIT);
 while(1);
}
