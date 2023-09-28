#include "toolFun.h"

constexpr auto HTTP_404_ERROR = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";

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
        std::ifstream file("src/Homepage.html");
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
             (strSrc.find(".jpg") != std::string::npos))
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
        else
    {
        // 处理其他路径的HTTP请求，可以根据需要添加更多逻辑
        return HTTP_404_ERROR;
    }
}
