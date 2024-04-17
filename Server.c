#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/time.h>

#define PORT 10200

typedef struct Message {
    char username[100];
    char text[1024];
} Message;

void *receive_messages(void *arg) {
    int client_socket = *((int *)arg);
    Message msg;
    while (1) {
        int n = recv(client_socket, &msg, sizeof(Message), 0);
        if (n <= 0) {
            printf("Server disconnected. Exiting...\n");
            exit(0);
        }
        printf("%s: %s", msg.username, msg.text);
    }
}

int main() {
    int client_socket;
    struct sockaddr_in server_addr;
    pthread_t tid;

    client_socket = socket(AF_INET, SOCK_STREAM, 0);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr));

    pthread_create(&tid, NULL, receive_messages, &client_socket);

    char username[100];
    printf("Enter Username: ");
    scanf("%s", username);
    username[strlen(username)] = '\0';

    while (1) {
        char message[1024];
        struct timeval start, end;
        Message newmsg;

        fgets(message, sizeof(message), stdin); // Capture newline after scanf
        fgets(message, sizeof(message), stdin);

        strcpy(newmsg.username, username);
        strcpy(newmsg.text, message);

        send(client_socket, &newmsg, sizeof(Message), 0);

        gettimeofday(&start, NULL);
        // Simulating message send for RTT, actual send is above
        gettimeofday(&end, NULL);

        long seconds = end.tv_sec - start.tv_sec;
        long microseconds = end.tv_usec - start.tv_usec;
        double elapsed = seconds + microseconds*1e-6;
        printf("RTT for message: %f seconds\n", elapsed);
    }

    return 0;
}
