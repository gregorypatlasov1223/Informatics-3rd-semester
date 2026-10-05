#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main(void)
{
    int fd[2];
    char buffer[100];

    pipe(fd);

    pid_t pid = fork();

    if (pid == 0) {
        close(fd[0]);

        char message[] = "hello";

        write(fd[1], message, strlen(message));

        close(fd[1]);
    } else {
        close(fd[1]);

        int bytesRead = read(fd[0], buffer, sizeof(buffer) - 1);

        if (bytesRead < 0) {
            perror("read");
            return 1;
        }

        if (bytesRead == 0) {
            printf("Pipe is empty\n");
            return 0;
        }

        buffer[bytesRead] = '\0';

        printf("Parent read: %s\n", buffer);

        close(fd[0]);
    }

    return 0;
}
