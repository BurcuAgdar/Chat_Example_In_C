#include <stdio.h>
#include <string.h> 
#include <sys/socket.h>
#include <arpa/inet.h> 
#define LocalHost "127.0.0.1"
#define Port 3205
#include <pthread.h>
#include <semaphore.h>

int socket_desc = 0;
int count=0;
char ArrayName[50];
char ArrayPhone[50];
char groupName[50];
char *jsonStart = "{\nfrom : ";
char *j1 = "to:";
char *j2 = "message:";
char *jsonFinish = "\n}\n";
int dng = 1;
void *ToMsg();
void *ReceiveMsg();
int *Finish();


int main(int argc, char *argv[])
{

    struct sockaddr_in FirstServ;
    socket_desc = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_desc == -1)
    {
        puts("Could not create socket");
        return 1;
    }
    FirstServ.sin_addr.s_addr = inet_addr(LocalHost);
    FirstServ.sin_family = AF_INET;
    FirstServ.sin_port = htons(Port);

    if (connect(socket_desc, (struct sockaddr *)&FirstServ, sizeof(FirstServ)) < 0)
    {
        puts("Connection error");
        return 1;
    }
    puts("Welcome *_* ");
    puts("Successfully connected to the server");

    pthread_t toMessage;
    pthread_t receiveMessage;

    pthread_create(&toMessage, NULL, (void *)ToMsg, NULL);
    pthread_create(&receiveMessage, NULL, (void *)ReceiveMsg, NULL);
    int f=Finish();
    
    close(socket_desc);

    return 0;
}

void *ToMsg()
{
    char buffer[1024];
    while (dng)
        if(count==0){
            printf("Phone : ");
            fgets(ArrayPhone, 50, stdin);
            int deleteNextLine = 0;
            while(ArrayPhone[deleteNextLine] != '\n'){
            deleteNextLine++;
            }
            ArrayPhone[deleteNextLine] = '\0';
            send(socket_desc, ArrayPhone, strlen(ArrayPhone), 0);
            bzero(ArrayPhone, sizeof(ArrayPhone));

            printf("Name : ");
            fgets(ArrayName, 50, stdin);
            deleteNextLine = 0;
            while(ArrayName[deleteNextLine] != '\n'){
            deleteNextLine++;
            }
            ArrayName[deleteNextLine] = '\0';
            count=1;
            printf("To find out your phone number -> -whoami \n");
            printf("To create a group -> -creat phonenumber+groupname \n");
            printf("To enter the current groups -> -join phonenumer/gropname \n");
            printf("To send the message -> -send yourmessage \n");
            printf("To leave the group -> -exit groupname \n");
            printf("To leave the program -> -exit \n");
            
        }

        else{
            fgets(buffer, 512, stdin);
        if (strncmp(buffer, "-send", 5) == 0)
        {  char sendMessage[512];


         
         bzero(sendMessage, sizeof(sendMessage));
            int i = 5;
            while (buffer[i] == ' ')
            {
                i++;
            }
            int index = 0;
          
            strcat(sendMessage,jsonStart);
            strcat(sendMessage,ArrayName);
            strcat(sendMessage,",\n");
            strcat(sendMessage,j1);
            strcat(sendMessage,groupName);
             strcat(sendMessage,",\n");
             strcat(sendMessage,j2);
             char temprory[256];
            while (buffer[i] != '\n' )
            {
                temprory[index] = buffer[i];
                i++;
                index ++;
            }
            strcat(sendMessage,temprory);
            strcat(sendMessage,jsonFinish);
              send(socket_desc, sendMessage, strlen(sendMessage), 0);
               bzero(temprory, sizeof(temprory));
        bzero(buffer, sizeof(buffer));
        bzero(sendMessage, sizeof(sendMessage));
        }
    else{
        if (strncmp(buffer, "-exit", 5) == 0 && strlen(buffer) == 6)
        {
            dng = 0;
        }
        if (strncmp(buffer, "-join", 5) == 0 )
        {

            bzero(groupName, sizeof(groupName));
            int i = 5;
            while (buffer[i] != '/')
            {
                i++;
            }
            i++;
            int index = 0;
            while (buffer[i] != '\n')
            {
                groupName[index] = buffer[i];
                i++;
                index++;
            }
        }

        if (strncmp(buffer, "-gcreate", 8) == 0 )
        {

            bzero(groupName, sizeof(groupName));
            int i = 8;
            while (buffer[i] != '+')
            {
                i++;
            }
            i++;
            int index = 0;
            while (buffer[i] != '\n')
            {
                groupName[index] = buffer[i];
                i++;
                index++;
            }
        }
            
        send(socket_desc, buffer, strlen(buffer), 0);
        bzero(buffer, sizeof(buffer));
       
        }       
    }

    
        
}
int *Finish(){
    
    while (1)
    {
        if (!dng)
        {
            printf("Program finished\n");
            return 0;
        }
    }
    
}

void *ReceiveMsg()
{
    char rply[3000];

    while (dng)
    {
        recv(socket_desc, rply, 2000, 0);
        printf("%s", &rply);
        bzero(rply, sizeof(rply));
    }
}
