c++ src/flow.cpp -I /usr/local/include/jsoncpp -c -std=c++0x
mv flow.o bin/
c++ -o bin/go bin/flow.o -L/usr/local/lib/libjsoncpp.a /usr/local/lib/libjsoncpp.so
