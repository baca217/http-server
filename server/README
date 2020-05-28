NameL Elmer Baca Holguin
Class: CSCI 4273 Network Systems
Project: Lab 2 HTTP Server
-------------------------------------------------------------------------
server.c

functions:
Main() - takes in one command line argument for the port number. After
that the program does all its work in terms of setting up a port number,
binding it to a socket and listening for a request. Most of the work
is just handed of to different functions. Main() is just used for 
creating the threads.

open_listenfd() - takes in one argument from main which is "port".
This function does all the nitty gritty work of opening up a socket,
forcing a bind of that socket, binding the port to the socket, and
then listening on that particular port. The file descriptor is
returned to the function Main().

msg_work() - takes in one argument which is the file descriptor for our
listening port. This function is what parses the request from the client
browser and sends the appropriate response. This will only take care of
the get command where a file that's requested will be sent if it exists.

thread() - takes in one argument. It's just used to set up the threads.
Converts the argument to an integer, because it's the file descriptor 
for the listening port. This file descriptor is passed of to the
msg_work() function, then the listening port is closed.
