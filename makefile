client: FTPclient.c
	gcc -o FTPclient FTPclient.c && ./FTPclient 127.0.0.1 9999

server: FTPserver.c
	gcc -o server FTPserver.c && ./server 

clean:
	rm *.o server client
