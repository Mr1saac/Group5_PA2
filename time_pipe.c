#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>



static int create_pipe(int pipefd[2])
{
    if (pipe(pipefd) == -1) {
        perror("pipe");
        return -1;
    }

    return 0;
}

static int write_start_time(int pipefd[2], struct timeval *start)
{
    if (close(pipefd[0]) == -1) {
        perror("close");
        return -1;
    }

    ssize_t bytes_written =
        write(pipefd[1], start, sizeof(*start));

    if (bytes_written == -1) {
        perror("write");
        close(pipefd[1]);
        return -1;
    }

    if (bytes_written != sizeof(*start)) {
        fprintf(stderr, "Error: incomplete pipe write\n");
        close(pipefd[1]);
        return -1;
    }

    /* Child is finished writing to the pipe. */
    if (close(pipefd[1]) == -1) {
        perror("close");
        return -1;
    }

    return 0;
}

static int read_start_time(int pipefd[2], struct timeval *start)
{
    if (close(pipefd[1]) == -1) {
        perror("close");
        close(pipefd[0]);
        return -1;
    }

    ssize_t bytes_read =
        read(pipefd[0], start, sizeof(*start));

    if (bytes_read == -1) {
        perror("read");
        close(pipefd[0]);
        return -1;
    }

    if (bytes_read != sizeof(*start)) {
        fprintf(stderr, "Error: incomplete pipe read\n");
        close(pipefd[0]);
        return -1;
    }

    if (close(pipefd[0]) == -1) {
        perror("close");
        return -1;
    }

    return 0;
}


int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <command> [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    int pipefd[2];

    if (create_pipe(pipefd) == -1) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}