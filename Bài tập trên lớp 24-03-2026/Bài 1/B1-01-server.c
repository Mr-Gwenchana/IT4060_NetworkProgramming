#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Create socket
    int sv_server_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sv_server_soc == -1)
    {
        printf("Can't create socket\n");
        return 1;
    }
    printf("Socket created\n");

    // Server address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // Binding address to socket
    bind(sv_server_soc, (struct sockaddr *)&addr, sizeof(addr));
B1:
    // Listening client connection
    listen(sv_server_soc, 5);
    printf("Waiting client....\n");

    // Accept client connection
    int client = accept(sv_server_soc, NULL, NULL);
    printf("Client connected!\n");

    // Received data
    char filepath[1024];
    char filename[256];
    int bytes_recv;
    int is_path = 0;
    int i = 0;
    char c;

    while (1)
    {
        // Received file path
        if (is_path == 0)
        {
            i = 0;
            while ((bytes_recv = recv(client, &c, 1, 0)) > 0)
            {
                if (c == '\n')
                    break;
                if (i < sizeof(filepath) - 1)
                    filepath[i++] = c;
            }
            if(bytes_recv > 0){
            filepath[i] = '\0';
            is_path = 1;
            printf("%s\n", filepath);
            }
        }

        //Received file name
        i = 0;
        while ((bytes_recv = recv(client, &c, 1, 0)) > 0)
        {
            if (c == '\n')
                break;
            if (i < sizeof(filename))
                filename[i++] = c;
        }
        filename[i] = '\0';
        if (filename[0] == '\0'){
            is_path = 0;
            continue;
        }

        // Receive file size
        uint32_t file_size;
        int size_recv = recv(client, &file_size, sizeof(file_size), 0);
        if (size_recv > 0)
        {
            file_size = ntohl(file_size);
            printf("%s - %u bytes\n", filename, file_size);
        }
    }
    close(sv_server_soc);
    return 0;
}
