// Guilherme Garcia Gloor
// RGM: 45535

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h> // Para O_CREAT, O_EXCL
#include <stdbool.h>

// Estrutura do nó da lista encadeada para a L1,L2,L3
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Cabeças das listas globais
Node *L1 = NULL, *L2 = NULL, *L3 = NULL;

// Semáforos nomeados
sem_t *l1_count, *l2_count, *l3_count;
sem_t *l1_mutex, *l2_mutex, *l3_mutex;

// Flags para sinalizar o fim da produção em cada estágio
volatile bool main_done = false;
volatile bool filter_pairs_done = false;
volatile bool filter_primes_done = false;

// Nomes para os semáforos
const char* SEM_L1_COUNT = "/semL1Count";
const char* SEM_L2_COUNT = "/semL2Count";
const char* SEM_L3_COUNT = "/semL3Count";
const char* SEM_L1_MUTEX = "/semL1Mutex";
const char* SEM_L2_MUTEX = "/semL2Mutex";
const char* SEM_L3_MUTEX = "/semL3Mutex";


// Funções da Lista 
void insert_end(Node** head, int data) {
    Node* new_node = (Node*)malloc(sizeof(Node));
    new_node->data = data;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }
    Node* current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

int remove_begin(Node** head) {
    if (*head == NULL) {
        return -1; // Lista vazia
    }
    Node* temp = *head;
    int data = temp->data;
    *head = (*head)->next;
    free(temp);
    return data;
}

//  Funções das Threads 

// Thread 1: Filtra pares > 2 de L1 e insere em L2
void* thread_filter_pairs(void* arg) {
    while (!main_done || L1 != NULL) {
        sem_wait(l1_count); // Espera por um item em L1

        // Se a thread principal terminou e não há mais itens, pode sair
        if (main_done && L1 == NULL) {
            sem_post(l1_count); // Libera o semáforo para outra thread (se houver) ou para o final
            break;
        }

        sem_wait(l1_mutex);
        int num = remove_begin(&L1);
        sem_post(l1_mutex);

        if (num != -1) {
            if (!(num > 2 && num % 2 == 0)) { // Condição: não é par maior que 2
                sem_wait(l2_mutex);
                insert_end(&L2, num);
                sem_post(l2_mutex);
                sem_post(l2_count); // Sinaliza que há um novo item em L2
            }
        }
    }
    filter_pairs_done = true;
    sem_post(l2_count); // Garante que a próxima thread não fique presa esperando
    printf("-> Thread 1 (filtra pares) terminou.\n");
    return NULL;
}

// Função auxiliar para verificação dos números primos
bool is_prime(int n) {
    if (n <= 1) return false;
    for (int i = 2; i * i <= n; i++) {
        if (n % i == 0) return false;
    }
    return true;
}

// Thread 2: Filtra não primos de L2 e insere em L3
void* thread_filter_primes(void* arg) {
    while (!filter_pairs_done || L2 != NULL) {
        sem_wait(l2_count);

        if (filter_pairs_done && L2 == NULL) {
            sem_post(l2_count);
            break;
        }

        sem_wait(l2_mutex);
        int num = remove_begin(&L2);
        sem_post(l2_mutex);

        if (num != -1) {
            if (is_prime(num)) { // Condição: é primo
                sem_wait(l3_mutex);
                insert_end(&L3, num);
                sem_post(l3_mutex);
                sem_post(l3_count); // Sinaliza que há um novo item em L3
            }
        }
    }
    filter_primes_done = true;
    sem_post(l3_count); // Garante que a próxima thread não fique presa esperando
    printf("-> Thread 2 (filtra primos) terminou.\n");
    return NULL;
}

// Thread 3: Imprime os elementos de L3
// Thread 3: Imprime os elementos de L3 em um arquivo de saída
void* thread_print_primes(void* arg) {
    //Abre o arquivo de saída no modo de escrita
    FILE *outfile = fopen("in.out.txt", "w");
    if (outfile == NULL) {
        perror("Nao foi possivel criar o arquivo de saida");
        return NULL; // Retorna nulo e encerra a thread se não conseguir criar o arquivo
    }

    printf("--- Processando e gravando a saida em in.out.txt ---\n");
    while (!filter_primes_done || L3 != NULL) {
        sem_wait(l3_count);

        if (filter_primes_done && L3 == NULL) {
            sem_post(l3_count);
            break;
        }

        sem_wait(l3_mutex);
        int num = remove_begin(&L3);
        sem_post(l3_mutex);

        if (num != -1) {
            fprintf(outfile, "%d\n", num);
        }
    }

// fecha o arquivo no final
    fclose(outfile);
    printf("-> Thread 3 (impressão) terminou. Saida salva em in.out.txt.\n");
    return NULL;
}

// --- Função Principal ---
int main() {
    // Limpa semáforos de execuções anterior
    sem_unlink(SEM_L1_COUNT);
    sem_unlink(SEM_L2_COUNT);
    sem_unlink(SEM_L3_COUNT);
    sem_unlink(SEM_L1_MUTEX);
    sem_unlink(SEM_L2_MUTEX);
    sem_unlink(SEM_L3_MUTEX);

    // Inicializa semáforos de contagem com 0 
    l1_count = sem_open(SEM_L1_COUNT, O_CREAT, 0644, 0);
    l2_count = sem_open(SEM_L2_COUNT, O_CREAT, 0644, 0);
    l3_count = sem_open(SEM_L3_COUNT, O_CREAT, 0644, 0);

    // Inicializa semáforos de mutex com 1
    l1_mutex = sem_open(SEM_L1_MUTEX, O_CREAT, 0644, 1);
    l2_mutex = sem_open(SEM_L2_MUTEX, O_CREAT, 0644, 1);
    l3_mutex = sem_open(SEM_L3_MUTEX, O_CREAT, 0644, 1);

    if (l1_count == SEM_FAILED || l2_count == SEM_FAILED || l3_count == SEM_FAILED ||
        l1_mutex == SEM_FAILED || l2_mutex == SEM_FAILED || l3_mutex == SEM_FAILED) {
        perror("Falha ao criar semaforos");
        exit(EXIT_FAILURE);
    }
    
    pthread_t t1, t2, t3;

    printf("Iniciando threads...\n");
    pthread_create(&t1, NULL, thread_filter_pairs, NULL);
    pthread_create(&t2, NULL, thread_filter_primes, NULL);
    pthread_create(&t3, NULL, thread_print_primes, NULL);

    // Tarefa da Thread Principal: ler do arquivo e popular a L1
    FILE* file = fopen("in.txt", "r");
    if (!file) {
        perror("Nao foi possivel abrir o arquivo in.txt");
        exit(EXIT_FAILURE);
    }

    int num;
    printf("Lendo arquivo e populando L1...\n");
    while (fscanf(file, "%d", &num) == 1) {
        sem_wait(l1_mutex);   // Trava para modificar a L1
        insert_end(&L1, num);
        sem_post(l1_mutex);   // Libera
        sem_post(l1_count);   // Sinaliza que um novo item foi adicionado
    }
    fclose(file);
    
    main_done = true;
    sem_post(l1_count); // Sinaliza uma última vez para acordar a thread consumidora caso esteja esperando
    printf("-> Thread Principal (leitura) terminou.\n");


    // Espera as threads terminarem
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    printf("Todas as threads terminaram. Limpando recursos.\n");

    // Fecha e remove os semáforos do sistema
    sem_close(l1_count);
    sem_close(l2_count);
    sem_close(l3_count);
    sem_close(l1_mutex);
    sem_close(l2_mutex);
    sem_close(l3_mutex);
    
    sem_unlink(SEM_L1_COUNT);
    sem_unlink(SEM_L2_COUNT);
    sem_unlink(SEM_L3_COUNT);
    sem_unlink(SEM_L1_MUTEX);
    sem_unlink(SEM_L2_MUTEX);
    sem_unlink(SEM_L3_MUTEX);

    return 0;
}