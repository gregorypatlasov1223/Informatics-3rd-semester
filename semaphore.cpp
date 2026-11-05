#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>

int main() {
    int n = 0;
    printf("Enter n: ");
    scanf("%d", &n);

    const char* sem_name = "/write_sem";
    sem_t* sem = sem_open(sem_name, O_CREAT | O_EXCL, 0600, 1);

    if (sem == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }

    for (int i = 0; i < n; ++i) {
        pid_t pid = fork();

        if (pid == 0) {
            sem_wait(sem);

            write(STDOUT_FILENO, "D", 1);
            write(STDOUT_FILENO, "Hello", 5);
            write(STDOUT_FILENO, "\n", 1);

            sem_post(sem);
            sem_close(sem);
            return 0;
        }
    }

    for (int i = 0; i < n; ++i) {
        wait(NULL);
    }

    sem_close(sem);
    sem_unlink(sem_name);

    return 0;
}
