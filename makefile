TARGET := tracer

$(TARGET): main.cxx
	g++ -std=c++11 $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET)

