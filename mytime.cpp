#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>

int main(int argc, char *argv[])
{
    struct timeval t1, t2;

    gettimeofday(&t1, NULL);

    pid_t pid = fork();

    if (pid == 0) {
        execvp(argv[1], &argv[1]);
        exit(1);
    }

    wait(NULL);

    gettimeofday(&t2, NULL);

    double time =
        (t2.tv_sec - t1.tv_sec) +
        (t2.tv_usec - t1.tv_usec) / 1000000.0;

    printf("time: %.03lf s\n", time);

    return 0;
}
