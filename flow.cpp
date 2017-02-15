#include <string>
#include "io.hpp"

std::string wholeFile;
int i=0;
std::string reader(){
  //  std::cout << "test "  << wholeFile.size() << (i < wholeFile.size()) << std::endl;
  if (i < wholeFile.size())
    return std::string(1, wholeFile[i++]);
  return "";
}

//0: Normal, 1: Escape, 2: Quotes, 3: Whitespace, 4: Openers, 5: Closers
char spec[] = "\t\n <>[]`{|}~";
char spec_len = 12;

int cats[] = {3, 3, 3, 4, 5, 4, 5, 2, 4, 2, 5, 1};

int sp_search(char c){
  //checks if c is a special char
  //returns its index in spec[] if so, -1 otherwise

  int mid, low = 0, high = spec_len-1;

  while (high > low){
    mid = (high+low)/2;

    if (c <= spec[low])
      return c == spec[low] ? low : -1;
  
    else if (c >= spec[high])
      return c == spec[high] ? high : -1;

    else {
      char diff = c - spec[mid];
      if (diff < 0) high = mid-1;
      else if (diff > 0) low = mid+1;
      else return mid;
    }
  }
  return 0;
}

int category(std::string c){
  int dex = sp_search(c[0]);
  if (dex == -1) return 0;
  else return cats[dex];
}

struct Chunk {

  int type;
  std::string it;

  //# initializer list
  Chunk(int t, std::string i){
    type = t;
    it = i;
  }
};

std::string onDeck;
int onDeckCat;
bool fin = false;
std::string glob;

Chunk* chunker(){
  if (fin) return 0;

  glob = "";

  std::string c; int cat;
  if (onDeck.size()) {
    c = onDeck;
    cat = onDeckCat;
    onDeck = "";
  } else {
    c = reader();
    if (! c.size()) return 0;
  }

  //0: Normal, 1: Escape, 2: Quotes, 3: Whitespace, 4: Openers, 5: Closers

  do {
    cat = category(c);

    //    std::cout << "char, cat: " << c <<", "<<cat<<std::endl;
    
    if (!cat) glob += c; //normal std::string
    
    else if (cat == 1){ //escape std::string
      std::string cn = reader();
      if (! cn.size()){
	fin = true;
	return new Chunk(0, glob);
      }
      else glob += category(cn) ? cn:c+cn;
    }

    else { //chunk breakers

      if (glob.size()){ //theres a word, pass it & come baclk
	onDeck = c;
	onDeckCat = cat;
	return new Chunk (0, glob);
	}
      //This guarantees that glob is clear for use in rest of bracket

      if (cat == 2) {//quote std::string
	glob += c; //c is the quote std::string
	std::string ci;
	ci = reader();
	  while(ci.size()){
	
	  if (ci == "~"){
	    std::string cz = reader();
	    if (! cz.size()) { fin = true; return new Chunk (0, glob + ci);}
	    else glob += ((cz==c || cz==ci)?cz:ci+cz);
	  }
	  else if (ci == c) return new Chunk (0, glob+c);
	  else glob += ci;
	  ci = reader();
	  } fin = true; return new Chunk (0, glob);
	
      } else {
	return new Chunk(cat, c);
      }
    }
    c = reader();
  } while(c.size());
}

int main(int argc, char *argv[]){

  wholeFile = readF(argv[1]);
  
  Chunk* ck;
  while (ck = chunker())
    std::cout << ck->type << " : " << ck->it << std::endl;
  
  return 0;
}
