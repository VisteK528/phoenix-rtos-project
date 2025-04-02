#include <stdlib.h>
#include <stdio.h>
#include <sys/scheduling.h>

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Invalid number of arguments.\n");
        return 1;
    }

    const int baseQuanta = atoi(argv[1]);
    setBaseQuanta(baseQuanta);
    const int setBaseQuanta = getBaseQuanta();

    printf("My PID: %d\n", (int)getpid());
    printf("New baseQuanta: %d\n", setBaseQuanta);
    return 0;
}