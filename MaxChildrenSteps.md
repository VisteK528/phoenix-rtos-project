# Dodanie wywołania systemowego maxChildren kroki

W pierwszym kroku należało przejść do katalogu `libphoenix/include/sys/` i zdefiniować nasze wywołanie systemowe w pliku `max_children.h`:

```
#ifndef PHOENIX_RTOS_PROJECT_MAX_CHILDREN_H
#define PHOENIX_RTOS_PROJECT_MAX_CHILDREN_H

#include <sys/types.h>

extern int maxChildren(pid_t* whoMaxChildren);

#endif //PHOENIX_RTOS_PROJECT_MAX_CHILDREN_H
```

Następnie należało dodać tak zdefiniowane wywołanie systemowe w pliku `phoenix-rtos-kernel/include/syscalls.h`

```
...
ID(mprotect) \

// Our syscall
ID(maxChildren)
...

```

Makro zdefiniowane w tym pliku wykorzystywane jest w czasie budowania obrazu systemu w celu implementacji wywołań systemowych, które następnie stają się dostępne w przestrzeni użytkownika.

Kolejny krok to implementacja ciała wywołania systemowego w pliku `phoenix-rtos-kernel/syscalls.c`

```
int syscalls_maxChildren(void* ustack){
    pid_t* whoMaxChildren;
    GETFROMSTACK(ustack, pid_t*, whoMaxChildren, 0);
    *whoMaxChildren = 0;
    return posix_maxChildren(whoMaxChildren);
}
```

Kolejnym krokiem jest implementacja funkcji posix_maxChildren w pliku `phoenix-rtos-kernel/posix/posix.h`