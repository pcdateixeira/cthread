#
# Makefile de EXEMPLO
#
# OBRIGATÓRIO ter uma regra "all" para geração da biblioteca e de uma
# regra "clean" para remover todos os objetos gerados.
#
# É NECESSARIO ADAPTAR ESSE ARQUIVO de makefile para suas necessidades.
#  1. Cuidado com a regra "clean" para não apagar o "support.o"
#
# OBSERVAR que as variáveis de ambiente consideram que o Makefile está no diretório "cthread"
# 

CC=gcc
LIB_DIR=./lib
INC_DIR=./include
BIN_DIR=./bin
SRC_DIR=./src

all: objeto biblioteca

objeto: $(SRC_DIR)/lib.c
	$(CC) -c $(SRC_DIR)/lib.c -Wall
	mv lib.o $(BIN_DIR)/lib.o

biblioteca: $(BIN_DIR)/lib.o $(BIN_DIR)/support.o
	mkdir -p $(LIB_DIR)
	ar crs $(LIB_DIR)/libcthread.a $(BIN_DIR)/lib.o $(BIN_DIR)/support.o

clean:
	rm -rf $(LIB_DIR)/*.a $(BIN_DIR)/lib.o