#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<stdio.h>
#include<string.h>

int main(int argc, char *argv[]){

    // Khởi tạo socket phía server
    int tcp_server_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(tcp_server_soc == -1){
        printf("Socket creation failed !\n");
        return 0;
    }
    else
        printf("Socket created\n");

    // Tạo địa chỉ IP cho server
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(atoi(argv[1]));

    //Bind địa chỉ vào server socket
    if(bind(tcp_server_soc, (struct sockaddr*) &addr, sizeof(addr))){
        printf("Binding server address to socket failed !\n");
        return 0;
    }

    //Chuyển cổng sang trạng thái chờ
    if(listen(tcp_server_soc, 5)){
        printf("Port listen failed !\n");
        return 0;
    }

    printf("Waiting client ....\n");

    // Chấp nhận kết nối client
    int client = accept(tcp_server_soc, NULL, NULL);
    if(client == -1){
        printf("Accept client failed !\n");
        return 0;
    }

    printf("Client connected!\n");

    // Gửi lời chào từ file cho client
    char *welcome_file = argv[2];
    char *store_file = argv[3];
    FILE *w_f = fopen(welcome_file, "r");
    if (w_f == NULL) {
        perror("Can't open welcome file !\n");
    } else {
        char message[512];
        if (fgets(message, sizeof(message), w_f) != NULL) {
            send(client, message, strlen(message), 0);
            printf("Welcome message was sent\n");
        }
        fclose(w_f);
    }

    // Nhận dữ liệu từ người dùng và lưu vào 1 file
    FILE *s_f = fopen(store_file, "w"); 
    if (s_f == NULL) {
        perror("Can't open stored file");
        close(client);
        return 0;
    }

    char buffer[512];
    int bytes_recv;

    while (1) {
        // Nhận dữ liệu từ client
        bytes_recv = recv(client, buffer, sizeof(buffer), 0);

        if (bytes_recv == -1) {
            perror("Receive failed !\n");
            break;
        } 
        else if (bytes_recv == 0) {
            printf("Client disconnected !\n");
            break;
        }
        if(bytes_recv < sizeof(buffer))
            buffer[bytes_recv] = 0;

        fprintf(s_f, "%s", buffer);
        fflush(s_f);
        printf("Received: %s", buffer);
    }

    fclose(s_f);
    close(client);
    close(tcp_server_soc);
    printf("Server closed\n");

    return 0;
}