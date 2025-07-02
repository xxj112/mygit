CC = gcc
CFLAGS = -Wall -g -Iinclude
LIBS = -lpthread -lmysqlclient

# 目录
SRCDIR = src
INCDIR = include
OUTDIR = output

# 文件列表
SERVER_SRCS = $(SRCDIR)/server.c $(SRCDIR)/mysql.c $(SRCDIR)/ser_op.c
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
SERVER_TARGET = $(OUTDIR)/ser

CLIENT_SRCS = $(SRCDIR)/client.c $(SRCDIR)/cli_op.c
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
CLIENT_TARGET = $(OUTDIR)/cli

# 默认目标
all: $(SERVER_TARGET) $(CLIENT_TARGET)

# 编译规则
$(SERVER_TARGET): $(SERVER_OBJS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

$(CLIENT_TARGET): $(CLIENT_OBJS)
	@mkdir -p $(OUTDIR)
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

# .o 文件生成规则
$(SRCDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(SERVER_OBJS) $(CLIENT_OBJS)
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
