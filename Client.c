#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>
#include <signal.h>
#include <unistd.h>

#include "communication.h"

static const char* streets[] = {"Alexadru cel bun","Copou","Gara"};

int sd;
int sdBroadcast;
struct sockaddr_in serverAddress;
int running = 1;

struct clientCredentials clt;

void generateCredentials(struct clientCredentials* clt);

void performSendId(int sd){
  mySendID(sd,clt);

}

void performSendSubscriptions(int sd){

  mySendSubscriptions(sd,clt);

}

void performSendMessageToBroadcats(int sd,char* message){

  mySendMessageToBroadcast(sd,message,clt);

}

void performSendMessage(){
  char*message;
  message = malloc(103);
  char* convertedSpeed;
  convertedSpeed = (char*)malloc(3);
  char* street;
  street = (char*)malloc(100);
  int speed;
  srandom(time(0));
  speed = (random() % 100);
  if(speed/10==0)
    sprintf(convertedSpeed,"00%d",speed);
  if(speed/100==0)
    sprintf(convertedSpeed,"0%d",speed);
  else sprintf(convertedSpeed,"%d",speed);
  int index;
  index = (random() % 3);
  strcpy(street,streets[index]);
  sprintf(message,"%s%s",convertedSpeed,street);


  mySendMessage(sd,message,clt);

}

void performSendDeauth(){

  mySendDeauth(sd);

}

void* broadcastListen(void* param);
void* userInput(void* param);

int main(int argc, char *argv[]){

    sd = socket(AF_INET,SOCK_STREAM,0);
    if(sd < 0){

        perror("Something went wrong at socket creation");
        // return errno;

        }

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress.sin_port = htons(8888);

    if(connect(sd,(struct sockaddr *)&serverAddress, sizeof(struct sockaddr)) < 0){
        printf("Something went wrong at connect");
        // return errno;
      }

    printf("Connection live!\n");


    generateCredentials(&clt);

    performSendId(sd);
    performSendSubscriptions(sd);

    pthread_t threadId;
    pthread_create(&threadId,NULL,broadcastListen,(void*)&running);
    sleep(2);
    // performSendMessageToBroadcats(sdBroadcast,"HELLO!");
    pthread_create(&threadId,NULL,userInput,(void*)&running);

    while(running){
      sleep(2);
      performSendMessage();

    }

    close(sd);

    printf("Connection done!\n");

}

void generateCredentials(struct clientCredentials* clt){

  //se genereaza random credentialele la conect pentru a simula o masina unica per client

  char clientId[11];
  int sd;
  int subscriptions[3];

  // struct clientCredentials clt;

  srandom(time(0));
  for(int i = 0; i<10; i++)
  {
    clt->clientId[i] = (random() % 10);
    printf("%d ",clt->clientId[i]);
  }

  printf("\n");

  clt->sd = -1;

  clt->subscriptions[0] = (random()%2);
  clt->subscriptions[1] = (random()%2);
  clt->subscriptions[2] = (random()%2);
  printf("SUBSCRIPTIONS Vreme: %d preturi: %d sport: %d\n",clt->subscriptions[0],clt->subscriptions[1],clt->subscriptions[2]);


}

void* broadcastListen(void* param){
  sdBroadcast = socket(AF_INET,SOCK_STREAM,0);
  if(sd < 0){

      perror("[boradacastListen]Something went wrong at socket creation");
      // return errno;

      }

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
  serverAddress.sin_port = htons(9999);

  if(connect(sdBroadcast,(struct sockaddr *)&serverAddress, sizeof(struct sockaddr)) < 0){
      printf("[boradacastListen]Something went wrong at connect");
      // return errno;
    }

  printf("Broadcast live!\n");
  int dataVolume;
  // int* running = *(int**)param;
  char* message;
  message = malloc(1024);

  performSendId(sdBroadcast);
  performSendSubscriptions(sdBroadcast);

  while(running){
    bzero(message,1024);
    sleep(2);

    dataVolume = recv(sdBroadcast,message,1024,0);
    if(dataVolume <= 0){
     perror("Something went wrong at write in [receiveMessage(%d)]\n");
    }

    printf("Broadcast message received:\n%s\n",message);
    dataVolume = 0;

  }

  close(sdBroadcast);

  printf("Broadcast done!\n");

  pthread_exit(0);
}

void* userInput(void* param){

  // int* running = *(int**)param;
  size_t len = 0;
  ssize_t read;
  char* line;
  char* message;
  message = malloc(1024);
  int size;
  while(running){
    bzero(message,1024);
    read = getline(&line,&len,stdin);
    strncpy(message,line,strlen(line)-1);
    if(strcmp(message,"exit")==0){ //daca input-ul este exit atunci clientul trimite un cod de deautentificare
      printf("Closing..\n");
      performSendDeauth(sd);
      performSendDeauth(sdBroadcast);
      running = 0;
      pthread_exit(0);
    }
    performSendMessageToBroadcats(sdBroadcast,message);
  }

  pthread_exit(0);
}
