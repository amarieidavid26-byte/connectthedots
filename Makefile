CXX = g++
## need at least cpp 14 for structured bindings

CXXFLAGS = -std=c++17 -Wall -Wextra -O2

SRCS = src/main.cpp src/grid.cpp src/display.cpp src/solver.cpp

OBJS = $(SRCS:.cpp=.o)
TARGET = connectthedots

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

src/%.o: src/%.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean: 
	rm -f $(OBJS) $(TARGET)

rebuild: clean all

.PHONY: all clean rebuild 