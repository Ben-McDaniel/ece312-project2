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
// #define MESSAGE "0900760468691892"
#define PORT 2324
#define BUFSIZE 1024
// #define VERSION 9
// #define commID_RHMP 0x1874
// #define commID_CTLMSG 118

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

    /* send a message to the server */
    // printf("Sending message to server: %s",(char)strtol(MESSAGE, NULL, 16));
    // printf("%s",package_message(MESSAGE));


    //hard coded message for milestone 1
    uint8_t tmp[] = {0x09, 0xA8, 0x00, 0x05, 0x68, 0x65, 0x6C, 0x6C, 0x6F, 0x00, 0xB2, 0x80};
    

    if (sendto(clientSocket, tmp, sizeof(tmp), 0,
            (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
    }
    // free(message);
    /* Receive message from server */
    nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);

    printf("Received from server: %s\n", buffer);
    unpackage_message(buffer);
    displayReceived(buffer, sizeof(buffer));

    close(clientSocket);
    return 0;
}

char* package_message(char* message){
    return message;
}



char* bufferToHexString(char* buffer, int bufferSize) {
    int resultSize = bufferSize * 2 + 1; 

    char *result = (char*)malloc(resultSize * sizeof(char));

    for (int i = 0; i < bufferSize; i++) {
        sprintf(result + i * 2, "%02X", buffer[i]);
    }


    result[resultSize - 1] = '\0';

    return result;
}



void unpackage_message(char message[]){
    //dont need to copy the buffer into new place?
    // size_t length = strlen(message);
    // char* result = (char*)malloc((length + 1) * sizeof(char));
    int bufferSize = sizeof(message) * 10 -1;

    // Convert the buffer to a hex string
    char* hexString = bufferToHexString(message, bufferSize);
    printf("Hex string: %s\n", hexString);

    printf("Message Recieved:\n");
    char version[2] = {hexString[0], hexString[1]};
    printf("RHP Version: %02X\n", (int)version);

    int length = 4;
    for(int i = 8; i < 8+length; i+=2){
        char hexPair[2] = {hexString[i], hexString[i + 1]};
        char asciiChar = strtol(hexPair, NULL, 16);


        printf("%c",(char)asciiChar);
    }
    free(hexString);
}



int getTwo(char message[], int index) {
    char tmp[17]; //+1 for /0

    for (int i = 0; i < 16; i++) {
        tmp[i] = message[index + i];
    }
    tmp[16] = '\0';

    return (int)strtol(tmp, NULL, 2);
}

int checkSum(char message[], int size) {
    uint16_t runningSum = 0;
    uint16_t previousSum = 0;

    for (int i = 0; i < size; i = i + 16) {
        runningSum += getTwo(message, i);
        if (previousSum > runningSum) {
            runningSum++;
        }
        previousSum = runningSum;
    }

    runningSum = ~runningSum;
    return runningSum;
}


void displayReceived(char* message, int bufferSize) {
    char parseVersion[8];
    char parseCommID[16];
    char parseLength[7];
    char parseCheckSum[16];

    for(int i = 0; i < 8; i++) { //For version (bits 0-7)
        parseVersion[i] = message[i];
    }
    for(int i = 8; i < 24; i++) { //For commID (bits 8-23)
        parseCommID[i - 8] = message[i];
    }
    for(int i = 24; i < 31; i++) { //For length (of payload) (bits 24-30)
        parseLength[i - 24] = message[i];
    }
    int parseLengthInt = (int)strtol(parseLength, NULL, 16); //Converting the char[<hex>] to long, to int
    char parsePayload[parseLengthInt];
    for(int i = 32; i < 31 + parseLength; i++) { //For payload (bits 32 - (31 + length))
        parsePayload[i - 32] = message[i];
    }
    for(int i = bufferSize - 16; i < bufferSize; i++) { //For checksum (last 16 bits)
        parseCheckSum[i - bufferSize - 16] = message[i];
    }

    int parseVersionInt = strtol(parseVersion, NULL, 16);
    int parseCommIDInt = strtol(parseCommID, NULL, 16);
    int parseCheckSumInt = strtol(parseCheckSum, NULL, 16);
    int parsePayloadInt = strtol(parsePayload, NULL, 16);

    printf("Message Received:\nRHP Verion: %d\nCommID: %d\nlength: %d\nchecksum: 0x%x", parseVersionInt, parseCommIDInt, parseLengthInt, parseCheckSumInt);
}