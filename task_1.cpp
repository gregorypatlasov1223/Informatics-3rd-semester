#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <signal.h>

const char *getSignalName(int signalNumber)
{
    switch (signalNumber) {

        case SIGKILL:
            return "SIGKILL";

        case SIGPIPE:
            return "SIGPIPE";

        case SIGTERM:
            return "SIGTERM";

        default:
            return "UNKNOWN";
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s command \n", argv[0]);
        return 1;
    }

    struct timeval t1, t2;

    gettimeofday(&t1, NULL);

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        execvp(argv[1], &argv[1]);

        perror("execvp");
        _exit(127);
    }

    int status;

    waitpid(pid, &status, 0);

    gettimeofday(&t2, NULL);

    double time =
        (t2.tv_sec - t1.tv_sec) +
        (t2.tv_usec - t1.tv_usec) / 1000000.0;

    printf("time: %.3lf s\n", time);

    if (WIFEXITED(status)) {
        int exitCode = WEXITSTATUS(status);

        printf("exited normally\n");
        printf("exit code: %d\n", exitCode);

        return exitCode;
    }

    if (WIFSIGNALED(status)) {
        int signalNumber = WTERMSIG(status);

        printf("terminated by signal: %s\n", getSignalName(signalNumber));
        printf("signal number: %d\n", signalNumber);

        return signalNumber;
    }

    return 0;
}
