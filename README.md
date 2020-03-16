# Computer-Networks-Project1-File-Transfer-Protocol-FTP-
The project is a simplified version of the File Transfer Protocol(FTP). It consists of two files: FTPclient.c and FTPserver.c. 
The FTP server works concurrently and is able to handle simultaneous users' requests through establishing a TCP connection.


To run the project clone it from git
Run "make client" command to run FTPclient.c
Enter the server folder and run "make server" to run FTPserver.c

Features:
User authemtication/password check
Can handle multiple users
File upload and Download


Limitations:
* In case, there is PUT or GET commands while server and client are at the same directory the file will get overwritten by the receiver. Therefore, to test PUT and GET commands I decided to store server program in seperate folder. However, upload and download works perfectly. This decition was made to testing process easier.

* Uploads and Downloads work only if a file exists in the current directory of a sender. To improve it we can look all folders to find a file and then send it.


Test Cases:
* Incorrect command
  client:ftp> mkdir folder
  server:An invalid ftp command

* User autentication
 There is a file containing usernames and passwords
 username: user, password: 123
 client:ftp> USER uesr
 server:Username does not exist
 client:ftp> USER user
 server:Username OK, password required
 client:ftp> PASS qwerty
 server:wrong password
 client:PASS 123
 server:Authentication complete
 
 
 *Downloading a file
  Goal: GET testget.txt
ftp> !LS
FTPclient	README.md	server
FTPclient.c	makefile	testsend.txt

ftp> GET testget.txt
Received the file successfuly

ftp> !LS
FTPclient	README.md	server		testsend.txt
FTPclient.c	makefile	testget.txt    <----- successfully downloaded

 
 *Uploading a file to server
  Goal: PUT testsend.txt to server
ftp> LS 
FTPserver
FTPserver.c
makefile
testget.txt
userpass.txt

ftp> PUT testsend.txt
ftp> LS
FTPserver
FTPserver.c
makefile
testget.txt
testsend.txt   <---- file was uploaded to server
userpass.txt






