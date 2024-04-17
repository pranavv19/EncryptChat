#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>
#include <ctype.h>

#define PORT 10200
#define MAX_CLIENTS 10
#define MAX_HISTORY_SIZE 100

typedef struct Message {
    char username[100];
    char text[1024];
} Message;

typedef struct {
    int socket;
    struct sockaddr_in address;
} Client;

Client clients[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
Message chat_history[MAX_CLIENTS][MAX_HISTORY_SIZE];
int chat_history_size[MAX_CLIENTS] = {0};

void encrypt(char *message, int shift) {
    int length = strlen(message);
    for (int i = 0; i < length; i++) {
        if (isalpha(message[i])) {
            char base = islower(message[i]) ? 'a' : 'A';
            message[i] = (message[i] - base + shift) % 26 + base;
        }
    }
}

void decrypt(char *message, int shift) {
    encrypt(message, 26 - shift);
}

void print_all_messages(int client_no) {
    printf("All Messages from client %d:\n", client_no);
    for (int i = 0; i < chat_history_size[client_no]; i++) {
        printf("%s: %s\n", chat_history[client_no][i].username, chat_history[client_no][i].text);
    }
    printf("End of Messages\n");
}

void add_message_to_history(Message msg, int client_no, int shift) {
    encrypt(msg.text, shift);
    if (chat_history_size[client_no] < MAX_HISTORY_SIZE) {
        chat_history[client_no][chat_history_size[client_no]] = msg;
        chat_history_size[client_no]++;
    } else {
        for (int i = 0; i < MAX_HISTORY_SIZE - 1; i++) {
            chat_history[client_no][i] = chat_history[client_no][i + 1];
        }
        chat_history[client_no][MAX_HISTORY_SIZE - 1] = msg;
    }
}

void send_chat_history(int client_socket, int client_no, int shift) {
    for (int i = 0; i < chat_history_size[client_no]; i++) {
        Message history_msg = chat_history[client_no][i];
        decrypt(history_msg.text, shift);
        send(client_socket, &history_msg, sizeof(Message), 0);
        encrypt(chat_history[client_no][i].text, shift);
    }
}

void send_to_all(Message msg, int current_client) {
    pthread_mutex_lock(&mutex);
    if (strlen(msg.username) < 1) return;
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket != current_client) {
            send(clients[i].socket, &msg, sizeof(Message), 0);
        }
    }
    pthread_mutex_unlock(&mutex);
}

void send_to_user(Message msg, int client_no) {
    pthread_mutex_lock(&mutex);
    send(clients[client_no].socket, &msg, sizeof(Message), 0);
    pthread_mutex_unlock(&mutex);
}

void *handle_client(void *arg) {
    int new_socket = *((int *)arg);
    Message msg;
    while (recv(new_socket, &msg, sizeof(Message), 0) > 0) {
        struct timeval start, end;
        gettimeofday(&start, NULL);
        if (msg.text[0] == '#') {
            send_chat_history(new_socket, client_count, 6);
        } else if (msg.text[0] == '@' && isdigit(msg.text[1])) {
            send_to_user(msg, (msg.text[1] - '0'));
        } else if (msg.text[0] == '%') {
            print_all_messages(client_count);
        } else {
            send_to_all(msg, new_socket);
        }
        add_message_to_history(msg, client_count, 6);
        gettimeofday(&end, NULL);
        double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1e6;
        printf("RTT for message: %f seconds\n", elapsed);
    }
    pthread_mutex_lock(&mutex);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].socket == new_socket) {
            memmove(&clients[i], &clients[i + 1], (client_count - i - 1* of Client)) * sizeof(Client));
    client_count--;
    break;
}
pthread_mutex_unlock(&mutex);
close(new_socket);
pthread_exit(NULL);
}

int main() {
int server_socket, new_socket;
struct sockaddr_in server_addr, new_addr;
socklen_t addr_size;

server_socket = socket(AF_INET, SOCK_STREAM, 0);
server_addr.sin_family = AF_INET;
server_addr.sin_port = htons(PORT);
server_addr.sin_addr.s_addr = INADDR_ANY;

bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));
listen(server_socket, MAX_CLIENTS);

printf("Server is running on port %d...\n", PORT);

while (1) {
    addr_size = sizeof(new_addr);
    new_socket = accept(server_socket, (struct sockaddr *)&new_addr, &addr_size);

    pthread_t tid;
    pthread_create(&tid, NULL, handle_client, &new_socket);

    pthread_mutex_lock(&mutex);
    clients[client_count].socket = new_socket;
    clients[client_count].address = new_addr;
    client_count++;
    pthread_mutex_unlock(&mutex);
}

return 0;
}
