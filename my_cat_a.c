#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define BUF_SIZE 4096

int write_all(int fd, const char *buf, size_t count)
{
    size_t written = 0;
    while (written < count)
    {
        ssize_t n = write(fd, buf + written, count - written); // передвинули указатель и уменьшили кол-во читаемых символов
        if (n < 0)
        {
            if (errno == EINTR)
            {
                // вызов прерван сигналом — не критично, повторяем
                continue;
            }
            return -1;
        }
        written += (size_t)n;
    }
    return 0;
}

// копирует содержимое дескриптора in_fd в STDOUT_FILENO
static int copy_fd(int in_fd, const char *name)
{
    char buf[BUF_SIZE];
    ssize_t n;

    for (;;) {
        n = read(in_fd, buf, sizeof(buf));
        if (n < 0)
        {
            if (errno == EINTR) continue;

            fprintf(stderr, "my_cat: %s: %s\n", name, strerror(errno));
            return -1;
        }
        if (n == 0) break; // конец файла

        if (write_all(STDOUT_FILENO, buf, (size_t)n) < 0)
        {
            fprintf(stderr, "my_cat: write error: %s\n", strerror(errno));
            return -1;
        }
    }
    return 0;
}

int main(int argc, char *argv[])
{
    int exit_status = 0;

    if (argc == 1)
    {
        // без аргументов — работаем как stdin -> stdout
        if (copy_fd(STDIN_FILENO, "stdin") < 0) exit_status = 1;

        return exit_status;
    }

    for (int i = 1; i < argc; ++i)
    {
        int fd = open(argv[i], O_RDONLY);
        if (fd < 0)
        {
            fprintf(stderr, "my_cat: %s: %s\n", argv[i], strerror(errno));
            exit_status = 1;
            continue; // продолжаем со следующим файлом
        }

        if (copy_fd(fd, argv[i]) < 0) exit_status = 1;

        if (close(fd) < 0)
        {
            fprintf(stderr, "my_cat: %s: close: %s\n", argv[i], strerror(errno));
            exit_status = 1;
        }
    }

    return exit_status;
}
