#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include "server.h"
#include <fcntl.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

// struct to pass the arguments to the process
struct HandleClientArgs {
    int* clientFD;
    char** rootDirectory;
};

// Function to check for errors in socket operations
void error_exit(const char* msg) 
{
    perror(msg);
    printf("Exiting...\n");
    exit(EXIT_FAILURE);
}

// Function to check errors (values should not differ)
void check_operation_same(int result, const char* operation_name, int check_equal_to, int ClientSocket) 
{
    if (result != check_equal_to) 
    {
        perror(operation_name);
        if (ClientSocket != -1) 
        {
            char* message = "500 INTERNAL ERROR\r\n\r\n"; 
            send_to(ClientSocket, message);
            close(ClientSocket);
        }
        exit(EXIT_FAILURE);
    }
}

// Function to check for errors in socket operations
void check_socket_operation(int result, const char* operation_name, int check_equal_to, int ClientSocket) 
{
    if (result == check_equal_to) 
    {
        perror(operation_name);
        if (ClientSocket != -1) 
        {
            char* message = "500 INTERNAL ERROR\r\n\r\n"; 
            send_to(ClientSocket, message);
            close(ClientSocket);
        }
        exit(EXIT_FAILURE);
    }
}

// Function to check for errors in socket operations
void handleClient(void* args) 
{

    struct HandleClientArgs* actualArgs = (struct HandleClientArgs*)args;
    int clientSocket = *(actualArgs->clientFD);
    char* dir = *(actualArgs->rootDirectory);

    char* message;
    char** msg_ptr = &message;
    recieve_from(clientSocket, msg_ptr);
    message = *msg_ptr;

    // parse the message
    char type = get_message_type(message);
    
    
    if (type != 'I')
    {
        parse_message(&message, type);
        if (type == 'G')
        {
            char* fullpath = malloc(strlen(message) + strlen(dir) + 3);
            strcpy(fullpath, dir);
            //strcat(fullpath, "/");
            strcat(fullpath, message);
            get_file(clientSocket, fullpath);
            free(fullpath);

        }
        else //(type == 'P')
        {
            char* fullpath = malloc(strlen(message) + strlen(dir) + 3);
            strcpy(fullpath, dir);
            char* path = strtok(message, "\r\n");
            strcat(fullpath, path);
            char* contents = strtok(NULL, "\r\n");
            post_file(clientSocket, fullpath, contents, strlen(contents));
            free(fullpath);
        }
    }
    close(clientSocket);
}


// Creates a server socket and checks for any errors during creation
void createServerSocket(int* serverSocket) 
{
    int domain = AF_INET;
    *serverSocket = socket(domain, SOCK_STREAM, 0);
    check_socket_operation(*serverSocket, "socket", -1, -1);
}

// Sets options for the server socket and checks for any errors
void setSocketOptions(int serverSocket) 
{
    int optVal = 1;
    int setsockopt_result = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(optVal));
    check_socket_operation(setsockopt_result, "setsockopt", -1, -1);
}

// Binds the server socket to an address and starts listening for connections
// Checks for any errors during binding and listening
void bindAndListen(int serverSocket, struct sockaddr_in addr) 
{
    int bind_result = bind(serverSocket, (struct sockaddr *) &addr, sizeof(addr));
    check_socket_operation(bind_result, "bind", -1, -1);

    int listen_result = listen(serverSocket, 5);
    check_socket_operation(listen_result, "listen", -1, -1);
}

// Accepts incoming connections and handles them in separate threads
// Checks for any errors during accepting connections
void acceptConnections(int serverSocket, char* rootDirectory) 
{
    while (1) 
    {
        struct sockaddr_in client_addr;
        socklen_t addrLen = sizeof(client_addr);
        int clientFD = accept(serverSocket, (struct sockaddr *) &client_addr, &addrLen);
        if (clientFD == -1) {
            perror("accept failed");
            continue;
        }

        printf("\nNew client accepted.\n");
        printf("Client details:\n");

        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, clientIP, INET_ADDRSTRLEN);
        printf("IP Address: %s\n", clientIP);
        printf("Port: %d\n", ntohs(client_addr.sin_port));

        // Handle client in a new thread
        pid_t pid;
        struct HandleClientArgs* args = malloc(sizeof(struct HandleClientArgs));
        args->clientFD = &clientFD;
        args->rootDirectory = &rootDirectory;

        pid = fork();

        if (pid < 0) 
        {
            perror("fork failed");
            continue;
        }
        else if (pid == 0) 
        {
            handleClient(args);
            exit(0);
        }
    }
}

// Opens a TCP server: creates a server socket, sets socket options,
// binds the socket to an address, starts listening for connections,
// and accepts incoming connections
void openTcpServer(int argc, char *const *argv) 
{
    int serverSocket;
    createServerSocket(&serverSocket);
    printf("Server socket created\n");
    setSocketOptions(serverSocket);
    printf("Socket options set\n");

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SERVER_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bindAndListen(serverSocket, addr);
    printf("Server socket bound and listening\n");
    char* rootDirectory = checkPath(argc, argv);
    acceptConnections(serverSocket, rootDirectory);

    close(serverSocket);
    free(rootDirectory);
}

//This function get the flag from the user when you run the server
char* checkPath(int argc, char *const *argv) {
    if (argc >= 2) 
    {
        char* path = argv[1];
        
        char* formattedPath = malloc(strlen(path) + 1);
        strcpy(formattedPath, path);
        return formattedPath;
         
    
    } 
    else
    {
        error_exit("no path was passed as an argument to the server\n");
        return NULL;
    }
}

//This function parse the message and skip the command
void parse_message(char** message, char type) 
{
    if(type == 'G')
    {
        (*message) += 4; //skip the command
    }
    else //(type == 'P')
    {
        (*message) += 5; //skip the command
    }

    char* end  = strstr((*message), "\r\n\r\n");
    if(end == NULL)
    {
        perror("Invalid message - without \\r\\n\\r\\n at end");
        return;
    }
    else
    {
        *end = '\0';
    }
}

//This function get the message type
char get_message_type(char* message) 
{
    if (strncmp(message, "GET ", 4) == 0) 
    {
        return 'G';
    } 
    else if (strncmp(message, "POST ", 5) == 0) 
    {
        return 'P';
    } 
    else 
    {
        perror("Invalid message type");
        return 'I';
    }
}

//This function get the file from the server
void get_file(int clientSocket, char* fullpath) 
{
    // Open the file
    int fd = open(fullpath, O_RDONLY);
    if (fd == -1) 
    {
        char* message = "404 File Not Found\r\n\r\n";
        send_to(clientSocket, message);
        return;
    }

    // Get the file size
    off_t fileSize = lseek(fd, 0, SEEK_END);
    lseek(fd, 0, SEEK_SET);

    // Allocate memory for the message
    char* crlf = "\r\n";
    char* message = malloc(fileSize + strlen("200 OK\r\n") + 2*strlen(crlf) + 1);

    // Construct the message
    strcpy(message, "200 OK\r\n");
    read(fd, message + strlen(message), fileSize);
    strcpy(message + strlen(message), crlf);
    strcpy(message + strlen(message), crlf);

    // Send the message
    send_to(clientSocket, message);

    close(fd);
    free(message);
}

//This function post the file to the server
void post_file(int clientSocket, char* fullpath, char* contents, int length) 
{
    char* message;
    if (access(fullpath, F_OK) != -1) 
    {
        message = "500 INTERNAL ERROR\r\n\r\n";
        
        send_to(clientSocket, message);

        return;
    }

    // Open the file
    int fd = open(fullpath, O_WRONLY | O_CREAT, 0666);
    if (fd == -1) 
    {
        message = "500 INTERNAL ERROR\r\n\r\n";
        
        send_to(clientSocket, message);
        close(clientSocket);
        free(fullpath);
        error_exit("open failed");
    }

    // Create a file lock
    struct flock fl;
    fl.l_type   = F_WRLCK;  // F_WRLCK = write lock (prevents other processes from both reading and writing to the fil)
    fl.l_whence = SEEK_SET; // SEEK_SET = set the position to the beginning of the file
    fl.l_start  = 0;        // Offset from l_whence
    fl.l_len    = 0;        // length, 0 = to EOF
    fl.l_pid    = getpid(); // our PID

    // Lock the file
    if (fcntl(fd, F_SETLKW, &fl) == -1) // F_SETLKW = set lock and wait
    {
        close(fd);
        message = "500 INTERNAL ERROR\r\n\r\n";
        
        send_to(clientSocket, message);
        close(clientSocket);
        free(fullpath);
        error_exit("fcntl failed");
    }

    // Write to the file
    int numbytes = write(fd, contents, length);
    if(numbytes != length)
    {
        perror("write failed, what do you want to do now?");
    }

    // Unlock the file
    fl.l_type = F_UNLCK;
    if (fcntl(fd, F_SETLK, &fl) == -1) 
    {
        perror("fcntl unlock failed, what do you want to do now?");
    }

    close(fd);
    message = "200 OK\r\n\r\n";
    send_to(clientSocket, message);
    return;
}

//This function send the message to the client
void send_to(int clientSocket, char* message) 
{
    uint32_t msg_len = strlen(message);
    uint32_t net_msg_len = htonl(msg_len);
    
    int send_result = send(clientSocket, &net_msg_len, sizeof(net_msg_len), 0);
    check_operation_same(send_result, "send", sizeof(net_msg_len), clientSocket);
    
    send_result = send(clientSocket, message, msg_len, 0);
    check_operation_same(send_result, "send", msg_len, clientSocket);
}

//This function recieve the message from the client
void recieve_from(int clientSocket, char** message_ptr) 
{
    uint32_t msg_len;
    int recv_result = recv(clientSocket, &msg_len, sizeof(msg_len), 0);
    check_operation_same(recv_result, "recv", sizeof(msg_len), clientSocket);

    msg_len = ntohl(msg_len);
    *(message_ptr) = malloc(msg_len + 1);
    

    recv_result = recv(clientSocket, *(message_ptr), msg_len, 0);
    check_operation_same(recv_result, "recv", msg_len, clientSocket);

    (*(message_ptr))[msg_len] = '\0';

    return;
}

int main(int argc, char *argv[]) 
{
    openTcpServer(argc, argv);
    return 0;
}