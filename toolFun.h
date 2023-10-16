#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <curl/curl.h>
#include <regex>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <thread>
#include <sys/epoll.h>

#include <nlohmann/json.hpp>

#include "Chatgpt.h"

// struct timeval tv_begin;
// gettimeofday(&tv_begin, NULL);

using json = nlohmann::json;

// 解析http的请求资源名称
std::string analysisHttp(const std::string &httpRequest);

// 处理http请求
std::string handleHttpRequest(const std::string &httpRequest);

// 组装Http的"Content-Length: "
void SerilizHttpReponseEnd(std::ostringstream &response, std::ifstream &file);

// 调用chatgpt
// std::string SearchChatgpt(std::string strReq);

std::string extractSearchString(const std::string &input);

// 将url字符串解码
std::string urlDecode(const std::string &input);

//网络线程回调函数
void routine(void *arg);
