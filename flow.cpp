#include <string>
#include <vector>

#include "io.hpp"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#define pl(t) (std::cout << t << std::endl)
#define pel(t) (std::cerr << t << std::endl)
#define pls(t) (std::cout << #t << " " << t << std::endl)

/*
!!! Can use std::string.push_back(char c) !!!!
!!! 
!!! 
 */
std::string wholeFile;
int i=0;
std::string reader(){
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
  return -1;
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
	std::string ci;
	ci = reader();
	int rt = (c=="`"?2:0);
	  while(ci.size()){
	
	  if (ci == "~"){
	    std::string cz = reader();
	    if (! cz.size()) { fin = true; return new Chunk (rt, glob + ci);}
	    else glob += ((cz==c || cz==ci)?cz:ci+cz);
	  }
	  else if (ci == c) return new Chunk (rt, glob);
	  else glob += ci;
	  ci = reader();
	  } fin = true; return new Chunk (rt, glob);
	
      } else {
	return new Chunk(cat, c);
      }
    }
    c = reader();
  } while(c.size());
}

#define TEXT 1
#define VAR 2

#include <map>

class Mold{

  std::vector<std::string> lists[2];
  int first, last;

  bool wasnt(std::string var, int type){
    if (last != type){
      if (! last) first = type;
      last = type;
      lists[type-1].push_back(var);
      return true;
    } else return false;
  }

public:
  std::map<std::string, Mold*> subMolds;
  std::string wad, pad, word;
  
  Mold* parent;
  
  Mold(Mold* m):
    parent(m)
  {
    last = first = 0;
  }

  bool addVar(std::string var){
    return wasnt(var, VAR);
  }

  bool addText(std::string txt){return wasnt(txt, TEXT);}

  Mold* addSub(std::string name){
    Mold *m = new Mold(this);
    subMolds[name] = m;
    return m;
  }

  std::vector<std::string> getList(int i){
    return lists[i-1];
  }
  
  bool firstWasVar(){return first == VAR;}

  bool hasSubs(){return subMolds.size() > 0;}

  bool isSimple()
  {return !hasSubs() && first == TEXT && ! lists[VAR-1].size();}

  void setWord(std::string w){
    wad += word+pad;
    word = w;
  }

  void clearWord(){word=pad="";}

  void closeText (){// only used for backquotes and eof
    if (word.length() || wad.length()){
      addText(wad+word+pad);
      wad=word=pad="";
    }
  }
  
  void print(){

    int i, j;
    for (i=0; i<2; i++){
      std::string p = i?"Vars:":"Texts";
      pl(p);
      for (j=0; j<lists[i].size(); j++){
	pl(lists[i][j]);
	pl("~~~~~~~~~~~~~~~~~~~~~~~~~");
      }}

    for (auto mi = subMolds.begin(); mi != subMolds.end(); ++mi){
      pl("SubMold:");
      pl(mi->first);
      mi->second->print();
    }
    //#iterate through a map
    //#print keys
    //#and then call this print function on the values
  }
};

struct Enclosing{

  virtual std::string open(){return "";}

  virtual void mutate(){}

  virtual std::string close() = 0;

  virtual bool match(std::string m) = 0;

  virtual std::string toString() = 0;
};

Mold* mld;

struct MoldEnclosing : public Enclosing{

  MoldEnclosing(std::string n){
    mld = mld->addSub(n);
  }

  std::string close(){
    mld->closeText();
    mld = mld->parent;
    return "";
  }

  bool match(std::string m){return "}" == m;}

  std::string toString(){return "mold}";}
};

struct TagEnclosing : public Enclosing{
  bool angle;
  std::string name;
  
  TagEnclosing(std::string n, bool a):
    angle(a), name(n){}

  std::string open(){
    return "<"+name+(angle?">":" ");
  }

  void mutate(){
    angle = true;
  }

  bool match(std::string m)
  {
    return (angle?">":"]") == m;
  }
  
  std::string close(){
    return angle?"</"+name+">":">";
  }

  std::string toString(){return name+(angle?">":"]");}
  
};

class Stack{
  std::vector<Enclosing*> closings;
  bool header;

  void print(){
    for (Enclosing *e : closings){
      std::cerr << e->toString() << " ";
    }
    std::cerr << std::endl;
  }

public:

  Stack():
    header(false){}

  void update(bool b){
    if (!b && header){
      header = 0;
      closings.pop_back();
    }
  }

  void push(std::string opener){

    if (header){
      closings.back()->mutate();
      header = false;
    }
    else if (mld->word.length()){
	mld->wad += openEnclosing(opener);
    } else ; //anonymous enclosing error
  }

  void pop(std::string closer){
    if (! closings.size()){
      pel("Empty stack!"); return;
    }
    Enclosing *inner = closings.back();
    if (! inner->match(closer)) {
      pel( "mismatched enclosings");
      return;
    }
    mld->setWord("");
    header = (closer == "]");
    if (! header) closings.pop_back();
    mld->wad += header?">":inner->close();
  }
  
  std::string openEnclosing(std::string opener){
    Enclosing *gnu;

    std::string word = mld->word;
    mld->clearWord();

    if (opener == "{")
      gnu = new MoldEnclosing(word);
    else
      gnu = new TagEnclosing(word, opener=="<");
    
    closings.push_back(gnu);
    return gnu->open();
  }
};

Stack stack;

Mold* run(){
  Chunk* ck;
  int type, lines = 0;
  std::string str;

  mld = new Mold(0);
  while (ck = chunker()){

    str = ck->it;
    type = ck->type;
    stack.update(type == 3 || (type == 4 && str == "<"));
    if (!type){ // i aint got no type
      mld->setWord(str);
    }
    else {
      if (type == 2){//back quote
	mld->closeText(); 
	mld->addVar(str);
      }
      else if (type == 3){ //whitespace
	if (str == "\n") lines++;
	mld->pad += str;
      }
      else if (type == 4){//opener
	stack.push(str);
      }
      else if (type == 5){//closer
	stack.pop(str);
      } else {
	pel("Your Chunk Was Maltyped!");
	return 0;
      }
    }
  }
  mld->closeText();
  return mld;
}


rapidjson::Document jdoc;
rapidjson::Document::AllocatorType& alloc = jdoc.GetAllocator();

rapidjson::Value* jArray(std::vector<std::string> lzt){
  rapidjson::Value *arr = new rapidjson::Value(rapidjson::kArrayType);
  for (int i=0; i<lzt.size(); i++)
    arr->PushBack(rapidjson::StringRef(lzt[i].c_str()), alloc);
  return arr;
}

rapidjson::Value* cycle(Mold *m){
  rapidjson::Value* v = new rapidjson::Value(rapidjson::kObjectType);  
  v->AddMember("firstWasVar", m->firstWasVar(), alloc);
  v->AddMember("vars", *jArray(m->getList(VAR)), alloc);
  v->AddMember("texts", *jArray(m->getList(TEXT)), alloc);

  rapidjson::Value* subs = new rapidjson::Value(rapidjson::kObjectType);
  for (auto mi = m->subMolds.begin(); mi != m->subMolds.end(); ++mi){
    subs->AddMember(rapidjson::StringRef(mi->first.c_str()), *cycle(mi->second), alloc);
  }
  v->AddMember("subs", *subs, alloc);
  return v;
}

std::string toJSON(Mold *m){

  jdoc.SetObject(); //change jdoc from an array to an obj

  rapidjson::Value* rootMold = cycle(m);
    
  jdoc.AddMember("firstWasVar", m->firstWasVar(), alloc);
  jdoc.AddMember("vars", *jArray(m->getList(VAR)), alloc);
  jdoc.AddMember("texts", *jArray(m->getList(TEXT)), alloc);

  rapidjson::Value* subs = new rapidjson::Value(rapidjson::kObjectType);
  for (auto mi = m->subMolds.begin(); mi != m->subMolds.end(); ++mi){
    subs->AddMember(rapidjson::StringRef(mi->first.c_str()), *cycle(mi->second), alloc);
  }
  jdoc.AddMember("subs", *subs, alloc);

  // 3. Stringify the DOM
  rapidjson::StringBuffer buffer2;
  rapidjson::Writer<rapidjson::StringBuffer> writer2(buffer2);
  jdoc.Accept(writer2);

  return buffer2.GetString();
}

int main(int argc, char *argv[]){
  std::cout << "translate.sh sites/community/domains/subs/slack/" << std::endl;

  wholeFile = readF(argv[1]);
  pl(toJSON(run()));
  return 0;
}
