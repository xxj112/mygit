CC = gcc
CFLAGS = -Wall -g
LIBS = -lpthread -lmysqlclient

# 服务器部分
SRCS = server.c mysql.c ser_op.c
OBJS = $(SRCS:.c=.o)
TARGET = ser

# 客户端部分
CLIENT_SRCS = client.c cli_op.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
CLIENT = cli

all: $(TARGET) $(CLIENT)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(CLIENT): $(CLIENT_OBJS)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

clean:
	rm -f $(OBJS) $(TARGET) $(CLIENT_OBJS) $(CLIENT)
