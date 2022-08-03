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
    char *local_addr="172.31.161.106";
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

    cout<<"创建成功1"<<endl;
    thread t1(Listenproc,this);

    for (int i=0;i<4;i++)
    {
        thread * lt = new thread(Workproc,this);
    }

    
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
            //事件可读
            if(epoll_events[i].events & EPOLLIN)
            {
                if(epoll_events[i].data.fd == listenfd)
                {
                    //监听socket(listenfd) 接受新连接
                     struct sockaddr_in clientaddr;
                     socklen_t clientaddrlen = sizeof(clientaddr);
                     int clientfd = accept(listenfd,(sockaddr*)&clientaddr,&clientaddrlen);
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
                              
                            
                         }
                         else
                         {
                            cout<<"添加客户端到epollfd失败"<<endl;
                            close(clientfd);
                         }

                     }
                }
                else
                {
                    cout<<"client fd :"<<epoll_events[i].data.fd<<"recv data."<<endl;
                    //普通chlientfd
                    char ch[100];
                    int m = recv(epoll_events[i].data.fd,&ch,100,0);
                    if(m==0)
                    {
                        //对端关闭了连接，从epollfd移除clientfd
                        if(epoll_ctl(epollfd,EPOLL_CTL_DEL,epoll_events[i].data.fd,NULL) != -1)
                        {
                            cout<<"客户端移除成功,cliend:"<<epoll_events[i].data.fd<<endl;
                        }

                        close(epoll_events[i].data.fd);
                    }   
                    else if(m<0)
                    {
                        //出错
                        if(errno!=EWOULDBLOCK && errno != EINTR)
                        {
                            if(epoll_ctl(epollfd,EPOLL_CTL_DEL,epoll_events[i].data.fd,NULL) != -1)
                            {
                                cout<<"客户端移除成功,cliend:"<<epoll_events[i].data.fd<<endl;
                            }
                            close(epoll_events[i].data.fd);
                        }
                    }
                    else 
                    {
                        //正常收到数据
                        cout<<"recv from client :"<<epoll_events[i].data.fd<<",   数据为:"<<ch<<endl;

                    }
                }
            }
            else if(epoll_events[i].data.fd & EPOLLERR)
            {
                  //暂不处理  
            }

        }

    }
    
    close(listenfd);
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

void Epoll_Server::ExecListen()
{
    cout<<"新的监听线程"<<endl;
    while(true)
    {
        sleep(1);
    }
}


void Epoll_Server::RunWork()
{
    cout<<"新的工作线程"<<endl;
    while(true)
    {
        sleep(1);
    }
}



 void Epoll_Server::Listenproc(void * aServer)
 {
    ((Epoll_Server*)aServer)->ExecListen();
    //cout<<"新的线程"<<endl;
 }

 void Epoll_Server::Workproc(void * aServer)
 {
    ((Epoll_Server*)aServer)->RunWork();
    //cout<<"新的线程"<<endl;
 }
 