#include "component_server.h"

//初始化Com_Server 添加ip和port
Com_Server::Com_Server(char * aip,int aport)
{
    ip_addr = aip;
    port_addr = aport;
}
Com_Server::~Com_Server()
{
    
}

//启动server
int Com_Server::start()
{   
    //创建socket
    if(InitSocket())
        cout<<"socket(listenfd)创建成功,开始监听"<<endl;
    else
        return -1;

    //开启线程，监听有无客户端连接服务端
    thread * linsten_client_thread = new thread(Listenproc,this);
    
    //同时开启四个线程 循环clientfd是否有数据可读
    for(int i=1;i<5;i++)
    {
        mythreads.push_back(thread(Workproc,this));
    }  

    for(int j=0;j<mythreads.size();j++)
    {
        mythreads[j];
    }
        

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
        cout<<"创建socket(listenfd)失败"<<endl;
        return false;
    }

    //设置socket为非阻塞
    setnonblocking(listenfd);

    //初始化服务器，绑定端口
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    char *local_addr=ip_addr;
    inet_aton(local_addr,&(serveraddr.sin_addr));
    serveraddr.sin_port=htons(port_addr);
    if(bind(listenfd,(sockaddr *)&serveraddr, sizeof(serveraddr))<0)
    {
        cout<<"bind绑定失败"<<endl;
        close(listenfd);
        return false;
    } 

    //启动监听
    if(listen(listenfd, SOMAXCONN) == -1)
    {   
        cout<<"监听失败"<<endl;
        close(listenfd);
        return false;
    }  

    return true;
}


//设置非阻塞 
bool Com_Server::setnonblocking(int sock)
{
    int oldSocketFlag = fcntl(sock,F_GETFL,0);
    int newSocketFlag = oldSocketFlag | O_NONBLOCK;
    if (fcntl(sock,F_SETFL,newSocketFlag) == -1)
    {
        close(sock);
        cout<<"设置非阻塞失败"<<endl;
        return false;
    }

    return true;
}

//开启监听线程
void Com_Server::Listenproc(void * aServer)
{
    ((Com_Server*)aServer)->linsten_client();
}

void Com_Server::Workproc(void * aServer)
{
    ((Com_Server*)aServer)->clentfd_work();
}

bool Com_Server::linsten_client()
{   
    //创建epollfd
    epollfd = epoll_create(256);
    if(epollfd==-1)
    {
        cout<<"创建epoll失败"<<endl;
        close(listenfd);
        return false;
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
        return false;
    } 
    
    int n;
    while(1)
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
            if(epoll_events[i].data.fd == listenfd)
            {
                //监听socket(listenfd) 接受新连接
                struct sockaddr_in clientaddr;
                socklen_t clientaddrlen = sizeof(clientaddr);
                clientfd = accept(listenfd,(sockaddr*)&clientaddr,&clientaddrlen);
                if(clientfd != -1)
                {
                    //设置socket(clientfd)为非阻塞
                    setnonblocking(clientfd);

                    epoll_event clientfd_fd_event;
                    clientfd_fd_event.data.fd = clientfd;
                    clientfd_fd_event.events = EPOLLIN;

                    //使用ET模式
                    //clientfd_fd_event.events |= EPOLLET;
    
                    if(epoll_ctl(epollfd,EPOLL_CTL_ADD,clientfd,&clientfd_fd_event) != -1)
                    {
                        cout<<"新的客户端连接 , cliendfd :"<<clientfd<<endl;
                        Client_Fd * lcfd = new Client_Fd(clientfd);
                        mClients[clientfd] = lcfd;
                    }
                    else
                    {
                        cout<<"添加客户端到epollfd失败"<<endl;
                        close(clientfd);
                    }

                }
            }
        }

    }

}

void Com_Server::clentfd_work()
{
    while(1)
    {   
       

        //如果队列为空的话
        if(!que_workfd.empty())
        {
            int lworkfd = que_workfd.front();
            que_workfd.pop();
            Client_Fd * lfd =  mClients.find(lworkfd)->second;
            lfd->fd_recv(this); 
        }

     

        
       
    }
}

int Com_Server::getEpollfd()
{
    return epollfd;
}




Client_Fd::Client_Fd(int afd)
{
    fd = afd;
}
Client_Fd::~Client_Fd()
{

}

void Client_Fd::fd_recv(void * aserver)
{
    cout<<"client fd: "<<fd<<" 开始读取数据"<<endl;
    char ch[100];
    int m = recv(fd,&ch,100,0);
    if(m==0)
    {
        //对端关闭了连接，从epollfd移除clientfd
        if(epoll_ctl(((Com_Server*)aserver)->getEpollfd(),EPOLL_CTL_DEL,fd,NULL) != -1)
        {
            cout<<"关闭了连接，客户端移除成功,cliend: "<<fd<<endl;
        }

         close(fd);
    }   
    else if(m<0)
    {
        //出错
        if(errno!=EWOULDBLOCK && errno != EINTR)
        {
            if(epoll_ctl(((Com_Server*)aserver)->getEpollfd(),EPOLL_CTL_DEL,fd,NULL) != -1)
            {
                cout<<"连接错误，客户端移除成功,cliend: "<<fd<<endl;
            }
            close(fd);
        }
    }
    else
    {
        cout<<"clienfd: "<<fd<<",   数据为:"<<ch<<endl;
    }

}