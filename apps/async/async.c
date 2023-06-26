#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#define MAX_LEN 100

void input_handler(int num)
{
    char data[MAX_LEN];
    int len;

    len = read(STDIN_FILENO, &data, MAX_LEN);
    data[len] = 0;
    printf("input availabel: %s\n", data);
}

void main(int argc, char *argv[])
{
    int oflags;
#if 0
    signal(SIGIO, input_handler);
    fcntl(STDIN_FILENO, F_SETOWN, getpid());
    oflags = fcntl(STDIN_FILENO, F_GETFL);
    oflags &= ~FASYNC;
    fcntl(STDIN_FILENO, F_SETFL, oflags);
    printf("oflags = 0x%x, FASYNC = 0x%x\n", oflags, FASYNC);
#endif

    while (1);
}
