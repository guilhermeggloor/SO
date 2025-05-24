#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>

#define INT_SIZE sizeof(int)

typedef struct {
    int *vetor;
    int inicio;
    int fim;
    int *resultado;
    int indice;
} DadosThread;

void *soma_subvetor(void *arg) {
    DadosThread *dados = (DadosThread *)arg;
    int soma = 0;
    for (int i = dados->inicio; i < dados->fim; i++) {
        soma += dados->vetor[i];
    }
    dados->resultado[dados->indice] = soma;
    printf("Thread %d: soma dos indices [%d, %d] = %d\n", 
           dados->indice, dados->inicio, dados->fim - 1, soma);
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <arquivo> <n_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *nome_arquivo = argv[1];
    int n_threads = atoi(argv[2]);

    // Abre o arquivo
    int fd = open(nome_arquivo, O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Erro ao obter tamanho do arquivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int total_bytes = st.st_size;
    int total_numeros = total_bytes / INT_SIZE;

    // Aloca o vetor de inteiros
    int *vetor = malloc(total_bytes);
    if (!vetor) {
        perror("Erro ao alocar memória para vetor");
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (read(fd, vetor, total_bytes) != total_bytes) {
        perror("Erro ao ler arquivo");
        free(vetor);
        close(fd);
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Aloca o vetor de resultados parciais
    int *resultados = calloc(n_threads, sizeof(int));
    if (!resultados) {
        perror("Erro ao alocar vetor de resultados");
        free(vetor);
        exit(EXIT_FAILURE);
    }

    // Cria as threads
    pthread_t threads[n_threads];
    DadosThread dados[n_threads];

    for (int i = 0; i < n_threads; i++) {
        int inicio = i * total_numeros / n_threads;
        int fim = (i + 1) * total_numeros / n_threads;

        dados[i].vetor = vetor;
        dados[i].inicio = inicio;
        dados[i].fim = fim;
        dados[i].resultado = resultados;
        dados[i].indice = i;

        if (pthread_create(&threads[i], NULL, soma_subvetor, &dados[i]) != 0) {
            perror("Erro ao criar thread");
            free(vetor);
            free(resultados);
            exit(EXIT_FAILURE);
        }
    }

    // Espera todas as threads
    for (int i = 0; i < n_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    // Soma total e resultado esperado
    int soma_total = 0;
    for (int i = 0; i < n_threads; i++) {
        soma_total += resultados[i];
    }

    printf("Soma total: %d\n", soma_total);

    free(vetor);
    free(resultados);

    return 0;
}
