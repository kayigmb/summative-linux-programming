#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define PORT 8080
#define MAX_CLIENTS 5
#define MAX_BOOKS 5

typedef struct {
  int id;
  char name[50];
  int reserved; // 0 = available, 1 = reserved
} Book;

typedef struct {
  int sockfd;
  char library_id[20];
} Client;

// Books database
Book books[MAX_BOOKS] = {{1, "The C Programming Language", 0},
                         {2, "Clean Code", 0},
                         {3, "Introduction to Algorithms", 0},
                         {4, "Design Patterns", 0},
                         {5, "Effective Java", 0}};

// Valid users
char valid_users[][20] = {"U001", "U002", "U003", "U004", "U005"};
char active_users[MAX_CLIENTS][20];
int active_count = 0;

// Mutexes for thread safety
pthread_mutex_t books_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t users_mutex = PTHREAD_MUTEX_INITIALIZER;

// Authenticate user
int authenticate_user(const char *lib_id) {
  for (int i = 0; i < (int)(sizeof(valid_users) / sizeof(valid_users[0]));
       i++) {
    if (strcmp(valid_users[i], lib_id) == 0)
      return 1;
  }
  return 0;
}

// Build book list + prompt into a single buffer and send it atomically
void send_books_and_prompt(int sockfd) {
  pthread_mutex_lock(&books_mutex);
  char msg[1024] = "Available Books:\n";
  for (int i = 0; i < MAX_BOOKS; i++) {
    char line[100];
    snprintf(line, sizeof(line), "%d. %s [%s]\n", books[i].id, books[i].name,
             books[i].reserved ? "Reserved" : "Available");
    strcat(msg, line);
  }
  strncat(msg, "Enter book ID to reserve (0 to exit): ",
          sizeof(msg) - strlen(msg) - 1);
  pthread_mutex_unlock(&books_mutex);
  send(sockfd, msg, strlen(msg), 0);
}

// Handle a single client
void *handle_client(void *arg) {
  Client *client = (Client *)arg;
  int sockfd = client->sockfd;
  char buffer[1024];

  int n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
  if (n <= 0) {
    close(sockfd);
    free(client);
    return NULL;
  }
  buffer[n] = '\0';

  if (!authenticate_user(buffer)) {
    send(sockfd, "Authentication Failed\n", 22, 0);
    close(sockfd);
    free(client);
    return NULL;
  }

  strcpy(client->library_id, buffer);
  pthread_mutex_lock(&users_mutex);
  strcpy(active_users[active_count++], client->library_id);
  pthread_mutex_unlock(&users_mutex);

  send(sockfd, "Authentication Successful\n", 26, 0);

  while (1) {
    send_books_and_prompt(sockfd);

    n = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
    if (n <= 0)
      break;
    buffer[n] = '\0';

    int book_id = atoi(buffer);
    if (book_id == 0)
      break;

    int success = 0;
    pthread_mutex_lock(&books_mutex);
    if (book_id >= 1 && book_id <= MAX_BOOKS &&
        books[book_id - 1].reserved == 0) {
      books[book_id - 1].reserved = 1;
      success = 1;
    }
    pthread_mutex_unlock(&books_mutex);

    if (success)
      send(sockfd, "Book Reserved Successfully\n", 27, 0);
    else
      send(sockfd, "Book Already Reserved\n", 22, 0);
  }

  pthread_mutex_lock(&users_mutex);
  for (int i = 0; i < active_count; i++) {
    if (strcmp(active_users[i], client->library_id) == 0) {
      active_users[i][0] = '\0';
      active_count--;
      break;
    }
  }
  pthread_mutex_unlock(&users_mutex);

  send(sockfd, "Session closed. Goodbye!\n", 25, 0);
  close(sockfd);
  free(client);
  return NULL;
}

int main() {
  int server_fd, new_socket;
  struct sockaddr_in address;
  int addrlen = sizeof(address);

  // Create socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }

  // Allow port reuse
  int opt = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("setsockopt failed");
    exit(EXIT_FAILURE);
  }

  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(PORT);

  if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd, MAX_CLIENTS) < 0) {
    perror("listen failed");
    exit(EXIT_FAILURE);
  }

  printf("Server running on port %d...\n", PORT);

  while (1) {
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
                             (socklen_t *)&addrlen)) < 0) {
      perror("accept");
      continue;
    }

    Client *client = malloc(sizeof(Client));
    client->sockfd = new_socket;

    pthread_t tid;
    pthread_create(&tid, NULL, handle_client, (void *)client);
    pthread_detach(tid); // Auto-cleanup thread
  }

  return 0;
}
