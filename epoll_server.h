#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <thread>
using namespace std;

class Epoll_Server
{
public:
    int Test();
    void setnonblocking(int sock);
    void ExecListen();
    void RunWork();
    static void Listenproc(void * aServer);
    static void Workproc(void * aServer);
};

