
/* Included libraries */

#include <stdio.h>		    /* for printf() and fprintf() */
#include <sys/socket.h>		    /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>		    /* for sockaddr_in and inet_addr() */
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>

/* Constants */
#define RCVBUFSIZE 512		    /* The receive buffer size */
#define SNDBUFSIZE 512		    /* The send buffer size */
#define REPLYLEN 32


/* The main function */
int main(int argc, char *argv[])
{

    int clientSock;		    /* socket descriptor */
    struct sockaddr_in serv_addr;   /* server address structure */

    char toAccountName[15];		    /* Account Name, also functions as to account in case of TRANSFER*/
    char fromAccountName[15];
    char command[10];
    int32_t sendBalance;

    char *servIP;		    /* Server IP address  */
    unsigned short servPort;	    /* Server Port number */


    char sndBuf[SNDBUFSIZE];	    /* Send Buffer */
    char rcvBuf[RCVBUFSIZE];	    /* Receive Buffer */

    int32_t balance;		    /* Account balance */
    /* Get the Account Name from the command line */
    if (argc < 3)
    {
	    printf("Invalid arguments. The format is\n\tServerIP, serverPort, command, command arguments\n");
    } else if (strcmp(argv[3], "BAL") == 0) {
        if (argc < 5) {
            printf("Invalid number of arguments to BAL command. Should be accountName\n");
            return -1;
        }
        strcpy(command, argv[3]);
        strcpy(toAccountName, argv[4]);
        if (strlen(toAccountName) + 1 > 15) {
        	printf("Account name too long. Max: 14 characters\n");
        }
    } else if (strcmp(argv[3], "WITHDRAW") == 0) {
        if (argc < 6) {
            printf("Invald number of arguments to WITHDRAW command. Should be accountName, amount\n");
            return -1;
        }
        strcpy(command, argv[3]);
        strcpy(toAccountName, argv[4]);
        if (strlen(toAccountName) + 1 > 15) {
        	printf("Account name too long. Max: 14 characters\n");
        }
        sendBalance = htonl((int32_t)atoi(argv[5]));
        if (sendBalance == 0) {
            printf("Invalid amount argument to WITHDRAW\n");
            return -1;
        }
    } else if (strcmp(argv[3], "TRANSFER") == 0) {
        if (argc < 7) {
            printf("Invalid number of arguments to TRANSFER command. Should be fromAccountName, toAccountName, amount\n");
            return -1;
        }
        strcpy(command, argv[3]);
        strcpy(fromAccountName, argv[4]);
        strcpy(toAccountName, argv[5]);
        if (strlen(toAccountName) + 1 > 15) {
        	printf("To account name too long. Max: 14 characters\n");
        }
        if (strlen(fromAccountName) + 1 > 15) {
        	printf("From account name too long. Max: 14 characters\n");
        }
        sendBalance = htonl((int32_t)atoi(argv[6]));
        if (sendBalance == 0) {
            printf("Invalid amount argument to TRANSFER");
        }
    } else {
        printf("That wasn't a valid command\n");
        return -1;
    }
    memset(&sndBuf, 0, SNDBUFSIZE);
    memset(&rcvBuf, 0, RCVBUFSIZE);

    /* Get the addditional parameters from the command line */
    /*	    FILL IN	*/
    servIP = argv[1];
    servPort = atoi(argv[2]);
    if (servPort < 0 || servPort > 65535) {
        printf("Invalid server port");
        return -1;
    }
    /* Create a new TCP socket*/
    /*	    FILL IN	*/
    clientSock = socket(PF_INET, SOCK_STREAM, 0);
    if (clientSock < 0) {
        printf("Client socket creation failed");
        return -1;
    }

    /* Construct the server address structure */
    /*	    FILL IN	 */
    struct in_addr addr = {0};
    int in_addr_status = inet_aton(servIP, &addr);
    if (!in_addr_status) {
        printf("Invalid server IP");
        return -1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = servPort;
    serv_addr.sin_addr = addr;

	

    /* Establish connecction to the server */
    /*	    FILL IN	 */
    int connection_status = connect(clientSock, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
    if (connection_status < 0) {
        printf("Client's connection to server failed");
        return -1;
    }
    /* Send the string to the server */
    /*	    FILL IN	 */
    strcpy(sndBuf, command);
    int message_send_status = send(clientSock, sndBuf, 10, 0); //send commnand so the server knows how to receive the following messages
    if (strcmp(command, "BAL") == 0) {
    	strcpy(sndBuf, toAccountName);
    	int message_send_status = send(clientSock, sndBuf, 15, 0); //send account name
    	int message_recv_status = recv(clientSock, rcvBuf, sizeof(int32_t), 0);
    	int32_t retVal = *(int32_t *)rcvBuf;
    	retVal = ntohl(retVal);
    	if (retVal == -1) {
    		printf("That account doesn't exist\n");
    	} else {
    		printf("Funds in account %s: $%d\n", toAccountName, retVal);
    	}
    } else if (strcmp(command, "WITHDRAW") == 0) {
    	strcpy(sndBuf, toAccountName);
    	int toAccountName_send_status = send(clientSock, sndBuf, 15, 0); //send account name
    	memcpy(sndBuf, (void *)&sendBalance, sizeof(int32_t));
    	int sendBalance_send_status = send(clientSock, sndBuf, sizeof(int32_t), 0); //send withdrawal amount
    	int sendBalance_recv_status = recv(clientSock, rcvBuf, sizeof(int32_t), 0); //receive withdrawal status, positive integer if valid, if not then invalid
    	int32_t withdrawalAmount = ntohl(*(int32_t *)rcvBuf);
    	if (withdrawalAmount > -1) {
    		printf("$%d withdrawn from account %s\n", withdrawalAmount, toAccountName);
    	} else  if (withdrawalAmount == -1) {
    		printf("Error: Insufficient funds!\n");
    	} else if (withdrawalAmount == -5) {
    		printf("Error: No such account!\n");
    	} else if (withdrawalAmount == -2) {
    		printf("Error: Too many withdrawals within a minute!\n");
    	} else {
    		printf("withdrawalAmount: %d\n", withdrawalAmount);
    	}
    } else if (strcmp(command, "TRANSFER") == 0) {
    	strcpy(sndBuf, fromAccountName);
    	int fromAccountName_send_status = send(clientSock, sndBuf, 15, 0); //send from name
    	memset(sndBuf, 0, SNDBUFSIZE);
    	strcpy(sndBuf, toAccountName);
    	int toAccountName_send_status = send(clientSock, sndBuf, 15, 0); //send to name
    	memset(sndBuf, 0, SNDBUFSIZE);
    	memcpy(sndBuf, (void *)&sendBalance, sizeof(int32_t));
    	int sendBalance_send_status = send(clientSock, sndBuf, sizeof(int32_t), 0); //send transfer amount
    	memset(rcvBuf, 0, RCVBUFSIZE);
    	int transferAmount_recv_status = recv(clientSock, rcvBuf, sizeof(int32_t), 0);
    	int32_t transferAmount = *(int32_t *)rcvBuf;
    	if (transferAmount >= 0) {
    		printf("$%d transfered from %s to %s\n", transferAmount, fromAccountName, toAccountName);
    	} else if (transferAmount == -2) {
    		printf("Neither account exists\n");
    	} else if (transferAmount == -3) {
    		printf ("The fromAccount didn't exist\n");
    	} else if (transferAmount == -4) {
    		printf("The toAccount didn't exist\n");
    	} else if (transferAmount == -5) {
    		printf("Error: Insufficient funds\n");
    	}
    }



    return 0;
}

