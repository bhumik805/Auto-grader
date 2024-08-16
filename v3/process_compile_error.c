#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <netinet/in.h>
#include "structures.h"

void process_compile_error(struct commands* cmds,int sockfd,int res_id){

    FILE* fd1;
    long size;
    int n=0;
    char buffer[1024];
    bzero(buffer, sizeof buffer);

    fd1 = fopen(cmds->open_comp_err,"ra+"); 
    if(fd1 == NULL){
        printf("E7: Error on opening file <comp_err>\n");
        close(sockfd);
        pthread_exit(NULL);
    }

    fseek(fd1, 0L, SEEK_END); 
    size = ftell(fd1);
    fseek(fd1,0,SEEK_SET);
    
    /* Sending Response ID */
    n = send(sockfd,&res_id,sizeof res_id,0);
    if (n < 0){
        if (errno == ECONNRESET) {
            printf("E8: Error on sending response Id\n");
            fclose(fd1);
            close(sockfd);
            pthread_exit(NULL);
        }
    }

    /* sending file size */
    n = send(sockfd,&size,sizeof(size),0);
    if (n < 0){
        if (errno == ECONNRESET) {
            printf("E9: Error on sending file size\n");
            fclose(fd1);
            close(sockfd);
            pthread_exit(NULL);
        }
    }
    
    /* appending to compilation error file */
    if(fwrite("Compilation Error:\n",1,19,fd1) == -1){
        printf("E10: Error on appending to file\n");
        fclose(fd1);
        close(sockfd);
        pthread_exit(NULL);
    }

    bzero(buffer, sizeof buffer);
    
    /* Sending the reply to client in a file */
    while(!feof(fd1)){

        if( fread(buffer,1,sizeof buffer,fd1) == -1 ){
            printf("E11: Error on Reading file\n");
            fclose(fd1);
            close(sockfd);
            pthread_exit(NULL);
        }

        n = send(sockfd,buffer,sizeof buffer,0);
        if (n < 0){
            if (errno == ECONNRESET) {
            printf("E12: Error on sending file to client\n");
            fclose(fd1);
            close(sockfd);
            pthread_exit(NULL);
            }
        }

        bzero(buffer,sizeof buffer); 
    }
    
    fclose(fd1);
}