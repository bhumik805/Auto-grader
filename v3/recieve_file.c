#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <netinet/in.h>
#include <unistd.h>
#include "structures.h"

int recieve_file(struct commands* cmds,int sockfd){

  /* buffer for file */
  char buffer[1024];
  bzero(buffer, 1024); 
  int n=0;
  int req_id;
  long file_size = 0;

  /* Recieve the Request ID */
  n = recv(sockfd, &req_id, sizeof req_id, 0);
  if(n <= 0){
    printf("E1: Error on recieving request Id\n");
    close(sockfd);
    pthread_exit(NULL);
  }

  /* Recieve the file size */
  n = recv(sockfd, &file_size, sizeof file_size, 0);
  if(n <= 0){
    printf("E2: Error on recieving File Size\n");
    close(sockfd);
    pthread_exit(NULL);
  }

  /* File Size Garbled */
  if(file_size >= 1000000){
    printf("E3: Error File Size Garbled\n");
    close(sockfd);
    pthread_exit(NULL);
  }

  printf("sockfd is %d\tSize: %ld\tReq_id is %d\n",sockfd,file_size,req_id);

  /* opening and writing the submission in a c file */
  FILE* fd2 = fopen(cmds->open_test,"w+");
  if(fd2 == NULL){
    printf("E4: Error on opening file\n");
    close(sockfd);
    pthread_exit(NULL);
  } 

  int total_bytes_read = 0;
  bzero(buffer, sizeof buffer);

  /* Recieving the submisiion program file and writing it to a c file <testing<id>.c>*/
  while(total_bytes_read<file_size){
    
    n = recv(sockfd, buffer, sizeof buffer, 0);
    if(n <= 0){
      printf("E5: Error on recieving Program file\n");
      fclose(fd2);
      close(sockfd);
      pthread_exit(NULL);
    }

    if(fwrite(buffer,1,n,fd2) == -1){
      printf("E6: Error on writing to c file\n");
      fclose(fd2);
      close(sockfd);
      pthread_exit(NULL);
    }

    bzero(buffer,sizeof buffer);
    total_bytes_read += n;
  }

  fclose(fd2);
  return req_id;

}