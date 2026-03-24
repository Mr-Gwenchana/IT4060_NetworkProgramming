#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int cnt_substr(char *str, char *find)
{
    int cnt = 0;
    char *tmp = str;
    while ((tmp = strstr(tmp, find)) != NULL)
    {
        cnt++;
        tmp += 1;
    }
    return cnt;
}

int main(int argc, char *argv[])
{
    // Create socket
    int server_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (server_soc == -1)
    {
        printf("Socket creation failed !\n");
        return 1;
    }
    printf("Socket created\n");

    // Server Address
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // Binding address to socket
    if (bind(server_soc, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Binding socket failed !\n");
        return 1;
    }

    listen(server_soc, 5);
    printf("Waiting client...\n");

    // Accept client
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client = accept(server_soc, (struct sockaddr *)&client_addr, &client_len);

    // Received data
    char buff[512];
    char lastt[10];
    int res = 0;

    memset(lastt, 0, sizeof(lastt));
    while (1)
    {
        memset(buff, 0, sizeof(buff));
        int bytes_recv = recv(client, buff, sizeof(buff) - 1, 0);
        if (bytes_recv <= 0)
        {
            printf("Can't received data from client !\n");
            break;
        }

        // Delete "\n"
        char *Enter = strstr(buff, "\n");
        if (Enter != NULL)
        {
            *Enter = '\0';
            bytes_recv--;
        }

        char through[526];
        snprintf(through, sizeof(through), "%s%s", lastt, buff);
        int cur_cnt = cnt_substr(through, "0123456789");
        res += cur_cnt;

        if (bytes_recv >= 9)
        {
            strncpy(lastt, buff + bytes_recv - 9, 9);
            lastt[9] = '\0';
        }
        else
        {
            int through_len = strlen(through);
            if (through_len >= 9)
            {
                strcpy(lastt, through + through_len - 9);
            }
            else
            {
                strcpy(lastt, through);
            }
        }

        printf("Number of occurrences of the string: %d\n", res);
    }

    close(client);
    close(server_soc);
    return 0;
}