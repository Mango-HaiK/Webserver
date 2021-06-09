#ifndef HTTPRESPONSE_H_
#define HTTPRESPONSE_H_

#include <unordered_map>
#include <sys/stat.h>   //stat
#include <sys/mman.h>
#include <fcntl.h>
#include "../Log/log.h"
#include "../Buffer/buffer.h"

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string&, std::string&, bool iskeepAlive = false, int code = -1);
    /*装备响应头准备发射*/
    void MakeResponse(Buffer& buff);
    /*取消映射文件，清空映射存储区*/
    void UnmapFile();
    /*映射存储区地址*/
    char* File();
    /*映射存储区地址长度*/
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    /*状态码*/
    int Code() const;

private:
    /*添加状态行*/
    void __AddStateLine(Buffer&);
    void __AddHeader(Buffer&);
    void __AddContent(Buffer&);

    /*只对403、404作处理*/
    void __ErrorHtml();
    std::string __GetFileType();

    int m_code;
    bool m_isKeepAlive;

    /*文件路径*/
    std::string m_path;
    /*目录*/
    std::string m_srcDir;

    char* m_mmFile;             //存储映射区起始地址
    struct stat m_mmFileStat;   //文件状态

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};

#endif