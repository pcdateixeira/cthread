#
# Makefile para programas exemplo
# 

CC=gcc
LIB_DIR=../lib

all: exemplo1 exemplo2

exemplo1: exemplo1.c $(LIB_DIR)/libcthread.a
	$(CC) -o exemplo1 exemplo1.c -L$(LIB_DIR) -lcthread -Wall

exemplo2: exemplo2.c $(LIB_DIR)/libcthread.a
	$(CC) -o exemplo2 exemplo2.c -L$(LIB_DIR) -lcthread -Wall
	
clean:
	rm -rf exemplo mandel teste_vetor barbeiro filosofos prodcons series *.o *~




