SRCMODULES = tests.cpp main_tests.cpp
OBJMODULES = $(SRCMODULES:.cpp=.o)
CXXLIBS = -lCppUTest -lCppUTestExt
CXX = g++

%.o: %.cpp
	$(CXX) -c $< $(CXXLIBS) -o $@

tests: $(OBJMODULES)
	$(CXX) $^ $(CXXLIBS) -o $@

clean:
	rm -f main tests *.o
