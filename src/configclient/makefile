TARGET=libconfig_client.so
SRC=$(wildcard c*.cpp)
OBJS=$(patsubst %.cpp,%.o,$(SRC) )
INCLUDE_PATH=/usr/include/cms
CPPFLAGS= -fPIC -g -I$(INCLUDE_PATH)
LIBS=-levent -lpthread -lccome -lprotobuf -lssl -lregister_client
INSTALL_PATH=/usr/lib/
exist = $(shell if [ ! -d $(INCLUDE_PATH) ]; then mkdir $(INCLUDE_PATH); fi;)
$(TARGET):$(OBJS)
	g++ -shared $+ -o $@ $(LIBS)
install:
	cp $(TARGET) $(INSTALL_PATH)
	@echo $(exist) "check include ..."
	cp *.h $(INCLUDE_PATH)
clean:
	rm -rf *.o  $(TARGET)


