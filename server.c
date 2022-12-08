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
#include <mysql/mysql.h>
// gcc -Wall server.c -o server `mysql_config --cflags --libs'
#define PORT 3076
#define MAXBUFFER 100

extern int errno;

typedef struct thData
{
    int idTh;
    int thDesc;
    int idUser;
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
// To do
void Register(int desc, thData th)
{
    printf("Am intrat in register\n");
    MYSQL *conn = mysql_init(NULL);
    //MYSQL_RES *res;
    //MYSQL_ROW row;

    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, "localhost", "root", "passwd", "offmess", 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(2);
    }

    char query[256];
    char username[100];
    char password[100];
    char *answer = (char *)malloc(50 * sizeof(char));

    read(desc, &username, sizeof(username));
    read(desc, &password, sizeof(password));
    printf("%s\n", username);
    printf("%s\n", password);

    if (verifyUser(username) == 0)
    {
        sprintf(query, "INSERT INTO users (name, password) VALUES ('%s','%s')", username, password);

        if (mysql_query(conn, query) != 0)
        {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(3);
        }
        else
        {
            strcpy(answer, "Te-ai inregistrat cu succes\n");
            mysql_close(conn);
        }
        if (write(desc, answer, strlen(answer)) <= 0)
        {
            printf("Eroare write register\n");
        }
    }
    else
    {
        write(desc, "Exista deja un utilizator cu acest nume!\n", 42);
    }

    fflush(stdin);
    fflush(stdout);
}

int verifyUser(char *name)
{
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, "localhost", "root", "passwd", "offmess", 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(2);
    }

    char query[256];
    sprintf(query, "SELECT * FROM users WHERE name = '%s'", name);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));

        return 1;
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (mysql_num_rows(result) > 0)
    {
        // name exist
        return 1;
    }
    else
    {
        // name does not exist
        return 0;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
}
// TO DO $$ login $$ ------------------------------------------
void Login(int desc, thData th)
{
    write(desc, "Te-ai logat cu succes\n", 22);
}
//----------------------------------------------------
void response(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    char buffer[MAXBUFFER];
    // int count = 0;
    while (1)
    {
        if (read(tdL.thDesc, &buffer, sizeof(buffer)) <= 0)
        {
            printf("[TH %d]\n", tdL.idTh);
            perror("Eroare la read()\n");
        }
        buffer[strlen(buffer)] = '\0';
        printf("[Th id: %d] Mesaj de la comandant : %s\n", tdL.idTh, buffer);

        if (strcmp(buffer, "register") == 0)
        {
            Register(tdL.thDesc, tdL);
        }
        else if (strcmp(buffer, "login") == 0)
        {
            Login(tdL.thDesc, tdL);
        }else if(strstr(buffer,"quit"))
        {
            printf("O sa murim\n");
        }
        bzero(buffer,sizeof(buffer));
    }

    /*if (write(tdL.thDesc, "te salut", strlen("te salut")) <= 0)
    {
        printf("[TH %d]\n", tdL.idTh);
        perror("Eroare la write()\n");
    }
    else
    {
        printf("Mesaj trimis cu succes comandante, te pup si te respect!\n");
    }*/
}