client: FTPclient.c
	gcc -o FTPclient FTPclient.c && ./FTPclient 127.0.0.1 9999

server: FTPserver.c
	gcc -o FTPserver FTPserver.c && ./FTPserver 

clean:
	rm *.o FTPserver FTPclient
