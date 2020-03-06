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

#define CLIENTMAX 50
#define USERMAX 100

struct client{
    int user_id;
    int fd;
    int authen;
    char dir[200];
};

struct user{
    char user_name[40];
    char pass[40];
};

int main(int argc, char* argv[]) {
    

    size_t length = 0;
    FILE *filepointer;
    char * line;
    ssize_t read;

    filepointer = fopen("userpass.txt", "r"); // opens the file to read usernames and passwords
    if(filepointer == 0){ // if fails
        printf("Error opening the file \n");
        exit(1);
    }

    read = getline(&line, &length, filepointer);

    int numofusers = atoi(line);
    int index = 0;

    printf("Read %d from file \n", numofusers);
    struct user *user_array = malloc(sizeof(struct user)*numofusers);
    printf("Allocated user array\n");
    while((read = getline(&line, &length, filepointer)) != -1){
        int count = 0;
        char * str;
        str  = strtok(line, " ");

        while(str != NULL){
            if(count == 0){
                strcpy(user_array[index].user_name, str);
            }
            else{
                strcpy(user_array[index].pass, str);
            }
            count += 1;
            str = strtok(NULL, " ");
        }
        index = index + 1;
    }
    fclose(filepointer);//close the file after reading
    printf("Finished reading user file \n");
    struct sockaddr_in clientAddr, serverAddr;
    int acc_socket, m_socket, c_socket;
    char buff[1024];
    fd_set read_fd_set;
    int max_fd;

    int port_begin = 9900;
    int port =9999;

    struct client* client_array = (struct client*)malloc(sizeof(struct client)* 100);
    int i;
    for(i = 0; i < CLIENTMAX; i++){
        client_array[i].authen = -1; // not authenticated
        client_array[i].fd = -1;
        getcwd(client_array[i].dir, 200);
        client_array[i].user_id = -1;
    }

    printf("Created client array \n");
    m_socket = socket(AF_INET, SOCK_STREAM, 0);
    if(m_socket < 0){
        printf("cannot open the socket \n");
        exit(1);
    }

    int util = 1;
    setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &util, sizeof(int));

    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if(bind(m_socket, (struct sockaddr *) &serverAddr, sizeof(serverAddr))){
        printf("Cannot bind the socket \n");
        close(m_socket);
        exit(1);
    }

    if(listen(m_socket, 5), 0){
        printf("Cannot listen on the socket\n");
        close(m_socket);
        exit(1);
    }

    printf("Bound and listening\n");
    while(1){
        // clear the socket set
        FD_ZERO(&read_fd_set);
        
        // add m_socket to file discriptor set
        FD_SET(m_socket, &read_fd_set);
        
        max_fd = m_socket;

        for(i= 0; i < CLIENTMAX; i++){
            //fetching file descriptor
            c_socket = client_array[i].fd;
            //check if it's valid
            if(c_socket > 0){
                FD_SET(c_socket, &read_fd_set);
            }
            if(c_socket > max_fd){
                max_fd = c_socket; //update max_fd
            }
        }
        //wait for activity on fiel descriptor set
        select(max_fd + 1, &read_fd_set, NULL, NULL, NULL);
        // check for activity on m_socket -> incoming request
        if(FD_ISSET(m_socket, &read_fd_set)){
            socklen_t length = sizeof(clientAddr);
            if((acc_socket  = accept(m_socket, (struct sockaddr *)(&clientAddr), &length)) < 0){
                printf("Cannot accept connection\n");
                exit(1);
            }
            printf("Accepted connection from %s:%d\n", inet_ntoa(clientAddr.sin_addr), clientAddr.sin_port);
            //add new socket to client_array

            for(i = 0; i < CLIENTMAX; i++){
                if(client_array[i].fd < 0){
                    printf("[%d]Adding a client to the list of sockets with socket #\n", i);
                    client_array[i].fd = acc_socket;
                    break;
                }
            }
            if(i == CLIENTMAX){ // if full
                 printf("Too many connections\n");
                close(acc_socket);
            }
        }
        for(i = 0; i < CLIENTMAX; i++){
            if(client_array[i].fd < 0) continue;
            if (FD_ISSET(client_array[i].fd, &read_fd_set)){
                memset(buff, 0, sizeof(buff)); //set buffer to zeros
                int n = recv(client_array[i].fd, buff, 1024, 0);
                if(0 == n){
                    printf("[%d]Closing connection for a client\n", i);
                    close(client_array[i].fd);
                    FD_CLR(client_array[i].fd, &read_fd_set); 
                    client_array[i].fd = -1;
                }
                else{
                    printf("[%d]Received:  %s\n", i, buff);
                    char * str;
                    int count = 0;
                    char command[20];
                    char arg[100];
                    str = strtok(buff, " ");
                    while(str != NULL){
                        if(0 == count){
                            strcpy(command, str);
                        }
                        else{
                            strcpy(arg, str);
                        }
                        count = count +1;
                        str = strtok(NULL, " ");
                    }   
                        if(strcmp(command, "USER") == 0){
                            int j = 0;
                            int loc = -1;
                            for(j = 0; j < numofusers; j++){
                                //printf("Arg is %s\n", arg);
                                if(strcmp(user_array[j].user_name, arg) == 0){
                                    loc = j;
                                    
                                    break;
                                }
                            }
                            //printf("%d test \n", loc);
                            if(loc != -1){
                                client_array[i].user_id = loc;
                                char msg[] = "Username OK, password required";
                                printf("Username OK, password required\n");
                                write(client_array[i].fd,msg,strlen(msg)+1);
                                continue; 
                            }
                            else{//user does not exist
                                char msg[] = "Username does not exist";
                                write(client_array[i].fd,msg,strlen(msg)+1);
                                printf("Username does not exist\n");
                                continue;
                            }
                        }
                        else if(strcmp(command, "PASS") == 0){
                            if(client_array[i].user_id == -1){//when user is not set yet
                                char msg[] = "set USER first";
                                printf("set USER first\n");
                                write(client_array[i].fd,msg,strlen(msg)+1);
                                continue;
                            }
                            else if(strncmp(user_array[client_array[i].user_id].pass, arg, strlen(arg)) == 0){
                                    char msg[] = "Authentication complete";
                                    printf("Authentication complete\n");
                                    client_array[i].authen = 1;
                                    write(client_array[i].fd,msg,strlen(msg)+1);
                                    continue;
                                    }
                            else{
                                char msg[] = "wrong password";
                                printf("wrong password\n");
                                write(client_array[i].fd,msg,strlen(msg)+1);//sending msg
                                continue;
                                    }

                        }
                        else if(strcmp(command, "QUIT") == 0){
                            close(client_array[i].fd);
                            printf("[%d]Closing connection for a client\n", i);
                            FD_CLR(client_array[i].fd, &read_fd_set); // clear the file descriptor 
                            client_array[i].fd = -1;
                            //client_array[i].authen = -1;
                            
                        }

                    

                    
                }

            }
        }

    }



    return 0;
}

