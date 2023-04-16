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

	cout << "Waiting for opponent to enter" << endl;
	clientSocket.Read(temp);
	cout << "Opponent has entered" << endl;

	while(true) {
		cout << "Enter a option: rock, paper, scissors" << endl;
		cin >> userIn;
		clientSocket.Write(ByteArray(userIn));
		cout << "Waiting for opponent to lock in" << endl;
		if(userIn == "done") {
			clientSocket.Close();
			cout << "ending client" <<endl;
			break;
		}
		bufferSize = clientSocket.Read(temp);
		if(temp.ToString() == "done" || bufferSize == 0) {
			cout << "Game Shutdown" << endl;
			clientSocket.Close();
			break;
		}
		cout << "Result: " << temp.ToString() << endl;
	}
}
