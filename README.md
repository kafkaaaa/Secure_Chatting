# 📌 Multi-User Chatting Room

<img src="https://github.com/kafkaaaa/chat/assets/20926959/ebc9d66d-8e5d-4ee5-80f7-dafdfb5348ab">

- Socket Chatting Program (Client/Server)
- C / Linux / CLI
- Multi-Thread
- Message Send Process:  Plain Text -> AES-256/CBC Decryption -> Binary -> Base64 Encoding
- Message Receive Process:  Base64 Encoded String -> Base64 Decoding -> Binary -> AES Decryption -> Plain Text
- Chatting Log: Auto save to file
  

# 📌 Build
```
$ make clean
$ make
$ ./server [Port]
$ ./client [IP] [Port] [Name]
```

# 📌 Example

### Client
<img src="https://github.com/kafkaaaa/chat/assets/20926959/b9073422-1232-4a0c-8e7a-d73372a1c4c5">
  
### Server
<img src="https://github.com/kafkaaaa/chat/assets/20926959/f76cfd9c-23aa-4e8c-968e-ded6eed46465">




