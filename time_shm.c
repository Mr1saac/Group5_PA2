#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>

/* Person 1: shared-memory setup and cleanup. */
static struct timeval *create_shared_memory(void)
{
    char name[64];
    snprintf(name, sizeof(name), "/pa2_time_%ld", (long)getpid());

    int fd = shm_open(name, O_CREAT | O_EXCL | O_RDWR, 0600);
    if (fd == -1) {
        perror("shm_open");
        return NULL;
    }

    /* Remove the name now; the open descriptor keeps the object alive. */
    if (shm_unlink(name) == -1) {
        perror("shm_unlink");
        close(fd);
        return NULL;
    }

    if (ftruncate(fd, sizeof(struct timeval)) == -1) {
        perror("ftruncate");
        close(fd);
        return NULL;
    }

    struct timeval *start = mmap(NULL, sizeof(*start),
                                PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (start == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return NULL;
    }

    /* The mapping stays valid after close() and is inherited by fork(). */
    if (close(fd) == -1) {
        perror("close");
        munmap(start, sizeof(*start));
        return NULL;
    }

    return start;
}

static int cleanup_shared_memory(struct timeval *start)
{
    if (munmap(start, sizeof(*start)) == -1) {
        perror("munmap");
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

    struct timeval *start = create_shared_memory();
    if (start == NULL) {
        return EXIT_FAILURE;
    }

    /* Hoang Nguyen: fork, run the command in the child, and measure the elapsed time in the parent. */
    pid_t pid = fork();
 
    if (pid == -1) {
        perror("fork");
        cleanup_shared_memory(start);
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // Child: save the start time into shared memory.
        if (gettimeofday(start, NULL) == -1) {
            perror("gettimeofday");
            exit(EXIT_FAILURE);
        }

        // Run the command. If this returns, the command could not run. 
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
     // Parent: wait for the child, then save the end time.
    int status;
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        cleanup_shared_memory(start);
        return EXIT_FAILURE;
    }
        
    struct timeval end;
    if (gettimeofday(&end, NULL) == -1) {
        perror("gettimeofday");
        cleanup_shared_memory(start);
        return EXIT_FAILURE;
    } 


     /* Elapsed time = end - start (seconds and microseconds). */
    long seconds = end.tv_sec - start->tv_sec;
    long microseconds = end.tv_usec - start->tv_usec;
    double elapsed = seconds + microseconds / 1000000.0;
 
    printf("Elapsed time: %.6f seconds\n", elapsed);

    /*
    fprintf(stderr, "Part 1 scaffold: command execution and timing are not added yet.\n");
    */

    if (cleanup_shared_memory(start) == -1) {
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
