CXX = g++
CFLAGS = -std=c++11 -O2 -Wall -g -DGFLAGS_NS=google

OBJS = service/*.hpp  

TARGET = service/*.pb.cc service/tinyxml2.cc service/WebPage.cc  -I/usr/local/include -pthread -lbrpc -lprotobuf  -lgflags -L/usr/local/lib

S_TARGET = search_server

server: $(OBJS)
	mkdir -p bin
	protoc -I=protobuf/ --cpp_out=service/ protobuf/service.proto
	$(CXX) $(CFLAGS) $(OBJS) -o bin/$(S_TARGET) server.cpp $(TARGET)
	
clean:
	rm -rf bin/*
