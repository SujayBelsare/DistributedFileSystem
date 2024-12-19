#include "../header/main.h"

void init_logging(FILE* log_file) {
    // Open the log file
    log_file = fopen(LOG_FILE, "w");
    if (log_file == NULL) {
        perror("Error opening log file");
        exit(1);
    }
}

void close_logging(FILE* log_file) {
    // Close the log file
    if(log_file){
        fclose(log_file);
    }
}

void log_client_request(FILE* log_file, const char* client_ip, const char* port, const char* operation, const char* status) {
    // Log the client request
    fprintf(log_file, "Client %s:%s requested %s: %s\n", client_ip, port, operation, status);
}

void log_server_response(FILE* log_file, const char* client_ip, const char* port, const char* operation, const char* status) {
    // Log the server response
    fprintf(log_file, "Server responded to %s:%s for %s: %s\n", client_ip, port, operation, status);
}

void log_system_event(const char* event, const char* description) {
    // Log a system event
    printf("System event: %s: %s\n", event, description);
}