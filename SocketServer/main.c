#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>
#include<unistd.h>
#include <pthread.h>

#define PORT 3500
#define MAX_CONNECTIONS 10

struct AcceptedSocket
{
    int acceptedSocketFD;
    struct sockaddr_in address;
    int acceptedSuccessfully;
};

int createTCPIPv4Socket();
struct sockaddr_in* createIPv4Address(char*ip, int port);
void startAcceptingConnections(int serverSocketFD);
struct AcceptedSocket* acceptIncomingConnection(int serverSocketFD);
void makeThread(struct AcceptedSocket *clientSocket);
void *receiveIncomingData(void* arg);
void sendMessage(char *buffer,int socketFD);
//void lastMessageBeforeExit();

struct AcceptedSocket connections[MAX_CONNECTIONS] ;
int connectionsCount = 0;

int main(){
    int serverSocketFD= createTCPIPv4Socket();
    struct sockaddr_in *serverAddress= createIPv4Address("",PORT);

    int result= bind(serverSocketFD, (struct sockaddr*) serverAddress, sizeof(*serverAddress));
    if(result==0){
        printf("Server socket is bound succesfully\n");
    }

    int listenResult= listen(serverSocketFD, 10);
    startAcceptingConnections(serverSocketFD);
    //lastMessageBeforeExit();
    shutdown(serverSocketFD,SHUT_RDWR);
    close(serverSocketFD);
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

void startAcceptingConnections(int serverSocketFD){
    //while(connectionsCount <= MAX_CONNECTIONS)
    while(1)
    {
        struct AcceptedSocket* clientSocket  = acceptIncomingConnection(serverSocketFD);
        connections[connectionsCount++] = *clientSocket;
        makeThread(clientSocket);
    }
}

struct AcceptedSocket * acceptIncomingConnection(int serverSocketFD) {
    struct sockaddr_in  clientAddress ;
    int clientAddressSize = sizeof (struct sockaddr_in);
    int clientSocketFD = accept(serverSocketFD,(struct sockaddr*) &clientAddress,&clientAddressSize);

    struct AcceptedSocket* acceptedSocket = (struct AcceptedSocket*) malloc(sizeof (struct AcceptedSocket));
    acceptedSocket->address = clientAddress;
    acceptedSocket->acceptedSocketFD = clientSocketFD;
    acceptedSocket->acceptedSuccessfully = (clientSocketFD>0)? 0: -1;

    return acceptedSocket;
}

void makeThread(struct AcceptedSocket *clientSocket){
    pthread_t id;
    pthread_create(&id,NULL,receiveIncomingData,(void *) &clientSocket->acceptedSocketFD);
}

void *receiveIncomingData(void* arg){
    int socketFD = *(int *)arg;
    char buffer[1024];
    while(1){
        ssize_t amountReceived= recv(socketFD, buffer, 1024, 0);
        if(amountReceived>0){
            buffer[amountReceived]=0;
            printf("%s\n", buffer);
            sendMessage(buffer,socketFD);
        }

        if(amountReceived<=0){
            break;
        }
    }
    close(socketFD);
}

void sendMessage(char *buffer,int socketFD){
    for(int i = 0 ; i<connectionsCount ; i++){
        if(connections[i].acceptedSocketFD !=socketFD)
        {
            send(connections[i].acceptedSocketFD,buffer, strlen(buffer),0);
        }
    }
}

/*
void lastMessageBeforeExit(){
    char* buffer= "Server closed. Kindly type 'exit' and press enter to quit.";
    for(int i = 0 ; i<connectionsCount ; i++){
        {
            send(connections[i].acceptedSocketFD,buffer, strlen(buffer),0);
            shutdown(connections[i].acceptedSocketFD, SHUT_RDWR);
            close(connections[i].acceptedSocketFD);
            
        }
    }
}
*/

//Exploring ways to terminate server socket
//One option - Assign max connections, above which server socket terminates.
