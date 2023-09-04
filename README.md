# OneTimePad

#### One-Time Pad encryption &amp; decryption written in C


<a name="readme-top"></a>

<!-- one-time pad gif -->
![one_time_pad](https://github.com/UreshiiPanda/OneTimePad/assets/39992411/063e3dff-7790-4d3c-af7b-1447db438305)



<!-- ABOUT THE PROJECT -->
## About The Project

This is a small collection of programs which carry out a One-Time Pad text encryption and decryption.
You can read about the One-Time Pad process here [One Time Pad Wiki](https://en.wikipedia.org/wiki/One-time_pad) 
but basically, it's an encryption method that involves the use of a single-use key that is >= the size of the 
message being sent and is shared between both parties (the encryptor and the decryptor). This program first
uses a key generation program to generate a random single-use key. This program then runs an encryption 
server which listens for encyrption clients and encrypts their messages. Finally, the program then runs a 
decryption server which listens for decryption clients and decrypts their encrypted messages using the 
same key that was used to encrypt that message.

NOTE:  the program only encrypts/decrypts messages using Capital letters and spaces

<p align="right">(<a href="#readme-top">back to top</a>)</p>



<!-- GETTING STARTED -->
## Getting Started

This program can be run from any command line interpreter after the program has been compiled
by a C compiler (eg. gcc, clang). Note that a makefile has also been provided in order to
handle compilation of all five C files by executing the "make" command.


### Installation / Execution Steps

1. Clone the repo
   ```sh
      git clone https://github.com/UreshiiPanda/OneTimePad.git
   ```
3. Compile
   ```sh
      make
   ```
4. Generate a random key (or create your own random key by any other means). Note that
   this key generation uses the dev/urandom file on Unix-like systems
   ```sh
      ./keygen 20 > key      # put a key of length 20 into "key" file
   ```
6. Start the encryption/decryption servers in the background on any open ports
   ```sh
      ./enc_server 11111 &      # start encryption server on port 11111
   ```
   ```sh
      ./dec_server 22222 &      # start decryption server on port 22222
   ```
7. Encrypt a message with the encryption client on the encryption server port
   ```sh
      ./enc_client message.txt key 11111 > enc_message.txt   # encrypt message.txt using key
   ```
6. Decrypt the encrypted message with the decryption client on the decryption server port
   ```sh
      ./dec_client enc_message.txt key 22222     # decrypt enc_message.txt using key
   ```


<p align="right">(<a href="#readme-top">back to top</a>)</p>
