#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include<stdlib.h>

int regist[FD_SETSIZE] = {0};

// Check username and password
int check_reg(char *inp, int cli_pos){
    char *username = strtok(inp, "-");
    char *password = strtok(NULL, "\n");
    if (username == NULL || password == NULL) 
        return 0;

    FILE *f_stream = fopen("auth.txt", "r");

    char f_username[30], f_password[30];
    while(fscanf(f_stream, "%s %s", f_username, f_password) != EOF){
        if(strcmp(username, f_username) == 0 && strcmp(password, f_password) == 0){
            regist[cli_pos] = 1;
            fclose(f_stream);
            return 1;
        }
    }

    fclose(f_stream);
    return 0;
}

int main()
{
    // Create server socket
    int telnet_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (telnet_soc == -1)
    {
        printf("Create socket failed !\n");
        return 1;
    }

    // Create server address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8080);

    // Bind address to socket
    if (bind(telnet_soc, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Binding address to socket failed !\n");
        close(telnet_soc);
        return 1;
    }

    // Server listening
    if (listen(telnet_soc, 5))
    {
        printf("Listen failed !\n");
        close(telnet_soc);
        return 1;
    }
    printf("Telnet server is listening...\n");

    // Receive connect and data
    fd_set fdread, fdtest;
    struct timeval tv;
    char buff[1024];
    char *request = "Vui lòng nhập username và password theo format username-password !\n";
    char *unauth_mess = "Thông tin đăng nhập không hợp lệ, vui lòng nhập lại !\n";

    FD_ZERO(&fdread);
    FD_SET(telnet_soc, &fdread);

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

                // Accept connect
                if (i == telnet_soc)
                {
                    int client = accept(telnet_soc, NULL, NULL);
                    if (client < FD_SETSIZE)
                    {
                        printf("New client connected !\n");
                        FD_SET(client, &fdread);
                        send(client, request, strlen(request), 0);
                    }
                    else
                    {
                        close(client);
                    }
                }

                // Received data
                else
                {
                    memset(buff, 0, sizeof(buff));
                    int recv_res = recv(i, buff, sizeof(buff), 0);
                    if (recv_res == 0)
                    {
                        printf("Client %d has disconnected.\n", i);
                        close(i);
                        FD_CLR(i, &fdread);
                        regist[i] = 0;
                    }
                    else
                    {
                        buff[recv_res] = 0;
                        if (!regist[i])
                        {
                            if (check_reg(buff, i))
                            {
                                char *mess = "Regist success, please send the command !\n";
                                send(i, mess, strlen(mess), 0); 
                            }
                            else
                            {
                                send(i, unauth_mess, strlen(unauth_mess), 0);
                            }
                        }

                        else {
                            buff[strcspn(buff, "\n")] = 0;
                            char command[1200];
                            sprintf(command,"(%s) > out.txt", buff);

                            system(command);

                            char outp[1024];

                            send(i, "Result:\n", 8, 0);
                            FILE *send_stream = fopen("out.txt", "r");
                            while(fgets(outp, sizeof(outp), send_stream) != NULL){
                                send(i, outp, strlen(outp), 0);
                            }
                            send(i, "\n", 1, 0);
                            fclose(send_stream);
                        }
                    }
                }
            }
        }
    }
    close(telnet_soc);
    return 0;
}