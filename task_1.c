#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>
#include <ctype.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        return 1;
    }

    int fd_out[2];
    int fd_err[2];

    pipe(fd_out);
    pipe(fd_err);

    struct timespec t1, t2;
    struct rusage usage;

    clock_gettime(CLOCK_MONOTONIC, &t1);

    pid_t pid = fork();

    if (pid == 0) {

        close(fd_out[0]);
        close(fd_err[0]);

        dup2(fd_out[1], STDOUT_FILENO);
        dup2(fd_err[1], STDERR_FILENO);

        close(fd_out[1]);
        close(fd_err[1]);

        execvp(argv[1], &argv[1]);

        perror("execvp");
        exit(1);
    }

    close(fd_out[1]);
    close(fd_err[1]);

    char buffer[1024];

    long bytes = 0;
    long words = 0;
    long lines = 0;

    int inWordOut = 0;
    int inWordErr = 0;

    ssize_t bytesRead;

    while ((bytesRead = read(fd_out[0], buffer, sizeof(buffer))) > 0) {

        bytes += bytesRead;

        for (ssize_t i = 0; i < bytesRead; i++) {

            if (buffer[i] == '\n') {
                lines++;
            }

            if (isspace((unsigned char)buffer[i])) {
                inWordOut = 0;
            } else if (!inWordOut) {
                words++;
                inWordOut = 1;
            }
        }
    }

    close(fd_out[0]);

    while ((bytesRead = read(fd_err[0], buffer, sizeof(buffer))) > 0) {

        bytes += bytesRead;

        for (ssize_t i = 0; i < bytesRead; i++) {

            if (buffer[i] == '\n') {
                lines++;
            }

            if (isspace((unsigned char)buffer[i])) {
                inWordErr = 0;
            } else if (!inWordErr) {
                words++;
                inWordErr = 1;
            }
        }
    }

    close(fd_err[0]);

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

    printf("in stdout bytes: %ld\n", bytes);
    printf("in stdout words: %ld\n", words);
    printf("in stdout lines: %ld\n", lines);

    printf("\nTime statistics:\n");
    printf("real: %.3f s\n", realTime);
    printf("user: %.3f s\n", userTime);
    printf("sys:  %.3f s\n", sysTime);

    fprintf(stderr, "in stderr bytes: %ld\n", bytes);
    fprintf(stderr, "in stderr words: %ld\n", words);
    fprintf(stderr, "in stderr lines: %ld\n", lines);

    fprintf(stderr, "\nTime statistics:\n");
    fprintf(stderr, "real: %.3f s\n", realTime);
    fprintf(stderr, "user: %.3f s\n", userTime);
    fprintf(stderr, "sys:  %.3f s\n", sysTime);

    return 0;
}
