# 📌 Multi-User Chatting Room (AES & Base64)

<img src="https://github.com/kafkaaaa/chat/assets/20926959/ebc9d66d-8e5d-4ee5-80f7-dafdfb5348ab">

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
<img src="https://github.com/kafkaaaa/chat/assets/20926959/085f0b12-e28c-4ce4-996d-a7f0f649548c">
  
### Server
<img src="https://github.com/kafkaaaa/chat/assets/20926959/d887f159-07e6-46a2-9c11-9613e08430c0">

### Log file
<img src="https://github.com/kafkaaaa/chat/assets/20926959/b3cefef6-dac4-43ef-9cfe-42285cc0309e">


