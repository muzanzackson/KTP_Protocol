
/*
Name    : Konduri Jeevan Varma
Roll No : 22CS10038
*/
#include <ksocket.h>

// Function to create an int8u value
int8u init_int8u(int value) {
    int8u result;
    result.value = value % 16; // Ensure value is within the range 0 to 15
    return result;
}

// Function to add two int8u values
int8u add_int8u(int8u a, int8u b) {
    int sum = (a.value + b.value) % 16; // Perform addition and modulo operation
    return init_int8u(sum);
}

// Function to subtract two int8u values
int8u sub_int8u(int8u a, int8u b) {
    int diff = (a.value - b.value + 16) % 16; // Perform subtraction and ensure positive result
    return init_int8u(diff);
}

ktpSocket *get_shared_KTP_Table() {
    key_t sm_key = ftok(".", KTP_TABLE);
    int sm_id = shmget(sm_key, (MAX_SOCKETS)*sizeof(ktpSocket), 0777|IPC_CREAT);
    if (sm_id == -1) {
        perror("shmget");
        return NULL;
    }
    ktpSocket *ktpTable = (ktpSocket *)shmat(sm_id, 0, 0);
    return ktpTable;
}

SOCK_INFO *get_SOCK_INFO() {
    key_t sm_key = ftok(".", SHARED_RESOURCE);
    int sm_id_sock_info = shmget(sm_key, sizeof(int), 0777|IPC_CREAT);
    if (sm_id_sock_info == -1) {
        perror("shmget");
        return NULL;
    }
    SOCK_INFO *vars = (SOCK_INFO *)shmat(sm_id_sock_info, 0, 0);
    return vars;
}

void get_sem1(int *id) {
    key_t sm_key = ftok(".", SEM1);
    *id = semget(sm_key, 1, 0777|IPC_CREAT);
}

void get_sem2(int *id) {
    key_t sm_key = ftok(".", SEM2);
    *id = semget(sm_key, 1, 0777|IPC_CREAT);
}

void get_mutex(int *id) {
    key_t sm_key = ftok(".", MUTEX);
    *id = semget(sm_key, 1, 0777|IPC_CREAT);
}

void get_mutex_swnd(int *id) {
    key_t sm_key = ftok(".", MUTEX_SWND);
    *id = semget(sm_key, 1, 0777 | IPC_CREAT);
}

void get_mutex_sendbuf(int *id) {
    key_t sm_key = ftok(".", MUTEX_SENDBUF);
    *id = semget(sm_key, 1, 0777 | IPC_CREAT);
}

void get_mutex_recvbuf(int *id) {
    key_t sm_key = ftok(".", MUTEX_RECVBUF);
    *id = semget(sm_key, 1, 0777 | IPC_CREAT);
}

int k_socket(int domain, int type, int protocol) {

    if(type != SOCK_KTP) {
        return ERROR;
    }
    
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    ktpSocket *KTP_Table = get_shared_KTP_Table();
    SOCK_INFO *sock_info = get_SOCK_INFO();

    int sem1;
    get_sem1(&sem1);

    int sem2;
    get_sem2(&sem2);

    int mutex;
    get_mutex(&mutex);

    for (int i = 0; i < MAX_SOCKETS; i++) {
        if(KTP_Table[i].available == 1) {
            down(mutex);
            int ktp_id = i;
            KTP_Table[i].available = 0;
            KTP_Table[i].pid = getpid();
            
            sock_info->status = 0;
            sock_info->ktp_id = ktp_id;

            up(sem1);
            down(sem2);

            if(sock_info->error_no != 0) {
                errno = sock_info->error_no;
                KTP_Table[i].available = 1;
                up(mutex);

                shmdt(KTP_Table);
                shmdt(sock_info);

                return ERROR;
            }
            
            shmdt(KTP_Table);
            shmdt(sock_info);

            up(mutex);
            return ktp_id;
        }
    }

    errno = ENOBUFS; 
    up(mutex);   
 
    shmdt(KTP_Table);
    shmdt(sock_info);

    return ERROR;    
}

int k_close(int socket_id) {
    
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    ktpSocket *KTP_Table = get_shared_KTP_Table();
    SOCK_INFO *sock_info = get_SOCK_INFO();

    int sem1;
    get_sem1(&sem1);

    int sem2;
    get_sem2(&sem2);

    int mutex;
    get_mutex(&mutex);

    int mutex_swnd;
    get_mutex_swnd(&mutex_swnd);

    int mutex_sendbuf;
    get_mutex_sendbuf(&mutex_sendbuf);

    int mutex_recvbuf;
    get_mutex_recvbuf(&mutex_recvbuf);

    down(mutex);
    
    if(KTP_Table[socket_id].available == 0) {
        KTP_Table[socket_id].available = 1;
        KTP_Table[socket_id].udp_sockid = -1;

        down(mutex_swnd);
        down(mutex_sendbuf);

        for (int k = 0; k < SEND_BUFFER_SIZE; k++) {
            KTP_Table[socket_id].send_buffer[k].header.seq_number = -1;
            memset(KTP_Table[socket_id].send_buffer[k].data, 0, MESSAGE_SIZE);
        }

        up(mutex_sendbuf);

        KTP_Table[socket_id].swnd.left_idx = 0;
        KTP_Table[socket_id].swnd.right_idx = (5 - 1) % SEND_BUFFER_SIZE;
        KTP_Table[socket_id].swnd.fresh_msg = 0;
        KTP_Table[socket_id].swnd.last_seq_no = 0;
        KTP_Table[socket_id].swnd.last_sent = -1;
        KTP_Table[socket_id].swnd.last_ack_seqno = 0;

        for (int k = 0; k < 5; k++) {
            KTP_Table[socket_id].swnd.last_sent_time[k] = 0;
        }

        up(mutex_swnd);

        down(mutex_recvbuf);
        
        KTP_Table[socket_id].rwnd.full = 0;
        KTP_Table[socket_id].rwnd.last_inorder_received = 0;
        KTP_Table[socket_id].rwnd.last_consumed = 0;

        for (int k = 0; k < 5; k++) {
            KTP_Table[socket_id].receive_buffer[k].header.seq_number = -1;
            memset(KTP_Table[socket_id].receive_buffer[k].data, 0, MESSAGE_SIZE);
        }

        for (int k = 0; k < RECEIVER_WINDOW_SIZE; k++) {
            KTP_Table[socket_id].rwnd.window[k] = k+1;
        }

        up(mutex_recvbuf);

        sock_info->ktp_id = socket_id;
        
        sock_info->status = 2;

        up(sem1);
        down(sem2);

        if(sock_info->error_no != 0) {
            errno = sock_info->error_no;
            KTP_Table[socket_id].available = 0;
            up(mutex);

            shmdt(KTP_Table);
            shmdt(sock_info);

            return ERROR;
        }

        int retval = sock_info->return_value;

        shmdt(KTP_Table);
        shmdt(sock_info);

        up(mutex);
        return retval;
    }

    up(mutex);
            
    shmdt(KTP_Table);
    shmdt(sock_info);

    return ERROR;
}

int k_bind(int socket_id, char *src_ip, unsigned short int src_port, char *dest_ip, unsigned short int dest_port){
    
    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    ktpSocket *KTP_Table = get_shared_KTP_Table();
    SOCK_INFO *sock_info = get_SOCK_INFO();

    int sem1;
    get_sem1(&sem1);

    int sem2;
    get_sem2(&sem2);

    int mutex;
    get_mutex(&mutex);

    down(mutex);

    if(KTP_Table[socket_id].available == 0) {
        sock_info->status = 1;
        sock_info->ktp_id = socket_id;

        strcpy(KTP_Table[socket_id].dest_ip, dest_ip);
        KTP_Table[socket_id].dest_port = dest_port;

        sock_info->src_addr.sin_addr.s_addr = inet_addr(src_ip);
        sock_info->src_addr.sin_port = htons(src_port);
        sock_info->src_addr.sin_family = AF_INET;

        up(sem1);
        down(sem2);

        if(sock_info->error_no!=0) {
            errno = sock_info->error_no;
            KTP_Table[socket_id].dest_ip[0] = '\0';
            KTP_Table[socket_id].dest_port = 0;
            up(mutex);

            shmdt(KTP_Table);
            shmdt(sock_info);

            return ERROR;
        }

        int retval = sock_info->return_value;

        shmdt(KTP_Table);
        shmdt(sock_info);

        up(mutex);
        return retval;
    }

    up(mutex);
            
    shmdt(KTP_Table);
    shmdt(sock_info);
    
    return ERROR;
}

int k_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {

    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    ktpSocket* KTP_Table = get_shared_KTP_Table();
    
    int mutex_swnd;
    get_mutex_swnd(&mutex_swnd);

    int mutex_sendbuf;
    get_mutex_sendbuf(&mutex_sendbuf);

    down(mutex_swnd);
    down(mutex_sendbuf);

    if(KTP_Table[sockfd].send_buffer[KTP_Table[sockfd].swnd.fresh_msg].header.seq_number != -1) {
        errno = ENOBUFS;

        up(mutex_sendbuf);
        up(mutex_swnd);

        shmdt(KTP_Table);

        return ERROR;
    }

    if (!(!KTP_Table[sockfd].available 
        && KTP_Table[sockfd].dest_port == ntohs(((struct sockaddr_in *)dest_addr)->sin_port) 
        && strcmp(KTP_Table[sockfd].dest_ip, inet_ntoa(((struct sockaddr_in *)dest_addr)->sin_addr))) == 0){

        shmdt(KTP_Table);
        errno = ENOTCONN;
        return ERROR;
    }

    int idx = KTP_Table[sockfd].swnd.fresh_msg;
    strncpy(KTP_Table[sockfd].send_buffer[idx].data, buf, MESSAGE_SIZE);

    KTP_Table[sockfd].send_buffer[idx].header.seq_number = (KTP_Table[sockfd].swnd.last_seq_no)%16 + 1;
    KTP_Table[sockfd].swnd.last_seq_no = KTP_Table[sockfd].send_buffer[idx].header.seq_number;
    KTP_Table[sockfd].swnd.fresh_msg = (KTP_Table[sockfd].swnd.fresh_msg+1)%SEND_BUFFER_SIZE;

    shmdt(KTP_Table);

    up(mutex_sendbuf);
    up(mutex_swnd);

    return 0;
}

int k_recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {

    pop.sem_num = vop.sem_num = 0;
    pop.sem_flg = vop.sem_flg = 0;
    pop.sem_op = -1; 
    vop.sem_op = 1;

    ktpSocket* KTP_Table = get_shared_KTP_Table();

    int mutex_recvbuf;
    get_mutex_recvbuf(&mutex_recvbuf);

    down(mutex_recvbuf);

    int min_seqno = (KTP_Table[sockfd].rwnd.last_consumed)%16+1;

    for(int i=0; i<5; i++) {
        if(KTP_Table[sockfd].receive_buffer[i].header.seq_number == min_seqno) {
            
            strncpy(buf, KTP_Table[sockfd].receive_buffer[i].data, MESSAGE_SIZE);
            KTP_Table[sockfd].receive_buffer[i].header.seq_number = -1;
            
            KTP_Table[sockfd].rwnd.last_consumed = min_seqno;

            shmdt(KTP_Table);
            up(mutex_recvbuf);
            return MESSAGE_SIZE;
        }
    }

    shmdt(KTP_Table);
    up(mutex_recvbuf);
    errno = ENOMSG;
    return ERROR;
}