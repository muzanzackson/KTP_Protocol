/*
Name    : Konduri Jeevan Varma
Roll No : 22CS10038
*/
#include <ksocket.h>

int main(int argc, char * argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <port> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int my_port = atoi(argv[1]);
    int op_port = atoi(argv[2]);

    int sockfd = k_socket(AF_INET, SOCK_KTP, 0);
    if(sockfd < 0){
        printf("Error in creating socket\n");
        printf("%d\n", errno);
        return 0;
    }

    int ret = k_bind(sockfd, "127.0.0.1", my_port, "127.0.0.1", op_port);
    if(ret < 0){
        printf("Error in binding\n");
        return 0;
    }

    printf("PID => %d\n", getpid());

    printf("Socket created and binded %d\n", sockfd);
    fflush(stdout);

    struct sockaddr_in src_addr;
    src_addr.sin_family = AF_INET;
    src_addr.sin_port = htons(op_port);
    src_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char buffer[MESSAGE_SIZE+1];
    memset(buffer, 0, MESSAGE_SIZE+1);

    socklen_t len = sizeof(src_addr);

    char filename[50];
    sprintf(filename, "output_%d.txt", my_port);
    FILE *file = fopen(filename, "w");

    int id = 1;
    FILE * user2log = fopen("user2_logs.txt", "w");
    while (1) {
        int ret = -1;
        while(ret < 0) {
            ret = k_recvfrom(sockfd, buffer, MESSAGE_SIZE+1, 0, (struct sockaddr *)&src_addr, &len);
        }

        if (strncmp(buffer, "END", 3) == 0) {
            // Uncomment this to print END on the local file
            // fprintf(file, "%s", buffer);
            printf("Received %d messages in total\n",id-1);
            printf("\nClosing local file.\n");
            break;
        }

        buffer[ret] = '\0';

        // printf("Received => %s\n", buffer);
        printf("Received message %d \n",id);
        fprintf(user2log,"Received %d => ", id++);
        fflush(stdout);
        fwrite(buffer, 1, ret, user2log);
        fprintf(user2log,"\n\n");
        fflush(stdout);

        fprintf(file, "%s", buffer);
        // fwrite(buffer, 1, ret, file);

        memset(buffer, 0, MESSAGE_SIZE);
    }
    printf("Output written in %s\n",filename);
    fclose(file);

    k_close(sockfd);

    return 0;
}