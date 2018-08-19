#include "io.hpp"
#include "string.hpp"
#include "json/json.h"
#include <map>
using namespace std;

struct SpecChar {
  char ch;
  string name;
  int category;
  SpecChar(char c, string n, int i):
    ch(c), name(n), category(i){}
};

vector<SpecChar> allChars;
map<char, SpecChar*> charMap;
void setChars(){
  allChars.push_back(SpecChar('\t', "tab", 3));
  allChars.push_back(SpecChar('\n', "newline", 3));
  allChars.push_back(SpecChar(' ', "space", 3));
  allChars.push_back(SpecChar('<', "open angle", 4));
  allChars.push_back(SpecChar('>', "close angle", 5));
  allChars.push_back(SpecChar('[', "open bracket", 4));
  allChars.push_back(SpecChar(']', "close bracket", 5));
  allChars.push_back(SpecChar('`', "back quotes", 2));
  allChars.push_back(SpecChar('{', "open curly", 4));
  allChars.push_back(SpecChar('|', "bar", 2));
  allChars.push_back(SpecChar('}', "close curly", 5));
  allChars.push_back(SpecChar('~', "tilde", 1));
  for (int i=0; i<allChars.size(); i++){
    SpecChar* sc = &allChars[i];
    charMap[sc->ch] = sc;
  }
}

string conF = "aml.conf";
CharReader cRead;

vector<string> getProps(){
  if (!readable(conF)){
    string orig = "";
    vector<SpecChar>::iterator ptr;
    for (ptr=allChars.begin(); ptr < allChars.end(); ptr++){
      orig += ptr->name + " = true\n";
    }
    writeF(conF, orig);
  }
  return readLzt(conF);
}

int findCharByName(std::string name){
  for (int i=0; i<allChars.size(); i++){
    if (allChars[i].name.compare(name) == 0){
      return i;
    }
  }
  return -1;
}

void getSpecChars(){
  setChars();
  vector<SpecChar> specChars;
  vector<string> props = getProps();
  for (int i=0; i<props.size(); i++){
    string prop = props[i];
    int d = prop.find("=");
    std::string key = trim(prop.substr(0, d));
    std::string val = trim(prop.substr(d+1));
    if (val.find("true") != string::npos){
      int sc = findCharByName(key);
      if (sc >= 0){
	specChars.push_back(allChars[sc]);
      }
    }
  }
}

int category(char c){
  if (charMap.count(c) == 0) {return 0;}
  return charMap.at(c)->category;
}

struct Chunk {
  int type;
  std::string it;
  Chunk(int t, std::string i): type(t), it(i){}
};

bool fin = false;
std::string glob;
char onDeck;
int onDeckCat;

Chunk* chunker(){
  if (fin) return 0;

  glob = "";

  char c; int cat;
  if (onDeck != '\0') {
    c = onDeck;
    cat = onDeckCat;
    onDeck = '\0';
  } else if (!cRead.isEof()){
    c = cRead.get();
  } else return 0;

  //0: Normal, 1: Escape, 2: Quotes, 3: Whitespace, 4: Openers, 5: Closers

  do {
    cat = category(c);
    
    if (!cat) glob.push_back(c); //normal char
    
    else if (cat == 1){ //escape char
      if (cRead.isEof()){
	fin = true;
	return new Chunk(0, glob);
      } else {
	char cn = cRead.get();
	if (! category(cn)) glob.push_back(c); //If next char needn't ne escaped, treat escape char as normal char
	glob.push_back(cn);
      }
    }

    else { //chunk breakers

      if (glob.size()){ //theres a word, pass it & come baclk
	onDeck = c;
	onDeckCat = cat;
	return new Chunk (0, glob);
	}
      //This guarantees that glob is clear for use in rest of bracket

      if (cat == 2) {//quote char
	int rt = (c=='`'?2:0);
	while(!cRead.isEof()){
	  char ci = cRead.get();	
	  if (ci == '~'){
	    if (cRead.isEof()){
	      fin = true;
	      glob.push_back(ci);
	      return new Chunk (rt, glob);
	    } else {
	      char cz = cRead.get();
	      //within quotes, ~ only escapes the type of quotes & itself
	      if (!(cz==c || cz=='~')) glob.push_back(ci);
	      glob.push_back(cz);
	    }
	  }
	  else if (ci == c) return new Chunk (rt, glob);
	  else glob.push_back(ci);
	}
	fin = true;
	return new Chunk (rt, glob);
	
      } else {
	return new Chunk(cat, string(1, c));
      }
    }
    c = cRead.isEof() ? '\0' : cRead.get();
  } while(c != '\0');
  return 0;
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

  std::string firstText(){
    return lists[TEXT-1][0];
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

int lines = 0;
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
      header = false;
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
      pel("On line "<<lines<<":Empty stak! Trying to use a " << closer); return;
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

Stack stak;

Mold* run(){
  Chunk* ck;
  int type;
  type = lines = 0;
  std::string str;

  mld = new Mold(0);
  while ((ck = chunker())){
    str = ck->it;
    type = ck->type;
    stak.update(type == 3 || (type == 4 && str == "<"));
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
	stak.push(str);
      }
      else if (type == 5){//closer
	stak.pop(str);
      } else {
	pel("Your Chunk Was Maltyped!");
	return 0;
      }
    }
  }
  mld->closeText();
  return mld;
}

Json::Value makeList(vector<string> lzt){
  Json::Value jsonArray = Json::arrayValue;
  for (int i=0; i<lzt.size(); i++){
    jsonArray.append(lzt[i]);
  }
  return jsonArray;
}

const string toString(Json::Value* json){
  Json::StreamWriterBuilder builder;
  // builder["indentation"] = ""; // If you want whitespace-less output
  return Json::writeString(builder, *json);
}

Json::Value* layer(Mold *m){
  Json::Value* root = new Json::Value;
  (*root)["firstWasVar"] = m->firstWasVar();
  (*root)["texts"] = makeList(m->getList(TEXT));
  (*root)["vars"] = makeList(m->getList(VAR));
  int i = 0;
  Json::Value subs = Json::objectValue;
  for (auto mi = m->subMolds.begin(); mi != m->subMolds.end(); ++mi, i++){
    Json::Value* sub = layer(mi->second);
    subs[mi->first] = *sub;
  }
  (*root)["subs"] = subs;
  return root;
}

std::string toJSON(Mold *m){
  Json::Value* root = layer(m);
 return toString(root);
}

int main(int argc, char *argv[]){
  getSpecChars(); 
  cRead = CharReader(argv[1]);

  Mold *mold = run();
  if (! mold->isSimple())
    pl('j' << toJSON(mold));
  else{
    pl('h' << mold->firstText());
  }
  return 0;
}
