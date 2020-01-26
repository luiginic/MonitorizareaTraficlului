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

struct client{
  char* id;
  int sd;
  int subs[3];
  int index;
};

struct data{
  int speed;
  char* location;
};

#define MAXCLIENTS 100

struct client clients[100];

int noOfClients = 0;

#define PORT 8888  /*portul de conectare*/
#define BORADCAST_PORT 9999 /*portul de boradcast*/

pthread_mutex_t mut;

void * clientDedicatedThread(void * param);/*threadul dedicat clientului*/
void * broadcastThread(void* param);
void * clientBroadcastThread(void * param);
void* subscriptionThread(void* param);

int main() {

    struct sockaddr_in thisAddress;
    struct sockaddr_in clientAddress;
    int sd, sd2;
    int port;
    int addrLength;
    pthread_t threadId;

    pthread_mutex_init(&mut,NULL);
    // initializeMutex();

    memset((char *)&thisAddress,0, sizeof(thisAddress)); //eliberarea memoriei in structura de date care retine adresa sererului
    thisAddress.sin_family = AF_INET;
    thisAddress.sin_addr.s_addr = htonl(INADDR_ANY); //adresa de IP a serverului este adresa locala

    port = PORT;
    thisAddress.sin_port = htons((u_short)port);


    sd = socket(PF_INET,SOCK_STREAM,0);
    if(sd<0){
        perror("Something went wrong at socket creation\n");
        exit(1);
    }

    if(bind(sd,(struct sockaddr *)&thisAddress, sizeof(thisAddress)) < 0){
        perror("Something went wrong at bind\n");
        exit(1);
    }

    if(listen(sd, MAXCLIENTS) < 0) {
        perror("Something went wrong at listen\n");
    }
     addrLength = sizeof(clientAddress);

     pthread_create(&threadId,NULL,broadcastThread,NULL);
     sleep(2);
     pthread_create(&threadId,NULL,subscriptionThread,NULL);

    /*de aici incepe serverul propriu-zis*/
    while(1) {

        printf("[Initial thread]: Waiting for connection\n");

        if( (sd2 = accept(sd, (struct sockaddr *)&clientAddress, &addrLength)) < 0 ) {
            perror("Something went wrong at accept\n");
            exit(1);
        } else printf("[Initial thread]: Request accepted with SOCKET DESCRIPTOR %d\n",sd2);

        pthread_create(&threadId, NULL, clientDedicatedThread, (void *) &sd2);

    }
    close(sd);
}

void receiveId(int clientSd,struct client* info){

  char* clientId;
  clientId =(char*) malloc(10);

  char* data;
  data =(char*) malloc(1024);

  char* response;
  response =(char*) malloc(100);
  int dataVolume;

  bzero(data,1024);

  dataVolume = recv(clientSd,data,1024,0);
  if(dataVolume < 0){
    perror("Something went wrong at receiv in [receiveId]!\n");
  }

  printf("[receiveId(%d)]The message '%s' has been recieved with a total of %dbytes.\n",clientSd, data,dataVolume);


  char* code;
  code = (char*)malloc(3);
  strncpy(code,data,3);
  int numericCode = atoi(code);
  switch(numericCode){
    case 101:
      strncat(clientId,data+3,10);
      info->id = (char*)malloc(strlen(clientId));
      strcpy(info->id,clientId);
      sprintf(response,"Welcome %s!",info->id);
      break;
    default:
      perror("[receiveId]: Something went wrong");
      sprintf(response,"Woops!");

  }

  dataVolume = send(clientSd,response,strlen(response),0);
  if(dataVolume <= 0){
   perror("Something went wrong at write in [receiveId]\n");
  }

}
void receiveSubs(int clientSd,struct client* info){

  int subs[3];

  char* clientId;
  clientId =(char*) malloc(10);

  char* data;
  data =(char*) malloc(1024);

  char* response;
  response =(char*) malloc(100);
  int dataVolume;

  bzero(data,1024);

  dataVolume = recv(clientSd,data,1024,0);
  if(dataVolume < 0){
    perror("Something went wrong at receiv in [receiveSubs(%d)]!\n");
  }

  printf("[receiveSubs(%d)]The message '%s' has been recieved with a total of %dbytes.\n",clientSd, data,dataVolume);


  char* code;
  code = (char*)malloc(3);
  strncpy(code,data,3);
  int numericCode = atoi(code);

  strncat(clientId,data+3,10);

  char* subscriptions;
  subscriptions = malloc(3);
  char* c;
  c = malloc(1);

  switch(numericCode){
    case 102:
      strncat(subscriptions,data+13,3);
      printf("Subscriptions received: %s\n",subscriptions);
      for(int i = 0;i<3;i++){
        bzero(c,1);
        strncpy(c,subscriptions+i,1);
        subs[i] = atoi(c);
        info->subs[i] = subs[i];
      }
      sprintf(response,"Well done %s! Subscriptions received!",info->id);
      break;
    default:
      perror("[receiveSubs(%d)]: Something went wrong");
      sprintf(response,"Woops! Subscriptions not received!");
  }

  dataVolume = send(clientSd,response,strlen(response),0);
  if(dataVolume <= 0){
   perror("Something went wrong at write in [receiveSubs(%d)]\n");
  }
}
int receiveMessage(int clientSd,struct client* info){

  int connected = 1;

  char* clientId;
  clientId =(char*) malloc(10);

  char* data;
  data =(char*) malloc(1024);

  char* response;
  response =(char*) malloc(100);
  int dataVolume;

  bzero(data,1024);

  dataVolume = recv(clientSd,data,1024,0);
  if(dataVolume < 0){
    perror("Something went wrong at receive in [receiveMessage(%d)]!\n");
  }

  printf("[receiveMessage(%d)]The message '%s' has been recieved with a total of %dbytes.\n",clientSd, data,dataVolume);

  struct data message;
  memset(&message,0,sizeof(struct data));

  char* code;
  code = (char*)malloc(3);
  strncpy(code,data,3);
  int numericCode = atoi(code);

  strncat(clientId,data+3,10);
  char* rawData;
  rawData=(char*)malloc(260);
  memset(rawData,'\0',260);
  switch(numericCode){
    case 201:
      strncpy(rawData,data+13,strlen(data)-13);
      printf("[receiveMessage]This is the rawaData %s \n",rawData);
      char* speed;
      speed = malloc(3);
      strncpy(speed,rawData,3);
      message.speed = atoi(speed);
      char* street;
      street=malloc(244);
      strncpy(street,rawData+3,dataVolume-16);
      printf("[receiveMessage]Separated---> Speed: %s  Street: '%s'\n",speed,street);

      message.location = (char*)malloc(strlen(street));
      strncpy(message.location,street,strlen(street));
      printf("[receiveMessage]Speed: %d  Street: '%s'\n",message.speed,message.location);

      if(message.speed>60)
        sprintf(response,"!!%dKm/h!!\nSpeed limit: 60Km/h around %s\n",message.speed,message.location);
      else sprintf(response,"%dKm/h\n",message.speed);

      break;
      case 000:
        pthread_mutex_lock(&mut);
        connected = 0;
        // for(int i = 0;i<noOfClients;i++){
        //   if(strcmp(clients[i].id,info->id)==0){
        //     for(int j = i;j<noOfClients-1;j++){
        //       clients[j]=clients[j+1];
        //     }
        //   }
        // }
        // noOfClients--;
        pthread_mutex_unlock(&mut);
        sprintf(response,"Bye %s!",info->id);
        break;
    default:
      perror("[receiveMessage(%d)]: Something went wrong\n");
      sprintf(response,"Woops! Something went wrong!");
  }

  dataVolume = send(clientSd,response,strlen(response),0);
  if(dataVolume <= 0){
   perror("Something went wrong at write in [receiveMessage(%d)]\n");
  }
  return connected;
}
int receiveMessageToBroadcast(int clientSd,struct client* info){
  int connected;

  char* clientId;
  clientId =(char*) malloc(10);

  char* data;
  data =(char*) malloc(1024);

  char* response;
  response =(char*) malloc(100);
  int dataVolume;

  bzero(data,1024);

  dataVolume = recv(clientSd,data,1024,0);
  if(dataVolume < 0){
    perror("Something went wrong at receive in [receiveMessageToBroadcast(%d)]!\n");
  }

  printf("[receiveMessageToBroadcast(%d)]The message '%s' has been recieved with a total of %dbytes.\n",clientSd, data,dataVolume);

  char* code;
  code = (char*)malloc(3);
  strncpy(code,data,3);
  int numericCode = atoi(code);

  strncat(clientId,data+3,10);
  char* rawData;
  rawData=(char*)malloc(260);
  memset(rawData,'\0',260);
  switch(numericCode){
    case 211:
      strncpy(rawData,data+13,strlen(data)-13);
      printf("[receiveMessagetoBroadcast]This is the rawaData %s \n",rawData);
      sprintf(response,"User%s: '%s'",info->id,rawData);
      break;
    case 0:
      connected = 0;
      sprintf(response,"Bye %s!",info->id);

      dataVolume = send(clientSd,response,strlen(response),0);
      if(dataVolume <= 0){
       perror("Something went wrong at write in [receiveMessage(%d)]\n");
      }
      return connected;
      break;
    default:
      perror("[receiveMessagetoBroadcast(%d)]: Something went wrong");
      sprintf(response,"Woops! Something went wrong!");
  }


  for(int i = 0;i<noOfClients;i++){
    int sd;
    if(strcmp(clients[i].id,info->id)!=0){
      sd = clients[i].sd;
      printf("Sending message %s to client %d\n",response,clients[i].sd);
      dataVolume = send(clients[i].sd,response,strlen(response),0);
      if(dataVolume <= 0){
       perror("Something went wrong at write in [receiveMessagetoBroadcast(%d)]\n");
      }
    }
  }
  return connected;
}

void * clientDedicatedThread(void * param){

  int connected = 1;

  int clientSd;
  clientSd = *(int*)param;
  int index;
  struct client info; //structura pentru client
  memset(&info,0,sizeof(struct client)); //initializarea structurii
  info.sd = clientSd;

  pthread_mutex_lock(&mut);
  //// index = noOfClients;
  //// info.index = index;
  ////noOfClients++;
  pthread_mutex_unlock(&mut);

  receiveId(clientSd,&info);
  receiveSubs(clientSd,&info);
  pthread_mutex_lock(&mut);
  // clients[index]=info;
  pthread_mutex_unlock(&mut);


  while(connected){
    connected = receiveMessage(clientSd,&info);

  }


  pthread_mutex_lock(&mut);
  // noOfClients--;
  // sdArray[index] = -1;
  pthread_mutex_unlock(&mut);

  pthread_exit(0);
}

void* broadcastThread(void * param){

  struct sockaddr_in thisAddress;
  struct sockaddr_in clientAddress;
  int sd, sd2;
  int port;
  int addrLength;
  pthread_t threadId;

  memset((char *)&thisAddress,0, sizeof(thisAddress)); /*eliberarea memoriei in structura de date care retine adresa sererului*/
  thisAddress.sin_family = AF_INET;
  thisAddress.sin_addr.s_addr = htonl(INADDR_ANY); /*adresa de IP a serverului este adresa locala*/

  port = BORADCAST_PORT;
  thisAddress.sin_port = htons((u_short)port);


  sd = socket(PF_INET,SOCK_STREAM,0);
  if(sd<0){
      perror("[broadacst]Something went wrong at socket creation\n");
      exit(1);
  }

  if(bind(sd,(struct sockaddr *)&thisAddress, sizeof(thisAddress)) < 0){
      perror("[broadacst]Something went wrong at bind\n");
      exit(1);
  }

  if(listen(sd, MAXCLIENTS) < 0) {
      perror("[broadacst]Something went wrong at listen\n");
  }
   addrLength = sizeof(clientAddress);

   while(1) {

       printf("[broadacst thread]: Waiting for connection\n");

       if( (sd2 = accept(sd, (struct sockaddr *)&clientAddress, &addrLength)) < 0 ) {
           perror("[broadacst]Something went wrong at accept\n");
           exit(1);
       } else printf("[broadacst thread]: Request accepted with SOCKET DESCRIPTOR %d\n",sd2);

       pthread_create(&threadId, NULL, clientBroadcastThread, (void *) &sd2);

   }
   close(sd);

  pthread_exit(0);
}

void* clientBroadcastThread(void* param){
  int connected = 1;

  int clientSd;
  clientSd = *(int*)param;
  int index;
  struct client info; //structura pentru client
  memset(&info,0,sizeof(struct client)); //initializarea structurii
  info.sd = clientSd;

  pthread_mutex_lock(&mut);
  index = noOfClients;
  info.index = index;
  noOfClients++;
  pthread_mutex_unlock(&mut);

  receiveId(clientSd,&info);
  receiveSubs(clientSd,&info);
  pthread_mutex_lock(&mut);
  clients[index]=info;
  printf("[clientBroadcastThread]Saved client %s with socket %d in array at index %d\n",clients[index].id,clients[index].sd,clients[index].index);
  pthread_mutex_unlock(&mut);


  printf("User:%s---> Vreme:%d Preturi:%d Sport:%d\n",clients[index].id,clients[index].subs[0],clients[index].subs[1],clients[index].subs[2]);
  while(connected){
      connected = receiveMessageToBroadcast(clientSd,&info);

  }


  pthread_mutex_lock(&mut);
  for(int i = info.index;i<noOfClients;i++){
        printf("client %s is copied in place of client %s",clients[i+1].id,clients[i].id);
        memset(&clients[i],0,sizeof(struct client));
        clients[i]=clients[i+1];
        clients[i].index = i;
      }
  memset(&clients[noOfClients],0,sizeof(struct client));
  printf("!!!!Client removed%s!\n",info.id);
  noOfClients--;
  // sdArray[index] = -1;
  pthread_mutex_unlock(&mut);

  pthread_exit(0);
}

//threadul care trimite informatiile la care s-au inscris userii
void* subscriptionThread(void* param){

  static const char* teams[] = {"Barcelona","Real Madrid","Arsenal","Lazio Roma"};
  static const char* wheatherCond[] = {"mostly sunny","chances of precipitations","high winds","cloudy","misty"};
  int dataVolume;
  char* message;
  message = malloc(1024);
  srand(time(0));

  //simulez o structura de tip infoWheather
  int temperature = 10; //intre -20 si 40
  char* conditions;
  conditions = malloc(30); //o serie de conditii cum ar fi
  // "cu soare predominant","cu averse de ploaie","cu sanse de ninsoare",
  // "cu intensitatea vantului crescuta"... etc

  //simulez o structura de tip infoPrices la valorile actuale se adauga intre 0.5 si 1.5
  float gasPrice = 5.281;
  float dieselPrice = 5.463;
  float gplPrice = 2.718;

  //simulez o structura de tip infoSports
  char* team1;
  team1 = malloc(20);
  char* team2;
  team2 = malloc(20);
  int scoreTeam1 = 3;
  int scoreTeam2 = 1;
  char* report; //compun un raport cu mai multe meciuri
                // generate random e.g. "Barcelona vs Real Madrid : 3-1\nFlorentina vs Arsenal : 0-2 ..."

  int clientsCount;
  int sd;
  while(1){
    //aici se decide informatia pe care serverul o trimite, pentru a putea fi trimisa
    // tuturor la fel; astel se simuleaza un request catre un sistem third party pentru
    // fiecare din acestea;
    if(rand()%2)
      gasPrice = gasPrice + (rand()%2);
    else gasPrice = gasPrice - (rand()%2);
    if(rand()%2)
      dieselPrice = dieselPrice + (rand()%2);
    else dieselPrice = dieselPrice - (rand()%2);
    if(rand()%2)
      gplPrice = gplPrice + (rand()%2);
    else gplPrice = gplPrice - (rand()%2);

    temperature = rand()%40;
    bzero(conditions,30);
    strcpy(conditions,wheatherCond[(rand()%5)]);

    bzero(team1,20);
    bzero(team2, 20);
    strcpy(team1,teams[(rand()%4)]);
    strcpy(team2,teams[(rand()%4)]);
    scoreTeam1 = (rand()%6);
    scoreTeam2 = (rand()%6);

    for(int i = 0;i<noOfClients;i++){
      pthread_mutex_lock(&mut);
      clientsCount = noOfClients;
      sd = clients[i].sd;
      printf("[subsriptionThread]User:%s---> Vreme:%d Preturi:%d Sport:%d\n",clients[i].id,clients[i].subs[0],clients[i].subs[1],clients[i].subs[2]);
      pthread_mutex_unlock(&mut);

      if(clients[i].subs[0]==1){
        //trimite informatii despre vreme prin portul de broadcast
        sprintf(message,"Wheather today: %d degree Celsius and %s\n",temperature,conditions);
        printf("Sending wheather %s to client %d\n",message,clients[i].sd);
        dataVolume = send(clients[i].sd,message,strlen(message),0);
        if(dataVolume <= 0){
         perror("Something went wrong at write in [subscriptionThread(%d)]\n");
        }
      }
      if(clients[i].subs[1]==1){
        //trimite informatii despre preturi prin portul de broadcast
        sprintf(message,"Prices today:\n ->%f lei Gas\n ->%f lei Diesel\n ->%f lei GPL\n",gasPrice,dieselPrice,gplPrice);
        printf("Sending prices '%s' to client %d\n",message,clients[i].sd);
        dataVolume = send(clients[i].sd,message,strlen(message),0);
        if(dataVolume <= 0){
         perror("Something went wrong at write in [subscriptionThread(%d)]\n");
        }
      }
      if(clients[i].subs[2]==1){
        //trimite informatii despre sport prin portul de broadcast
        sprintf(message,"Sports today:\n -%s vs %s : %d-%d\n",team1,team2,scoreTeam1,scoreTeam2); //se transforma in sprintf(message,"Sports today:\n%s",report);
        printf("Sending sport '%s' to client %d\n",message,clients[i].sd);
        dataVolume = send(clients[i].sd,message,strlen(message),0);
        if(dataVolume <= 0){
         perror("Something went wrong at write in [subscriptionThread(%d)]\n");
        }
      }
    }
    sleep(10); //perioada de timp in care se trimite (din cauza timpului scurt de prezentare vom utiliza 10 secunde)
  }

  pthread_exit(0);
}
