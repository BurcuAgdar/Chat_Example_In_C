#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <pthread.h>
#include <semaphore.h>

void *Chats(void *);

typedef struct
{
    char phoneNumber[20];
    int sockfd;
    struct sockaddr_in clientAddress;
    char groupID[50]; 
} clientObject;

typedef struct
{
    char groupName[50];
    int prive;          
    int password;
    int groupmembersIndex;        
    int groupMembetsSockets[100]; 
} group;

clientObject *clients[1000];
group *groups[500];
static int clientIndex = 0;
static int groupIndex = 0;
sem_t comLock;

int main(int argc, char *argv[])
{
    int socket_desc, client_sock, c;
    clientObject *new_sock = (clientObject *)malloc(sizeof(clientObject));
    struct sockaddr_in server, client;
    sem_init(&comLock, 0, 1);
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        printf("Socket Error.");
    }
    puts("Socket created");
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(3205);
    if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
        perror("Error for bind");
        return 1;
    }
    puts("*_*    *_*");
    listen(socket_desc, 10);
    puts("Welcome to Server.Clients are waited...");
    c = sizeof(struct sockaddr_in);

    while ((client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c)))
    {
        printf("Connection accepted from %s:%d\n", inet_ntoa(client.sin_addr), ntohs(client.sin_port));

        clientObject *cli = (clientObject *)malloc(sizeof(clientObject));

        char phonenumTemp[20];
        recv(client_sock, phonenumTemp, 20, 0);

        cli->clientAddress = client;
        cli->sockfd = client_sock;
        strcat(cli->phoneNumber, phonenumTemp);
        bzero(phonenumTemp,sizeof(phonenumTemp));
        clients[clientIndex] = cli;
        clientIndex++;

        pthread_t sniffer_thread;
        if (pthread_create(&sniffer_thread, NULL, (void *)Chats, (void *)cli) < 0)
        {
            perror("error for thread");
            return 1;
        }
    }

    return 0;
}

void *Chats(void *socket_desc)
{
    clientObject *sock = (clientObject *)socket_desc;
    char buffer[512];
    while (1)
    {

        recv(sock->sockfd, buffer, 512, 0);
        if (strlen(buffer) > 0)
        {
            if (strncmp(buffer, "-exit", 5) == 0 && strlen(buffer) == 6)
            {
               
                break;
            }
            else if (strncmp(buffer, "-whoami", 7) == 0)
            {
                char phoneTemp[50];
                bzero(phoneTemp, sizeof(phoneTemp));
                for (int i = 0; i < clientIndex; i++)
                {
                    if (clients[i]->sockfd == sock->sockfd)
                    {   strcat(phoneTemp,"Your phone number: ");
                        strcat(phoneTemp, clients[i]->phoneNumber);
                        strcat(phoneTemp, "\n");
                        write(clients[i]->sockfd, phoneTemp, strlen(phoneTemp));
                        bzero(phoneTemp, sizeof(phoneTemp));
                        bzero(buffer, sizeof(buffer));
                        break;
                    }
                }
            }
            else if (strncmp(buffer, "-exit", 5) == 0 && strlen(buffer) != 6)
            {
                int i = 5;
                while (buffer[i] == ' ')
                {
                    i++;
                }
                char groupTempName[50];
                int indexer = 0;
                for (i; i < strlen(buffer); i++)
                {
                    if (buffer[i] == '\n')
                    {
                        break;
                    }
                    groupTempName[indexer] = buffer[i];
                    indexer++;
                }
                for (i = 0; i < groupIndex; i++)
                {
                    if (strcmp(groupTempName, groups[i]->groupName) == 0)
                    {
                        for (indexer = 0; indexer < groups[i]->groupmembersIndex; indexer++)
                        {
                            if (groups[i]->groupMembetsSockets[indexer] == sock->sockfd)
                            {
                                char tt[50] = "\0";
                                strcpy(sock->groupID, tt);
                                groups[i]->groupMembetsSockets[indexer] = -1;
                                strcat(tt, "left from group\n");
                                write(sock->sockfd, tt, strlen(tt));
                                break;
                            }
                        }
                        break;
                    }
                }
            }
            else if (strncmp(buffer, "{", 1) == 0)
            {
                sem_wait(&comLock);
                char fromwho[50];
                char toGroup[50];
                char messagetoGroup[256];
                char temproryMessage[256];
                int q = 0;
                int slashcount = 0;
                bzero(messagetoGroup,sizeof(messagetoGroup));
                 bzero(temproryMessage,sizeof(temproryMessage));
                while (buffer[q] != '\0')
                {
                    
                    if (slashcount == 1)
                    {
                        int m = 0;
                        while (buffer[q] != ':')
                        {
                            q++;
                        }
                        q++;
                        while (buffer[q] != '\n')
                        {
                            if(buffer[q]!=','){
                            fromwho[m] = buffer[q];
                            }
                            m++;
                            q++;
                        }
                    }
                    if (slashcount == 2)
                    {
                        int m = 0;
                        while (buffer[q] != ':')
                        {
                            q++;
                        }
                        q++;
                        while (buffer[q] != '\n')
                        {
                            if(buffer[q]!=','){
                            toGroup[m] = buffer[q];
                            }
                            m++;
                            q++;
                        }
                    }
                    if (slashcount == 3)
                    {
                        int m = 0;
                        while (buffer[q] != ':')
                        {
                            q++;
                        }
                        q++;
                        strcat(messagetoGroup, fromwho);
                        strcat(messagetoGroup, ":");
                        while (buffer[q] != '\n')
                        {
                            temproryMessage[m] = buffer[q];
                            m++;
                            q++;
                        }
                        strcat(messagetoGroup,temproryMessage);
                    }
                    if (buffer[q] == '\n')
                    {
                        slashcount++;
                    }
                    q++;
                }
                strcat(messagetoGroup,"\n");
                         
                for (int i = 0; i < groupIndex; i++)
                {

                    if (strcmp(sock->groupID, groups[i]->groupName) == 0)
                    {
                        for (int m = 0; m < groups[i]->groupmembersIndex; m++)
                        {
                            if (groups[i]->groupMembetsSockets[m] != sock->sockfd)
                            {
                                write(groups[i]->groupMembetsSockets[m], messagetoGroup, strlen(messagetoGroup));
                            }
                        }
                    }
                }
                bzero(messagetoGroup, sizeof(messagetoGroup));
                bzero(buffer, sizeof(buffer));
                bzero(temproryMessage, sizeof(temproryMessage));
                sem_post(&comLock);
            }
            else if (strncmp(buffer, "-gcreate", 8) == 0)
            {
                char tempGroupName[50];
                int i = 0;
                while (buffer[i] != NULL && buffer[i] != '+')
                {
                    i++;
                }
                int k = 0;
                for (k = 0; k < strlen(buffer); k++)
                {
                    if (buffer[k] == '\n')
                    {
                        buffer[k] = '\0';
                        break;
                    }
                }
                k = 0;
                while (k < 50 && i < strlen(buffer))
                {
                    tempGroupName[k] = buffer[++i];
                    k++;
                }

                for (int i = 0; i < clientIndex; i++)
                {
                    if (clients[i]->sockfd == sock->sockfd)
                    {
                        strcat(clients[i]->groupID, tempGroupName);

                        break;
                    }
                }

                group *grp = (group *)malloc(sizeof(group));
                strcat(grp->groupName, tempGroupName);
                grp->groupmembersIndex = 0;
                grp->groupMembetsSockets[grp->groupmembersIndex] = sock->sockfd;
                grp->groupmembersIndex = grp->groupmembersIndex + 1;
                groups[groupIndex] = (group *)grp;
                groupIndex++;
                char *retMessage[50];
                strcat(retMessage, " Group ");
                strcat(retMessage,tempGroupName);
                strcat(retMessage, " created\n");

                write(sock->sockfd, retMessage, strlen(retMessage));
                bzero(retMessage, sizeof(retMessage));
            }
            else if (strncmp(buffer, "-join", 5) == 0)
            {

                char tempGroupName[50];
                int i = 5;
                while (buffer[i] != NULL && buffer[i] != '/')
                {
                    i++;
                }
                int k = 0;
                for (k = 0; k < strlen(buffer); k++)
                {
                    if (buffer[k] == '\n')
                    {
                        buffer[k] = '\0';
                        break;
                    }
                }
                k = 0;
                while (k < 50 && i < strlen(buffer))
                {
                    tempGroupName[k] = buffer[++i];
                    k++;
                }
                int check = 0;
                for (i = 0; i < groupIndex; i++)
                {
                    if (strcmp(groups[i]->groupName, tempGroupName) == 0)
                    {
                        for (k = 0; k < clientIndex; k++)
                        {
                            if (clients[k]->sockfd == sock->sockfd)
                            {
                                groups[i]->groupMembetsSockets[groups[i]->groupmembersIndex] = sock->sockfd;
                                groups[i]->groupmembersIndex = groups[i]->groupmembersIndex + 1;
                                break;
                            }
                        }
                        check = 1;
                        break;
                    }
                }
                if (check)
                {
                    for (i = 0; i < clientIndex; i++)
                    {
                        if (clients[i]->sockfd == sock->sockfd)
                        {
                            strcat(clients[i]->groupID, tempGroupName);
                            char *retMessage[50];
                            strcat(retMessage, " You got into group ");
                            strcat(retMessage, tempGroupName);
                            strcat(retMessage, "\n");
                            write(sock->sockfd, retMessage, strlen(retMessage));

                            bzero(retMessage, sizeof(retMessage));
                            break;
                        }
                    }
                }
                else
                {
                    char *retMessage[50];
                    strcat(retMessage, tempGroupName);
                    strcat(retMessage, " Error\n");

                    write(sock->sockfd, retMessage, strlen(retMessage));
                    bzero(retMessage, sizeof(retMessage));
                }
            }
            bzero(buffer, sizeof(buffer));
        }

        else
        {
            printf("Client has left the group.\n");
            break;
        }
    }

    printf("Left\n");
}
