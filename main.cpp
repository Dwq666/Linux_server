#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../nanomsg/src/pair.h"
#include "../nanomsg/src/bus.h"
#include "../nanomsg/src/nn.h"
using namespace std;

#define cLen 100 



int main()
{
    char* lUrl = "tcp://172.18.30.239:8888";
    int server_sock=0;
    char buf[cLen] = {0};

    if(server_sock = nn_socket(AF_SP,NN_PAIR) < 0)
    {
        cout<<"create server socket failed!"<<endl;
        return -1;
    }
    
    if(nn_bind(server_sock,lUrl)<0)
    {
        cout<<"bind server sock failed!"<< errno <<endl ;
        nn_close(server_sock);
        return -1;
    }

    cout<<"server init success!"<<endl;

    while(1)
    {
        if(nn_recv(server_sock,buf,sizeof(buf),0) < 0)
        {
            cout<<"recv failed!"<<endl;
            nn_close(server_sock);
            exit(EXIT_FAILURE);
        }
        else
        {
            cout<<("recieve client msg: %s", buf)<<endl;
            if(nn_send(server_sock,buf,sizeof(buf),0)<0)
            {
                cout<<"send  failed!"<<endl;
                nn_close(server_sock);
                exit(EXIT_FAILURE);
            }
        }
    }

    nn_close(server_sock);
    return 0;

}