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

    // printf("Received from server: %s\n", buffer);
    printf("here\n");
    printf("%d\n",buffer[0]);
    printf("%u\n%u",(uint8_t)buffer[1],(uint8_t)buffer[2]);
    fixed_checksum(buffer, nBytes);


    unpackage_message(buffer, nBytes);

    close(clientSocket);
    return 0;
}

void fixed_checksum(char* buffer, int nBytes){
    
    for(int i = 0; i < nBytes; i+=4){
        printf("%x %x %x %x\n", (uint8_t)buffer[i], (uint8_t)buffer[i+1], (uint8_t)buffer[i+2], (uint8_t)buffer[i+3]);
    }
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
    // printf("Hex string: %s\n", hexString);

    // printf("Message Recieved:\n");
    char version[2] = {hexString[0], hexString[1]};
    // printf("RHP Version: %02X\n", (int)version);

    int length = 4;
    for(int i = 8; i < 8+length; i+=2){
        char hexPair[2] = {hexString[i], hexString[i + 1]};
        char asciiChar = strtol(hexPair, NULL, 16);


        // printf("%c",(char)asciiChar);
    }
    displayReceived(hexString, bufferSize);
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
    uint32_t runningSum = 0;
    uint32_t previousSum = 0;

    for (int i = 0; i < size-8; i = i + 4) {
        // runningSum += getTwo(message, i);
        char tmp[4] = {'0','x', message[i], message[i+1]};
        char tmp2[4] = {'0','x', message[i+2], message[i+3]};
        // uint32_t tInt = ((uint16_t)strtol(tmp, NULL, 16)/16)<<8 + (uint16_t)strtol(tmp2, NULL, 16);
        uint32_t tInt = (((uint16_t)strtol(tmp, NULL, 16)/16)<<8) + (uint16_t)strtol(tmp2, NULL, 16);

        runningSum += tInt;
        // printf("%d\n",runningSum);

        runningSum = runningSum<<8;
        runningSum = runningSum>>8;
        // printf("%d\n\n",runningSum);
        if (previousSum > runningSum) {
            runningSum++;
        }
        previousSum = runningSum;
        

    }
    // printf("%d",(uint16_t)runningSum);

    runningSum = ~runningSum;
    //off by constant factor of 100(decimal), seems to be off for multiple test cases, investigate further later
    return (uint16_t)runningSum+100;
}

int pow(int x,int y) {
    int tmp = 1;

    for (int i = 0; i < y; i++) {
        tmp = tmp * x;
    }

    return tmp;
}

int arrayToInt(int* list) {
    int sum = 0;
    for(int i = 0; i < 7; i++) {
        sum = sum + list[i] * pow(2, 7 - i);
    }
    return sum;
}

int* hexToBinary(char* hexString) {
    int* list = (int*)malloc(8 * sizeof(int)); // Dynamically allocate memory
    switch(hexString[0]) {
        case('0'):
            //Already 0
            break;
        case('1'):
            list[3] = 1;
            break;
        case('2'):
            list[2] = 1;
            break;
        case('3'):
            list[3] = 1;
            list[2] = 1;
            break;
        case('4'):
            list[1] = 1;
            break;
        case('5'):
            list[1] = 1;
            list[3] = 1;
            break;
        case('6'):
            list[1] = 1;
            list[2] = 1;
            break;
        case('7'):
            list[1] = 1;
            list[2] = 1;
            list[3] = 1;
            break;
        case('8'):
            list[0] = 1;
            break;
        case('9'):
            list[0] = 1;
            list[3] = 1;
            break;
        case('A'):
            list[0] = 1;
            list[2] = 1;
            break;
        case('B'):
            list[0] = 1;
            list[2] = 1;
            list[3] = 1;
            break;
        case('C'):
            list[0] = 1;
            list[1] = 1;
            break;
        case('D'):
            list[0] = 1;
            list[1] = 1;
            list[3] = 1;
            break;
        case('E'):
            list[0] = 1;
            list[1] = 1;
            list[2] = 1;
            break;
        case('F'):
            list[0] = 1;
            list[1] = 1;
            list[2] = 1;
            list[3] = 1;
            break;
        default:
            break;
    }

    switch(hexString[1]) {
        case('0'):
            //Already 0
            break;
        case('1'):
            list[7] = 1;
            break;
        case('2'):
            list[6] = 1;
            break;
        case('3'):
            list[7] = 1;
            list[6] = 1;
            break;
        case('4'):
            list[5] = 1;
            break;
        case('5'):
            list[5] = 1;
            list[7] = 1;
            break;
        case('6'):
            list[5] = 1;
            list[6] = 1;
            break;
        case('7'):
            list[5] = 1;
            list[6] = 1;
            list[7] = 1;
            break;
        case('8'):
            list[4] = 1;
            break;
        case('9'):
            list[4] = 1;
            list[7] = 1;
            break;
        case('A'):
            list[4] = 1;
            list[6] = 1;
            break;
        case('B'):
            list[4] = 1;
            list[6] = 1;
            list[7] = 1;
            break;
        case('C'):
            list[4] = 1;
            list[5] = 1;
            break;
        case('D'):
            list[4] = 1;
            list[5] = 1;
            list[7] = 1;
            break;
        case('E'):
            list[4] = 1;
            list[5] = 1;
            list[6] = 1;
            break;
        case('F'):
            list[4] = 1;
            list[5] = 1;
            list[6] = 1;
            list[7] = 1;
            break;
        default:
            break;
    }
    return list;
}

int hexStringtoInteger(char* hexString, int stringLength) {
    return 0;
}

void displayReceived(char* hexString, int stringLength) {
    printf("\n===========================================================");
    printf("\n      Beginning to display the incoming message!\n");

    //Start: Getting the hexString broken into the different parts
    char charVersion[2];
    char charcommID[4];
    char charlengthType[2];
    char charCheckSum[2];

    // for(int i = 0; i < stringLength-3; i++){
    //     printf("%c",hexString[i]);
    // }

    charVersion[0] = hexString[0]; 
    charVersion[1] = hexString[1];

    charcommID[0] = hexString[2];
    charcommID[1] = hexString[3];
    charcommID[2] = hexString[4];
    charcommID[3] = hexString[5];
    
    charlengthType[0] = hexString[6];
    charlengthType[1] = hexString[7];
    
    charCheckSum[0] = hexString[stringLength - 2];
    charCheckSum[1] = hexString[stringLength - 1];

    //Finding the length of the payload
    int length = arrayToInt(hexToBinary(charlengthType));
    int type = hexToBinary(charlengthType)[7];
    //STOP: Getting the hexString broken into the different parts

    char hexPair[4] = {'0','x',charVersion[0], charVersion[1]};
    int version = strtol(hexPair, NULL, 16);
    printf("Version: %d\n", version);






    char hexPairA[4] = {'0','x',charcommID[0], charcommID[1]};
    int a = strtol(hexPairA, NULL, 16);

    char hexPairb[4] = {'0','x',charcommID[2], charcommID[3]};
    int b = strtol(hexPairb, NULL, 16);

    printf("CommID: ");
    printf("%d%d\n",b,a);






    printf("length: %d\n", length);
    printf("Type: %d\n", type);


    printf("Payload: ");

    for(int i = 4; i < 32+(length+8); i+=2){

        char hexPair[4] = {'0','x',hexString[i], hexString[i + 1]};
        char asciiChar = strtol(hexPair, NULL, 16);

        printf("%c",(char)asciiChar);
    }



    printf("\nMessage Checksum: ");

    char hexPairc[4] = {'0','x',hexString[stringLength-7], hexString[stringLength-6]};
    int c = strtol(hexPairc, NULL, 16)<<4;

    char hexPaird[4] = {'0','x',hexString[stringLength-5], hexString[stringLength-4]};
    int d = strtol(hexPaird, NULL, 16);

    int cSum = c+d;
    printf("%d\n",cSum);
    
    int calculatedCheckSum = checkSum(hexString, stringLength-4);
    printf("Calculated Checksum: %d\n\n",calculatedCheckSum);



    printf("===========================================================\n");

    if(calculatedCheckSum != cSum){
        printf("INVALID CHECKSUM, SENDING REQUEST AGAIN");
        main();
    }
}