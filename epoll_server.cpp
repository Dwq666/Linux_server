#include "epoll_server.h"

int Epoll_Server::Test()
{   
    //创建socket
    int listenfd = socket (AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        cout<<"创建socket失败"<<endl;
    }

    //设置socket为非阻塞
    setnonblocking(listenfd);

    //初始化服务器，绑定端口
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    char *local_addr="172.31.165.8";
    inet_aton(local_addr,&(serveraddr.sin_addr));
    serveraddr.sin_port=htons(8888);
    if(bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr))<0)
    {
        cout<<"绑定失败"<<endl;
        close(listenfd);
        return -1;
    }

    //启动监听
    if(listen(listenfd, SOMAXCONN) == -1)
    {   
        cout<<"监听失败"<<endl;
        close(listenfd);
        return -1;
    }

    //创建epollfd
    int epollfd = epoll_create(256);
    if(epollfd==-1)
    {
        cout<<"创建epoll失败"<<endl;
        close(listenfd);
        return -1;
    }

    epoll_event listen_fd_event;
    listen_fd_event.data.fd = listenfd;
    listen_fd_event.events = EPOLLIN;

    //使用ET模式
    //listen_fd_event.events |= EPOLLET;

    //将监听socket(listenfd)绑定epollfd上
    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,listenfd,&listen_fd_event) < 0)
    {
        cout<<"绑定失败"<<endl;
        close(listenfd);
        return -1;
    }    

    cout<<"创建成功"<<endl;
    int n;
    while (true)
    {
    
        epoll_event epoll_events[1024];
        n = epoll_wait(epollfd,epoll_events,1024,1000);
        if(n<0)
        {
            //信号中断
            if(errno==EINTR)
                continue;

            //出错，退出
            break;    
        }
        else if(n==0)
        {
            //超时，继续
            continue;
        }

        for(size_t i=0;i<n;++i)
        {   


        }




    }
    

    return 0;
}

//设置socket为非阻塞
void Epoll_Server::setnonblocking(int sock)
{
    int oldSocketFlag = fcntl(sock,F_GETFL,0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if (fcntl(sock,F_SETFL,newSocketFlag) == -1)
    {
        close(sock);
        cout<<"设置非阻塞失败"<<endl;
        exit(1);
    }
    
}