TARGET=cdiskserver
OBJS=cdiskserver.o cfileservertask.o
CPPFLAGS=-I../ccome
LIBS=-L../ccome -lccome
$(TARGET):$(OBJS)
	g++ $+ -o $@ $(LIBS)
clean:
	rm -rf $(OBJS) $(TARGET)