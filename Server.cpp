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

void threadFunction(Socket s, vector<Socket> &socketVector, int &threadStatus) {
    ByteArray temp;
    bool execute = false;
    int bufferSize;
    while(true) {
        bufferSize = s.Read(temp);
        if(temp.ToString() == "done" || bufferSize == 0 ) {
            s.Close();
            break;
        }
        // if(temp.ToString() == "murder") {
        //     execute = true;
        //     break;
        // }
        s.Write(ByteArray("Server: " + temp.ToString()));
    }
    threadStatus += 1;
    // if (execute) {
    //     for(int x=0;x<socketVector.size();x++) {
    //         socketVector[x].Write(ByteArray("done"));
    //     }
    //     s.Close();
    //     kill(getpid(),SIGKILL);
    // }
}

void startSockServer(SocketServer &socketServer, vector<Socket> &socketVector, vector<thread> &threadVector,  int &threadStatus, ThreadSem &sockServerShutdown) {
    //handle creating the threads to respond to each connection will check termiante each time after accept if we need to kill our threads
    //kill threads by closing all open sockets.
    while(true) {
        try {
            socketVector.emplace_back(socketServer.Accept());
            // if(terminate) {
            // cout << "Terminate Was Triggered" << endl;
            // for(int x=0;x<socketVector.size();x++) {
            //     cout << "Server Closed A Socket" << endl;
            //     socketVector[x].Close();
            //     //mayben eed sleep here
            //     threadVector[x].join();
            //     cout << "Server Closed The Thread" << endl;
            // }
            // break;
            threadVector.emplace_back([&](){threadFunction(socketVector.back(),socketVector, threadStatus);});
        } catch(...) {
            cout << "Caught the Shutdown Exception" << endl;
            for(int x=0;x<socketVector.size();x++) {
                cout << "Server Closed A Socket" << endl;
                socketVector[x].Close();
                //mayben eed sleep here
                threadVector[x].join();
                cout << "Server Closed The Thread" << endl;
            }
            cout << "Finished Closing All Sockets and Socket Threads" << endl;
            break;
        }
        // cout << recv << endl;
        // cout << "Survived Accept" << endl;
        // socketVector.emplace_back(recv);
        // cout << "Got Connection" << endl;
        // if(terminate) {
        //     cout << "Terminate Was Triggered" << endl;
        //     for(int x=0;x<socketVector.size();x++) {
        //         cout << "Server Closed A Socket" << endl;
        //         socketVector[x].Close();
        //         //mayben eed sleep here
        //         threadVector[x].join();
        //         cout << "Server Closed The Thread" << endl;
        //     }
        //     break;
        // }
        // threadStatus.emplace_back(0);
        // threadVector.emplace_back([&](){threadFunction(socketVector.back(),socketVector, threadStatus.back());});
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
        if(threadVector.size() > 0) {
            // cout << "Thread Vector Size in manager is: " << threadVector.size() << endl;
            // for (int x=0;x<threadVector.size();x++) {
            //     sum += threadStatus[x];
            //     cout << threadStatus[x] << " ";
            // }
            // cout << endl;
            // cout << "Sum is: " << sum << " Connections is: " << threadVector.size() << " ThreadStatus Size: " << threadStatus << endl;
            if(threadStatus == threadVector.size()) {
                break;
            }
        }
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
    int threadStatus = 0;
    SocketServer socketServer = SocketServer(socketNum);


    masterThread.emplace_back([&](){startSockServer(socketServer, socketVector, threadVector, threadStatus, sockServerShutdown);});
    controlThread.emplace_back([&](){startThreadManager(socketServer, socketVector, threadVector, threadStatus, shutdown, sockServerShutdown);});
    shutdown.Wait();
    cout << "Main Server Closing" << endl;
    // socketServer.Shutdown();
    // sockServerShutdown.Wait();
    masterThread[0].join();
    cout << "Closed Server Thread" << endl;
    controlThread[0].join();
    cout << "Closed Control Thread" << endl;


    // int counter = 1;
    // bool first = true;
    // vector<Socket> socketVector;
    // vector<thread> threadVector;
    // SocketServer socketServer = SocketServer(2000);
    // // FlexWait flex = FlexWait(1, socketServer);

    // while(true) {
    //     socketVector.emplace_back(socketServer.Accept());
    //     // if(!first) {
    //     //     flex.Wait(1);
    //     // }
    //     // if(first) {
    //     //     thread manager(watch, socketVector, threadVector, socketServer);
    //     //     first = false;
    //     // }
    //     threadVector.emplace_back([&](){threadFunction(socketVector.back(),socketVector);});

    // }
}
