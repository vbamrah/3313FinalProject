
#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;
using namespace std;

class ClientThread : public Thread {
private:
	Socket& socket;
	string userInput;
	ByteArray userInputBytes;
	bool &running;
	string roomID;
	string name;
public:
	ClientThread(Socket& socket, bool &running, string roomID, string name)
	: socket(socket), running(running), roomID(roomID), name(name)
	{}
	
	~ClientThread()
	{}
	
	virtual long ThreadMain()
	{
		cout<<"Connecting..."<<endl;
		sleep(1);
		sendToServer("/nameset "+name);
		sleep(1);
		sendToServer("/join "+roomID);
		
		int response;
		while (running) {
			response = socket.Read(userInputBytes);
			
			if(response != 0) {
				cout<<userInputBytes.ToString()<<endl;
			}
		}
		return 0;
	}
	void sendToServer(string message) {
		if (message == "/quit") {
            Socket.Write(ByteArray(message));
			running = false;
		}
		else if (message.rfind("/nameset",0) == 0) {
			socket.Write(ByteArray(message));
		        name = message.erase(0,9);
		}
		else if (message.rfind("/join",0)==0) {
			socket.Write(ByteArray(message));
		        roomID = message.erase(0,6);
		}
		else {
			socket.Write(ByteArray(message));
		}
	}
	
	
};

int main(void)
{
	// Welcome the user
	string roomID;
	string name;
	cout << "Welcome to the chat server!" << endl;
	cout << "Please enter your name: ";
	cin >> name;
	cout<< "Please enter the name of the chat room you want to create/join: "<<endl;
	cin >> roomID;
	bool running = true;

	// Create our socket
	Socket socket("10.0.0.36", 3000);
	//Socket socket("127.0.0.1", 3000);
	ClientThread clientThread(socket, running, roomID, name);
	socket.Open();
	sleep(1);
	//To write to socket and read from socket. You may use ByteArray 
	//socket.Write(data);
	//socket.Read(data);
	
	string message;
	getline(cin,message);
	while (running) {
		getline(cin,message);
		//cin.clear();
		clientThread.sendToServer(message);	
	}
	socket.Close();

	return 0;
}
