#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{

    // Create socket of application
    int app_soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (app_soc == -1)
    {
        printf("Create socket failed !\n");
        return 1;
    }

    // Change socket to non-blocking mode
    unsigned long ul = 1;
    ioctl(app_soc, FIONBIO, &ul);
    ioctl(STDIN_FILENO, FIONBIO, &ul);

    // Create address of application
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    // Bind socket to address and listen
    if (bind(app_soc, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Binding socket failed !\n");
        return 1;
    }

    printf("Listening.....\n");

    // Create address to connect
    struct sockaddr_in cn_addr;
    cn_addr.sin_family = AF_INET;
    cn_addr.sin_addr.s_addr = inet_addr(argv[2]);
    cn_addr.sin_port = htons(atoi(argv[3]));

    // Send and received data
    char buff[512];
    char inp[1024];
    
    struct sockaddr_in recv_IP;
    socklen_t recv_IP_len = sizeof(recv_IP);

    printf("Enter message: ");
    memset(inp, 0, sizeof(inp));
    memset(buff, 0, sizeof(buff));

    while (1)
    {
        memset(inp, 0, sizeof(inp));
        memset(buff, 0, sizeof(buff));
        // Received data
        int bytes_recv = recvfrom(app_soc, buff, sizeof(buff), 0, (struct sockaddr *)&recv_IP, &recv_IP_len);
        if (bytes_recv > 0)
        {

            printf("\nReceived: %s\n", buff);
            printf("Enter message: ");
        }
        else if(bytes_recv == -1 && errno != EWOULDBLOCK)
        {
            printf("\nReceive data failed !\n");
            return 1;
        }

        // Send data
        if (fgets(inp, sizeof(inp), stdin) != NULL)
        {
            if (sendto(app_soc, inp, sizeof(inp), 0, (struct sockaddr *)&cn_addr, sizeof(cn_addr)) != -1)
            {
                printf("Enter message: ");
            }
        }
    }
}