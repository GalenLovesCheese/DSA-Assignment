CXX = g++
CXXFLAGS = -I./lib --std=c++17 -MMD -O3
SRC = src/main.cpp
LIBS = $(wildcard lib/**/*.cpp)
OBJECTS = $(SRC:.cpp=.o) $(LIBS:.cpp=.o)
DEPFILES = $(OBJECTS:.o=.d)
TARGET = movieApp

.PHONY: debug_vsc debug run clean large-dataset

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPFILES)

debug_vsc: CXXFLAGS += -g -DDEBUG
debug_vsc: $(TARGET)

debug: CXXFLAGS += -g -DDEBUG
debug: $(TARGET)
	./$(TARGET)

large-dataset: CXXFLAGS += -g -DDEBUG -DLARGE
large-dataset: $(TARGET)
	./$(TARGET)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET) $(OBJECTS) $(DEPFILES)
	rm -rf *.dSYM
