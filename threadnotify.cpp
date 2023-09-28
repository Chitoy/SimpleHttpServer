#include "threadnotify.h"
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
#include <map>

std::mutex mtx;                                  // 互斥锁
std::condition_variable cv;                      // 条件变量
std::queue<struct epollReactor> messages; // 消息队列

void MsgNotify(struct epollReactor mapMsg)
{
    {
        std::lock_guard<std::mutex> lock(mtx); // 锁住互斥锁
        messages.push(mapMsg);
    }

    cv.notify_one(); // 通知等待的线程B
}

// 线程B，负责接收并处理消息
void MsgHandle()
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []
            { return !messages.empty(); }); // 等待消息队列非空

    // 取出队列中的消息
    // std::string message = messages.front()[0];
    // messages.pop();

    // 此时消息已准备好
    // std::cout << "Thread B received message: " << message << std::endl;

    std::cout << "发送数据+++++++++++++++++++++++=" << std::endl;

    struct epollReactor mapResponce=messages.front();
    int totalSent = 0;
    int dataLength = mapResponce.strResponce.length();

    while (totalSent < dataLength)
    {
        int epoll_ret = send(mapResponce.clientfd, mapResponce.strResponce.c_str() + totalSent, dataLength - totalSent, 0);
        if (epoll_ret == -1)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // 资源暂时不可用，稍后重试
                continue;
            }
            else
            {
                // 发生其他错误，需要处理错误并退出循环
                perror("send");
                break;
            }
        }
        totalSent += epoll_ret;
    }

    // 因为是Http所以处理完数据后关闭连接
    close(mapResponce.clientfd);
    epoll_ctl(mapResponce.epfd, EPOLL_CTL_DEL, mapResponce.clientfd, nullptr);
}
