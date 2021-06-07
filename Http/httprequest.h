#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <unordered_map>
#include <unordered_set>
#include <string>
#include <regex>

#include "../Log/log.h"

class HttpRequest
{
public:
    /*请求行匹配状态*/
    enum PARSE_STATE {
        REQUEST_LINE,   
        HEADERS,        
        BODY,           
        FINISH          
    };

    HttpRequest();
    ~HttpRequest() = default;
    
    //匹配相应头 - 主状态机
    bool Parse(Buffer&);

    void Init();

private:
    /*匹配请求行*/
    bool __ParseRequestLine(const std::string& line);
    /*匹配请求头*/
    void __ParseRequestHead(const std::string& line);
    /*匹配请求实体*/
    void __ParseRequestBody(const std::string& line);

    /*匹配Post*/
    void __ParsePost();
    /*匹配路径*/
    void __ParsePath();
    /*匹配URL编码方式*/
    void __ParseFromUrlencoded();

    PARSE_STATE m_state;
    std::string m_method, m_path, m_version, m_body;

    /*请求头以key-value方式存放信息*/
    std::unordered_map<std::string, std::string> m_head;
    std::unordered_map<std::string, std::string> m_post;

    static const std::unordered_set<std::string> DEFAULT_HTML;
    static const std::unordered_map<std::string, int> DEFAULT_HTML_TAG;
    
};

#endif