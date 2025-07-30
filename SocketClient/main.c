#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include <pthread.h>

#define PORT 3500

int createTCPIPv4Socket();
struct sockaddr_in* createIPv4Address(char*ip, int port);
void makeThread(int socketFD);
void sendMessages(int socketFD);
void* receiveMessages(void* arg);

int main(){
    int socketFD = createTCPIPv4Socket();

    char* ip= "127.0.0.1";
    struct sockaddr_in* address= createIPv4Address(ip, PORT);
    int result= connect(socketFD, (struct sockaddr*) address, sizeof(*address));
    if(result==0)
        printf("Connection was successful\n");

    makeThread(socketFD);
    sendMessages(socketFD);
    
    close(socketFD);
    return 0;
}

int createTCPIPv4Socket(){
    return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in* createIPv4Address(char* ip, int port){
    struct sockaddr_in* address= (struct sockaddr_in*) malloc(sizeof(struct sockaddr_in));
    address->sin_family= AF_INET;
    address->sin_port= htons(port);

    if(strlen(ip) == 0)
        address->sin_addr.s_addr= INADDR_ANY;
    else
        inet_pton(AF_INET, ip, &address->sin_addr.s_addr);
    return address;
}

void makeThread(int socketFD) {

    pthread_t id ;
    pthread_create(&id,NULL,receiveMessages,(void*) &socketFD);
}

void sendMessages(int socketFD){
    char *name = NULL;
    size_t nameSize= 0;
    printf("Enter your name : ");
    ssize_t  nameCount = getline(&name,&nameSize,stdin);
    name[nameCount-1]=0;

    printf("Hi %s, send and receive messages.\n", name);
    printf("To exit, type 'exit'\n");

    char* line= NULL;
    size_t lineSize =0;
    char buffer[1024];

    while(1){
        ssize_t  charCount = getline(&line,&lineSize,stdin);
        line[charCount-1]=0;
        sprintf(buffer,"%s : %s",name,line);
        
        if(charCount>0){
            if(strcmp(line,"exit")==0) break;
            send(socketFD, buffer, strlen(buffer), 0);
        }
    }
}

void *receiveMessages(void* arg){
    int socketFD= *(int*) arg;
    char buffer[1024];
    while (1)
    {
        ssize_t  amountReceived = recv(socketFD,buffer,1024,0);
        if(amountReceived>0)
        {
            buffer[amountReceived] = 0;
            printf("%s\n",buffer);
        }
        if(amountReceived==0)
            break;
    }
    close(socketFD);
}
