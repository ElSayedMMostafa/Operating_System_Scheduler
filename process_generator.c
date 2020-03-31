#include "headers.h"
#include "priority_queue.h"
#include <string.h>

void clearResources(int);
void read_process_file();

struct msgbuff
{
    long mtype;
    struct Process_data data;
};


int main(int argc, char *argv[])
{
 int scheduler_pid = atoi(argv[1]); //In case of SRTN
 //signal(SIGINT, clearResources);  
struct Process_data p_process;
   // Intialize the priority queue
    heap_t *p_queue = (heap_t *)calloc(1, sizeof(heap_t));
  // For IPC
 key_t msgqid; int msg_key = 12345; int sending_status;
 struct msgbuff pg_message; //Instance of the buffer (Sent by the process generator)
 pg_message.mtype=14; //tag "Can't be ZERO"
 msgqid = msgget(msg_key,IPC_CREAT|0644);
 if(msgqid == -1) perror("Error in establishing..");

   
    printf("process generator has started and now reading the process txt file");
    FILE *file_pointer;

    char *line;
    size_t lenght = 0;

    file_pointer = fopen("processes.txt", "r");

    if (file_pointer == NULL)
    {

        printf("Process generator : opening input processes txt file failure");

        exit(EXIT_FAILURE);
    }
    printf(" Process generator : input file has successfully been opened and now reading\n ");

    while ((getline(&line, &lenght, file_pointer)) != -1)
    {
        if (line[0] == '#')
        {

            printf(" Header line detected, process generator would skip the line\n");

            continue;
        }
        p_process.name = atoi(strtok(line, "\t"));
        p_process.arrival_time = atoi(strtok(NULL, "\t"));
        p_process.processing_time = atoi(strtok(NULL, "\t"));
        p_process.priority = atoi(strtok(NULL, "\t"));
        p_process.remaining_time = p_process.processing_time;
        p_process.waiting_time = 0;

        push(p_queue, p_process.arrival_time, p_process);
    }
	printf("Finished reading.\n");

    fclose(file_pointer);
// We read the file, NOW START Communication..
      initClk();
 int my_peak=peak_time(p_queue);

while(my_peak != -1){ //not empty queue
  if(my_peak == getClk()){
   pg_message.data=pop(p_queue);
  sending_status = msgsnd(msgqid,&pg_message,sizeof(pg_message.data),!IPC_NOWAIT);
  kill(scheduler_pid,SIGUSR2); //In case of SRTN ONLY
 if(sending_status == -1) printf("Sending Failed..\n"); //Just for monitoring
  printf("Process %d is sent\n",pg_message.data.name);
  my_peak=peak_time(p_queue);
  
  }
}
    destroyClk(true);
}

void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
}
