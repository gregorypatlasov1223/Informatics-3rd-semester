#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("Hello");
    fork();
    printf("world");

    return 0;
}
