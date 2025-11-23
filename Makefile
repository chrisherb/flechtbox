CXX = g++
CXXFLAGS = -std=c++17 -Wall
LDLIBS = -lportaudio -lftxui-component -lftxui-dom -lftxui-screen
TARGET = flechtbox
SRC = src/main.cpp src/audio.cpp src/ui.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRC) $(LDLIBS)

clean:
	rm -f $(TARGET)
