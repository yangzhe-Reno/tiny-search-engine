CXX = g++
CFLAGS = -std=c++11 -O2 -Wall -g -DGFLAGS_NS=google

OBJS = service/*.hpp 

TARGET = service/base_service.pb.cc -I/usr/local/include -pthread -lbrpc -lprotobuf  -lgflags -L/usr/local/lib

S_TARGET = server

server: $(OBJS)
	mkdir -p bin
	protoc -I=protobuf/ --cpp_out=service/ protobuf/base_service.proto
	$(CXX) $(CFLAGS) $(OBJS) -o bin/$(S_TARGET) server.cc $(TARGET)
	
clean:
	rm -rf bin/*
