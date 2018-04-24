# one-time-pad
A pair of client and server programs that encrypt and decrypt a message stored in a file passed in at the command line

To compile put all files into the same directory and type "compileall" without quotes which will run a bash script to create the 
executables. 

To use the program run the keygen program to generate a string of random characters and redirect the output into the file that will
store the key. To use keygen type "keygen keylegnth" where keylength is how many characters long you want the key to be. 

Once you have a key and a plaintext file of equal or fewer characters to the keyfile start otp_enc_d running on an open port 
as a background process. Start otp_enc by typeing "otp_enc plaintext keyfile port" where plaintext is a file containing upperacse
letters A - Z, spaces or newilne characters, keyfile is the file of random characters generated from redirecting the output of keygen
into a file and port is the port that otp_end_d is running on. opt_end_d will encrypt the message and pass it back to opt_enc which should be redirected and stored in a file. 

To decrypt the file back to plaintext start otp_dec_d on an open port as a background process. Then type "otp_dec cyphertext keyfile port"
where cyphertext is the file that was generated from running otp_enc, keyfile is the file that was used to generate cyphertext, and port
is the port that opt_dec_d is listeng on. 

The program can support up to five client/server connections at a time. 
