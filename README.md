![Animated Character Developing Winsock Communication](images/animated-image.jpeg)

# Winsocky
Winsocket implementation for Cobalt Strike. Used to communicate with the victim using winsockets instead of the traditional ways.

## Blog Post

[Developing Winsock Communication in Malware](https://whiteknightlabs.com/2023/07/06/developing-winsock-communication-in-malware/)

## Usage  

### client.c  
Is the client which receives the command from the server, executes the command in a child process, parse its output and send it back to the server.  
Open the solution (.sln) file to compile the code with Visual Studio.

### server.c  
The BOF script which is loaded to Cobalt Strike. It connects to the client's Winsocket's server, thus sending the command and receiving back the response.

To compile it, use `make`:  
```
cd Server && make
```

Then load `socket.cna` to Cobalt Strike. To use it, run the following command:    
```
socky <command>
```

**Note:** Commands with whiteline spaces (e.x.: `whoami /all`) must be wrapped in "".

## Demo  

![winsocket_bof_demo](https://github.com/WKL-Sec/Winsocky/assets/97109724/b876351f-893d-490d-bcc4-82fa55896ff7)


## Author  
Kleiton Kurti ([@kleiton0x00](https://github.com/kleiton0x00))

