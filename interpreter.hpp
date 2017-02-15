
char spec[] = "\t\n <>[]`{|}~";
char spec_len = 12;

int cats[] = {1, 1, 1, 4, 5, 4, 5, 3, 4, 3, 5, 2};

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



int category(char c){
  int dex = sp_search(c);
  if (dex == -1) return 0;
  else return cats[dex];
}

int lineN = 0; //line number
int cws = 0; //start of current word
int cwl = 0; //len of current word
int wcs = 0; //start of current write chunk
int i = 0;
bool inWord = false;
bool hasWord = false;
bool inTag = true;
std::string html = "";
std::string aml;

void whsp(){
  if (inWord){
    cwl = cws - i;
    inWord = false;
  }
}

void plain(){ //only requires action if not in a word
  if (!inWord){
    hasWord = inWord = true;
    cws = i;
  }
}

void open_ang() {// <
  //two cases eiter already in tag or not..
  if (! hasWord || inTag)
    throw std::runtime_error("Tag with no name.");

  html += aml.substr(wcs, wcs-i);
  std::string word = aml.substr(cws, inWord?cws-i:cwl);
      
}

int sWord, sChunk, sWordA, eWord;

#define InWord (sWord != -1)
#define InChunk (sChunk != -1)
#define HasWord (sWordA != -1)

std::string interpretB(){
  sWord = sChunk = sWordA = -1;
  for (i=0; i< aml.size(); i++){
    char c = aml[i];
    int cat = category(c);

    std::string chunk = "";

    if (! cat) { //part of a word
      if (! InWord){
	sWord = i;
	if (! InChunk)
	  sChunk = i;
      }
    } else { //not part of a word
      if (InWord){
	sWordA = sWord;
	eWord = i;
	sWord = -1;
      }

      if(cat == 1 && ! InChunk) sChunk = i;
      } else {

      //feed chunk
      chunk += aml.substring(sChunk, i);
      sChunk = -1;
      
      if (cat == 2){//escape char
	sWord = sChunk = (category(aml[i+1]))?++i:i;
      } else {
	bool opener;
	if (cat == 3){

	} else if (cat == 4){
	  opener = true;
	} else {
	  opener = false;
	}
}

std::string interpret(){
  for (i=0; i< aml.size(); i++){
    char c = aml[i];
    int cat = category(c);

    if (cat == 0) plain();
    else { //is special

      if (cat == 1){ //whitespace
	if(c == '\n')
	  lineN++;
	whsp();
      }
      else { //is a consumer

	if (cat == 2){ //opener
	  
	}
    
	else if (cat == 3){ //a closer
	  char opener;
	  if (c == '}') opener = '{';
	  else if (c == ']') opener = '[';
	  else opener = '<'; //closer must be >, or something's wrong
	}

	else if (cat == 4){ //a quote
	  
	}

	else if (cat == 5){ //escape, aka ~
	  
	}
      }
    }
  }
  return "burgers";
}
