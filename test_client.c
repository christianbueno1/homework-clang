// FILE: project1/test_chismeGPT.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <assert.h>
#include "client.c"

// Test function for generarMensaje
void test_generarMensaje() {
    char mensaje[MAX_MESSAGE_LENGTH];
    generarMensaje(mensaje, 1, PREPAGO);
    assert(strstr(mensaje, "ID: 1, Tipo: PREPAGO, Mensaje: Hola desde el cliente!") != NULL);

    generarMensaje(mensaje, 2, POSPAGO);
    assert(strstr(mensaje, "ID: 2, Tipo: POSPAGO, Mensaje: Hola desde el cliente!") != NULL);
}

// Test function for socket creation and connection
void test_socket_creation_and_connection() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    assert(inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) > 0);

    assert(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == 0);
    close(sock);
}

// Test function for message sending
void test_message_sending() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    assert(sock >= 0);

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    assert(inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) > 0);

    assert(connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == 0);

    char mensaje[MAX_MESSAGE_LENGTH];
    generarMensaje(mensaje, 1, PREPAGO);
    assert(send(sock, mensaje, strlen(mensaje), 0) >= 0);

    close(sock);
}

int main() {
    test_generarMensaje();
    test_socket_creation_and_connection();
    test_message_sending();

    printf("All tests passed!\n");
    return 0;
}