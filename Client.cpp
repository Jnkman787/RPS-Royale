// Joshua Matte (250957298)
// Huzaifa Ahmad (251025254)

// when executed, attempts to open a socket connection on the appropriate port
// UI: just sit and wait for user input
// once the user has typed a string, it sends the string to the Server over the socket connection
// display the changes applied to the string by the Server
// when the user enters "done", Client should terminate gracefully
// i.e., it should close the socket, clean up after itself, and terminate, without core dumps, unhandled execptions, etc.

#include "thread.h"
#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <algorithm>

using namespace Sync;
using namespace std;

class ClientThread : public Thread {
	private:
		Socket& socket;		// reference to the connected socket
		bool &active;		// boolean to track whether or not the client is active

		ByteArray data;		// data to send to the Server
		string data_str;	// string for storing the user's input
		
	public:
		// constructor for the ClientThread class
		ClientThread(Socket& socket, bool &active) : socket(socket), active(active) {}

		// destructor for the ClientThread class
		~ClientThread() {}

		// main function for the ClientThread class, called when thread is started
		virtual long ThreadMain() {
			while (true) {
				try {
					cout << "Please input a string (done to exit): ";	// prompt the user to input a string
					cout.flush();	// flush the output stream to ensure prompt is displayed before the program starts waiting for input

					getline(cin, data_str);		// get the user's input from standard input
					data = ByteArray(data_str);		// convert user input to byte array (in order to send the data as a sequence of bytes)

					// make a temprary copy of the string in lowercase to check for the word "done" in both upper & lower case
					string lowercase_data_str = data_str;
					transform(lowercase_data_str.begin(), lowercase_data_str.end(), lowercase_data_str.begin(), ::tolower);
					if (lowercase_data_str == "done") {	// check if user input is "done"
						active = false;		// set active flag to false to exit loop
						break;
					}

					// write byte array to socket (allowing data to be transmitted over the socket connection to the server)
					socket.Write(data);

					// read the response from the Server, blocking the client until data is available on the socket
					// read will return an int of # of bytes received
					int connection = socket.Read(data);

					if (connection == 0) {	// check if zero bytes were received, indicating there is no connection
						active = false;		// set active flag to false to exit loop
						break;
					}

					// display response/changes made to the string from the Server
					// byte array is converted back to a string using the ToString method
					cout << "Server Response: " << data.ToString() << endl;
				} catch (string err) {		// catch any exceptions thrown by socket operations
					cout << err << endl;	// display error message
				}
			}
			
			return 0;
		}
};

int main(void) {
	cout << "I am a client" << endl;
	bool active = true;		// initialize the active flag to true

	Socket socket("127.0.0.1", 3000);	// create socket object with server IP address and port number
	ClientThread clientThread(socket, active);		// create ClientThread object with socket and active flag
	socket.Open();		// open the socket

	while (active) {
		sleep(1);		// update every second
	}

	socket.Close();		// close the socket

	cout << "Good-bye :)" << endl;

	return 0;
}