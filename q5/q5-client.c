#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080

int main() {
  int sock = 0;
  struct sockaddr_in serv_addr;
  char send_buf[1024];
  char recv_buf[1024];
  int n;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Socket creation error\n");
    return -1;
  }

  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(PORT);

  if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
    printf("Invalid address / Address not supported\n");
    return -1;
  }

  if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
    printf("Connection Failed\n");
    return -1;
  }

  printf("Enter Library ID: ");
  fflush(stdout);
  fgets(send_buf, sizeof(send_buf), stdin);
  send_buf[strcspn(send_buf, "\n")] = 0;
  send(sock, send_buf, strlen(send_buf), 0);

  n = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
  if (n <= 0) {
    printf("Server closed connection\n");
    close(sock);
    return -1;
  }
  recv_buf[n] = '\0';
  printf("%s", recv_buf);

  if (strstr(recv_buf, "Failed")) {
    close(sock);
    return 0;
  }

  while (1) {
    // Receive book list and prompt in one message
    n = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
    if (n <= 0)
      break;
    recv_buf[n] = '\0';
    printf("%s", recv_buf);
    fflush(stdout);

    // Send user input
    fgets(send_buf, sizeof(send_buf), stdin);
    send_buf[strcspn(send_buf, "\n")] = 0;
    send(sock, send_buf, strlen(send_buf), 0);

    if (strcmp(send_buf, "0") == 0)
      break;

    // Receive reservation result
    n = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
    if (n <= 0)
      break;
    recv_buf[n] = '\0';
    printf("%s", recv_buf);
  }

  // Receive goodbye message
  n = recv(sock, recv_buf, sizeof(recv_buf) - 1, 0);
  if (n > 0) {
    recv_buf[n] = '\0';
    printf("%s", recv_buf);
  }

  close(sock);
  return 0;
}
