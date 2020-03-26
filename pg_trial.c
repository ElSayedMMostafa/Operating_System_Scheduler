
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
 while(1);
}
