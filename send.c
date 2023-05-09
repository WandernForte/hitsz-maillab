#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <getopt.h>
#include "base64_utils.h"

#define MAX_SIZE 4095
typedef int sockid;
char buf[MAX_SIZE+1];
void custom_send(int __fd, void * __buf, size_t __len, int __flags, int need_out){
    /**
     * @param: __fd socket_fd
     * @param: __buf buffer
     * @param: __len size of buffer sent
     * @param: __flags sendflag
     * @param: need_out whether print out or not 
    */
    if(send(__fd, __buf, __len, 0)==-1)
        exit(EXIT_FAILURE);
    if(need_out==1)
        printf("%s\r\n", (char*)buf);
    
}
void custom_recv(int __fd, void * __buf, size_t __len, int flags, int need_out){
    /**
     * @brief  
     * @note   
     * @retval None
     */
    int recv_size = -1;
    if((recv_size=recv(__fd, __buf, __len, 0)) == -1)
        exit(EXIT_FAILURE);
    if(need_out){
        char *tbuf = (char *)__buf;
        tbuf[recv_size] = '\0';
        printf("%s", tbuf);
        }
}
// receiver: mail address of the recipient
// subject: mail subject
// msg: content of mail body or path to the file containing mail body
// att_path: path to the attachment
void send_mail(const char* receiver, const char* subject, const char* msg, const char* att_path)
{
    const char* end_msg = "\r\n.\r\n";
    const char* host_name = "demo@hitsz-lab.com"; // TODO: Specify the mail server domain name
    const unsigned short port = 25; // SMTP server port
    const char* user = "sento"; // TODO: Specify the user
    const char* pass = "114514"; // TODO: Specify the password
    const char* from = "sento@hitsz-lab.com"; // TODO: Specify the mail address of the sender
    char dest_ip[16]; // Mail server IP address
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
    //===========================================================================
    // TODO: Create a socket, return the file descriptor to s_fd, and establish a TCP connection to the mail server
    s_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    if(-1==s_fd) exit(EXIT_FAILURE); // fail to create socket, exit failure
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(dest_ip);
    servaddr.sin_port = htons(port);
    socklen_t addrlen = sizeof(servaddr);
    bzero(&servaddr, addrlen);
    if(!connect(s_fd, (struct sockaddr*)&servaddr, addrlen)) exit(EXIT_FAILURE); // fail to create a TCP connection, exit failure
    // ==========================================================================
    // Print welcome message
    if ((r_size = recv(s_fd, buf, MAX_SIZE, 0)) == -1)
    {
        perror("recv");
        exit(EXIT_FAILURE);
    }
    buf[r_size] = '\0'; // Do not forget the null terminator
    printf("%s", buf);

    // Send EHLO command and print server response
    const char* EHLO = "EHLO @hitsz-lab.com\r\n"; // TODO: Enter EHLO command here
    send(s_fd, EHLO, strlen(EHLO), 0);
    // TODO: Print server response to EHLO command
    recv(s_fd, buf, MAX_SIZE, 0);
    printf("recv: %s", buf);
    // TODO: Authentication. Server response should be printed out.
    const char* AUTH = "AUTH login";
    send(s_fd, AUTH, strlen(AUTH), 0);
    recv(s_fd, buf, MAX_SIZE, 0);
    printf("recv Authentication: %s", buf);
    // TODO: Send MAIL FROM command and print server response
    // const char* MAIL = "MAIL ";
    // strcat(MAIL, from);
    sprintf(buf, "MAIL FROM:<%s>\r\n", from);// 用sprintf可以同时写buf和打印输出
    custom_send(s_fd, buf, strlen(buf), 0, 1);
    custom_recv(s_fd, buf, MAX_SIZE, 0, 1);
    // printf("%s\r\n", buf);
    // TODO: Send RCPT TO command and print server response
    sprintf(buf, "RCPT TO:<%s>\r\n", receiver);
    custom_send(s_fd, buf, strlen(buf), 0, 1);
    custom_recv(s_fd, buf, MAX_SIZE, 0, 1);
    // printf("%s\r\n", buf);
    // TODO: Send DATA command and print server response
    
    // TODO: Send message data
    
    // TODO: Message ends with a single period

    // TODO: Send QUIT command and print server response

    close(s_fd);
}

int main(int argc, char* argv[])
{
    int opt;
    char* s_arg = NULL;
    char* m_arg = NULL;
    char* a_arg = NULL;
    char* recipient = NULL;
    const char* optstring = ":s:m:a:";
    while ((opt = getopt(argc, argv, optstring)) != -1)
    {
        switch (opt)
        {
        case 's':
            s_arg = optarg;
            break;
        case 'm':
            m_arg = optarg;
            break;
        case 'a':
            a_arg = optarg;
            break;
        case ':':
            fprintf(stderr, "Option %c needs an argument.\n", optopt);
            exit(EXIT_FAILURE);
        case '?':
            fprintf(stderr, "Unknown option: %c.\n", optopt);
            exit(EXIT_FAILURE);
        default:
            fprintf(stderr, "Unknown error.\n");
            exit(EXIT_FAILURE);
        }
    }

    if (optind == argc)
    {
        fprintf(stderr, "Recipient not specified.\n");
        exit(EXIT_FAILURE);
    }
    else if (optind < argc - 1)
    {
        fprintf(stderr, "Too many arguments.\n");
        exit(EXIT_FAILURE);
    }
    else
    {
        recipient = argv[optind];
        send_mail(recipient, s_arg, m_arg, a_arg);
        exit(0);
    }
}