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

bool HttpRequest::IsKeepAlive() const
{
    if (m_head.count("Connection") == 1)
    {
        return m_head.find("Connection")->second == "keep-alive" && m_version == "1.1";
    }
    return false;
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
        switch (m_state)
        {
        case REQUEST_LINE:      /*请求行*/
            if(!__ParseRequestLine(line))
                return false;
            __ParsePath();
            break;
        case HEADERS:
            __ParseRequestHead(line);
            if(buff.ReadableBytes() <= 2) m_state = FINISH; //存在空行
            break;
        case BODY:
            __ParseRequestBody(line);
        default:
            break;
        }
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
    m_body = line;
    __ParsePost();  //检验是否为Post请求
    m_state = FINISH;
    LOG_DEBUG("Body:%s, len:%d",line.c_str(),line.size());
}

void HttpRequest::__ParsePost()
{
    if (m_method == "POST" && m_head["Content-Type"] == "application/x-www-form-urlencoded")
    {
        __ParseFromUrlencoded();
        /*验证请求页面是否存在*/
        if (DEFAULT_HTML_TAG.count(m_path))
        {
            int tag = DEFAULT_HTML_TAG.find(m_path)->second;
            if (tag == 0 || tag == 1)
            {
                bool isLogin = (tag == 1);
                if (UserVerify(m_post["username"],m_post["password"],isLogin))
                {
                    m_path = "/weclome.html";
                }
                else{
                    m_path = "/error.html";
                }
            }  
        } 
    }   
}

int HttpRequest::ConverHex(char ch)
{
    if(ch >= 'A' && ch <= 'F')
        return ch - 'A' + 10;
    if(ch >= 'a' && ch <= 'f')
        return ch - 'a' + 10;
    return ch;
}

void HttpRequest::__ParseFromUrlencoded()
{
    if(m_body.size() == 0)
        return;
    
    string key, value;
    int num = 0;
    int n = m_body.size();
    int i = 0,j = 0;

    for ( ; i < n; i++)
    {
        char ch = m_body[i];
        switch (ch)
        {
        case '=':   /*等号前面是key*/
            key = m_body.substr(j, i - j);
            j = i + 1;
            break;
        case '+':   /*URL中+表示空格*/
            m_body[i] = ' ';
            break;
        case '%':   /*特殊字符转换*/
            num = ConverHex(m_body[i + 1]) * 16 + ConverHex(m_body[i + 2]);
            m_body[i + 2] = num % 10 + '0';
            m_body[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&':   /*&分割一对参数，&前面为value*/
            value = m_body.substr(j, i - j);
            j = i + 1;
            m_post[key] = value;
            LOG_DEBUG("%s = %s",key.c_str(), value.c_str());
        default:
            break;
        }
    }
    //处理最后一对参数的value
    assert(j <= i);
    if (m_post.count(key) == 0 && j < i)
    {
        value = m_body.substr(j, i - j);
        m_post[key] = value;
    }
}

void HttpRequest::__ParsePath()
{
    if(m_path == "/")
    {
        m_path = "/index.html";
    }else
    {
        for (auto &item : DEFAULT_HTML)
        {
            if(item == m_path)
            {
                m_path = item;
                break;
            }
        }
    }
}

string HttpRequest::path() const
{
    return m_path;
}

string& HttpRequest::path()
{
    return m_path;
}

string HttpRequest::method() const
{
    return m_method;
}

string& HttpRequest::method()
{
    return m_method;
}

string HttpRequest::version() const
{
    return m_version;
}

string HttpRequest::GetPost(const string& key) const
{
    assert(key != "");
    if (m_post.count(key) == 1)
    {
        return m_post.find(key)->second;
    }
    return "";
}

string HttpRequest::GetPost(const char* key) const
{
    assert(key != nullptr);
    if (m_post.count(key) == 1)
    {
        return m_post.find(key)->second;
    }
    return "";
}

bool HttpRequest::UserVerify(const string& user,const string& pwd, bool isLogin)
{
    
}