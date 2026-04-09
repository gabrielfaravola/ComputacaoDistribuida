CC     = gcc
FLAGS  = -Wall -Wextra -std=c11

all: servidor cliente

servidor: servidor.c jogo.c
	$(CC) $(FLAGS) -o servidor servidor.c jogo.c -lpthread

cliente: cliente.c jogo.c
	$(CC) $(FLAGS) -o cliente cliente.c jogo.c

clean:
	rm -f servidor cliente
