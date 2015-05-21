#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <signal.h>

#define DIE(msg) {perror(msg); exit(1);}


void com(int src, int dst) {
  char buf[1024*4];
  int r,i,j;
  while(1) {
    r = read(src, buf, 1024*4);
    if(r == 0) break;
    if(r == -1) DIE("read");
    i=0;
    while(i < r) {
      j = write(dst, buf+i, r-i);
      if(j == -1) DIE("write"); //TODO is errno EPIPE
      i+=j;
    }
  }
  shutdown(src, SHUT_RD);
  shutdown(dst, SHUT_WR);
  close(src);
  close(dst);
  exit(0);
}

int main(int argc, char **argv)
{
  int server_socket, client_socket, forward_socket;
  struct sockaddr_in server_address, forward_address;
  struct hostent * forward;

  int server_port, forward_port;
  pid_t up, down;

  signal(SIGCHLD,  SIG_IGN);

  if(argc < 4) {fprintf(stderr,"Not enough arguments\n"); exit(1);}
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if(server_socket == -1) DIE("socket");
  bzero((char *) &server_address, sizeof(server_address));
  server_port = atoi(argv[1]);

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(server_port);
  if(bind(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) DIE("bind");
  if(listen(server_socket,40) == -1) DIE("listen");

  forward_port = atoi(argv[3]);
  forward = gethostbyname(argv[2]);
  if(forward == NULL) DIE("gethostbyname");
  bzero((char *)&forward_address, sizeof(forward_address));
  forward_address.sin_family = AF_INET;
  bcopy((char *)forward->h_addr, 
	(char *)&forward_address.sin_addr.s_addr,
	forward->h_length);
  forward_address.sin_port = htons(forward_port);
  
  while(1) {
    client_socket = accept(server_socket, NULL, NULL);
    if(client_socket == -1) DIE("accept");
    up = fork();
    if(up == -1) DIE("fork");
    if(up == 0) {
      forward_socket = socket(AF_INET, SOCK_STREAM, 0);
      if(forward_socket == -1) DIE("socket");
      if(connect(forward_socket, (struct sockaddr *)&forward_address, sizeof(forward_address)) == -1) DIE("connect");
      down = fork();
      if(down == -1) DIE("fork");
      if(down == 0) com(forward_socket, client_socket);
      else com(client_socket, forward_socket);
      exit(1);
    }
    close(client_socket);
  }
  return 0; 
}
