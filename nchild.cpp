#include <stdio.h>
#include <unistd.h>

int main(void)
{
    int n;

    scanf("%d", &n);

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            printf("Child: PID = %d, PPID = %d\n", getpid(), getppid());
            return 0;
        }
    }

    printf("Parent: PID = %d, PPID = %d\n", getpid(), getppid());

    return 0;
}
