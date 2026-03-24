#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

int main(int argc, char *argv[])
{
    // Create socket
    int b1_client_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (b1_client_soc == -1)
    {
        printf("Socket creation failed !\n");
        return 0;
    }

    // Server address
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Connect to server
    int res = connect(b1_client_soc, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (res == -1)
    {
        printf("Connect to server failed !\n");
        return 0;
    }

    printf("----Connect to server success----\n");

    // Received command and send data
    char command[20];
    DIR *dp;
    struct dirent *cur_file;
    char temp_buff[1024];
    struct stat f_stat;
    while (1)
    {
        printf("\nEnter command: ");
        fgets(command, sizeof(command), stdin);
        command[strcspn(command, "\n")] = 0;

        if (strncmp(command, "exit", 4) == 0)
        {
            printf("Disconnect to server\n");
            break;
        }
        else if (strncmp(command, "send", 4) == 0)
        {
            char cur_dir[512];
            if (getcwd(cur_dir, sizeof(cur_dir)) == NULL)
            {
                printf("Can't read current directory !\n");
                continue;
            }

            sprintf(temp_buff, "%s\n", cur_dir);
            send(b1_client_soc, temp_buff, strlen(temp_buff), 0);
            dp = opendir(".");
            if (dp == NULL)
            {
                printf("Can't open current directory !\n");
                continue;
            }

            while ((cur_file = readdir(dp)) != NULL)
            {
                if (strcmp(cur_file->d_name, ".") == 0 || strcmp(cur_file->d_name, "..") == 0)
                    continue;

                if (stat(cur_file->d_name, &f_stat) == 0)
                {
                    sprintf(temp_buff, "%s\n", cur_file->d_name);
                    send(b1_client_soc, temp_buff, strlen(temp_buff), 0);

                    uint32_t file_size = htonl((uint32_t)f_stat.st_size);
                    send(b1_client_soc, &file_size, sizeof(file_size), 0);
                }
            }
            send(b1_client_soc, "\n", 1, 0);
        }
    }

    // Close connection
    close(b1_client_soc);
    return 0;
}