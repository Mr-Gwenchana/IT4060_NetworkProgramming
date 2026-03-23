#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdio.h>
#include<time.h>

int main(int argc, char *argv[]){

    // Tạo socket
    int sv_client_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sv_client_soc == -1){
        printf("Socket creation failed!\n");
        return 0 ;
    }
    else
        printf("Socket created\n");
        
    // Tạo địa chỉ của server cần kết nối
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(argv[1]);
    server_addr.sin_port = htons(atoi(argv[2]));

    // Kết nối tới server
    int res = connect(sv_client_soc, (struct sockaddr*) &server_addr, sizeof(server_addr));
    if(res == -1){
        printf("Can't connect to server !\n");
        return 0;
    }
    else
        printf("Connect to server success !\n");
    // Nhập vào và gửi đi dữ liệu
    char MSSV[20];
    char Name[40];
    char avg_grade[10];

    while(1)
    {
        printf("MSSV: ");
        fgets(MSSV, sizeof(MSSV), stdin);
        if(strcmp(MSSV, "exit") == 0)
            break;
        MSSV[strcspn(MSSV, "\n")] = 0;
        printf("Ho va ten: ");
        fgets(Name, sizeof(Name), stdin);
        Name[strcspn(Name, "\n")] = 0;
        printf("Diem trung binh: ");
        fgets(avg_grade, sizeof(avg_grade), stdin);
        avg_grade[strcspn(avg_grade, "\n")] = 0;
        char data[128];

        time_t now;
        time(&now);
        struct tm *cur_time = localtime(&now);
        char time_format[50];
        strftime(time_format, sizeof(time_format), "%Y-%m-%d %H:%M:%S", cur_time);
        sprintf(data, "%s %s %s %s", time_format, MSSV, Name, avg_grade);

        int res_sent = send(sv_client_soc, data, sizeof(data), 0);
        if (res_sent == -1) {
            perror("Send failed\n");
            break;
        }
        printf("Sent data success\n\n");
    }

    // Đóng kết nối
    close(sv_client_soc);
    printf("TCP has disconnected");
    return 0;
}