build:
	gcc mapreduce.c worker.c server.c -w -O -c -g
	ar rcs mapreduce.a mapreduce.o server.o worker.o

test:
	gcc example.c mapreduce.c worker.c server.c -g -w -o example
