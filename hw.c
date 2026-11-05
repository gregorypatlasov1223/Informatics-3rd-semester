#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <semaphore.h>

#define N 4
#define BLOCK_SIZE 4096

typedef struct {
    char buffer[N][BLOCK_SIZE];
    int size[N];

    int write_pos;
    int read_pos;

    sem_t full;
    sem_t empty;
    sem_t mutex;
} SharedData;


void producer(int fd, SharedData *data)
{
    while (1) {
        sem_wait(&data->full);
        sem_wait(&data->mutex);

        int pos = data->write_pos;

        int n = read(fd, data->buffer[pos], BLOCK_SIZE);
        data->size[pos] = n;

        data->write_pos = (data->write_pos + 1) % N;

        sem_post(&data->mutex);
        sem_post(&data->empty);

        if (n == 0)
            break;
    }
}


void consumer(int fd, SharedData *data)
{
    while (1) {
        sem_wait(&data->empty);
        sem_wait(&data->mutex);

        int pos = data->read_pos;
        int n = data->size[pos];

        if (n > 0)
            write(fd, data->buffer[pos], n);

        data->read_pos = (data->read_pos + 1) % N;

        sem_post(&data->mutex);
        sem_post(&data->full);

        if (n == 0)
            break;
    }
}


int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Usage: %s source destination\n", argv[0]);
        return 1;
    }

    int src = open(argv[1], O_RDONLY);

    if (src == -1) {
        perror("open source");
        return 1;
    }

    int dst = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (dst == -1) {
        perror("open destination");
        return 1;
    }

    SharedData *data = mmap(NULL, sizeof(SharedData), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    data->write_pos = 0;
    data->read_pos = 0;

    sem_init(&data->full, 1, N);
    sem_init(&data->empty, 1, 0);
    sem_init(&data->mutex, 1, 1);

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        return 1;
    }

    if (pid == 0) {
        close(dst);

        producer(src, data);

        close(src);
        return 0;
    }

    close(src);

    consumer(dst, data);

    wait(NULL);

    close(dst);

    sem_destroy(&data->full);
    sem_destroy(&data->empty);
    sem_destroy(&data->mutex);

    munmap(data, sizeof(SharedData));

    return 0;
}
