#include "headers.h"
#include "priority_queue.h"
#include <string.h>
#define MAX_SIZE 20
void clearResources(int);
int Get_user_desired_algorithm();
void ExecuteClock();
int ExecuteScheduler(int algo);


struct msgbuff
{
    long mtype;
    struct Process_data data;
};

int msg_key1 = 12345;
int msg_key2 = 12346;

int main(int argc, char *argv[])
{
    printf("PG PID=%d\n",getpid());
    int status;
    int pid1,pid2;
    signal(SIGINT, clearResources);
    struct Process_data p_process;
    // Intialize the priority queue
    heap_t *p_queue = (heap_t *)calloc(1, sizeof(heap_t));
    // For IPC
    int sending_status;
    key_t msgqid1,msgqid2;
    struct msgbuff pg_message; //Instance of the buffer (Sent by the process generator)
    pg_message.mtype=14; //tag "Can't be ZERO"
    msgqid1 = msgget(msg_key1,IPC_CREAT|0644);
    msgqid2 = msgget(msg_key2,IPC_CREAT|0644);
    if(msgqid1 == -1 || msgqid2 == -1) perror("Error in establishing..");


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
    int user_desired_algorithm= Get_user_desired_algorithm();
// We read the file, NOW START FORKING
    pid1 = fork();
    if(pid1==0) ExecuteClock();
    else {
        pid2 = fork();
        if(pid2==0) ExecuteScheduler(user_desired_algorithm);
        else {
            initClk();
            int my_peak=peak_time(p_queue);
            while(my_peak != -1) { //not empty queue
                if(my_peak == getClk()) {
                    pg_message.data=pop(p_queue);
                    sending_status = msgsnd(msgqid1,&pg_message,sizeof(pg_message.data),!IPC_NOWAIT);
                    if(user_desired_algorithm == 2) kill(pid2,SIGUSR2); //In case of SRTN ONLY
                    if(sending_status == -1) printf("Sending Failed..\n"); //Just for monitoring
                    printf("Process %d is sent\n",pg_message.data.name);
                    my_peak=peak_time(p_queue);

                }
            }
        }
    }
    msgsnd(msgqid2,&pg_message,sizeof(pg_message.data),!IPC_NOWAIT);
    wait(&status);
    destroyClk(true);
}


void clearResources(int signum)
{
    //TODO Clears all resources in case of interruption
    int x = msgget(msg_key1,0644);
    msgctl(x, IPC_RMID, (struct msqid_ds *) 0);
    int y = msgget(msg_key2,0644);
    msgctl(y, IPC_RMID, (struct msqid_ds *) 0);
}
int Get_user_desired_algorithm() {
    printf ("Please enter: \n 1 for HPF \n 2 for SRTN \n 3 for RR \n");
    printf ("Your Choice: ");
    int x; // which represents user's choice
    scanf("%d",&x) ;
    return x;
}

int ExecuteScheduler(int algo) {
    char file_str[MAX_SIZE];
    if(algo==1) strcpy(file_str,"./hpf.out");

    else if(algo==2) strcpy(file_str,"./srtn.out");
    else if(algo==3) strcpy(file_str,"./rr.out");
    char* argv[]= {file_str,0};
    execve(argv[0],&argv[0],NULL);

}

void ExecuteClock() {
    char* argv[]= {"./clk.out",0};
    execve(argv[0],&argv[0],NULL);
}
