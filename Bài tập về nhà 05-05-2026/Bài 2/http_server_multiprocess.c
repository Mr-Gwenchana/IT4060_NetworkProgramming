#include <sys/socket.h>
#include<signal.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include<sys/types.h>
#include <unistd.h>

int main()
{
    // Create socket
    int http_sv_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (http_sv_soc == -1)
    {
        printf("Create socket failed !\n");
        return 1;
    }

    // Create address of socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8888);

    // Bind address to socket
    if (bind(http_sv_soc, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Binding address to socket failed !\n");
        close(http_sv_soc);
        return 1;
    }

    // Server listening
    if (listen(http_sv_soc, 5))
    {
        printf("Listen failed !\n");
        close(http_sv_soc);
        return 1;
    }

    printf("Server is listening !\n");

    // Connect and sending HTTP body to client, using preforking
    int num_process = 5;
    for (int i = 0; i < num_process; i++)
    {
        if (fork() == 0)
        {
            while (1)
            {
                int client = accept(http_sv_soc, NULL, NULL);
                printf("New client connected !\n");

                char buff[256];
                int ret = recv(client, buff, sizeof(buff), 0);

                if (ret > 0)
                {
                    buff[ret] = 0;
                    printf("Recv from client: %s\n\n", buff);

                    char *msg = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";
                    send(client, msg, strlen(msg), 0);
                    printf("Send HTTP Body success\n");
                }
                close(client);
            }
        }
    }
    wait(NULL);
    close(http_sv_soc);
    return 0;
}