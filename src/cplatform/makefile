TARGET=libccome.so
SRC=$(wildcard *.cpp *.cc)
TMP=$(patsubst %.cpp,%.o,$(SRC) )
OBJS=$(patsubst %.cc,%.o,$(TMP) )
CPPFLAGS= -fPIC -g
LIBS=-levent -lpthread -levent_openssl
INSTALL_PATH=/usr/lib/
INCLUDE_PATH=/usr/include/cms
exist = $(shell if [ ! -d $(INCLUDE_PATH) ]; then mkdir $(INCLUDE_PATH); fi;)
$(TARGET):$(OBJS)
	g++ -shared $+ -o $@ $(LIBS)
	cp *.so /usr/lib/
proto:
	protoc -I=./ --cpp_out=./ cmsgcom.proto cmsgtype.proto
install:$(TARGET) $(OBJS)
	@echo "begin install in "$(INSTALL_PATH)
	@cp *.so $(INSTALL_PATH)
	@echo $(exist) "check include ..."
	cp *.h $(INCLUDE_PATH)
	@echo "install success!"
uninstall:
	rm -rf *.o $(TARGET)
	rm -rf $(INSTALL_PATH)/$(TARGET)
clean:
	rm -rf *.o $(TARGET)


