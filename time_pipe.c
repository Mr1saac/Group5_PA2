#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/wait.h>
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

    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        struct timeval start;

        if (gettimeofday(&start, NULL) == -1) {
            perror("gettimeofday");
            close(pipefd[0]);
            close(pipefd[1]);
            _exit(EXIT_FAILURE);
        }

        if (write_start_time(pipefd, &start) == -1) {
            _exit(EXIT_FAILURE);
        }
        
        execvp(argv[1], &argv[1]);

        perror("execvp");
        _exit(EXIT_FAILURE);
    }
    /* Parent: close the unused writer, wait, then record the end time. */
    if (close(pipefd[1]) == -1) {
        perror("close");
        close(pipefd[0]);
        while (waitpid(pid, NULL, 0) == -1 && errno == EINTR) {}
        return EXIT_FAILURE;
    }

    int status;
    pid_t waited;
    do {
        waited = waitpid(pid, &status, 0);
    } while (waited == -1 && errno == EINTR);

    if (waited == -1) {
        perror("waitpid");
        close(pipefd[0]);
        return EXIT_FAILURE;
    }

    struct timeval end;
    if (gettimeofday(&end, NULL) == -1) {
        perror("gettimeofday");
        close(pipefd[0]);
        return EXIT_FAILURE;
    }

    struct timeval start;
    if (read_start_time(pipefd, &start) == -1) {
        return EXIT_FAILURE;
    }

    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;

    double elapsed = seconds + microseconds / 1000000.0;

    printf("Elapsed time: %.6f seconds\n", elapsed);

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }
    if (WIFSIGNALED(status)) {
        return 128 + WTERMSIG(status);
    }
    return EXIT_FAILURE;
}
