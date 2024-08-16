#include <stdio.h>
#include <errno.h>
#include <pthread.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "utils.h"


void *gradeSubmission(void *td) {


  int queue_size = 0;
  char ID[30];
  char *t;
  char queSize[100];
  struct commands* cmds= (struct commands*)malloc(sizeof(struct commands));
  FILE* fd1;
  system("mkdir Graded_Files 2>/dev/null");
  
  

  /* Thread */
  while(1){
    sleep(20);
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
      t = dequeue(queue);
      strcpy(ID,t);
      free(t); 
      /* writitng the queue size to a file */
      bzero(queSize,sizeof queSize);
      queue_size = sizeOf(queue);
      sprintf(queSize,"%d\n#",queue_size);
      int c=0;
      while(queSize[c]!='#') c++;
      FILE* fd3 = fopen("queueSize.txt","a+");
      if(fd3 == NULL){
        perror("E29: Error on opening file in append mode");
        // close(sockfd);
        continue;
      }
      if(fwrite(queSize,1,c,fd3) == -1){
        perror("E30: Error on writing to file in append mode");
        // close(sockfd);
        continue;
      }
      fclose(fd3);

    }
    pthread_mutex_unlock(&queue_lock);


    /* Creating all the commands Required */
    cmds = create_commands(ID);
    printf("Thread created with ID %s\n",ID);
    char buffer[1024];
    bzero(buffer, 1024); 
    int n=0;
    

    /* compiling the program */
    if (system(cmds->compile)!=0){
    }

    /* running the program */
    else if(system(cmds->run)!=0){

    }

    /* comparing the output */
    else if(system(cmds->diff)!=0){

    }

    /* Output is correct */
    else{

      char name[200];
      bzero(name,sizeof name);
      sprintf(name,"Graded_Files/%s.txt",ID);
      fd1 = fopen(name,"w+");
      fwrite("PASS",1,4,fd1);
      fclose(fd1);   

    }

    printf("thread %s exited\n",ID);
    
  }

  free(cmds);
}