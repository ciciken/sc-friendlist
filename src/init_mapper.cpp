#include <iostream>
#include <vector>
#include <sstream>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
using namespace std;
using std::tr1::unordered_set;
using std::tr1::unordered_map;

// can probably optimize here and avoid istringstream
// but wanted to keep to stl ( also my c++ is very dusty atm )
vector<string> toArray(string in, char delim) {
    vector<string> result;
    istringstream iss(in);
    string token;
    while(getline(iss, token, delim)) {
        result.push_back(token);
    }
    return result;
}

int main()
{
  typedef unordered_map <string, unordered_set<string> > cicimap;
  cicimap m;
  string line;

  while(getline(cin, line))
  {
    vector<string> names = toArray(line, '\t');
    m[names[0]].insert(names[1]);
    m[names[1]].insert(names[0]);
  }

  // this traverses through entire map which can get costly
  // if there are n friends and they are all friends with each other this is n^2
  for (cicimap::iterator im = m.begin(); im != m.end(); ++im) {
    cout << im->first << '\t';
    string onedeg = "";
    for (unordered_set<string>::iterator i = im->second.begin(); i != im->second.end(); ++i) {
      onedeg = onedeg + *i + ",";
        //cout << *i << ',';
    }
    cout << onedeg << '\t' << onedeg << '\t' << onedeg << endl;
  }
}


