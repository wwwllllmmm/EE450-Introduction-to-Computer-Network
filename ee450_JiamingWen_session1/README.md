# Socket Programming Project, Spring 2022

**Name:   Jiaming Wen**
**USC ID:  4152412003**

In this project, two clients issue requests for finding their current amount of alicoins in their account, transfer amount and provide a file statement with all the transactions in order. These requests will be sent to a Central Server which in turn interacts with three other backend servers for pulling information and data processing.

## What I have done

I have already finished requires phases in description:

1. **Phase 1:**  ClientA and ClientB establish TCP connections with the server, send the requests of "CHECK WALLET" or "TXCOIN" to the server M.
2. **Phase 2A:** Server M send messages to the three backend-servers (Server A, Server B and Server C) with UDP connections. The request will be sent to their respective backend server depending on which information they need to get and which operation they need to execute.
3. **Phase 2B:** Server M receives the relevant data for the desired operation from the backend servers (serverA, serverB and serverC),perform the required computation and send the results to the clients. (CHECK WALLET or TXCOINS)
4. **Phase 3:** Clients send a keyword to get the full text version of all the transactions that have been taking place in Alichain, server get the log information from backend servers and save it on a file named ”alichain.txt” .

## Code Files and their fucntions

```
* clientA.c
    Code for clientA has following functions:
    1. Send requests to serverM using TCP.
    2. Transmit the request of "CHECK WALLET","TXCOIN","TXLIST" function to serverM
    3. Get result from serverM.

*  clientB.c
    Code for clientB.c has almost the same as clientA.c.

* serverM.c
    Code for serverM has following functions:
    1. Receive requests from clientA/B using TCP.
    2. communicate with backend serverA/B/C, use UDP to get relevant information.
    3. communicate with backend serverA/B/C, choose one backend server to write log information.
    4. Get log information from backend serverA/B/C,sort the list of transactions and generate the “alichain.txt” file
    5. Send clientA/B the result using TCP.

* serverA.c
    Code for serverA has following functions:
    1. communicate with serverM by UDP, receive and send relevant information.
    2. Read "block1.txt",get required message.
    3. Write transaction log into "block1.txt"

* serverB.c
    Code for serverB.c has almost the same as serverA.c but the UDP port and file are different.  

* serverC.c
    Code for serverC.c has almost the same as serverA.c but the UDP port and file are different.  
```


## Format of message exchange

**CHECK WALLET** 
use command `./clientA Chinmay`  as an example to show the format of message exchange.

```

1. Sample output of serverA

The Server A is up and running using UDP on port 21003
The Server A received a request from the Main Server.
The Server A finished sending the response to the Main Server.


2. Sample output of serverB

The Server B is up and running using UDP on port 22003
The Server B received a request from the Main Server.
The Server B finished sending the response to the Main Server.


3. Sample output of serverC

The Server C is up and running using UDP on port 23003
The Server C received a request from the Main Server.
The Server C finished sending the response to the Main Server.


4. Sample output of serverM

The main server is up and running.
The main server received input=<Chinmay> from the client using TCP over port <25003>.
The main server sent a request to server A
The main server received transactions from Server A using UDP over port 21003
The main server sent a request to server B
The main server received transactions from Server B using UDP over port 22003
The main server sent a request to server C
The main server received transactions from Server C using UDP over port 23003
The main server sent the current balance to client A


5. Sample output of clientA

a)if Chinmay exists
The client A is up and running.
<Chinmay> sent a balance enquiry request to the main server.
The current balance of <Chinmay> is : <881> alicoins.

b)if Chinmay is not exists
The client A is up and running.
<Chinmay> sent a balance enquiry request to the main server.
Chinmay is not part of the network.
```

**TXCOIN**
use command `./clientB Chinmay Luke 100`  as an example to show the format of message exchange.

```

1. Sample output of serverA

The Server A received a request from the Main Server.
The Server A finished sending the response to the Main Server.
The Server A received a request from the Main Server.
The Server A finished sending the response to the Main Server.


2. Sample output of serverB（suppose add the transaction to block2.txt ）

The Server B received a request from the Main Server.
The Server B finished sending the response to the Main Server.
The Server B received a request from the Main Server.
The Server B finished sending the response to the Main Server.
The Server B received a request from the Main Server.
The Server B finished sending the response to the Main Server.


3. Sample output of serverC

The Server C received a request from the Main Server.
The Server C finished sending the response to the Main Server.
The Server C received a request from the Main Server.
The Server C finished sending the response to the Main Server.


4. Sample output of serverM

The main server received from <Chinmay> to transfer <100> coins to <Luke> using TCP over port <26003>.
The main server sent a request to server A
The main server received  the feedback from Server A using UDP over port 21003
The main server sent a request to server B
The main server received  the feedback from Server B using UDP over port 22003
The main server sent a request to server C
The main server received  the feedback from Server C using UDP over port 23003
The main server sent a request to server B
The main server received the feedback from Server B using UDP over port 22003
The main server sent the result of the transaction to client B.


5. Sample output of clientB
a)if Chinmay and Luke exists, transaction successes
The client B is up and running.
<Chinmay> has requested to transfer <100> coins to <Luke>.
<Chinmay> successfully transferred <100> alicoins to <Luke>.
The current balance of <Chinmay> is : <781> alicoins.

b)if Chinmay and Luke exists, transaction fails due to insufficient balance
The client B is up and running.
<Chinmay> has requested to transfer <100000> coins to <Luke>.
<Chinmay> was unable to transfer <100000> alicoins to <Luke> because of insufficient balance.
The current balance of <Chinmay> is : <781> alicoins.

c)if Chinmay is not exists
The client B is up and running.
<Chinmay> has requested to transfer <100> coins to <Luke>.
Unable to proceed with the transaction as <Chinmay> is not part of the network.

d)if Luke is not exists
The client B is up and running.
<Chinmay> has requested to transfer <100> coins to <Luke>.
Unable to proceed with the transaction as <Luke> is not part of the network.

e)if Chinmay and Luke are not exists
The client B is up and running.
<Chinmay> has requested to transfer <100> coins to <Luke>.
Unable to proceed with the transaction as <Chinmay> and <Luke> are not part of the network.
```

**TXLIST** 
use command `./clientA TXLIST`  as an example to show the format of message exchange.

```

1. Sample output of serverA

The Server A received a request from the Main Server.
The Server A finished sending the response to the Main Server.


2. Sample output of serverB

The Server B received a request from the Main Server.
The Server B finished sending the response to the Main Server.


3. Sample output of serverC

The Server C received a request from the Main Server.
The Server C finished sending the response to the Main Server.


4. Sample output of serverM

A TXLIST request has been received from the client using TCP over port <25003>.
The main server sent a request to server A
The main server received the feedback from Server A using UDP over port 21003
The main server sent a request to server B
The main server received the feedback from Server B using UDP over port 22003
The main server sent a request to server C
The main server received the feedback from Server C using UDP over port 23003
The sorted file is up and ready.


5. Sample output of clientA

The client A is up and running.
client A sent a sorted list request to the main server.
```


## Idiosyncrasy in project

1.To handle the variable strings, there are lot of buffer with large size. The max length of buffers is 4000. If a single message or file exceeds 4000, the program will crash.

2. If sender transfer money to himself, the program will make some mistake.


## Resued Code

1. The implementation of TCP and UDP connection according to **Beej’s Guide to Network Programming**.  
2. The implementation of IO multiplexing(let clientA clientB can connect to serverM using TCP) is based on (https://blog.csdn.net/weixin_34211761/article/details/85570161?spm=1001.2101.3001.6650.8&utm_medium=distribute.p) with some modifications. 


