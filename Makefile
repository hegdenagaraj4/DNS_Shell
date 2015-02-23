CXX := clang
CXXFLAGS := -g -w -fPIC -Wall
INCLUDES := -I

OBJECTS	:= myshell.o myfunctions.o

myshell	: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o dnsshell

myfunctions	: myfunctions.o
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o myfunctions

myshell.o	: myshell.c
	$(CXX) $(INCLUDES) $(CXXFLAGS) -c myshell.c -o myshell.o

myfunctions.o	: myfunctions.c
	$(CXX) $(CXXFLAGS) -c myfunctions.c -o myfunctions.o

clean	:
	rm -rf dnsshell *.o
