#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/resource.h>

int main(int argc, char *argv[])
{
    struct timeval t1, t2;
    struct rusage usage;

    gettimeofday(&t1, NULL);
    // clock_gettime(CLOCK_MONOTONIC, &t2);

    pid_t pid = fork();

    if (pid == 0) {
        execvp(argv[1], &argv[1]);

        perror("execvp");
        exit(1);
    }

    wait(NULL);

    gettimeofday(&t2, NULL);
    // clock_gettime(CLOCK_MONOTONIC, &t2);

    getrusage(RUSAGE_CHILDREN, &usage);

    double realTime =
        (t2.tv_sec - t1.tv_sec) +
        (t2.tv_usec - t1.tv_usec) / 1000000.0;

    double userTime =
        usage.ru_utime.tv_sec +
        usage.ru_utime.tv_usec / 1000000.0;

    double sysTime =
        usage.ru_stime.tv_sec +
        usage.ru_stime.tv_usec / 1000000.0;

    printf("\nTime statistics:\n");
    printf("real: %.3f s\n", realTime);
    printf("user: %.3f s\n", userTime);
    printf("sys:  %.3f s\n", sysTime);

    return 0;
}


/* solution with wait4
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>

int main(int argc, char *argv[])
{
    struct timespec t1, t2;
    struct rusage usage;

    clock_gettime(CLOCK_MONOTONIC, &t1);

    pid_t pid = fork();

    if (pid == 0) {
        execvp(argv[1], &argv[1]);

        perror("execvp");
        exit(1);
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

    printf("\nTime statistics:\n");
    printf("real: %.3f s\n", realTime);
    printf("user: %.3f s\n", userTime);
    printf("sys:  %.3f s\n", sysTime);

    return 0;
}
*/
