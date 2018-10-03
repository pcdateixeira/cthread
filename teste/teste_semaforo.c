#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>
#include <stdlib.h>


void* func() {

	printf("Thread func trocando sua prioridade para m�xima e tentando entrar na se��o cr�tica. Eu n�o vou conseguir entrar nela e passarei para estado bloqueado\n");

	csetprio(NULL, 0);

	//cwait(semaforo);

	printf("Com a se��o cr�tica liberada, a thread func � desbloqueada e pode executar\n");

	//csignal(semaforo);

	return 0;
}

int main(int argc, char *argv[]) {

	int tid = 0;

	//csem_init(semaforo, 1);

	csetprio(NULL, 1);

	tid = ccreate(func, 0, 1);

	printf("Main entrando na se��o cr�tica...\n");

	//cwait(semaforo);

	printf("Thread main dentro da se��o cr�tica. Eu vou ceder controle para a thread func\n");

	cyield();

	printf("Thread main em controle novamente, agora saindo da se��o cr�tica para liberar a thread func\n");

	//csignal(semaforo);

	printf("Terminando o programa...\n");

	return 0;
}

