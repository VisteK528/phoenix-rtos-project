# Sprawozdanie z drugiego zadania laboratoryjnego

Autor: Piotr Patek, 324 789

## Treść zadania - szeregowanie procesów

Instrukcje
Proszę zapoznać sie z prezentacja zamieszczoną na Teams, w filmach w kanale głównym przedmiotu SOI opisano szeregowanie procesów w RTOS Phoenix.



Na tej podstawie zrealizować samodzielnie projekt opisany na filmach, który służy zmodyfikowaniu domyślnego algorytmu szeregowania procesu.



Przygotować procedurę i/lub program (skrypty), które potwierdzą  prawidłowość działania algorytmu.



W kolejnym kroku należy zmodyfikować projekt z prezentacji o poniższe rozszerzenie.



Rozszerzenie:

W projekcie istnieją 3 grup procesów o różnych kwantach pracy, przydzielonych przez system.

Mamy grupę procesów:

1. typu A, które mają kwant czasu równy 1,

2. typu B, które mają kwant czasu równy 5

3. typu C, które mają kwant czasu równy 10.



Można uruchomić tyle samo (parametryzowane) procesów każdego typu (z podobnym sleepem) i ocenić czas wykonania procesów komendą ps. Akceptowanie jest inna metoda testowania prawidłowości działania projektu.

## Rozwiązanie
### Modyfikacja algorytmu szeregowania wątków

Podstawowy algorytm szeregowania zadań występujący w systemie operacyjnym PhoenixRTOS został zmodyfikowany w dwóch miejscach.

Po pierwsze przy warunku sprawdzajacym, czy należy się przełączyć na kolejny wątek czy zostawić obecny do wykonywania dodany został nowy predykat sprawdzajacy czy pole `currentQuanta` jest mniejsze lub równe zero. Pole to zostało równocześnie dodane do struktury `thread_t`, czyli deskryptora wątku.

Drugą zmianą jest ustawianie wartości nowego pola currentQuanta. Dla wątków systemowych bez deskryptora procesu ustawiany jest on na wartość 1, natomiast dla reszty procesów wartość tego pola to suma podstawowego `baseQuanta` oraz wartości `quanta`, która jest ustawiana przy powoływaniu do życia każdego nowego procesu.


Zmienna `currentQuanta` została dodana do deskryptora wątku zdefiniowanego w pliku `threads.h`

Zmienna `quanta` została dodana do deskryptora procesu zdefiniowanego w pliku `process.h`.

Wartość tej zmiennej jest inicjowana w funkcji `proc_start` wraz z innymi polami deskryptora procesu, która jest zdefiniowana w pliku `process.c `. Ponadto w tym pliku zadeklarowana jest zmienna globalna `baseQuanta`, której wartość ustawiana jest przy uruchomieniu systemu operacyjnego w funkcji `_process_init`.

#### Dodatkowe wywołania systemowe

W celu modyfikacji oraz pobierania wartości pól `baseQuanta` oraz `quanta` stworzone zostały 4 wywołania systemowe:
- `int proc_setBaseQuanta(int quanta)`
- `int proc_getBaseQuanta()`
- `int proc_setQuanta(int pid, int quanta)`
- `int proc_getQuanta(int pid)`

Ich deklaracje zostały dodane do pliku `process.h`, natomiast implementacja logiki została umieszczona w pliku `process.c`:

```
extern int proc_setBaseQuanta(int quanta);
extern int proc_getBaseQuanta();
extern int proc_setQuanta(int pid, int quanta);
extern int proc_getQuanta(int pid);
```

```
int proc_setBaseQuanta(int quanta){
    if(quanta < 1 || quanta > 100){
        return 0;
    }
    baseQuanta = quanta;
    return 1;
}

int proc_getBaseQuanta(){
    return baseQuanta;
}

int proc_setQuanta(int pid, int quanta){
    if(quanta < 0 || quanta > 100){
        return 0;
    }
    
    process_t* process = proc_find(pid);
    if(process == NULL){
        return 0;
    }
    process->quanta = quanta;
    proc_put(process);
    return 1;
}

int proc_getQuanta(int pid){
    int quanta = -1;
    process_t* process = proc_find(pid);
    if(process == NULL){
        return 0;
    }
    quanta = process->quanta;
    proc_put(process);
    return quanta;
}
```

Ostatecznie, aby wywołania systemowe były dostępne dla użytkownika systemu operacyjnego wspomniane wywołania systemowe zostały także zdefiniowane w plikach `phoenix-rtos-kernel/syscalls.c`, `phoenix-rtos-kernel/include/syscalls.h` oraz w bibliotece systemu `libphoenix/include/sys/scheduling.h`, tak samo jak w poprzednim zadaniu `max_children`:

`phoenix-rtos-kernel/syscalls.c`
```
...
int syscalls_seBaseQuanta(void* ustack){
    int quanta;
    GETFROMSTACK(ustack, int, quanta, 0);
    return proc_setBaseQuanta(quanta);
}

int syscalls_getBaseQuanta(void* ustack){
    return proc_getBaseQuanta();
}

int syscalls_setQuantaForProcess(void* ustack){
    pid_t pid;
    int quanta;
    GETFROMSTACK(ustack, pid_t, pid, 0);
    GETFROMSTACK(ustack, int, quanta, 1);
    return proc_setQuanta(pid, quanta);
}

int syscalls_getQuantaForProcess(void* ustack){
    pid_t pid;
    GETFROMSTACK(ustack, pid_t, pid, 0);
    return proc_getQuanta(pid);
}
...
```

`phoenix-rtos-kernel/include/syscalls.h`
```
...
ID(setBaseQuanta) \
ID(getBaseQuanta) \
ID(setQuantaForProcess) \
ID(getQuantaForProcess)
...
```

`libphoenix/include/sys/scheduling.h`
```
#ifndef PHOENIX_RTOS_PROJECT_SCHEDULING_H
#define PHOENIX_RTOS_PROJECT_SCHEDULING_H

extern int setBaseQuanta(int quanta);

extern int getBaseQuanta();

extern int setQuantaForProcess(int pid, int quanta);

extern int getQuantaForProcess(int pid);

#endif //PHOENIX_RTOS_PROJECT_SCHEDULING_H
```

### Aplikacja testująca
W ramach zadania należało także stworzyć aplikację testującą zmodyfikowany algorytm szeregowania. W tym celu stworzono aplikację, która tworzy procesy należące do 3 grup o różnym czasie dostępu do procesora:
- Grupa A - kwant 1
- Grupa B - kwant 5
- Grupa C - kwant 10

W tym miejscu należy zwrócić uwagę, że modyfikacja algorytmu szeregowania obejmowała dodanie także elementu `baseQuanta`. Przez to czas dostępu do procesora jest równy `baseQuanta` + `quanta`, które jest unikalne dla każdego procesu. Dlatego też, ostateczne kwanty dla procesów wynosiły:
- Grupa A - kwant 1 + baseQuanta (1) = 2
- Grupa B - kwant 5 + baseQuanta (1) = 6
- Grupa C - kwant 10 + baseQuanta (1) = 11

Dzięki takiemu podejściu mieliśmy też pewność, że procesy z grupy A będą miały wyższy priorytet niż inne podstawowe procesy i możliwe będzie przeprowadzenie badań.

Przy takich czasach dostępu stosunek GrupaC : GrupaB : GrupaA powinien wynosić 5.5 : 3 : 1.

#### Kod
```
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
```
#### Testy
Na początku uruchomiono program dla 1 procesu każdego typu
```
root@?:~ # /usr/bin/soi_lab02_spawn_procs 1
Main Parent PID = 31
Quanta for parent process with PID = 31         Quanta = 11
Quanta for process with PID = 32         Quanta = 1
Quanta for process with PID = 33         Quanta = 5
Quanta for process with PID = 34         Quanta = 10


=====================================================================================
                         Running Information                                        
=====================================================================================
Average CPU time for group with quanta 1 and PID 32 : 0.22 s
Average CPU time for group with quanta 5 and PID 33 : 0.34 s
Average CPU time for group with quanta 10 and PID 34 : 0.46 s
Estimated ratio of quanta 10:5:1 -> 5.500 : 3.000 : 1.000
Real ratio of quanta 10:5:1 -> 2.064 : 1.517 : 1.000

...

...
=====================================================================================
                         Running Information                                        
=====================================================================================
Average CPU time for group with quanta 1 and PID 32 : 53.24 s
Average CPU time for group with quanta 5 and PID 33 : 166.26 s
Average CPU time for group with quanta 10 and PID 34 : 294.12 s
Estimated ratio of quanta 10:5:1 -> 5.500 : 3.000 : 1.000
Real ratio of quanta 10:5:1 -> 5.524 : 3.123 : 1.000
```

Jak można zobaczyć po pewnym czasie stosunek czasu wykonania grup procesów ustabilizował się i jest podobny do oczekiwanego wyniku.


W następnym kroku uruchomiono po 2 procesy z każdej grupy. Tutaj tak samo po dłuższym czasie stosunek ten się ustabilizował.

```
=====================================================================================
                         Running Information                                        
=====================================================================================
Average CPU time for group with quanta 1 and PID 25 : 340.05 s
Average CPU time for group with quanta 5 and PID 26 : 987.22 s
Average CPU time for group with quanta 10 and PID 27 : 1808.62 s
Average CPU time for group with quanta 1 and PID 28 : 350.16 s
Average CPU time for group with quanta 5 and PID 29 : 1016.60 s
Average CPU time for group with quanta 10 and PID 30 : 1862.13 s
Estimated ratio of quanta 10:5:1 -> 5.500 : 3.000 : 1.000
Real ratio of quanta 10:5:1 -> 5.318 : 2.903 : 1.000
```








