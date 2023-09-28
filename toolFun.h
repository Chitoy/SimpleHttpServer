#include <iostream>
#include <string>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

// 解析http的请求资源名称
std::string analysisHttp(const std::string &httpRequest);

// 处理http请求
std::string handleHttpRequest(const std::string &httpRequest);

// 组装Http的"Content-Length: "
void SerilizHttpReponseEnd(std::ostringstream &response, std::ifstream &file);
