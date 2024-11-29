#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define SERVER_IP "127.0.0.1"  // Dirección IP del servidor
#define SERVER_PORT 8080       // Puerto del servidor
#define MAX_MESSAGE_LENGTH 256 // Longitud máxima del mensaje

// Tipos de usuario
// typedef enum { PREPAGO, POSPAGO } TipoUsuario;
typedef enum { PREPAID, POSTPAID } TipoUsuario;

// Función para generar un mensaje aleatorio
void generarMensaje(char *mensaje, int id, TipoUsuario tipo) {
    const char *tipoUsuario = (tipo == PREPAID) ? "PREPAID" : "POSTPAID";
    // snprintf(mensaje, MAX_MESSAGE_LENGTH, "ID: %d-%s, Mensaje: Hola desde el cliente!", id, tipoUsuario);
    snprintf(mensaje, MAX_MESSAGE_LENGTH, "ID: %d, Tipo: %s, Mensaje: Hola desde el cliente!", id, tipoUsuario);
}

int main(int argc, char *argv[]) {
    // Comprobar si se ha proporcionado el número de mensajes como argumento
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <numero_de_mensajes>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int numMensajes = atoi(argv[1]);
    if (numMensajes <= 0) {
        fprintf(stderr, "El número de mensajes debe ser mayor que 0.\n");
        exit(EXIT_FAILURE);
    }

    // check user id input
    int userId;
    if (argc >= 3) {
        userId = atoi(argv[2]);
        if (userId <= 0) {
            fprintf(stderr, "El ID de usuario debe ser mayor que 0.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        srand(time(NULL)); // Inicializar la semilla para generar números aleatorios
        userId = rand() % 1000; // Generar un ID aleatorio
    }

    // Check tipoUsuario input
    TipoUsuario tipo;
    if (argc >= 4) {
        if (strcmp(argv[3], "PREPAID") == 0) {
            tipo = PREPAID;
        } else if (strcmp(argv[3], "POSTPAID") == 0) {
            tipo = POSTPAID;
        } else {
            fprintf(stderr, "El tipo de usuario debe ser PREPAID o POSTPAID.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        tipo = (rand() % 2 == 0) ? PREPAID : POSTPAID; // Seleccionar el tipo de usuario aleatoriamente
    }

    // Crear el socket
    int sock;
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Error al crear el socket");
        exit(EXIT_FAILURE);
    }

    // Configurar la dirección del servidor
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        perror("Error al convertir la dirección IP");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Conectar al servidor
    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Error al conectar con el servidor");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Enviar los mensajes
    for (int i = 0; i < numMensajes; ++i) {
        char mensaje[MAX_MESSAGE_LENGTH];
        generarMensaje(mensaje, userId, tipo);

        // Enviar el mensaje al servidor
        if (send(sock, mensaje, strlen(mensaje), 0) < 0) {
            perror("Error al enviar el mensaje");
            close(sock);
            exit(EXIT_FAILURE);
        }

        printf("Mensaje enviado: %s\n", mensaje);

        // Recibir la respuesta del servidor
        char respuesta[MAX_MESSAGE_LENGTH];
        memset(respuesta, 0, MAX_MESSAGE_LENGTH);
        int bytesRecibidos = recv(sock, respuesta, MAX_MESSAGE_LENGTH, 0);
        if (bytesRecibidos < 0) {
            perror("Error al recibir la respuesta");
        } else {
            printf("Respuesta del servidor: %s\n", respuesta);
        }
    }

    // Cerrar el socket
    close(sock);
    return 0;
}
