CC = gcc
OBJS = queue.o main.o gameroom.o
TARGET = game_server

.SUFFIXES = .c .o
all : $(TARGET)
$(TARGET): $(OBJS)
	     $(CC) -O3 -o $@ $(OBJS) -lsqlite3 -lcrypto
clean :
	rm -f $(OBJS) $(TARGET) 
