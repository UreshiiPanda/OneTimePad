#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  	// ssize_t
#include <sys/socket.h> 	// send(),recv()
#include <netdb.h>      	// gethostbyname()  getnameinfo()
#include <netinet/in.h>
#include <arpa/inet.h>



static char s[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber)
{ 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 
  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = inet_addr("127.0.0.1");
}


int main(int argc, char* argv[])
{
  int connectionSocket, charsRead, charsWritten;
  char bufferIn[3];
  char bufferOut[2];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check for input errors
  if (argc < 2) { 
    fprintf(stderr, "Too few arguments given to dec_server, please enter port number\n"); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    fprintf(stderr, "DEC_SERVER: error creating socket");
    exit(1);
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
    fprintf(stderr, "DEC_SERVER: ERROR binding the server socket to the supplied port: %s\n", argv[1]);
    exit(1);
  }

  // Start listening for connections. Allow up to 5 connections to queue up
  if (listen(listenSocket, 5) < 0) {
      fprintf(stderr, "DEC_SERVER: error listening to client connections\n");
  }
 
  // Accept a connection, blocking if one is not available until one connects
  for(;;) {
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      fprintf(stderr, "DEC_SERVER: error accepting connection to client\n");
    }

    pid_t child_pid = fork();

    if (child_pid == -1) {
        fprintf(stderr, "DEC_SERVER: failed to fork a new child\n");
        continue;
    }

    else if (child_pid == 0) {  
        // child code

        // if the connection was made to enc_client, reject it
        char verifyIn[3];
        char verifyOut[3];

        memset(verifyIn, '\0', 3);
        // Read the client's data from the socket
        charsRead = recv(connectionSocket, verifyIn, 2, 0); 
        if (charsRead < 0) {
            fprintf(stderr, "DEC_SERVER: ERROR reading verification chars from client\n");
        }
        else if (charsRead != 2) {
            fprintf(stderr, "DEC_SERVER: ERROR num of verification chars read in is not 2\n");
        }
        //fprintf(stderr, "DEC_SERVER: I received these verification chars from the client: %s \n", verifyIn);

        if (verifyIn[0] == '0' && verifyIn[0] == '0') {
            fprintf(stderr, "DEC_SERVER: Rejecting connection to enc_client\n");
            memset(verifyOut, '\0', 3);
            verifyOut[0] = '3';
            verifyOut[1] = '3';
            charsWritten = send(connectionSocket, verifyOut, 2, 0);
            if (charsWritten < 0) {
                fprintf(stderr, "DEC_SERVER: ERROR no verification chars sent back to client\n");
            }
            else if (charsWritten != 2) {
                fprintf(stderr, "DEC_SERVER: ERROR num of verification chars sent out is not 2\n");
            }
            close(connectionSocket);
            continue;
        }
        else if (verifyIn[0] != '4' && verifyIn[1] != '4') {
            fprintf(stderr, "DEC_SERVER: Rejecting connection to a client that isn't dec_client\n");
            close(connectionSocket);
            continue;
        }

        else {
          // enc_client is verified, so send back enc_server verification chars
          memset(verifyOut, '\0', 3);
          verifyOut[0] = '3';
          verifyOut[1] = '3';
          charsWritten = send(connectionSocket, verifyOut, 2, 0);
          if (charsWritten < 0) {
              fprintf(stderr, "DEC_SERVER: ERROR no verification chars sent back to client\n");
          }
          else if (charsWritten != 2) {
              fprintf(stderr, "DEC_SERVER: ERROR num of verification chars sent out is not 2\n");
          } 

          for(;;) { 
            // Get ciphertext & key from the client and decrypt it
            if (bufferIn[0] == '1' && bufferIn[1] == '1') break;

            memset(bufferIn, '\0', 3);
            // Read the client's data from the socket
            charsRead = recv(connectionSocket, bufferIn, 2, 0); 
            if (charsRead < 0) {
                fprintf(stderr, "DEC_SERVER: ERROR reading from client\n");
            }
            else if (charsRead != 2) {
                fprintf(stderr, "DEC_SERVER: ERROR num of chars read in is not 2\n");
            }

            // DECRYPT THE TEXT
            int idx_text = 0;
            int idx_key = 0;           
            while (s[idx_text] != bufferIn[0]) idx_text++;
            while (s[idx_key] != bufferIn[1]) idx_key++;
            int dec_idx_text = idx_text - idx_key;
            if (dec_idx_text < 0) dec_idx_text = dec_idx_text + 27;
            dec_idx_text = dec_idx_text % 27;
            memset(bufferOut, '\0', 2);
            bufferOut[0] = s[dec_idx_text];

            // Send encrypted msg back to the client
            charsWritten = send(connectionSocket, bufferOut, 1, 0);
            if (charsWritten < 0) {
                fprintf(stderr, "DEC_SERVER: ERROR no chars sent back to client\n");
            }
            else if (charsWritten != 1) {
                fprintf(stderr, "DEC_SERVER: ERROR num of chars sent out is not 1\n");
            }
            //fprintf(stderr, "DEC_SERVER:  sending this encrypted char back to client: %c \n", bufferOut[0]);
          }
        
          // Close the connection socket for this client
          close(connectionSocket);
          // end the child process
          exit(0);
        }
    }

    else {
        // parent code
        // parent just jumps back to for-loop for next client connection 
    }

  }

  // Close the listening socket
  close(listenSocket); 
  return 0;
}

