#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
    pid_t childPid = fork();

    if (childPid == 0) {
        printf("Child:  PID = %d, PPID = %d\n", getpid(), getppid());
    } else {
        printf("Parent: PID = %d, PPID = %d\n", getpid(), getppid());
    }

    return 0;
}
