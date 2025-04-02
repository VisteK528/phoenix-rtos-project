#include <stdlib.h>
#include <stdio.h>
#include <sys/scheduling.h>

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Invalid number of arguments.\n");
        return 1;
    }

    const int pid = atoi(argv[1]);
    const int quanta = getQuantaForProcess(pid);

    printf("New quanta for process with PID %d = %d\n", pid, quanta);
    return 0;
}