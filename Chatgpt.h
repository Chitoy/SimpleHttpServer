#include <iostream>
#include <string>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class CChatgpt
{
public:
    CChatgpt();
    ~CChatgpt();

        //通过该接口向chatgpt提问，传参为提问的问题，返回值为chatgpt返回的结果
    std::string SearchChatgpt(std::string &strReq);

    //设置chatgpt的system角色,如果不调用该接口，默认角色是"You are a helpful assistant."
    //调用可以是这样子SetJsonSystemRole("你是一个资深小说家。");
    void SetJsonSystemRole(std::string strNewRole);

protected:
    void InitRequestData();
    //void AddJsonSystemRole(json& jsReq);
    void AppendRoleMessage(std::string &strRole, std::string &strMsg);
    std::string parseJSONAndGetContent(const std::string &jsonString);

private:
    CURL *m_pCurl;
    struct curl_slist *m_pHeaders = nullptr;
    
    json m_jsRequestData;
};
