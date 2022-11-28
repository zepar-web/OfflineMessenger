#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>

#define PORT 3059
extern int errno;

void socketMaker(int sd)
{
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
        perror("Eroare socket!\n");
}

void serverConnection(int sd, struct sockaddr_in server)
{
    if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        perror("Eroare connect[client]");
}

void socketBind(int sd, struct sockaddr_in server)
{
    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
        perror("eroare bind[server]\n");
}
char *conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    /* adresa IP a clientului */
    strcpy(str, inet_ntoa(address.sin_addr));
    /* portul utilizat de client */
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}

int main()
{
    struct sockaddr_in server;
    struct sockaddr_in from;

    fd_set readDesc;
    fd_set activDesc;

    struct timeval tmp;

    int socketDesc, client;
    int optTmp = 1;

    int parcDesc, maxDesc, length;

    socketMaker(socketDesc);

    setsockopt(socketDesc, SOL_SOCKET, SO_REUSEADDR, &optTmp, sizeof(optTmp));

    bzero(&server, sizeof(server));

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    socketBind(socketDesc, server);

    if (listen(socketDesc, 5) == -1)
        perror("eroare listen[parent]");

    FD_ZERO(&activDesc);
    FD_SET(socketDesc, &activDesc);

    tmp.tv_sec = 1;
    tmp.tv_usec = 0;

    maxDesc = socketDesc;

    printf("[server]Waiting at : %d\n", PORT);
    fflush(stdout);

    char buffer[100];
    int bytes;
    while (1)
    {
        bcopy((char *)&activDesc, (char *)&readDesc, sizeof(readDesc));

        if (select(maxDesc + 1, &readDesc, NULL, NULL, &tmp) < 0)
            perror("[server]eroare select");

        if (FD_ISSET(socketDesc, &readDesc))
        {
            length = sizeof(from);
            bzero(&from, sizeof(from));

            client = accept(socketDesc, (struct sockadder*)&from,&length);

            if(client<0)
                perror("[server]eroare accept");

            if(maxDesc<client)
                maxDesc=client;

            FD_SET(client,&activDesc);

            printf("[server]%s s-a conectat.",conv_addr(from));
            fflush(stdout);
        }
        for(parcDesc=0;parcDesc<=maxDesc;parcDesc++)
        {
            if(parcDesc!=socketDesc && FD_ISSET(parcDesc,&readDesc))
            {
                bytes=read(parcDesc,buffer,sizeof(buffer));

                if(strcmp(buffer,"login")==0)
                {
                    write(parcDesc,"connected",strlen("connected"));
                }
            }
        }
    }
}
