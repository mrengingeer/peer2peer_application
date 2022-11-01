/* time_server.c - main */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <arpa/inet.h>

#define BUFSIZE 100
#define EMSG "Cannot find content"
#define NAMESIZ 20
#define MAXCONT 200


typedef struct pdu{
	char type;
	char data[BUFSIZE];
}PDU;                                                               
PDU rpdu;

struct {
	char usr[NAMESIZ]; /* Username of host */
	char name[NAMESIZ];
	int addr;
	int port;
} list[MAXCONT];

char user[NAMESIZ];
char fname[NAMESIZ];
/*------------------------------------------------------------------------
 * main - Iterative UDP server
 *------------------------------------------------------------------------
 */
int
main(int argc, char *argv[])
{
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[100];		/* "input" buffer; any size > 0	*/
	char    *pts;
	int	sock;			/* server socket		*/
	time_t	now;			/* current time			*/
	int	alen;			/* from-address length		*/
	struct  sockaddr_in sin; /* an Internet endpoint address         */
        int     s, type;        /* socket descriptor and socket type    */
	int 	port=3000;
	int	n_cont = 0;	/* number of registered content */
	int i, j, done;
        
        
	switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}

        memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't creat socket\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);
	
	
	//printf("S_IP - %d:%d\n", sin.sin_addr.s_addr, sin.sin_port);

	while (1) {
		PDU spdu, rpdu;
		recvfrom(s, &spdu, BUFSIZE, 0, (struct sockaddr *)&fsin, &alen);
		printf("%s\n",spdu.data);

		if (spdu.type == 'R'){ //Content Registration
			strcpy (list[n_cont].usr, spdu.data); // Received username 1st
			recvfrom(s, &list[n_cont].name, BUFSIZE, 0, (struct sockaddr *)&fsin, &alen); // content name
			recvfrom(s, &list[n_cont].port, BUFSIZE, 0, (struct sockaddr *)&fsin, &alen); // port
			list[n_cont].addr = fsin.sin_addr.s_addr; // ip address in dot format -> inet_ntoa(fsin.sin_addr)
			printf("Owner: %s\nFile: %s\nAddress: %d:%d\n", list[n_cont].usr, list[n_cont].name, list[n_cont].addr, list[n_cont].port);
			
			n_cont++;
			
			
		}
		else if(spdu.type == 'D'){ // Content Download Request
			done = 0;
			for (i=0; i<n_cont; i++){
				if (strcmp(list[i].name, spdu.data) == 0){
					done = 1;
					spdu.type = 'A';
					strcpy(spdu.data,"registered content found.\n");
					(void)sendto(s, &spdu, strlen(spdu.data), 0, 
						(struct sockaddr *)&fsin, sizeof(fsin)); // Sent acknowledgement
					bzero(spdu.data, BUFSIZE);
					spdu.type = 'D';
					sprintf(spdu.data, "%d", list[i].addr);
					(void)sendto(s,&spdu, BUFSIZE, 0, 
						(struct sockaddr *)&fsin, sizeof(fsin)); // Sending peer server address
					bzero(spdu.data, BUFSIZE);
					sprintf(spdu.data, "%d", list[i].port);
					(void)sendto(s,&spdu , BUFSIZE, 0, 
						(struct sockaddr *)&fsin, sizeof(fsin)); // Sending peer server port
					break;
				}	
			}
			if (done == 0){
				spdu.type = 'E';
				strcpy(spdu.data,"Registered content not found");
				(void)sendto(s,&spdu, BUFSIZE, 0,
					(struct sockaddr *)&fsin, sizeof(fsin));
			}
			
		}
		else if(spdu.type == 'T'){ // Content De-Registration
			strcpy(user,spdu.data);
			recvfrom(s, &fname, BUFSIZE, 0, (struct sockaddr *)&fsin, &alen);
			bzero(spdu.data, BUFSIZE);
			if(n_cont != 0) {
				for (i=0; i<n_cont; i++){
					if (strcmp(list[i].usr, user) == 0){
						if (strcmp(list[i].name, fname) == 0) {
							for(j = i; j < n_cont - 1; j++) {
								list[j] = list[j + 1];
							}
							strcpy(list[n_cont-1].usr, "\0");
							strcpy(list[n_cont-1].name, "\0");
							list[n_cont-1].addr = '\0';
							list[n_cont-1].port = '\0';
							n_cont--;
							spdu.type = 'T';
							sprintf(spdu.data, " File: %s deregistered successfully.\n", fname);
						}
					}
					else {
						spdu.type = 'E';
						strcpy(spdu.data, " Error: User not found.");
					}
				}
			}
			else {
				spdu.type = 'E';
				strcpy(spdu.data, " Error: There is no content registered.");
			}
			(void)sendto(s, &spdu.data, strlen(spdu.data), 0, 
						(struct sockaddr *)&fsin, sizeof(fsin));
		}

		else if(spdu.type == 'O'){ // Content Data
			spdu.type = 'O';
			bzero(spdu.data, BUFSIZE);
			sprintf(spdu.data, "%d", n_cont);
			(void)sendto(s, &spdu, BUFSIZE, 0,
					(struct sockaddr *)&fsin, sizeof(fsin));
			for (i=0; i<n_cont; i++) {
				spdu.type = 'O';
				bzero(spdu.data, BUFSIZE);
				sprintf(spdu.data, "Owner: %s, File: %s, Address: %d:%d\n", list[i].usr, list[i].name, list[i].addr, list[i].port);
				(void)sendto(s, &spdu, BUFSIZE, 0,
					(struct sockaddr *)&fsin, sizeof(fsin));
			}
		}

		else if(spdu.type == 'Q'){ // Quit
			for(i = 0; i < n_cont; i++) {
				strcpy(list[n_cont-1].usr, "\0");
				strcpy(list[n_cont-1].name, "\0");
				list[n_cont-1].addr = '\0';
				list[n_cont-1].port = '\0';
				n_cont--;
			}
		}
		
	}
}


