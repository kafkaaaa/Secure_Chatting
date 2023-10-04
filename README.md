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

# 📌 Example
<Server>
![image](https://github.com/kafkaaaa/chat/assets/20926959/59b94ba9-7973-4ac6-bc38-e92e550c2a5d)


<Client>
![image](https://github.com/kafkaaaa/chat/assets/20926959/b15ff459-75fb-4f9a-bf5d-328f4836c12d)
