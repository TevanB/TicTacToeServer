static void addclient(int fd, struct in_addr addr);
static void removeclient(int fd);
static void broadcast(char *s, int size);

int main(int argc, char **argv)
{
        //below 3 are for extractline implementation
        extern char *extractline(char *p, int size);
        extern void  activity(struct client *p);

        void insert(int key, struct in_addr data, int fd);
        int maxfd;
        fd_set fds;


    int c, port = 3000, status = 0;
    int fd, clientfd;
    socklen_t len;
    struct sockaddr_in r, q;

    while ((c = getopt(argc, argv, "p:")) != EOF) {
        switch (c) {
        case 'p':
            port = atoi(optarg);
            break;
        default:
            status = 1;
        }
    }
    if (status || optind < argc) {
        fprintf(stderr, "usage: %s [-p port]\n", argv[0]);
        return(1);
    }

    if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return(1);
    }

    memset(&r, '\0', sizeof r);
    r.sin_family = AF_INET;
    r.sin_addr.s_addr = INADDR_ANY;
    r.sin_port = htons(port);

    if (bind(fd, (struct sockaddr *)&r, sizeof r) < 0) {
        perror("bind");
        return(1);
    }

        for(int i=0; i<9; i++){
                board[i] = 1 + i;
        }



    if (listen(fd, 5)) {
        perror("listen");
        return(1);
    }

        len = sizeof(q);
        while(1){

                int connec_change = 0;
                FD_ZERO(&fds);
                FD_SET(fd, &fds);
                maxfd=fd;
                struct client *h = head;
                for(h=head; h; h=h->next){
                        FD_SET(h->fd, &fds);
                        if(h->fd > maxfd)
                                maxfd = h->fd;
                }
                int selec_r = select(maxfd+1, &fds, NULL, NULL, NULL);
                switch(selec_r){
                case 0:
                        printf("timeout happened\n");
                        break;
                case -1:
                        perror("select");
                        return(1);
                default:
                        if(FD_ISSET(fd, &fds)){
                                if((clientfd = accept(fd, (struct sockaddr *)&q, &len)) < 0){
                                        perror("accept");
                                        return(1);
                                }
                                addclient(fd, q.sin_addr);

                                FD_SET(clientfd, &fds);
                                maxfd = clientfd;
                                client_num++;
                                printf("new connection from %s\n", inet_ntoa(q.sin_addr));
                        }

                        struct client *p = head;
                        for(p=head; p; p=p->next){


                                for (p = head; p; p = p->next)
                                if (FD_ISSET(p->fd, &fds))
                                    break;
                                /*
                                 * it's not very likely that more than one client will drop at
                                 * once, so it's not a big loss that we process only one each
                                 * select(); we'll get it later...
                                 */
                                 if (p)
                                        activity(p);  /* might remove p from list, so can't be in the loop */




                        }
                        if(connec_change == 1){
                                connec_change = 0;
                                break;
                        }

                }
                printf("main server loop completed iter\n");
        }
}

char *extractline(char *p, int size){
        int n1;
        for (n1=0; n1 < size && p[n1] != '\r' && p[n1] != '\n'; n1++)
                ;
        if(n1 == size)
                return(NULL);


        if(p[n1] == '\r' && n1 + 1 < size && p[n1+1] == '\n'){
                p[n1] = '\0';
                return (p+n1+2);
        } else {
                p[n1] = '\0';
                return(p+n1+1);
        }

}

void showboard(int fd)
{
    char buf[100], *bufp, *boardp;
    int col, row;

    for (bufp = buf, col = 0, boardp = board; col < 3; col++) {
        for (row = 0; row < 3; row++, bufp += 4)
            sprintf(bufp, " %c |", *boardp++);
        bufp -= 2;  // kill last " |"
        strcpy(bufp, "\r\n---+---+---\r\n");
        bufp = strchr(bufp, '\0');
    }
    if (write(fd, buf, bufp - buf) != bufp-buf)
        perror("write");
}


int game_is_over()  /* returns winner, or ' ' for draw, or 0 for not over */
{
    int i, c;
    extern int allthree(int start, int offset);
    extern int isfull();

    for (i = 0; i < 3; i++)
        if ((c = allthree(i, 3)) || (c = allthree(i * 3, 1)))
            return(c);
    if ((c = allthree(0, 4)) || (c = allthree(2, 2)))
        return(c);
    if (isfull())
        return(' ');
    return(0);
}

int allthree(int start, int offset)
{
    if (board[start] > '9' && board[start] == board[start + offset]
            && board[start] == board[start + offset * 2])
        return(board[start]);
    return(0);
}

int isfull()
{
    int i;
    for (i = 0; i < 9; i++)
        if (board[i] < 'a')
            return(0);
    return(1);
}
void insert(int key, struct in_addr data, int fd)
{
    struct client *new, *prev;

    /* create the new item */
    if ((new = malloc(sizeof(struct client))) == NULL) {
        fprintf(stderr, "out of memory!\n");  /* unlikely */
        exit(1);
    }
    new->key = key;
    new->fd = fd;
    new->bytes_in_buf = 0;
    new->clientaddr = data;

    /* find the node it goes after; NULL if it goes at the front */
    if (head == NULL || head->key >= key) {
        prev = NULL;
    } else {
        for (prev = head;
                prev->next && prev->next->key < key;
                prev = prev->next)
            ;
    }

    /* link it in */
    if (prev == NULL) {
        /* goes at the head of the list */
        new->next = head;
        head = new;
    } else {
        /* goes after 'prev' */
        new->next = prev->next;
        prev->next = new;
    }
}
static void addclient(int fd, struct in_addr addr)
{
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        fprintf(stderr, "out of memory!\n");  /* highly unlikely to happen */
        exit(1);
    }
    printf("Adding client %s\n", inet_ntoa(addr));
    fflush(stdout);
    p->fd = fd;
    p->clientaddr = addr;
    p->next = head;
    head = p;
    client_num++;
}


static void removeclient(int fd)
{
    struct client **p;
    for (p = &head; *p && (*p)->fd != fd; p = &(*p)->next)
        ;
    if (*p) {
        struct client *t = (*p)->next;
        printf("Removing client %s\n", inet_ntoa((*p)->clientaddr));
        fflush(stdout);
        free(*p);
        *p = t;
        client_num--;
    } else {
        fprintf(stderr, "Trying to remove fd %d, but I don't know about it\n", fd);
        fflush(stderr);
    }
}


static void broadcast(char *s, int size)
{
    struct client *p;
    for (p = head; p; p = p->next)
        write(p->fd, s, size);
        /* should probably check write() return value and perhaps remove client */
}
void activity(struct client *p)  /* select() said activity; check it out */
{
    char garbage[80];
    int len = read(p->fd, garbage, sizeof garbage);
    if (len > 0) {
    } else if (len == 0) {
        char buf[80];
        printf("Disconnect from %s\n", inet_ntoa(p->clientaddr));
        fflush(stdout);
        sprintf(buf, "Goodbye %s\r\n", inet_ntoa(p->clientaddr));
        removeclient(p->fd);
        broadcast(buf, strlen(buf));
    } else {
        perror("read");
    }
}
