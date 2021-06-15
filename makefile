CC = g++
CFLAGS = -std=c++11 -w
LIB = -lpthread -lmysqlclient
OBJS = main.cpp ./Server/server.cpp ./epoll/epoller.cpp ./Timer/timer.cpp \
		./Http/httprequest.cpp ./Http/httpresponse.cpp ./Http/httpconnection.cpp \
		./Pool/sqlConnPool.cpp ./Log/log.cpp ./Buffer/buffer.cpp 

server:$(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o server $(LIB)

clean: 
	rm -r server