
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#define LOW_PRIORITY 2
#define MEDIUM_PRIORITY 1
#define HIGH_PRIORITY 0

#define BYTES_IN_STACK 8192                   //LEMBRAR : antes de entregar aumentar o stack.
#define DEBUG_MODE 1                          //para encapsular os prints de debug com algo do tipo  if(DEBUG_MODE) print("aconteceu x \n");


// Relacionado a variável data do tcb
typedef struct tcbExtra{
    TCB_t *join;
} tcbExtra_t;

typedef struct sMultiLevel{
	FILA2 high;
	FILA2 medium;
	FILA2 low;
} MULTILEVEL;

MULTILEVEL PriorityQueue;

int CurrentThreadID = 0;
int isInitialized = 0;
int idCounter = 0;

/*-------------------------------------------------------------------
                            ESCALONADOR
-------------------------------------------------------------------*/

int getNewThreadID(){
	idCounter++;

	return idCounter;
}

TCB_t *findThreadByID(PFILA2 PQueue, int id){
    FirstFila2(PQueue);
    TCB_t *tcb = (TCB_t *)GetAtIteratorFila2(PQueue);

    while(tcb != NULL)
    {
        if(tcb->tid == id)
            return tcb;

        NextFila2(PQueue);
        tcb = (TCB_t *)GetAtIteratorFila2(PQueue);
    }

    return NULL;
}

TCB_t *findThreadByState(PFILA2 PQueue, int state){
    FirstFila2(PQueue);
    TCB_t *tcb = (TCB_t *)GetAtIteratorFila2(PQueue);

    while(tcb != NULL)
    {
        if(tcb->state == state && tcb->tid != CurrentThreadID)
            return tcb;

        NextFila2(PQueue);
        tcb = (TCB_t *)GetAtIteratorFila2(PQueue);
    }

    return NULL;
}

TCB_t *findThreadByIDInAllQueues(int id){
    TCB_t *TCBHigh = findThreadByID(&(PriorityQueue.high), id);
    TCB_t *TCBMedium = findThreadByID(&(PriorityQueue.medium), id);
    TCB_t *TCBLow = findThreadByID(&(PriorityQueue.low), id);

    if(TCBHigh != NULL)
        return TCBHigh;

    if(TCBMedium != NULL)
        return TCBMedium;

    if(TCBLow != NULL)
        return TCBLow;

    return NULL;
}

TCB_t *getRunningThread(){
    return findThreadByIDInAllQueues(CurrentThreadID);
}

TCB_t *findThreadByStateInAllQueues(int state){
    TCB_t *TCBHigh = findThreadByState(&(PriorityQueue.high), state);
    TCB_t *TCBMedium = findThreadByState(&(PriorityQueue.medium), state);
    TCB_t *TCBLow = findThreadByState(&(PriorityQueue.low), state);

    if(TCBHigh != NULL)
        return TCBHigh;

    if(TCBMedium != NULL)
        return TCBMedium;

    if(TCBLow != NULL)
        return TCBLow;

    return NULL;
}

TCB_t *getReadyThread(){
    return findThreadByStateInAllQueues(PROCST_APTO);
}

// Coloca o TCB em um novo item e coloca-o no final da fila correspondente a sua prioridade
int addThreadToQueue(TCB_t *tcb){
    if(tcb->prio == 0)
        return AppendFila2(&(PriorityQueue.high), tcb);

    if(tcb->prio == 1)
        return AppendFila2(&(PriorityQueue.medium), tcb);

    if(tcb->prio == 2)
        return AppendFila2(&(PriorityQueue.low), tcb);

    return -1;
}

int deleteThreadByID(int id){
    TCB_t *tcb = findThreadByIDInAllQueues(id); // A função só chama a findThread... para iterar a fila na qual a thread está

    switch(tcb->prio) {
        case 0: return DeleteAtIteratorFila2(&(PriorityQueue.high));
        case 1: return DeleteAtIteratorFila2(&(PriorityQueue.medium));
        case 2: return DeleteAtIteratorFila2(&(PriorityQueue.low));
    }

    return 1;
}

void deleteCurrentThread(){
    deleteThreadByID(CurrentThreadID);
}

void ScheduleThreads(int yielding){
    TCB_t *TCBCurrent = getRunningThread();
    TCB_t *TCBReady = getReadyThread();


    if(TCBReady == NULL)
        return;

    if(TCBCurrent == TCBReady)
        return;

    if(TCBCurrent->state == PROCST_APTO || TCBCurrent->state == PROCST_EXEC)
    {
        // Preempção só ocorre se a prioridade for superior (no caso se valor for menor)
        // Alternativamente, se a thread atual cedeu controle para outra de mesma prioridade, ela **não** deve continuar em execução
        if(TCBReady->prio < TCBCurrent->prio || (TCBReady->prio == TCBCurrent->prio && yielding))
        {
            TCBCurrent->state = PROCST_APTO;
            TCBReady->state = PROCST_EXEC;

            CurrentThreadID = TCBReady->tid;
            swapcontext(&(TCBCurrent->context), &(TCBReady->context));
        }
    }
    else
    {
        TCBReady->state = PROCST_EXEC;

        CurrentThreadID = TCBReady->tid;
        swapcontext(&(TCBCurrent->context), &(TCBReady->context));
    }
}

/*-------------------------------------------------------------------
                            ESCALONADOR
-------------------------------------------------------------------*/

/*-------------------------------------------------------------------
                              SUPORTE
-------------------------------------------------------------------*/

void initializeMainTCB(){
	TCB_t *TCBMain = (TCB_t *)malloc(sizeof(TCB_t));

	getcontext(&(TCBMain->context));

	TCBMain->state = PROCST_EXEC;
	TCBMain->tid = 0;
	TCBMain->prio = LOW_PRIORITY;

	tcbExtra_t *mainExtra = (tcbExtra_t *)malloc(sizeof(tcbExtra_t));
	mainExtra->join = NULL;
	TCBMain->data = (void *)mainExtra;

	addThreadToQueue(TCBMain);

	return;
}

void initializeCthread(){
    CreateFila2(&(PriorityQueue.high));
    CreateFila2(&(PriorityQueue.medium));
    CreateFila2(&(PriorityQueue.low));

	initializeMainTCB();

	isInitialized = 1;

	return;
}

// Causa a thread que chamou a função a terminar sua execução (não precisa recuperar qualquer retorno)
void exitThread(){
    TCB_t *TCBCurrent = getRunningThread();

    TCBCurrent->state = PROCST_TERMINO;

    // Se a thread terminou e estava sendo esperada por uma outra thread, desbloqueia essa thread
    if(((tcbExtra_t *)(TCBCurrent->data))->join != NULL && ((tcbExtra_t *)(TCBCurrent->data))->join->state == PROCST_BLOQ)
        ((tcbExtra_t *)(TCBCurrent->data))->join->state = PROCST_APTO;

    ScheduleThreads(0);
}

ucontext_t *makeLinkContext(void (*start)(void)){
    ucontext_t *context = (ucontext_t *)malloc(sizeof(ucontext_t));

    getcontext(context);

    context->uc_stack.ss_sp = malloc(sizeof(char)*BYTES_IN_STACK);
    context->uc_stack.ss_size = sizeof(char)*BYTES_IN_STACK;
    context->uc_link = NULL;

    makecontext(context, start, 0);

    return context;
}

ucontext_t *makeThreadContext(void (*start)(void), ucontext_t *linkContext, void *arg){
    ucontext_t *context = (ucontext_t *)malloc(sizeof(ucontext_t));

    getcontext(context);

    context->uc_stack.ss_sp = malloc(sizeof(char)*BYTES_IN_STACK);
    context->uc_stack.ss_size = sizeof(char)*BYTES_IN_STACK;
    context->uc_link = linkContext;

    makecontext(context, start, 1, arg);

    return context;
}

/*-------------------------------------------------------------------
                              SUPORTE
-------------------------------------------------------------------*/

// Cria uma thread para executar a função que começa em start
// Segundo especificação, retorna o ID da nova thread caso sucesso
int ccreate (void *(*start)(void *), void *arg, int prio){
    if(prio > LOW_PRIORITY || prio < HIGH_PRIORITY)
        return -1;

    if(isInitialized == 0)
		initializeCthread();

    ucontext_t *linkContext = makeLinkContext(exitThread);

    // Quando contexto termina, irá mudar para o contexto apontado no uc_link
    // Contexto link garante que estado passe para terminado com função exitThread() (e não precisa recuperar qualquer retorno)
    ucontext_t *threadContext = makeThreadContext((void (*)(void))start, linkContext, arg);

    TCB_t *tcb = (TCB_t *)malloc(sizeof(TCB_t));

    tcb->prio = prio;
    tcb->tid = getNewThreadID();
    tcb->state = PROCST_APTO;
    tcb->context = *threadContext;

    tcbExtra_t *mainExtra = (tcbExtra_t *)malloc(sizeof(tcbExtra_t));
    mainExtra->join = NULL;
	tcb->data = (void *)mainExtra;

    addThreadToQueue(tcb);

    if(DEBUG_MODE) printf("THREAD ID: %d FOI ADICIONADA!\n", tcb->tid);

    ScheduleThreads(0);

    return tcb->tid;
}

// Causa a thread que chamou a função a mudar sua PRÓPRIA prioridade
// Segundo especificação, o parâmetro tid deve ser sempre NULL
int csetprio(int tid, int prio){

    if(prio > LOW_PRIORITY || prio < HIGH_PRIORITY)
        return -1;

    if(isInitialized == 0)
        initializeCthread();

	  TCB_t *TCBCurrent = getRunningThread();

	  if(TCBCurrent == NULL)
        return -1;

	  if(DEBUG_MODE) printf("THREAD PRIO: %d -> ", TCBCurrent->prio);

    TCBCurrent->prio = prio;

    if(DEBUG_MODE) printf("THREAD PRIO: %d\n", TCBCurrent->prio);

    deleteCurrentThread();
    addThreadToQueue(TCBCurrent);

	  ScheduleThreads(0);

	  return 0;
}

// Causa a thread que chamou a função a ceder sua execução e ativar o escalonador
int cyield(void){

    TCB_t *TCBCurrent = getRunningThread();

    if(TCBCurrent == NULL)
        return -1;

    if(DEBUG_MODE) printf("THREAD STATE: %d -> ", TCBCurrent->state);

    TCBCurrent->state = PROCST_APTO;

    if(DEBUG_MODE) printf("THREAD STATE: %d\n", TCBCurrent->state);

    ScheduleThreads(1);

	return 0;
}

// Thread atual é bloqueada até que thread com ID == tid chame a exitThread()
int cjoin(int tid){
    // Uma determinada thread só pode ser esperada por uma única outra thread
    // Se duas ou mais threads fizerem cjoin para uma mesma thread, apenas a primeira que realizou a chamada será bloqueada
    TCB_t *tcb = findThreadByIDInAllQueues(tid);
    TCB_t *TCBCurrent = getRunningThread();

    if(tcb != NULL && TCBCurrent != NULL && ((tcbExtra_t *)(tcb->data))->join == NULL)
    {
        ((tcbExtra_t *)(tcb->data))->join = TCBCurrent;

        if(tcb->state != PROCST_TERMINO)
        {
            TCBCurrent->state = PROCST_BLOQ;

            ScheduleThreads(0);
        }
    }
    else
    {
        // Já foi bloqueada antes, não existe ou já terminou
        return -1;
    }

	return 0;
}

int csem_init(csem_t *sem, int count){

    //LEMBRAR:perguntar se a temos que alocar o semaforo no csem_init ou se ele ja vai estar alocado


    if(sem == NULL)
        return -1;

    //if(CreateFila2(sem->fila) != 0)
      //  return -1;
    FILA2 filaTemp ;
    CreateFila2(&filaTemp);         //por alguma razao da segmentation fault se tentarmos mandar um PFILA2 mas isso nao acontece para &FILA2
    sem->fila =&filaTemp;

    sem->count = count;

	return 0;
}

int cwait(csem_t *sem){
    sem->count = sem->count - 1;
    TCB_t *TCBCurrent = getRunningThread();


	if(TCBCurrent == NULL)
        return -1;

    if(sem == NULL)
        return -1;

    if(sem->count < 0)
    {
        TCBCurrent->state = PROCST_BLOQ;

        if(AppendFila2(sem->fila, TCBCurrent) != 0)
            return -1;

        if(DEBUG_MODE) printf("antes de escalonar em cwait\n" );
        ScheduleThreads(0);
    }


	return 0;
}

int csignal(csem_t *sem){

    sem->count = sem->count + 1;
    if(DEBUG_MODE) printf("antes de incrementar o contador em csignal. %d = count \n",sem->count);

    PFILA2 PQueue=sem->fila;
    int prio;

    if(sem == NULL)
        return -1;


  if(DEBUG_MODE) printf("antes de checar as filas do semaforo em csignal. %p = semaforo %p=semaforo->fila\n",sem,sem->fila);

	if(FirstFila2(sem->fila) != 0)
        return -1;

    TCB_t* tcb_sem = (TCB_t *)GetAtIteratorFila2(sem->fila);

    if(DEBUG_MODE) printf("tcb_sem = %p \n",tcb_sem );

    if(sem->count >= 0)
    {
        if(DEBUG_MODE) printf("antes de pegar a prioridade em csignal\n" );
        prio = tcb_sem->prio;
        if(DEBUG_MODE) printf("depois de pegar a prioridade em csignal\n" );

        while(tcb_sem != NULL)
        {
            if(tcb_sem->prio < prio)
            {
                if(DEBUG_MODE) printf("dentro do if do loop em csignal\n");
                prio = tcb_sem->prio;
                PQueue = sem->fila;
            }

            if(DEBUG_MODE) printf("antes de pegar o prox tcb no loop em csignal\n" );
            NextFila2(sem->fila);
            tcb_sem = (TCB_t *)GetAtIteratorFila2(sem->fila);
        }

        if(DEBUG_MODE) printf("antes de pegar o tcb de maior prioridade em csignal");
        tcb_sem = (TCB_t *)GetAtIteratorFila2(PQueue); // Encontra o TCB na fila do semáforo, mas falta encontrar o mesmo TCB nas filas de prioridades

        TCB_t *tcb = findThreadByIDInAllQueues(tcb_sem->tid);
        tcb->state = PROCST_APTO; // o TCB, que estava bloqueado, volta para o estado apto

        if(DEBUG_MODE) printf("antes de escalonar em csignal\n" );
        ScheduleThreads(0);
    }

	return 0;
}

int cidentify (char *name, int size){
	strncpy (name, "Humberto Lentz - 242308\nMakoto Ishikawa - 216728\nPedro Teixeira - 228509", size);

	return 0;
}
