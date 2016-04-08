CC = gcc
OBJS = queue.o main.o
TARGET = game_server

.SUFFIXES = .c .o
all : $(TARGET)
$(TARGET): $(OBJS)
	     $(CC) -o $@ $(OBJS)
clean :
	rm -f $(OBJS) $(TARGET) 
