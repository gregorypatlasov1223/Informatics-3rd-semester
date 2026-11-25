#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        return 1;
    }

    int fd_out[2];
    int fd_err[2];

    if (pipe(fd_out) == -1) {
        perror("pipe");
        return 1;
    }

    if (pipe(fd_err) == -1) {
        perror("pipe");
        close(fd_out[0]);
        close(fd_out[1]);
        return 1;
    }

    struct timespec t1, t2;
    struct rusage usage;

    clock_gettime(CLOCK_MONOTONIC, &t1);

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(fd_out[0]);
        close(fd_out[1]);
        close(fd_err[0]);
        close(fd_err[1]);
        return 1;
    }

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

    int flags = fcntl(fd_out[0], F_GETFL);
    fcntl(fd_out[0], F_SETFL, flags | O_NONBLOCK);

    flags = fcntl(fd_err[0], F_GETFL);
    fcntl(fd_err[0], F_SETFL, flags | O_NONBLOCK);

    char buffer[1024];

    long bytesOut = 0;
    long wordsOut = 0;
    long linesOut = 0;

    long bytesErr = 0;
    long wordsErr = 0;
    long linesErr = 0;

    int inWordOut = 0;
    int inWordErr = 0;

    int out_open = 1;
    int err_open = 1;

    ssize_t bytesRead;

    while (out_open || err_open) {

        if (out_open) {

            bytesRead = read(fd_out[0], buffer, sizeof(buffer));

            if (bytesRead > 0) {

                bytesOut += bytesRead;

                for (ssize_t i = 0; i < bytesRead; i++) {

                    if (buffer[i] == '\n') {
                        linesOut++;
                    }

                    if (isspace((unsigned char)buffer[i])) {
                        inWordOut = 0;
                    } else if (!inWordOut) {
                        wordsOut++;
                        inWordOut = 1;
                    }
                }
            }

            if (bytesRead == 0) {
                close(fd_out[0]);
                out_open = 0;
            }

            if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {

                perror("read stdout");
                close(fd_out[0]);
                out_open = 0;
            }
        }

        if (err_open) {

            bytesRead = read(fd_err[0], buffer, sizeof(buffer));

            if (bytesRead > 0) {

                bytesErr += bytesRead;

                for (ssize_t i = 0; i < bytesRead; i++) {

                    if (buffer[i] == '\n') {
                        linesErr++;
                    }

                    if (isspace((unsigned char)buffer[i])) {
                        inWordErr = 0;
                    } else if (!inWordErr) {
                        wordsErr++;
                        inWordErr = 1;
                    }
                }
            }

            if (bytesRead == 0) {
                close(fd_err[0]);
                err_open = 0;
            }

            if (bytesRead == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {

                perror("read stderr");
                close(fd_err[0]);
                err_open = 0;
            }
        }
    }

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

    printf("stdout bytes: %ld\n", bytesOut);
    printf("stdout words: %ld\n", wordsOut);
    printf("stdout lines: %ld\n", linesOut);

    printf("\nstderr bytes: %ld\n", bytesErr);
    printf("stderr words: %ld\n", wordsErr);
    printf("stderr lines: %ld\n", linesErr);

    printf("\nTime statistics:\n");
    printf("real: %.3f s\n", realTime);
    printf("user: %.3f s\n", userTime);
    printf("sys:  %.3f s\n", sysTime);

    return 0;
}
