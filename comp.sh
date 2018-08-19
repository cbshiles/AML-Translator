c++ src/flow.cpp -I jsoncpp/include/ -c -std=c++0x
mv flow.o bin/
c++ -o bin/go bin/flow.o bin/jsoncpp.o
