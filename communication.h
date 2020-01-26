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

#include "clientCredentials.h"

#define CLIENT_ID 101
#define CLIENT_SUBSCRIPTION 102

#define SUBSCRIPTION_REQUEST 120

#define MESSAGE 201
#define MESSAGE_BROADCAST 211

#define EXIT 000

#define APPROVED 111
#define DECLINED 100

//Specifice clientului

void mySend(int sd,char* message){

  char* response;
  int dataVolume;

  response = malloc(100);

  dataVolume = send(sd,message,strlen(message),0);
  if(dataVolume <= 0){
    perror("[mySend]Something went wrong at write in [sendData]!\n");
  }

  dataVolume = 0;

  dataVolume = recv(sd,response,100,0);
  if(dataVolume <0){
    perror("[mySend]Something went wrong at read in [sendData]!\n");
  }

  printf("%s\n",response);

}

void mySendID(int sd,struct clientCredentials clt){

  char* message;
  char* id;

  id = malloc(10);
  message = malloc(14);

  for(int i=0;i<10;i++){
    sprintf(id,"%s%d",id,clt.clientId[i]);
  }
  sprintf(message,"%d%s",CLIENT_ID,id);
  // printf("[mySendID]Message ready: '%s'\n",message);

  mySend(sd,message);
}

void mySendSubscriptions(int sd, struct clientCredentials clt){

    char* message;
    char* subscriptions;
    char* id;

    id = malloc(10);
    subscriptions = malloc(3);
    message = malloc(17);

    for(int i=0;i<10;i++){
      sprintf(id,"%s%d",id,clt.clientId[i]);
    }

    for(int i=0;i<3;i++){
      sprintf(subscriptions,"%s%d",subscriptions,clt.subscriptions[i]);
    }
    sprintf(message,"%d%s%s",CLIENT_SUBSCRIPTION,id,subscriptions);
    // printf("[mySendSubscriptions]Message ready: '%s'\n",message);

    mySend(sd,message);
}

void mySendMessage(int sd,char* content,struct clientCredentials clt){

  char* message;
  char* id;

  id = malloc(10);

  for(int i=0;i<10;i++){
      sprintf(id,"%s%d",id,clt.clientId[i]);
    }

  message = malloc(260);

  sprintf(message,"%d%s%s",MESSAGE,id,content);
  // printf("[mySendMessage]Message ready: '%s'\n",message);
  // system("clear");
  mySend(sd,message);

}

void mySendMessageToBroadcast(int sd,char* content,struct clientCredentials clt){
  char* message;
  char* id;

  id = malloc(10);

  for(int i=0;i<10;i++){
      sprintf(id,"%s%d",id,clt.clientId[i]);
    }

  message = malloc(260);

  sprintf(message,"%d%s%s",MESSAGE_BROADCAST,id,content);
  // printf("[mySendMessageToBroadcast]Message ready: '%s'\n",message);

  int dataVolume = 0;

  dataVolume = send(sd,message,strlen(message),0);
  if(dataVolume <= 0){
    perror("[mySend]Something went wrong at write in [sendData]!\n");
  }


}

void mySendAuth(int sd,struct clientCredentials clt){

  printf("[mySendAuth]Sending authentification request..\n");
  mySendID(sd, clt);
  mySendSubscriptions(sd,clt);
  printf("[mySendAuth]Authentification done!\n");

}

void mySendDeauth(int sd){

  mySend(sd,"000");

}
///
