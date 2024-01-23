/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#define SERVER "137.112.38.47"
#define PORT 2324
#define BUFSIZE 1024


uint16_t fixed_checksum(char* buffer, int nBytes);
void displayMessage(char* buffer, int nBytes);
void printPayload(char* buffer, int length);
char* getNextMessage();
void messageSendSuccess();
void printPayloadRHMP(char* buffer, int length);

int messageCount = 1;

int main() {
    int clientSocket, nBytes;
    char buffer[BUFSIZE];
    struct sockaddr_in clientAddr, serverAddr;

    /*Create UDP socket*/
    if ((clientSocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("cannot create socket");
        return 0;
    }

    /* Bind to an arbitrary return address.
     * Because this is the client side, we don't care about the address 
     * since no application will initiate communication here - it will 
     * just send responses 
     * INADDR_ANY is the IP address and 0 is the port (allow OS to select port) 
     * htonl converts a long integer (e.g. address) to a network representation 
     * htons converts a short integer (e.g. port) to a network representation */
    memset((char *) &clientAddr, 0, sizeof (clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(0);

    if (bind(clientSocket, (struct sockaddr *) &clientAddr, sizeof (clientAddr)) < 0) {
        perror("bind failed");
        return 0;
    }

    /* Configure settings in server address struct */
    memset((char*) &serverAddr, 0, sizeof (serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER);
    memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);
    

    //only send 4 requests
    if(messageCount > 4){
        exit(0);
    }

    uint8_t msg1[] = {0x09, 0xA8, 0x00, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0xB2, 0x80};
    uint8_t msg2[] = {0x09, 0xA8, 0x00, 0x02, 0x68, 0x69, 0x8D, 0xEC};
    uint8_t msg3[] = {0x09, 0x74, 0x18, 0x41, 0x31, 0x20, 0x76, 0x18, 0x37, 0x12};
    uint8_t msg4[] = {0x09, 0x01, 0x18, 0x35, 0x31, 0x20, 0x76, 0x03, 0x37, 0xA6};


    //iterate through messages with each main call
    if(messageCount == 1){
        printf("Sending RHP Message: hello\n");
        if (sendto(clientSocket, msg1, sizeof(msg1), 0,
                (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
            perror("sendto failed");
            return 0;
        }

        /* Receive message from server */
        nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);
        displayMessage(buffer, nBytes);
    }else if(messageCount == 2){
        printf("Sending RHP Message: hi\n");
        if (sendto(clientSocket, msg2, sizeof(msg2), 0,
                (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
            perror("sendto failed");
            return 0;
        }


        /* Receive message from server */
        nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);
        displayMessage(buffer, nBytes);
    }else if(messageCount == 3){
        printf("Sending RHMP Message of type: Message_Request\n");
        if (sendto(clientSocket, msg3, sizeof(msg3), 0,
                (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
            perror("sendto failed");
            return 0;
        }


        /* Receive message from server */
        nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);
        displayMessage(buffer, nBytes);
    }else{
        printf("Sending RHMP Message of type: ID_Request\n");
                if (sendto(clientSocket, msg4, sizeof(msg4), 0,
                (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
            perror("sendto failed");
            return 0;
        }


        /* Receive message from server */
        nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);
        displayMessage(buffer, nBytes);
    }

    close(clientSocket);
    return 0;
}

void messageSendSuccess(){
    messageCount += 1;
}

void displayMessage(char* buffer, int nBytes){
    // for(int i = 0; i < nBytes; i+=2){
    //     printf("\n%x %x\n", (uint8_t)buffer[i],(uint8_t)buffer[i+1]);

    // }
    printf("==================================\n");
    printf("Message Recieved\n");
    printf("    RHP Version: %d\n",buffer[0]);
    uint16_t commID = (uint8_t)buffer[2]<<8;
    commID += (uint8_t)buffer[1];
    printf("    commID: %u\n",commID);

    uint8_t length = (uint8_t)buffer[3]&0b01111111;

    printf("    Length: %d\n", length);

    uint8_t type = (uint8_t)buffer[3]&0b10000000;
    printf("    Type: %d\n",type);

    if(type == 0){
        //RHP message
        printPayload(buffer, length);
    }else{
        //RHMP message
        printPayloadRHMP(buffer, length);
    }

    uint16_t messageChecksum = (uint16_t)buffer[nBytes-2]<<8;
    messageChecksum += (uint8_t)buffer[nBytes-1];
    printf("    Message Checksum: %d\n", messageChecksum);

    uint16_t checksum = fixed_checksum(buffer, nBytes);
    printf("    Calculated Checksum: %d", checksum);

    printf("\n==================================\n");

    if(checksum != messageChecksum){
        printf("Checksum Failed, resending message\n\n");
    }else{
        messageSendSuccess();
    }
    
    main();
}

void printPayload(char* buffer, int length){
    //known that message starts on buffer[4]
    printf("    Payload: ");
    for(int i = 0; i < length; i++){
        printf("%c", buffer[4+i]);
    }
    printf("\n");
}

void printPayloadRHMP(char* buffer, int length){
    //know that the payload starts on buffer[4]

    // for(int i = 0; i < length; i+=2){
    //     printf("\n%x %x\n", (uint8_t)buffer[i],(uint8_t)buffer[i+1]);
    // }
    printf("    RHMP Protocol:\n");

    int dstPort = (buffer[4]<<4) + (buffer[5]>>4);
    printf("        dstPort: %d\n", dstPort);
}

uint16_t fixed_checksum(char* buffer, int nBytes){
    uint16_t sum = 0;
    uint16_t pastSum = 0;
    for(int i = 0; i < nBytes-2; i+=2){
        sum += (uint8_t)buffer[i]<<8;
        sum += (uint8_t)buffer[i+1];
        if(sum < pastSum){
            sum += 1;
        }
        pastSum = sum;
    }

    sum = ~(uint16_t)sum;
    return sum;
}
