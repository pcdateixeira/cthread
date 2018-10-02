#
# Makefile para programas de teste criados pelos alunos
# 

CC=gcc
LIB_DIR=../lib

all: teste1 teste2

teste1: teste1.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste1 teste1.c -L$(LIB_DIR) -lcthread -Wall

teste2: teste2.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste2 teste2.c -L$(LIB_DIR) -lcthread -Wall
	
clean:
	rm -rf teste1 teste2 *.o *~




