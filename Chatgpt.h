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

    std::string SearchChatgpt(std::string &strReq);

protected:
    void InitRequestData();
    void AddJsonSystemRole(json& jsReq);
    void AppendRoleMessage(std::string &strRole, std::string &strMsg);
    std::string parseJSONAndGetContent(const std::string &jsonString);

private:
    CURL *m_pCurl;
    struct curl_slist *m_pHeaders = nullptr;
    
    json m_jsRequestData;
};
