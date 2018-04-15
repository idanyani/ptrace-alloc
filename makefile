TARGET := tracer

$(TARGET): main.cxx
	g++ -std=c++11 -Wall -Werror $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET)

