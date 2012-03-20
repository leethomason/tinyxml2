all: xmltest
xmltest: xmltest.cpp tinyxml2.cpp tinyxml2.h
clean:
	rm -f *.o xmltest
