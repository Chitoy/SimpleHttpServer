#include "toolFun.h"

CChatgpt g_Chatgpt;

constexpr auto HTTP_404_ERROR = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";

// 将url字符串解码
std::string urlDecode(const std::string &input)
{
    std::ostringstream decoded;
    for (std::string::size_type i = 0; i < input.length(); ++i)
    {
        if (input[i] == '%')
        {
            if (i + 2 < input.length())
            {
                char decodedChar = std::stoi(input.substr(i + 1, 2), nullptr, 16);
                decoded << decodedChar;
                i += 2;
            }
        }
        else if (input[i] == '+')
        {
            decoded << ' ';
        }
        else
        {
            decoded << input[i];
        }
    }
    return decoded.str();
}

std::string extractSearchString(const std::string &input)
{
    std::string searchString = "/search/";
    size_t startPos = input.find(searchString);

    if (startPos != std::string::npos)
    {
        // 找到了匹配的子字符串
        startPos += searchString.length();         // 移动到搜索字符串的末尾
        size_t endPos = input.find('/', startPos); // 寻找下一个斜杠

        if (endPos != std::string::npos)
        {
            // 找到了下一个斜杠，提取子字符串
            return input.substr(startPos, endPos - startPos);
        }
        else
        {
            // 没有找到下一个斜杠，返回从起始位置到字符串末尾的子字符串
            return input.substr(startPos);
        }
    }

    // 如果没有匹配的子字符串，返回空字符串
    return "";
}

std::string analysisHttp(const std::string &httpRequest)
{
    std::string getField;

    // 查找"GET "的位置
    size_t getPos = httpRequest.find("GET ");
    if (getPos != std::string::npos)
    {
        // 找到"GET "后的第一个空格位置
        size_t spacePos = httpRequest.find(" ", getPos + 4);

        if (spacePos != std::string::npos)
        {
            // 提取GET字段
            getField = httpRequest.substr(getPos + 4, spacePos - getPos - 4);
        }
    }

    return getField;
}

void SerilizHttpReponseEnd(std::ostringstream &response, std::ifstream &file)
{
    // 获取文件大小
    file.seekg(0, std::ios::end);
    int fileSize = file.tellg();
    file.seekg(0, std::ios::beg);
    response << "Content-Length: " + std::to_string(fileSize) + "\r\n";

    response << "\r\n";
    response << file.rdbuf();

    file.close();
}

std::string handleHttpRequest(const std::string &httpRequest)
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

void routine(void *arg)
{

    int clientfd = *(int *)arg;

    while (1)
    {

        int nBufferLength = 8196;
        unsigned char buffer[nBufferLength] = {0};
        int ret = recv(clientfd, buffer, nBufferLength, 0);
        if (ret == 0)
        {
            close(clientfd);
            break;
        }
        printf("buffer : %s, ret: %d\n", buffer, ret);

        std::string httpRequest(reinterpret_cast<char *>(buffer), ret);
        std::string httpResponse = handleHttpRequest(httpRequest);
        //  ret = send(clientfd, httpResponse.c_str(), httpResponse.length(), 0);

        int totalSent = 0;
        int dataLength = httpRequest.length();
        while (totalSent < dataLength)
        {
            ret = send(clientfd, httpRequest.c_str() + totalSent, dataLength - totalSent, 0);
            if (ret == -1)
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
            totalSent += ret;
        }

        // close(clientfd);
        // break;
    }
}
