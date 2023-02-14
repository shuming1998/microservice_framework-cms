TARGET=libccome.so
OBJS=cservertask.o cthread.o cthreadpool.o
CPPFLAGS=-fPIC
LIBS=-levent -lpthread
$(TARGET):$(OBJS)
	g++ -shared $+ -o $@ $(LIBS)
clean:
	rm -rf $(OBJS) $(TARGET)