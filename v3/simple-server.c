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
    if(pthread_create(thread_ids+i, NULL, &gradeSubmission, td)!=0){
      perror("E34: Failed to create Thread\n");
      exit(EXIT_FAILURE);
    }
  }

  int queue_size=0;
  char queSize[100];
  struct commands* cmds = (struct commands*) malloc(sizeof(struct commands));
  cmds = create_commands(1);

  /* accept a new request, create a newsockfd */
  while (1) {
    
    int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
    if (newsockfd < 0)
      printf("E33: ERROR on accept\n");
    
    /* Queeuing the socketfd's of the connected requests */
    pthread_mutex_lock(&queue_lock);
    enqueue(queue,newsockfd);

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

    pthread_cond_signal(&empty_cond);
    pthread_mutex_unlock(&queue_lock);

  }

  freeQueue(queue);
  return 0;
}
