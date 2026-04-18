#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char client_id[64][50];
int is_regist[64];

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
    int chat_multiplex_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (chat_multiplex_soc == -1)
    {
        printf("Create socket failed !\n");
        return 1;
    }

    // Create address for server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8888);

    // Binding address to socket
    if (bind(chat_multiplex_soc, (struct sockaddr *)&server_addr, sizeof(server_addr)))
    {
        printf("Binding socket failed !\n");
        close(chat_multiplex_soc);
        return 1;
    }

    // Listening connection
    if (listen(chat_multiplex_soc, 5))
    {
        printf("Listening failed\n");
        close(chat_multiplex_soc);
        return 1;
    }

    printf("Server is listening...\n");

    // Connect, send and receive data multiplex
    struct pollfd fds[64];
    int nfds = 1;

    char *request = "Vui lòng nhập id đúng định dạng \"client_id: client_name\": \n";
    fds[0].fd = chat_multiplex_soc;
    fds[0].events = POLLIN;

    char buff[512];
    memset(buff, 0, sizeof(buff));
    memset(client_id, 0, sizeof(client_id));
    memset(is_regist, 0, sizeof(is_regist));

    while (1)
    {
        int res = poll(fds, nfds, -1);
        if (res == 0)
        {
            continue;
        }

        if (res == -1)
        {
            printf("Poll socket failed !\n");
            close(chat_multiplex_soc);
            return 1;
        }

        if (fds[0].revents & POLLIN)
        {
            int client = accept(chat_multiplex_soc, NULL, NULL);
            if (client == -1)
            {
                printf("Refuse new user connection !\n");
                close(client);
            }

            else
            {
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                is_regist[nfds] = 0; 
                nfds++;
                printf("New client connected !\n");
                send(client, request, strlen(request), 0);
            }
        }

        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                memset(buff, 0, sizeof(buff));
                int recv_res = recv(fds[i].fd, buff, sizeof(buff), 0);
                if (recv_res <= 0)
                {
                    printf("Client name %s disconnected!\n", client_id[i]);
                    close(fds[i].fd);

                    // Replace client i when i disconnect
                    if (i == nfds - 1)
                    {
                        nfds--;
                    }
                    else
                    {
                        fds[i] = fds[nfds - 1];
                        strcpy(client_id[i], client_id[nfds - 1]);
                        is_regist[i] = is_regist[nfds - 1];
                        nfds--;
                        i--;
                    }
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
                            send(fds[i].fd, mess, strlen(mess), 0);
                        }
                        else
                        {
                            send(fds[i].fd, request, strlen(request), 0);
                        }
                    }

                    // Client has an ID to interact with server
                    else
                    {
                        char send_buff[1200];
                        sprintf(send_buff, "%s: %s", client_id[i], buff);

                        for (int j = 1; j < nfds; j++)
                        {
                            // Send to other client
                            if (is_regist[j] && j != i)
                            {
                                send(fds[j].fd, send_buff, strlen(send_buff), 0);
                            }
                        }
                    }
                }
            }
        }
    }
}