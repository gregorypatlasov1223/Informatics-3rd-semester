#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    int fd[2];

    pipe(fd);

    struct timespec t1, t2;
    struct rusage usage;

    clock_gettime(CLOCK_MONOTONIC, &t1);

    pid_t pid = fork();

    if (pid == 0) {

        close(fd[0]); // Child only writes to pipe
        dup2(fd[1], STDOUT_FILENO); // Redirect stdout to pipe

        // close(fd[1]);

        execvp(argv[1], &argv[1]);

        perror("execvp");
        exit(1);
    }

    close(fd[1]); // Parent only reads from pipe

    char buffer[1024];

    long bytes = 0;
    long words = 0;
    long lines = 0;

    int inWord = 0;

    ssize_t bytesRead;

    while ((bytesRead = read(fd[0], buffer, sizeof(buffer))) > 0) {

        bytes += bytesRead;

        for (ssize_t i = 0; i < bytesRead; i++) {

            if (buffer[i] == '\n') {
                lines++;
            }

            if (isspace((unsigned char)buffer[i])) {
                inWord = 0;
            } else if (!inWord) {
                words++;
                inWord = 1;
            }
        }
    }

    close(fd[0]);
    wait4(pid, NULL, 0, &usage);
    clock_gettime(CLOCK_MONOTONIC, &t2);

    double realTime =
        (t2.tv_sec - t1.tv_sec) +
        (t2.tv_nsec - t1.tv_nsec) / 1000000000.0;

    double userTime =
        usage.ru_utime.tv_sec +
        usage.ru_utime.tv_usec / 1000000.0;

    double sysTime =
        usage.ru_stime.tv_sec +
        usage.ru_stime.tv_usec / 1000000.0;

    printf("bytes: %ld\n", bytes);
    printf("words: %ld\n", words);
    printf("lines: %ld\n", lines);

    printf("\nTime statistics:\n");
    printf("real: %.3f s\n", realTime);
    printf("user: %.3f s\n", userTime);
    printf("sys:  %.3f s\n", sysTime);

    return 0;
}
