#include "EpollServer.h"

class CHttpServer : public CEpollServer
{
public:
    CHttpServer();
    ~CHttpServer();

protected:
    std::string handleRequest(const std::string &httpRequest) override;
    // 判断发送完数据后是否断开连接，主要用于区分长连接和短连接，默认是发送完数据断开
    bool bCloseAfterSend() override { return true; }

private:
    std::string analysisHttp(const std::string &httpRequest);
    void SerilizHttpReponseEnd(std::ostringstream &response, std::ifstream &file);
    std::string extractSearchString(const std::string &input);
    std::string urlDecode(const std::string &input);

private:
    std::string HTTP_404_ERROR = "HTTP/1.1 404 Not Found\r\n\r\nFile not found";
};
