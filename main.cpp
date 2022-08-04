#include "nanomsg_server.h"
#include "epoll_server.h"
#include "component_server.h"
using namespace std;


int main()
{
    Com_Server * lsr = new Com_Server("172.31.160.185",8888);  
    lsr->start();

    while (true)
    {
        sleep(1);
    }

    return 0;

}