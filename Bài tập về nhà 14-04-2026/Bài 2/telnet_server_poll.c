#include <sys/socket.h>
#include <sys/poll.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define MAX_CLIENTS 64

int is_regist[MAX_CLIENTS];

int check_reg(char *inp, int cli_pos)
{
    char *username = strtok(inp, " ");
    char *password = strtok(NULL, "\n");

    if (username == NULL || password == NULL)
        return 0;

    char f_username[30], f_password[30];
    FILE *f_stream = fopen("user.txt", "r");
    while (fscanf(f_stream, "%s %s", f_username, f_password) != EOF)
    {
        if (strcmp(f_username, username) == 0 && strcmp(f_password, password) == 0)
        {
            is_regist[cli_pos] = 1;
            fclose(f_stream);
            return 1;
        }
    }
    return 0;
}

int main()
{
    // Create socket
    int telnet_poll_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (telnet_poll_soc == -1)
    {
        printf("Create socket failed !\n");
        return 1;
    }

    // Create address for server socket
    struct sockaddr_in telnet_addr;
    telnet_addr.sin_family = AF_INET;
    telnet_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    telnet_addr.sin_port = htons(8888);

    // Binding address to the socket
    if (bind(telnet_poll_soc, (struct sockaddr *)&telnet_addr, sizeof(telnet_addr)))
    {
        printf("Binding socket failed !\n");
        return 1;
    }

    // Listening connection
    if (listen(telnet_poll_soc, 5))
    {
        printf("Listening failed !\n");
        return 1;
    }

    printf("Server listening....\n");

    // Receive and send data
    struct pollfd fds[MAX_CLIENTS];
    int nfds = 1;
    fds[0].fd = telnet_poll_soc;
    fds[0].events = POLLIN;

    char *request = "Hãy gửi thông tin đăng nhập theo định dạng \"username password\":\n";
    char *unauth_mess = "Thông tin đăng nhập không hợp lệ, vui lòng nhập lại !\n";
    char buff[512];
    memset(buff, 0, sizeof(buff));
    memset(is_regist, 0, sizeof(is_regist));

    while (1)
    {
        int res = poll(fds, nfds, -1);
        if (res == 0){
            continue;
        }

        if (res == -1){
            printf("Poll socket failed !\n");
            close(telnet_poll_soc);
            return 1;
        }

        // Accept new client
        if (fds[0].revents & POLLIN)
        {
            int client = accept(fds[0].fd, NULL, NULL);
            if (client == -1)
            {
                printf("Refuse new user !\n");
                close(client);
            }
            else
            {
                fds[nfds].fd = client;
                fds[nfds].events = POLLIN;
                nfds++;
                is_regist[nfds] = 0;
                printf("New client connected :))\n");
                send(client, request, strlen(request), 0);
            }
        }

        // Received message from client
        for (int i = 1; i < nfds; i++)
        {
            if (fds[i].revents & POLLIN)
            {
                memset(buff, 0, sizeof(buff));
                int res_recv = recv(fds[i].fd, buff, sizeof(buff), 0);
                buff[res_recv] = 0;
                if (res_recv <= 0)
                {
                    printf("Client %d disconnected\n", fds[i].fd);
                    close(fds[i].fd);

                    if (i == nfds - 1)
                    {
                        nfds--;
                    }
                    else
                    {
                        fds[i] = fds[nfds - 1];
                        is_regist[i] = is_regist[nfds - 1];
                        nfds--;
                        i--;
                    }
                }

                else
                {
                    if (!is_regist[i])
                    {
                        if (check_reg(buff, i))
                        {
                            char *mess = "Regist success ! Please send commands\n";
                            send(fds[i].fd, mess, strlen(mess), 0);
                        }
                        else{
                            send(fds[i].fd, unauth_mess, strlen(unauth_mess), 0);
                        }
                    }

                    else
                    {
                        buff[strcspn(buff, "\n")] = 0;
                        char command[1200];
                        sprintf(command, "(%s) > out.txt", buff);

                        system(command);

                        char outp[1024];

                        send(fds[i].fd, "Result:\n", 8, 0);
                        FILE *send_stream = fopen("out.txt", "r");
                        while (fgets(outp, sizeof(outp), send_stream) != NULL)
                        {
                            send(fds[i].fd, outp, strlen(outp), 0);
                        }
                        send(fds[i].fd, "\n", 1, 0);
                        fclose(send_stream);
                    }
                }
            }
        }
    }
}