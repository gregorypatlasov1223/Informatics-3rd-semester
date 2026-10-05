#include <errno.h>
#include <stdio.h>
#include <string.h>

#define BUF_SIZE 4096

static int copy_stream(FILE *in)
{
    char buf[BUF_SIZE];
    size_t n;

    while ((n = fread(buf, 1, sizeof(buf), in)) > 0)
    {
        size_t written = fwrite(buf, 1, n, stdout);
        if (written < n)
        {
            fprintf(stderr, "my_cat: write error: %s\n", strerror(errno));
            return -1;
        }
    }

    if (ferror(in))
    {
        fprintf(stderr, "my_cat: %s\n", strerror(errno));
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[])
{
    int exit_status = 0;

    if (argc == 1)
    {
        if (copy_stream(stdin) < 0) exit_status = 1;

        fflush(stdout);
        return exit_status;
    }

    for (int i = 1; i < argc; ++i)
    {
        FILE *f = fopen(argv[i], "r");
        if (f == NULL)
        {
            fprintf(stderr, "my_cat: %s: %s\n", argv[i], strerror(errno));
            exit_status = 1;
            continue;
        }

        if (copy_stream(f) < 0) exit_status = 1;

        if (fclose(f) != 0)
        {
            fprintf(stderr, "my_cat: %s: close: %s\n", argv[i], strerror(errno));
            exit_status = 1;
        }
    }

    fflush(stdout);
    return exit_status;
}


