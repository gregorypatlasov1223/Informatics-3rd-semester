#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int n = 0;

    printf("Enter n: ");
    scanf("%d", &n);

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            write(STDOUT_FILENO, "D", 1);
            write(STDOUT_FILENO, "Hello", 5);
            write(STDOUT_FILENO, "\n", 1);

            return 0;
        }
    }

    for (int i = 0; i < n; ++i)
        wait(NULL);

    return 0;
}
