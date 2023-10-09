#include "Chatgpt.h"

// 回调函数，用于处理HTTP响应
size_t WriteCallback(void *contents, size_t size, size_t nmemb, std::string *output)
{
    size_t totalSize = size * nmemb;
    output->append(static_cast<char *>(contents), totalSize);
    return totalSize;
}

CChatgpt::CChatgpt()
{
    InitRequestData();
    //  初始化Curl
    m_pCurl = curl_easy_init();
    if (m_pCurl)
    {
        std::string url = "https://api.openai.com/v1/chat/completions";
        std::string apiKey = ""; // 替换为您的ChatGPT API密钥

        // 设置HTTP请求头
        m_pHeaders = curl_slist_append(m_pHeaders, "Content-Type: application/json");
        m_pHeaders = curl_slist_append(m_pHeaders, ("Authorization: Bearer " + apiKey).c_str());
        curl_easy_setopt(m_pCurl, CURLOPT_HTTPHEADER, m_pHeaders);

        // 设置HTTP请求的URL
        curl_easy_setopt(m_pCurl, CURLOPT_URL, url.c_str());

        // 设置响应处理回调函数
        curl_easy_setopt(m_pCurl, CURLOPT_WRITEFUNCTION, WriteCallback);
    }
    else
    {
        std::cout << "Curl初始化失败" << std::endl;
    }
}

CChatgpt::~CChatgpt()
{
    if (m_pCurl)
    {
        // 清理Curl资源
        curl_easy_cleanup(m_pCurl);
        // 释放HTTP请求头资源
        curl_slist_free_all(m_pHeaders);
    }
}

void CChatgpt::InitRequestData()
{
    m_jsRequestData.clear();
    m_jsRequestData["model"] = "gpt-3.5-turbo";

    // 添加"messages"键的值，这是一个数组
    m_jsRequestData["messages"] = json::array();

    json message1;
    message1["role"] = "system";
    message1["content"] = "You are a helpful assistant.";
    m_jsRequestData["messages"].emplace_back(message1);
}

void CChatgpt::SetJsonSystemRole(std::string strNewRole)
{
    json &messages = m_jsRequestData["messages"];

    for (auto &message : messages)
    {
        if (message.find("role") != message.end() && message["role"] == "system")
        {
            if (message.find("content") != message.end())
            {
                message["content"] = strNewRole;
            }
        }
    }
}

void CChatgpt::AppendRoleMessage(std::string &strRole, std::string &strMsg)
{
    // 添加另一个消息对象到数组中
    json message;
    message["role"] = strRole;
    message["content"] = strMsg;
    m_jsRequestData["messages"].emplace_back(message);
}

std::string CChatgpt::SearchChatgpt(std::string &strReq)
{
    CURLcode res;
    std::string strGPTResult;
    if (m_pCurl)
    {
        // 设置HTTP请求体
        std::string strUser = "user";
        std::cout << "用户：" << strReq << std::endl;
        AppendRoleMessage(strUser, strReq);
        std::string strJson = m_jsRequestData.dump();
        curl_easy_setopt(m_pCurl, CURLOPT_POSTFIELDS, strJson.c_str());

        //设置接收数据
        std::string strResponse;
        curl_easy_setopt(m_pCurl, CURLOPT_WRITEDATA, &strResponse);

        // 执行HTTP POST请求
        res = curl_easy_perform(m_pCurl);

        // 检查请求是否成功
        std::string strAssistant = "assistant";
        if (res == CURLE_OK)
        {
            strGPTResult = parseJSONAndGetContent(strResponse);
            std::cout << "AI：" << strGPTResult << std::endl;
            AppendRoleMessage(strAssistant, strGPTResult);
        }
        else
        {
            std::cerr << "HTTP请求失败：" << curl_easy_strerror(res) << std::endl;
            strGPTResult = "";
            AppendRoleMessage(strAssistant, strGPTResult);
        }
    }
    else
    {
        std::cerr << "Curl初始化失败" << std::endl;
        return "Curl初始化失败";
    }
    return strGPTResult;
}

std::string CChatgpt::parseJSONAndGetContent(const std::string &jsonString)
{
    try
    {
        // 解析JSON字符串
        json jsonData = json::parse(jsonString);

        // 提取"message"字段中的"content"值
        std::string content = jsonData["choices"][0]["message"]["content"];

        return content;
    }
    catch (const std::exception &e)
    {
        // 处理解析或提取错误
        std::cerr << "Error: " << e.what() << std::endl;
        return ""; // 或者您可以返回一个错误值
    }
}
