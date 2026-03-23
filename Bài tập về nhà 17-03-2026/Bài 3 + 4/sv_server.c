#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include<stdio.h>
#include<stdlib.h>

int main(int argc, char *argv[])
{
    // Tạo socket
    int sv_server_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sv_server_soc == -1)
    {
        printf("Socket creation failed !\n");
        return 0;
    }
    printf("Socket created\n");

    // Tạo address cho server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // Bind address vào server socket
    if (bind(sv_server_soc, (struct sockaddr*)&addr, sizeof(addr))){
        printf("Binding server address to socket failed !\n");
        return 0;
    }

    // Lắng nghe client kết nối
    if (listen(sv_server_soc, 5))
    {
        printf("Port listen failed !\n");
        return 0;
    }

    B1: 
    printf("Waiting client....\n");

    // Đồng ý kết nối client
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client = accept(sv_server_soc, (struct sockaddr*) &client_addr, &client_len);
    if (client == -1)
    {
        printf("Accept client failed !\n");
    }
    
    printf("Client connected !\n");
    // Nhận dữ liệu
    char *store_file = argv[2];
    FILE *s_f = fopen(store_file, "w");
    if (s_f == NULL)
    {
        perror("Can't open stored file");
        close(client);
        return 0;
    }

    char buffer[256];
    int bytes_recv;

    while (1)
    {
        // Nhận dữ liệu từ client
        bytes_recv = recv(client, buffer, sizeof(buffer), 0);

        if (bytes_recv == -1)
        {
            perror("Receive failed !\n");
            break;
        }
        else if (bytes_recv == 0)
        {
            printf("Client disconnected !\n");
            goto B1;
            break;
        }
        if (bytes_recv < sizeof(buffer))
            buffer[bytes_recv] = 0;

        char *client_IP = inet_ntoa(client_addr.sin_addr);
        fprintf(s_f, "%s %s\n", client_IP, buffer);
        fflush(s_f);
        printf("Received: %s %s\n", client_IP, buffer);
    }

    // Đóng kết nối
    fclose(s_f);
    close(client);
    close(sv_server_soc);
    printf("Server closed\n");

    return 0;
}