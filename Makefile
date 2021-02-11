CC = gcc -Wall 

all : main serveur client cuisinier

main : main.c types.h helpers.o
	$(CC) main.c -o main helpers.o

helpers.o : helpers.c
	gcc -c helpers.c

client : client.c helpers.h types.h helpers.o
	$(CC) client.c -o client helpers.o 

serveur : serveur.c helpers.h types.h helpers.o
	$(CC) serveur.c -o serveur helpers.o

cuisinier : cuisinier.c helpers.h types.h helpers.o
	$(CC) cuisinier.c -o cuisinier helpers.o

clean :
	rm -f *.o
