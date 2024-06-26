#include "utils.h"
#include "client.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#define SERVER_PORT 8080
#define BUFFER_SIZE 1024
#define CWD_SIZE pathconf(".", _PC_PATH_MAX)
#define DEBUG 0

//Error handling functions
void error_exit(const char* msg) 
{
    perror(msg);
    exit(EXIT_FAILURE);
}

//Check if the result of an operation is same of a certain value (should be different)
void check_operation_differ(int result, const char* operation_name, int check_equal_to) 
{
    if (result == check_equal_to) 
    {
        printf("%s not good\n", operation_name);
        perror(operation_name);
        exit(EXIT_FAILURE);
    }
}

//Check if the result of an operation is different from a certain value (should be equal)
void check_operation_same(int result, const char* operation_name, int check_equal_to) 
{
    if (result != check_equal_to) 
    {
        printf("%s not good\n", operation_name);
        perror(operation_name);
        exit(EXIT_FAILURE);
    }
}

//This function decodes the contents of a file
void Decode_contents(char* contents, char** decoded_contents, int client)
{
    int cont_len = strlen(contents);
    base64_decode_string(contents, cont_len, decoded_contents);
    free(contents);
    return;
}

//This function gets the current working directory
void get_check_cwd(char* cwd, char* path, char* server_ip) 
{
    if (getcwd(cwd, CWD_SIZE) == NULL) 
    {
        free(path);
        free(server_ip);
        error_exit("getcwd error");
    }
    return;
}

//This function handles the file
void file_handler_write(char* cwd, char* path, char* decoded_response, char* server_ip)
{
    int filename_len = get_length_filename(path);
    char* filename = malloc(filename_len + 1);
    get_filename(path, &filename);
    char* filepath = malloc(strlen(cwd) + strlen(filename) + 2);
    sprintf(filepath, "%s/%s", cwd, filename);
    FILE* file = fopen(filepath, "w");
    free(filepath);
    free(filename);

    if (file == NULL) 
    {
        free(decoded_response);
        free(path);
        free(server_ip);
        error_exit("fopen error");
    }
    
    fwrite(decoded_response, 1, strlen(decoded_response), file);
    fclose(file);
    free(decoded_response);
    return;
}

//Sends a message to the server
void send_to(int clientSocket, char* message) 
{
    uint32_t msg_len = strlen(message);
    uint32_t net_msg_len = htonl(msg_len);

    int send_result = send(clientSocket, &net_msg_len, sizeof(net_msg_len), 0);
    check_operation_same(send_result, "send", sizeof(net_msg_len));
    
    send_result = send(clientSocket, message, msg_len, 0);
    check_operation_same(send_result, "send", msg_len);
}

//Receives a message from the server
void recieve_from(int clientSocket, char** message_ptr) 
{
    uint32_t msg_len;
    int recv_result = recv(clientSocket, &msg_len, sizeof(msg_len), 0);
    check_operation_same(recv_result, "recv", sizeof(msg_len));

    msg_len = ntohl(msg_len);
    *(message_ptr) = malloc(msg_len + 1);
    

    recv_result = recv(clientSocket, *(message_ptr), msg_len, 0);
    check_operation_same(recv_result, "recv", msg_len);

    (*(message_ptr))[msg_len] = '\0';

    return;
}

//Opens a client socket and connects to the server
int openClient(char* server_ip, char* path)
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    check_operation_differ(sock, "socket", -1);

    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(SERVER_PORT);
    server.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) 
    {
        perror("Failed to connect to server");
        free(server_ip);
        free(path);
        exit(EXIT_FAILURE);
    }
    
    if (DEBUG) printf("Connected to server!\n");

    return sock;
}

//Checks if the method is valid and if the number of arguments is correct
void checkmethod(int argc, char *argv[]) 
{
    if (argc < 3) {
        error_exit("Usage: ./client <method> <path>");
    }
    char* method = argv[1];
    if ((strcmp(method,"GET") != 0) && (strcmp(method,"POST") != 0)) {
        printf("Invalid method: %s\n", method);
        exit(EXIT_FAILURE);
    }
    if ((strcmp(method,"POST") == 0) && argc < 4) {
        error_exit("Usage: ./client POST <path> <Path to file>");
    }
    return;
}

//Returns the length of the filename
int get_length_filename(char* path)
{
    char* filename = strrchr(path, '/');
    if (filename == NULL) {
        return strlen(path);
    }
    return strlen(filename + 1);
}

//Returns the filename
void get_filename(char* path, char** filename)
{
    char* file_name = strrchr(path, '/');
    if (file_name == NULL) {
        strcpy(*filename, path);
    }
    else {
        strcpy(*filename, file_name + 1);
    }
    return;
}

//Parses the response from the server
void parse_response(char* response, char** message, char** contents)
{
    char* token = strtok(response, "\r\n");
    *message = malloc(strlen(token) + 1);
    strcpy(*message, token);
    token = strtok(NULL, "\r\n");
    if (token != NULL)
    {
        *contents = malloc(strlen(token) + 1);
        strcpy(*contents, token);
        token = strtok(NULL, "\r\n");
    }
    else *contents = malloc(1);
    return;

}

//Formats the request
void format_request(char* method, char* path, char** request, char** added_contents)
{
    char* crlf;
    if (strcmp(method, "GET") == 0)
    {
        crlf = "\r\n\r\n";
        *request = malloc(strlen(method) + strlen(path) + strlen(crlf) + 2);
        sprintf(*request, "%s %s", method, path);
        strcat(*request, crlf);
        return;
    }
    else // (strcmp(method, "POST") == 0)
    {
        crlf = "\r\n";
        *request = malloc(strlen(method) + strlen(path) + 3 * strlen(crlf) + strlen(*added_contents) + 2);
        sprintf(*request, "%s %s", method, path);
        strcat(*request, crlf);
        strcat(*request, *added_contents);
        strcat(*request, crlf);
        strcat(*request, crlf);
        return;
    }
}

//This function gets a path to a file and sends a GET request for the file
void sendGetRequestFile(int client, char* path, char* server_ip) 
{
    //Sending the request
    char* request;
    format_request("GET", path, &request, NULL);
    send_to(client, request);
    free(request);

    //Receiving the response
    char* response;
    recieve_from(client, &response);

    //parsing the response
    char* message;
    char* contents;
    parse_response(response, &message, &contents);
    
    //validating (the desired message is "OK")
    if(strcmp(message, "200 OK") != 0)
    {
        free(response);
        free(contents);
        close(client);
        perror(message);
        free(message);
        free(server_ip);
        free(path);
        exit(0);
    }
    printf("%s\n", message);
    free(response);
    free(message);

    //Decoding the response
    char* decoded_response;
    Decode_contents(contents, &decoded_response, client);

    //Writing the response to a file
    char cwd[CWD_SIZE];
    get_check_cwd(cwd, path, server_ip);

    file_handler_write(cwd, path, decoded_response, server_ip);

    close(client);
    return;
}

//This function sends a GET request for a list of files
void sendGetRequestList(int client, char* path, char* server_ip)
{
    //Sending the request
    char* request;
    format_request("GET", path, &request, NULL);
    send_to(client, request);
    free(request);

    //Receiving the response
    char* response;
    recieve_from(client, &response);

    //parsing the response
    char* message;
    char* contents;
    parse_response(response, &message, &contents);
    
    //validating (the desired message is "OK")
    if(strcmp(message, "200 OK") != 0)
    {
        free(response);
        free(contents);
        close(client);
        perror(message);
        free(message);
        free(server_ip);
        free(path);
        exit(0);
    }
    printf("%s\n", message);
    free(response);
    free(message);

    //Decoding the response
    char* decoded_response;
    Decode_contents(contents, &decoded_response, client);

    sendListToPoll(decoded_response, client, path, server_ip);
    return;

}

//This function gets the contents of the files into a struct
void get_into_struct(char* list, int num_files, int client, char* cwd,  struct pollfd* fds, char*** filenames, char*** parsed_list)
{
    if (DEBUG) printf("list = %s\n", list);
    char* token = strtok(list, "\n");

    *parsed_list = (char**) malloc(num_files * sizeof(char*));
    *filenames = (char**) malloc(num_files * sizeof(char*));

    // filing lists with the file names and path for each path on the list
    for (int i = 0; i < num_files; i++)
    {
        (*parsed_list)[i] = (char*) malloc(strlen(token) + 1);
        memcpy((*parsed_list)[i], token, strlen(token));
        (*parsed_list)[i][strlen(token)] = '\0';

        char* file_name = strrchr(token, '/');
        (*filenames)[i] = (char*) malloc(strlen(file_name) + 1);
        memcpy((*filenames)[i], file_name, strlen(file_name));
        (*filenames)[i][strlen(file_name)] = '\0';

        token = strtok(NULL, "\n");
    }
    
    // getting into the struct for the poll() function
    for (int i = 0; i < num_files; i++)
    {
        token = (char*)malloc(strlen(((*parsed_list)[i])+1) * sizeof(char));
        strcpy(token, (*parsed_list)[i]);
        char* server_ip;
        char* path;

        parse_path(token, &server_ip, &path);
        char* request;
        format_request("GET", path, &request, NULL);
        int new_client = openClient(server_ip, path);
        free(path);
        free(server_ip);
        send_to(new_client, request);
        free(request);
        fds[i].fd = new_client;
        fds[i].events = POLLIN;
        free(token);
    }
    free(list);
    return;
}

//This function sends a list of files to the server
void sendListToPoll(char* list, int client, char* path, char* server_ip)
{
    char cwd[CWD_SIZE];
    get_check_cwd(cwd, path, server_ip);
    free(path);
    
    char* list_copy = malloc(strlen(list) + 1);
    strcpy(list_copy, list);
    char* token = strtok(list, "\n");
    int num_files = 0;

    while (token != NULL)
    {
        num_files++;
        token = strtok(NULL, "\n");
    }
    if (DEBUG) printf("num_files = %d\n", num_files);
    free(list);
    
    struct pollfd fds[num_files];
    char** filenames;
    char** paths;
    get_into_struct(list_copy, num_files, client, cwd, fds, &filenames, &paths);

    //Using poll() to receive the responses
    int poll_result = poll(fds, num_files, -1);
    check_operation_differ(poll_result, "poll", -1);

    for (int i = 0; i < num_files; i++)
    {
        if (fds[i].revents & POLLIN)
        {
            char* response;
            recieve_from(fds[i].fd, &response);
            char* message;
            char* contents;

            parse_response(response, &message, &contents);
            free(response);

            //check what the mesage is
            if(strcmp(message ,"200 OK") != 0)
            {
                perror(message);
                free(message);
                free(contents);
                close(fds[i].fd);
                continue;
            }
            free(message);
            

            int extension = file_extension_list(filenames[i]);
            if(extension == 1) //got a .list file in the list file
            {
                free(filenames[i]);
                close(fds[i].fd);
                char* cur_server_ip;
                char* cur_path;
                parse_path(paths[i], &cur_server_ip, &cur_path);
                int new_client = openClient(cur_server_ip, cur_path);
                free(paths[i]);
                sendGetRequestList(new_client, cur_path, cur_server_ip);
                free(cur_server_ip);
            }
            else
            {
                //decoding the contents
                char* decoded_contents;
                Decode_contents(contents, &decoded_contents, fds[i].fd);

                file_handler_write(cwd, filenames[i], decoded_contents, server_ip);
                free(filenames[i]);
                close(fds[i].fd);
            }
        }
    }
    free(filenames);
    free(paths);
    close(client);
    return;
}

//This function reads the contents of a file
int file_handler_read(int client, char* file_path, char** file_contents, char* server_ip, char* path)
{
    FILE* file = fopen(file_path, "r");
    if (file == NULL) 
    {
        free(path);
        free(server_ip);
        error_exit("fopen error");
    }
    fseek(file, 0, SEEK_END);
    int file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    *file_contents = malloc(file_size + 1);
    fread(*file_contents, 1, file_size, file);
    (*file_contents)[file_size] = '\0';

    fclose(file);
    return file_size;
}

//This function sends a POST request
void sendPostRequest(int client, char* path, char* file_path, char* server_ip)
{
    //Reading the file
    char* file_contents;
    int file_size = file_handler_read(client, file_path, &file_contents, server_ip, path);

    //Encoding the file
    char* encoded_file;
    base64_encode_string(file_contents, file_size, &encoded_file);
    free(file_contents);

    int encoded_size = size_of_encoded_string(file_size);
    encoded_file = realloc(encoded_file, encoded_size + strlen("\r\n\r\n") + 1);
    if (!encoded_file) 
    {
    perror("Failed to allocate memory for encoded file");
    exit(EXIT_FAILURE);
    }
    strcat(encoded_file, "\r\n\r\n");
    
    //Sending the request
    char* request;
    format_request("POST", path, &request, &encoded_file);
    if (DEBUG) printf("%s\n", request);
    send_to(client, request);
    free(request);

    //Sending the file
    send_to(client, encoded_file);
    free(encoded_file);
    if (DEBUG) printf("sent file\n");

    //Receiving the response
    char* response;
    recieve_from(client, &response);
    if (DEBUG) printf("response = %s\n", response);

    //parsing the response
    char* message;
    char* contents;
    parse_response(response, &message, &contents);
    if (DEBUG) printf("parsed response\n");
    
    //validating (the desired message is "OK")
    if(strcmp(message, "200 OK") != 0)
    {
        free(response);
        free(contents);
        close(client);
        perror(message);
        free(message);
        free(server_ip);
        free(path);
        exit(0);
    }
    printf("%s\n", message);
    free(response);
    free(message);
    free(contents);
    close(client);
    return;
}

//This function checks the file extension
int file_extension_list(char* path)
{
    char* filename = strrchr(path, '/');
    char* extension = strrchr(filename, '.');
    if (extension == NULL) {
        return -1;
    }
    else if (strcmp(extension, ".list") == 0) {
        return 1;
    }
    else {
        return 0;
    }
}

//function to parse the path (using DNS to get the server IP - somethig like "http://www.google.com/file.txt")
void parse_path(char* arg_path, char** server_ip, char** route_path) 
{
    char* path = arg_path;

    // server name = whats between "http://" and the first "/" or between "https://" and the first "/"
    //get the server name
    //check the prefix of the path
    if (strncmp(path, "http://", 7) != 0 && strncmp(path, "https://", 8) != 0) {
        error_exit("Invalid path - must start with http:// or https://");
    }
    else if (strncmp(path, "http://", 7) == 0) {
        path += 7;
    }
    else {
        path += 8;
    }
    
    char* file_path = malloc(strlen(path) + 1);
    char* token = strtok(path, "/");
    char* server_name = token;
    
    // getting the full path afte the DNS
    token = strtok(NULL, "/");
    strcpy(file_path, token);
    while ((token = strtok(NULL, "/")) != NULL) {
        char tmp_buffer[strlen(file_path) + strlen(token) + 2];
        sprintf(tmp_buffer, "%s/%s", file_path, token);
        strcpy(file_path, tmp_buffer);
    }
    
    // parsing DNS
    struct hostent *server = gethostbyname(server_name);
    if (server == NULL) {
        free(server_ip);
        free(route_path);
        free(file_path);
        error_exit("No such host");
    }
    
    // getting the ip
    *server_ip = malloc(16);
    strcpy(*server_ip, inet_ntoa(*((struct in_addr *)server->h_addr_list[0])));

    *route_path = malloc(strlen(file_path) + 2);
    strcpy(*route_path, "/");
    strcat(*route_path, file_path);
    free(file_path);
    return;
}

int main(int argc, char *argv[]) {
    char* server_ip;
    char* path;
    checkmethod(argc, argv);
    parse_path(argv[2], &server_ip, &path);
    int client = openClient(server_ip, path);
    if (DEBUG) printf("path = %s\n", path);
    if (DEBUG) printf("method = %s\n", argv[1]);
    if (DEBUG) printf("server_ip = %s\n", server_ip);
    char* method = argv[1];
    
    if (strcmp(method,"GET") == 0)
    {
        if (DEBUG) printf("method is GET\n");
        int extension = file_extension_list(path);
        if (extension == 1) {
            sendGetRequestList(client, path, server_ip);
        }
        else if (extension == 0) {
            if (DEBUG) printf("requested a file\n");
            sendGetRequestFile(client, path, server_ip);
        }
        else { 
            free(server_ip);
            free(path);
            error_exit("No file extension provided");
        }
    }
    else // POST
    {
        if (DEBUG) printf("method is POST\n");
        sendPostRequest(client, path, argv[3], server_ip);
    }
    free(server_ip);
    return 0;
}