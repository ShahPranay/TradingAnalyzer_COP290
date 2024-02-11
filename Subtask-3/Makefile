CXX := g++
CXXFLAGS := -std=c++11

# Specify your source file and executable name
SRC := Basic_strat.cpp
EXEC := basic

# Default target when running just 'make' without any arguments
default: run

# Compile the source file and run the executable with the specified command line arguments
run: $(SRC)
	$(CXX) $(CXXFLAGS) -o $(EXEC) $(SRC)
	./$(EXEC) $(strategy) $(symbol) $(n) $(x) $(start_date) $(end_date) $(p)

# Clean up generated files
clean:
	rm -f $(EXEC)

.PHONY: clean run
