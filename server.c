

/*Included libraries*/

#include <stdio.h>	  /* for printf() and fprintf() */
#include <sys/socket.h>	  /* for socket(), connect(), send(), and recv() */
#include <arpa/inet.h>	  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>	  /* supports all sorts of functionality */
#include <unistd.h>	  /* for close() */
#include <string.h>	  /* support any string ops */
#include <stdint.h>
#include <time.h>
#include <limits.h>

#define RCVBUFSIZE 512		/* The receive buffer size */
#define SNDBUFSIZE 512		/* The send buffer size */
#define BUFSIZE 40		/* Your name can be as many as 40 chars*/


/* The main function */
int main(int argc, char *argv[])
{

    int serverSock;				/* Server Socket */
    int clientSock;				/* Client Socket */
    struct sockaddr_in changeServAddr;		/* Local address */
    struct sockaddr_in changeClntAddr;		/* Client address */
    unsigned short changeServPort;		/* Server port */
    unsigned int clntLen;			/* Length of address data struct */

    char nameBuf[BUFSIZE];			/* Buff to store account name from client */
    int32_t  balance;				/* Place to record account balance result */

    char recvBuf[BUFSIZE];
    char sendBuf[BUFSIZE];

    char toAccountName[15];
    char fromAccountName[15];

    char *servIP;
    char servIPHolder[18] = {0};
    int servPort;
    char servPortStrHolder[6] = {0};

    char *accountNames[5];
    accountNames[0] = "mySavings";
    accountNames[1] = "myChecking";
    accountNames[2] = "myCD";
    accountNames[3] = "my401k";
    accountNames[4] = "my529";

    int32_t balances[5];
    balances[0] = 14;
    balances[1] = 149;
    balances[2] = 250;
    balances[3] = 500;
    balances[4] = 1000000000;

    /* Get the additional parameters from the command line */
    if (argc != 3) {
        printf("Incorrect number of arguments. The correct format is:\n\tserverIP serverPort");
        exit(-1);
    }
    servIP = argv[1];
    servPort = atoi(argv[2]);

    /* Create new TCP Socket for incoming requests*/
    /*	    FILL IN	*/
    serverSock = socket(PF_INET, SOCK_STREAM, 0);
    if (serverSock < 0) {
        printf("Server socket creation failed");
        exit(-1);
    }

    /* Construct local address structure*/
    /*	    FILL IN	*/
    changeServAddr.sin_family = AF_INET;
    changeServAddr.sin_port = servPort;
    struct in_addr serv_addr = {0};
    int in_addr_status = inet_aton(servIP, &serv_addr);
    changeServAddr.sin_addr = serv_addr;

    /* Bind to local address structure */
    /*	    FILL IN	*/
    printf("Beginning bind\n");
    int bind_return = -1;
    while (bind_return == -1) {
        bind_return = bind(serverSock, (struct sockaddr *)&changeServAddr, sizeof(changeServAddr));
        if (bind_return < 0) {
            printf("Server socket address binding failed\n");
            printf("Enter a new IP address: \n");
            fgets(servIPHolder, 18, stdin);
            printf("Enter a new port: \n");
            fgets(servPortStrHolder, 6, stdin);
            servPort = atoi(servPortStrHolder);
            changeServAddr.sin_port = servPort;
            inet_aton(servIPHolder, &serv_addr);
            changeServAddr.sin_addr = serv_addr;
        }
    }
    /* Listen for incoming connections */
    /*	    FILL IN	*/
    int listen_return = listen(serverSock, SOMAXCONN);
    if (listen_return < 0) {
        printf("Server socket listen call failed\n");
    }
    /* Loop server forever*/
    printf("Beginning loop\n");
    time_t times[3] = {0};
    int tracker = 0;
    while(1)
    {
	    /* Accept incoming connection */
	    /*	FILL IN	    */
        clntLen = sizeof(changeClntAddr);
        printf("Beginning accept\n");
        clientSock = accept(serverSock, (struct sockaddr*)(&changeClntAddr), (socklen_t*)&clntLen);
        if (clientSock == -1) {
            printf("Socket acception failed\n");
            return -1;
        }
	    /* Extract the account name from the packet, store in nameBuf */
	    /* Look up account balance, store in balance */
	    /*	FILL IN	    */
        printf("Beginning receive\n");
        int message_length = recv(clientSock, recvBuf, 10, 0); //receiving command, decide how to receive next messages based on that
        printf("%s\n", recvBuf);
        strcpy(nameBuf, recvBuf); //command in namebuf
        memset(recvBuf, 0, BUFSIZE); //recvBuf set to 0
        if (strcmp(nameBuf, "BAL") == 0) {
            recv(clientSock, recvBuf, 15, 0); 
            memset(nameBuf, 0, BUFSIZE);
            memcpy(nameBuf, recvBuf, 15);
            int found = 0;
            for (int i = 0; i < 5 && !found; i++) {
                if (strcmp(accountNames[i], nameBuf) == 0) {
                    found = 1;
                    balance = balances[i];
                }
            }
            if (!found) {
                printf("Account not found\n");
                balance = -1;
            } else {
                balance = htonl(balance);
            }
            memcpy(sendBuf, (void *)&balance, sizeof(int32_t));
            send(clientSock, sendBuf, sizeof(int32_t), 0);
        } else if (strcmp(nameBuf, "WITHDRAW") == 0) {
            int accountName_recv_status = recv(clientSock, recvBuf, 15, 0);
            memset(nameBuf, 0, BUFSIZE);
            memcpy(nameBuf, recvBuf, 15);
            memset(recvBuf, 0, BUFSIZE);
            int amount_recv_status = recv(clientSock, recvBuf, sizeof(int32_t), 0);
            int32_t amount = ntohl(*(int32_t *)recvBuf);
            int exceededWindow = 0;
            time_t timeVal = time(0);
            time_t timeValCurrent;
            time_t timeValSmallest = (time_t)LONG_MAX; //start at largest value possible
            int smallestIndex = 0;
            if (tracker < 3) { //3 requests haven't come yet, rather just fill the array first than deal with corner cases
                times[tracker] = timeVal;
                tracker++;
            } else {
                for (int k = 0; k < 3; k++) { //array is full, find oldest value, which should be the smallest
                    timeValCurrent = times[k];
                    if (timeValCurrent < timeValSmallest) {
                        timeValSmallest = timeValCurrent;
                        smallestIndex = k;
                    }
                }
                double diff = difftime (timeVal, timeValSmallest);
                if (diff < 60.0) {
                    exceededWindow = 1;
                } else {
                    times[smallestIndex] = timeVal;
                }
            }
            int32_t withdrawAmount = -1;
            if (exceededWindow) {
                withdrawAmount = -2; //-2 means there were too many withdrawals within a minute
            } else {
                int found = 0;
                int foundIndex = 0;
                for (int i = 0; i < 5 && !found; i++) {
                    if (strcmp(accountNames[i], nameBuf) == 0) {
                        found = 1;
                        foundIndex = i;
                        balance = balances[i];
                    }
                }
                if (!found) {
                    withdrawAmount = -5;
                } else {
                    memset(recvBuf, 0, BUFSIZE);
                    withdrawAmount = -1;
                    if (balances[foundIndex] >= amount) {
                        balances[foundIndex] -= amount;
                        withdrawAmount = amount;
                    }
                }
            }
            withdrawAmount = htonl(withdrawAmount);
            memcpy(sendBuf, (void *)&withdrawAmount, sizeof(int32_t));
            int withdrawAmount_send_status = send(clientSock, sendBuf, sizeof(int32_t), 0);
        } else if (strcmp(nameBuf, "TRANSFER") == 0) {
            int fromAccountName_recv_status = recv(clientSock, recvBuf, 15, 0);
            memcpy(fromAccountName, recvBuf, 15);
            memset(recvBuf, 0, BUFSIZE);
            int toAccountName_recv_status = recv(clientSock, recvBuf, 15, 0);
            memcpy(toAccountName, recvBuf, 15);
            memset(recvBuf, 0, BUFSIZE);
            int amount_recv_status = recv(clientSock, recvBuf, sizeof(int32_t), 0);
            int32_t amount = ntohl(*(int32_t *)recvBuf);
            printf("From account: %s, To Account: %s, Transfer Amount: %d\n", fromAccountName, toAccountName, amount);
            int foundToAccount = 0;
            int foundToAccountIndex = 0;
            int foundFromAccount = 0;
            int foundFromtAcountIndex = 0;
            int32_t transferAmount = 0;
            for (int i = 0; i < 5; i++) {
                if (strcmp(toAccountName, accountNames[i]) == 0) {
                    foundToAccount = 1;
                    foundToAccountIndex = i;
                }
                if (strcmp(fromAccountName, accountNames[i]) == 0) {
                    foundFromAccount = 1;
                    foundFromtAcountIndex = i;
                }
            }
            if (!foundToAccount && !foundFromAccount) {
                printf("Neither account existed\n");
                transferAmount = -2; //-2 indicates neither exists
            } else if(!foundFromAccount) {
                printf("The fromAccount didn't exist\n");
                transferAmount = -3; //only fromAccount didn't exist
            } else if(!foundToAccount) {
                printf("The toAccount didn't exist\n");
                transferAmount = -4; //only toAccount didn't exist
            } else {
                if (balances[foundFromtAcountIndex] < amount) {
                    transferAmount = -5; //both accounts exist, not enough funds in from account
                } else { //do the transfer, transferAmount becomes amount
                    balances[foundFromtAcountIndex] -= amount;
                    balances[foundToAccountIndex] += amount;
                    transferAmount = amount;
                }
            }
            memset(sendBuf, 0 , BUFSIZE);
            memcpy(sendBuf, (void *)&transferAmount, sizeof(int32_t));
            int transferAmount_send_status = send(clientSock, sendBuf, sizeof(int32_t), 0);
        }
        
    }
}

