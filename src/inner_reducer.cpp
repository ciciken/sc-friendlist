#include <iostream>
#include <vector>
#include <sstream>
#include <tr1/unordered_map>
#include <tr1/unordered_set>
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
  cicimap accumkmap;
  string line;
  // for each input, accumulate the k degree friends
  while(getline(cin, line))
  {
    vector<string> names = toArray(line, '\t');
    string namekey = names[0] + '\t' + names[1] + '\t' + names[2];
    vector<string> kfriends = toArray(names.back(), ',');
    accumkmap[namekey].insert(kfriends.begin(), kfriends.end());
  }

  // output the names with k degree friends accumulated
  // sort doesn't need to happen until the final reducer
  for (cicimap::iterator im = accumkmap.begin(); im != accumkmap.end(); ++im) {
    cout << im->first << '\t';

    for (unordered_set<string>::iterator i = im->second.begin(); i != im->second.end(); ++i) {
        cout << *i << ",";
    }

    cout << endl;
  }

}




