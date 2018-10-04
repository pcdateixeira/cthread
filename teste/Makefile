#
# Makefile para programas de teste criados pelos alunos
# 

CC=gcc
LIB_DIR=../lib

all: teste_yield teste_setprio teste_semaforo

teste_yield: teste_yield.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste_yield teste_yield.c -L$(LIB_DIR) -lcthread -Wall

teste_setprio: teste_setprio.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste_setprio teste_setprio.c -L$(LIB_DIR) -lcthread -Wall

teste_semaforo: teste_semaforo.c $(LIB_DIR)/libcthread.a
	$(CC) -o teste_semaforo teste_semaforo.c -L$(LIB_DIR) -lcthread -Wall
	
clean:
	rm -rf teste_yield teste_setprio teste_semaforo *.o *~




