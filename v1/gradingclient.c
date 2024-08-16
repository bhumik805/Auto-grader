#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/time.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n, count, sucres = 0;
    struct sockaddr_in serv_addr;
    struct hostent *server;
    struct timeval tv;
    double totrestime = 0, start_s = 0, end_s = 0, start_u = 0, end_u = 0, timetocompleteloop;
    char buffer[256];
    int numofreq = atoi(argv[3]);   // Number of requests to send
    int sleeptime = atoi(argv[4]);  // Sleep time in seconds
    

    // Check if the number of arguments is correct
    if (argc != 5)
    {
        printf("%s  <serverIP:port>  <sourceCodeFileTobeGraded>  <loopNum> <sleepTimeSeconds>\n", argv[0]);
        return 1;
    }

    char *server_addr, *port, *sourceCodeFile;

    // Tokenize the server address and port
    char *token = strtok(argv[1], ":");
    if (token != NULL)
    {
        server_addr = token;
        port = strtok(NULL, ":");
    }
    else
    {
        printf("invalid tokenization");
        return 1;
    }

    sourceCodeFile = argv[2];
    portno = atoi(port);

    // loop through num of requests
    for (int i = 0; i < numofreq; i++)
    {
        // Create a new socket
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0)
            error("ERROR opening socket");

        // Get the server address
        server = gethostbyname(server_addr);
        if (server == NULL)
        {
            fprintf(stderr, "ERROR, no such host\n");
            exit(1);
        }

        // Initialize server address structure
        bzero((char *)&serv_addr, sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
        serv_addr.sin_port = htons(portno);


        // Connect to the server
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR connecting");


        // Measure time elapsed for the current request
        if (gettimeofday(&tv, NULL) == 0)
        {
            start_u = tv.tv_usec;
            start_s = tv.tv_sec;
        }
        else
        {
            perror("gettimeofday");
            return 1;
        }

        struct stat fileStat;
        char sourcCodeFilesize[24];

        // Get the size of the source code file
        if (stat(sourceCodeFile, &fileStat) == 0)
        {
            sprintf(sourcCodeFilesize, "%ld", fileStat.st_size);
            write(sockfd, sourcCodeFilesize, 24);
        }
        else
        {
            perror("stat");
        }

        int fd;
        fd = open(sourceCodeFile, O_RDONLY);
        if (fd < 0)
            error("ERROR opening source code file");

        // Send the source code to the server
        while (count < fileStat.st_size && (n = read(fd, buffer, sizeof(buffer))) > 0)
        {
            if (write(sockfd, buffer, n) < 0)
                error("ERROR writing to socket");
            count += n;
        }
        close(fd);

        bzero(buffer, sizeof(buffer));
        int sizeofrep;
        count = 0;
        char sizeofrepsponse[24];

        // Read the size of the server response
        if (read(sockfd, sizeofrepsponse, sizeof(sizeofrepsponse)) == -1)
        {
            perror("size code read from client to server");
        };
        sizeofrep = atoi(sizeofrepsponse);

        // Read the server response
        while (count < sizeofrep && (n = read(sockfd, buffer, sizeof(buffer))) > 0)
        {
            if (n < 0)
                error("ERROR reading from server");
            sucres += 1;
            bzero(buffer, sizeof(buffer));
        }

        // Measure time elapsed for the current request
        if (gettimeofday(&tv, NULL) == 0)
        {
            end_u = tv.tv_usec;
            end_s = tv.tv_sec;

            if (start_s < end_s)
            {
                if (start_u < end_u)
                {
                    totrestime += ((end_s - start_s) + (end_u - start_u) / 1000000);
                }
                else
                {
                    totrestime += ((end_s - start_s) - (start_u - end_u) / 1000000);
                }
            }
            else
            {
                totrestime += ((end_u - start_u) / 1000000);
            }
        }
        else
        {
            perror("gettimeofday");
            return 1;
        }

        // Sleep for the specified time before the next request
        sleep(sleeptime);

        // Measure time elapsed for the entire loop
        if (gettimeofday(&tv, NULL) == 0)
        {
            end_u = tv.tv_usec;
            end_s = tv.tv_sec;

            if (start_s < end_s)
            {
                if (start_u < end_u)
                {
                    timetocompleteloop += ((end_s - start_s) + (end_u - start_u) / 1000000);
                }
                else
                {
                    timetocompleteloop += ((end_s - start_s) - (start_u - end_u) / 1000000);
                }
            }
            else
            {
                timetocompleteloop += ((end_u - start_u) / 1000000);
            }
        }
        else
        {
            perror("gettimeofday");
            return 1;
        }
    }

    // Close the socket
    close(sockfd);

    // Print the average response time, successful responses, and time to complete the entire loop
    printf("%lf %d %lf", totrestime / numofreq, sucres, timetocompleteloop);

    return 0;
}
