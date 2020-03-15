#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>



int main(int argc, char* argv[]) {
    int server_port;
    char server_addr[100];

    if(argc != 3){
        printf("Some arguments are missing, enter server address  and port number\n");
        return 1;
    }
    else{
        server_port = atoi(argv[2]);
        strcpy(server_addr, argv[1]);
        printf("Server's IP address: %s\n", server_addr);
        printf("Server's port number: %d\n", server_port);
    }
    char buf[1024];// to send and receive messages
    int sockfd;//Communication socket
    int sockfdt;//Transport socket
    struct sockaddr_in address;
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Cannot open socket \n");
        exit(1);
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(server_port);
    inet_aton(server_addr, &(address.sin_addr));
    if (connect(sockfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        printf("Connection failed\n");
        exit(1);
    }
    printf("Connected to the server\n");
    while(1){
        char *arg;
        char *cmd;
        while(1){
            printf("ftp> ");
            char in[1024];
            fgets(buf,1024, stdin);
            strcpy(in, buf);
            cmd = strtok(in, "\n");
            if(NULL == cmd){
                continue;
            }
            cmd = strtok(cmd, " ");
            if(NULL == cmd){
                continue;
            }
            break;
        }
        
        
        if((strcmp(cmd, "USER")) == 0){
            write(sockfd, buf, strlen(1+buf));
            memset(buf, 0, strlen(buf));
            if (read(sockfd, buf, 1024) == 0) {
                printf("Connection closed by server\n");
                exit(0);
            }
            if((strcmp(buf, "Username does not exist")) == 0){
                printf("Username does not exist\n");
            }
            else if((strcmp(buf, "Username OK, password required")) == 0){
                printf("Username OK, password required\n");
            }
            else{
                printf("Incorrect response from server\n");
            }
        }
        else if((strcmp(cmd, "PASS")) == 0){
            write(sockfd, buf, strlen(1+buf));
            memset(buf, 0, strlen(buf));
            if (read(sockfd, buf, 1024) == 0) {
                printf("Connection closed by server\n");
                exit(0);
            }
            if((strcmp(buf, "wrong password")) == 0){
                printf("wrong password\n");
            }
            else if((strcmp(buf, "Authentication complete")) == 0){
                printf("Authentication complete\n");
            }
            else if((strcmp(buf, "set USER first")) == 0){
                printf("set USER first\n");
            }
            else{
                printf("Incorrect response from server\n");
            }
        }
        else if ((strcmp(cmd, "PUT")) == 0) {
            arg = strtok(NULL, " ");//name of the file to be send
            if(NULL == arg){
                printf("Please enter PUT fileName");
                continue;
            } 
            FILE* ptr = fopen(arg, "r");// open the file with name arg
            if(ptr == NULL){ // if fails
                printf("Error opening the file \n");
                continue;
            }
            char* aux = (char*)malloc(1024);
            strcpy(aux, buf);
            char *response;
            memset(buf, 0, strlen(buf));//put zeros in buf
            write(sockfd, aux, strlen(1+aux));// send the command to the server
            free(aux);
            if (read(sockfd, buf, 1024) == 0) {
                printf("Connection closed by server\n");
                exit(0);
            }
            response = strtok(buf, "\n");
            if(NULL == response){
                printf("Corrupted response from server\n");
                continue;
            }
            else if(strcmp(response, "Autentication required") == 0){
                printf("Autentication required\n");
                continue;
            }
            else{
                response = strtok(response, " ");
                if(strcmp(response, "Ready") != 0){
                    printf("Server is not ready\n");
                    continue;
                }
                else if(strcmp(response,"Ready") == 0){
                    response = strtok(NULL, " ");
                    if(response == NULL){
                        printf("Port is missing\n");
                        continue;
                    }
                }
            }
            //once server is ready and port number is provided ...
            if((sockfdt = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                printf("Cannot open socket\n");
                exit(0);
            }
            address.sin_port = htons(atoi(response));
            size_t length = 0;
            char *line = (char*)malloc(1024);
            int readln = 0;
            if (connect(sockfdt, (struct sockaddr *)&address, sizeof(address)) < 0) {
                printf("Can't connect to port\n");
                continue;
            }
            while((readln = getline(&line, &length, ptr)) != -1){
                //printf("%s\n", line); 
                write(sockfdt, line, readln);
            }
            fclose(ptr);
            close(sockfdt);
            free(line);



            //open file
            //read it and send it to server
        }



        else if ((strcmp(cmd, "GET")) == 0) {
            arg = strtok(NULL, " ");//name of the file to be send
            if(NULL == arg){
                printf("Please enter GET fileName");
                continue;
            } 
            write(sockfd, buf, strlen(buf+1));
            memset(buf, 0, strlen(buf));
            if (read(sockfd, buf, 1024) == 0) {
                printf("Server closed connection\n");
                exit(0);
            }
            char* response;
            response = strtok(buf, "\n");
            if(NULL == response){
                printf("Corrupted response from server\n");
                continue;
            }
            else if(strcmp(response, "Autentication required") == 0){
                printf("Autentication required\n");
                continue;
            }
            else{
                response = strtok(response, " ");
                if(strcmp(response, "Ready") != 0){
                    printf("Server is not ready\n");
                    continue;
                }
                else if(strcmp(response,"Ready") == 0){
                    response = strtok(NULL, " ");
                    if(response == NULL){
                        printf("Port is missing\n");
                        continue;
                    }
                }

            }
            if((sockfdt = socket(AF_INET, SOCK_STREAM, 0)) < 0){
                printf("Cannot open socket\n");
                exit(0);
            }
            address.sin_port = htons(atoi(response));
            size_t length = 0;
            char *line = (char*)malloc(1024);
            int readln = 0;
            if (connect(sockfdt, (struct sockaddr *)&address, sizeof(address)) < 0) {
                printf("Can't connect to port\n");
                continue;
            }
            int fp = open(arg,O_CREAT|O_WRONLY, 0666);
            if(fp == -1){
                printf("Error opening the file\n");
                continue;
            }
            int i = 0;
            do{
                readln = read(sockfdt, line, 1024);
                //printf("%s :%d\n", line, i++);// might print twice but gets correct file
                write(fp, line, readln);
                }while(readln != 0);
                close(fp);
                close(sockfdt);
                //free(line);




            //check if the fiile is there
        }







        else if(strcmp(cmd, "QUIT") == 0){
            printf("Shutting down...\n");

            if(close(sockfd) == 0){//quits the connection
            printf("Socket closed\n");
        } else {
            printf("Error while closing the socket\n");
        }
            return(1);
    }
  }



    return 0;
}

