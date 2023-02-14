TARGET=register_client
SRC=$(wildcard *.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) )
CPPFLAGS= -fPIC -I../cplatform -I../registerclient
LIBS=-levent -lpthread -lccome -lprotobuf -lssl
INSTALL_PATH=/usr/lib/

$(TARGET):$(OBJS)
	g++  $+ -o $@ $(LIBS)
clean:
	rm -rf *.o  $(TARGET)


