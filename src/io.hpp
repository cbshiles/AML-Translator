#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <vector>

#define po(y) (cout << y)
#define pl(t) (cout << t << endl)
#define pel(t) (cerr << t << endl)
#define pls(t) (cout << #t << " " << t << endl)

using namespace std;

bool readable(string fname){
  ifstream rf;
  rf.open(fname.c_str());
  bool ope = rf.is_open();
  rf.close();
  return ope;

}

vector<string> readLzt(string fname){
  ifstream rf;
  rf.open(fname.c_str());

  if (! rf.is_open())
    throw runtime_error("Could not open read file.");

  vector<string> lzt;
  string line;
  while(! (getline(rf, line)).eof()){
    lzt.push_back(line);
  } lzt.push_back(line);
  rf.close();
  return lzt;
}

string readF(string fname){
  ifstream rf;
  rf.open(fname.c_str());

  if (! rf.is_open())
    throw runtime_error("Could not open read file.");

  string line, tot="";
  while(! (getline(rf, line)).eof()){
    tot += line+'\n';
  } tot += line+'\n';
  rf.close();
  return tot;
}

void writeF(string fname, string data){
  ofstream wf;
  wf.open(fname.c_str());
  if (! wf.is_open())
    throw runtime_error("Could not open write file.");

  wf.write(data.c_str(), data.length());
  
  wf.close();
  return;
}

void nix2dows(string fname){
  string om = readF(fname);
  //om.replaceString "\n" -> "\r\n"
  writeF(fname, om);
}

//struct to read file char by char
struct CharReader {
  ifstream stream;
  int lines;
  
  CharReader(string fname): lines(0){
    stream.open(fname.c_str());
    if (! stream.is_open())
      throw runtime_error("Could not open write file. [CharReader]");
  }

  CharReader(){}

  char get(){
    char c;
    stream.get(c);
    if (c == '\n') lines++;
    return c;
  }

  bool isEof(){
    bool ans = stream.eof();
    if (ans) {stream.close();}
    return ans;
  }
};
