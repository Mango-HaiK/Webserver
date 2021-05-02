#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>

#define BUFFER_SZIE 4096    //读缓存区大小

//主状态机的两种状态
enum CHECK_STATE {
    CHECK_STATE_REQUESTLINE = 0,    //正在分析请求行
    CHECK_STATE_HEADER              //正在分析头部字段
};           

//从状态机的三种状态
enum LINE_STATUS {
    LINE_OK = 0,    //读取完整的行
    LINE_BAD,       //行出错
    LINE_OPEN       //行数据尚未完整
};

//服务器处理HTTP请求的结果
enum HTTP_COED {
    NO_REQUEST,         //请求不完整，继续读取client数据
    GET_REQUEST,        //获取了一个完整的请求
    BAD_REQUEST,        //client请求语法有误
    FORBINDDEN_REQUEST, //client对资源无足够访问权限
    INTERNAL_ERROR,     //服务器内部出错
    CLOSED_CONNECTION   //client已关闭连接
};

//处理信息发送的回执结果信息(代替HTTP应答报文)
static const char* szret[] = {
    "I get a correct result\n",
    "Something wrong\n"
};

//从状态机，用于解析出一行内容
LINE_STATUS parse_line(char* buffer,int& checked_index,int& read_index)
{
    printf("3\n");
    char temp;
    for ( ; checked_index < read_index; ++checked_index)
    {
        //获取当前要分析的字节
        temp = buffer[checked_index];
        //如果当前字节是'\r'，则可能读取到一个完整的行，后面还有'\n'
        if(temp == '\r')
        {
            //'\r'可能是当前buffer最后一个字节，尚未是完整的行
            if((checked_index + 1) == read_index)
                return LINE_OPEN;
            else if(buffer[checked_index + 1] == '\n')   //下一个字符是'\n'则找到
            {
                //?为何置'\0'
                buffer[checked_index++] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
        else if(temp == '\n')
        {
            if((checked_index > 1) && buffer[checked_index - 1] == '\r')   //'\n'也可能是完整的行
            {
                buffer[checked_index - 1] = '\0';
                buffer[checked_index++] = '\0';
                return LINE_OK;
            }
            return LINE_BAD;
        }
    }
    //内容尚未是完整的行
    return LINE_OPEN;
}

//分析请求行
HTTP_COED parse_requestline(char* temp, CHECK_STATE& checkstate)
{
    printf("1\n");
    char* url = strpbrk(temp," \t");
    //请求行存在空白符和\t
    if(!url)
        return BAD_REQUEST;
    *url++ = '\0';
    
    char *method = temp;
    if (strcasecmp(method,"GET") == 0)  //GET 方法
    {
        printf("The request method is GET\n");
    }
    else
    {
        return BAD_REQUEST;
    }

    url += strspn(url, " \t");
    
    char* version = strpbrk(url," \t");
    if (!version)   
    {
        return BAD_REQUEST;
    }

    *version++ = '\0';

    version += strspn(version," \t");
    
    //HTTP/1.1
    if (strcasecmp(url,"HTTP:/1.1") != 0)
    {
        return BAD_REQUEST;
    }
    //检查URL是否合法
    if (strncasecmp(url,"http://",7) == 0)
    {
        url += 7;
        url = strchr(url,'/');
    }

    if (! url || url[0] != '/')
    {
        return BAD_REQUEST;
    }
    
    printf("The request URL is :%s\n",url);
    //HTTP请求行处理完毕，状态转移到头部字段处理
    checkstate = CHECK_STATE_HEADER;
    return NO_REQUEST;
}

//分析头部字段
HTTP_COED parse_headers(char* temp)
{
    //遇到空行说明是一个正确的HTTP请求
    if(temp[0] == '\0')
        return GET_REQUEST;
    else if (strncasecmp(temp, "HOST:", 5) == 0)    //处理HOST头部字段
    {
        temp += 5;
        temp += strspn(temp, " \t");
        printf("the request host is %s.\n", temp);
    }
    else
        printf("I cann't handle this header.\n");
    
    return NO_REQUEST;
}
//分析请求入口函数
HTTP_COED parse_content(char* buffer,int& checked_index,
                        CHECK_STATE& checkstate,int& read_index,
                        int& start_line)
{
    printf("2\n");
    //当前行读取状态
    LINE_STATUS linestatus = LINE_OK;
    //记录HTTP请求的处理结果
    HTTP_COED retcode = NO_REQUEST;

    //主状态机，用于从buffer中取出所有的完整的行
    while ((linestatus = parse_line(buffer,checked_index,read_index)) == LINE_OK)
    {
        char* temp = buffer + start_line;
        start_line = checked_index;         //下一行的起始位置

        //checkstate 记录主状态机的当前状态
        switch (checkstate)
        {
            case CHECK_STATE_REQUESTLINE:       //分析请求行
                retcode = parse_requestline(temp, checkstate);
                if(retcode == BAD_REQUEST)
                    return BAD_REQUEST;
                break;
            case CHECK_STATE_HEADER :            //分析头部字段
                retcode = parse_headers(temp);
                if(retcode == BAD_REQUEST)
                    return BAD_REQUEST;
                else if(retcode == GET_REQUEST)
                    return GET_REQUEST;
                break;
            default:
                return INTERNAL_ERROR;
        }
    }
    //尚未读取到一个完整的行，继续读取
    if(linestatus == LINE_OPEN)
        return NO_REQUEST;
    else 
        return BAD_REQUEST;
}

int main(int argc, char *argv[])
{
    if(argc <= 2)
    {
        printf("usage: %s ip port",argv[0]);
        return 1;
    }
    char* ip = argv[1];
    int port = atoi(argv[2]);
    int ret,connfd;
    int sock = socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in address,client_address;
    socklen_t clilen;
    bzero(&address,sizeof(address));
    address.sin_family = AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port = htons(port);

    ret = bind(sock,(sockaddr*)&address,sizeof(address));
    assert(ret != -1);
    
    ret = listen(sock,5);
    assert(ret != -1);

    bzero(&client_address,sizeof(client_address));
    clilen = sizeof(client_address);

    connfd = accept(sock,(sockaddr*)&client_address,&clilen);
    
    if(connfd < 0)
    {
        printf("errno is: %d",errno);
    }else
    {
        char buffer[BUFFER_SZIE];
        memset(&buffer,'\0',BUFFER_SZIE);
        int data_read = 0;      
        int read_index = 0;     //已读取的客户数据
        int checked_index = 0;  //已分析完的数据
        int start_line = 0;     //行在buffer中的起始位置

        //设置主状态机的初始状态
        CHECK_STATE checkstate = CHECK_STATE_REQUESTLINE;
        //循环读取数据并分析
        while (1)
        {
            data_read = recv(connfd,buffer + read_index,
                            BUFFER_SZIE - read_index, 0);
            if(data_read == -1)
            {
                printf("reading failed\n");
                break;
            }
            else if (data_read == 0)
            {
                printf("remote client has closed the connection\n");
                break;
            }
            read_index += data_read;

            //分析目前已经或获得到的所有数据
            HTTP_COED result = parse_content(buffer,checked_index,checkstate,
                                            read_index,start_line);

            if(result == NO_REQUEST)    //尚未接受完一个完整的请求行
            {
                continue;
            }
            else if(result == GET_REQUEST)//正确的请求行
            {
                send(connfd,szret[0],strlen(szret[0]),0);
                break;
            }
            else        //其他错误
            {
                send(connfd,szret[1],strlen(szret[1]),0);
                break;
            }
        }   
        close(connfd);
    }
    close(sock);
    return 0;
}
