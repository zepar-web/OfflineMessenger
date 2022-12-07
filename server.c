#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#define PORT 3059
#define MAXBUFFER 100

extern int errno;

typedef struct thData
{
    int idTh;
    int thDesc;
} thData;

static void *treat(void *);

void response(void *);

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in client2server;
    int socketDesc;
    pthread_t th[100];

    if ((socketDesc = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Eroare socket\n");

        return errno;
    }

    int sockopt = 1;
    setsockopt(socketDesc, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(sockopt));

    bzero(&server, sizeof(server));
    bzero(&client2server, sizeof(client2server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    if (bind(socketDesc, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("Eroare bind\n");

        return errno;
    }

    if (listen(socketDesc, 100) == -1)
    {
        perror("Eroare listen\n");

        return errno;
    }

    int i = 0;

    printf("Asteptam la portul %d..\n", PORT);
    while (1)
    {
        int client;
        thData *th;
        int len = sizeof(client2server);

        if ((client = accept(socketDesc, (struct sockaddr *)&client2server, &len)) < 0)
        {
            perror("Eroare accept\n");
            continue;
        }

        th = (struct thData *)malloc(sizeof(struct thData));
        th->idTh = i++;
        th->thDesc = client;

        pthread_create(&th[i], NULL, &treat, th);
    }
}

static void *treat(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    printf("[thread]- %d - Asteptam mesajul...\n", tdL.idTh);

    pthread_detach(pthread_self());

    response((struct thData *)arg);

    close((intptr_t)arg);
    return (NULL);
}
//to do register
void response(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    char buffer[MAXBUFFER];
    int count = 0;
    if (read(tdL.thDesc, &buffer, sizeof(buffer)) <= 0)
    {
        printf("[TH %d]\n", tdL.idTh);
        perror("Eroare la read()\n");
    }
    buffer[strlen(buffer)]='\0';
    printf("[Th id: %d] Mesaj de la comandant : %s\n", tdL.idTh, buffer);

    if (write(tdL.thDesc, "te salut", strlen("te salut")) <= 0)
    {
        printf("[TH %d]\n", tdL.idTh);
        perror("Eroare la write()\n");
    }
    else
    {
        printf("Mesaj trimis cu succes comandante, te pup si te respect!\n");
    }
}