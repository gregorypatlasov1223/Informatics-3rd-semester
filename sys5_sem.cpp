#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>

int main() {
    int n = 0;
    printf("Enter n: ");
    scanf("%d", &n);

    int sem_id = semget(IPC_PRIVATE, 1, 0666);

    if (sem_id == -1) {
        perror("semget");
        return 1;
    }

    if (semctl(sem_id, 0, SETVAL, n) == -1) {
        perror("semctl");
        return 1;
    }

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            printf("PID %d ready\n", getpid());
            fflush(stdout);

            struct sembuf decrement = {0, -1, 0};
            semop(sem_id, &decrement, 1);

            struct sembuf waitZero = {0, 0, 0};
            semop(sem_id, &waitZero, 1);

            printf("PID %d run\n", getpid());
            return 0;
        }
    }

    for (int i = 0; i < n; ++i)
        wait(NULL);

    semctl(sem_id, 0, IPC_RMID);

    return 0;
}
