CC = gcc
CFLAGS = -Wall -O2

# Alvos
all: processos thread

processos: processos.c	# compila o arquivo de processos.c gerando o arquivo processos
	$(CC) $(CFLAGS) -o processos processos.c

thread: thread.c	# compila o arquivo de thread.c gerando o arquivo thread
	$(CC) $(CFLAGS) -o thread thread.c -pthread	

15_processo:	# roda o arquivo processo com make e usa o arquivo binario 15
	./processos 15 3

15_thread:	# roda o arquivo thread com make e usa o arquivo binario 15
	./thread 15 3

8388607_processo:   # roda o arquivo processo com make e usa o arquivo binario 8388607
	./processos 8388607 4

8388607_thread:   # roda o arquivo thread com make e usa o arquivo binario 8388607
	./thread 8388607 4

clean:
	rm -f processos thread
