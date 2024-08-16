#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <netdb.h>
#include <errno.h>
#include <fcntl.h>

// Function to check timeout error
int checkTimeoutError(int n)
{
    if (n < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
        {
            printf("E14: Timeout\n");
            // exit(1);
        }
    }
}

int main(int argc, char *argv[])
{
    // Variable declarations
    int sockfd, portno, n;
    struct timeval start_time, end_time, timeout;
    double elapsed_time;
    struct sockaddr_in serv_addr; // Socket address structure
    struct hostent *server;       // Return type of gethostbyname
    timeout.tv_sec = atoi(argv[5]);
    timeout.tv_usec = 0;

    char buffer[1024]; // Buffer for message

    // Command line argument validation
    if (argc < 4)
    {
        fprintf(stderr, "./submit  <new|status> <serverIP:port> <sourceCodeFileTobeGraded|requestID>\n");
        exit(1);
    }

    // Tokenizing the input to get hostname and port
    char *token;
    token = strtok(argv[2], ":");
    char *hostname = token;
    token = strtok(NULL, ":");
    char *port = token;
    portno = atoi(port); // 2nd argument of the command is the port number

    // Fill in server address in sockaddr_in data structure
    server = gethostbyname(hostname);
    // Finds the IP address of a hostname.
    // Address is returned in the 'h_addr' field of the hostent struct
    if (server == NULL)
    {
        printf("E1:ERROR, no such host\n");
        exit(1);
    }

    bzero((char *)&serv_addr, sizeof(serv_addr)); // Set server address bytes to zero
    serv_addr.sin_family = AF_INET;               // Address Family is IP
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    // Copy server IP address being held in h_addr field of the server variable
    // to sin_addr.s_addr of serv_addr structure

    // Convert host order port number to network order
    serv_addr.sin_port = htons(portno);

    // Iniitializations for Client Requests
    double sum = 0.0;
    int errors = 0;
    char *request_type = argv[1];

    // Create socket, get sockfd handle
    sockfd = socket(AF_INET, SOCK_STREAM, 0); // Create the half socket.
    if (sockfd < 0)
        printf("E2: ERROR opening socket\n");

    // Set Timeout on Receive
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        printf("E3: Set receive timeout failed\n");
        // exit(1);
    }

    // Set Timeout on Connection
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        printf("E11: Set receive timeout failed\n");
        // exit(1);
    }

    // Connect to the server
    if (!strcmp(request_type, "new"))
    {
        int count = 0;
        while (count < 10 && connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            count++;
        }
        if (count == 10)
        {
            printf("E12: Connection Timeout\n");
            exit(1);
        }
        bzero(buffer, 1024); // Reset buffer to zero
        FILE *fd1 = fopen(argv[3], "r");
        if (fd1 == NULL)
        {
            perror("E4: Error opening file\n");
            exit(1);
        }

        fseek(fd1, 0L, SEEK_END);
        long size = ftell(fd1);
        fseek(fd1, 0L, SEEK_SET);

        gettimeofday(&start_time, NULL);
        char start_time_string[40];
        sprintf(start_time_string, "%ld", start_time.tv_sec);

        // Sending file size
        char fsize[1024];
        bzero(fsize, sizeof fsize);
        sprintf(fsize, "%ld", size);
        n = send(sockfd, fsize, sizeof fsize, 0);
        if (n < 0)
            printf("E6: ERROR writing to socket\n");

        bzero(buffer, sizeof buffer);

        // Sending file to the server
        while (!feof(fd1))
        {
            if (fread(buffer, 1, sizeof buffer, fd1) == -1)
            {
                printf("E7: Error reading from file\n");
            }
            printf("%s\n", buffer);
            if ((n = send(sockfd, buffer, sizeof buffer, 0)) == -1)
            {
                printf("E8: Error Sending Data to server\n");
            }

            bzero(buffer, sizeof buffer);
        }
        fclose(fd1);

        // Receiving request id
        char req_id_buf[30];
        bzero(req_id_buf, sizeof req_id_buf);
        n = recv(sockfd, req_id_buf, sizeof req_id_buf, 0);

        for (int i = 0; i < n; i++)
        {
            if (req_id_buf[i] == '@')
                req_id_buf[i + 1] = '\0';
        }
        checkTimeoutError(n);
        FILE *fd;

        char time[200];
        bzero(time, sizeof time);
        sprintf(time, "%s %ld\n", req_id_buf, start_time.tv_sec);

        int c = 0;
        for (int i = 0; i < sizeof time; i++)
        {
            if (time[i] == '\n')
                break;
            c++;
        }

        fd = fopen("request_id.txt", "a+");
        if (fd < 0)
        {
            printf("E9: Error Opening File - request_id.txt");
        }
        if (fwrite(time, 1, c + 1, fd) == -1)
        {
            printf("E10: Error writing to File\n");
        }
        fclose(fd);

        close(sockfd);
    }
    else if (!strcmp(request_type, "status"))
    {
        char *request_id = argv[3];
        int res_size = 0; // size of the response
        char res_type;
        char responseSize[8];
        char queuepose[4];
        int bytes_read = 0;
        int file_size = 0;
        while (1)
        {
            int count = 0;
            while (count < 10 && connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            {
                count++;
            }
            if (count == 10)
            {
                printf("E12: Connection Timeout\n");
                exit(1);
            }

            // Sending request ID
            n = send(sockfd, request_id, strlen(request_id), 0);
            if (n < 0)
                printf("E5: ERROR writing to socket\n");

            n = recv(sockfd, &res_type, 1, 0);
            checkTimeoutError(n);

            if (res_type == 'a')
            {
                printf("Your grading request ID %s has been accepted and is currently being processed.", request_id);
            }

            // Request is in the queue
            else if (res_type == 'b')
            {
                n = recv(sockfd, queuepose, sizeof queuepose, 0);
                checkTimeoutError(n);
                printf("Your grading request ID %s has been accepted. It is currently at position %s in the queue.", request_id, queuepose);
            }

            // Request is processed
            else if (res_type == 'c')
            {
                gettimeofday(&end_time, NULL);
                char update_time_command[50];
                sprintf(update_time_command, "sed -i '/%s/ s/$/ %ld/' request_id.txt", request_id, end_time.tv_sec);
                system(update_time_command);
                n = recv(sockfd, responseSize, sizeof responseSize, 0);
                checkTimeoutError(n);

                file_size = atoi(responseSize);
                printf("Your grading request ID %s processing is done, here are the results:", request_id);
                while (bytes_read < file_size)
                {
                    n = recv(sockfd, buffer, sizeof buffer, 0);
                    printf("%s", buffer);
                    bzero(buffer, sizeof buffer);
                    bytes_read += n;
                }

                if (bytes_read < 0)
                {
                    if (errno == EWOULDBLOCK || errno == EAGAIN)
                    {
                        printf("E14: Timeout\n");
                        // exit(1);
                    }
                }
                else
                {
                    exit(0);
                }
            }
            else
            {
                printf("Grading request %s not found. Please check and resend your request ID or re-send your original grading request.", request_id);
                exit(1);
            }

            sleep(5);
        }
    }
    else
    {
        printf("E13: Invalid Request Type\n");
        exit(1);
    }

    return 0;
}

