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
#include <vector>
#include <unordered_map>
#include <queue>
#include <mutex>
using namespace std;

class Client_Fd;
class Com_Server;

//事件处理
class IServerHandle
{
public:
     //收到数据时发生的事件
    virtual void OnRead(Client_Fd *lfd)=0;    
    //数据发送完的事件
    virtual void OnSend(Client_Fd *lfd)=0;    
    //断开事件
    virtual void OnDisConnect(Client_Fd *lfd)=0;    
    //客户端已经连接时间
    virtual void OnConnected(Client_Fd *lfd)=0; 
};



class Client_Fd
{
protected:
    int fd;
    Com_Server  * mServer;
public:
    Client_Fd(Com_Server  * aServer,int afd);
    virtual ~Client_Fd();
    int fd_recv();//接受数据
    void fd_send();//发送数据
    int get_fd();
};




class Com_Server
{
friend Client_Fd;     
protected:
    int listenfd; // 定义监听socket
    int iWorkEpollfd;  // 工作监听epoll句柄 
    char * ip_addr; //定义ip地址
    int port_addr;// 定义端口号
    IServerHandle * shand;
    vector<thread> mythreads;
    unordered_map<int,Client_Fd *> mClients;
    

protected:
    bool InitSocket();//创建socket(listenfd)，bind(listenfd)  listen(listenfd)
    bool setnonblocking(int sock);//设置非阻塞
    void addfd(int epollfd, int fd, bool oneshot);//增加epoll
    void reset_oneshot(int epollfd, int fd);//修改时间状态
    bool delfd(int epollfd, int fd);//删除epoll
    static void Listenproc(void * aServer);//开启监听线程
    bool linsten_client();//监听有无客户端连接，有连接创建clientfd   accept(listenfd)
    static void Workproc(void * aServer);//开启工作线程
    void clentfd_work(); //epoll（clientfd）循环是否有数据可读
    void ondisconnect(Client_Fd * aClient);
    void get_reset_oneshot(int epollfd, int fd);
    void get_delfd(int epollfd, int fd);
    int  getEpollfd();
    
public:
    Com_Server(IServerHandle * ahand,char * aip,int aport); //初始化
    virtual ~Com_Server();
    int start();//server启动
    void stop();//server停止
};