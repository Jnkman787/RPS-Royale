"# se3313-2017-Lab4" 

You have make and make clean 

To run the program simply do ./Server to start a server and in a new windows ./Client to run a client

The program will wait until 2 clients have joined and will pair them off once the clients have been paired they will allow users to 
input a option between rock, paper, and scissors, one each person has locked in a option the game will run and return the results
the game will continue forever until both clients type in done in which case the game will close

Limitation in code 1
first off the way i made the server kill itself is by making it so that once the number of created sockets == number of closed sockets 
indicated by threadStatus, the server will start to clean up all sockets and threads it however cannot be shutdown unless all client 
manually shut themselves down.

Limitation in code 2
i cant get it to work such that one player can call done and the 2nd player will automatically close since the second player is going to 
sit on a block call (waiting for them to pick rock paper or scissors) technically what you can do is here 

```
        if(input[0] == "done" && input[1] == "done") {
            // cout << "done Detected" << endl;
            playerVector[1].join();
            playerVector[0].join();
            // cout << "joined" << endl;
            break;
        }
```

make it such that it writes to the other player "done" the player clients should have a clause that detects if the server wrote done to it
when the server writes done to the other client have it kill itself. Now the only problem with this however is that the client that didnt 
originally put in done is still going to sit at the make a selection part, so its kinda the same thing since the client has to make a selection
before read even becomes available, in which case may as well have both of them input done since they both have to get past the make a selection
part of the code before they can even take in a server input otherwise unless you have a way to bypass that cin line magically when the server
want to kill the other client then idk how to do this.