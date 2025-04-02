#include <stdlib.h>
#include <stdio.h>
#include <sys/scheduling.h>

int main(int argc, char ** argv){
    if(argc != 3){
        printf("Invalid number of arguments.\n");
        return 1;
    }

    const int pid = atoi(argv[1]);
    const int quanta = atoi(argv[1]);

    setQuantaForProcess(pid, quanta);
    const int setQuanta = getQuantaForProcess(pid);

    printf("New quanta for process with PID %d = %d\n", pid, setQuanta);
    return 0;
}