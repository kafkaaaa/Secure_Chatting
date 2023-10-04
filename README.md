# 📌 Multi-User Chatting Room
- Socket Chatting Program
- C / Linux / CLI
- Multi-Thread, Server-Client
- Message Send Process:  Plain Text -> AES-256/CBC Decryption -> Base64 Encoding
- Message Receive Process:  Base64 Encoded String -> Base64 Decoding -> AES Decryption -> Plain Text
- Chatting log file (Encrypted)
  

# 📌 Build
```
$ make clean
$ make
$ ./server [Port]
$ ./client [IP] [Port] [Name]
```
