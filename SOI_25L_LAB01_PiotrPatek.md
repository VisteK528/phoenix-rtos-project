# Sprawozdanie z pierwszego zadania laboratoryjnego

Autor: Piotr Patek, 324 789

## Treść zadania

Instrukcje
Stworzyć wywołanie systemowe zwracające informacje podane poniżej. Proszę opracować sposób sprawdzania działania stworzonego wywołania systemowego (napisać aplikację testującą np. tworzącą grupę procesów).

Zwracają Państwo pliki dodane lub zmodyfikowane w wzorcowej obrazie Phoenixa, oraz sprawozdanie w formacie PDF opisujące krótko zakres modyfikacji plików lub zaimplementowane nowe procedury.



Treści zadań:

Zwrócić PID procesów mającego najwięcej dzieci, zwracamy liczbę dzieci takich procesów. Zadanie jest rozbudowaną wersją projektu MaxChildren.
Zadanie jest podobne do projektu MaxChildren (rzekłbym nawet, że  identyczne) dlatego zmodyfikujmy to zadanie tak aby zwracało PID i liczbę dzieci dla procesów, które mają więcej niż N dzieci (gdzie N to parametr wywołania).

Uwaga:
Pierwszym krokiem powinno być inplementacja przykładu z MaxChildren

## Rozwiązanie

### Implementacja wywołania systemowego
W ramach rozwiązania zaimplementowano wywołanie systemowe `find_n_procs_children`, które przyjmuje dwa argumenty:
- własną strukturę `soi_list_of_processes_t`, która posiada nastepujące pola:
    - `pids` - tablica z PID'ami procesów
    - `numberOfChildren` - tablica z numerem dzieci procesu
    - `numberOfProcesses` - liczba procesów, które spełniły warunek i zostały zapisane do tablic
    - `arrayLength` - długość tablic pids oraz numberOfChildren 
```
typedef struct{
    pid_t* pids;
    int* numberOfChildren;
    size_t numberOfProcesses;
    size_t arrayLength;
} soi_list_of_processes_t;
```
- parametr N zdefiniowany w treści zadania

W pierwszym kroku dodana była deklaracja wywołania systemowego w pliku `libphoenix/include/sys/find_procs_n_children.h`:
```
#ifndef PHOENIX_RTOS_PROJECT_FIND_PROCS_N_CHILDREN_H
#define PHOENIX_RTOS_PROJECT_FIND_PROCS_N_CHILDREN_H

#include <sys/types.h>

extern void findProcsNChildren(soi_list_of_processes_t* list, int N);

#endif //PHOENIX_RTOS_PROJECT_FIND_PROCS_N_CHILDREN_H
```

Następnie zmiany dokonywane były już w folderze jądra systemu począwszy dodania następującej linijki:

```
...
ID(findProcsNChildren)
...
```
do makra znajdującego się w pliku `phoenix-rtos-kernel/include/syscalls.h`. Jest ono następnie wykorzystywane przy budowie obrazu systemu operacyjnego.

W kolejnym kroku zaimplementowano ciało wywołania systemowego w pliku `phoenix-rtos-kernel/syscalls.c`

```
...
void syscalls_findProcsNChildren(void* ustack){
    int N = 0;
    soi_list_of_processes_t* list;
    GETFROMSTACK(ustack, soi_list_of_processes_t*, list, 0);
    GETFROMSTACK(ustack, int, N, 1);
    return posix_findProcsNChildren(list, N);
}
...
```

Podobnie jak w przypadku przykładu maxChildren logikę wywołania systemowego przeniesiono do plików `posix.h` i `posix.c` w katalogu `phoenix-rtos-kernel/posix/` przez co uzyskano oddzielenie warstwy logiki od warstwy obsługi wywołania systemowego.

W pliku `posix.h` dodano deklarację:

```
...
extern void posix_findProcsNChildren(soi_list_of_processes_t* list, int N);
...
```

Natomiast w pliku `posix.c` zaimplementowano ciało funkcji:

```
void posix_findProcsNChildren(soi_list_of_processes_t* list, int N){
    process_info_t *process;

    // lock descriptors tree
    proc_lockSet(&posix_common.lock);

    // get first descriptor
    process = lib_treeof(process_info_t, linkage, lib_rbMinimum(posix_common.pid.root));

    int i = 0;
    while (process != NULL && i < list->arrayLength) { // descriptor exist
        int children = posix_countChildren(process);

        if(children > N) {
            list->pids[i] = process->process;
            list->numberOfChildren[i] = children;
            ++i;
        }
        
        process = lib_treeof(process_info_t, linkage, lib_rbNext(&process->linkage));
    }
    list->numberOfProcesses = i;

    // unlock descriptors tree
    proc_lockClear(&posix_common.lock);
}
```

Jak widać funkcja jest bardzo podobna do funkcji implementującej warstwę logiczną wywołania systemowego `maxChildren`. Użyto przy tym także funkcji `posix_countChildren` zaimplementowanej na potrzeby przykładu.

Na sam koniec należy także wspomnieć, że do plików `libphoenix/include/sys/types.h` oraz `phoenix-rtos-kernel/include/posix.h` dodano definicję struktury `soi_list_of_processes_t`, którą omówiono powyżej.

### Testy wywołania systemowego

Do testów wywołania systemowego stworzono dwa programy w części przeznaczonej dla użytkownika:;
- `deploy_empty_processes` - tworzy proces z podaną liczbą dzieci
- `find_n_procs_children` - używa wywołania systemowego `findNProcsChildren`

Kod programu `deploy_empty_processes`:
```
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char ** argv){
    if(argc != 2){
        printf("Invalid number of arguments.\n");
        return 1;
    }

    const int numberOfChildren = atoi(argv[1]);
    printf("My PID: %d\n", (int)getpid());

    for(int i = 0; i < numberOfChildren; ++i){
        if(fork() == 0){
            sleep(10);
            exit(0);
        }
    }

    sleep(10);
    printf("Process has finished!\n");
    return 0;
}
```

Kod programu `find_n_procs_children`:
```
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
```

W programie `find_n_procs_children` zakładane jest na początku, że lista procesów, które spełnią warunek jest mniejsza lub równa 20.
Następnie na tej podstawie alokowana jest pamięć na dwie tablice, a także uzupełniane są pola `arrayLength` oraz zerowane jest pole `numberOfProcesses`.

W kolejnym kroku wywoływane jest wywołanie procesowe `findProcsNChildren`, które wpisuje do tablicy numery PID oraz liczbę dzieci procesów, które posiadają więcej niż `N` dzieci, gdzie N to argument programu.
Ostatecznie na standardowe wyjście wypisywane są numery PID i liczby dzieci procesów spełniających warunek.

### Działanie programu

Przy testach najpierw uruchomiono program `deploy_empty_processes` z argumentem 10, a następnie sprawdzono wywołanie systemowe `findNProcsChildren`.

```bash
root@?:/usr/bin # ./find_n_procs_children 3
My PID: 32
Process 1 has 7 children which is greater than 3. 
Process 21 has 10 children which is greater than 3. 
root@?:/usr/bin #
```


