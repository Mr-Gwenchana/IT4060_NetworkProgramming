#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char client_id[FD_SETSIZE][50];
int is_regist[FD_SETSIZE];

// Check format and add new client name
int check_reg(char *buff, int cli_pos)
{
    if (strncmp(buff, "client_id: ", 11) == 0)
    {
        char *name = buff + 11;
        name[strcspn(name, "\n")] = 0;

        if (strlen(name) > 0)
        {
            strcpy(client_id[cli_pos], name);
            is_regist[cli_pos] = 1;
            return 1;
        }
    }
    return 0;
}

int main()
{
    // Create socket
    int server_multiplex = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_multiplex == -1)
    {
        printf("Create socket failed !\n");
        return 1;
    }

    // Create address of socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    // Bind address to socket
    if (bind(server_multiplex, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Binding address to socket failed !\n");
        close(server_multiplex);
        return 1;
    }

    // Server listening
    if (listen(server_multiplex, 5))
    {
        printf("Listen failed !\n");
        close(server_multiplex);
        return 1;
    }

    printf("Server is listening !\n");

    // Receive and process data from client
    fd_set fdread, fdtest;
    struct timeval tv;
    char buff[1024];
    memset(buff, 0, sizeof(buff)); 
    memset(client_id, 0, sizeof(client_id));
    memset(is_regist, 0, sizeof(is_regist));
   
    char *request = "Vui lòng nhập tên người dùng và đúng cú pháp!\n";

    FD_ZERO(&fdread);
    FD_SET(server_multiplex, &fdread);

    while (1)
    {
        fdtest = fdread;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int res = select(FD_SETSIZE, &fdtest, NULL, NULL, &tv);

        if (res < 0)
        {
            printf("Select socket failed !\n");
            return 1;
        }

        if (res == 0)
        {
            continue;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &fdtest))
            {
                // Has a connect event
                if (i == server_multiplex)
                {
                    int client = accept(server_multiplex, NULL, NULL);
                    if (client < FD_SETSIZE)
                    {
                        printf("New client connected !\n");
                        send(client, request, strlen(request), 0);
                        FD_SET(client, &fdread);
                    }

                    else {
                        close(client);
                    }
                }

                // Receive data event
                else
                {
                    memset(buff, 0, sizeof(buff));
                    int recv_res = recv(i, buff, sizeof(buff), 0);
                    if (recv_res <= 0)
                    {
                        printf("Client name %s disconnected!\n", client_id[i]);
                        close(i);
                        FD_CLR(i, &fdread);
                        is_regist[i] = 0;
                    }
                    else
                    {
                        buff[recv_res] = 0;

                        // Client doesn't have an ID to interact with server
                        if (!is_regist[i])
                        {
                            if (check_reg(buff, i))
                            {
                                char *mess = "Regist name success !\n";
                                send(i, mess, strlen(mess), 0);
                            }
                            else
                            {
                                send(i, request, strlen(request), 0);
                            }
                        }

                        // Client has an ID to interact with server
                        else
                        {
                            char send_buff[1200];
                            sprintf(send_buff, "%s: %s", client_id[i], buff);

                            for (int j = 0; j < FD_SETSIZE; j++)
                            {
                                // Send to other client
                                if (FD_ISSET(j, &fdread) && j != server_multiplex && j != i && is_regist[j])
                                {
                                    send(j, send_buff, strlen(send_buff), 0);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    close(server_multiplex);
    return 0;
}
