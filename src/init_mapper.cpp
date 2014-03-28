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
vector<string> toArray(string in) {
    vector<string> result;
    istringstream iss(in);
    string token;
    while(getline(iss, token, '\t')) {
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
    vector<string> names = toArray(line);
    m[names[0]].insert(names[1]);
    m[names[1]].insert(names[0]);
  }

  // unforuntaely nested for loop
  // insert is n (*2) but mapping here can get costly, could even be
  // suboptimized again into parallel task :P
  for (cicimap::iterator im = m.begin(); im != m.end(); ++im) {
    cout << im->first << '\t';
    for (unordered_set<string>::iterator i = im->second.begin(); i != im->second.end(); ++i) {
      cout << *i << '\t';
    }
    cout << endl;
  }
}


