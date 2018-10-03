#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>

void* func() {

	printf("Eu sou a thread criada pela main. Existe outra mensagem que posso enviar ao usuário, mas antes vou mudar a minha prioridade para baixa, perdendo o controle de execução sem enviar a segunda mensagem.\n");
	
	csetprio(NULL, 2);

	printf("Você não deve conseguir ler isto!\n");

	return 0;
}

int main(int argc, char *argv[]) {

	int tid = 0;

	printf("Eu sou a main prestes a alterar a minha prioridade para média. Como não existem outras threads, devo manter o controle de execução.\n");

	csetprio(NULL, 1);

	printf("Agora vou criar uma thread de tid %d com prioridade média. Como a minha prioridade também é média, não devo perder o controle de execução.\n", tid);

	tid = ccreate(func, 0, 1);


	printf("Eu sou a main após a criação da thread e alteração de prioridade. Agora vou ceder controle para ela...\n");

	cyield();

	printf("Eu sou a main voltando para terminar o programa.\n");

	return 0;
}

