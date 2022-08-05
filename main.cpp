#include "nanomsg_server.h"
#include "epoll_server.h"
#include "component_server.h"
using namespace std;


struct DataPack {
   int    aSize;    //数据大小
   char * mData;  //实际数据指针
   char * ptr;    //数据包内存
};


/**
 *  传一个字符串  3 字节  abc  
 * 
 *  1024字节
 *   
 *  0300000048495000
 * 
 * 
 */

//


int 

class CdwqServer:public IServerHandle
{   
public:
     //收到数据时发生的事件
    virtual void OnRead(Client_Fd *lfd,char * a,int aSize)
    {
        if ()
        
        
        pid_t ltid = gettid();
        cout<<"线程id: "<<ltid<<endl;
        cout<<"开始接受客户端clientfd: "<<lfd->get_fd()<<" 的数据"<<endl;
    }   
    //数据发送完的事件
    virtual void OnSend(Client_Fd *lfd)
    {
        cout<<"数据接收完毕"<<endl;
    }   
    //断开事件
    virtual void OnDisConnect(Client_Fd *lfd)
    {
        cout<<"客户端clientfd: "<<lfd->get_fd()<<" 已断开连接"<<endl;
    }   
    //客户端连接事件
    virtual void OnConnected(Client_Fd *lfd)
    {
        cout<<"客户端连接成功,clientfd: "<<lfd->get_fd()<<endl;
    }
};



int main()
{   
    CdwqServer lshand;
    Com_Server * lsr = new Com_Server(&lshand,"172.27.65.154",8888);  
    lsr->start();

    while (true)
    {
        sleep(1);
    }

    return 0;

}