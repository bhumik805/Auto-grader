/* run client using: ./client localhost <server_port> */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>

void error(char *msg) {
  perror(msg);
  exit(0);
}

int main(int argc, char *argv[]) {
  int sockfd, portno, n;
  struct timeval start_time,end_time,start_time1,end_time1,timeout;
  double elapsed_time; 
  struct sockaddr_in serv_addr; //Socket address structure
  struct hostent *server; //return type of gethostbyname

  char buffer[1024]; //buffer for message

  if (argc < 6) {
    fprintf(stderr, "usage %s hostname:port filename loopnum sleeptime timeout\n", argv[0]);
    exit(0);
  }
  
  char* token;
  token = strtok(argv[1],":");
  char* hostname = token;
  token = strtok(NULL,":");
  char* port = token;
  portno = atoi(port); // 2nd argument of the command is port number
  timeout.tv_sec = atoi(argv[5]);
  timeout.tv_usec = 0;
  

  server = gethostbyname(hostname);
  //finds the IP address of a hostname. 
  //Address is returned in the 'h_addr' field of the hostend struct

  if (server == NULL) {
    fprintf(stderr, "ERROR, no such host\n");
    exit(0);
  }

  bzero((char *)&serv_addr, sizeof(serv_addr)); // set server address bytes to zero

  serv_addr.sin_family = AF_INET; // Address Family is IP

  bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
/*Copy server IP address being held in h_addr field of server variable
to sin_addr.s_addr of serv_addr structure */

//convert host order port number to network order
  serv_addr.sin_port = htons(portno);



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
  
  sockfd = socket(AF_INET, SOCK_STREAM, 0); //create the half socket. 
  
  if (sockfd < 0)
    error("ERROR opening socket");

  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0) {
        perror("Set receive timeout failed");
        exit(1);
  }

  if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR connecting");
  
  
  bzero(buffer, 1024); //reset buffer to zero
  FILE* fd1 = fopen(argv[2],"r");
   
  if (fd1 == NULL) {
    perror("Error opening file");
    return 1;
  }
  
  fseek(fd1,0L,SEEK_END);
  long size = ftell(fd1);
  fseek(fd1,0L,SEEK_SET);

  gettimeofday(&start_time,NULL);
  
  n = send(sockfd,&z,sizeof z,0);
  if (n < 0)
    error("ERROR writing to socket");
  
  n = send(sockfd,&size,sizeof size,0);
  if (n < 0)
    error("ERROR writing to socket");


  /* send user message to server 
  write call: first argument is socket FD, 2nd is the string to write, 3rd is length of 2nd
  */
  
  bzero(buffer,sizeof buffer);
  while(!feof(fd1)){

    if( fread(buffer,1,sizeof buffer,fd1) == -1 ){
        error("Error reading from file");
    }
    
    if( send(sockfd, buffer,sizeof buffer,0) == -1){
        error("Error Sending Data to server");
    }
    bzero(buffer, sizeof buffer);
  }
  fclose(fd1);
  /* read reply from server 
  First argument is socket, 2nd is string to read into, 3rd is number of bytes to read
  */
  

  res_id=-1;
  n = recv(sockfd,&res_id,sizeof res_id,0);
  if (n < 0){
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
        
        timeouts++;
        z++;
        continue;            
      }
  }
  char tempo[200];
  bzero(tempo,sizeof tempo);
  
  if(res_id != z){
    errors++;
    z++;
    continue;
  }

  long res_size=0;
  n = recv(sockfd,&res_size,sizeof res_size,0);
  if (n < 0){
    if (errno == EWOULDBLOCK || errno == EAGAIN) {
        
        timeouts++;
        z++;
        continue;            
      }
    
  }
  system("rm -r server_response.txt 2>/dev/null");
  system("touch server_response.txt");

  fd1 = fopen("server_response.txt","w");
  if(fd1 == NULL) 
    error("Error Opening File - server_response.txt");  

  long total_bytes_read=0;
  bzero(buffer , sizeof buffer);
  while(total_bytes_read<res_size){
  
    n = recv(sockfd, buffer, sizeof buffer ,0);
     
    if (n < 0){
      if (errno == EWOULDBLOCK || errno == EAGAIN) {
        break;            
      }
     
    }
    if(fwrite(buffer,1,n,fd1) == -1){
        error("Error writing to File");
    }
    
    bzero(buffer,sizeof buffer);
    total_bytes_read += n;
  }
  
  gettimeofday(&end_time,NULL);
  fseek(fd1,0L,SEEK_END);
  long server_response_file_size = ftell(fd1);
  fclose(fd1);
  if(server_response_file_size>0) count++;

  elapsed_time = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec)/1e6;
  sum += elapsed_time;
  sleep(st);
  z++;
  
  }

  close(sockfd);
  gettimeofday(&end_time1,NULL);
  double elapsed_time1 = (end_time1.tv_sec - start_time1.tv_sec) + (end_time1.tv_usec - start_time1.tv_usec)/1e6;

  printf("%f %f %f %d %d\n",sum/y,count,elapsed_time1,timeouts,errors);
  return 0;
}
