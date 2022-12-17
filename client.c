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
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <time.h>

// gcc -Wall client.c -o client -pthread

extern int errno;
int port;
#define MAX 1000

int loginFlag = 0;
pthread_t th;

void offlineMesseges(int desc)
{
    write(desc, "offMess", 7);
}

void *msgTh(void *socket_desc)
{

    int sock = *(int *)socket_desc;
    char buffer[MAX];
    char message[MAX];
    while (1)
    {

        bzero(buffer, sizeof(buffer));

        if (read(sock, buffer, MAX) <= 0)
        {
            printf("eroare\n");
            fflush(stdout);
        }

        if (strcmp(buffer, "Te-ai deconectat cu succes!") == 0)
        {
            loginFlag = 0;
        }
        else if (strcmp(buffer, "Te-ai conectat cu succes!\n") == 0)
        {
            loginFlag = 1;
            offlineMesseges(sock);
        }
        else if (strncmp(buffer, "esteOnline", 10) == 0)
        {
            printf("Mesaj:\n");

            scanf(" %[^\n]s", message);

            write(sock, message, strlen(message));

            printf("Mesaj trimis\n");

            bzero(buffer, sizeof(buffer));

            // pthread_kill(th, SIGUSR2);
            //  exit(1);
        }
        else if (strncmp(buffer, "Ai parasit aplicatia cu succes!", 31) == 0)
        {
            // printf("Ai parasit aplicatia cu succes!\n");
            // exit(EXIT_SUCCESS);
        }
        else if (strncmp(buffer, "replyFlag", 9) == 0)
        {
            printf("Mesaj:\n");

            scanf(" %[^\n]s", message);

            write(sock, message, strlen(message));

            printf("Mesaj trimis\n");

            bzero(buffer, sizeof(buffer));

            // pthread_kill(th, SIGUSR2);
            //  exit(1);
        }
        else if (strncmp(buffer, "Utilizatorul", 12) == 0)
        {
            printf("Mesaj:\n");

            scanf(" %[^\n]s", message);

            write(sock, message, strlen(message));

            printf("Mesaj trimis\n");
        }

        printf("%s\n", buffer);

        fflush(stdout);
        fflush(stdin);
    }
    pthread_exit(NULL);
}

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
    // char command2[MAX];
    char username[100];
    char password[100];
    char userData[1000];

    printf("Welcome to my Messenger\n");

    while (1)
    {
        fflush(stdin);
        fflush(stdout);

        // scanf("%s", command);
        int sock2 = socketDesc;
        pthread_create(&th, NULL, &msgTh, (void *)&sock2);

        bzero(command, sizeof(command));

        scanf(" %[^\n]s", command);

        if (loginFlag == 0)
        {
            if (strcmp(command, "register") == 0 || strcmp(command, "login") == 0)
            {
                write(socketDesc, command, strlen(command));

                printf("Sintaxa:nume parola\n");
                scanf(" %[^\n]s", userData);

                sscanf(userData, "%s %s", username, password);

                write(socketDesc, userData, strlen(userData));
            }
            else if (strstr(command, "quit") != 0)
            {
                write(socketDesc, command, strlen(command));
                sleep(1);
                exit(EXIT_SUCCESS);
            }
            else
            {
                printf("Nu esti logat.\nDaca nu ai cont foloseste comanda: register.\n");
                fflush(stdout);
            }
        }
        else
        {
            if (strcmp(command, "logout") == 0)
            {
                write(socketDesc, command, strlen(command));
            }
            else if (strcmp(command, "register") == 0 ||
                     strcmp(command, "login") == 0 ||
                     strcmp(command, "users") == 0 ||
                     strncmp(command, "msghistory", 10) == 0 ||
                     strncmp(command, "setnameto", 9) == 0 ||
                     strncmp(command, "setpassto", 9) == 0)
            {
                write(socketDesc, command, strlen(command));
                // sleep(1);
            }
            else if (strncmp(command, "sendmsgto", 9) == 0)
            {
                write(socketDesc, command, strlen(command));

                sleep(2);
            }
            else if (strncmp(command, "reply", 5) == 0)
            {
                write(socketDesc, command, strlen(command));

                sleep(2);
            }
            else if (strstr(command, "quit") != 0)
            {
                write(socketDesc, command, strlen(command));
                // sleep(2);
                // exit(EXIT_SUCCESS);
            }
            else
            {
                printf("WRONG COMMAND!!\n");
                printf("Comenzi disponibile!\n");
                printf("1.users\n2.msghistory <name>\n3.sendmsgto <name>\n4.reply <name> <messageId>\n5.setnameto <name>\n6.setpassto <name>\n7.logout\n8.quit\n\n");
                fflush(stdout);
            }
        }
    }
    close(socketDesc);
}