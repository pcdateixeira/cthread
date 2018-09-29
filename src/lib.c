
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"


#define LOW_PRIORITY 2
#define MEDIUM_PRIORITY 1
#define HIGH_PRIORITY 0

#define BYTES_IN_STACK 8388608         //deixando assim mesmo porque o compilador n'ao esta reconhecendo SIGSTKS

typedef struct tcbExtra{ //relacionado a variavel data do tcb,coloquem aqui as variaveis que voces precisarem usar no tcb que nao foi definido pelos professores
	//  ;
}tcbExtra_t;

typedef struct sMultiLevel {
	struct sFila2 high;
	struct sFila2 medium;
	struct sFila2 low;
} MULTILEVEL;

MULTILEVEL PriorityQueue;
int CurrentThreadID;

int isCthreadInitialized = 0;                //checar se a main ja ganhou tcb
int idCounter = -1;                          //para gerar os ids
int isEndOfThread = 0;                       //avisar para o ESCALONADOR se eh o fim de uma thread ou nao


/*-------------------------------------------------------------------
                            ESCALONADOR
-------------------------------------------------------------------*/

PFILA2 findThreadByID(PFILA2 PQueue, int id)
{
    FirstFila2(PQueue);
    TCB_t *tcb = (TCB_t *)((PNODE2)GetAtIteratorFila2(PQueue))->node;

    while(tcb != NULL)
    {
        if(tcb->tid == id)
            return PQueue;

        NextFila2(PQueue);
        tcb = (TCB_t *)((PNODE2)GetAtIteratorFila2(PQueue))->node;
    }

    return NULL;
}

PFILA2 findThreadByIDInAllQueues(int id)
{
    PFILA2 PQueueHigh = findThreadByID(&PriorityQueue.high, id);
    PFILA2 PQueueMedium = findThreadByID(&PriorityQueue.medium, id);
    PFILA2 PQueueLow = findThreadByID(&PriorityQueue.low, id);

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
    PNODE2 newNode;

    newNode = malloc(sizeof(NODE2));
    newNode->node = (void *)tcb;
    newNode->ant = NULL;
    newNode->next = NULL;

    if(tcb->prio == 0)
        flag = AppendFila2(&PriorityQueue.high, (void *)newNode);
    else if(tcb->prio == 1)
        flag = AppendFila2(&PriorityQueue.medium, (void *)newNode);
    else if(tcb->prio == 2)
        flag = AppendFila2(&PriorityQueue.low, (void *)newNode);
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
    if(FirstFila2(&PriorityQueue.high) == 0)
        return &PriorityQueue.high;
    else if(FirstFila2(&PriorityQueue.medium) == 0)
        return &PriorityQueue.medium;
    else if(FirstFila2(&PriorityQueue.low) == 0)
        return &PriorityQueue.low;
    else
        return NULL;
}

// Retorna ponteiro para um nodo com TCB de estado APTO na fila mais prioritária
PFILA2 getReadyThread()
{
    PFILA2 PQueue = getNonEmptyQueue();
    TCB_t *tcb = (TCB_t *)((PNODE2)GetAtIteratorFila2(PQueue))->node;

    while(tcb != NULL)
    {
        if(tcb->state == PROCST_APTO)
            return PQueue;

        NextFila2(PQueue);
        tcb = (TCB_t *)((PNODE2)GetAtIteratorFila2(PQueue))->node;
    }

    return NULL;
}

void ScheduleThreads()
{
    PFILA2 PQueueCurrent = getRunningThread();
    PFILA2 PQueueReady = getReadyThread();
    TCB_t *TCBCurrent = (TCB_t *)((PNODE2)GetAtIteratorFila2(PQueueCurrent))->node;
    TCB_t *TCBReady = (TCB_t *)((PNODE2)GetAtIteratorFila2(PQueueReady))->node;


		if(isEndOfThread == 1){
			//funcao que lida com semaforo
		}

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

						CurrentThreadID = TCBReady->tid;
            swapcontext(&TCBCurrent->context, &TCBReady->context); // usa-se "->" pois TCBCurrent é um *ponteiro* pra uma estrutura, e "&" pois swapcontext requer o *endereço* de um contexto
        }
    }
    else
    {
        TCBReady->state = PROCST_EXEC;

        CurrentThreadID = TCBReady->tid;
        swapcontext(&TCBCurrent->context, &TCBReady->context);
    }
}

void ScheduleThreadsPreemptive(){
	isEndOfThread=0;
	ScheduleThreads();
}

void ScheduleThreadsEndOfThread(){
	isEndOfThread=1;
	ScheduleThreads();
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


	getcontext(&(mainThread->context));

	tcbExtra_t* mainExtra = malloc(sizeof(tcbExtra_t));  //alocando as variaveis que nos julgaremos uteis para a implementacao;
	mainThread->data = mainExtra;

	addThreadToQueue(mainThread);

	return;
}

void addNewTCB(TCB_t* fatherThread,int prio,void* (*start)(void*),void *arg){ //daria para fazer o numero de argumentos ficar menor,mas nao sei se vale muito a pena


	TCB_t* newThread;
	ucontext_t* tempContext=malloc(sizeof(ucontext_t*));
	newThread = malloc(sizeof(TCB_t));
	newThread->tid = getNewThreadId();
	newThread->state = PROCST_APTO;
	newThread->prio = prio;

	getcontext(&(newThread->context));                                 //criacao do novo contexto
	newThread->context.uc_stack.ss_sp = malloc(sizeof(char)*BYTES_IN_STACK);
	newThread->context.uc_stack.ss_size = sizeof(char)*BYTES_IN_STACK;

	makecontext(tempContext,ScheduleThreadsEndOfThread,0);
	newThread->context.uc_link = tempContext;
	makecontext(&(newThread->context),start(arg),0);//makecontext necessita de um void(*)(void),start eh do tipo void*(start)(void*),tenho certeza de que tem algo errado aqui

	tcbExtra_t* newExtra = malloc(sizeof(tcbExtra_t));              //recursos extras
	newThread->data = newExtra;

	addThreadToQueue(newThread);

}

void initializeCthread(){               //se no futuro mais coisas precisem ser inicializadas para cthread colocar aqui

    // Inicializa as filas de threads
    CreateFila2(&PriorityQueue.high);
    CreateFila2(&PriorityQueue.medium);
    CreateFila2(&PriorityQueue.low);
	createMainTCB();
	isCthreadInitialized = 1;
	return;
}


int getCurrentThread(TCB_t** currentTCB){   //retorna 0 para sucesso -1 caso contrario
	PFILA2 filaTemp;

	filaTemp=getRunningThread();

	if(filaTemp==NULL)
		return -1;

	*currentTCB=(TCB_t*)((PNODE2)GetAtIteratorFila2(filaTemp))->node;

	return 0;
}

void copyTcb(TCB_t** destination,TCB_t* source){

	*destination = malloc(sizeof(TCB_t));
	(*destination)->tid = source->tid;
	(*destination)->state = source->state;
	(*destination)->prio = source->prio;
	(*destination)->context = source->context;
	(*destination)->data = source->data;

}


/*-------------------------------------------------------------------
														funcoes suporte-fim (lembrar de colocar um nome melhor para essas funcoes)
---------------------------------------------------------------------*/

int ccreate (void* (*start)(void*), void *arg, int prio) {


	TCB_t* currentTCB;


	if(!isCthreadInitialized){
		initializeCthread();
	}

	if(getCurrentThread(&currentTCB)<0) return -1;

	addNewTCB(currentTCB,prio,start,arg);

	ScheduleThreadsPreemptive();

	return 0;

}



int csetprio(int tid, int prio) {

	PFILA2 filaTemp;
	PNODE2 nodeTemp;

  filaTemp = findThreadByIDInAllQueues(tid);

	if(filaTemp == NULL)
		return -1;

	nodeTemp=(PNODE2)GetAtIteratorFila2(filaTemp);

	((TCB_t*)nodeTemp->node)->prio=prio;

	TCB_t* copyNode;
	copyTcb(&copyNode,(TCB_t*)nodeTemp->node);

	deleteThreadByID(((TCB_t*)nodeTemp->node)->tid);

	addThreadToQueue(copyNode);

	ScheduleThreadsPreemptive();

	return 0;
}

int cyield(void) {

	TCB_t* currentTCB;

	if(getCurrentThread(&currentTCB)<0)
		return -1;

	currentTCB->state=PROCST_APTO;

	ScheduleThreadsPreemptive();

	return 0;
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
	strncpy (name, "Humberto Lentz - 242308\nMakoto Ishikawa - 216728\nPedro Teixeira - 228509", size);
	return 0;
}
