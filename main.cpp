#include "nanomsg_server.h"
#include "epoll_server.h"
using namespace std;

void abc()
{
    
}


int main()
{
     Epoll_Server a;
     a.Test();

     thread t1(abc);

    return 0;

}