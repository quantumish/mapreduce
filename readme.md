# C++ "Port"

## About
Functional programming and anonymous functions are rather useful, and the C++ standard library `functional` implements them. However, MapReduce uses C function pointers to refer to the `map`, `reduce`, and `translate` functions, so this "port (which was literally just getting it to run right with `g++`) was designed to allow a more flexible use of MapReduce.

## Compiling the Archive File

Same as normal, but with `g++` instead of `gcc` for the compiler.

```
g++ mapreduce.cpp worker.cpp server.cpp -O -c -g
ar rcs mapreduce.a mapreduce.o server.o worker.o
```
