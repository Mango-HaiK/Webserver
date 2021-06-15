CC = g++
CFLAGS = -std=c++11 -Wall

testLog:testLog.cpp ../Log/log.cpp ../Buffer/buffer.cpp
	$(CC) $(CFLAGS) -o testLog ../Log/log.cpp ../Buffer/buffer.cpp testLog.cpp -lpthread

clean:
	rm *.o