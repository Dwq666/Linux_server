#include "component_server.h"

//初始化Com_Server 添加ip和port
Com_Server::Com_Server(IServerHandle *ahand, char *aip, int aport)
{
    ip_addr = aip;
    port_addr = aport;
    shand = ahand;
}
Com_Server::~Com_Server()
{
}

//启动server
int Com_Server::start()
{
    //创建socket
    if (InitSocket())
        cout << "socket(listenfd)创建成功,开始监听" << endl;
    else
        return -1;

    //开启线程，监听有无客户端连接服务端
    thread *linsten_client_thread = new thread(Listenproc, this);

    iWorkEpollfd = epoll_create(256);
    //同时开启四个线程 循环clientfd是否有数据可读
    for (int i = 1; i < 5; i++)
    {
        mythreads.push_back(thread(Workproc, this));
    }

    return 0;
}

/**
 * @brief 创建socket(listenfd)，
 * bind(listenfd)
 * listen(listenfd)
 */
bool Com_Server::InitSocket()
{
    //创建socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        cout << "创建socket(listenfd)失败" << endl;
        return false;
    }

    //设置socket为非阻塞
    setnonblocking(listenfd);

    //初始化服务器，绑定端口
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    char *local_addr = ip_addr;
    inet_aton(local_addr, &(serveraddr.sin_addr));
    serveraddr.sin_port = htons(port_addr);
    if (bind(listenfd, (sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        cout << "bind绑定失败" << endl;
        close(listenfd);
        return false;
    }

    //启动监听
    if (listen(listenfd, SOMAXCONN) == -1)
    {
        cout << "listen监听失败" << endl;
        close(listenfd);
        return false;
    }

    return true;
}

//设置非阻塞
bool Com_Server::setnonblocking(int sock)
{
    int oldSocketFlag = fcntl(sock, F_GETFL, 0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if (fcntl(sock, F_SETFL, newSocketFlag) == -1)
    {
        close(sock);
        cout << "设置非阻塞失败" << endl;
        return false;
    }

    return true;
}

//增加epollfd
void Com_Server::addfd(int epollfd, int fd, bool oneshot)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    if (oneshot)
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblocking(fd);
}

//重置修改epollfd状态
void Com_Server::reset_oneshot(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_MOD, fd, &event);
}

//删除epoll
bool Com_Server::delfd(int epollfd, int fd)
{
    bool lok= (epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, NULL) != -1);
    close(fd);
    return lok;
}

void Com_Server::ondisconnect(Client_Fd * aClient)
{
    if (delfd(iWorkEpollfd,aClient->get_fd()))
        shand->OnDisConnect(aClient);
}


//开启线程调用函数
void Com_Server::Listenproc(void *aServer)
{
    ((Com_Server *)aServer)->linsten_client();
}

void Com_Server::Workproc(void *aServer)
{
    ((Com_Server *)aServer)->clentfd_work();
}

// socket(listenfd) epoll监听
bool Com_Server::linsten_client()
{
    //创建epollfd
    int listenEpollFd = epoll_create(1);
    if (listenEpollFd == -1)
    {
        cout << "创建epoll失败" << endl;
        close(listenfd);
        return false;
    }

    addfd(listenEpollFd, listenfd, false);

    int n;
    //状态stop,要退出循环
    while (1)
    {
        epoll_event epoll_events[1024];
        n = epoll_wait(listenEpollFd, epoll_events, 1024, 1000);
        if (n < 0)
        {
            //信号中断
            if (errno == EINTR)
                continue;

            //出错，退出
            break;
        }
        else if (n == 0)
        {
            //超时，继续
            continue;
        }
        for (size_t i = 0; i < n; ++i)
        {
            if (epoll_events[i].data.fd == listenfd)
            {
                //监听socket(listenfd) 接受新连接
                struct sockaddr_in clientaddr;
                socklen_t clientaddrlen = sizeof(clientaddr);
                int clientfd = accept(listenfd, (sockaddr *)&clientaddr, &clientaddrlen);
                if (clientfd != -1)
                {
                    //设置socket(clientfd)为非阻塞
                    setnonblocking(clientfd);
                    Client_Fd *lcfd = new Client_Fd(clientfd);
                    mClients[clientfd] = lcfd;
                    addfd(iWorkEpollfd, clientfd, false);
                    shand->OnConnected(lcfd);
                }
            }
        }
    }

    return true;
}

//开启clientfd工作监听
void Com_Server::clentfd_work()
{
    while (1)
    {
        epoll_event work_epoll_events[1024];
        int ret = epoll_wait(iWorkEpollfd, work_epoll_events, 1024, 1000);
        if (ret < 0)
        {
            //信号中断
            if (errno == EINTR)
                continue;

            //出错，退出
            break;
        }
        else if (ret == 0)
        {
            //超时，继续
            continue;
        }
        for (int i = 0; i < ret; i++)
        {
            int lclientfd = work_epoll_events[i].data.fd;
            Client_Fd *lcfd = mClients.find(lclientfd)->second;
            if (work_epoll_events[i].events & EPOLLIN)
            {
                lcfd->fd_recv();
            }
        }
    }
}

int Com_Server::getEpollfd()
{
    return iWorkEpollfd;
}

void Com_Server::get_reset_oneshot(int epollfd, int fd)
{
    reset_oneshot(epollfd, fd);
}

void Com_Server::get_delfd(int epollfd, int fd)
{
    // delfd(epollfd,fd);
    // shand->OnDisConnect();
}

Client_Fd::Client_Fd(Com_Server  * aServer,int afd)
{
    mServer = aServer;
    fd = afd;
}
Client_Fd::~Client_Fd()
{
}

int Client_Fd::get_fd()
{
    return fd;
}

int Client_Fd::fd_recv()
{
    char lData[1024];
    while (true)
    {
        int m = recv(fd, &lData, 4, 0);
        if ((m == 0)
           ||  (m < 0 && errno != EWOULDBLOCK && errno != EINTR)
           )
        {
            //对端关闭了连接，从epollfd移除clientfd
            server->ondisconnect(this); //
            return 0;
        }
        else if (m < 0 && errno == EAGAIN)
        {
            //server->OnRead(this); //
            server->reset_oneshot(server->iWorkEpollfd,fd);
            shand->OnSend(lcfd);
            break;
        }
    }
    return m;
}