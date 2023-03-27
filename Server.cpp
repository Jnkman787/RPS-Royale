// Joshua Matte (250957298)
// Huzaifa Ahmad (251025254)

// opens a socket server on a particular port and waits for connections
// designed in such a way that an arbitrary (undefined) number of Clients can be run at the same time
// once the Server receives the transmitted string from the Client, it does something to it, and sends it back
// doesn't matter what it does to it
// Server must terminate gracefully (either by typing something at the Server or by sending a command from a Client)

#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <algorithm>

using namespace Sync;
using namespace std;

// used to handle incoming client connections
class SocketThread : public Thread {
    private:
        Socket& socket;     // socket for the client connect
        ByteArray data;     // data received from the client
        bool &terminate;    // reference to the bool that determines whether the server should terminate
        vector<SocketThread*> &socketThreadHolder;      // reference to a vector holding all of the SocketThread objects
    
    public:
        // constructor for the SocketThread class
        SocketThread(Socket& socket, bool &terminate, vector<SocketThread*> &clientSocketThread)
        : socket(socket), terminate(terminate), socketThreadHolder(clientSocketThread) {}

        // destructor for the SocketThread class
        ~SocketThread() {
            this->terminationEvent.Wait();  // wait for thread to finish
        }

        Socket& GetSocket() {   // returns reference to the socket
            return socket;
        }

        // main function for the SocketThread class
        virtual long ThreadMain() {
            try {
                while (!terminate) {
                    socket.Read(data);  // read data from the client

                    string recv_str = data.ToString();  // convert the received data to a string

                    // make a temprary copy of the string in lowercase to check for the word "done" in both upper & lower case
					string lowercase_recv_str = recv_str;
					transform(lowercase_recv_str.begin(), lowercase_recv_str.end(), lowercase_recv_str.begin(), ::tolower);

                    if (lowercase_recv_str == "done") {     // check if the received string is "done"
                        // remove the SocketThread from the vector and set the terminate bool to true
                        socketThreadHolder.erase(remove(socketThreadHolder.begin(), socketThreadHolder.end(), this), socketThreadHolder.end());
                        terminate = true;
                        break;
                    }

                    // reverse the received string and append the original string to the end
                    string reverse_recv_str = recv_str;
                    reverse(reverse_recv_str.begin(), reverse_recv_str.end());
                    reverse_recv_str.append(" ('" + recv_str + "' reverse order equivalent)");

                    socket.Write(ByteArray(reverse_recv_str));      // send the reversed string to the client
                }
            } catch (string &s) {
                cout << s << endl;
            } catch (string err) {
                cout << err << endl;
            }

            cout << "Client has disconnected" << endl;      // print a message when the client disconnects

            return 0;
        }
};

// used to handle incoming client connections and create new SocketThread objects to handle them
class ServerThread : public Thread {
    private:
        SocketServer& server;       // SocketServer object used to accept incoming connections
        bool terminate = false;     // bool that determines whether the server should terminate   
        vector<SocketThread*> socketThreadHolder;       // vector holding all of the SocketThread objects
    
    public:
        // constructor for the ServerThread class
        ServerThread(SocketServer& server) : server(server) {}

        // destructor for the ServerThread class
        ~ServerThread() {
            // close all of the sockets
            for (auto thread : socketThreadHolder) {
                try {
                    Socket& toClose = thread->GetSocket();    // get reference to socket
                    toClose.Close();                          // close the socket
                } catch (...) {

                }
            }
            vector<SocketThread*>().swap(socketThreadHolder);   // remove all elements from the vector
            cout << "Closing client from server" << endl;
            terminate = true;
        }

        // main function for the ServerThread class
        virtual long ThreadMain() {
            while (true) {
                try {
                    Socket* newConnection = new Socket(server.Accept());    // accept a new socket connection

                    ThreadSem serverBlock(1);   // create a semaphore with initial count of 1

                    Socket& socketReference = *newConnection;   // get a reference to the new socket connection

                    // add a new SocketThread object to the SocketThreadHolder vector
                    socketThreadHolder.push_back(new SocketThread(socketReference, terminate, ref(socketThreadHolder)));
                } catch (string err) {  // catch any exceptions thrown by the code
                    return 1;
                } catch (TerminationException terminationException) {   // catch any termination exeptions thrown by the code
                    cout << "Server has shut down!" << endl;    // display the termination message
                    return terminationException;
                }
            }
            return 0;
        }
};

int main(void) {
    cout << "I am a server" << endl;
    cout << "Press ENTER to terminate the server...";
    cout.flush();   // flush the output stream to ensure all messages are printed

    SocketServer server(3000);      // create a server on port 3000

    ServerThread serverThread(server);      // create a thread to perform server operations

    FlexWait inputWaiter(1, stdin);     // create a FlexWait object to wait for input to shutdown the server
    inputWaiter.Wait();
    cin.get();

    server.Shutdown();      // shutdown and clean up the server

    cout << "Good-bye :)" << endl;
}
