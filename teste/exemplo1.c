#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void* func() {
	printf("Eu sou a thread criada pela main. Ol�!\n");

	return 0;
}

int main(int argc, char *argv[]) {

	int tid = 0;

	printf("Eu sou a main prestes a criar uma thread de tid %d com prioridade baixa. Como a minha prioridade tamb�m e baixa, n�o devo perder o controle de execu��o.\n", tid);

	tid = ccreate(func, 0, 2);

	printf("Eu sou a main ap�s a cria��o da thread. Agora vou ceder controle para ela...\n");

	cyield();

	printf("Eu sou a main voltando para terminar o programa.\n");

	return 0;
}

