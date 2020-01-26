#include <stdio.h>
#include <string.h>

#define WEATHER 0
#define FUEL 1
#define SPORT 2

 struct clientCredentials{
  char clientId[11];
  int sd;
  int subscriptions[3];
};

// void convertDataFromBytes(char* data){
//
//   struct clientCredentials client;
//   FILE *file;
//   file=fopen("clientCredRecv.data","wb");
//   if(file==NULL)
//     perror("Could  not open file\n");
//
//   fwrite(data, strlen(data),1,file);
//
//   fclose(file);
//
//   FILE * infile;
//
//   infile = fopen("clientCred.data","r");
//   while(fread(&client, sizeof(struct clientCredentials),1,infile))
//
//   printf("Client ID: ");
//   for(int i = 0; i<10; i++)
//   {
//     printf("%d ",client.clientId[i]);
//   }
//   printf("\n");
//   printf("Client sd: %d\n",client.sd);
//   printf("Client subscriptions: Wheather[%d] Sport[%d] Gas Price[%d]\n",client.subscriptions[0],client.subscriptions[1],client.subscriptions[2]);
//   fclose(infile);
//
// }

// char* convertDataToBytes(struct clientCredentials clt){
//
//   FILE *file;
//   file=fopen("clientCred.data","wb");
//   if(file==NULL)
//     perror("Could  not open file\n");
//
//   fwrite(&clt, sizeof(struct clientCredentials),1,file);
//
//   fclose(file);
//
//   FILE * infile;
//   char* buff;
//   long filelen;
//
//
//
//   infile = fopen("clientCred.data","rb");
//   fseek(infile,0,SEEK_END);
//   filelen = ftell(infile);
//   rewind(infile);
//
//   buff=(char*)malloc((filelen+1)*sizeof(char));
//   fread(buff,filelen,1,infile);
//
//   printf("Content of file: '%s'\n", buff);
//   fclose(infile);
//
//   return buff;
// }

void updateCredentials(struct clientCredentials* clt); //update al variabilei clt din client.

// struct clientCredentials readDataFromBytes(char* data);

void saveUser(char* id, int sd){

  char* filename;
  // sprintf(filename,"%s.data",id);
  //
  //
  // FILE* file;
  // file = fopen(filename,"w");
  // fprintf(file,"%s\n%d",id,sd);
  // fclose(file);
  printf("[updateClientId]Saved user %s in file %s.data\n",id,filename);

}

void updateSd(int sd);

void updateSubscriptions(int subscriptions[3]){

  printf("[updateSubscriptions]Updated subscriptions\n");

}

void saveCredentials(struct clientCredentials clt);

void readCredentials();

void updateClientId(char* id){




}
