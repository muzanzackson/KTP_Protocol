/*
Name    : Konduri Jeevan Varma
Roll No : 22CS10038
*/
#include <ksocket.h>

int main(int argc, char * argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <port> <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    char * filename ;

    int my_port = atoi(argv[1]);
    int op_port = atoi(argv[2]);
    filename = argv[3];

    int sockfd = k_socket(AF_INET, SOCK_KTP, 0);
    if(sockfd < 0){
        printf("Error in creating socket\n");
        printf("%d\n", errno);
        return 0;
    }

    printf("PID => %d\n", getpid());
    
    printf("Socket created and binded %d\n", sockfd);
    fflush(stdout);

    int ret = k_bind(sockfd, "127.0.0.1", my_port, "127.0.0.1", op_port);
    if(ret < 0){
        printf("Error in binding\n");
        return 0;
    }

    struct sockaddr_in dest_addr;
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(op_port);
    dest_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    char buffer[MESSAGE_SIZE];
    memset(buffer, 0, MESSAGE_SIZE);

    socklen_t len = sizeof(dest_addr);


    FILE *file = fopen(filename, "r");

    
    size_t bytesRead;

    int total_messages = 0;
    FILE * user1log = fopen("user1_logs.txt", "w");
    while ((bytesRead = fread(buffer, 1, MESSAGE_SIZE, file)) > 0) {
        // Process the buffer (print it as an example)

        ret = -1;
        while(ret < 0) {
            ret = k_sendto(sockfd, buffer, bytesRead, 0, (const struct sockaddr*)&dest_addr, len);
        }

        total_messages++;

        printf("Message %d sent\n",total_messages);


        fprintf(user1log,"Sent %d => ",total_messages);
        fflush(stdout);
        fwrite(buffer, 1, bytesRead, user1log);
        fprintf(user1log,"\n\n");
        fflush(stdout);

        memset(buffer, 0, MESSAGE_SIZE);
    }

    printf("\n");

    printf("Total messages sent: %d\n", total_messages);

    ret = -1;
    while(ret < 0) {
        ret = k_sendto(sockfd, "END", 3, 0, (const struct sockaddr*)&dest_addr, len);
    }

    fclose(file);

    sleep(10);

    // k_close(sockfd);
    
    return 0;
}