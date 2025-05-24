#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#define INT_SIZE sizeof(int)

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <arquivo> <n_processos>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *nome_arquivo = argv[1];
    int n_processos = atoi(argv[2]);

    // Abre o arquivo
    int fd = open(nome_arquivo, O_RDONLY);
    if (fd == -1) {
        perror("Erro ao abrir o arquivo");
        exit(EXIT_FAILURE);
    }

    // Determina o tamanho do arquivo
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("Erro ao obter informações do arquivo");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int total_bytes = st.st_size;
    int total_numeros = total_bytes / INT_SIZE;

    // Cria a memória compartilhada para os números
    int shm_id_vetor = shmget(IPC_PRIVATE, total_bytes, IPC_CREAT | 0666);
    if (shm_id_vetor == -1) {
        perror("Erro ao criar memória compartilhada para vetor");
        close(fd);
        exit(EXIT_FAILURE);
    }

    int *vetor = (int *)shmat(shm_id_vetor, NULL, 0);
    if (vetor == (void *)-1) {
        perror("Erro ao mapear memória compartilhada");
        close(fd);
        shmctl(shm_id_vetor, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Le o arquivo para a memória compartilhada
    if (read(fd, vetor, total_bytes) != total_bytes) {
        perror("Erro ao ler o arquivo para memória");
        close(fd);
        shmdt(vetor);
        shmctl(shm_id_vetor, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }
    close(fd);

    // Cria a memória compartilhada para os resultados parciais
    int shm_id_resultados = shmget(IPC_PRIVATE, n_processos * sizeof(int), IPC_CREAT | 0666);
    if (shm_id_resultados == -1) {
        perror("Erro ao criar memória para resultados");
        shmdt(vetor);
        shmctl(shm_id_vetor, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    int *resultados = (int *)shmat(shm_id_resultados, NULL, 0);
    if (resultados == (void *)-1) {
        perror("Erro ao mapear memória de resultados");
        shmdt(vetor);
        shmctl(shm_id_vetor, IPC_RMID, NULL);
        shmctl(shm_id_resultados, IPC_RMID, NULL);
        exit(EXIT_FAILURE);
    }

    // Inicializa os resultados
    memset(resultados, 0, n_processos * sizeof(int));

    int i;
    for (i = 0; i < n_processos; i++) {
        pid_t pid = fork();
        if (pid == 0) { // processo filho
            int inicio = i * total_numeros / n_processos;
            int fim = (i + 1) * total_numeros / n_processos;
            int soma = 0;
            for (int j = inicio; j < fim; j++) {
                soma += vetor[j];
            }
            printf("Processo %d (PID %d): soma dos indices [%d, %d) = %d\n", 
                i, getpid(), inicio, fim - 1, soma);
            resultados[i] = soma;
            shmdt(vetor);
            shmdt(resultados);
            exit(EXIT_SUCCESS);
        } else if (pid < 0) {
            perror("Erro ao criar processo");
            exit(EXIT_FAILURE);
            // notifica o erro e sai, mas ainda continua criando os outros processos
        }
    }

    // Espera os filhos e compara i com numero de processos
    for (i = 0; i < n_processos; i++) {
        wait(NULL);
    }

    // Soma total e o resultado esperado
    int soma_total = 0;
    for (i = 0; i < n_processos; i++) {
        soma_total += resultados[i];
    }

    printf("Soma: %d\n", soma_total);

    // aqui vai limpar tudo
    shmdt(vetor);
    shmdt(resultados);
    shmctl(shm_id_vetor, IPC_RMID, NULL);
    shmctl(shm_id_resultados, IPC_RMID, NULL);

    return 0;
}