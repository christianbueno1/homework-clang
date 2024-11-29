#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "uthash.h"

#define SERVER_PORT 8080
#define MAX_CLIENTS 100
#define MAX_MESSAGE_LENGTH 256
#define MAX_PREPAID_MESSAGES 10

// Estructura para manejar la información del usuario
typedef enum
{
    PREPAID,
    POSTPAID
} TipoUsuario;

typedef struct
{
    int id;
    TipoUsuario tipo;
    int mensajesProcesados;
    char mensaje[MAX_MESSAGE_LENGTH];
    UT_hash_handle hh; // Campo necesario para uthash
} Usuario;

// Hash table para almacenar los usuarios
Usuario *usuarios = NULL;

// Mutex para sincronización
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

// Función para simular el procesamiento de un mensaje
void procesarMensaje(char *mensaje, Usuario *usuario)
{
    // Simular el tiempo de procesamiento
    int tiempoProcesamiento = 1000; // X milisegundos
    usleep(tiempoProcesamiento * 1000);

    // Procesar mensaje
    printf("Procesando mensaje de usuario ID: %d, Tipo: %s\n", usuario->id,
           (usuario->tipo == PREPAID) ? "PREPAID" : "POSTPAID");

    // Incrementar el conteo de mensajes procesados para usuarios pre-pago
    if (usuario->tipo == PREPAID)
    {
        usuario->mensajesProcesados++;
        if (usuario->mensajesProcesados >= MAX_PREPAID_MESSAGES)
        {
            printf("Usuario ID: %d ha alcanzado el máximo de mensajes PREPAID.\n", usuario->id);
        }
    }
}

// Función para manejar a cada cliente
void *manejarCliente(void *arg)
{
    int clientSock = *(int *)arg;
    free(arg);
    char buffer[MAX_MESSAGE_LENGTH];
    Usuario *usuario;

    // Recibir mensajes del cliente
    while (1)
    {
        memset(buffer, 0, MAX_MESSAGE_LENGTH);
        int bytesRecibidos = recv(clientSock, buffer, MAX_MESSAGE_LENGTH, 0);
        if (bytesRecibidos <= 0)
        {   
            // close the conection with the current client
            printf("Cliente desconectado.\n");
            close(clientSock);
            return NULL;

            // continue listening for messages from the client
            // continue;

        }

        // Aquí se debería analizar el mensaje para obtener el ID y el tipo de usuario
        char tipoStr[10];
        int id;
        int parsed = sscanf(buffer, "ID: %d, Tipo: %[^,], Mensaje: %[^\n]", &id, tipoStr, buffer);
        // Remove trailing comma from tipo
        char *comma = strchr(tipoStr, ',');
        // if not NULL, replace the comma with a null terminator
        if (comma) {
            *comma = '\0';
        }
        if (parsed != 3)
        {
            // Terminates the client connection and the thread handling the client upon encountering a parsing error. This is a more severe response, effectively disconnecting the client.
            // fprintf(stderr, "Mensaje no válido: %s\n", buffer);
            // close(clientSock);
            // return NULL;

            //  Ignores the invalid message and continues to handle the client connection, allowing the server to process future messages from the client. This is a more lenient response, maintaining the connection despite the error.
            printf("Error al analizar el mensaje.\n");
            continue;

        }

        // Buscar el usuario en la tabla hash
        pthread_mutex_lock(&lock); // Bloquear el mutex
        HASH_FIND_INT(usuarios, &id, usuario);
        if (usuario == NULL)
        {
            // Crear un nuevo usuario
            usuario = (Usuario *)malloc(sizeof(Usuario));
            usuario->id = id;
            usuario->mensajesProcesados = 0;
            HASH_ADD_INT(usuarios, id, usuario);
        }
        pthread_mutex_unlock(&lock); // Desbloquear el mutex
        
        // actualiza el tipo de usuario
        if (strcmp(tipoStr, "PREPAID") == 0)
        {
            usuario->tipo = PREPAID;
        }
        else if (strcmp(tipoStr, "POSTPAID") == 0)
        {
            usuario->tipo = POSTPAID; 
        } else
        {
            printf("Tipo de usuario no válido.\n");
            continue;
            // fprintf(stderr, "Tipo de usuario desconocido: %s\n", tipoStr);
            // close(clientSock);
            // return NULL;
        }

        // Procesar el mensaje
        pthread_mutex_lock(&lock);
        procesarMensaje(buffer, usuario);
        pthread_mutex_unlock(&lock);       

        // Responder al cliente
        char respuesta[MAX_MESSAGE_LENGTH];
        snprintf(respuesta, MAX_MESSAGE_LENGTH, "Mensaje procesado para el usuario ID: %d", usuario->id);
        send(clientSock, respuesta, strlen(respuesta), 0);
    }

    close(clientSock);
    return NULL;
}

int main(int argc, char *argv[])
{
    // Crear el socket del servidor
    int serverSock = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSock < 0)
    {
        perror("Error al crear el socket del servidor");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    // Enlazar el socket a la dirección y puerto
    if (bind(serverSock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        perror("Error al enlazar el socket");
        close(serverSock);
        exit(EXIT_FAILURE);
    }

    // Escuchar conexiones entrantes
    if (listen(serverSock, MAX_CLIENTS) < 0)
    {
        perror("Error al escuchar en el socket");
        close(serverSock);
        exit(EXIT_FAILURE);
    }

    printf("Servidor ChismeGPT escuchando en el puerto %d...\n", SERVER_PORT);

    while (1)
    {
        // Aceptar conexiones de clientes
        struct sockaddr_in clientAddr;
        socklen_t clientLen = sizeof(clientAddr);
        int *clientSock = malloc(sizeof(int));
        *clientSock = accept(serverSock, (struct sockaddr *)&clientAddr, &clientLen);
        if (*clientSock < 0)
        {
            perror("Error al aceptar la conexión");
            free(clientSock);
            continue;
        }

        printf("Cliente conectado.\n");

        // Crear un hilo para manejar al cliente
        pthread_t threadId;
        if (pthread_create(&threadId, NULL, manejarCliente, clientSock) != 0)
        {
            perror("Error al crear el hilo");
            free(clientSock);
            continue;
        }

        pthread_detach(threadId); // Separar el hilo para manejar múltiples clientes concurrentemente
    }

    close(serverSock);
    return 0;
}
