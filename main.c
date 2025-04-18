#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>


void res_200(int sockfd){

	char* buff ="HTTP/1.1 200 OK\r\n\r\n";
	
	if((write(sockfd, buff, strlen(buff))) < 0)
		close(sockfd);	
}




void res_404(int sockfd){

	char* buff ="HTTP/1.1 404 Not Found\r\n\r\n";
	
	if((write(sockfd, buff, strlen(buff))) < 0)
		close(sockfd);


}






void process(int sockfd){
	char buf[1024];
	char res[2048];
	char header_buf[2048];
	char* saveptr;

	int nb_readed = read(sockfd,buf, 1023);
	buf[nb_readed] = '\0';
	char *line = strtok_r(buf,"\r\n", &saveptr);
	char req[10], path[1024], vers[20];
	sscanf(line,"%s %s %s", req, path, vers);

	char ua_buf[1024];
	char* ptoh ;
	char* ptou;
	int iter = 0;
	char* header;
	char* po;
	char* p;
	if( (p = strstr(path, "/echo/")) != NULL){
		
		char* r = p+6;
		int lenght = strlen(r);
		char echoed[1000];
		strcpy(echoed, r);
		sprintf(res ,"HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %d\r\n\r\n%s",lenght, echoed);
		write(sockfd, res,strlen(res));
	}	

	else if( (strcmp(path, "/user-agent")) == 0){
		char *user_agent = NULL;
	        char *header;
        
        
        while ((header = strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
            if (strncasecmp(header, "User-Agent:", 11) == 0) {
                user_agent = header + 11;  
                while (*user_agent == ' ') {
                    user_agent++;
                }
                break;
            }
        }
        
        if (user_agent) {
	     sprintf(res, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n%s",
                    strlen(user_agent), user_agent);
            
            write(sockfd, res, strlen(res));
        } else {
          
            res_404(sockfd);
        }	 
		
	}


	else if(strcmp(path, "/") == 0){
		res_200(sockfd);
		}
	else{
		res_404(sockfd);
	}


}




int main() {
    int client_fd;
    fd_set master, readset;
    FD_ZERO(&master);
    FD_ZERO(&readset);

    printf("Logs from your program will appear here!\n");

    int server_fd, client_addr_len;
    struct sockaddr_in client_addr;
    client_addr_len = sizeof(client_addr);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("socket");
        return 1;
    }

    int reuse = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_port = htons(4221),
        .sin_addr = { htonl(INADDR_ANY) },
    };

    if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
        perror("bind");
        return 1;
    }

    if (listen(server_fd, 10) != 0) {
        perror("listen");
        return 1;
    }

    FD_SET(server_fd, &master);
    int maxfd = server_fd;

    for (;;) {
        readset = master;
        if (select(maxfd + 1, &readset, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(1);
        }

        for (int i = 0; i <= maxfd; i++) {
            if (FD_ISSET(i, &readset)) {
                if (i == server_fd) {
                    client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
                    if (client_fd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(client_fd, &master);
                        if (client_fd > maxfd) maxfd = client_fd;
                    }
                } else {
         
                    process(i);
                    close(i);
                    FD_CLR(i, &master);
                }
            }
        }
    }

    close(server_fd);
    return 0;
}



