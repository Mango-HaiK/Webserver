#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include <unordered_map>
#include <sys/stat.h>   //stat
#include <sys/mman.h>
#include "../Log/log.h"
#include "../Buffer/buffer.h"

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string&, std::string&, bool iskeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const;

private:
    void __AddStateLine(Buffer&);
    void __AddHeader(Buffer&);
    void __AddContent(Buffer&);

    void __ErrorHtml();
    std::string __GetFileType();

    int m_code;
    bool m_isKeepAlive;

    std::string m_path;
    /*目录*/
    std::string m_srcDir;

    char* m_mmFile;
    struct stat m_mmFileStat;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif