CXX = g++
CXXFLAGS = -I./lib --std=c++11
SRC = src/main.cpp
LIBS = $(wildcard lib/**/*.cpp)
TARGET = movieApp

$(TARGET): $(SRC) $(LIBS)
	$(CXX) $(CXXFLAGS) $^ -o $@

debug_vsc:
	$(CXX) $(CXXFLAGS) -g $(SRC) $(LIBS) -o $(TARGET) -DDEBUG

debug:
	$(CXX) $(CXXFLAGS) -g $(SRC) $(LIBS) -o $(TARGET) -DDEBUG
	./$(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
	rm -rf *.dSYM
