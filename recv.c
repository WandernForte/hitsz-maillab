#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

#define MAX_SIZE 65535

char buf[MAX_SIZE+1];
// custom utils wrap
void custom_send(int __fd, void * __buf, size_t __len, int __flags, int need_out){
    /**
     * @param: __fd socket_fd
     * @param: __buf buffer
     * @param: __len size of buffer sent
     * @param: __flags sendflag
     * @param: need_out whether print out or not 
    */
    if(send(__fd, __buf, __len, 0)==-1)
    {
        perror(__buf);
        exit(EXIT_FAILURE);
        
        }
    if(need_out==1)
        printf("%s\r\n", (char*)__buf);
    
}
int custom_recv(int __fd, void * __buf, size_t __len, int flags, int need_out){
    /**
     * @brief  
     * @note   
     * @retval None
     */
    int recv_size = -1;
    if((recv_size=recv(__fd, __buf, __len, 0)) == -1){
        perror(__buf);
        exit(EXIT_FAILURE);
        }
    if(need_out){
        char *tbuf = (char *)__buf;
        tbuf[recv_size] = '\0';
        printf("%s\r\n", tbuf);
        }
    return recv_size;
}




void recv_mail()
{
    const char* host_name = "smtp.qq.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 110; // POP3 server port
    const char* user = "1648639935@qq.com"; // TODO: Specify the user
    const char* pass = "*********"; // TODO: Specify the password
    char dest_ip[16];
    int s_fd; // socket file descriptor
    struct hostent *host;
    struct in_addr **addr_list;
    int i = 0;
    int r_size;
    struct sockaddr_in servaddr;

    // Get IP from domain name
    if ((host = gethostbyname(host_name)) == NULL)
    {
        herror("gethostbyname");
        exit(EXIT_FAILURE);
    }

    addr_list = (struct in_addr **) host->h_addr_list;
    while (addr_list[i] != NULL)
        ++i;
    strcpy(dest_ip, inet_ntoa(*addr_list[i-1]));

    // TODO: Create a socket,return the file descriptor to s_fd, and establish a TCP connection to the POP3 server
        s_fd = socket(AF_INET, SOCK_STREAM, 0);
    // printf("ip: %s\n", dest_ip);
    // runned here
    if(-1==s_fd) {
        perror("socket");
        exit(EXIT_FAILURE);
        } // fail to create socket, exit failure
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(dest_ip);
    servaddr.sin_port = htons(port);
    socklen_t addrlen = sizeof(servaddr);
    bzero((&servaddr)->sin_zero, addrlen);
    if(connect(s_fd, (struct sockaddr*)&servaddr, addrlen)==-1){ 
        perror("connect");
        exit(EXIT_FAILURE);
        } // fail to create a TCP connection, exit failure
    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // TODO: Send user and password and print server response
    // we donnot need base64 here
    sprintf(buf, "USER %s\r\n", user); 
    // strcat(user64, "\r\n");
    custom_send(s_fd, (void *)buf, strlen(buf), 0, 1);
    custom_recv(s_fd, (void *)buf, MAX_SIZE, 0, 1);

    sprintf(buf, "PASS %s\r\n", pass);
    custom_send(s_fd, (void *)buf, strlen(buf), 0, 1);
    custom_recv(s_fd, (void *)buf, MAX_SIZE, 0, 1);
    // TODO: Send STAT command and print server response
    sprintf(buf, "STAT\r\n");
    custom_send(s_fd, (void *)buf, strlen(buf), 0, 1);
    custom_recv(s_fd, (void *)buf, MAX_SIZE, 0, 1);
    // TODO: Send LIST command and print server response
    sprintf(buf, "LIST\r\n");
    custom_send(s_fd, (void *)buf, strlen(buf), 0, 1);
    custom_recv(s_fd, (void *)buf, MAX_SIZE, 0, 1);
    // TODO: Retrieve the first mail and print its content
    custom_send(s_fd, "RETR 1\r\n", 8, 0, 1);
    r_size = custom_recv(s_fd, buf, MAX_SIZE, 0, 1);// get recv_size
    printf("%s\r\n",buf);
    int total_size = atoi(buf + 4);
    total_size -= r_size;
    while(total_size > 0){
        r_size = custom_recv(s_fd, buf, MAX_SIZE, 0, 1);
        total_size -= r_size;
    }
    // TODO: Send QUIT command and print server response
    sprintf(buf, "QUIT\r\n");
    custom_send(s_fd, (void *)buf, strlen(buf), 0, 1);
    custom_recv(s_fd, (void *)buf, MAX_SIZE, 0, 1);
    close(s_fd);
}

int main(int argc, char* argv[])
{
    recv_mail();
    exit(0);
}
