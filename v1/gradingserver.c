#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/stat.h>

void error(char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  int sockfd, newsockfd, portno;
  socklen_t clilen;
  char buffer[256];
  struct sockaddr_in serv_addr, cli_addr;
  int n, x = 1;

  // Check if the port is provided as a command-line argument
  if (argc < 2)
  {
    fprintf(stderr, "ERROR, no port provided\n");
    exit(1);
  }

  // Create a new socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  // Check if the socket is created successfully
  if (sockfd < 0)
    error("ERROR opening socket");

  // Initialize server address structure
  bzero((char *)&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  portno = atoi(argv[1]);
  serv_addr.sin_port = htons(portno);

  // Bind the socket to the server address
  if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    error("ERROR on binding");

  // Listen for incoming connections
  listen(sockfd, 20);
  clilen = sizeof(cli_addr);

  while (1)
  {
    // Accept a new connection
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    // Check if the connection is accepted
    if (newsockfd < 0)
      error("ERROR on accept");

    // Initialize buffer and file descriptors
    bzero(buffer, 256);
    int fd, n1, count = 0, sizeofcli;
    char sizeofclientcode[24];

    // Read the size of client code from the client
    if (read(newsockfd, sizeofclientcode, sizeof(sizeofclientcode)) == -1)
    {
      perror("size code read from client to server");
    };

    // Check if the client wants to break the connection
    if (strcmp(sizeofclientcode, "break") == 0)
    {
      break;
    }

    // Convert the size to an integer
    sizeofcli = atoi(sizeofclientcode);

    // Create or truncate the test.c file for writing
    fd = open("test.c", O_CREAT | O_RDWR | O_TRUNC, S_IRWXU | S_IRWXG | S_IRWXO);

    // Read client code and write it to the file
    while (count < sizeofcli && (n = read(newsockfd, buffer, 255)) > 0)
    {
      if (n < 0)
      {
        error("ERROR reading from socket");
        break;
      }
      count += n;
      write(fd, buffer, n);
    }
    close(fd);

    // Compile the client code using GCC
    int ret = system("gcc -w -o test test.c 2> output.txt");

    if (ret == 0)
    {
      // Run the compiled executable
      ret = system("sh -c './test' 1> output.txt 2> output.txt");

      if (ret == 0)
      {
        // Compare the output with the expected result
        ret = system("diff -w actoutput.txt output.txt > temp.txt");

        if (ret == 0)
        {
          // If the output is as expected, mark as PASS
          system("echo \"PASS\" > output.txt");
        }
        else
        {
          // If there is a difference in the output, mark as Output error
          system("sed -i \"1i Output error\n\" output.txt");
        }
      }
      else
      {
        // If there is a runtime error, mark as Runtime error
        system("sed -i \"1i Runtime error\n\" output.txt");
      }
    }
    else
    {
      // If there is a compilation error, mark as Compilation error
      system("sed -i \"1i Compilation error\n\" output.txt");
    }

    // Get the size of the output file
    struct stat fileStat;
    char outputsize[24];
    if (stat("output.txt", &fileStat) == 0)
    {
      sprintf(outputsize, "%ld", fileStat.st_size);
      write(newsockfd, outputsize, 24);
    }
    else
    {
      perror("stat");
    }

    // Send the contents of the output file to the client
    fd = open("output.txt", O_RDONLY);
    if (fd == -1)
    {
      error("ERROR writing to socket");
    }
    else
    {
      while ((n = read(fd, buffer, sizeof(buffer))) != 0)
      {
        n = write(newsockfd, buffer, n);
        if (n < 0)
          error("ERROR writing to socket");
      }
    }
    close(fd);
    close(newsockfd);
  }
  return 0;
}
