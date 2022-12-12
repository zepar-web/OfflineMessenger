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
#define PORT 3004
#define MAXBUFFER 100
#define MYSQL_HOST "localhost"
#define MYSQL_USER "root"
#define MYSQL_PASSWORD "passwd"
#define MYSQL_DATABASE "offmess"
// gcc -Wall server.c -o server `mysql_config --cflags --libs'
//`mysql_config --cflags --libs'

// gcc -o server server.c $(mysql_config --cflags --libs)

extern int errno;

typedef struct thData
{
    int idTh;
    int thDesc;
    int idUser;
} thData;

thData *clients[100];
int clientNumber = 0;

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
        thData *td;
        socklen_t len = sizeof(client2server);

        if ((client = accept(socketDesc, (struct sockaddr *)&client2server, &len)) < 0)
        {
            perror("Eroare accept\n");
            continue;
        }

        td = (struct thData *)malloc(sizeof(struct thData));
        td->idTh = i++;
        td->thDesc = client;
        td->idUser = -1;
        clients[i] = td;
        clientNumber = i;

        pthread_create(&th[i], NULL, &treat, td);
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

int verifyUser(char name[50])
{
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, 0, NULL, 0))
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
int getUserID(char name[50])
{
    MYSQL *conn = mysql_init(NULL);

    int userID;

    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(2);
    }

    char query[256];

    sprintf(query, "SELECT id FROM users WHERE name='%s'", name);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (result == NULL)
    {
        printf("Eroare login!\n");
        return -1;
    }

    MYSQL_ROW row = mysql_fetch_row(result);

    if (row == NULL)
    {
        return -1;
    }

    userID = atoi(row[0]);
    // clients[th.idTh]->idUser = userId;

    mysql_free_result(result);
    mysql_close(conn);

    // printf("%i\n",userId);
    fflush(stdout);

    return userID;
}

void modifyLoginFlag(int id)
{
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(2);
    }

    char query[256];
    // char *answer = (char *)malloc(50 * sizeof(char));

    sprintf(query, "UPDATE users SET loginflag = 1 WHERE id = '%i'", id);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
    }

    mysql_close(conn);
}

void modifyLogoutFlag(int id)
{
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(2);
    }

    char query[256];
    // char *answer = (char *)malloc(50 * sizeof(char));

    sprintf(query, "UPDATE users SET loginflag = 0 WHERE id = '%i'", id);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
    }

    mysql_close(conn);
}

int Login(int desc, thData th)
{
    // write(desc, "Te-ai logat cu succes\n", 22);
    printf("Am intrat in LOGIN!\n");
    fflush(stdout);

    MYSQL *conn = mysql_init(NULL);
    // MYSQL *result;
    // MYSQL_ROW row;

    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(2);
    }

    char query[256];
    char username[50];
    char password[50];
    char userData[100];
    // char *answer = (char *)malloc(50 * sizeof(char));
    int userId;

    bzero(username, sizeof(username));
    bzero(password, sizeof(password));
    // bzero(answer, sizeof(answer));
    fflush(stdin);
    fflush(stdout);

    read(desc, &userData, sizeof(userData));
    // read(desc, &password, sizeof(password));
    sscanf(userData, "%s %s", username, password);
    printf("Name : %s\n", username);
    printf("Password : %s\n", password);
    fflush(stdout);

    if (verifyUser(username) == 1)
    {
        printf("am verificat\n");

        // modifyLoginFlag(username);

        sprintf(query, "SELECT id, loginflag = 1 FROM users WHERE name='%s' AND password='%s'", username, password);

        if (mysql_query(conn, query))
        {
            fprintf(stderr, "%s\n", mysql_error(conn));

            return -1;
        }

        MYSQL_RES *result = mysql_store_result(conn);

        if (result == NULL)
        {
            printf("Eroare login!\n");
            return -1;
        }

        MYSQL_ROW row = mysql_fetch_row(result);

        if (row == NULL)
        {
            return -1;
        }

        userId = atoi(row[0]);
        // clients[th.idTh]->idUser = userId;

        mysql_free_result(result);
        mysql_close(conn);

        // printf("%i\n",userId);
        fflush(stdout);

        return userId;

        // if (!(row = mysql_fetch_row(result)))
        // {
        //     return -1;
        // }
        // else
        // {
        //     userId = atoi(row[0]);
        //     clients[th.idTh]->idUser = userId;

        //     // write(desc, "Te-ai logat cu succes!", 23);
        //     mysql_free_result(result);

        //     mysql_close(conn);

        //     return userId;
        // }
    }
    else
    {
        // write(desc, "Nume/parola gresit.\n", 21);
        return -1;
    }

    mysql_close(conn);

    return -1;
}

void Register(int desc, thData th)
{
    printf("Am intrat in register\n");
    fflush(stdout);

    MYSQL *conn = mysql_init(NULL);
    // MYSQL_RES *res;
    // MYSQL_ROW row;

    char query[256];
    char username[100];
    char password[100];
    char userData[100];
    char *answer = (char *)malloc(50 * sizeof(char));
    // int userLen = 0;
    // int passLen = 0;

    // bzero(username, sizeof(username));
    // bzero(password, sizeof(password));
    // bzero(answer, sizeof(answer));

    if (read(desc, &userData, sizeof(userData)) < 0)
    {
        perror("Eroare read username\n");
    }
    // if (read(desc, &password, sizeof(password)) < 0)
    // {
    //     perror("Eroare read password\n");
    // }
    // username[userLen]='\0';
    // password[passLen]='\0';
    sscanf(userData, "%s %s", username, password);

    printf("Name: %s\n", username);
    printf("Password: %s\n", password);
    fflush(stdout);
    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        exit(2);
    }

    if (verifyUser(username) == 0)
    {
        sprintf(query, "INSERT INTO users (name, password) VALUES ('%s','%s')", username, password);

        if (mysql_query(conn, query) != 0)
        {
            fprintf(stderr, "%s\n", mysql_error(conn));
            exit(3);
            mysql_close(conn);
        }
        else
        {
            strcpy(answer, "Te-ai inregistrat cu succes\n");
            mysql_close(conn);
        }
    }
    else
    {
        strcpy(answer, "Exista deja un utilizator cu acest nume!\n");
        mysql_close(conn);
    }
    write(desc, answer, strlen(answer));
}
// TO DO showUsers----------------------------
char *showUsers(thData th)
{
    MYSQL *conn = mysql_init(NULL);

    if (conn == NULL)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }

    if (!mysql_real_connect(conn, MYSQL_HOST, MYSQL_USER, MYSQL_PASSWORD, MYSQL_DATABASE, 0, NULL, 0))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
        exit(2);
    }

    char query[5000];
    strcpy(query, "SELECT name,loginflag FROM users");

    if (mysql_query(conn, query) != 0)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(3);
        mysql_close(conn);
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (result == NULL)
    {
        printf("Eroare showUsers!\n");
    }

    int num_fields = mysql_num_fields(result);
    char *tables =malloc(10000 * sizeof(char*));

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)))
    {
        for (int i = 0; i < num_fields; i++)
        {
            strcat(tables, row[i]);
            strcat(tables, " ");
        }
        strcat(tables,"\n");
    }
    mysql_free_result(result);
    mysql_close(conn);

    // printf("%i\n",userId);
    fflush(stdout);

    return tables;
}
//---------------------------------
void response(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    char buffer[MAXBUFFER];
    // char answer[MAXBUFFER];
    int loginFlag = 0;
    // int count = 0;
    while (1)
    {
        // bzero(buffer,sizeof(buffer));

        if (read(tdL.thDesc, &buffer, sizeof(buffer)) <= 0)
        {
            printf("[TH %d]\n", tdL.idTh);
            perror("Eroare la read()\n");
        }
        buffer[strlen(buffer)] = '\0';
        printf("[Th id: %d] Mesaj de la comandant : %s\n", tdL.idTh, buffer);
        fflush(stdout);

        if (tdL.idUser <= -1)
        {
            // printf("%i\n", tdL.idUser);
            if (strcmp(buffer, "register") == 0)
            {
                Register(tdL.thDesc, tdL);
            }
            else if (strncmp(buffer, "login", 6) == 0)
            {
                loginFlag = Login(tdL.thDesc, tdL);
                if (loginFlag == -1)
                {
                    write(tdL.thDesc, "Nume/parola gresit.\n", 21);
                }
                else
                {
                    tdL.idUser = loginFlag;
                    modifyLoginFlag(loginFlag);
                    // clients[tdL.idTh]->idUser = loginFlag;
                    write(tdL.thDesc, "Te-ai conectat cu succes!\n", 27);
                    // write(tdL.thDesc, "Nume/parola gresit.\n", 21);
                }
            }
            else if (strstr(buffer, "quit"))
            {
                printf("O sa murim\n");
                exit(EXIT_SUCCESS);
            }
        }
        else
        {
            printf("Id-ul tau:%i\n", tdL.idUser);
            if (strcmp(buffer, "register") == 0 || strcmp(buffer, "login") == 0)
            {

                write(tdL.thDesc, "Esti deja conectat!", 20);
            }
            else if (strcmp(buffer, "logout") == 0)
            {
                modifyLogoutFlag(loginFlag);
                tdL.idUser = -1;
                write(tdL.thDesc, "Te-ai deconectat cu succes!", 28);
            }
            else if (strcmp(buffer, "users") == 0)
            {
                // showUsers(tdL);
                // strcpy(answer, showUsers(tdL));
                write(tdL.thDesc, showUsers(tdL), strlen(showUsers(tdL)));
            }
            else if (strstr(buffer, "quit"))
            {
                printf("O sa murim\n");
                exit(EXIT_SUCCESS);
            }
        }
        // fflush(stdout);
        // fflush(stdin);
        bzero(buffer, sizeof(buffer));
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