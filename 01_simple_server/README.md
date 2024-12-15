# Server Example
I'm using a simple C# echo server to test retro.link. The code use dotnet 9.
I'm using Fedora Linux for development, but it should work in any OS with 
dotnet support. Instructions can be found on Microsoft's [website](https://dotnet.microsoft.com/en-us/download).

## C# Echo Server
After downloading the project, change directories into the server directory
and run `dotnet` build.
```bash
cd SGDK_Retro.Link/01_simple_server/server
dotnet build
```

Run the server from the command-line by typing 
```bash
dotnet run <YOUR_IP_ADDRESS>
```


## C# Client
To make sure the server is working, I've made a simple test client in C#.
Change directories into the client directory and run `dotnet` build.
```bash
cd SGDK_Retro.Link/01_simple_server/csharp_client
dontnet build
```

WHile the server is running, run the client from the command-line by typing 
```bash
dotnet run <SERVER_IP_ADDRESS> <YOUR MESSAGE>
```


## SGDK Client
There are a number of different ways to build SGDK. I will not cover that here.
To use the client, you **_MUST_** change the IP addresses in `sega_client/src/main.c` 
to match yours.


