CXX = g++ 
CXXFLAGS = -std=c++17 -Wall -Wextra -pedantic

TARGET = backtest
SRCS = main.cpp backtest.cpp
OBJS = $(SRCS:.cpp=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)