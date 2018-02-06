/* README */
Name: Kevin Bastoul
USC ID: 4393748478

Pt.1 was developed on Windows and thus uses WinSock and various associated functions. 
It should be fully working but needs to be run on a Windows machine.

Pt.2 was developed on a borrowed Linux machine because pcap wouldn't work in my Windows environment. Keerthan
spent a long time with me trying to get pcap to work on my machine and eventually we decided I needed to switch
environments. The output of part 2 is partially complete and it doesn't have the functionality of reading from 
a pcap file.

To run P1 (on a windows machine):
-navigate to the pt1 folder
-type 'make'
-then type 'pinger -p data -c N -d destIP' to run the program

To run P2 (on Linux):
-navigate to the pt2 folder
-type 'make'
-then type 'viewer -i interface -c N' to run the program
-the -r filname command will not work bc this function isn't implemented


**You must be in Administrator/Root Mode to run both programs
