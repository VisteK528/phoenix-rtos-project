#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/threads.h>

volatile sig_atomic_t stop = 0;


int quanta_table[3] = {1, 5, 10};

void handle_signal(int sig) {
    stop = 1;
}

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Invalid number of arguments.\n");
        return 1;
    }
    signal(SIGINT, handle_signal);

    const int proc_pid = getpid();
    printf("Main Parent PID = %d\n", proc_pid);
    setQuantaForProcess(proc_pid, 11);
    printf("Quanta for parent process with PID = %d\t Quanta = %d\n", proc_pid, getQuantaForProcess(proc_pid));


    const int number_of_procs = atoi(argv[1]);
    pid_t pids[number_of_procs*3];

    for(int i = 0; i < number_of_procs*3; ++i){
        pid_t leader = fork();

        if(leader == 0){
            const int quanta = quanta_table[i % 3];
            const int proc_pid = getpid();
            setQuantaForProcess(proc_pid, quanta);
            printf("Quanta for process with PID = %d\t Quanta = %d\n", proc_pid, getQuantaForProcess(proc_pid));

            while(!stop){
                asm volatile("nop");
            }
            return;
        }
        else{
            pids[i] = leader;
        }
    }

    int n = 32*4;
    threadinfo_t *info = malloc(n * sizeof(threadinfo_t));
    if (!info) {
        perror("malloc failed");
        return 1;
    }

    float processes_time[3] = {0.f, 0.f, 0.f};


    while (!stop) {
        printf("\n\n=====================================================================================\n");
        printf("                         Running Information                                        \n");
        printf("=====================================================================================\n");

        for(int i = 0; i < number_of_procs*3; ++i){
            pid_t current_pid = pids[i];

            int tcnt = threadsinfo(n, info);
            for (int j = 0; j < tcnt; ++j) {
                //printf("PID: %d\n", info[j].pid);
                if (info[j].pid == current_pid) {
                    processes_time[i % 3] += (float)(info[j].cpuTime + 500000.f) / 1000000.f; //seconds
                }
            }
            printf("Average CPU time for group with quanta %d and PID %d : %.2f s\n", quanta_table[i % 3], current_pid, processes_time[i % 3] / 3);
        }
        const float ratio10to1 = processes_time[2] / processes_time[0];
        const float ratio5to1 = processes_time[1] / processes_time[0];
        const float ratio1to1 = processes_time[0] / processes_time[0];
        printf("\nEstimated ratio of quanta 10:5:1 -> %.3f : %.3f : %.3f\n", 5.5f, 3.f, 1.f);
        printf("Real ratio of quanta 10:5:1 -> %.3f : %.3f : %.3f\n", ratio10to1, ratio5to1, ratio1to1);
        sleep(10);
    }

    free(info);

    for(int i = 0; i < number_of_procs*3; ++i){
        kill(pids[i], SIGKILL);
    }
    printf("Killed all child processes!\n");

    return 0;
}