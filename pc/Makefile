CC=gcc
CFLAGS=
LDFLAGS=
LDLIBS=-pthread -lm
CXX=g++
CXXFLAGS=$(CFLAGS) -std=c++11




COMMON_DIR=../common


server: server.o serial.o $(COMMON_DIR)/messages_x64.o $(COMMON_DIR)/trilateration_x64.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $^ -o $@ $(LDLIBS)

server.o: server.cpp

serial.o: serial.cpp

$(COMMON_DIR)/messages_x64.o: $(COMMON_DIR)/messages.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $^ -o $@ $(LDLIBS)

$(COMMON_DIR)/trilateration_x64.o: $(COMMON_DIR)/trilateration.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $^ -o $@ $(LDLIBS)

run:
	./server


clean:
	rm *.o server
