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