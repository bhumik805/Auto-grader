#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <errno.h>
#include <pthread.h>


struct id_socket{
  int id;
  int sockfd;
};


void *start_function(void *thread_data) {
  
  /* Giving Id and SockFd to each Thread */
  int id=((struct id_socket*)thread_data)->id;
  int sockfd=((struct id_socket*)thread_data)->sockfd;
  free(thread_data);
  
  char buffer[1024];
  int n=0;
  int req_id;

  bzero(buffer, 1024); 
  long file_size = 0;

  /* Recieving Req Id */
  n = recv(sockfd, &req_id, sizeof req_id, 0);
  if(n <= 0){
    close(sockfd);
    return 0;
    
  }
  
  /* Recieving file size */
  n = recv(sockfd, &file_size, sizeof file_size, 0);
  if(n <= 0){
    close(sockfd);
    return 0;
    
  }

  if(file_size >= 1000000){
    close(sockfd);
    return 0;
  }

  /* creating all commands with thread ID */
  char str_id[100];
  sprintf(str_id,"%d",id);

  char rm_out[200];
  bzero(rm_out,sizeof rm_out);
  sprintf(rm_out,"rm -r output%d.txt 2>/dev/null",id);

  char rm_diff[200];
  bzero(rm_diff,sizeof rm_diff);
  sprintf(rm_diff,"rm -r diff%d.txt 2>/dev/null",id);
  

  char rm_comp_err[200];
  bzero(rm_comp_err,sizeof rm_comp_err);
  sprintf(rm_comp_err,"rm -r comp_err%d.txt 2>/dev/null",id);

  char rm_run_err[200];
  bzero(rm_run_err,sizeof rm_run_err);
  sprintf(rm_run_err,"rm -r run_err%d.txt 2>/dev/null",id);

  char rm_test[200];
  bzero(rm_test,sizeof rm_test);
  sprintf(rm_test,"rm -r testing%d.c 2>/dev/null",id);

  char open_test[200];
  bzero(open_test,sizeof open_test);
  sprintf(open_test,"testing%d.c",id);
  
  char open_comp_err[200];
  bzero(open_comp_err,sizeof open_comp_err);
  sprintf(open_comp_err,"comp_err%d.txt",id);

  char open_run_err[200];
  bzero(open_run_err,sizeof open_run_err);
  sprintf(open_run_err,"run_err%d.txt",id);

  char open_diff[200];
  bzero(open_diff, sizeof open_diff);
  sprintf(open_diff,"diff%d.txt",id);

  char compile[200];
  bzero(compile,sizeof compile);
  sprintf(compile,"gcc testing%d.c -o testing%d.o 2>comp_err%d.txt",id,id,id);

  char run[200];
  bzero(run,sizeof run);
  sprintf(run,"./testing%d.o 1>output%d.txt 2>run_err%d.txt",id,id,id);

  char diff[200];
  bzero(diff,sizeof diff);
  sprintf(diff,"diff output%d.txt exp_out.txt > diff%d.txt",id,id);

  char rm_binaries[200];
  bzero(rm_binaries,sizeof rm_binaries);
  sprintf(rm_binaries,"rm -r testing%d.o 2>/dev/null",id);

  /* Clearing the files */
  system(rm_out);
  system(rm_diff);
  system(rm_comp_err);
  system(rm_run_err);
  system(rm_test);
  system(rm_binaries);

  /* Redinag the file from server */
  FILE* fd2 = fopen(open_test,"w");
  if(fd2 == NULL){
    system(rm_test);
    close(sockfd);
    return 0;
  } 

  int total_bytes_read = 0;
  bzero(buffer, sizeof buffer);

  while(total_bytes_read<file_size){
    
    n = recv(sockfd, buffer, sizeof buffer, 0);
    if(n <= 0){
      system(rm_test);
      fclose(fd2);
      close(sockfd);
      return 0;
    }

    if(fwrite(buffer,1,n,fd2) == -1){
      fclose(fd2);
      system(rm_test);
      close(sockfd);
      return 0;
    }

    bzero(buffer,sizeof buffer);
    total_bytes_read += n;
  }
  fclose(fd2);
  
  /* Processing Compilation Error */
  FILE* fd1;
  long size;
  int res_id = req_id;


  if (system(compile)!=0){

    fd1 = fopen(open_comp_err,"ra+"); 
    if(fd1 == NULL){
      system(rm_comp_err);
      system(rm_test);
      close(sockfd);
      return 0;
    }

    fseek(fd1, 0L, SEEK_END); 
    size = ftell(fd1);
    fseek(fd1,0,SEEK_SET);
    
    n = send(sockfd,&res_id,sizeof res_id,0);
    if (n < 0){
      if (errno == ECONNRESET) {
        fclose(fd1);
        close(sockfd);
        return 0;
      }
    }

    n = send(sockfd,&size,sizeof(size),0);
    if (n < 0){
      if (errno == ECONNRESET) {
        fclose(fd1);
        close(sockfd);
        return 0;
      }
    }
    
    if(fwrite("Compilation Error:\n",1,19,fd1) == -1){
      fclose(fd1);
      close(sockfd);
      return 0;
    }

    bzero(buffer, sizeof buffer);
    
    while(!feof(fd1)){
      if( fread(buffer,1,sizeof buffer,fd1) == -1 ){
        fclose(fd1);
        close(sockfd);
        return 0;
      }

      n = send(sockfd,buffer,sizeof buffer,0);
      if (n < 0){
        if (errno == ECONNRESET) {
          fclose(fd1);
          close(sockfd);
          return 0;
        }
      }
      bzero(buffer,sizeof buffer); 
    }
      
    fclose(fd1);
  }
  
  /* Processing Runtime Error */
  else if(system(run)!=0){

    fd1 = fopen(open_run_err,"ra+");
    if(fd1 == NULL){
        close(sockfd);
        return 0;
    }
    fseek(fd1, 0L, SEEK_END); 
    size = ftell(fd1);
    fseek(fd1,0,SEEK_SET);

    n = send(sockfd,&res_id,sizeof res_id,0);
    if (n < 0){
      if (errno == ECONNRESET) {
        fclose(fd1);
        close(sockfd);
        return 0;
      }
    }

    n = send(sockfd,&size,sizeof(size),0);
    if (n < 0){
        fclose(fd1);
        close(sockfd);
        return 0;
    }
    if(fwrite("Runtime Error:\n",1,15,fd1) == -1){
        fclose(fd1);
        close(sockfd);
        return 0;
    }
  
    fseek(fd1,0L,SEEK_SET);
    bzero(buffer, sizeof buffer);
    

    while(!feof(fd1)){

      if( fread(buffer,1,sizeof buffer,fd1) == -1 ){
        fclose(fd1);
        close(sockfd);
        return 0;

      }

      n = send(sockfd,buffer,sizeof buffer,0);
      if (n < 0){
        if (errno == ECONNRESET) {
        fclose(fd1);
        close(sockfd);

          return 0;
        }
      }

      bzero(buffer,sizeof buffer);
    }

    
    fclose(fd1);

  } 


  /* Processing Difference Error */
  else if(system(diff)!=0){
  

    fd1 = fopen(open_diff,"ra+");
    if(fd1 == NULL){
        fclose(fd1);
        close(sockfd);
        return 0;

    } 
    fseek(fd1, 0L, SEEK_END); 
    size = ftell(fd1);
    fseek(fd1,0,SEEK_SET);


    n = send(sockfd,&res_id,sizeof res_id,0);
    if (n < 0){
      if (errno == ECONNRESET) {
        fclose(fd1);
        close(sockfd);
        return 0;
      }
    }

    n = send(sockfd,&size,sizeof(size),0);
    if (n < 0){
      if (errno == ECONNRESET) {
        fclose(fd1);
        close(sockfd);

        return 0;
      }
    }
    
    if(fwrite("Difference in Output:\n",1,22,fd1) == -1){
        fclose(fd1);
        close(sockfd);
        return 0;
    }

    bzero(buffer, sizeof buffer);
    
    while(!feof(fd1)){
      if( fread(buffer,1,sizeof buffer,fd1) == -1 ){
        fclose(fd1);
        close(sockfd);
        return 0;
      }
      n = send(sockfd,buffer,sizeof buffer,0);
      if (n < 0){
        if (errno == ECONNRESET) {
        fclose(fd1);
        close(sockfd);

          return 0;
        }
      } 
      bzero(buffer,sizeof buffer);
    }

  
    fclose(fd1);

  }

  /* Processing  PASS response */
  else{

    size = 4;

    n = send(sockfd,&res_id,sizeof res_id,0);
    if (n < 0){
      if (errno == ECONNRESET) {
        close(sockfd);
        return 0;
      }
    }

    n = write(sockfd,&size,sizeof(size));
    if (n < 0){
      if (errno == ECONNRESET) {
        close(sockfd);
        return 0;
      }
    }
    
    n = write(sockfd, "PASS", 4);
    if (n < 0){
      if (errno == ECONNRESET) {
        close(sockfd);
        return 0;
      }
    }
      
  }

  /* Clearing all files and resetting the bufferes to zero */
  system(rm_out);
  system(rm_diff);
  system(rm_comp_err);
  system(rm_run_err);
  system(rm_test);
  system(rm_binaries);

  bzero(rm_out,sizeof rm_out);
  bzero(rm_diff,sizeof rm_diff);
  bzero(rm_comp_err,sizeof rm_comp_err);
  bzero(rm_run_err,sizeof rm_run_err);
  bzero(rm_test,sizeof rm_test);
  bzero(rm_binaries,sizeof rm_binaries);
  bzero(open_test,sizeof open_test);
  bzero(open_run_err,sizeof open_run_err);
  bzero(open_comp_err,sizeof open_comp_err);
  bzero(open_diff,sizeof open_diff);
  bzero(compile,sizeof compile);
  bzero(run,sizeof run);
  bzero(diff,sizeof diff);



  close(sockfd);

}


int main(int argc, char *argv[]) {
  
  
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  struct sockaddr_in serv_addr, cli_addr;
  int n;

  if (argc < 2) {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  /* create socket */

  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0)
    perror("ERROR opening socket");
  

  /* fill in port number to listen on. IP address can be anything (INADDR_ANY)
   */

  bzero((char *)&serv_addr, sizeof(serv_addr));
  portno = atoi(argv[1]);
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  /* bind socket to this port number on this machine */

  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    perror("ERROR on binding");

  /* listen for incoming connection requests */

  listen(sockfd, 100);
  clilen = sizeof(cli_addr);

  /* accept a new request, create a newsockfd */
  int id=0;
  
  // int u=0;
  while (1) {

    struct id_socket* thread_data = (struct id_socket *)malloc(sizeof(struct id_socket)); 
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      perror("ERROR on accept");
    
    
    pthread_t thread;
    
    id++;
    
    thread_data->id = id;
    thread_data->sockfd = newsockfd;

    if (pthread_create(&thread, NULL, start_function, thread_data) != 0)
      printf("Failed to create Thread\n");

  }

  
  return 0;
}
