#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1
int main(int agrc, char * argv[])
{
printf("Hello I'm Process A & I'm running\n");
pid_t parent_pid = atoi(argv[1]); //convert to integer
int c = atoi(argv[2]); 
printf("my processing time = %d \n",c);
printf("my ppid            = %d \n",parent_pid);

//HERE WHILE LOOP UNTIL THE TIME ENDS
 sleep(1); //replace 1 with c
//HERE WHILE LOOP UNTIL THE TIME ENDS
printf("I am FINISHED \n");
 //exit(4); //Is not sent or recieved
  kill(parent_pid,SIGINT);
}


