#define _GNU_SOURCE  // Necessário para pthread_setaffinity_np no Linux (falta implementar)
#include <time.h>
#include <math.h> // para o round()
#include <sched.h>   // Para a manipulação de CPU sets 
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#include "ep1.h"


const char *arquivo_saida; // global para o arquivo de saída para facilitar escrever no output.txt
int preempcoes = 0; // Contador global de preempções caso seja escolhido o ROund Robin ( para facilitar escrever no arquivo de saída)
struct timespec start_t;
double tempo_inicial = 0.0; // para implementar o Busy-Waiting nas threads

    // agora estão definidas no header
// typedef struct Node{
//     Processo* p;
//     struct Node *next;
// } Node;

// typedef struct Queue{
//     Node* head;
//     Node* tail;
// } Queue;

// typedef struct{
//     char nome[33];
//     int deadline;
//     int t0;
//     int dt;
//     int tf; // para conseguir imprimir o tempo final no output.txt
//     int tempo_restante;
//     int tempo_rodar; // mudança para alocação dinamica de quantuns pelo escalonador escolihdo
//     pthread_t thread_id;
//     pthread_mutex_t mutex;
//     pthread_cond_t cond;
//     int pode_executar; // usado como flag
//     int finalizado;
// } Processo;

static Processo* pop(Queue* q){
    if(q==NULL || q->head==NULL){
        return NULL;
    }
    Node* n = q->head;
    Processo* p = n->p;
    q->head=q->head->next;
    free(n);
    return p;
}

    // implementando a queue para Ready Queue (preciso refatorar os escalonadores)
static void insert(Queue* q, Processo* p, int op){ // op 1=rr, op 2=sjf, op 3=priority_sch
    Node* n = malloc(sizeof(Node));
    if(n==NULL){
        printf("Erro de alocação da Fila\n");
        exit(1);
    }
    n->p=p;
    n->next=NULL;
    if(q->head==NULL){
        q->head=n;
        return;
    }
    
    if(op==1){ // rr é FIFO
        // n->next = q->head;
        // q->head=n;
        // É mais eficiente inserir no final e dar pop() no primeiro.
        Node* last = q->head;
        while(last->next!=NULL) last=last->next;
        last->next=n;
    }
    if(op==2){ // sjf ordena por dt
        Node* cur = q->head;
        Node* prev = NULL;
        while(cur!= NULL && cur->p->dt < p->dt){
            prev=cur;
            cur=cur->next;
        }
        if(prev==NULL){
            n->next=q->head;
            q->head=n;
        } else{
            n->next=cur;
            prev->next=n;
        }
    }
    if(op==3){ // priority_sch ordena por deadline (falta implementar direito, pois poderia ser por tempo restante também ou dealine-t0)
        Node* cur = q->head;
        Node* prev = NULL;
        while(cur!= NULL && cur->p->deadline < p->deadline){
            prev=cur;
            cur=cur->next;
        }
        if(prev==NULL){
            n->next=q->head;
            q->head=n;
        } else{
            n->next=cur;
            prev->next=n;
        }
    }
}

// // simulação de execução (que as threads vão executar)
// void* executar_processo(void* arg){
//     Processo* p= (Processo*)arg;
//     while(1){
//         pthread_mutex_lock(&p->mutex); 
//         while(!p->pode_executar && !p->finalizado){ // agora espera o escalonador liberar
//             pthread_cond_wait(&p->cond, &p->mutex);
//         }

//         if(p->finalizado || p->tempo_restante <= 0){
//             pthread_mutex_unlock(&p->mutex);
//             break;
//         }
//         sleep(p->tempo_rodar);
//         p->tempo_restante -= p->tempo_rodar;
//         p->pode_executar=0;
//         pthread_cond_signal(&p->cond);
//         pthread_mutex_unlock(&p->mutex);
//     }
//     return NULL;
// }

static void* executar_processo(void* arg){
    Processo* p = (Processo*)arg;
    // time_t start, now;
    // time(&start);
    while(1){
        pthread_mutex_lock(&p->mutex); 
        while(!p->pode_executar && !p->finalizado){ // agora espera o escalonador liberar
            pthread_cond_wait(&p->cond, &p->mutex);
        }

        if(p->finalizado || p->tempo_restante <= 0){
            pthread_mutex_unlock(&p->mutex);
            break;
        }

        pthread_mutex_unlock(&p->mutex);

        struct timespec start, now; // a struct timespec é um par (tv_sec, tv_nsec), que captura segundos e nanosegundos
        clock_gettime(CLOCK_MONOTONIC, &start); // melhor usar clock_gettime() pois apenas time() retorna em segundos e daria muitos erros
        double tempo_gasto = 0.0;

        while(tempo_gasto< p->tempo_rodar){
            volatile int dummy =0; // volatile para forçar a CPU a rodar isso e não apagar o "loop inutil"
            for(int i=0;i<1000;i++){ // loop apenas para fazer Busy-Wait
                dummy++;
            }
            clock_gettime(CLOCK_MONOTONIC, &now);
            tempo_gasto = (now.tv_sec - start.tv_sec) + ((now.tv_nsec - start.tv_nsec) / 1e9); // 1e9 aqui pois tv_nsec é nanosegundos
        }
        pthread_mutex_lock(&p->mutex);

        // sleep(p->tempo_rodar); // troquei para Busy-Wait
        p->tempo_restante -= p->tempo_rodar;
        p->pode_executar=0;
        pthread_cond_signal(&p->cond);
        pthread_mutex_unlock(&p->mutex);
    }
}

void salvar_resultados(Processo *processos, int num_processos){
    FILE *fp = fopen(arquivo_saida, "w");
    if(fp == NULL){
        perror("Erro ao criar arquivo de saída");
        return;
    }

    for(int i = 0; i < num_processos; i++){
        int tf = processos[i].tf; 
        int tr = tf - processos[i].t0;
        int cumpriu = (tf <= processos[i].deadline) ? 1 : 0;

        fprintf(fp, "%d %s %d %d\n", cumpriu, processos[i].nome, tf, tr);
    }

    fprintf(fp, "%d\n", preempcoes);

    fclose(fp);
}

// agora isso só serve pra ordenar os processos por tempo de inicio
static void sort(Processo* processos, int n){
    Processo tmp;
    for(int i=0;i<n;i++){
            for(int j=0; j<n-i-1;j++){
                if(processos[j].t0 > processos[j+1].t0){
                    tmp = processos[j];
                    processos[j]=processos[j+1];
                    processos[j+1]=tmp;
                }
            }
        }
}

void shortest_job_first(Processo* processos, int num_processos){
    int tempo_atual =0;
    int processos_concluidos=0;
    int ct=0; // ct controla os processos ja inicializados
    Queue ready_queue ={NULL, NULL};
    struct timespec now;
    double act_time;

        // lembra que processos já está ordenado por tempo de início
    while(processos_concluidos < num_processos && round(act_time) < 120){ // limite de tempo de 120 segundos
        clock_gettime(CLOCK_MONOTONIC, &now);
        act_time = (now.tv_sec - start_t.tv_sec) + ((now.tv_nsec - start_t.tv_nsec) / 1e9);
        // primeiro eu inicializo os processos que devem ser inicializados na Fila de Prontos
        while(ct<num_processos && processos[ct].t0<=act_time){
            insert(&ready_queue, &processos[ct], 2);
            pthread_create(&processos[ct].thread_id, NULL, executar_processo, &processos[ct]);
            ct++;
        }
        // depois efetivamente roda O PRIMEIRO PROCESSO na Fila de Prontos
        Processo* pp = pop(&ready_queue); 
        if(pp == NULL){
            usleep(800000); // botei para 800ms (menos que 1s para contar pelos jitterings das outras funções) 
            continue;
            //tempo_atual++; // caso não tenha nenhum processo na fila nesse instante de tempo, só passa 1 segundo
        } else{
            // Processo p = *pp;
            pthread_mutex_lock(&pp->mutex);
            pp->tempo_rodar = pp->dt; // no sjf sem preempção, o escalonador autoriza tudo
            pp->pode_executar = 1;
            pthread_cond_signal(&pp->cond);
            // pthread_mutex_unlock(&pp->mutex); // nao tem pq dar unlock aqui
            
            while(pp->pode_executar){
                pthread_cond_wait(&pp->cond, &pp->mutex);
            }
            // sleep(pp->dt); // aqui o escalonador "dorme" pra simular a passagem de tempo
            // tempo_atual += pp->dt; // então tem que aumentar o tempo atual pra poder inicializar os próximos processos na Fila de Prontos

            pp->finalizado = 1; // finalizado = 1 pois no sfj sem preempção o processo roda até acabar
            clock_gettime(CLOCK_MONOTONIC, &now);
            pp->tf = now.tv_sec-start_t.tv_sec;
            processos_concluidos++;
            pthread_mutex_unlock(&pp->mutex); // só preciso dar o unlock no final
        }
    }
    salvar_resultados(processos, num_processos);
}

void round_robin(Processo* processos, int num_processos){
    int tempo_atual = 0;
    int processos_concluidos = 0;
    Queue ready_queue ={NULL, NULL};
    int ct = 0;
    Processo* pp = NULL;
    struct timespec now;
    double act_time;

    while(processos_concluidos < num_processos && round(act_time) < 120){
        clock_gettime(CLOCK_MONOTONIC, &now);
        act_time = (now.tv_sec - start_t.tv_sec) +  ((now.tv_nsec - start_t.tv_nsec) / 1e9);
        while(ct < num_processos && processos[ct].t0 <= act_time){
            pthread_create(&processos[ct].thread_id, NULL, executar_processo, &processos[ct]);
            insert(&ready_queue, &processos[ct], 1);
            ct++;
        }
        if(pp!=NULL && pp->finalizado==0) insert(&ready_queue, pp, 1); // agora insiro o processo anterior só DEPOIS da nova batch 
        pp = pop(&ready_queue);
        if(pp == NULL){
            //tempo_atual++; // Avança o relógio virtual
            usleep(800000);
            continue;
        }

        pthread_mutex_lock(&pp->mutex);
        int q = (pp->tempo_restante < QUANTUM) ? pp->tempo_restante : QUANTUM;
        pp->tempo_rodar = q;
        pp->pode_executar = 1;
        pthread_cond_signal(&pp->cond);

        while(pp->pode_executar){
            pthread_cond_wait(&pp->cond, &pp->mutex);
        }
        // tempo_atual += q;
        
        if(pp->tempo_restante <= 0){
            pp->finalizado = 1;
            clock_gettime(CLOCK_MONOTONIC, &now);
            pp->tf = (now.tv_sec - start_t.tv_sec) + ((now.tv_nsec - start_t.tv_nsec) / 1e9);
            processos_concluidos++;
            pp=NULL;
        } else{
            preempcoes++;
            // insert(&ready_queue, pp, 1);
        }
        pthread_mutex_unlock(&pp->mutex);
    }
    salvar_resultados(processos, num_processos);
}

void priority_scheduler(Processo* processos, int num_processos){ // aqui a base é a mesma do ao round_robin(), somente com quantums fixos e a prioridade é reavaliada na inserção na fila
    int tempo_atual = 0;
    int processos_concluidos = 0;
    Queue ready_queue ={NULL, NULL};
    int ct = 0;
    Processo* pp = NULL;
    struct timespec now;
    double act_time;

    while(processos_concluidos < num_processos && round(act_time) < 120){
        clock_gettime(CLOCK_MONOTONIC, &now);
        act_time = (now.tv_sec - start_t.tv_sec) +  ((now.tv_nsec - start_t.tv_nsec) / 1e9);
        while(ct < num_processos && processos[ct].t0 <= act_time){
            pthread_create(&processos[ct].thread_id, NULL, executar_processo, &processos[ct]);
            insert(&ready_queue, &processos[ct], 1);
            ct++;
        }
        if(pp!=NULL && pp->finalizado==0) insert(&ready_queue, pp, 1); // agora insiro o processo anterior só DEPOIS da nova batch 
        pp = pop(&ready_queue);
        if(pp == NULL){
            //tempo_atual++; // Avança o relógio virtual
            usleep(800000);
            continue;
        }

        pthread_mutex_lock(&pp->mutex);
        int margem = pp->deadline - tempo_atual;
        int q;

        if(margem <= pp->tempo_restante){
            q = pp->tempo_restante; 
        } else if(margem < MARGEM){
            q = QUANTUM * 2; 
        } else{
            q = QUANTUM;
        } if(q > pp->tempo_restante) q = pp->tempo_restante; // volta para tempo_restante caso o algoritmo tenha dado mais quantuns do que o necessário
        
        pp->tempo_rodar = q;
        pp->pode_executar = 1;
        pthread_cond_signal(&pp->cond);

        while(pp->pode_executar){
            pthread_cond_wait(&pp->cond, &pp->mutex);
        }
        tempo_atual += q;
        
        if(pp->tempo_restante <= 0){
            pp->finalizado = 1;
            clock_gettime(CLOCK_MONOTONIC, &now);
            pp->tf = (now.tv_sec - start_t.tv_sec) + ((now.tv_nsec - start_t.tv_nsec) / 1e9);
            processos_concluidos++;
            pp=NULL;
        } else{
            preempcoes++;
        }
        pthread_mutex_unlock(&pp->mutex);
    }
    salvar_resultados(processos, num_processos);
}

int main(int argc, char* argv[]){
    if(argc != 4){  // Não contém os 3 argumentos + o nome inicial(argv[0])
        perror("imesh: erro ao executar o simulador");
        exit(1);
    }
    Processo processos[MAX_PROCESSOS + 5]; // buffer com 5 de sentinela para não ter segfault
    arquivo_saida = argv[3];
    int ct=0; // contador da qtd. de processos
    FILE *fp = fopen(argv[2], "r");
    if(fp==NULL){   // Não acohu o arquivo de trace
        perror("Erro ao abrir arquivo de trace");
        exit(1);
    }
    char* linha = NULL;
    size_t len = 0;
    ssize_t lida;

    while((lida=getline(&linha, &len, fp))!= -1){
        // bufferização do processo(no array processos)+ criação das threads
        if(sscanf(linha, "%s %d %d %d", processos[ct].nome, &processos[ct].deadline, &processos[ct].t0, &processos[ct].dt)== 4){
            printf("Lido: %s | Deadline: %d | t0: %d | dt: %d\n", processos[ct].nome, processos[ct].deadline, processos[ct].t0, processos[ct].dt); // debug
            processos[ct].tempo_restante = processos[ct].dt;
            processos[ct].pode_executar = 0;
            processos[ct].finalizado = 0;
            pthread_mutex_init(&processos[ct].mutex, NULL);
            pthread_cond_init(&processos[ct].cond, NULL);
            // pthread_create(&processos[ct].thread_id, NULL, executar_processo, &processos[ct]); // agora vou criar as threads dentro do escalonador
            ct++;
        } else{ // erro caso alguma linha não tenha os 4 argumentos necessários
            printf("processo %s (linha %d) faltando argumentos", processos[ct].nome, ct);
            exit(1);
        }
    }

    sort(processos, ct); // ordena os processos por tempo de inicio (bubble sort mesmo)
    clock_gettime(CLOCK_MONOTONIC, &start_t); // agora captura o momento inicial (pois há busy-wait)
    // escolhe o processo baseado no argumento passado
    if(strcmp(argv[1], "1")==0)shortest_job_first(processos, ct);
    else if(strcmp(argv[1], "2")==0)round_robin(processos, ct);
    else if(strcmp(argv[1], "3")==0)priority_scheduler(processos, ct);
    else perror("Algoritmo não encontrado.");

    // após o escalonador terminar, espera as threads
    for(int i=0; i<ct; i++){
        pthread_join(processos[i].thread_id, NULL);
        pthread_mutex_destroy(&processos[i].mutex);
        pthread_cond_destroy(&processos[i].cond);
    }

    return 0;
}
