#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>
#include "utils.h"

pthread_mutex_t queue_lock;
pthread_cond_t empty_cond;
struct Queue* queue;


int main(int argc, char *argv[]) {

  int sockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  char buffer[1024];
  char Request_ID[70];
  char filename[2000];
  char rep[2000]; 
  int req_recieved=1;
  int req_being_served=0;
  char fname[2000];
  FILE * fd1;
  int n;

  if (argc < 3) {
    fprintf(stderr, "Improper Usage: ./server <port> <thread_pool_size>\n");
    exit(1);
  }

  /* create socket */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    printf("E31: ERROR opening socket\n");

  /* Reuse the same socket if in TIME_WAIT state */
  int enable = 1;
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
    perror("setsockopt(SO_REUSEADDR) failed");
    // Handle the error
  }

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)*/
  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    printf("E32: ERROR on binding\n");

  /* listen for incoming connection requests */
  listen(sockfd, 100);
  clilen = sizeof(cli_addr);

  /* initialize Queue */
  queue = createQueue();

  /* create thread pool */
  int thread_pool_size = atoi(argv[2]);
  pthread_t thread_ids[thread_pool_size];

  /*Initializing Mutexes and Cond Variables*/
  pthread_mutex_init(&queue_lock,NULL);
  pthread_cond_init(&empty_cond,NULL);

  for(int i=0;i<thread_pool_size;i++){
    int *td = (int *)malloc(sizeof(int));
    *td = i;
    if(pthread_create(thread_ids+i, NULL, &gradeSubmission, NULL)!=0){
      perror("E34: Failed to create Thread\n");
      exit(EXIT_FAILURE);
    }
  }

  int queue_size=0;
  char queSize[100];
  struct commands* cmds = (struct commands*) malloc(sizeof(struct commands));
  // cmds = create_commands(1);

  system("touch queueSize.txt 2>/dev/null");

  /* accept a new request, create a newsockfd */
  while (1) {
    req_recieved = 1;
    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      printf("E33: ERROR on accept\n");
    

    bzero(buffer, 1024);
    n=0;

    /* Recieve the data from client */
    n = recv(newsockfd, buffer, sizeof buffer, 0);
    if(n <= 0){
      printf("E1: Error on recieving request Id\n");
      close(newsockfd);
      continue;
    }

    /* Checking if recieved a file or Request ID */

    for(int i=0;i<n;i++){

      if(buffer[i]=='\0') break;
      if(buffer[i]=='#') req_recieved=0;

    }


    if(req_recieved){

      /*Creating a new Request Id using Timestamp*/
      if(system("date +%s#%N@ > t.txt")!=0) printf("Error generating timestamp\n");
      
      /*Storing Req Id in Request_ID buffer*/
      fd1 = fopen("t.txt","r");
      bzero(Request_ID,sizeof Request_ID);
      while(!feof(fd1)){
        if( fread(Request_ID,1,sizeof Request_ID,fd1) == -1 ) printf("Error on reading\n");
      }
      fclose(fd1);

      system("rm -r t.txt 2>/dev/null");

      /* Printing output*/
      printf("New File Recieved for Grading - Request Id %s\n", Request_ID );

      for(int i=0;i<sizeof(Request_ID);i++){
          if(Request_ID[i]=='@') Request_ID[i+1]='\0';
      }
      bzero(filename,sizeof filename);
      sprintf(filename,"Client_Submission/%s.c",Request_ID);
      
      system("mkdir Client_Submission 2>/dev/null");
      /* opening and writing the submission in a c file */
      FILE* fd2 = fopen(filename,"w+");
      if(fd2 == NULL){
        printf("E48: Error on opening file\n");
        close(newsockfd);
        continue;
      } 

      int file_size = atoi(buffer);
      printf("file size %d\n",file_size);
      int total_bytes_read = 0;
      bzero(buffer, sizeof buffer);

      /* Recieving the submisiion program file and writing it to a c file <testing<id>.c>*/
      while(total_bytes_read<file_size){

        n = recv(newsockfd, buffer, sizeof buffer, 0);
        if(n <= 0){
          printf("E57: Error on recieving Program file\n");
          fclose(fd2);
          close(newsockfd);
          continue;
        }


        if(fwrite(buffer,1,n,fd2) == -1){
          printf("E62: Error on writing to c file\n");
          fclose(fd2);
          close(newsockfd);
          continue;
        }

        bzero(buffer,sizeof buffer);
        total_bytes_read += n;

      }
      fclose(fd2);      

      /* Sending Request Id to Client*/
      n = send(newsockfd,Request_ID,sizeof Request_ID,0);
      if (n < 0){
          if (errno == ECONNRESET) {
              printf("E75: Error on sending Req ID to client\n");
              continue;
          }
      }

      /* Enqueue the Request ID for Threads */
      pthread_mutex_lock(&queue_lock);
      enqueue(queue,Request_ID);

      /* writitng the queue size to a file */
      bzero(queSize,sizeof queSize);
      queue_size = sizeOf(queue);
      sprintf(queSize,"%d\n#",queue_size);
      int c=0;
      while(queSize[c]!='#') c++;
      FILE* fd3 = fopen("queueSize.txt","a+");
      if(fd3 == NULL){
        perror("E29: Error on opening file in append mode");
        close(sockfd);
        continue;
      }
      if(fwrite(queSize,1,c,fd3) == -1){
        perror("E30: Error on writing to file in append mode");
        close(sockfd);
        continue;
      }
      fclose(fd3);

      pthread_cond_signal(&empty_cond);
      pthread_mutex_unlock(&queue_lock);

      req_recieved = 1;
    }
    else{
      // printf("inside else\n");
      int pos;
      char r;
      buffer[n] = '\0';

      if((pos=find(queue,buffer))!=-1){
        printf("Found Request at Position %d in Queue",pos);
        
        /* Sending Reply to Client*/
        r='b';
        n = send(newsockfd,&r,sizeof r,0);
        if (n < 0){
          if (errno == ECONNRESET) {
              printf("E75: Error on sending Req ID to client\n");
              continue;
          }
        }

        /* Sending reply to client */
        bzero(rep,sizeof rep);
        sprintf(rep,"%d",pos);
        n = send(newsockfd,rep,sizeof rep,0);
        if (n < 0){
          if (errno == ECONNRESET) {
              printf("E76: Error on sending Req ID to client\n");
              continue;
          }
        }
      }
      else{
        
        for(int i=0;i<sizeof(buffer);i++){
          if(buffer[i]=='@') buffer[i+1]='\0';
        }
        char * ungraded_folder = "Client_Submission";
        char filep[2000];
        snprintf(filep,sizeof(filep),"%s/%s.c",ungraded_folder,buffer);
        
        if(access(filep,F_OK)!=-1){
          req_being_served = 1;
        }
        else{
          req_being_served = 0;
        }

        char * folder = "Graded_Files";
        char filepath[2000];
        snprintf(filepath,sizeof(filepath),"%s/%s.txt",folder,buffer);

        if(access(filepath, F_OK) != -1){

          /* Sending Reply to Client*/
          r='c';
          n = send(newsockfd,&r,sizeof r,0);
          if (n < 0){
            if (errno == ECONNRESET) {
                printf("E95: Error on sending Req ID to client\n");
                continue;
            }
          }


          bzero(fname,sizeof fname);
          sprintf(fname,"Graded_Files/%s.txt",buffer);
          fd1 = fopen(fname,"r+");
          fseek(fd1,0L,SEEK_END);
          int si = ftell(fd1);
          fseek(fd1,0L,SEEK_SET);

          char size1[8];
          bzero(size1,sizeof size1);
          sprintf(size1,"%d",si);
          printf("resp size is %d\n",si);
          n = send(newsockfd,size1,sizeof size1,0);
          if(n < 0){
            printf("E43: Error on sending Req ID to client\n");
            continue;
          }


          bzero(buffer, sizeof buffer);
          while(!feof(fd1)){
            
            if( fread(buffer,1,sizeof buffer,fd1) == -1 ) printf("Error on reading\n");
            n = send(newsockfd,buffer,sizeof buffer,0);
            if(n<0) printf("Error on sending file\n");
            bzero(buffer,sizeof buffer);
          }
          fclose(fd1);
        }

        else{

          if(req_being_served){

            /* Sending Reply to Client*/
            r='a';
            n = send(newsockfd,&r,sizeof r,0);
            if (n < 0){
              if (errno == ECONNRESET) {
                  printf("E95: Error on sending Req ID to client\n");
                  continue;
              }
            }
            
          }
          else{
            /* Sending Reply to Client*/
            r='d';
            n = send(newsockfd,&r,sizeof r,0);
            if (n < 0){
              if (errno == ECONNRESET) {
                  printf("E96: Error on sending Req ID to client\n");
                  continue;
              }
            }

          }
          req_being_served = 0;
        }
      }
    }
  }

  freeQueue(queue);
  return 0;
}
