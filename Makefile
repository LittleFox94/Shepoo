CC		=	/usr/bin/gcc
CFLAGS	=	-Wall -g
LDFLAGS	=	-lssl
OBJ		=	secnet.o protocol.o blockstorage.o shuffle.o main.o 

shepoo:	$(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LDFLAGS)

%.o: src/%.cpp
	$(CC) $(CFLAGS) -c $<
