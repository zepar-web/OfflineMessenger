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
// gcc -Wall client.c -o client
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
    char username[100];
    char password[100];
    char userData[1000];
    int loginFlag = 0;

    printf("Welcome to my Messenger\n");
    while (1)
    {
        fflush(stdin);
        fflush(stdout);
        scanf("%s", command);

        if (loginFlag == 0)
        {
            if ((strcmp(command, "register") == 0 || strcmp(command, "login") == 0) && loginFlag == 0)
            {
                // bzero(command, strlen(command));
                write(socketDesc, command, strlen(command));

                printf("Sintaxa:nume parola\n");
                scanf(" %[^\n]s", userData);
                // fgets(userData,100,stdin);
                sscanf(userData, "%s %s", username, password);

                write(socketDesc, userData, strlen(userData));
                // write(socketDesc, password, strlen(password));

                bzero(buffer, sizeof(buffer));
                // bzero(username, strlen(username));
                // bzero(password, strlen(password));

                // int msgReceived = 0;
                read(socketDesc, &buffer, sizeof(buffer));

                if (strcmp(buffer, "Te-ai conectat cu succes!\n") == 0)
                {
                    loginFlag = 1;
                }
                // buffer[msgReceived]='\0';
                // printf("%i\n",loginFlag);
                printf("%s\n", buffer);
                fflush(stdout);
            }
            else if (strstr(command, "quit"))
            {
                write(socketDesc, command, strlen(command));
                // break;
                // printf("O sa murim!\n");
                exit(EXIT_SUCCESS);
            }
            else
            {
                printf("Nu esti logat.\nDaca nu ai cont foloseste comanda: register.\n");
                fflush(stdout);
            }
        }
        else if (strcmp(command, "logout") == 0 && loginFlag == 1)
        {
            write(socketDesc, command, strlen(command));

            bzero(buffer, sizeof(buffer));

            read(socketDesc, &buffer, sizeof(buffer));

            loginFlag = 0;

            printf("%s\n", buffer);
        }
        else if ((strcmp(command, "register") == 0 ||
                  strcmp(command, "login") == 0 ||
                  strcmp(command, "users") == 0) &&
                 loginFlag == 1)
        {
            write(socketDesc, command, strlen(command));

            bzero(buffer, sizeof(buffer));

            read(socketDesc, &buffer, sizeof(buffer));

            printf("%s\n", buffer);
        }
        else if (strstr(command, "quit"))
        {
            write(socketDesc, command, strlen(command));
            // break;
            // printf("O sa murim!\n");
            exit(EXIT_SUCCESS);
        }
        else
        {
            printf("Wrong command\n");
            fflush(stdout);
        }

        /*if (read(socketDesc, &buffer, sizeof(buffer)) <= 0)
        {
            perror("Eroare la read()\n");
            return errno;
        }
        printf("%s\n", buffer);*/
    }
}