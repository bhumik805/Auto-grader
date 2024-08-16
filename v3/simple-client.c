#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>


int main(int argc, char *argv[]) {

  int sockfd, portno, n;
  struct timeval start_time,end_time,start_time1,end_time1,timeout;
  double elapsed_time; 
  struct sockaddr_in serv_addr; //Socket address structure
  struct hostent *server; //return type of gethostbyname
  timeout.tv_sec = atoi(argv[5]);
  timeout.tv_usec = 0;

  char buffer[1024]; //buffer for message

  if (argc < 6) {
    fprintf(stderr, "usage %s hostname:port filename loopnum sleeptime timeout\n", argv[0]);
    exit(1);
  }
  
  /* tokenizing the input to get hostname and port */
  char* token;
  token = strtok(argv[1],":");
  char* hostname = token;
  token = strtok(NULL,":");
  char* port = token;
  portno = atoi(port); // 2nd argument of the command is port number
  
  
  
  /* fill in server address in sockaddr_in datastructure */
  server = gethostbyname(hostname);
  //finds the IP address of a hostname. 
  //Address is returned in the 'h_addr' field of the hostend struct
  if (server == NULL) {
    printf("E1:ERROR, no such host\n");
    exit(1);
  }

  bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero
  serv_addr.sin_family = AF_INET; // Address Family is IP
  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
  /*Copy server IP address being held in h_addr field of server variable
  to sin_addr.s_addr of serv_addr structure */

  //convert host order port number to network order
  serv_addr.sin_port = htons(portno);
  
  /*Iniitializations for Client Requests*/
  int z=0;
  int res_id;
  int y = atoi(argv[3]);
  int st = atoi(argv[4]);
  double sum=0.0;
  double count=0;
  int timeouts=0;
  int errors=0;

  gettimeofday(&start_time1,NULL);
  while(z<y)
  {
    
    /* create socket, get sockfd handle */
    sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the half socket.     
    if (sockfd < 0)
      printf("E2: ERROR opening socket\n");
      

    /* Set Timeout on Recieve */
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        printf("E3: Set receive timeout failed\n");
        // exit(1);
    }

    /* Set Timeout on Connection */
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0) {
        printf("E11: Set receive timeout failed\n");
        // exit(1);
    }

    /* connect to server 
    First argument is the half-socket, second is the server address structure
    which includes IP address and port number, third is size of 2nd argument
    */
    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
        if (errno == ETIMEDOUT) {
            perror("E12: Timeout on connection");
            timeouts++;
            z++;
            continue;
        }
        else if (errno == ECONNREFUSED){
            perror("E14: Connection failed for some reason");
            errors++;
            z++;
            continue;
        } 
        else {
            perror("E13: Connection failed due to some reason");
            errors++;
            z++;
            continue;
        }
    }
    //If here means connection was complete


    /* Reading the file to be submitted */
    bzero(buffer, 1024); //reset buffer to zero
    FILE* fd1 = fopen(argv[2],"r"); 
    if (fd1 == NULL) {
      perror("E4: Error opening file\n");
      exit(1);
    }
    
    fseek(fd1,0L,SEEK_END);
    long size = ftell(fd1);
    fseek(fd1,0L,SEEK_SET);

    gettimeofday(&start_time,NULL);
    
    /* sending request ID */
    n = send(sockfd,&z,sizeof z,0);
    if (n < 0)
      printf("E5: ERROR writing to socket\n");

    /* sending file size */
    n = send(sockfd,&size,sizeof size,0);
    if (n < 0)
      printf("E6: ERROR writing to socket\n");

    bzero(buffer,sizeof buffer);
    
    /* Sending file to the server */
    while(!feof(fd1)){

      if( fread(buffer,1,sizeof buffer,fd1) == -1 ){
          printf("E7: Error reading from file\n");
      }

      if( send(sockfd, buffer,sizeof buffer,0) == -1){
          printf("E8: Error Sending Data to server\n");
      }

      bzero(buffer, sizeof buffer);
    }
    fclose(fd1);
    

    res_id=-1;
    /* recieving response id */
    n = recv(sockfd,&res_id,sizeof res_id,0);
    if (n < 0){
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        timeouts++;
        z++;
        continue;            
      }
    }

    /* matching request and response ID */
    if(res_id != z){
      errors++;
      z++;
      continue;
    }

    long res_size=0;
    /* recieving the file size */
    n = recv(sockfd,&res_size,sizeof res_size,0);
    if (n < 0){
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
          timeouts++;
          z++;
          continue;            
        }
    }

    /* storing the server response in server_response.txt */
    system("rm -r server_response.txt 2>/dev/null");
    system("touch server_response.txt");

    fd1 = fopen("server_response.txt","w");
    if(fd1 == NULL) 
      printf("E9: Error Opening File - server_response.txt");  

    long total_bytes_read=0;
    bzero(buffer , sizeof buffer);
    /* Reading the reply from server */
    while(total_bytes_read<res_size){
    
      n = recv(sockfd, buffer, sizeof buffer ,0);
      if (n < 0){
        if (errno == EWOULDBLOCK || errno == EAGAIN) {
          break;        
        }
      }

      if(fwrite(buffer,1,n,fd1) == -1){
        printf("E10: Error writing to File\n");
      }
      
      bzero(buffer,sizeof buffer);
      total_bytes_read += n;
    }
    gettimeofday(&end_time,NULL);

    /* Checking if there was any response from server */
    fseek(fd1,0L,SEEK_END);
    long server_response_file_size = ftell(fd1);
    fclose(fd1);
    if(server_response_file_size>0){
      count++;
    }
    else{
      errors++;
    }

    /* Calculating the total time elaspsed for 1 Request-Response */
    elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec)/1e6;
    sum += elapsed_time; // summing the req-res time for total time 

    sleep(st);
    z++;
  }
  close(sockfd);
  gettimeofday(&end_time1,NULL);


  /* calculating elapsed time of loop */
  double elapsed_time1 = (end_time1.tv_sec - start_time1.tv_sec) + (end_time1.tv_usec - start_time1.tv_usec)/1e6;
  
  printf("%f %f %f %d %d\n",sum/y,count,elapsed_time1,timeouts,errors);
  return 0;
}
