# Group 5 PA2

These programs run a command and show how long it takes. `time_shm.c` uses shared memory and `time_pipe.c` uses a pipe to pass the start time from the child to the parent.

To run the shared memory version on Linux:

```bash
gcc time_shm.c -o time -lrt
./time ls -l
```

To run the pipe version:

```bash
gcc time_pipe.c -o time
./time ls -l
```

You can also try `./time pwd` or `./time sleep 1`. The sleep command should show a time close to 1 second.

The test results are in `time_shm_output.txt` and `time_pipe_output.txt`. To save another test, use the matching filename after compiling that version:

```bash
./time ls -l | tee -a time_shm_output.txt
```

For the pipe version, use `time_pipe_output.txt` instead. The `-a` adds the output to the file without deleting the earlier tests.

Only the two `.c` files and the two output `.txt` files need to be submitted.
