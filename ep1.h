#ifndef EP1_H
#define EP1_H

#include <pthread.h>

#define QUANTUM 2
#define MARGEM 5
#define MAX_PROCESSOS 50

typedef struct{
    char nome[33];
    int deadline;
    int t0;
    int dt;
    int tf;
    int tempo_restante;
    int tempo_rodar;
    pthread_t thread_id;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
    int pode_executar;
    int finalizado;
} Processo;

typedef struct Node{
    Processo* p;
    struct Node *next;
} Node;

typedef struct Queue{
    Node* head;
    // Node* tail; // tail acabou nem sendo usado
} Queue;

Processo* pop(Queue* q);
void insert(Queue* q, Processo* p, int op);
void shortest_job_first(Processo* processos, int num_processos);
void round_robin(Processo* processos, int num_processos);
void priority_scheduler(Processo* processos, int num_processos);
void* executar_processo(void* arg);
void salvar_resultados(Processo *processos, int num_processos);
void sort(Processo* processos, int n);

#endif
