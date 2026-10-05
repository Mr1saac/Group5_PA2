# Group 5 — Programming Assignment 2

This project measures how long a command takes to run. Both C programs use `fork()`, `execvp()`, and `gettimeofday()`. The child records the start time, and the parent waits for it to finish before recording the end time.

- `time_shm.c` shares the start time through shared memory.
- `time_pipe.c` sends the start time through a pipe.
- The two output text files contain test results.

## Compile and run

Run these commands in a Linux/Unix terminal from the project folder. The commands below are for Linux. Compile one version at a time as `time`.

Shared-memory version:

```bash
gcc -Wall -Wextra time_shm.c -o time -lrt
./time ls -l
./time sleep 1
```

Pipe version:

```bash
gcc -Wall -Wextra time_pipe.c -o time
./time ls -l
./time sleep 1
```

The program prints the command's output followed by the elapsed time in seconds. `sleep 1` should take about one second.

## Save test output

These commands replace the existing output files with a fresh run of each test set:

```bash
gcc time_shm.c -o time -lrt
{ ./time ls; ./time ls -l; ./time pwd; ./time sleep 1; } 2>&1 | tee time_shm_output.txt

gcc time_pipe.c -o time
{ ./time ls; ./time ls -l; ./time pwd; ./time sleep 1; } 2>&1 | tee time_pipe_output.txt
```

## Work split

1. Shared-memory setup and cleanup.
2. Shared-memory process creation and timing.
3. Pipe setup and timestamp transfer.
4. Pipe process creation and timing.
5. Testing and collecting output.

## Submission

Submit `time_shm.c`, `time_shm_output.txt`, `time_pipe.c`, and `time_pipe_output.txt`. This README is just for the GitHub repo.
