#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  	// ssize_t
#include <sys/socket.h> 	// send(),recv()
#include <netdb.h>      	// gethostbyname()
#include <stdbool.h>
#include <netinet/in.h>
#include <arpa/inet.h>
                        

static char s[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";


/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments
* 2. Prompt the user for input and send that input as a message to the server
* 3. Print the message received from the server and exit the program
*/


// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber)
{
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));
  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Set server address to localhost   
  address->sin_addr.s_addr = inet_addr("127.0.0.1");                                                          
}


// args are:  ciphertext, key, port
int main(int argc, char* argv[])
{
    // check for input errors
    if (argc > 4) {
        fprintf(stderr, "too many arguments given to dec_client\n");
        return 1;
    }
    else if (argc < 4) {
        fprintf(stderr, "too few arguments given to dec_client\n");
        return 1;
    }
    
    FILE* ciphertext;
    FILE* key; 

    // check that key has length >= ciphertext length
    // check that all chars in both ciphertext and key are the valid 27 chars in s[] global var
    char ciphertext_char;
    ciphertext = fopen(argv[1], "r");
    fseek(ciphertext, 0, SEEK_SET);
    ciphertext_char = fgetc(ciphertext);
    bool isValid;

    while (!feof(ciphertext)) {
        isValid = false;
        for (int k = 0; k < strlen(s); k++) {
            if (ciphertext_char == s[k]) {
                isValid = true;
                break;
            }
        }
        if (isValid == false && ciphertext_char != '\n') break;
        else isValid = true;
        ciphertext_char = getc(ciphertext);
    }

    if (isValid == false) {
        fprintf(stderr, "ciphertext file contains an invalid char\n");
        return 1;
    }

    size_t ciphertext_len = ftell(ciphertext);
    fclose(ciphertext);
    
    char key_char;
    key = fopen(argv[2], "r");
    fseek(key, 0, SEEK_SET);
    key_char = fgetc(key);

    while (!feof(key)) {
        isValid = false;
        for (int k = 0; k < strlen(s); k++) {
            if (key_char == s[k]) {
                isValid = true;
                break;
            }
        }
        if (isValid == false && key_char != '\n') break;
        else isValid = true;
        key_char = getc(key);
    } 

    if (isValid == false) {
        fprintf(stderr, "key file contains an invalid char\n");
        return 1;
    }

    size_t key_len = ftell(key);
    fclose(key);
    if (key_len < ciphertext_len) {
        fprintf(stderr, "key has less chars than ciphertext\n");
        return 1;
    }
        
    
    int socketFD, charsWritten, charsRead;
    int port = atoi(argv[3]);
    struct sockaddr_in serverAddress;
 
  for(;;) {

    // Create a socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); 
    if (socketFD < 0) {
      fprintf(stderr, "DEC_CLIENT: ERROR creating socket in dec_client\n");
    }

    // Set up the server address struct
    setupAddressStruct(&serverAddress, port);

    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
      fprintf(stderr, "DEC_CLIENT: ERROR connecting client socket to localhost\n");
    }

    // connect to port and make sure that dec_server is NOT on that port 
    // try to connect to enc_server, if not return 2
    char verifyOut[3];
    char verifyIn[3];
    memset(verifyOut, '\0', 3); 
    verifyOut[0] = '4';
    verifyOut[1] = '4';
    charsWritten = send(socketFD, verifyOut, 2, 0); 
    if (charsWritten < 0){
        fprintf(stderr, "DEC_CLIENT: ERROR writing verification chars to socket\n");
    }
    if (charsWritten != 2){
        fprintf(stderr, "DEC_CLIENT: WARNING wrong num of verification chars written to socket!\n");
    }

    // Clear out the buffer again for reuse 
    memset(verifyIn, '\0', 3);
    // Read data from the socket
    charsRead = recv(socketFD, verifyIn, 2, 0);
    if (charsRead < 0) {
        fprintf(stderr, "DEC_CLIENT: ERROR reading verification chars from socket\n");
    }
    else if (charsRead != 2) {
        fprintf(stderr, "DEC_CLIENT: WARNING wrong num of verification chars received from server\n");
    }   
    // fprintf(stderr, "DEC_CLIENT: I received these verification chars from the server: %s \n", verifyIn);
   
    if (verifyIn[0] == '2' && verifyIn[1] == '2') {
        fprintf(stderr, "DEC_CLIENT: Rejecting connection to enc_server\n");
        close(socketFD);
        break;
    }
    else if (verifyIn[0] != '3' && verifyIn[1] != '3') {
        fprintf(stderr, "DEC_CLIENT: Rejecting connection to a server that isn't dec_server\n");
        close(socketFD);
        break;
    }

    else {

    // enc_server has been verified
    // Send message to server
    char bufferOut[3];
    int countOut = 0;   
    char bufferIn[2];
    int countIn = 0;
    char output[ciphertext_len + 1];
    memset(output, '\0', ciphertext_len + 1);
    ciphertext = fopen(argv[1], "r");
    key = fopen(argv[2], "r");
    fseek(ciphertext, 0, SEEK_SET);
    fseek(key, 0, SEEK_SET);

    // send ciphertext and key to server for encryption
    while (countIn < ciphertext_len - 1 && !feof(ciphertext) && !feof(key)) {

        memset(bufferOut, '\0', 3);
        bufferOut[0] = getc(ciphertext);
        bufferOut[1] = getc(key); 
        charsWritten = send(socketFD, bufferOut, 2, 0); 
        if (charsWritten < 0){
            fprintf(stderr, "DEC_CLIENT: ERROR writing to socket\n");
        }
        if (charsWritten != 2){
            fprintf(stderr, "DEC_CLIENT: WARNING wrong num of chars written to socket!\n");
        }
        countOut++;
    
        // send termination chars
        if (countOut == ciphertext_len - 1) {
            bufferOut[0] = '1';
            bufferOut[1] = '1';
            charsWritten = send(socketFD, bufferOut, 2, 0); 
            if (charsWritten < 0){
                fprintf(stderr, "DEC_CLIENT: ERROR writing terminators to socket\n");
            }
            if (charsWritten != 2){
                fprintf(stderr, "DEC_CLIENT: WARNING wrong num of terminators written to socket!\n");
            }
        }

        // Get return message from server
        // Clear out the buffer again for reuse 
        memset(bufferIn, '\0', 2);
        // Read data from the socket
        charsRead = recv(socketFD, bufferIn, 1, 0);
        output[countIn] = bufferIn[0];

        if (charsRead < 0) {
            fprintf(stderr, "DEC_CLIENT: ERROR reading from socket\n");
        }
        else if (charsRead != 1) {
            fprintf(stderr, "ENC_CLIENT: WARNING wrong num of chars received from server\n");
        }   
        countIn++;
    }
    fclose(ciphertext);
    fclose(key);

    // put '\n' back on
    output[ciphertext_len - 1] = '\n';    

    // output the encrypted text to stdout
    fprintf(stdout, "%s", output);
    
    // Close the socket
    close(socketFD);
    break;

   }

  }
  
  return 0;
}

