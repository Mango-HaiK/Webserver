#include "httprequest.h"
using namespace std;

const std::unordered_set<std::string> HttpRequest::DEFAULT_HTML {
    "/index","/register","/login",
    "/welcome","/video","picture"
};

const unordered_map<string,int> HttpRequest::DEFAULT_HTML_TAG {
    {"/register.html", 0},
    {"/login.html", 1}
};

HttpRequest::HttpRequest()
{
    Init();
}

void HttpRequest::Init()
{
    m_path = m_version = m_body = m_method = "";
    m_post.clear();
    m_head.clear();
    m_state = REQUEST_LINE;
}

bool HttpRequest::Parse(Buffer& buff)
{
    const char CRLF[] = "\r\n";
    if (buff.ReadableBytes() <= 0)
    {
        return false;
    }
    while (buff.ReadableBytes() && m_state != FINISH)
    {
        const char* lineEnd = search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2);
        string line(buff.Peek(), lineEnd);
    }
    return true;
}

bool HttpRequest::__ParseRequestLine(const string& line)
{
    //先匹配不带空格的串
    regex base_regex("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch sub_regex;
    if (regex_match(line,sub_regex,base_regex))
    {
        m_method = sub_regex[1];        //sub_regex[0]是整个匹配到的字符串
        m_path = sub_regex[2];
        m_version = sub_regex[3];
        m_state = HEADERS;
        return true;
    }
    return false;
}

void HttpRequest::__ParseRequestHead(const string& line)
{
    regex base_regex("^([^:]*): ?(.*)$");   //有些冒号后面无":"
    smatch sub_regex;
    if (regex_match(line, sub_regex, base_regex))
    {
        m_head[sub_regex[1]] = sub_regex[2];
    }else   
    {
        m_state = BODY;
    }
}

void HttpRequest::__ParseRequestBody(const string& line)
{

}