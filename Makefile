build:
	g++ mapreduce.cpp worker.cpp server.cpp -w -O -c -g
	ar rcs mapreduce.a mapreduce.o server.o worker.o

test:
	g++ example.cpp mapreduce.cpp worker.cpp server.cpp -g -w -o example
