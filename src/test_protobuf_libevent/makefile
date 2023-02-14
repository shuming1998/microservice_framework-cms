TARGET=test_protobuf_libevent
OBJS=$(TARGET).o cmsgserver.o cserverevent.o cmsgclient.o cclientevent.o cmsgevent.o cmsgtype.pb.o cmsgcom.pb.o 
$(TARGET):$(OBJS)
	g++ -o $@ $(OBJS) -lprotobuf -levent -lpthread
clean:
	rm -rf $(TARGET) $(OBJS)
	rm *.pb.*