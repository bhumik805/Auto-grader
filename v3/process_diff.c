#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include "structures.h"

void process_diff(struct commands* cmds,int sockfd, int res_id){
    
    FILE* fd1;
    long size;
    int n=0;
    char buffer[1024];
    bzero(buffer, sizeof buffer);

    fd1 = fopen(cmds->open_diff,"ra+");
      if(fd1 == NULL){
        printf("E19: Error on opening file\n");
        fclose(fd1);
        close(sockfd);
        pthread_exit(NULL);
      } 

      fseek(fd1, 0L, SEEK_END); 
      size = ftell(fd1);
      fseek(fd1,0,SEEK_SET);

      /* Sending response Id */
      n = send(sockfd,&res_id,sizeof res_id,0);
      if (n < 0){
        if (errno == ECONNRESET) {
          printf("E20: Error on sending response id to client\n");
          fclose(fd1);
          close(sockfd);
          pthread_exit(NULL);
        }
      }

      /* Sending file size */
      n = send(sockfd,&size,sizeof(size),0);
      if (n < 0){
        if (errno == ECONNRESET) {
          printf("E21: Error on sending file size to client\n");
          fclose(fd1);
          close(sockfd);
          pthread_exit(NULL);
        }
      }
      
      /* appending to diff output file */
      if(fwrite("Difference in Output:\n",1,22,fd1) == -1){
          printf("E22: Error on appending\n");
          fclose(fd1);
          close(sockfd);
          pthread_exit(NULL);
      }

      bzero(buffer, sizeof buffer);
      
      /* Sending response file to client */
      while(!feof(fd1)){

        if( fread(buffer,1,sizeof buffer,fd1) == -1 ){
          printf("E23: Error on reading file\n");
          fclose(fd1);
          close(sockfd);
          pthread_exit(NULL);
        }

        n = send(sockfd,buffer,sizeof buffer,0);
        if (n < 0){
          if (errno == ECONNRESET) {
            printf("E24: Error on sending file to client\n");
            fclose(fd1);
            close(sockfd);
            pthread_exit(NULL);
          }
        } 

        bzero(buffer,sizeof buffer);
      }

    fclose(fd1);
}
