#include <stdlib.h>
#include <stdio.h>
#include <sys/scheduling.h>

int main(int argc, char ** argv){

    int baseQuanta = getBaseQuanta();

    printf("My PID: %d\n", (int)getpid());
    printf("Base quanta: %d\n", baseQuanta);
    return 0;
}