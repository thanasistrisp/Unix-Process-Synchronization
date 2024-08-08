# OS Project 1

## Compiling instructions

Just run `make` from the root directory of the project or from the source_code directory.

The executable will be created in the source_code directory with the name `program`.

## Running instructions

Just run `make run` from the root directory of the project or from the source_code directory.
The command line arguments are defined in Makefile of source_code directory.

Else, run `./program` from the source_code directory by following the below format:

```bash
./program <file> <lines per sequence> <N child processes> <requests>
```

## Structure of the project

- `source_code` directory contains the source code of the project.
- `include` directory contains the header files of the project.
- A demo file is provided in `files` directory.
- In `defines.h` file, some constants are defined as they are indicated from instructions.
- In `utilities.h` file, some utility functions are defined, like splitting a specific sequence to a char ** array so that the child can request a line by just indexing the array.
- In `workers.h` file, the worker functions are defined. There are 2 worker functions, one for the parent and one for the child.
They implement the readers-writers problem using semaphores.
- Log files are created in `Logs` directory. When the program reruns, the old log files are deleted and new ones are created, so no need to delete them manually.

## Description of the project

The struct `shared_seg` defined in `global_vars.h` is used to store the shared memory segment. It contains different variables that are used to store the information about the shared memory segment guarded by appropriate locks. We need shared memory to share variables through processes (a `new` operation is redundant as is allocates space space only in the heap of a certain process).

The problem is very similar to the writer-multiple readers problem, but inducted to a more complex scenario. In our case, we need at least as semaphores as the number of segments, plus some semaphores to communicate between child and parent for requesting a segment to main memory. 

The FIFO structure is implemented implicitly through the semaphore `rw_mutex`. The first who will manage to down it, it will be able to request a segment. The others will wait till there is no interest for this segment.

It will write to shared memory the id of the segment that it has requested. The parent will read this value and will be able to serve the request. The child is waiting for the parent to serve the request. The parent will up the semaphore `child` to notify the child that the request has been served. The child will down the semaphore `child` and will continue its execution of requests.

We need to delete `new` operations not only from parent but also from children as when they are forked they will get a copy of the parent's memory space to a different address. To avoid memory leaks before exiting child, we need to free this space.

Error messages are written to stderr.

Time operation are using `clock` function (cpu ticks) and we print the difference between the start and the end of the section requested.

Parent process will "stop listening from children" when all of them have exited. This is done by using a counter `child_count` that is incremented when a child is exited with the help of a boolean value to know when they exit. They are both protected by a semaphore `child_finished_sem`.

For having different randomness in each child, we need to seed the random generator with a different value. We use the pid of the child as a seed with time for having different results for each execution.
