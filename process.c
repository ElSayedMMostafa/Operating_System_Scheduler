#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/wait.h>
#define STDIN_FILENO 0
#define STDOUT_FILENO 1

void Stop_handler();

int main(int agrc, char * argv[])
{
clock_t past_clk=0, current_clk=0;
double seconds_elapsed1=0,seconds_elapsed2=0;
int c = atoi(argv[1]); 
past_clk=clock();
    seconds_elapsed1=(double)past_clk/CLOCKS_PER_SEC;
    
    
    while (seconds_elapsed2 <  (seconds_elapsed1 + c))
    {
        current_clk=clock();
        seconds_elapsed2=(double)current_clk/CLOCKS_PER_SEC;
    }
exit(20);
}





