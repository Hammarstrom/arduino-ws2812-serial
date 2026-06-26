CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -I test/mocks
TEST_SRC = test/test_main.cpp
TEST_BIN = test/run_tests

.PHONY: test clean

test: $(TEST_BIN)
	./$(TEST_BIN)

$(TEST_BIN): $(TEST_SRC) test/mocks/Arduino.h test/mocks/Adafruit_NeoPixel.h
	$(CXX) $(CXXFLAGS) -o $(TEST_BIN) $(TEST_SRC)

clean:
	rm -f $(TEST_BIN)
