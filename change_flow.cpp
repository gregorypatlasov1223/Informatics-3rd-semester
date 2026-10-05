#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int fd[2];
    char buffer[100];

    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]); // Child writes to pipe
        dup2(fd[1], STDOUT_FILENO);

        close(fd[1]);

        printf("hello\n");

        return 0;
    }

    close(fd[1]); // Parent reads from pipe

    ssize_t bytesRead = read(fd[0], buffer, sizeof(buffer) - 1);

    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        printf("Parent got: %s", buffer);
    }

    close(fd[0]);

    wait(NULL);

    return 0;
}
