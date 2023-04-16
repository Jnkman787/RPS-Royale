#include "thread.h"
#include "socketserver.h"
#include <stdlib.h>
#include <time.h>
#include <list>
#include <vector>
#include <thread>
#include <future>
#include <signal.h>

using namespace Sync;
using namespace std;

int rockPaperScissors(string arr[]) {
    // Check for ties
    if (arr[0] == arr[1]) {
        return 0;
    }
    // Check for rock-paper and paper-scissors combinations
    if ((arr[0] == "rock" && arr[1] == "paper") || 
        (arr[0] == "paper" && arr[1] == "scissors") ||
        (arr[0] == "scissors" && arr[1] == "rock")) {
        return 2;
    }
    // All other combinations are won by the first player
    return 1;
}


void socketHandler(Socket s, int &ThreadStatus, string &playerInput) {
    int bufferSize = -1;
    ByteArray msg;
    while(true) {
        bufferSize = s.Read(msg);
        if(msg.ToString() == "done" || bufferSize == 0) {
            s.Close();
            playerInput = "done";
            break;
        }
        playerInput = msg.ToString();
    }
    // cout << "TheadStatus Inc" << endl;
    ThreadStatus++;
}

void gameHandler(vector<Socket> &playerPair, int &threadStatus) {
    int result;
    int p1Index = playerPair.size()-1;
    int p2Index = playerPair.size()-2;
    string input[2] = {"", ""};
    vector<thread> playerVector;

    //spin up 2 threads to handle inputs
    playerVector.emplace_back([&](){socketHandler(playerPair[p1Index], threadStatus, input[0]);});
    playerVector.emplace_back([&](){socketHandler(playerPair[p2Index], threadStatus, input[1]);});

    while(true) {
        if((input[0] == "rock" || input[0] == "paper" || input[0] == "scissors") && (input[1] == "rock" || input[1] == "paper" || input[1] == "scissors")) {
            // cout << "Valid Inputs Detected" << endl;
            result = rockPaperScissors(input);
            if(result == 1) {
                playerPair[p1Index].Write(ByteArray("Winner"));
                playerPair[p2Index].Write(ByteArray("Loser"));
            }
            else if(result == 2) {
                playerPair[p1Index].Write(ByteArray("Loser"));
                playerPair[p2Index].Write(ByteArray("Winner"));
            }
            else {
                playerPair[p1Index].Write(ByteArray("Tie"));
                playerPair[p2Index].Write(ByteArray("Tie"));
            }
            //reset game
            input[0] = "";
            input[1] = "";
        }

        if(input[0] == "done" && input[1] == "done") {
            // cout << "done Detected" << endl;
            playerVector[1].join();
            playerVector[0].join();
            // cout << "joined" << endl;
            break;
        }

    }
}

void startSockServer(SocketServer &socketServer, vector<Socket> &socketVector, vector<thread> &threadVector,  int &threadStatus, ThreadSem &sockServerShutdown) {
    int pairCount = 0;
    //handle creating the threads to respond to each connection will check termiante each time after accept if we need to kill our threads
    //kill threads by closing all open sockets.
    while(true) {
        try {
            
            socketVector.emplace_back(socketServer.Accept());
            pairCount++;

            //we got 2 players
            if(pairCount == 2) {
                //signal the player pair to start by writting any junk message
                socketVector[socketVector.size()-1].Write(ByteArray("Start"));
                socketVector[socketVector.size()-2].Write(ByteArray("Start"));


                // cout << "Created new game with pairs" << endl;
                //socketPair is now the pair of 2 sockets that have connected to me
                // pair<Socket, Socket> socketPair(socketVector.end()[-2], socketVector.back());
                //reset paircount to make new pair
                pairCount = 0;
                threadVector.emplace_back([&](){gameHandler(socketVector, threadStatus);});
            }
            // threadVector.emplace_back([&](){threadFunction(socketVector.back(), threadStatus);});
        } catch(...) {
            cout << "Caught the Shutdown Exception" << endl;
            for(int x=0;x<socketVector.size();x++) {
                cout << "Server Closed A Socket" << endl;
                socketVector[x].Close();
                //close all the socket listeners as well
                sleep(1);
            }
            for(int x=0;x<threadVector.size();x++) {
                cout << "Server Closed A Thread" << endl;
                threadVector[x].join();
            }
            cout << "Finished Closing All Sockets and Socket Threads" << endl;
            break;
        }
    }
    cout << "Finished Socket Server Shutdown" << endl;
    sockServerShutdown.Signal();
}

void startThreadManager(SocketServer &socketServer, vector<Socket> &socketVector, vector<thread> &threadVector, int &threadStatus, ThreadSem &shutdown, ThreadSem &sockServerShutdown) {
    //checks to see if the threads are actually running 
    //if all threads are idle then kill server
    // time.sleep(5);
    while(true) {
        int sum = 0;
        //termination only possible if we have threads
        if(socketVector.size() > 0) {
            // cout << "SocketVector Size: " << socketVector.size() <<  " ThreadStatus: " << threadStatus << endl;
            if(threadStatus == socketVector.size()) {
                break;
            }
        }
        sleep(2);
    }
    //signal terminate
    cout << "Called for socketServer Shutdown" << endl;
    socketServer.Shutdown();
    cout << "Waiting for Socker Server To finish Shutdown" << endl;
    sockServerShutdown.Wait();
    //signal main to kill
    cout << "Signal Main To Shutdown" << endl;
    shutdown.Signal();
}

int main(void) {
    ThreadSem shutdown = ThreadSem(0);
    ThreadSem sockServerShutdown = ThreadSem(0);
    int socketNum = 2000;
    bool terminate = false;
    vector<thread> masterThread;
    vector<thread> controlThread;
    vector<Socket> socketVector;
    vector<thread> threadVector;
    // vector<thread> playerVector;
    int threadStatus = 0;
    SocketServer socketServer = SocketServer(socketNum);


    masterThread.emplace_back([&](){startSockServer(socketServer, socketVector, threadVector, threadStatus, sockServerShutdown);});
    controlThread.emplace_back([&](){startThreadManager(socketServer, socketVector, threadVector, threadStatus, shutdown, sockServerShutdown);});
    shutdown.Wait();
    cout << "Main Server Closing" << endl;
    masterThread[0].join();
    cout << "Closed Server Thread" << endl;
    controlThread[0].join();
    cout << "Closed Control Thread" << endl;
}
