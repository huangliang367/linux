#include <stdio.h>
#include <sys/select.h>
/* According to earlier standards */
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "scull.h"

int main(int argc, char *argv[])
{
    int fd = -1;

    fd = open("/dev/scull0", O_RDWR);
    if (fd < 0) {
        printf("Open %s failed.\n", "/dev/scull0");
        return -1;
    }

    ioctl(fd, SCULL_IOCRESET);

    printf("Bye\n");
    return 0;
}
