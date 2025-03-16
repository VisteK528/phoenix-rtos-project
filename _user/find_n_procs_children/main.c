#include <stdlib.h>
#include <stdio.h>
#include <sys/find_procs_n_children.h>

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Invalid number of arguments.\n");
        return 1;
    }

    const int N = atoi(argv[1]);
    printf("My PID: %d\n", (int)getpid());

    // Lets assume that max number of possible processes that statisfy the condition is 20
    const int maxProcesses = 20;
    soi_list_of_processes_t my_list;
    my_list.pids = (pid_t*) malloc(sizeof(pid_t) * maxProcesses);
    my_list.numberOfChildren = (int*)malloc(sizeof(int) * maxProcesses);
    my_list.arrayLength = maxProcesses;
    my_list.numberOfProcesses = 0;


    findProcsNChildren(&my_list, N);
    for(int i = 0; i < my_list.numberOfProcesses; ++i){
        printf("Process %d has %d children which is greater than %d. \n", (int)my_list.pids[i], my_list.numberOfChildren[i], N);
    }
    return 0;
}