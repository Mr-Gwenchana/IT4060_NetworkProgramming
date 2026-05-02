#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MAX_TOPICS_PER_CLIENT 10

char client_topics[FD_SETSIZE][MAX_TOPICS_PER_CLIENT][50];
int topic_counts[FD_SETSIZE];

int main()
{
    // Create socket
    int psub_server_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (psub_server_soc == -1)
    {
        printf("Create socket failed !\n");
        return 1;
    }

    // Create address and bind
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8888);

    if (bind(psub_server_soc, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Binding address to socket failed !\n");
        close(psub_server_soc);
        return 1;
    }

    // Listening connections from clients
    if (listen(psub_server_soc, 5))
    {
        printf("Listen failed !\n");
        close(psub_server_soc);
        return 1;
    }

    printf("Server is listening.....\n");


    // Receive command and send data
    fd_set fdread, fdtest;
    char buff[1024];
    memset(client_topics, 0, sizeof(client_topics));
    memset(topic_counts, 0, sizeof(topic_counts));

    FD_ZERO(&fdread);
    FD_SET(psub_server_soc, &fdread);

    while (1)
    {
        fdtest = fdread;
        int res = select(FD_SETSIZE, &fdtest, NULL, NULL, NULL);

        if (res < 0)
        {
            printf("Select socket failed !\n");
            return 1;
        }

        for (int i = 0; i < FD_SETSIZE; i++)
        {
            if (FD_ISSET(i, &fdtest))
            {
                if (i == psub_server_soc)
                {
                    int client = accept(psub_server_soc, NULL, NULL);
                    if (client < FD_SETSIZE)
                    {
                        printf("New client connected !\n");
                        FD_SET(client, &fdread);
                        topic_counts[client] = 0;
                    }
                    else close(client);
                }
                else
                {
                    memset(buff, 0, sizeof(buff));
                    int recv_res = recv(i, buff, sizeof(buff) - 1, 0);

                    if (recv_res <= 0)
                    {
                        printf("Client %d disconnected!\n", i);
                        close(i);
                        FD_CLR(i, &fdread);
                        topic_counts[i] = 0;
                    }
                    else
                    {
                        buff[recv_res] = 0;
                        buff[strcspn(buff, "\r\n")] = 0;

                        if (strncmp(buff, "SUB ", 4) == 0)
                        {
                            char *topic = buff + 4;
                            int exists = 0;
                            for (int t = 0; t < topic_counts[i]; t++) {
                                if (strcmp(client_topics[i][t], topic) == 0) exists = 1;
                            }

                            if (!exists && topic_counts[i] < MAX_TOPICS_PER_CLIENT) {
                                strcpy(client_topics[i][topic_counts[i]], topic);
                                topic_counts[i]++;
                                send(i, "SUB successful!\n", 16, 0);
                            } else if (exists) {
                                send(i, "Already subbed to this topic!\n", 30, 0);
                            } else {
                                send(i, "Topic limit reached!\n", 21, 0);
                            }
                        }

                        else if (strncmp(buff, "UNSUB ", 6) == 0)
                        {
                            char *topic = buff + 6;
                            int found = -1;
                            for (int t = 0; t < topic_counts[i]; t++) {
                                if (strcmp(client_topics[i][t], topic) == 0) {
                                    found = t;
                                    break;
                                }
                            }
                            if (found != -1) {
                                for (int t = found; t < topic_counts[i] - 1; t++) {
                                    strcpy(client_topics[i][t], client_topics[i][t+1]);
                                }
                                topic_counts[i]--;
                                send(i, "UNSUB successful!\n", 18, 0);
                            } else {
                                send(i, "You are not subbed to this topic!\n", 34, 0);
                            }
                        }

                        else if (strncmp(buff, "PUB ", 4) == 0)
                        {
                            char *topic_name = strtok(buff + 4, " ");
                            char *body = strtok(NULL, "");

                            if (topic_name != NULL && body != NULL)
                            {
                                char send_buff[1100];
                                sprintf(send_buff, "[%s]: %s\n", topic_name, body);

                                for (int j = 0; j < FD_SETSIZE; j++)
                                {
                                    if (FD_ISSET(j, &fdread) && j != psub_server_soc)
                                    {
                                        for (int t = 0; t < topic_counts[j]; t++) {
                                            if (strcmp(client_topics[j][t], topic_name) == 0) {
                                                send(j, send_buff, strlen(send_buff), 0);
                                                break; 
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    close(psub_server_soc);
    return 0;
}