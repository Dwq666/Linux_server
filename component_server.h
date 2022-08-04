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




class Client_Fd
{
protected:
    int fd;


public:
    Client_Fd(int afd);
    virtual ~Client_Fd();
    void fd_recv(void * aserver);//接受数据
    void fd_send();//发送数据
};




class Com_Server
{
protected:
    int listenfd; // 定义监听socket
    int iWorkEpollfd;  // 工作监听epoll句柄 
    char * ip_addr; //定义ip地址
    int port_addr;// 定义端口号
    vector<thread> mythreads;
    unordered_map<int,Client_Fd *> mClients;
    

protected:
    bool InitSocket();//创建socket(listenfd)，bind(listenfd)  listen(listenfd)
    bool setnonblocking(int sock);//设置非阻塞
    void addfd(int epollfd, int fd, bool oneshot);//增加epoll
    void reset_oneshot(int epollfd, int fd);//重置修改状态
    static void Listenproc(void * aServer);//开启监听线程
    bool linsten_client();//监听有无客户端连接，有连接创建clientfd   accept(listenfd)
    static void Workproc(void * aServer);//开启工作线程
    void clentfd_work(); //epoll（clientfd）循环是否有数据可读  
    
public:
    Com_Server(char * aip,int aport); //初始化
    virtual ~Com_Server();
    int start();//server启动
    void stop();//server停止
    int getEpollfd();
    void get_reset_oneshot(int epollfd, int fd);

};