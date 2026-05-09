#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void signalHandler(int sig) {
    int pid = wait(NULL);
    printf("Child process terminated: %d\n", pid);
}

int check_reg(char *inp) {
    char *username = strtok(inp, " ");
    char *password = strtok(NULL, "\n");

    if (username == NULL || password == NULL)
        return 0;

    char f_username[30], f_password[30];
    FILE *f_stream = fopen("user.txt", "r");
    if (f_stream == NULL) {
        perror("Cannot open user.txt");
        return 0;
    }

    while (fscanf(f_stream, "%s %s", f_username, f_password) != EOF) {
        if (strcmp(f_username, username) == 0 && strcmp(f_password, password) == 0) {
            fclose(f_stream);
            return 1;
        }
    }
    fclose(f_stream);
    return 0;
}

int main() {

    // Create server socket
    int telnet_sv_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (telnet_sv_soc == -1) {
        perror("Creat socket failed!");
        return 1;
    }
    
    // Create address for server socket
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8888);
    
    // Binding address to socket
    if (bind(telnet_sv_soc, (struct sockaddr *)&addr, sizeof(addr))) {
        perror("socket binding failed");
        close(telnet_sv_soc);
        return 1;
    }
    
    // Listening connection
    if (listen(telnet_sv_soc, 5)) {
        perror("Socket listening failed");
        close(telnet_sv_soc);
        return 1;
    }
    
    printf("Server is listening...\n");

    // Regist handle SIGCHILD signal
    signal(SIGCHLD, signalHandler);

    char *request = "Hãy gửi thông tin đăng nhập theo định dạng \"username password\":\n";
    char *unauth_mess = "Thông tin đăng nhập không hợp lệ, vui lòng nhập lại !\n";
    char *auth_success = "Regist success ! Please send commands\n";

    while (1) {
        int client = accept(telnet_sv_soc, NULL, NULL);
        if (client == -1) 
            continue;
        
        printf("New client connected\n");
        
        if (fork() == 0) {
            close(telnet_sv_soc);

            int is_login = 0;
            char buff[512];
            
            send(client, request, strlen(request), 0);

            while (1) {
                memset(buff, 0, sizeof(buff));
                int len = recv(client, buff, sizeof(buff) - 1, 0);
                
                if (len <= 0) {
                    printf("Client %d disconnected\n", client);
                    break;
                }
                
                buff[len] = 0;
                
                if (!is_login) {
                    if (check_reg(buff)) {
                        is_login = 1;
                        send(client, auth_success, strlen(auth_success), 0);
                    } else {
                        send(client, unauth_mess, strlen(unauth_mess), 0);
                    }
                } 

                else {
                    buff[strcspn(buff, "\n")] = 0;
                    if (strlen(buff) == 0) 
                        continue;

                    char command[600];
                    sprintf(command, "(%s) > out.txt", buff);
                    system(command);

                    send(client, "Result:\n", 8, 0);
                    FILE *f_out = fopen("out.txt", "r");
                    if (f_out) {
                        char line[1024];
                        while (fgets(line, sizeof(line), f_out)) {
                            send(client, line, strlen(line), 0);
                        }
                        fclose(f_out);
                    }
                    send(client, "\n", 1, 0);
                }
            }
            close(client);
            exit(EXIT_SUCCESS);
        }
        close(client);
    }

    close(telnet_sv_soc);
    return 0;
}