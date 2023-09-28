#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <string>
#include <map>

struct epollReactor
{
    /* data */
    int epfd;
    int clientfd;
    std::string strResponce;

};

// 线程A，负责向消息队列发送消息
void MsgNotify(struct epollReactor mapMsg);

// 线程B，负责接收并处理消息
void MsgHandle();
