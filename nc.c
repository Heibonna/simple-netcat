/* Mateusz Murawski 2020*/
/* Simple netcat utility implementation*/

#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>

void udp_client(char* hostname, int port){
    int sock;
    struct sockaddr_in server_addr;
    struct hostent *host;
    char send_data[1024];

    host= (struct hostent *) gethostbyname(hostname);


    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero),8);

    printf("Ready. Q or q to exit.\n");
    while(fgets(send_data, 100, stdin), !feof(stdin))
    {
        if ((strcmp(send_data , "q") == 0) || strcmp(send_data , "Q") == 0
          || strcmp(send_data , "q\n") == 0 || strcmp(send_data , "Q\n") == 0)
            break;
        else
            sendto(sock, send_data, strlen(send_data), 0,
                    (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

    }
    exit(0);
}

void udp_server(int port){
    int sock;
    int addr_len, bytes_read;
    char recv_data[1024];
    struct sockaddr_in server_addr , client_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);

    if (bind(sock,(struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1)
    {
        perror("Bind");
        exit(1);
    }
    addr_len = sizeof(struct sockaddr);
    printf("UDPServer Waiting for client on port %d\n", port);
    fflush(stdout);

    while (1)
    {
        bytes_read = recvfrom(sock,recv_data,1024,0,
                (struct sockaddr *)&client_addr, (socklen_t *) &addr_len);
        recv_data[bytes_read] = '\0';
        printf("(%s, %d): ",inet_ntoa(client_addr.sin_addr),
                ntohs(client_addr.sin_port));
        printf("%s", recv_data);
        fflush(stdout);

    }
return;
}

void udp6_client(char* hostname, int port){
    int sock;
    struct sockaddr_in6 server_addr;
    char send_data[1024];

    if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket");
        exit(1);
    }

    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    inet_pton(AF_INET6, hostname, &server_addr.sin6_addr);


    printf("Ready. Q or q to exit.\n");
    while(fgets(send_data, 100, stdin), !feof(stdin))
    {
        if ((strcmp(send_data , "q") == 0) || strcmp(send_data , "Q") == 0
          || strcmp(send_data , "q\n") == 0 || strcmp(send_data , "Q\n") == 0)
            break;
        else
            sendto(sock, send_data, strlen(send_data), 0,
                    (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

    }
    exit(0);
}

void udp6_server(int port){
    int sock;
    int addr_len, bytes_read;
    char recv_data[1024];
    char str_addr[INET6_ADDRSTRLEN];
    struct sockaddr_in6 server_addr , client_addr;

    if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    server_addr.sin6_addr = in6addr_any;

    if (bind(sock,(struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1)
    {
        perror("Bind");
        exit(1);
    }
    addr_len = sizeof(struct sockaddr);
    printf("UDPServer Waiting for client on port %d\n", port);
    fflush(stdout);

    while (1)
    {
        bytes_read = recvfrom(sock,recv_data,1024,0,
                (struct sockaddr *)&client_addr, (socklen_t *) &addr_len);
        recv_data[bytes_read] = '\0';
        printf("(%s, %d): ",str_addr, ntohs(client_addr.sin6_port));
        printf("%s", recv_data);
        fflush(stdout);

    }
return;
}

void tcp_client(char* hostname, int port){
    int sock, bytes_recieved, pid;
    char send_data[1024], recv_data[1024];
    struct hostent *host;
    struct sockaddr_in server_addr;
    int ppid = getpid();
    host = gethostbyname(hostname);

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    bzero(&(server_addr.sin_zero), 8);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(struct sockaddr)) == -1) {
        perror("Connect");
        exit(1);
    }

    if((pid=fork()) == 0){
        while(fgets(send_data, 1024, stdin)){
          if (strcmp(send_data, "q") != 0 && strcmp(send_data, "Q") != 0
            && strcmp(send_data, "Q\n") != 0 && strcmp(send_data, "q\n") != 0){
              send(sock, send_data, strlen(send_data), 0);
          }
          else {
              send(sock, send_data, strlen(send_data), 0);
              close(sock);
              printf("Closing connection...\n");
              kill(ppid, SIGKILL);
              exit(0);
          }
        }
    }
    else if(pid > 0){
        while((bytes_recieved = recv(sock, recv_data, 1024, 0))){
            recv_data[bytes_recieved] = '\0';

            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0
              || strcmp(recv_data, "Q\n") == 0 || strcmp(recv_data, "q\n") == 0) {
                close(sock);
                printf("Closing connection...\n");
                kill(pid, SIGKILL);
                exit(0);
            }
            else
                printf("%s", recv_data);
        }

        fflush(stdout);
    }
    else{
      perror("fork_server");
      exit(1);
    }

    return;
}

void tcp_server(int port){
    int sock, connected, bytes_recieved , pid, true = 1;
    char send_data [1024] , recv_data[1024];
    struct sockaddr_in server_addr, client_addr;
    int sin_size;
    int ppid = getpid();

    if ((sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Socket");
        exit(1);
    }
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(server_addr.sin_zero),8);
    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1) {
        perror("Unable to bind");
        exit(1);
    }
    if (listen(sock, 10) == -1) {
        perror("Listen");
        exit(1);
    }

    printf("TCPServer Waiting for client on port %d", port);
    fflush(stdout);

    while(1){
        ppid = getpid();
        sin_size = sizeof(struct sockaddr_in);
        connected = accept(sock, (struct sockaddr *) &client_addr,(socklen_t *)  &sin_size);
        if (connected == -1) {
      			perror("Accept");
      			exit(1);
    		}

        printf("\nI got a connection from (%s , %d). q or Q to exit\n",
                inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port));

        if((pid=fork()) == 0){
          while(fgets(send_data, 1024, stdin)){
              if (strcmp(send_data, "q") != 0 && strcmp(send_data, "Q") != 0
                && strcmp(send_data, "Q\n") != 0 && strcmp(send_data, "q\n") != 0){
                  send(connected, send_data, strlen(send_data), 0);
                }
              else {
                  printf("Closing connection...\n");
                  send(connected, send_data, strlen(send_data), 0);
                  close(connected);
                  kill(ppid, SIGKILL);
                  exit(0);
              }
          }
        }
        else if(pid > 0){
          while(1){
            bytes_recieved = recv(connected,recv_data,1024,0);
            recv_data[bytes_recieved] = '\0';

            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0
              || strcmp(recv_data, "Q\n") == 0 || strcmp(recv_data, "q\n") == 0
                || strcmp(recv_data, "") == 0)
            {
                close(connected);
                printf("\nTCPServer Waiting for client on port %d\n", port);
                kill(pid, SIGKILL);
                break;
            }
            else{
                printf("%s" , recv_data);
                fflush(stdout);
            }
          }
        }
        else{
          perror("fork_server");
          exit(1);
        }
    }
    close(sock);
    return;
}

void tcp6_client(char* hostname, int port){
    int sock, bytes_recieved, pid;
    char send_data[1024], recv_data[1024];
    struct sockaddr_in6 server_addr;
    int ppid = getpid();

    if ((sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_TCP)) == -1) {
        perror("Socket");
        exit(1);
    }
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    inet_pton(AF_INET6, hostname, &server_addr.sin6_addr);

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) == -1) {
        perror("Connect");
        exit(1);
    }

    if((pid=fork()) == 0){
        while(fgets(send_data, 1024, stdin)){
          if (strcmp(send_data, "q") != 0 && strcmp(send_data, "Q") != 0
            && strcmp(send_data, "Q\n") != 0 && strcmp(send_data, "q\n") != 0){
              send(sock, send_data, strlen(send_data), 0);
          }
          else {
              send(sock, send_data, strlen(send_data), 0);
              close(sock);
              printf("Closing connection...\n");
              kill(ppid, SIGKILL);
              exit(0);
          }
        }
    }
    else if(pid > 0){
        while((bytes_recieved = recv(sock, recv_data, 1024, 0))){
            recv_data[bytes_recieved] = '\0';

            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0
              || strcmp(recv_data, "Q\n") == 0 || strcmp(recv_data, "q\n") == 0) {
                close(sock);
                printf("Closing connection...\n");
                kill(pid, SIGKILL);
                exit(0);
            }
            else
                printf("%s", recv_data);
        }

        fflush(stdout);
    }
    else{
      perror("fork_server");
      exit(1);
    }

    return;
}

void tcp6_server(int port){
    int sock, connected, bytes_recieved , pid, true = 1;
    char send_data [1024] , recv_data[1024];
    char str_addr[INET6_ADDRSTRLEN];
    struct sockaddr_in6 server_addr, client_addr;
    int sin_size;
    int ppid = getpid();

    if ((sock = socket(AF_INET6, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        exit(1);
    }
    if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&true,sizeof(int)) == -1) {
        perror("Setsockopt");
        exit(1);
    }
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    server_addr.sin6_addr = in6addr_any;

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
        perror("Unable to bind");
        exit(1);
    }
    if (listen(sock, 10) == -1) {
        perror("Listen");
        exit(1);
    }

    printf("TCPServer Waiting for client on port %d", port);
    fflush(stdout);

    while(1){
        ppid = getpid();
        sin_size = sizeof(struct sockaddr_in);
        connected = accept(sock, (struct sockaddr *) &client_addr,(socklen_t *)  &sin_size);
        if (connected == -1) {
      			perror("Accept");
      			exit(1);
    		}

        inet_ntop(AF_INET6, &(client_addr.sin6_addr),
    				str_addr, sizeof(str_addr));

        printf("\nI got a connection from (%s:%d). q or Q to exit\n",
                str_addr, ntohs(client_addr.sin6_port));

        if((pid=fork()) == 0){
          while(fgets(send_data, 1024, stdin)){
              if (strcmp(send_data, "q") != 0 && strcmp(send_data, "Q") != 0
                && strcmp(send_data, "Q\n") != 0 && strcmp(send_data, "q\n") != 0){
                  send(connected, send_data, strlen(send_data), 0);
                }
              else {
                  printf("Closing connection...\n");
                  send(connected, send_data, strlen(send_data), 0);
                  close(connected);
                  kill(ppid, SIGKILL);
                  exit(0);
              }
          }
        }
        else if(pid > 0){
          while(1){
            bytes_recieved = recv(connected,recv_data,1024,0);
            recv_data[bytes_recieved] = '\0';

            if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0
              || strcmp(recv_data, "Q\n") == 0 || strcmp(recv_data, "q\n") == 0
                || strcmp(recv_data, "") == 0)
            {
                close(connected);
                printf("\nTCPServer Waiting for client on port %d\n", port);
                kill(pid, SIGKILL);
                break;
            }
            else{
                printf("%s" , recv_data);
                fflush(stdout);
            }
          }
        }
        else{
          perror("fork_server");
          exit(1);
        }
    }
    close(sock);
    return;
}

int main(int argc, char** argv){
int u = 0, l = 0, v4 = 0, v6 = 0, i;
char *host, *port;

  if(argc <= 2 || argc > 5){
    fprintf(stderr, "Usage: nc [-l] [-u] [-4] [-6] [host] port\n");
    exit(1);
  }
  else{
    i=0;
    while(1){
      i++;
      if(argv[i][0] == '-' && argv[i][1] == 'l')
        l = 1;
      else if(argv[i][0] == '-' && argv[i][1] == 'u')
        u = 1;
      else if(argv[i][0] == '-' && argv[i][1] == '6')
        v6 = 1;
      else if(argv[i][0] == '-' && argv[i][1] == '4')
        v4 = 1;
      else{
        if(l==1){             //server
          port = argv[i];
          break;
        }
        else{                 //client
          host = argv[i];
          port = argv[i+1];
          break;
        }
      }
    }
  }
/*  printf("l=%d, u=%d, v6=%d, v4=%d, host=%s, port=%s\n", l, u, v6, v4, host, port);*/
  if(v4 && v6){
    printf("Choose IPv4 or IPv6\n");
    exit(0);
  }
  if(l == 1){       //server
    if(u == 1){
      if(v6 == 1){
        udp6_server(atoi(port));
      }
      else{
        udp_server(atoi(port));
      }
    }
    else{
      if(v6 == 1){
          tcp6_server(atoi(port));
      }
      else{
          tcp_server(atoi(port));
      }
    }
  }
  else{             //client
    if(u == 1){
      if(v6 == 1){
        udp6_client(host, atoi(port));
      }
      else{
          udp_client(host, atoi(port));
      }
    }
    else{
      if(v6 == 1){
        tcp6_client(host, atoi(port));
      }
      else{
        tcp_client(host, atoi(port));
      }
    }
  }

  return 0;
}
