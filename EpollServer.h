#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <string>
#include <cstring>
#include <thread>
#include <sys/epoll.h>
#include <sys/time.h>
#include <functional>

#define BUFFER_LENGTH 1024
#define RESOURCE_LENGTH 1024

using CLIENTCALLBACK = std::function<int(int, int, void *)>;

// 对应每个连接到服务器的fd
typedef struct CClientfd
{
    int fd;
    int events;
    void *arg;
    // int (*callback)(int fd, int events, void *arg); // 该fd的回调函数
    CLIENTCALLBACK callback;

    int status; // 是否受epfd管理，默认值为0----不受管理 ，1----受fd管理
    char buffer[BUFFER_LENGTH];

    char wbuffer[BUFFER_LENGTH];
    std::string strRequest;
    std::string strResponse;

    int length;
    int wlength;
    // long last_active;

    // http reqeust
    int method;
    char resource[RESOURCE_LENGTH];
} CClientfd;

// fd链表,管理clientfd
typedef struct CfdBlock
{
    struct CfdBlock *next;
    CClientfd *clientfd;
} CfdBlock;

// epfd实现管理，epfd下所有的clientfd
typedef struct CEpfd
{
    int epfd;
    int blkcnt;

    CfdBlock *cfdblks;

    void init()
    {
        epfd = -1;
        blkcnt = -1;
        cfdblks = nullptr;
    }

} CEpfd;

// 服务器类对象
class CEpollServer
{
public:
    CEpollServer();
    ~CEpollServer();

    // 初始化服务器,后续可改为指定端口和端口号
    int InitServer();
    // Epoll_Wait封装
    int RunServer();

protected:
    int ClearEpfd(CEpfd *pEpfd);
    // 初始化epfd,并分配内存
    int InitEpfd(CEpfd *pEpfd);
    int AllocEpfd(CEpfd *pEpfd);
    // 获取fd在内存块中的位置指针
    CClientfd *IdxCEpfd(CEpfd *pEpfd, int sockfd);

    // 设置CClientfd
    void SetClientfd(CClientfd *clientfd, int fd, CLIENTCALLBACK callback, void *arg);
    // 添加fd和事件到epfd,并更改CClientfd的state=1,若事件已添加，则是更改事件状态
    int AddEventClientfd(int epfd, int events, CClientfd *pClientfd);
    // 从epfd中移除fd，并更改CClientfd的state=0
    int DelEventClientfd(int epfd, CClientfd *ev);

    // fd的accept回调函数
    int ClientfdAccept(int fd, int events, void *arg);
    // fd的recv回调函数
    int ClientfdRecv(int fd, int events, void *arg);
    // fd的send回调函数
    int ClientfdSend(int fd, int events, void *arg);
    // 创建监听端口并返回fd
    int InitSocket(short port);
    // 添加fd监听回调函数
    int Addlistener(CEpfd *pEpfd, int sockfd, CLIENTCALLBACK acceptor);

    // 处理http请求
    std::string handleHttpRequest(const std::string &httpRequest);

private:
    CEpfd *m_pEpfd = nullptr;
    struct timeval tv_begin;
};
