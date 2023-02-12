TARGET=test_openssl_tls
SRC=$(wildcard *.cpp)
OBJS = $(patsubst %.cpp,%.o,$(SRC))
LIBS=-lssl -lcrypto
$(TARGET):$(OBJS)
					g++ $+ -o $@ ${LIBS}