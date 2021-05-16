#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#define BUFFER_SIZE 64

class util_timer;

//用户数据结构，客户端socket地址，socket描述符、读缓存和定时器
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer *timer;
};

//定时器类
class util_timer
{

public:
    util_timer() : prev(NULL),next(NULL){}
    ~util_timer();

    time_t expire;        //任务超时时间-绝对时间
    
    void (*cb_func)(client_data*);  //任务回调函数
    
    //回调函数处理的用户数据，由定时器的执行者传递给回调函数
    client_data* user_data;

    util_timer *prev;//前一个定时器
    util_timer *next;//后一个定时器
};

//定时器链表，升序、双向、具有头尾节点
class sort_timer_lst
{
public:
    sort_timer_lst() : head(NULL),tail(NULL) {}
    
    //销毁所有定时器
    ~sort_timer_lst() 
    {
        util_timer *tmp = head;
        while (tmp)
        {
            head = tmp->next;
            delete tmp;
            tmp = head;
        }
    }

    //将目标定时器timer添加到链表中
    void add_timer(util_timer *timer)
    {
        if(!timer) return;
        if(!head)
        {
            head = tail = timer;
            return;
        }
        //如果超时时间小于当前链表中所有定时器的超时时间，则插入头部
        //作为新的头节点，否则调用另一个add_timer，保证升序性
        if (timer->expire < head->expire)
        {
            timer->next = head;
            head->prev = timer;
            head = timer;
            return;
        }
        add_timer(timer,head);
    }

    //当某个定时任务发生变化时调整对应的定时器在链表中的位置，
    //这个函数只考虑被调整的定时器的超时时间延长的情况，该定时器往尾部移动
    void adjust_timer(util_timer* timer)
    {
        if (!timer) return;
        util_timer *tmp = timer->next;

        //如果被调整的目标定时器在尾部，或其超时值仍然小于其下一个定时器的超时值则不调整
        if (!tmp || (timer->expire < tmp->expire))
            return;
        
        //如果为头节点，则取出重新插入链表
        if (timer == head)
        {
            head = head->next;
            head->prev = NULL;
            timer->next = NULL;
            add_timer(timer,head);
        }
        //不是头节点则取出插入该节点位置之后的部分链表中
        else{
            timer->prev->next = timer->next;
            timer->next->prev = timer->prev;
            add_timer(timer,timer->next);
        }
    }

    //删除目标定时器timer
    void del_timer(util_timer* timer)
    {
        if(!timer)
            return;
        //只有一个定时器
        if ((timer == head) && (timer == tail))
        {
            delete timer;
            head = NULL;
            tail = NULL;
            return ;
        }
        //超过两个定时器且是第一个
        if (timer == head)
        {
            head = head->next;
            head->prev = NULL;
            delete timer;
            return;
        }
        //是最后一个
        if (timer == tail)
        {
            tail = tail->prev;
            tail->next = NULL;
            delete timer;
            return;
        }
        //在中间
        timer->next->prev = timer->prev;
        timer->prev->next = timer->next;
        delete timer;
    }

    //SIGALRM信号每次被触发就在其信号处理函数中执行一次tick，以处理链表上到期的任务
    void tick()
    {
        if (!head)
        {
            return;
        }
        printf("timer tick\n");
        time_t cur = time(NULL);
        util_timer* tmp = head;
        //从头结点开始依次处理每个定时器，直到遇到一个尚未到期的定时器
        while (tmp)
        {
            //每个定时器都使用绝对时间作超时值，
            if(cur < tmp->expire)
            {
                break;
            }
            //调用定时器的回调函数,执行定时任务
            tmp->cb_func(tmp->user_data);
            //执行之后删除定时器，并重置链表
            head = tmp->next;
            if (head)
            {
                head->prev = NULL;
            }
            delete tmp;
            tmp = head;
        }
        
    }
private:
    //将目标定时器添加到lst_head之后的部分链表中
    void add_timer(util_timer* timer,util_timer* lst_head)
    {
        util_timer* prve = lst_head;
        util_timer* tmp = prve->next;
        //遍历lst_head节点之后的部分链表，直到找到一个超时时间大于
        //目标定时器的超时时间结点，并将目标定时器该结点之前
        while (tmp)
        {
            if(timer->expire < tmp->expire)
            {
                prve->next = timer;
                timer->next = tmp;
                tmp->prev = timer;
                timer->prev = prve;
                break;
            }
            prve = tmp;
            tmp = tmp->next;
        }
        //如未找到合适结点，则插入末尾作为新的尾结点
        if (!tmp)
        {
            prve->next = timer;
            timer->prev = prve;
            timer->next = NULL;
            tail = timer;
        }
    }
    util_timer *head;
    util_timer *tail;
};

#endif