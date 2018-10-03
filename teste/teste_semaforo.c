#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>
#include <stdlib.h>


void* func() {

	printf("Thread func trocando sua prioridade para máxima e tentando entrar na seção crítica. Eu não vou conseguir entrar nela e passarei para estado bloqueado\n");

	csetprio(NULL, 0);

	//cwait(semaforo);

	printf("Com a seção crítica liberada, a thread func é desbloqueada e pode executar\n");

	//csignal(semaforo);

	return 0;
}

int main(int argc, char *argv[]) {

	int tid = 0;

	//csem_init(semaforo, 1);

	csetprio(NULL, 1);

	tid = ccreate(func, 0, 1);

	printf("Main entrando na seção crítica...\n");

	//cwait(semaforo);

	printf("Thread main dentro da seção crítica. Eu vou ceder controle para a thread func\n");

	cyield();

	printf("Thread main em controle novamente, agora saindo da seção crítica para liberar a thread func\n");

	//csignal(semaforo);

	printf("Terminando o programa...\n");

	return 0;
}

