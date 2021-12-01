#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync;
using namespace std;

class SocketThread : public Thread
{
private:
    
    Socket& socket;
    string roomID;
    string name;
    ByteArray userInputBytes;
    bool &running;
    
    
    vector<SocketThread*> &threads;
public:
    SocketThread(Socket& socket, bool &running, vector<SocketThread*> &threads)
    : socket(socket), running(running), threads(threads)
    {}

    ~SocketThread()
    {this->terminationEvent.Wait();}

    Socket& GetSocket()
    {
        return socket;
    }

    virtual long ThreadMain() {

            while(running)
            {
                
                int res = socket.Read(userInputBytes);
                if (res != 0) {
		        string data = userInputBytes.ToString();
		        if (data.rfind("/nameset",0) == 0) {
		            cout<<data<<endl;
		            name = data.erase(0,9);
		            socket.Write(ByteArray("\nName set to: "+name+"!\n"));
		        }
		        else if (data.rfind("/join",0)==0) {
		            cout<<data<<endl;
		            roomID = data.erase(0,6);
		            socket.Write(ByteArray("\nWelcome to room: "+roomID+"!\n"));
		            MessageRoom(name+" has joined the room!");
		        }
		        else if (data=="/quit") {
		            break;      
		        }
		        else {
		        	cout<<name+" - #"+roomID+": "+data<<endl;
		        	MessageRoom(name+": "+data);
		        }
		}

            }
            
        cout<<"Client Disconnected" <<endl;
        return 0;
    }
    
    string getRoomID() {
    	return roomID;
    }
    
    string getName() {
    	return name;
    }
    
    void MessageRoom(string message) {
    	for (int i =0; i<threads.size(); i++) {
    		SocketThread *clientSocketThread = threads[i];
    		if (clientSocketThread->getRoomID() == roomID && clientSocketThread->getName() != name) {
    			Socket &cliSocket = clientSocketThread->GetSocket();
    			cliSocket.Write(ByteArray(message));
    		}
    	}
    }
};
// This thread handles the server operations
class ServerThread : public Thread
{
private:
    SocketServer& server;
    
    bool running = true;
    vector<SocketThread*> threads;
    
public:
    ServerThread(SocketServer& server)
    : server(server)
    {}

    ~ServerThread()
    {
        // Cleanup
	//...
	for (auto thread: threads) {
		Socket& toClose = thread->GetSocket();
		toClose.Close();
	}
	vector<SocketThread*>().swap(threads);
	running = false;
    }

    virtual long ThreadMain()
    {
    	while(true) {
    		Socket* newConnection = new Socket(server.Accept());
    		

        	Socket& socketReference = *newConnection;
        	threads.push_back(new SocketThread(socketReference, running, ref(threads)));
    	}
    	
	return 1;
    }
};


int main(void)
{
    cout << "I am a server." << endl;

    // Create our server
    SocketServer server(3000);    

    // Need a thread to perform server operations
    ServerThread serverThread(server);
	
    // This will wait for input to shutdown the server
    FlexWait cinWaiter(1, stdin);
    cinWaiter.Wait();

    // Shut down and clean up the server
    server.Shutdown();

}
