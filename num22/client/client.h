#ifndef CLIENT_H
#define CLIENT_H
#include <poll.h>

void error_exit(const char *msg);

void check_operation_differ(int result, const char* operation_name, int check_equal_to);

void check_operation_same(int result, const char* operation_name, int check_equal_to) ;

void send_to(int clientSocket, char* message);

void receive_from(int clientSocket, char* message);

int openClient(char* server_ip, char* path);

void checkmethod(int argc, char *argv[]);

int get_length_filename(char* path);

void get_filename(char* path, char** filename);

void parse_response(char* response, char** message, char** contents);

void format_request(char* method, char* path, char** request, char** added_content);

void sendGetRequestFile(int client, char* path, char* server_ip);

void get_check_cwd(char* cwd, char* path, char* server_ip);

void file_handler_write(char* cwd, char* path, char* decoded_response, char* server_ip);

void sendGetRequestList(int client, char* path, char* server_ip);

void Decode_contents(char* contents, char** decoded_contents, int client);

void sendListToPoll(char* list, int client, char* path, char* server_ip);

void get_into_struct(char* list, int num_files, int client, char* cwd, struct pollfd *fds, char*** filenames,  char*** parsed_list);

int file_handler_read(int client, char* file_path, char** file_contents, char* server_ip, char* path);

void sendPostRequest(int client, char* path, char* file_path, char* server_ip);

int file_extension_list(char* path);

void parse_path(char* arg_path, char** server_ip, char** route_path);

#endif