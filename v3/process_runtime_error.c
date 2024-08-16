#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include "structures.h"

void process_runtime_error(struct commands* cmds, int sockfd, int res_id){

    FILE* fd1;
    long size;
    int n=0;
    char buffer[1024];
    bzero(buffer, sizeof buffer);

    fd1 = fopen(cmds->open_run_err,"ra+");
    if(fd1 == NULL){
        printf("E13: Error on opening file\n");
        close(sockfd);
        pthread_exit(NULL);
    }

    fseek(fd1, 0L, SEEK_END); 
    size = ftell(fd1);
    fseek(fd1,0,SEEK_SET);

    /* Sending Response Id */
    n = send(sockfd,&res_id,sizeof res_id,0);
    if (n < 0){
        if (errno == ECONNRESET) {
            printf("E14: Error on sending response Id\n");
            fclose(fd1);
            close(sockfd);
            pthread_exit(NULL);
        }
    }

    /* sending file size to client */
    n = send(sockfd,&size,sizeof(size),0);
    if (n < 0){
        printf("E15: Error on sending file size to client\n");
        fclose(fd1);
        close(sockfd);
        pthread_exit(NULL);
    }

    /* appending to file runtime error */
    if(fwrite("Runtime Error:\n",1,15,fd1) == -1){
        printf("E16: Error on appending to file\n");
        fclose(fd1);
        close(sockfd);
        pthread_exit(NULL);
    }

    fseek(fd1,0L,SEEK_SET);
    bzero(buffer, sizeof buffer);
    
    /* sending runtime error file to client */
    while(!feof(fd1)){
    
        if( fread(buffer,1,sizeof buffer,fd1) == -1 ){
            printf("E17: Error on reading file\n");
            fclose(fd1);
            close(sockfd);
            pthread_exit(NULL);
        }

        n = send(sockfd,buffer,sizeof buffer,0);
        if (n < 0){
            if (errno == ECONNRESET) {
                printf("E18: Error on sending file to client\n");
                fclose(fd1);
                close(sockfd);
                pthread_exit(NULL);
            }
        }
            
        bzero(buffer,sizeof buffer);
    }

    fclose(fd1);
}