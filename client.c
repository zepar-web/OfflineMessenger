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
//gcc -Wall client.c -o client
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

    char buffer[MAX];
    char command[MAX];

    printf("Welcome to my Messenger\n");
    while (1)
    {
        fflush(stdin);
        fflush(stdout);
        bzero(command,strlen(command));
        scanf("%s",command);

        write(socketDesc, command, strlen(command));

        if(strcmp(command,"register")==0 || strcmp(command,"login")==0)
        {
            printf("Username:\n");
            char username[100];
            scanf("%s",username);

            printf("Password:\n");
            char password[100];
            scanf("%s",password);

            write(socketDesc,username,strlen(username));
            write(socketDesc,password,strlen(password));

            bzero(buffer,sizeof(buffer));
            bzero(username,strlen(username));
            bzero(password,strlen(password));

            read(socketDesc,&buffer,sizeof(buffer));

            printf("%s",buffer);

        }else if(strstr(command,"quit")){
            //break;
            printf("O sa murim!\n");
        }else{
            printf("Nu esti logat.\n");
            printf("Daca nu ai cont, foloseste comanda: register\n");
        }

        /*if (read(socketDesc, &buffer, sizeof(buffer)) <= 0)
        {
            perror("Eroare la read()\n");

            return errno;
        }
        printf("%s\n", buffer);*/
    }
}