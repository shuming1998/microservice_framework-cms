SRC=$(wildcard *.cpp *.cc)					# 搜索本目录文件
TMP=$(patsubst %.cpp,%.o,$(SRC) )		# .cpp 替换为 .o
OBJS=$(patsubst %.cc,%.o,$(TMP) )
TARGET=libccome.so
CPPFLAGS=-fPIC
LIBS=-levent -lpthread -levent_openssl
INSTALL_PATH=/usr/lib

$(TARGET):$(OBJS)
	g++ -shared $+ -o $@ $(LIBS)
	cp *.so /usr/lib/

proto:
	protoc -I=./ --cpp_out=./ cmsgcom.proto cmsgtype.proto
install:$(TARGET) $(OBJS)
	cp *.so $(INSTALL_PATH)
uninstall:
	rm -rf *.o $(TARGET)
	rm -rf $(INSTALL_PATH)/$(TARGET)
clean:
	rm -rf *.o $(TARGET)
