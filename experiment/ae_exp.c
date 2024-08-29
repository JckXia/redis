#include "../ae.h"
#include "stdio.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>


static void testFileHandler(aeEventLoop* el, int fd, void *privData, int mask) {
    printf("INovked! Doing some work (Current thread: %lu)\n", thrd_current());

    for (;;) {
        char buffer[1024];
        ssize_t bytes = read(fd, buffer, sizeof(buffer));
        if(bytes > 0 ) {
            printf("Data received on socket (fd: %d\n)",fd);
            printf("Data %s\n", buffer);
        } else {
            if(errno == EWOULDBLOCK) {
                printf("Done draining pipe\n");
                break;
            } else {
                printf("Somethings bad \n");
                printf("Errno %d\n", errno);
                return -1;
            }
        }
    }
}

static void testFinalizer(aeEventLoop * el, void * clientData) {
    printf("Finalzier! (Current thread %lu)\n", thrd_current());
    aeStop(el);
    aeDeleteEventLoop(el);
}


int main() {
    printf("Test program, current thread %lu\n", thrd_current());

    int fd = open("/home/jack/concurrency/redis/redis/jack-fifo", O_RDWR | O_NONBLOCK);
    if(fd == -1) 
    {
        printf("Can't open file, error %d\n", errno);
        exit(1);
    }
    printf("Opened file test.md, fd %d\n", fd);

    aeEventLoop *el = aeCreateEventLoop();
    aeCreateFileEvent(el, fd, AE_READABLE, testFileHandler, NULL, testFinalizer);
    printf("Event loop created!\n");
    aeMain(el);

}