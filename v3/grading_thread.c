#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <strings.h>
#include "utils.h"

extern pthread_mutex_t queue_lock;
extern pthread_cond_t empty_cond;
extern struct Queue* queue;

void *gradeSubmission(void *td) {

  int id = (*(int *)td);
  int queue_size = 0;
  int sockfd = -1;
  char queSize[100];
  struct commands* cmds= (struct commands*)malloc(sizeof(struct commands));

  /* Creating all the commands Required */
  cmds = create_commands(id);

  /* creating a file for storing the size of queue */
  if(system(cmds->create_queue_size)<0){
    close(sockfd);
    perror("E28: Error on creating file queueSize.txt");
    pthread_exit(NULL);
  }
  

  /* Thread */
  while(1){

    /* waiting on empty queue */
    pthread_mutex_lock(&queue_lock);
    
    while(isEmpty(queue))
    {
      pthread_cond_wait(&empty_cond,&queue_lock);
    }
    
    pthread_mutex_unlock(&queue_lock);
    
    pthread_mutex_lock(&queue_lock);
    if(!isEmpty(queue))
    {
      sockfd = dequeue(queue);

      /* writitng the queue size to a file */
      bzero(queSize,sizeof queSize);
      queue_size = sizeOf(queue);
      sprintf(queSize,"%d\n#",queue_size);
      int c=0;
      while(queSize[c]!='#') c++;
      FILE* fd3 = fopen(cmds->append_queue_size,"a+");
      if(fd3 == NULL){
        perror("E29: Error on opening file in append mode");
        close(sockfd);
        pthread_exit(NULL);
      }
      if(fwrite(queSize,1,c,fd3) == -1){
        perror("E30: Error on writing to file in append mode");
        close(sockfd);
        pthread_exit(NULL);
      }
      fclose(fd3);

    }
    pthread_mutex_unlock(&queue_lock);

    /* clearing files to avoid rewriting */
    system(cmds->rm_out);
    system(cmds->rm_diff);
    system(cmds->rm_comp_err);
    system(cmds->rm_run_err);
    system(cmds->rm_test);
    system(cmds->rm_binaries);

    char buffer[1024];
    bzero(buffer, 1024); 
    int n=0;
    /* Recieve file from client */
    int req_id = recieve_file(cmds,sockfd);

    /* Initialization for processing */
    int res_id = req_id;

    /* compiling the program */
    if (system(cmds->compile)!=0){

      /*Processing on Compilation Error*/      
      process_compile_error(cmds,sockfd,res_id);
    }

    /* running the program */
    else if(system(cmds->run)!=0){
      
      /* Processing Runtime Error */
      process_runtime_error(cmds,sockfd,res_id);
    }

    /* comparing the output */
    else if(system(cmds->diff)!=0){
    
      /* Processing the difference in output */
      process_diff(cmds,sockfd,res_id);
    }

    /* Output is correct */
    else{
        
      /* Sending a PASS response to client */
      long size = 4;
      n = send(sockfd,&res_id,sizeof res_id,0);
      if (n < 0){
        if (errno == ECONNRESET) {
          printf("E25: Error on sending response id to client\n");
          close(sockfd);
          pthread_exit(NULL);
        }
      }

      n = send(sockfd,&size,sizeof(size),0);
      if (n < 0){
        if (errno == ECONNRESET) {
          printf("E26: Error on sending file size to client\n");
          close(sockfd);
          pthread_exit(NULL);
        }
      }
      
      n = send(sockfd, "PASS", 4,0);
      if (n < 0){
        if (errno == ECONNRESET) {
          printf("E27: Error on sending response to client\n");
          close(sockfd);
          pthread_exit(NULL);
        }
      }
    }

    /* Clearing all the files created */
    system(cmds->rm_out);
    system(cmds->rm_diff);
    system(cmds->rm_comp_err);
    system(cmds->rm_run_err);
    system(cmds->rm_test);
    system(cmds->rm_binaries);

    close(sockfd);
    printf("thread %d exited\n",id);
  }

  free(cmds);
}