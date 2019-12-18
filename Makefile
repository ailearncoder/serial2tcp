CC = gcc
CFLAGS = -Wall -O -g
OBJS = helloworld.o
TARGET = main

$(TARGET) : $(OBJS)
	$(CC) -o $(TARGET) $(OBJS)
	@echo "=======finish========="
	cp main ../../../nfs/ser2tcp
	
helloworld.o : helloworld.c
	$(CC) $(INCLUDE) $(CFLAGS) -c helloworld.c -o helloworld.o

clean :
	rm *.o $(TARGET) -rf
	@echo "=============clean ok========"
