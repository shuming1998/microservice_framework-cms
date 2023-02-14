TARGET=test_bufferevent_ssl
SRC=$(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRC))
LIBS=-lssl -lcrypto -levent -levent_openssl
$(TARGET):$(OBJS)
					g++ $+ -o $@ ${LIBS}