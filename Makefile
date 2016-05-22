all: xmltest staticlib

rebuild: clean all

xmltest: xmltest.cpp tinyxml2.a

clean:
	$(RM) *.o xmltest tinyxml2.a
	
test: clean xmltest
	./xmltest

staticlib: tinyxml2.a

tinyxml2.a: tinyxml2.o
	$(AR) $(ARFLAGS)s $@ $^
	
tinyxml2.o: tinyxml2.cpp tinyxml2.h

