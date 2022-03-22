Compile and run:
In one terminal:
go to file directory
gcc -o server sever.c
gcc -o client client.c
./server

In another terminal:
go to file directory
./client


More:
IP: localhost
port: 59002

User inputs:
end of packet identifier (to test for error: missing end id)
segment number
message(payload)
message length