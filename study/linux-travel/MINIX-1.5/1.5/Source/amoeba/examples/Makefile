CFLAGS=-F
all: client1 server1 client2 server2 client3 server3
	
client1: header.h client1.s
	 cc -o client1 client1.s

client2: header.h client2.s
	 cc -o client2 client2.s

client3: header.h client3.s
	 cc -o client3 client3.s

server1: header.h server1.s
	 cc -o server1 server1.s
	 chmem =1000 server1

server2: header.h server2.s
	 cc -o server2 server2.s
	 chmem =1000 server2

server3: header.h server3.s
	 cc -o server3 server3.s
	 chmem =1000 server3
