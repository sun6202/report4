#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <zconf.h>

#define PORT 3918
#define QLEN 10
#define BUF_SIZE 1024

void *handle_clnt(int csock);
void send_err(int csock);
void send_msg_get(int client_sock);
void send_msg_post(int client_sock);

void *handle_clnt(int csock){
    int recv = 0, str_len = 0;
    char msg[BUF_SIZE];
    char method[10];

    if((str_len = read(csock, &msg[recv], BUF_SIZE)) == -1){
        printf("read() error!\n");
        exit(1);
    }
    recv += str_len;
    strcpy(method, strtok(msg, " "));
    if(strcmp(method, "GET") != 0){
        send_err(csock);
    }else if(strcmp(method, "GET") == 0){
        send_msg_get(csock);
    }
    else if(strcmp(method, "POST") == 0){
        send_msg_post(csock);
    }else {
        send_err(csock);
    }
    return NULL;
}

void send_err(int csock){
    char protocol[] = "HTTP/1.1 400 Bad Request\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char contenttype[] = "Content-Type:text/html\r\n";
    char html[] = "<html><head>BADConnection</head><body><H1>BadRequest</H1></body></html>\r\n";
    char end[]="\r\n";

    printf("send err\n");
    write(csock, protocol, strlen(protocol));
    write(csock, server, strlen(server));
    write(csock, contenttype, strlen(contenttype));
    write(csock, end, strlen(end));
    write(csock, html, strlen(html));
    fflush(fdopen(csock, "w"));
}

void send_msg_get(int client_sock) // send to all
{
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char contenttype[] = "Content-Type:text/html\r\n";
    char html[] = "<html><head>get test</head><body><H1>This is System test</H1></body></html>\r\n";
    char contentlength[] = "Content-Length: ";
    sprintf(contentlength, "%d", strlen(html));
    strcat(contentlength, "\r\n");
    char end[] = "\r\n";

    printf("send msg, len=%d\n", strlen(html));
    write(client_sock, protocol, strlen(protocol));
    write(client_sock, server, strlen(server));
    write(client_sock, contentlength, strlen(contentlength));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
    write(client_sock, html, strlen(html));
    fflush(fdopen(client_sock, "w"));
}

void send_msg_post(int client_sock){
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char contenttype[] = "Content-Type:text/html\r\n";
    char html[] = "<html><head>post test</head><body><H1>POST DONGDONG</H1></body></html>\r\n";
    char contentlength[] = "Content-Length: ";
    sprintf(contentlength, "%d", strlen(html));
    strcat(contentlength, "\r\n");
    char end[] = "\r\n";

    printf("send msg, len=%d\n", strlen(html));
    write(client_sock, protocol, strlen(protocol));
    write(client_sock, server, strlen(server));
    write(client_sock, contentlength, strlen(contentlength));
    write(client_sock, contenttype, strlen(contenttype));
    write(client_sock, end, strlen(end));
    write(client_sock, html, strlen(html));
    fflush(fdopen(client_sock, "w"));
}

int main(int argc, char* argv[]){
    int sockfd, new_fd;
    struct sockaddr_in server_addr;
    struct sockaddr_in client_addr;

    int alen;
    fd_set readfds, activefds;

    int i, maxfd = 0, numbytes;
    char buf[BUF_SIZE];

    if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket() failed");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind() failed");
        exit(1);
    }

    if(listen(sockfd, QLEN) < 0){
        fprintf(stderr, "listen failed\n");
        exit(1);
    }

    alen = sizeof(client_addr);
    FD_ZERO(&activefds);
    FD_SET(sockfd, &activefds);
    maxfd = sockfd;

    fprintf(stderr, "Server up and running\n");
    while(1){
        //printf("SERVER: Waiting for contact...%d\n", maxfd);
        readfds = activefds;

        if(select(maxfd + 1, &readfds, NULL, NULL, NULL) < 0){
            perror("select");
            exit(EXIT_FAILURE);
        }

        for(i = 0; i <= maxfd; i++){
            if(FD_ISSET(i, &readfds)){

                if(i == sockfd){
                    if((new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &alen)) < 0){
                        fprintf(stderr, "accept failed\n");
                        exit(1);
                    }
                    FD_SET(new_fd, &activefds);
                    if(new_fd > maxfd){
                        maxfd = new_fd;
                    }
                }
                else{
                    printf("handle clnt\n");
                    handle_clnt(i);
                    close(i);
                    FD_CLR(i, &activefds);
                }
            }
        }
    }
    close(sockfd);
    return 0;
}
