#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#define ITERATIONS 1000000

int main(void)
{
    int n = 0;

    printf("Enter n: ");
    scanf("%d", &n);

    int shmid = shmget(IPC_PRIVATE, sizeof(long long), 0666 | IPC_CREAT);

    if (shmid == -1) {
        perror("shmget");
        return 1;
    }

    void* memory = shmat(shmid, NULL, 0);
    volatile long long *count = memory;
    *count = 0;

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            return 1;
        }
        if (pid == 0) {
            for (int j = 0; j < ITERATIONS; ++j)
                (*count)++;

            shmdt((const void*)count);
            return 0;
        }
    }

    for (int i = 0; i < n; ++i)
        wait(NULL);

    printf("Expected: %lld\n", (long long)n * ITERATIONS);
    printf("Actual:   %lld\n", *count);

    shmdt((const void *)count);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
