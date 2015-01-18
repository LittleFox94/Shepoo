CXX?	=	/usr/bin/g++
CFLAGS	=	-Wall -g
LDFLAGS	=	-lssl -lcrypto
OBJ		=	secnet.o protocol.o blockstorage.o shuffle.o main.o 

shepoo:	$(OBJ)
	$(CXX) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: src/%.cpp
	$(CXX) $(CFLAGS) -c $<

.PHONY: clean

clean:
	rm $(OBJ) shepoo > /dev/null 2>&1 || true
