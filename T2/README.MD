Aluno: Guilherme Garcia Gloor
RGM: 45535

para compilação: make

para remover os arquivos: make clean

após o make: 
    - cria os arquivos objetos ./thread e ./processos 
    - pode rodar make com as opções abaixo.
    - processo: make 15_processo
    - thread:   make 15_thread
    - processo: make 8388607_processo
    - thread:   make 8388607_thread

para o processo.c: 
    ./processos (arquivo.bin) (numero de processos)
    ./processos 15 3

    resultado esperado da soma: 120

para thread.c: 
    ./thread (arquivo.bin) (numero de threads)
    ./thread 8388607 4

    resultadoo esperado da soma: -4194304


ambos os arquivos tem osa testes esperados da soma tanto em thread.c quanto processos.c, 
com 15 sendo 120 e 8388607 sendo -4194304.

## a saida esperada para processo com 15 e 3 é: 

Processo 0 (PID 41494): soma dos indices [0, 4] = 42
Processo 1 (PID 41495): soma dos indices [5, 9] = 40
Processo 2 (PID 41496): soma dos indices [10, 14] = 38
Soma: 120

## a saida esperada com thread com 15 e 3 é: 
Thread 1: soma dos indices [5, 9] = 40
Thread 2: soma dos indices [10, 14] = 38
Thread 0: soma dos indices [0, 4] = 42
Soma total: 120

## a saida esperada para com processo com 8388607 e 4 é: 
Processo 1 (PID 42412): soma dos indices [2097151, 4194302] = -1688443699
Processo 2 (PID 42413): soma dos indices [4194303, 6291454] = 475487869
Processo 3 (PID 42414): soma dos indices [6291455, 8388606] = 1577060772
Processo 0 (PID 42411): soma dos indices [0, 2097150] = -368299246
Soma: -4194304

## a saida esperada para com thread com 8388607 e 4 é: 
Thread 1: soma dos indices [2097151, 4194302] = -1688443699
Thread 2: soma dos indices [4194303, 6291454] = 475487869
Thread 3: soma dos indices [6291455, 8388606] = 1577060772
Thread 0: soma dos indices [0, 2097150] = -368299246
Soma total: -4194304
