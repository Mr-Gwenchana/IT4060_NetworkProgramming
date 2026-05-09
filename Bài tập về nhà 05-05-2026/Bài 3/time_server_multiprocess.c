#include <sys/socket.h>
#include<signal.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <stdio.h>
#include <string.h>
#include<sys/types.h>
#include <unistd.h>
#include<signal.h>
#include<stdlib.h>
#include<time.h>



void handleSigchild(int sig){
    int pid = wait(NULL);
    printf("Child process terminated: %d\n", pid);
}

char* process_time_command(char inp[]){
    static char res[128];
    char cmd[20];
    char format[40];
    
    inp[strcspn(inp, "\n")] = 0;
    memset(res, 0, sizeof(res));

    int n = sscanf(inp, "%s %s", cmd, format);

    if (n < 1 || strcmp(cmd, "GET_TIME") != 0) {
        return "Command is not supported. Please follow format: GET_TIME <timeformat>\n";
    }

    if (n < 2) {
        return "Time format must not null !\n";
    }

    time_t rawtime;
    struct tm *timee;
    time(&rawtime);
    timee = localtime(&rawtime);

    char *time_format_oup = NULL;

    if (strcmp(format, "dd/mm/yyyy") == 0)      
        time_format_oup = "%d/%m/%Y";
    else if (strcmp(format, "dd/mm/yy") == 0)   
        time_format_oup = "%d/%m/%y";
    else if (strcmp(format, "mm/dd/yyyy") == 0) 
        time_format_oup = "%m/%d/%Y";
    else if (strcmp(format, "mm/dd/yy") == 0)   
        time_format_oup = "%m/%d/%y";

    if (time_format_oup != NULL) {
        strftime(res, sizeof(res), time_format_oup, timee);
        strcat(res, "\n");
        return res;
    } else {
        return "Your time format is not supported!\n";
    }
}

int main()
{
    // Create socket
    int time_sv_soc = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (time_sv_soc == -1)
    {
        printf("Create socket failed !\n");
        return 1;
    }

    // Create address of socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8888);

    // Bind address to socket
    if (bind(time_sv_soc, (struct sockaddr *)&addr, sizeof(addr)))
    {
        printf("Binding address to socket failed !\n");
        close(time_sv_soc);
        return 1;
    }

    // Server listening
    if (listen(time_sv_soc, 5))
    {
        printf("Listen failed !\n");
        close(time_sv_soc);
        return 1;
    }

    printf("Server is listening !\n");

    // Accept client and send data
    signal(SIGCHLD, handleSigchild);
    while(1){
        int client = accept(time_sv_soc, NULL, NULL);
        if(client == -1)
            continue;

        printf("New client connected !\n");

        if(fork() == 0){
            close(time_sv_soc);

            char recvData[256];
            while(1){
                int ret = recv(client, recvData, sizeof(recvData), 0);
                if(ret <= 0 ){
                    printf("Client %d disconnected\n", client);
                    break;
                }

                recvData[ret] = 0;

                char *sendData = process_time_command(recvData);
                send(client, sendData, strlen(sendData), 0);
                printf("Repply command success !\n");
            }
            close(client);
            exit(EXIT_SUCCESS);
        }
        close(client);
    }
    close (time_sv_soc);
    return 0;
}