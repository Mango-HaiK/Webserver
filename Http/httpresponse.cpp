#include "httpresponse.h"

using namespace std;

const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = 
{
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "}
};

const unordered_map<int ,string> HttpResponse::CODE_STATUS = 
{
    { 200 , "OK"},
    { 400 , "Bad Request"},
    { 403 , "Forbidden"},
    { 404 , "Not Found"}
};

const unordered_map<int, string> HttpResponse::CODE_PATH = 
{
    { 400 , "/400.html"},
    { 403 , "/403.html"},
    { 404 , "/404.html"}
};

HttpResponse::HttpResponse() : m_code(-1), m_path(""),
                            m_srcDir(""), m_isKeepAlive(false),
                            m_mmFile(nullptr), m_mmFileStat({0})
{

}

HttpResponse::~HttpResponse()
{
    UnmapFile();
}
void HttpResponse::Init(const string& srcDir,string& path, bool isKeepAlive, int code)
{
    assert(srcDir != "");
    if (m_mmFile)
    {
        UnmapFile();
    }
    m_code = code;
    m_isKeepAlive = isKeepAlive;
    m_path = path;
    m_srcDir = srcDir;
    m_mmFile = nullptr;
    m_mmFileStat = {0};
}

char* HttpResponse::File()
{
    return m_mmFile;
}

size_t HttpResponse::FileLen() const
{
    return m_mmFileStat.st_size;
}

int HttpResponse::Code() const
{
    return m_code;
}

void HttpResponse::UnmapFile()
{
    if (m_mmFile)
    {
        //解除映射区
        munmap(m_mmFile,m_mmFileStat.st_size);
        m_mmFile = nullptr;
    }
}

void HttpResponse::MakeResponse(Buffer& buff)
{
    /*文件不存在或为文件夹， 状态码为404->访问的资源不存在*/
    if (stat((m_srcDir + m_path).data(), &m_mmFileStat) < 0
            || S_ISDIR(m_mmFileStat.st_mode))
    {
        m_code = 404;
    }else if (!(m_mmFileStat.st_mode & S_IROTH))
    {
        /*对该资源没有访问权限，403->拒绝请求*/
        m_code = 403;
    }
    else if(m_code == -1)
    {
        m_code = 200;
    }
    __ErrorHtml();
    __AddStateLine(buff);
    __AddHeader(buff);
    __AddContent(buff);
}

void HttpResponse::__ErrorHtml()
{
    if (CODE_PATH.count(m_code) == 1)
    {
        m_path = CODE_PATH.find(m_code)->second;
        stat((m_srcDir + m_path).data(), &m_mmFileStat);
    }
}

void HttpResponse::__AddStateLine(Buffer& buff)
{
    string status;
    if (CODE_STATUS.count(m_code) == 1)
    {
        status = CODE_STATUS.find(m_code)->second;
    }
    else
    {
        m_code = 400;   //通用错误状态码
        status = CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + to_string(m_code) + " " + status + "\r\n");
}

void HttpResponse::__AddHeader(Buffer &buff)
{
    buff.Append("Connection: ");
    /*HTTP/1.1默认支持长连接*/
    if (m_isKeepAlive)
    {
        buff.Append("keep-alive\r\n");
        buff.Append("keep=alive: max=6, timeout=120\r\n");
    }else
    {
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + __GetFileType() + "\r\n");
}

void HttpResponse::__AddContent(Buffer &buff)
{
    //需要映射到内存中的文件
    int srcFd = open((m_srcDir + m_path).data(), O_RDONLY);
    if (srcFd < 0)
    {
        ErrorContent(buff, "File NotFound!");
        return;
    }
    
    /*使用存储映射I/O以提高文件访问速度，
      MAP_PRIVATE标志将创建映射文件的私有副本，
      改变其地址上的内容不会对文件产生影响*/
    int* mmtemp = (int*) mmap(0, m_mmFileStat.st_size,PROT_READ,MAP_PRIVATE,srcFd,0);
    if (*mmtemp == -1)
    {
        //欺骗客户
        ErrorContent(buff, "File NotFound!");
        return;
    }
    m_mmFile = (char*)mmtemp;
    //关闭映射存储区使用的文件描述符并不解除映射区
    close(srcFd);
    buff.Append("Content-length: " + to_string(m_mmFileStat.st_size) + "\r\n\r\n");
}

string HttpResponse::__GetFileType()
{
    //前面可能也会出现 .，如果未找到则返回string::npos = -1
    string::size_type index = m_path.find_last_of('.');
    if(index == string::npos)
    {   
        //将文件类型设置为纯文本，但浏览器不会对这玩意处理
        return "text/plain";
    }
    string suffix = m_path.substr(index);
    if (SUFFIX_TYPE.count(suffix) == 1)
    {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    /*还有其他未记录的类型*/
    return "text/plain";
}

void HttpResponse::ErrorContent(Buffer& buff, string message)
{
    string body;
    string status;
    body += "<html><title>Error<title><body>";
    if (CODE_STATUS.count(m_code) == 1)
    {
        status = CODE_STATUS.find(m_code)->second;
    }else
    {
        status = "Bad Request";
    }
    body += to_string(m_code) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>WebServer</em></body></html>";
    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}