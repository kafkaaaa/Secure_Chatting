# 📌 Multi-User Chatting Room (AES & Base64)

<img src="https://github.com/kafkaaaa/Secure_Address_Book/assets/20926959/a8a920dd-2e08-47c8-8f53-973e22ed196c">

- Socket Chatting Program (Client/Server)
- C / Linux / CLI
- Multi-Thread
- Message Send Process:  Plain Text -> AES-256/CBC Decryption -> Binary -> Base64 Encoding
- Message Receive Process:  Base64 Encoded String -> Base64 Decoding -> Binary -> AES Decryption -> Plain Text
- Chatting Log: Auto save to file (dialog messages are base64-encoded)
- Exit: input 'X' or 'Ctrl + C'
  

# 📌 Build
```
$ make clean
$ make
$ ./server [Port]
$ ./client [IP] [Port] [Name]
```

# 📌 Example

### Client
<img src="https://github.com/kafkaaaa/Secure_Address_Book/assets/20926959/844504ce-985b-4855-a7be-96efa7aab598">
  
### Server
<img src="https://github.com/kafkaaaa/Secure_Address_Book/assets/20926959/fec5cf2d-a886-4d48-ab25-fd7a28a12c5f">

### Log file
<img src="https://github.com/kafkaaaa/Secure_Address_Book/assets/20926959/d1cc8190-354d-484b-b81f-f3d26e495f03">


