#include "socket.h"
#include <iostream>
#include <stdlib.h>
#include <time.h>

using namespace Sync;
using namespace std;
int main(void)
{
	int bufferSize;
	ByteArray temp;
	string userIn;
	Socket clientSocket = Socket("127.0.0.1",2000);
	clientSocket.Open();
	while(true) {
		cout << "enter a string" << endl;
		cin >> userIn;
		clientSocket.Write(ByteArray(userIn));
		if(userIn == "done") {
			clientSocket.Close();
			cout << "ending client" <<endl;
			break;
		}
		// if(userIn == "murder") {
		// 	clientSocket.Close();
		// 	cout << "ending client" << endl;
		// 	break;
		// }
		bufferSize = clientSocket.Read(temp);
		if(temp.ToString() == "done" || bufferSize == 0) {
			cout << "Server Shutdown" << endl;
			clientSocket.Close();
			break;
		}
		cout << temp.ToString() << endl;
	}
}
