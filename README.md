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
<Client>
<p align="Left">
  <img src="https://github.com/kafkaaaa/chat/assets/20926959/293f6b21-bb0e-48b7-802e-90901e9d8935">
</p>
  
<Server>
<p align="Left">
  <img src="https://github.com/kafkaaaa/chat/assets/20926959/3736a68b-7afe-4255-bc1b-eed0e7b0c692)https://github.com/kafkaaaa/chat/assets/20926959/3736a68b-7afe-4255-bc1b-eed0e7b0c692">
</p>




