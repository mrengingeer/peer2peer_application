#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/signal.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>

#define BUFLEN	100		/* buffer length */
#define NAMESIZ 20
#define MAXCON	200

typedef struct 
{
char 	type;
char	data[BUFLEN];
} PDU;	

struct {
	int	val;
	char 	name[NAMESIZ];
} table[MAXCON];  //Keep Track of the registered content

void 	registration(int, int, char *, struct sockaddr_in); 
void 	search_content(int, char *, PDU *);
int		client_download(int);
void 	server_download(int, int, int, char *, int, struct sockaddr_in, char *, struct sockaddr_in);
void 	deregistration(int, int, char *);
void	online_list(int);
void	local_list();
void	quit(int);
void	handler();
void	reaper(int);

int main(int argc, char **argv)
{
	PDU rpdu;
	char usr[NAMESIZ]; // User name of peer
	
	char *host = "localhost"; /* default host ip */ 
	char p_host[20]; /* peer content host ip */
	int s_port = 3000; /* default server port */
	int p_port; /* peer content host port */
	int s_sock, p_sock, s_type, p_type, d_sock; /* socket descriptor and socket type	*/
	
	int fd, nfds; // file descriptor and number of fds
	fd_set	rfds, afds;
	
	int n, new_sd, rdfs;
	int alen = sizeof(struct sockaddr_in);
	
	char buf[BUFLEN];		/* 32-bit integer to hold user inputs	*/ 
	struct hostent	*hp;	/* pointer to host information entry	*/
	struct sockaddr_in server, client, reg_addr, p_server;	/* an Internet endpoint address */
	char name[NAMESIZ];

	
	switch (argc) {
	case 1:
		break;
	case 2:
		host = argv[1];
	case 3:
		host = argv[1];
		s_port = atoi(argv[2]);
		break;
	default:
		fprintf(stderr, "usage: UDPtime [host [port]]\n");
		exit(1);
	}

	memset(&server, 0, alen);
	server.sin_family = AF_INET;                                                                
	server.sin_port = htons(s_port);
		                                                                        
	/* Map host name to IP address, allowing for dotted decimal */
	if ( hp = gethostbyname(host) ){
		memcpy(&server.sin_addr, hp->h_addr, hp->h_length);
	}
	else if ( (server.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
		fprintf(stderr, "Can't get host entry \n");
		                                                                
	/* Allocate a socket for UDP index server and connection of socket*/
	s_sock = socket(AF_INET, SOCK_DGRAM, 0); // UDP socket
	p_sock = socket(AF_INET, SOCK_STREAM,0); // TCP server socket
	d_sock = socket(AF_INET, SOCK_STREAM,0); // TCP socket
	if (s_sock < 0){
		fprintf(stderr, "Can't create socket \n");
		exit(1);
	}
        if (connect(s_sock, (struct sockaddr *)&server, sizeof(server)) < 0){
		fprintf(stderr, "Can't connect to %s \n", host);
		exit(1);
	}
	
	/* Open TCP socket for other peers */
	reg_addr.sin_family = AF_INET; 
	reg_addr.sin_port = htons(0); // TCP dynamically allocates port number
	reg_addr.sin_addr.s_addr = htonl(INADDR_ANY); 
	bind (p_sock,(struct sockaddr*)&reg_addr,sizeof(reg_addr));
	listen(p_sock, 5);
	(void) signal(SIGCHLD, reaper);
        
        /* Enter Username */
    printf("Enter a user name:\n");
	n = read(0, usr, NAMESIZ);
	usr[n-1] = '\0';
	PDU spdu, dpdu;
        
        printf("Command:\n");

    while(1){
	/* Initialization of Select and Table structure */
	FD_ZERO(&afds);
	FD_ZERO(&rfds);
	FD_SET(p_sock, &afds); /* Listening on the TCP socket  */
	FD_SET(0, &afds);    /* Listening on stdin from terminal */
	memcpy(&rfds, &afds, sizeof(rdfs));
	select(FD_SETSIZE, &rfds, NULL, NULL, NULL);
	if (FD_ISSET(0, &rfds)){ // Selecting stdin/*
		n = read(0, buf, BUFLEN);
		buf[n-1] = '\0';

/*	Command options	*/
	    if(strcmp(buf, "?") == 0){
		printf("R-Content Registration; T-Content Deregistration; L-List Local Content\n");
       	printf("D-Download Content; O-List all the On-line Content; Q-Quit\n\n");
	    }

/*	Content Regisration	*/
    	if(strcmp(buf, "R") == 0){
    		registration(s_sock, p_sock, usr, reg_addr);
  	    }

/*	List on-line Content	*/
	    if(strcmp(buf, "O") == 0){
	    	online_list(s_sock);
		}

/*	Download Content	*/
  	    if(strcmp(buf, "D") == 0){
  	    	server_download(s_sock, d_sock, p_sock, p_host, p_port, p_server, usr, reg_addr);
	      }

/*	Content Deregistration	*/
	    if(strcmp(buf, "T") == 0){
	    	deregistration(s_sock, p_sock, usr);
		}

/*	Quit	*/
	    if(strcmp(buf, "Q") == 0){
	    	quit(s_sock);
	  	}

	    printf("Command:\n");
  	  }


/* Content transfer: Server to client		*/
  	else {
		new_sd = accept(p_sock,(struct sockaddr *)&client, &alen);
		if (new_sd < 0){
				fprintf(stderr, "Can't accept client\n");
		}
		else{
			switch (fork()){
			case 0: /* child process */
				(void) close(p_sock);
				exit(client_download(new_sd));
			default: /* parent process */
				(void) close(new_sd);
			}
		}
	}
}
}


void	quit(int s_sock)
{
	/* De-register all the registrations in the index server	*/
	PDU spdu;

	spdu.type = 'Q';
	strcpy (spdu.data, "quit");
	write(s_sock, &spdu, BUFLEN);
	exit(0);
}

void	online_list(int s_sock)
{
	/* Contact index server to acquire the list of content */
	PDU spdu, rpdu;
	int i, n_cont;

	spdu.type = 'O';
	bzero(spdu.data, BUFLEN);
	strcpy(spdu.data, "list request");
	write(s_sock, &spdu, BUFLEN);
	read(s_sock, &rpdu, BUFLEN);
	printf("%s files found:\n", rpdu.data);
	n_cont = atoi(rpdu.data);
	for (i=0; i<n_cont; i++) {
		bzero(rpdu.data, BUFLEN);
		read(s_sock, &rpdu, BUFLEN);
		printf("%s\n", rpdu.data);
	}
}

void	server_download(int s_sock, int d_sock, int p_sock, char p_host[], int p_port, struct sockaddr_in p_server, char *name, struct sockaddr_in reg_addr)
{
	/* Respond to the download request from a peer	*/
	PDU dpdu, rpdu, spdu;
	char filename[20];
	int n, alen;

	dpdu.type = 'D';
	printf("Enter the name of the content:\n");
	n = read(0, dpdu.data,100);
	dpdu.data[n-1] = '\0';
	strcpy(filename, dpdu.data);
	write(s_sock, &dpdu, BUFLEN); // Send request to server
	read(s_sock, &rpdu, BUFLEN); // Waiting for acknowledgement
	printf("%c %s\n", rpdu.type, rpdu.data);
	if (rpdu.type == 'A'){ // Registered file exists
		read(s_sock, &spdu, sizeof(struct sockaddr_in)); // peer server address 
		strcpy(p_host, spdu.data); 
		bzero(spdu.data, BUFLEN);
		read(s_sock, &spdu, BUFLEN); // peer server port
		p_port = atoi(spdu.data);
		
		// Connecting to peer content host
		bzero((char *)&p_server, sizeof(struct sockaddr_in));
		p_server.sin_family = AF_INET;
		p_server.sin_port = p_port;
		p_server.sin_addr.s_addr = atoi(p_host);
		printf("IP address of host : %s:%d\n", inet_ntoa(p_server.sin_addr), ntohs(p_server.sin_port));
		if (connect(d_sock, (struct sockaddr *)&p_server, sizeof(p_server)) == -1){
			fprintf(stderr, "Can't connect \n");
		}
		else{
			// Downloads Data
			spdu.type = 'C';
			strcpy(spdu.data, filename);
			write(d_sock, &spdu, BUFLEN); // Sending filename
			int f_data = 0;
			FILE *fp;
			fp = fopen(filename, "w");
			printf("Downloading Content...\n");
			PDU rdata; //received data
			while (f_data == 0){
				bzero(rdata.data,100);
				n = read(d_sock, &rdata, 100);
				if (n < 0)
					fprintf(stderr, "Read failed\n");
				if (rdata.type == 'F')
					f_data = 1;
				fputs(rdata.data,fp);
			}
			printf("Download Completed Successfully.\n");
			fclose(fp);

			alen = sizeof(struct sockaddr_in);
			getsockname(p_sock,(struct sockaddr *)&reg_addr, &alen);
			spdu.type = 'R';
			bzero(spdu.data, 100);
			strcpy (spdu.data, name);
			write(s_sock, &spdu, BUFLEN); 				// username
			write(s_sock, &filename, BUFLEN); 				// file name
			write(s_sock, &reg_addr.sin_port, BUFLEN); // port number
		}
	}	
}

int client_download(int sd)
{
	/* Make TCP connection with the content server to initiate the
	   Download.	*/
	PDU rpdu, sdata;
	int n, i;
	n = read(sd, &rpdu, BUFLEN); // recieving filename
	FILE *fp;
	fp = fopen(rpdu.data, "r");
	struct stat test;
	int fd = fileno(fp);
	fstat(fd, &test);
	off_t size = test.st_size;
	int n_sends = size/99;
	int f_bytes =  size%99;
	if (f_bytes > 0)
		n_sends++;
	else
		f_bytes = 100;
	for(i=0; i<n_sends-1; i++){
		sdata.type = 'D';
		bzero(sdata.data,100);
		fread(sdata.data, 1, 99, fp);
		write(sd, &sdata, BUFLEN);
	}
	sdata.type = 'F';
	bzero(sdata.data,100);
	fread(sdata.data, 1, f_bytes, fp);
	write(sd, &sdata, BUFLEN);
	printf("Upload Completed Successfully.\n"); /* File Uploaded */
	fclose(fp);

}

void deregistration(int s_sock, int p_sock, char *name)
{
 	/* Contact the index server to deregister a content registration;	   Update nfds. */
	char buf[BUFLEN];
	PDU spdu, rpdu;
	int n;

	printf("Enter the name of the content:\n");
	bzero(buf,BUFLEN);
	n = read(0, buf,100);
	buf[n-1] = '\0';
	spdu.type = 'T';
	strcpy (spdu.data, name);
	write(s_sock, &spdu, BUFLEN);
	write(s_sock, &buf, BUFLEN);
	read(s_sock, &rpdu, BUFLEN);
	printf("%s\n", rpdu.data);
}

void registration(int s_sock, int p_sock, char *name, struct sockaddr_in reg_addr)
{
	/* Create a TCP socket for content download 
			– one socket per content;
	   Register the content to the index server;
	   Update nfds;	*/
	char buf[BUFLEN];
	PDU spdu;
	FILE *fp;
	int n, alen;

	printf("Enter the name of the content:\n");
	bzero(buf,BUFLEN);
	n = read(0, buf,100);
	buf[n-1] = '\0';

	fp = fopen(buf, "r");
	if (fp == NULL) {
		fprintf(stderr, "File not found.\n");
	}
	else {
		alen = sizeof(struct sockaddr_in);
		getsockname(p_sock,(struct sockaddr *)&reg_addr, &alen);
		spdu.type = 'R';
		strcpy (spdu.data, name);
		write(s_sock, &spdu, BUFLEN); 				// username
		write(s_sock, &buf, BUFLEN); 				// file name
		write(s_sock, &reg_addr.sin_port, BUFLEN); // port number
	}
}

void reaper(int sig){
	int status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}

