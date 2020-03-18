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
int c = atoi(argv[1]); 
printf("Hello I'm Process A &my processing time = %d & PID=%d \n",c,getpid());

//HERE WHILE LOOP UNTIL THE TIME ENDS
 sleep(c); //replace 1 with c
//HERE WHILE LOOP UNTIL THE TIME ENDS
printf("I am FINISHED \n");
kill(getpgrp(),SIGCONT);
exit(2); //Sent to the scheduler
}



