#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void* func() {

	printf("Eu sou a thread criada pela main. Existe outra mensagem que posso enviar ao usu�rio, mas antes vou mudar a minha prioridade para baixa, perdendo o controle de execu��o sem enviar a segunda mensagem.\n");
	
	csetprio(NULL, 2);

	printf("Voc� n�o deve conseguir ler isto!\n");

	return 0;
}

int main(int argc, char *argv[]) {

	int tid = 0;

	printf("Eu sou a main prestes a alterar a minha prioridade para m�dia. Como n�o existem outras threads, devo manter o controle de execu��o.\n");

	csetprio(NULL, 1);

	printf("Agora vou criar uma thread de tid %d com prioridade m�dia. Como a minha prioridade tamb�m � m�dia, n�o devo perder o controle de execu��o.\n", tid);

	tid = ccreate(func, 0, 1);


	printf("Eu sou a main ap�s a cria��o da thread e altera��o de prioridade. Agora vou ceder controle para ela...\n");

	cyield();

	printf("Eu sou a main voltando para terminar o programa.\n");

	return 0;
}

