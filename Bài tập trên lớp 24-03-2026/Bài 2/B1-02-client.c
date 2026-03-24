#include<stdio.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>

int main(int argc, char *argv[]){


    // Create socket
    int client_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(client_soc == -1)
    {
        printf("Socket creation failed !\n");
        return 1;
    }
    else 
        printf("Socket created\n");

    // Server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Connect to server
    int res = connect(client_soc, (struct sockaddr *) &server_addr, sizeof(server_addr));
    if(res == -1){
        printf("Can't connect to server !\n");
        return 0;
    }

    else 
        printf("Connect to server success !\n");

    // Send data to server
    char buffer[512];
    while (1) {
        printf("Enter line: ");
        fgets(buffer, sizeof(buffer), stdin);

        if (strncmp(buffer, "exit", 4) == 0) 
            break;

        int bytes_sent = send(client_soc, buffer, strlen(buffer), 0);
        
        if (bytes_sent == -1) {
            perror("Send failed\n");
            break;
        }
    }
    
    close(client_soc);
    printf("TCP has disconnected");

    return 0;
}