#include "EpollServer.h"
#include "toolFun.h"

extern constexpr auto HTTP_404_ERROR = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
extern CChatgpt g_Chatgpt;

#define MAX_EPOLL_CLIENTFD 1024
#define PORT_COUNT 1
#define SERVER_PORT 80

int g_sockfds[PORT_COUNT] = {0};

CEpollServer::CEpollServer()
{
    // InitServer();
}

CEpollServer::~CEpollServer()
{
    ClearEpfd(m_pEpfd);
}

int CEpollServer::RunServer()
{
    CEpfd *pEpfd = m_pEpfd;
    if (pEpfd == NULL)
        return -1;
    if (pEpfd->epfd < 0)
        return -1;
    if (pEpfd->cfdblks == NULL)
        return -1;

    struct epoll_event events[MAX_EPOLL_CLIENTFD + 1];

    int checkpos = 0, i;

    while (true)
    {

        int nready = epoll_wait(pEpfd->epfd, events, MAX_EPOLL_CLIENTFD, 1000);
        if (nready < 0)
        {
            printf("epoll_wait error, exit\n");
            continue;
        }

        for (i = 0; i < nready; i++)
        {

            CClientfd *clientfd = (CClientfd *)events[i].data.ptr;
            clientfd->callback(clientfd->fd, events[i].events, clientfd->arg);
        }
    }
}

int CEpollServer::InitServer()
{
    m_pEpfd = new CEpfd;

    CEpfd *pEpfd = m_pEpfd;
    InitEpfd(pEpfd);
    unsigned short port = SERVER_PORT;

    for (size_t sockCnt = 0; sockCnt < PORT_COUNT; sockCnt++)
    {
        // 监听端口的fd
        int lsfd = InitSocket(port + sockCnt);
        if (lsfd <= 0)
        {
            std::cout << "初始化监听端口失败" << std::endl;
            exit(0);
        }
        g_sockfds[sockCnt] = lsfd;
        // 监听端口socket绑定到epoll
        Addlistener(pEpfd, g_sockfds[sockCnt], std::bind(&CEpollServer::ClientfdAccept, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    }
    return 0;
}

int CEpollServer::InitEpfd(CEpfd *pEpfd)
{
    if (!pEpfd)
        pEpfd = new CEpfd;

    pEpfd->init();

    // 创建epfd
    pEpfd->epfd = epoll_create(1);
    if (pEpfd->epfd <= 0)
    {
        std::cout << "创建 epfd 在函数" << __func__ << strerror(errno);
        return -2;
    }

    // 分配并初始化clientfd内存
    CClientfd *pClientfds = (CClientfd *)malloc((MAX_EPOLL_CLIENTFD) * sizeof(CClientfd));
    if (pClientfds == nullptr)
    {
        std::cout << "创建epfd在函数" << __func__ << strerror(errno);
        close(pEpfd->epfd);
        return -3;
    }
    memset(pClientfds, 0, (MAX_EPOLL_CLIENTFD) * sizeof(CClientfd));

    CfdBlock *block = new CfdBlock;
    if (block == NULL)
    {
        free(pClientfds);
        close(pEpfd->epfd);
        return -4;
    }

    block->clientfd = pClientfds;
    block->next = NULL;

    pEpfd->cfdblks = block;
    pEpfd->blkcnt = 1;
    return 0;
}

// 给epfd分配clientfd的内存
int CEpollServer::AllocEpfd(CEpfd *pEpfd)
{
    if (pEpfd == nullptr)
        return -1;
    if (pEpfd->cfdblks == nullptr)
        return -1;

    CfdBlock *blk = pEpfd->cfdblks;

    while (blk->next != nullptr)
    {
        blk = blk->next;
    }

    CClientfd *pClientfds = (CClientfd *)malloc((MAX_EPOLL_CLIENTFD) * sizeof(CClientfd));
    if (pClientfds == NULL)
    {
        std::cout << __func__ << "CClientfd 失败" << std::endl;
        return -2;
    }
    memset(pClientfds, 0, (MAX_EPOLL_CLIENTFD) * sizeof(CClientfd));

    CfdBlock *block = new CfdBlock;
    if (block == NULL)
    {
        std::cout << __func__ << "CfdBlock 失败" << std::endl;
        return -3;
    }
    block->clientfd = pClientfds;
    block->next = NULL;

    blk->next = block;
    pEpfd->blkcnt++; // 内存块数+1

    return 0;
}

CClientfd *CEpollServer::IdxCEpfd(CEpfd *pEpfd, int sockfd)
{
    if (pEpfd == NULL)
        return NULL;
    if (pEpfd->cfdblks == NULL)
        return NULL;

    // 取整如果大于内存块数，则分配内存
    int blkidx = sockfd / MAX_EPOLL_CLIENTFD;
    while (blkidx >= pEpfd->blkcnt)
    {
        AllocEpfd(pEpfd);
    }

    // 指针指到到可分配的内存块
    int i = 0;
    CfdBlock *blk = pEpfd->cfdblks;
    while (i++ != blkidx && blk != NULL)
    {
        blk = blk->next;
    }

    return &blk->clientfd[sockfd % MAX_EPOLL_CLIENTFD];
}

void CEpollServer::SetClientfd(CClientfd *clientfd, int fd, CLIENTCALLBACK callback, void *arg)
{
    clientfd->fd = fd;
    clientfd->callback = callback;
    clientfd->events = 0;
    clientfd->arg = arg;
}

int CEpollServer::AddEventClientfd(int epfd, int events, CClientfd *pClientfd)
{
    struct epoll_event ep_ev = {0, {0}};
    ep_ev.data.ptr = pClientfd;
    ep_ev.events = pClientfd->events = events;

    int op;
    if (pClientfd->status == 1)
    {
        op = EPOLL_CTL_MOD;
    }
    else
    {
        op = EPOLL_CTL_ADD;
        pClientfd->status = 1;
    }

    if (epoll_ctl(epfd, op, pClientfd->fd, &ep_ev) < 0)
    {
        printf("event add failed [fd=%d], events[%d]\n", pClientfd->fd, events);
        return -1;
    }

    return 0;
}

int CEpollServer::DelEventClientfd(int epfd, CClientfd *pClientfd)
{
    struct epoll_event ep_ev = {0, {0}};

    if (pClientfd->status != 1)
    {
        return -1;
    }

    ep_ev.data.ptr = pClientfd;
    pClientfd->status = 0;
    epoll_ctl(epfd, EPOLL_CTL_DEL, pClientfd->fd, &ep_ev);

    return 0;
}

int CEpollServer::InitSocket(short port)
{
    // 创建socket
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd == -1)
    {
        return -1;
    }

    int flags = fcntl(listenfd, F_GETFL, 0);
    fcntl(listenfd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    // 绑定socket和端口
    if (-1 == bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)))
    {
        return -2;
    }
    // 监听socket
    if (listen(listenfd, 10) < 0)
    {
        std::cout << "监听socket失败：" << strerror(errno) << std::endl;
        return -1;
    }
    std::cout << "监听服务端口：" << port << std::endl;
    gettimeofday(&tv_begin, NULL);
    return listenfd;
}

int CEpollServer::ClientfdAccept(int fd, int events, void *arg)
{
    CEpfd *pEpfd = (CEpfd *)arg;
    if (pEpfd == NULL)
        return -1;

    struct sockaddr_in client_addr;
    socklen_t len = sizeof(client_addr);

    int clientfd;
    // 接收客户端的fd
    if ((clientfd = accept(fd, (struct sockaddr *)&client_addr, &len)) == -1)
    {
        if (errno != EAGAIN && errno != EINTR)
        {
        }
        std::cout << __func__ << ":accept:" << strerror(errno) << std::endl;
        return -1;
    }

    // 设置客户端为非阻塞
    int flags = fcntl(clientfd, F_GETFL, 0);
    if ((flags = fcntl(clientfd, F_SETFL, flags | O_NONBLOCK)) < 0)
    {
        std::cout << __func__ << ":fcntl:"
                  << "nonblocking failed" << MAX_EPOLL_CLIENTFD << std::endl;
        return -1;
    }

    // 给其分配内存
    CClientfd *structClientfd = IdxCEpfd(pEpfd, clientfd);

    if (structClientfd == NULL)
        return -1;

    // 设置fd参数和回调函数
    SetClientfd(structClientfd, clientfd, std::bind(&CEpollServer::ClientfdRecv, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), pEpfd);

    //  添加fd到epfd
    AddEventClientfd(pEpfd->epfd, EPOLLIN | EPOLLET, structClientfd);
    // nty_event_add(reactor->epfd, EPOLLIN, event);

    // //建立1000个连接耗时
    // if (curfds++ % 1000 == 999) {
    // 	struct timeval tv_cur;
    // 	memcpy(&tv_cur, &tv_begin, sizeof(struct timeval));

    // 	gettimeofday(&tv_begin, NULL);

    // 	int time_used = TIME_SUB_MS(tv_begin, tv_cur);
    // 	printf("connections: %d, sockfd:%d, time_used:%d\n", curfds, clientfd, time_used);
    // }

    // printf("new connect [%s:%d], pos[%d]\n",
    //	inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), clientfd);

    return 0;
}

int CEpollServer::ClientfdRecv(int fd, int events, void *arg)
{
    CEpfd *pEpfd = (CEpfd *)arg;
    CClientfd *structClientfd = IdxCEpfd(pEpfd, fd);

    if (structClientfd == NULL)
        return -1;
    // 下面的逻辑需要优化，主要是接受完后要设置fd为send的回调函数
    std::string strRequest;
    while (true)
    {
        int len = recv(fd, structClientfd->buffer, BUFFER_LENGTH, 0);
        if (len > 0)
        {
            std::string receivedData(structClientfd->buffer, len);
            strRequest += receivedData;
        }
        else if (len == -1 && errno == EAGAIN)
        {
            // 暂时没有更多数据可读，退出循环等待下一次 EPOLLIN 事件
            break;
        }
        else if (len == 0)
        {
            DelEventClientfd(pEpfd->epfd, structClientfd);
            close(fd);
            std::cout << "客户端断开关闭连接:" << structClientfd->fd << std::endl;
            return -1;
        }
        else
        {
            // 处理接收错误
            perror("recv");
            DelEventClientfd(pEpfd->epfd, structClientfd);
            close(fd);
            return -2;
        }
    }

    // 解析请求数据
    structClientfd->strRequest=strRequest;
    //std::cout << "接收到的数据：" << structClientfd->strRequest << std::endl;
    //structClientfd->strResponse = structClientfd->strRequest;
    structClientfd->strResponse = handleHttpRequest(structClientfd->strRequest);

    // 更改回调函数为send
    SetClientfd(structClientfd, fd, std::bind(&CEpollServer::ClientfdSend, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3), pEpfd);
    // 更改epollfd事件状态
    AddEventClientfd(pEpfd->epfd, EPOLLOUT, structClientfd);
    return 0;
}

int CEpollServer::ClientfdSend(int fd, int events, void *arg)
{
    CEpfd *pEpfd = (CEpfd *)arg;
    CClientfd *structClientfd = IdxCEpfd(pEpfd, fd);

    if (structClientfd == NULL)
        return -1;

    int totalSent = 0;
    int dataLength = structClientfd->strResponse.length();

    while (totalSent < dataLength)
    {
        int bytesToSend = std::min(dataLength - totalSent, BUFFER_LENGTH); // 每次发送的数据大小

        int len = send(fd, structClientfd->strResponse.c_str() + totalSent, bytesToSend, MSG_NOSIGNAL);
        if (len == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 资源暂时不可用，稍后重试
                continue;
            }
            else if (errno == EPIPE)
            {
                DelEventClientfd(pEpfd->epfd, structClientfd);
                close(fd);
                std::cout << "浏览器主动关闭连接:" << fd << std::endl;
            }
            else
            {
                // 发生其他错误，需要处理错误并退出循环
                perror("send");
                break;
            }
        }
        totalSent += len;
    }

    //std::cout << "发送的数据：" << structClientfd->strResponse << std::endl;
    // 因为是Http所以处理完数据后关闭连接
    DelEventClientfd(pEpfd->epfd, structClientfd);
    close(fd);
    std::cout << "数据发送完毕，关闭连接:" << fd << std::endl;
    return 0;
}

int CEpollServer::Addlistener(CEpfd *pEpfd, int sockfd, CLIENTCALLBACK acceptor)
{

    if (pEpfd == NULL)
        return -1;
    if (pEpfd->cfdblks == NULL)
        return -1;

    CClientfd *structClientfd = IdxCEpfd(pEpfd, sockfd);
    if (structClientfd == NULL)
        return -1;

    SetClientfd(structClientfd, sockfd, acceptor, pEpfd);
    AddEventClientfd(pEpfd->epfd, EPOLLIN, structClientfd);

    return 0;
}

int CEpollServer::ClearEpfd(CEpfd *pEpfd)
{
    close(pEpfd->epfd);

    CfdBlock *blk = pEpfd->cfdblks;
    CfdBlock *blk_next;
    while (blk != NULL)
    {
        blk_next = blk->next;

        free(blk->clientfd);
        delete blk;

        blk = blk_next;
    }

    for (int i = 0; i < PORT_COUNT; i++)
    {
        close(g_sockfds[i]);
    }
    delete m_pEpfd;

    return 0;
}

std::string CEpollServer::handleHttpRequest(const std::string &httpRequest)
{
    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\n";

    std::string strSrc = analysisHttp(httpRequest);
    if (strSrc == "/")
    {
        std::ifstream file("/home/Pys/Cpp/SeverHttp/build/src/Homepage.html");
        if (!file.is_open())
        {
            return HTTP_404_ERROR;
        }
        response << "Content-Type: text/html\r\n";
        SerilizHttpReponseEnd(response, file);
        return response.str();
    }
    else if (strSrc.find(".html") != std::string::npos)
    {
        std::string strHtml = "src" + strSrc;
        std::ifstream file(strHtml);
        if (!file.is_open())
        {
            return HTTP_404_ERROR;
        }
        response << "Content-Type: text/html\r\n";
        SerilizHttpReponseEnd(response, file);
        return response.str();
    }
    else if ((strSrc.find(".png") != std::string::npos) ||
             (strSrc.find(".jpg") != std::string::npos) ||
             (strSrc.find(".ico") != std::string::npos))
    {
        std::string strHtml = "src" + strSrc;
        std::ifstream file(strHtml);
        if (!file.is_open())
        {
            return HTTP_404_ERROR;
        }
        response << "Content-Type: image/png\r\n";
        SerilizHttpReponseEnd(response, file);
        return response.str();
    }
    else if (strSrc.find(".mp4") != std::string::npos)
    {
        std::string strHtml = "src" + strSrc;
        std::ifstream file(strHtml);
        if (!file.is_open())
        {
            return HTTP_404_ERROR;
        }
        response << "Content-Type: video/mp4\r\n";
        SerilizHttpReponseEnd(response, file);
        return response.str();
    }
    else if (strSrc.find("search") != std::string::npos)
    {
        std::string strQues = urlDecode(extractSearchString(strSrc));
        std::string strChatgpt = g_Chatgpt.SearchChatgpt(strQues);
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " + std::to_string(strChatgpt.length()) + "\r\n";
        response << "\r\n";
        response << strChatgpt;
        return response.str();
    }
    else
    {
        // 处理其他路径的HTTP请求，可以根据需要添加更多逻辑
        return HTTP_404_ERROR;
    }
}
