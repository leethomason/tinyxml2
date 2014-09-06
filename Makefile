all: xmltest
xmltest: tinyxml2.o
test: clean xmltest
	./xmltest
tinyxml2:
clean:
	rm -f *.o xmltest
