#include "../include/support.h"
#include "../include/cthread.h"
#include <stdio.h>
#include <stdlib.h>

csem_t* semaforo;

void* func() {

	printf("Thread func trocando sua prioridade para maxima e tentando entrar na secaoo critica.\n");
	printf("Eu nao vou conseguir entrar nela e passarei para estado bloqueado\n");


	csetprio(NULL, 0);

	cwait(semaforo);

  printf("Com a secao critica liberada, a thread func e desbloqueada e pode executar\n");

	csignal(semaforo);

	return 0;
}

int main(int argc, char *argv[]) {

	int tid = 0;

	semaforo = malloc(sizeof(csem_t));
	csem_init(semaforo, 1);

	csetprio(NULL, 1);

	tid = ccreate(func, 0, 1);

  printf("Main entrando na secao critica...\n");

	cwait(semaforo);

	printf("Thread main dentro da secao critica.\n");
	printf("Eu vou ceder controle para a thread func\n");

	cyield();

	printf("Thread main em controle novamente,\n");
	printf("agora saindo da secao critica para liberar a thread func\n");

	printf("ponteiro do semaforo = %p \n",semaforo);
	csignal(semaforo);

  printf("Terminando o programa...\n");

	return 0;
}
