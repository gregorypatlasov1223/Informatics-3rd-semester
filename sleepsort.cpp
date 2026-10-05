#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int a[] = {4, 1, 15, 8, 2};
    int n = 5;

    for (int i = 0; i < n; i++) {
        pid_t pid = fork();

        if (pid == 0) {
            sleep(a[i]);
            printf("%d ", a[i]);
            exit(0);
        }
    }

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    printf("\n");

    return 0;
}
