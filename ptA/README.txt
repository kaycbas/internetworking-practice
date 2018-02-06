Assignment: CS-353 Programming Assign1
Name: Kevin Bastoul
USC ID: 4393748478

Compile Instructions:
-Navigate to the folder container the files
-Type 'make' on the command line
-Afterward, in order to run the server simply enter 'server' into the command line
-In order to run a client, open a new terminal, navigate to the folder, and type 'client'
-This process can be repeated for as many clients as desired

Additional Notes:
-each time a client is started it will register with the server and then wait for cmd line input
-if the user types 'sendto <client#> <message>', a message will be sent to that client's terminal
-to exit a client, type 'exit'
-to exit a server, type 'server'

**
-This program was built on a Windows machine in c++11 using Winsock and Pthreads -> It likely will not run on a Mac in its current state
-I had four midterms in the last week and wasn't able to get to part3 (implementing server to server TCP)

