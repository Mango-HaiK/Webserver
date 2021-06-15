CC = g++
CFLAGS = -std=c++11 -Wall

testmain:testmain.cpp ../Log/log.cpp ../Buffer/buffer.cpp
	$(CC) $(CFLAGS) -o testmain testmain.cpp ../Log/log.cpp ../Buffer/buffer.cpp -lmysqlclient -lpthread

clean:
	rm *.o