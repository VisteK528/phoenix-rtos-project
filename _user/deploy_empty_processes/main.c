#include <stdlib.h>
#include <stdio.h>
#include <sys/max_children.h>

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Invalid number of arguments.\n");
        return 1;
    }

    const int numberOfChildren = atoi(argv[1]);
    printf("My PID: %d\n", (int)getpid());

    for(int i = 0; i < numberOfChildren; ++i){
        if(fork() == 0){
            sleep(5);
            exit(0);
        }
    }

    sleep(10);
    printf("Process has finished!\n");
    return 0;
}