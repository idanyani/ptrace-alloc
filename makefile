TARGET := tracer
CHILD_MMAP := child_mmap
all : $(TARGET) $(CHILD_MMAP)

$(TARGET): main.cxx
	g++ -std=c++11 -Wall -Werror $< -o $@

$(CHILD_MMAP): child_mmap_program.cpp
	g++ -std=c++11 -Wall -Werror $< -o $@


.PHONY: clean all
clean:
	rm -rf $(TARGET)
	rm -rf $(CHILD_MMAP)
