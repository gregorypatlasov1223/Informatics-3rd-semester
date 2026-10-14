#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define BUF_SIZE 4096

int writeAll(int fd, const char *buffer, size_t count)
{
    size_t written = 0;

    while (written < count) {
        ssize_t n = write(fd, buffer + written, count - written);

        if (n < 0) {
            if (errno == EINTR) {
                continue;
            }

            return -1;
        }

        written += (size_t)n;
    }

    return 0;
}


int copyFd(int inputFd, int outputFd, const char *name)
{
    char buffer[BUF_SIZE];

    while (1) {
        ssize_t bytesRead = read(inputFd, buffer, sizeof(buffer));

        if (bytesRead < 0) {
            if (errno == EINTR) {
                continue;
            }

            fprintf(stderr, "pcat: %s: %s\n", name, strerror(errno));

            return -1;
        }

        if (bytesRead == 0) {
            break;
        }

        if (writeAll(outputFd, buffer, (size_t)bytesRead) < 0) {
            fprintf(stderr, "pcat: write error: %s\n", strerror(errno));

            return -1;
        }
    }

    return 0;
}

int readerProcess(int argc, char *argv[], int pipeWrite)
{
    int result = 0;

    if (argc == 1) {
        if (copyFd(STDIN_FILENO, pipeWrite, "stdin") < 0) {
            result = 1;
        }

        return result;
    }

    for (int i = 1; i < argc; i++) {
        int fd = open(argv[i], O_RDONLY);

        if (fd < 0) {
            fprintf(stderr, "pcat: %s: %s\n", argv[i], strerror(errno));

            result = 1;
            continue;
        }

        if (copyFd(fd, pipeWrite, argv[i]) < 0) {
            result = 1;
        }

        if (close(fd) < 0) {
            fprintf(stderr, "pcat: %s: close: %s\n", argv[i], strerror(errno));

            result = 1;
        }
    }

    return result;
}

int main(int argc, char *argv[])
{
    int pipeFd[2];

    if (pipe(pipeFd) < 0) {
        fprintf(stderr, "pcat: pipe: %s\n", strerror(errno));
        return 1;
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "pcat: fork: %s\n", strerror(errno));

        close(pipeFd[0]);
        close(pipeFd[1]);

        return 1;
    }

    // Ребёнок — reader
    if (pid == 0) {
        close(pipeFd[0]);

        int result = readerProcess(argc, argv, pipeFd[1]);

        close(pipeFd[1]);

        return result;
    }

    // Родитель — writer
    close(pipeFd[1]);

    int result = 0; // родительский процесс

    if (copyFd(pipeFd[0], STDOUT_FILENO, "pipe") < 0) {
        result = 1;
    }

    close(pipeFd[0]);

    int status;

    if (wait(&status) < 0) {
        fprintf(stderr, "pcat: wait: %s\n", strerror(errno));
        return 1;
    }

    if (status != 0) {
        result = 1;
    }

    return result;
}
