#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

std::string readF(std::string fname){
  std::ifstream rf;
  rf.open(fname.c_str());

  if (! rf.is_open())
    throw std::runtime_error("Could not open read file.");

  std::string line, tot="";
  while(! (getline(rf, line)).eof()){
    tot += line+'\n';
  } tot += line+'\n';
  rf.close();
  return tot;
}

void writeF(std::string fname, std::string data){
  std::ofstream wf;
  wf.open(fname.c_str());
  if (! wf.is_open())
    throw std::runtime_error("Could not open write file.");

  wf.write(data.c_str(), data.length());
  
  wf.close();
  return;
}

void nix2dows(std::string fname){
  std::string om = readF(fname);
  //om.replaceString "\n" -> "\r\n"
  writeF(fname, om);
}
