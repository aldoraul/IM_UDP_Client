# IM_UDP_Client

By Aldo Anaya

README

The program begins by asking the for a user name.  It begins by initializing a socket and a random message number.  
It then begins a child thread with _beginthread with the listen function which continuously listens for incoming 
messages.  a menu loop is created.  the user may check messages, send a message or quit the program.  if a user
chooses to send a message they must specify a logged in user and a message body.  if the user is not 
logged in an error message will be received by the user.

OS

Windows 10 Home

IDE Platform

Microsoft Visual Studio Enterprise 2015

LANGUAGE

C++
