# MapReduce

## Motivation/About

With the rise of multi-core CPUs and companies like ARM producing low cost processing power, distributed algorithms that can utilize parallelization to its fullest extent are more valuable than ever. Google's MapReduce provides a framework for the distribution of algorithms, handling distribution and logistics of parallelization for the user and only requiring the input of two user defined functions: `map` and `reduce`  (inspired by functional programming languages like Lisp). 

The `map` function takes a key value pair and produces a set of intermediate key value pairs, i.e. <document, contents> to a list of <letter, count> with different letters.

The `reduce` function merges a set of key value pairs (with the same key) and returns a singular combined key value pair, i.e. list of <letter, count> to just one <letter, total>.

The commmutative nature of these functions allows easy parallelization, as many `map` functions can take a subset of the document and combining their results will yield the same answer. Many `reduce` functions can be run on a subset of the total intermediate pairs and combining their results will also yield the same answer. This means that if the input file is split, the work of mapping and reducing can be assigned to many different processors at once, significantly decreeasing computing time.

## Examples/Testing

### Examples
The only current example is a program to find the number of occurences of each letter of the alphabet in a text document.

### Usage
*Note! As of now, number of workers must be a factor of linecount!*
Run the executable with two arguments--in order they are the path of the file to be processed and your public IP, both strings.

### Releases
To simply run a built executable of the MapReduce demo, go to the Releases tab and download the release for your OS (Windows is as of yet not supported). Run the executable from the command line with `./example NUM-WORKERS PATH-TO-INPUT YOUR-PUBLIC-IP`.

### Building from Source
First, clone the repository with `git clone https://github.com/richardfeynmanrocks/mapreduce.git`.

For MacOS, run the following commands inside the project directory:
```
gcc example.c mapreduce.c worker.c server.c -g -w -o example
./example NUM-WORKERS PATH-TO-INPUT YOUR-PUBLIC-IP
```
To build on Linux, run the following commands in the project directory:
```
gcc example.c mapreduce.c worker.c server.c -pthread -lm -w -o example
./example NUM-WORKERS PATH-TO-INPUT YOUR-PUBLIC-IP
```

## Known Issues
* ~~Last split piece of file is somehow unreadable and is essentially ignored~~ FIXED! 
* Server will sometimes miss a message and get 'stuck'.
  * Temporary hack-fix: Re-run program.
* Sometimes program will not output correct numbers.
  * Temporary hack-fix: Re-run program.
* Number of workers must be factor of linecount.
* Coordination between separate machines is essentially nonexistent.

## Execution Overview
Note: Although similar, this does not describe the Google implementation's execution but this implementation's execution.

1. User calls MapReduce code with the `begin` function, specifying path to input file, number of workers, IP, and the `map` and `reduce` functions.
2. `begin` function calls `split` which splits input file into NUM-WORKERS pieces for mapping.
3. `begin` function creates a thread for server with `start_server` and NUM-WORKERS threads for workers using `start_worker`, then waits for server to finish.
4. `start_server` launches a UDP server and waits for workers to reveal themselves and sets up "data structure" for tracking statuses.
5. `start_worker` opens a socket to communicate with server and declares its existence.
6. The server then orders each of the workers to perform a different `map` task and awaits reports of their completion.
7. Worker processes then read split files into array of key value pairs and call `map` function then write output to intermediate file and report to server as done.
8. Once all map tasks are completed, server begins ordering worker processes to perform `reduce` tasks.
9. Worker processes then read all intermediate files and aggregate them into one large file, sort said file to group keys together, then run reduce on a subset of those key value pairs while ensuring that pairs of a specific key are not split up (i.e all <A, number> pairs must be processed by one worker). Finally, worker process writes output to file.
10. Once server hears that all processes are completed, the `start_server` function exits successfully.
11. `begin` function kills all threads, aggregates output files into one file for convienience, then sorts said file for convienience.
12. Program exits back into user code.

## Future Features
[ ] Error handling and testing
[ ] Cross-machine computation
[ ] Refinements detailed in paper
[ ] Fault tolerance
[ ] Backup tasks
[ ] Benchmarking
[ ] Reporting via website
[ ] Package manager support
