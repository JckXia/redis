#include "../ae.h"
#include "stdio.h"
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

/**
 *  Imagine we have an fd describing an FIFO pipe
 *  
 *  Writer ->  PIPE  <- READER
 *  
 *  Scenario 1:
 *      -> Writer writes some data to pipe
 *      -> Blocks until an reader reads from pipe
 *      -> ae/select notifes that some fd socket is avail for read (AF_READABLE)
 *      -> reader (this app) reads data
 *      -> both process exits
 * 
 *  Scenario 2:
 *      -> Reader read from pipe 
 *      -> Blocks (since no data)
 *      -> ae/select notifies that some fd data is avail for write (AF_WRITABLE)
 *      -> Writer (this app) writes some data to pipe
 *      -> both process exits
 *
 */

static void testReadHandler(aeEventLoop* el, int fd, void *privData, int mask) {
    printf("Reader handler invoked! Doing some work (Current thread: %lu)\n", thrd_current());

    int s = (int)privData;
    printf("Client data %d\n", s);

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

static void testWriteHandler(aeEventLoop* el, int fd, void* privData, int mask) {
    int s = (int)privData;

    printf("Write Handler invoked \n");
    char buffer[20] = "Hello, World!\n";
    write(fd, buffer, sizeof(buffer));

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
    int clientData = 42;

    aeCreateFileEvent(el, fd, AE_READABLE, testReadHandler, (void *)clientData, testFinalizer);
    // aeCreateFileEvent(el, fd, AE_WRITABLE, testWriteHandler, NULL, testFinalizer);
    printf("Event loop created!\n");
    aeMain(el);

}