CC = gcc
CFLAGS = -Wall -g
LIBS = -lpthread -lmysqlclient

SRCS = server.c mysql.c
OBJS = $(SRCS:.c=.o)
TARGET = ser
CLIENT = cli

all: $(TARGET) $(CLIENT)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(CLIENT): client.c
	$(CC) $(CFLAGS) -o $(CLIENT) client.c -lpthread

clean:
	rm -f $(OBJS) $(TARGET) $(CLIENT)
