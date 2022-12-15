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
#define PORT 3005
#define MAXBUFFER 100
#define MYSQL_HOST "localhost"
#define MYSQL_USER "root"
#define MYSQL_PASSWORD "passwd"
#define MYSQL_DATABASE "offmess"
// gcc -Wall server.c -o server `mysql_config --cflags --libs'

// gcc -o server server.c $(mysql_config --cflags --libs)

extern int errno;

typedef struct thData
{
    int idTh;
    int thDesc;
    int idUser;
} thData;

thData *clients[50];
int clientNumbers = 0;

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

    if (listen(socketDesc, 50) == -1)
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
        clients[i] = td;
        td->idTh = i++;
        td->thDesc = client;
        td->idUser = -1;
        clientNumbers = i;

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

int verifyUser(char name[])
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
        mysql_close(conn);
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (mysql_num_rows(result) > 0)
    {
        // name exist
        mysql_close(conn);
        return 1;
    }
    else
    {
        // name does not exist
        mysql_close(conn);
        return 0;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return 0;
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

char *showUsers()
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

    char query[256];
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
    char *tables = malloc(1000 * sizeof(char *));

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)))
    {
        for (int i = 0; i < num_fields; i++)
        {
            strcat(tables, row[i]);
            strcat(tables, " ");
        }
        strcat(tables, "\n");
    }
    mysql_free_result(result);
    mysql_close(conn);

    // printf("%i\n",userId);
    fflush(stdout);

    // printf("*%s*\n",tables);

    return tables;
}

void *insertOnlineMesssageIntoDataBase(int sender, int receiver, char message[])
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

    char query[256];

    sprintf(query, "INSERT INTO messages (id_sender, id_receiver, is_read, message) VALUES (%i, %i, 1, '%s')", sender, receiver, message);

    if (mysql_query(conn, query) != 0)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(3);
        mysql_close(conn);
    }

    mysql_close(conn);
}

void *insertOfflineMesssageIntoDataBase(int sender, int receiver, char message[])
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

    char query[256];

    sprintf(query, "INSERT INTO messages (id_sender, id_receiver, is_read, message) VALUES (%i, %i, 0, '%s')", sender, receiver, message);

    if (mysql_query(conn, query) != 0)
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(3);
        mysql_close(conn);
    }

    mysql_close(conn);
}

int getUserId(char name[])
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
    char query[256];
    int userID = 0;

    sprintf(query, "SELECT id FROM users WHERE name='%s'", name);

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

    userID = atoi(row[0]);
    // clients[th.idTh]->idUser = userId;

    mysql_free_result(result);
    mysql_close(conn);

    // printf("%i\n",userId);
    fflush(stdout);

    return userID;
}

bool isOnline(char name[])
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
    sprintf(query, "SELECT * FROM users WHERE name = '%s' and loginflag = 1", name);

    if (mysql_query(conn, query))
    {
        fprintf(stderr, "%s\n", mysql_error(conn));
        mysql_close(conn);
    }

    MYSQL_RES *result = mysql_store_result(conn);

    if (mysql_num_rows(result) >= 1)
    {
        // is online
        mysql_close(conn);
        return true;
    }
    else
    {
        // is offline
        mysql_close(conn);
        return false;
    }

    mysql_free_result(result);
    mysql_close(conn);

    return false;
}

char *getNameById(int id)
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

    char query[256];
    sprintf(query, "SELECT name FROM users WHERE id = %i", id);

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
    char *tables = malloc(1000 * sizeof(char *));

    MYSQL_ROW row;

    while ((row = mysql_fetch_row(result)))
    {
        for (int i = 0; i < num_fields; i++)
        {
            strcat(tables, row[i]);
            strcat(tables, " ");
        }
        // strcat(tables, "\n");
    }
    mysql_free_result(result);
    mysql_close(conn);

    // printf("%i\n",userId);
    fflush(stdout);

    // printf("*%s*\n",tables);

    return tables;
}

void sendMessage(int desc, thData th, char buffer[])
{
    printf("am intrat in sendmsg\n");
    char nume[100] = " ";
    char comanda[100] = " ";
    char messageRead[500] = " ";
    char warning[5000] = " ";
    char toSend[1000] = " ";
    int clientDesc;

    sscanf(buffer, "%s %s", comanda, nume);
    printf("%s\n", nume);

    if (verifyUser(nume) == 1)
    {
        printf("am verificat\n");
        if (isOnline(nume) == true)
        {
            write(desc, "esteOnline", 10);

            read(desc, messageRead, sizeof(messageRead));

            sprintf(toSend, "Mesaj de la %s: ", getNameById(th.idUser));

            strcat(toSend, messageRead);

            printf("%s\n", messageRead);

            for (int i = 0; i < clientNumbers; i++)
            {
                if (clients[i]->idUser == getUserId(nume))
                {
                    clientDesc = clients[i]->thDesc;
                    // printf("%i\n", clientDesc);
                    // printf("Am gasit\n");
                    write(clientDesc, toSend, strlen(toSend));
                    insertOnlineMesssageIntoDataBase(th.idUser, getUserId(nume), messageRead);
                    printf("Am trimis\n");
                }
            }
        }
        else if (isOnline(nume) == false)
        {
            sprintf(warning, "Utilizatorul '%s' nu este online.\nMesajul trimis o sa il primeasca cand se conecteaza!", nume);
            write(desc, warning, strlen(warning));

            read(desc, messageRead, sizeof(messageRead));

            printf("%s\n", messageRead);

            insertOfflineMesssageIntoDataBase(th.idUser, getUserId(nume), messageRead);
        }
    }
    else
    {
        sprintf(warning, "Nu exista niciun utilizator cu numele '%s'.", nume);
        write(desc, warning, strlen(warning));
    }
}

char *showMessageHistory(char buffer[], int idUser)
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

    char comanda[100];
    char nume[100];
    char query[256];

    sscanf(buffer, "%s %s", comanda, nume);

    sprintf(query, "SELECT id_sender,message FROM messages WHERE (id_sender = %i AND id_receiver = %i) OR (id_sender = %i AND id_receiver = %i)", idUser, getUserId(nume), getUserId(nume), idUser);

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
    // char *returnTable = malloc(1000 * sizeof(char *));

    MYSQL_ROW row;

    char *table = malloc(1000 * sizeof(char *));
    bzero(table, sizeof(table));
    strcat(table, "Istoric mesaje!\n");
    while ((row = mysql_fetch_row(result)))
    {
        strcat(table, "Mesaj de la: ");
        for (int i = 0; i < num_fields; i++)
        {
            if (i == 0)
            {
                strcat(table, getNameById(atoi(row[0])));
                strcat(table, "--->");
            }
            else
            {
                strcat(table, row[1]);
            }
        }
        strcat(table, "\n");
    }
    mysql_free_result(result);
    mysql_close(conn);

    // printf("%i\n",userId);
    fflush(stdout);

    // printf("%s\n",table);

    // printf("%s\n", finalTables);
    // strcpy(returnTable,table);

    return table;
}

void response(void *arg)
{
    struct thData tdL;
    tdL = *((struct thData *)arg);

    char buffer[MAXBUFFER];
    char buffMSG[MAXBUFFER];
    char *answer;
    // char answer[MAXBUFFER];
    int idLogin = 0;
    // int count = 0;
    while (1)
    {
        // bzero(buffer,sizeof(buffer));

        if (read(tdL.thDesc, &buffer, sizeof(buffer)) <= 0)
        {
            printf("[TH %d]\n", tdL.idTh);
            perror("Eroare la read()\n");
            // close(tdL.thDesc);
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
                idLogin = Login(tdL.thDesc, tdL);
                if (idLogin == -1)
                {
                    write(tdL.thDesc, "Nume/parola gresit.\n", 21);
                }
                else
                {
                    tdL.idUser = idLogin;
                    modifyLoginFlag(idLogin);
                    printf("%i\n", tdL.thDesc);
                    write(tdL.thDesc, "Te-ai conectat cu succes!\n", 27);
                    clients[tdL.idTh]->idUser = idLogin;
                    //  write(tdL.thDesc, "Nume/parola gresit.\n", 21);
                }
            }
            else if (strstr(buffer, "quit"))
            {
                // printf("O sa murim\n");
                pthread_exit(NULL);
                // exit(EXIT_SUCCESS);
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
                modifyLogoutFlag(idLogin);
                tdL.idUser = -1;
                write(tdL.thDesc, "Te-ai deconectat cu succes!", 28);
            }
            else if (strcmp(buffer, "users") == 0)
            {
                // showUsers(tdL);
                // strcpy(answer, showUsers(tdL));
                write(tdL.thDesc, showUsers(), strlen(showUsers()));
            }
            else if (strncmp(buffer, "sendmsgto", 9) == 0)
            {
                // read(tdL.thDesc,&buffMSG,sizeof(buffMSG));
                sendMessage(tdL.thDesc, tdL, buffer);
                // sendMessage(tdL.thDesc, tdL, buffer);
                // answer = sendMessage(buffMSG);
                // printf("%s\n",answer);
                // write(tdL.thDesc, sendMessage(tdL.thDesc), strlen(sendMessage(tdL.thDesc)));
            }
            else if (strncmp(buffer, "msghistory", 10) == 0)
            {
                write(tdL.thDesc, showMessageHistory(buffer, tdL.idUser), strlen(showMessageHistory(buffer, tdL.idUser)));
            }
            else if (strstr(buffer, "quit"))
            {
                // printf("O sa murim\n");
                pthread_exit(NULL);
                // exit(EXIT_SUCCESS);
            }
        }
        // fflush(stdout);
        // fflush(stdin);
        bzero(buffer, sizeof(buffer));
    }
}