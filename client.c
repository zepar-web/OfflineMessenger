#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

extern int errno;
int port;
#define MAX 100

int main(int argc, char *argv[])
{
    int socketDesc;            // descriptor socket
    struct sockaddr_in server; // struct pentru connect

    if (argc != 3)
    {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    }

    port = atoi(argv[2]);

    if ((socketDesc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare socket!\n");

        return errno;
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = inet_addr(argv[1]);
    server.sin_port = htons(port);

    if (connect(socketDesc, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    char command[MAX];
    char buffer[MAX];

    printf("Welcome to my Messenger\n");
    while (1)
    {
        //fflush(stdin);
        //fflush(stdout);

        scanf("%[^\n]",command);

        write(socketDesc, command, strlen(command));

        if (read(socketDesc, &buffer, sizeof(buffer)) <= 0)
        {
            perror("Eroare la read()\n");

            return errno;
        }
        printf("%s\n", buffer);
    }
}