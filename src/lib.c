
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"


#define LOW_PRIORITY 2
#define MEDIUM_PRIORITY 1
#define HIGH_PRIORITY 0

#define BYTES_IN_STACK 2048

typedef struct tcbExtra{ //relacionado a variavel data do tcb,coloquem aqui as variaveis que voces precisarem usar no tcb que nao foi definido pelos professores
	int continueThread ;                //identifica se a thread ja passou ou nao pelo escalonador,toda vez que um swapcontext eh chamado,inverter o continueThread dos tcb involvidos
}tcbExtra_t;

typedef struct sMultiLevel {
	struct sFila2 *high;
	struct sFila2 *medium;
	struct sFila2 *low;
} MULTILEVEL;

MULTILEVEL PriorityQueue;
int CurrentThreadID;

int isCthreadInitialized =0 ;
int idCounter = -1;

CreateFila2(PriorityQueue.high);
CreateFila2(PriorityQueue.medium);
CreateFila2(PriorityQueue.low);

/*-------------------------------------------------------------------
                            ESCALONADOR
-------------------------------------------------------------------*/

PFILA2 findThreadByID(PFILA2 PQueue, int id)
{
    FirstFila2(PQueue);
    TCB_t *tcb = (TCB_t *)GetAtIteratorFila2(PQueue);

    while(tcb != NULL)
    {
        if(tcb->tid == id)
            return PQueue;

        NextFila2(PQueue);
        tcb = (TCB_t *)GetAtIteratorFila2(PQueue);
    }

    return NULL;
}

PFILA2 findThreadByIDInAllQueues(int id)
{
    PFILA2 PQueueHigh = findThreadByID(PriorityQueue.high, id);
    PFILA2 PQueueMedium = findThreadByID(PriorityQueue.medium, id);
    PFILA2 PQueueLow = findThreadByID(PriorityQueue.low, id);

    if(PQueueHigh != NULL)
        return PQueueHigh;
    else if(PQueueMedium != NULL)
        return PQueueMedium;
    else if(PQueueLow != NULL)
        return PQueueLow;
    else
        return NULL;
}

// Coloca o TCB em um novo item e coloca-o no final da fila correspondente a sua prioridade
int addThreadToQueue(TCB_t *tcb)
{
    int flag;

    if(tcb->prio == 0)
        flag = AppendFila2(PriorityQueue.high, (void *)tcb);
    else if(tcb->prio == 1)
        flag = AppendFila2(PriorityQueue.medium, (void *)tcb);
    else if(tcb->prio == 2)
        flag = AppendFila2(PriorityQueue.low, (void *)tcb);
    else
        return -1;

    return flag;
}

PFILA2 getRunningThread()
{
    return findThreadByIDInAllQueues(CurrentThreadID);
}

int deleteThreadByID(int id)
{
    PFILA2 PQueue = findThreadByIDInAllQueues(id);
    return DeleteAtIteratorFila2(PQueue);
}

void deleteCurrentThread()
{
    deleteThreadByID(CurrentThreadID);
}

// Retorna ponteiro para uma fila de prioridade que não estiver vazia com iterador da fila no primeiro item da mesma
PFILA2 getNonEmptyQueue()
{
    if(FirstFila2(PriorityQueue.high) == 0)
        return PriorityQueue.high;
    else if(FirstFila2(PriorityQueue.medium) == 0)
        return PriorityQueue.medium;
    else if(FirstFila2(PriorityQueue.low) == 0)
        return PriorityQueue.low;
    else
        return NULL;
}

// Retorna ponteiro para um nodo com TCB de estado APTO na fila mais prioritária
PFILA2 getReadyThread()
{
    PFILA2 PQueue = getNonEmptyQueue();
    TCB_t *tcb = (TCB_t *)GetAtIteratorFila2(PQueue);

    while(tcb != NULL)
    {
        if(tcb->state == PROCST_APTO)
            return PQueue;

        NextFila2(PQueue);
        tcb = (TCB_t *)GetAtIteratorFila2(PQueue);
    }

    return NULL;
}

void ScheduleThreads()
{
    PFILA2 PQueueCurrent = getRunningThread();
    PFILA2 PQueueReady = getReadyThread();
    TCB_t *TCBCurrent = (TCB_t *)GetAtIteratorFila2(PQueueCurrent);
    TCB_t *TCBReady = (TCB_t *)GetAtIteratorFila2(PQueueReady);

    if(TCBReady == NULL)
        return;

    if(TCBCurrent == TCBReady)
        return;

    if(TCBCurrent->state == PROCST_APTO || TCBCurrent->state == PROCST_EXEC)
    {
        // 0 = alta, 1 = média, 2 = baixa
        // Preempção só ocorre se a prioridade for superior
        if(TCBReady->prio < TCBCurrent->prio)
        {
            TCBCurrent->state = PROCST_APTO;
            TCBReady->state = PROCST_EXEC;

						(*tcbExtra_t)TCBCurrent->data->continueThread = 0;
						(*tcbExtra_t)TCBReady->data->continueThread = 1;

						CurrentThreadID = TCBReady->tid;
            swapcontext(TCBCurrent->context, TCBReady->context);
        }
    }
    else
    {
        TCBReady->state = PROCST_EXEC;
				(*tcbExtra_t)TCBReady->data->continueThread = 1;

        CurrentThreadID = TCBReady->tid;
        swapcontext(TCBCurrent->context, TCBReady->context);
    }
}

/*-------------------------------------------------------------------
                            ESCALONADOR(fim)
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
														funcoes suporte (lembrar de colocar um nome melhor para essas funcoes)
---------------------------------------------------------------------*/

int getNewThreadId(){
	idCounter++;
	return idCounter;
}


void createMainTCB(){
	TCB_t* mainThread ;
	mainThread = malloc(sizeof(TCB_t));          //criar uma funcao para alocar de maneira segura LEMBRAR
	mainThread->tid = getNewThreadId();
	mainThread->state = PROCST_EXEC;
	mainThread->prio = LOW_PRIORITY;


	getContext(&(mainThread->context));

	tcbExtra_t* mainExtra = malloc(sizeof(tcbExtra_t));  //alocando as variaveis que nos julgaremos uteis para a implementacao;
	mainExtra->continueThread = 0 ;
	(tcbExtra_t*)mainThread->data = mainExtra;

	addThreadToQueue(mainThread);

	return;
}

void addNewTCB(TCB_t* newThread,TCB_t* fatherThread,int prio,void* (*start)(void*),void *arg){ //daria para fazer o numero de argumentos ficar menor,mas nao sei se vale muito a pena

	newThread = malloc(sizeof(TCB_t));
	newThread->tid = getNewThreadId();
	newThread->state = PROCST_APTO;
	newThread->prio = prio;

	getContext(&(newThread->context));                                 //criacao do novo contexto
	newThread->context.ss_sp = malloc(sizeof(char)*BYTES_IN_STACK);
	newThread->context.ss_size = sizeof(char)*BYTES_IN_STACK
	newThread->context.uc_link = &(fatherThread->context);
	makecontext(&(newThread->context),start,1,arg);

	tcbExtra_t* newExtra = malloc(sizeof(tcbExtra_t));              //recursos extras
	newExtra->continueThread = 0;
	(tcbExtra_t*)newThread->data = newExtra;

	addThreadToQueue(newThread);

}

void initializeCthread(){               //se no futuro mais coisas precisem ser inicializadas para cthread colocar aqui
	createMainThread();
	isCthreadInitialed = 1;
	return;
}

void updateContext(TCB_t* currentTCB){
	ucontext_t* currentContext;
	getContext(currentContext);
	currrentTCB->context = *currentContext;
}

void getCurrentThread(TCB_t** currentTCB){
	*currentTCB=(TCB_t*)GetAtIteratorFila2(getRunningThread());
}


/*-------------------------------------------------------------------
														funcoes suporte-fim (lembrar de colocar um nome melhor para essas funcoes)
---------------------------------------------------------------------*/

int ccreate (void* (*start)(void*), void *arg, int prio) {


	TCB_t* currentTCB,newTCB;


	if(!isCthreadInitialized){
		initializeCthread();
	}

	getCurrentThread(&currentTCB);

	addNewTCB(newTCB,currentTCB,prio,start,arg);

	ScheduleThreads();

	return 0;       //LEMBRAR ,colocar testes nas funcoes acima para aumentar a resiliencia,de acordo com o pdf -1 eh considero erro e 0 eh sucesso na operacao
	//return -1;
}

int csetprio(int tid, int prio) {
	return -1;
}

int cyield(void) {
	return -1;
}

int cjoin(int tid) {
	return -1;
}

int csem_init(csem_t *sem, int count) {
	return -1;
}

int cwait(csem_t *sem) {
	return -1;
}

int csignal(csem_t *sem) {
	return -1;
}

int cidentify (char *name, int size) {
	strncpy (name, "Sergio Cechin - 2017/1 - Teste de compilacao.", size);
	return 0;
}
