#include <iostream>
#include <vector>
#include <sstream>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
#include <set>
using namespace std;
using std::tr1::unordered_set;
using std::tr1::unordered_map;

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
  cicimap accummap;
  string line;
  // for each input, accumulate the previously traversed (1 to k-1 degree) friends
  // as well as all the k degree friends
  while(getline(cin, line))
  {
    vector<string> names = toArray(line, '\t');
    string namekey = names[0];
    vector<string> prevfriends = toArray(names[2], ',');
    vector<string> kfriends = toArray(names.back(), ',');

     // add k degree friends
    accummap[namekey].insert(kfriends.begin(), kfriends.end());
    // add prev traversed friends
    accummap[namekey].insert(prevfriends.begin(), prevfriends.end());
  }

  // output for each name the sorted list of 1 to k degree friends
  for (cicimap::iterator im = accummap.begin(); im != accummap.end(); ++im) {
    cout << im->first << '\t';

    // sort im->second (the unsorted set of 1-k degree friends)
    set<string> sorted(im->second.begin(), im->second.end());

    for (set<string>::iterator i = sorted.begin(); i != sorted.end(); ++i) {
        cout << *i << '\t';
    }

    cout << endl;
  }

}





