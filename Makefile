CC = g++
CFLAGS = -g -Wall
SRCS = dart.cp
PROG = dart

OPENCV = `pkg-config opencv --cflags --libs`
LIBS = $(OPENCV)

$(PROG):$(SRCS)
	$(CC) $(CFLAGS) -o $(PROG) $(SRCS) $(LIBS)
clean:
	rm *.o
