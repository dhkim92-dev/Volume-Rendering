CC=g++
CFLAGS = -std=c++14 -w -o
TARGET = main
SRC = main.cpp
LDFLAGS = -lglfw -framework OpenGL -Iinclude

$(TARGET) : $(SRC)
	$(CC) $(CFLAGS) $(TARGET) $(SRC) $(LDFLAGS)

run : $(TARGET)
	make clean && make ./main

clean :
	rm main