
#include <stdio.h>
#include <string.h>
#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

typedef struct sMultiLevel {
	struct sFila2 *high;
	struct sFila2 *medium;
	struct sFila2 *low;
} MULTILEVEL;

MULTILEVEL PriorityQueue;
PFILA2 CurrentThread;

/*-------------------------------------------------------------------
                            ESCALONADOR
-------------------------------------------------------------------*/

PFILA2 findThreadByID(PFILA2 pFila, int id)
{
    TCB_t *tcb = (TCB_t *)GetAtIteratorFila2(pFila);

    while(tcb != NULL)
    {
        if(tcb->tid == id)
            return pFila;

        NextFila2(pFila);
        tcb = (TCB_t *)GetAtIteratorFila2(pFila);
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
int addThreadToPriorityQueue(TCB_t *tcb)
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
    return CurrentThread;
}

int deleteThreadFromPriorityQueue(PFILA2 pFila)
{
    return DeleteAtIteratorFila2(pFila);
}

void deleteCurrentThread()
{
    PFILA2 pFilaCurrent = getRunningThread();

    deleteThreadFromPriorityQueue(pFilaCurrent);
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
    PFILA2 pFilaCurrent = getRunningThread();
    PFILA2 pFilaReady = getReadyThread();
    TCB_t *TCBCurrent = (TCB_t *)GetAtIteratorFila2(pFilaCurrent);
    TCB_t *TCBReady = (TCB_t *)GetAtIteratorFila2(pFilaReady);

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

            CurrentThread = pFilaReady;
            swapcontext(TCBCurrent->context, TCBReady->context);
        }
    }
    else
    {
        TCBReady->state = PROCST_EXEC;

        CurrentThread = pFilaReady;
        swapcontext(TCBCurrent->context, TCBReady->context);
    }
}

/*-------------------------------------------------------------------
                            ESCALONADOR
-------------------------------------------------------------------*/

int ccreate (void* (*start)(void*), void *arg, int prio) {
	return -1;
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

