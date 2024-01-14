/************* UDP CLIENT CODE *******************/

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#define SERVER "137.112.38.47"
#define MESSAGE "0900760468691892"
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
    printf("Sending message to server: %s",MESSAGE);
    // printf("%s",package_message(MESSAGE));
    if (sendto(clientSocket, MESSAGE, strlen(MESSAGE), 0,
            (struct sockaddr *) &serverAddr, sizeof (serverAddr)) < 0) {
        perror("sendto failed");
        return 0;
    }

    /* Receive message from server */
    nBytes = recvfrom(clientSocket, buffer, BUFSIZE, 0, NULL, NULL);

    printf("Received from server: %s\n", buffer);
    unpackage_message(buffer);

    close(clientSocket);
    return 0;
}

char* package_message(char* message){
    return message;
}



char* bufferToHexString(char* buffer, int bufferSize) {
    // Calculate the length needed for the result array
    int resultSize = bufferSize * 2 + 1;  // Each byte requires 2 characters, plus one for the null terminator

    // Allocate memory for the result array
    char *result = (char*)malloc(resultSize * sizeof(char));

    // Convert each byte to hex and store in the result array
    for (int i = 0; i < bufferSize; i++) {
        sprintf(result + i * 2, "%02X", buffer[i]);
    }

    // Add null terminator
    result[resultSize - 1] = '\0';

    return result;
}



void unpackage_message(char message[]){
    //dont need to copy the buffer into new place?
    // size_t length = strlen(message);
    // char* result = (char*)malloc((length + 1) * sizeof(char));
    int bufferSize = sizeof(message) / sizeof(message[0]);

    // Convert the buffer to a hex string
    char* hexString = bufferToHexString(message, bufferSize);
    printf("Hex string: %s\n", hexString);

    printf("Message Recieved:\n");
    char version[2] = {hexString[0], hexString[1]};
    printf("RHP Version: %i", (int)version);

    int length = 4;
    //print payload
    for(int i = 8; i < 8+length; i+=2){
        // printf("%s",(char)hexString[i]);
        char hexPair[2] = {hexString[i], hexString[i + 1]};
        char asciiChar = strtol(hexPair, NULL, 16);

        // Print the ASCII character
        // printf("%c", (char)hexPair);
        // printf("%ld", asciiChar);
        printf("%c",(char)asciiChar);
    }
    free(hexString);
}



// uint16_t checkSum(char[] message, int size) {
//     uint16_t runningSum = 0;
//     uint16_t previousSum = 0;

//     //Summing in chunks of 16 bits
//     for(int i  = 0; i < sizeof(message); i = i + 16) {
//         runningSum += getString(message[i]);
//         if(previousSum > runningSum) { //Checking if there was overflow, and add one if so. (This is done for it)
//             runningSum++;
//         }
//         previousSum = runningSum;
//     }

//     runingSum = ~runningSum; //Bitwise not
//     return runningSum;
// }
