// Guilherme Garcia Gloor
// RGM: 45535

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <dirent.h>
#include <ctype.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 100


// função para imprimir o tree PID
void print_tree_PID(pid_t root_pid, int level) {
    DIR *proc = opendir("/proc");
    struct dirent *entrada;
    pid_t pid, ppid;
    
    if (!proc) {
        perror("opendir /proc");
        return;
    }

    // armazenar os processos em uma struct
    typedef struct {
        pid_t pid;
        pid_t ppid;
    } Process;

    Process processes[4096]; //definindo limite de 4096 processos
    int process_count = 0;

    // Varre o diretório /proc
    while ((entrada = readdir(proc)) != NULL) {
        if (isdigit(entrada->d_name[0])) {
            pid = atoi(entrada->d_name);
            char path[256];
            snprintf(path, sizeof(path), "/proc/%d/stat", pid);

            FILE *stat = fopen(path, "r");
            if (stat) {
                int dummy;
                char comm[256];
                char state;
                fscanf(stat, "%d %s %c %d", &dummy, comm, &state, &ppid);
                fclose(stat);

                processes[process_count].pid = pid;
                processes[process_count].ppid = ppid;
                process_count++;
            }
        }
    }

    closedir(proc);

    // Imprimir a árvore recursivamente
    for (int i = 0; i < process_count; i++) {
        if (processes[i].ppid == root_pid) {
            // Indentação para mostrar hierarquia
            for (int j = 0; j < level; j++) {
                printf("  ");
            }
            printf("|- PID %d\n", processes[i].pid);
            // Chamada recursiva para filhos
            print_tree_PID(processes[i].pid, level + 1);
        }
    }
}


int main() {
    char comando[MAX_CMD_LEN];
    char *args[MAX_ARGS];
    
    while (1) {
        printf("$ "); // prompt simples
        fflush(stdout);
        
        if (fgets(comando, sizeof(comando), stdin) == NULL) {
            break; 
        }

        // Remove o '\n' final
        comando[strcspn(comando, "\n")] = '\0';

        // Ignorar linhas vazias
        if (strlen(comando) == 0) {
            continue;
        }

        // Verificação se o comando termina com o '&'
        int background = 0;
        if (comando[strlen(comando) - 1] == '&') {
            background = 1;
            comando[strlen(comando) - 1] = '\0'; // Remove o '&'
            // Também remove espaços sobrando, se tiver
            while (strlen(comando) > 0 && comando[strlen(comando) - 1] == ' ') {
                comando[strlen(comando) - 1] = '\0';
            }
        }

        // Quebra o comando em argumentos
        int i = 0;
        char *token = strtok(comando, " ");
        while (token != NULL && i < MAX_ARGS - 1) {
            args[i++] = token;
            token = strtok(NULL, " ");
        }
        args[i] = NULL; // Terminar lista de argumentos com NULL

        if (args[0] == NULL) {
            continue; // Nenhum comando digitado
        }

        if (strcmp(args[0], "exit") == 0) {
            printf("Saindo do terminal...\n");
            break;
        }

        pid_t pid = fork();

        if (pid < 0) {
            perror("fork");
            continue;
        }

        if (pid == 0) {
            // Processo filho: executa o comando
            if (execvp(args[0], args) == -1) {
                perror("execvp");
                exit(EXIT_FAILURE);
            }
        } else {
            // Processo pai
            if (!background) {
                waitpid(pid, NULL, 0); 
            } else {
                printf("Processo em background com PID %d\n", pid);
            }
        }

        if (strcmp(args[0], "tree") == 0 && args[1] != NULL) {
            pid_t root_pid = atoi(args[1]);
            print_tree_PID(root_pid, 0);
            continue;
        }


        
    }
    
    return 0;
}
