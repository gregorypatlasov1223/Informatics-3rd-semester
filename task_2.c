#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s source destination\n", argv[0]);
        return 1;
    }

    int srcFd = open(argv[1], O_RDONLY);

    if (srcFd == -1) {
        perror("open source");
        return 1;
    }

    struct stat st;

    if (fstat(srcFd, &st) == -1) {
        perror("fstat");
        close(srcFd);
        return 1;
    }

    size_t size = st.st_size;

    int dstFd = open(argv[2], O_RDWR | O_CREAT | O_TRUNC, 0644);

    if (dstFd == -1) {
        perror("open destination");
        close(srcFd);
        return 1;
    }

    if (ftruncate(dstFd, size) == -1) {
        perror("ftruncate");
        close(srcFd);
        close(dstFd);
        return 1;
    }

    if (size == 0) {
        close(srcFd);
        close(dstFd);
        return 0;
    }

    void *src = mmap(NULL, size, PROT_READ, MAP_PRIVATE, srcFd, 0);

    if (src == MAP_FAILED) {
        perror("mmap source");
        close(srcFd);
        close(dstFd);
        return 1;
    }

    void *dst = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, dstFd, 0);

    if (dst == MAP_FAILED) {
        perror("mmap destination");
        munmap(src, size);
        close(srcFd);
        close(dstFd);
        return 1;
    }

    memcpy(dst, src, size);

    munmap(src, size);
    munmap(dst, size);

    close(srcFd);
    close(dstFd);

    return 0;
}
