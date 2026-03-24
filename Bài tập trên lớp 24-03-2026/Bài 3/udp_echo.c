#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(int argc, char *argv[]){
    // Create socket UDP
    int udp_echo_soc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(udp_echo_soc == -1){
        printf("Create UDP socket failed !\n");
        return 1;
    }

    printf("Socket created.\n");
    // Address of receiver
    struct sockaddr_in echo_addr;
    echo_addr.sin_family = AF_INET;
    echo_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    echo_addr.sin_port = htons(atoi(argv[1]));

    // Binding address to socket
    int Binding = bind(udp_echo_soc, (struct sockaddr*) &echo_addr, sizeof(echo_addr));
    if(Binding == -1){
        printf("Binding address to socket failed !\n");
        return 1;
    }

    printf("Listening...\n");
    // Received data and send back
    char buff[256];
    char send_back[512];
    struct sockaddr_in recv_addr;
    socklen_t recv_addr_len = sizeof(recv_addr);
    while(1){
        // Received data
        memset(buff, 0, sizeof(buff));
        memset(send_back, 0, sizeof(send_back));
        int bytes_recv = recvfrom(udp_echo_soc, buff, sizeof(buff), 0, (struct sockaddr*) &recv_addr, &recv_addr_len);
        if(bytes_recv <= 0){
            printf("Can't received data !\n");
            continue;
        }

        //Send data back
        buff[bytes_recv] = '\0';
        snprintf(send_back, sizeof(send_back), "From echo: %s", buff);
        printf("Data received, send back on progress....\n");
        int sb_res = sendto(udp_echo_soc, send_back, sizeof(send_back), 0, (struct sockaddr*) &recv_addr, sizeof(recv_addr));
        if(sb_res == -1){
            printf("Send back failed !\n");
            continue;
        }
        else
            printf("Send back success !\n");
    }
}