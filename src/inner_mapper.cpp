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

// the friend node
struct friends {
  vector<string> prevdegree; // the list of 1 to k-1 degree friends
  vector<string> onedegree;  // the list of 1st degree friends (children)
  vector<string> kdegree;    // the list of k degree friends
  unordered_set<string> k1degree;   // the list of k+1 degree friends
  // for output purposes:
  string prevdegreestr;
  string onedegreestr;
  string kdegreestr;
} ;

int main()
{
  typedef unordered_map <string, friends> cicimap;
  cicimap fmap;
  string line;
  // for each input, map the first degree friends as well as the k degree friends
  while(getline(cin, line))
  {
    vector<string> names = toArray(line, '\t');
    fmap[names[0]].onedegree = toArray(names[1], ',');
    fmap[names[0]].prevdegree = toArray(names[2], ',');
    fmap[names[0]].kdegree = toArray(names.back(), ',');
    // for output purposes:
    fmap[names[0]].onedegreestr = names[1];
    fmap[names[0]].prevdegreestr = names[2];
    fmap[names[0]].kdegreestr = names[3];
  }

  // for each person
  for (cicimap::iterator im = fmap.begin(); im != fmap.end(); ++im) {
    string name = im->first;
    // for each person's k degree friend
    for (int i=0; i < im->second.kdegree.size() ; ++i) {
        string kname = im->second.kdegree[i];             // the name of k degree friend
        vector<string> children  = fmap[kname].onedegree; // the children of k degree friend
        vector<string> traversed = fmap[name].prevdegree; // the traversed friends

        // for each k degree friend's 1st degree friend (child)
        for (int j=0; j < children.size() ; ++j ) {

          // check if child is in previously traversed
          if (children[j] != name &&
            std::find(traversed.begin(), traversed.end(), children[j]) == traversed.end() ) {
            // add to k+1 list
            fmap[name].k1degree.insert(children[j]);
          }
        }
    }
  }

  // output the names with k degree friends added to previously traversed list
  // and with the k+1 friends set to be the next k degree friend iteration
  for (cicimap::iterator im = fmap.begin(); im != fmap.end(); ++im) {
    friends f = im->second;

    cout << im->first << '\t' << f.onedegreestr << '\t' << f.prevdegreestr;

    if (f.k1degree.empty()) {
        cout << '\t' << f.kdegreestr; // if no new k1degree friends, keep things same
    } else {

        // append the k degree friends to traversed list avoiding duplicates
        for ( int i=0; i<f.kdegree.size(); ++i) {
          if (std::find(f.prevdegree.begin(), f.prevdegree.end(), f.kdegree[i]) == f.prevdegree.end() ) {
            cout << f.kdegree[i] << ',';
          }
        }

        // now add the k+1 degree friends
        cout << '\t';
        for (unordered_set<string>::iterator i = f.k1degree.begin(); i != f.k1degree.end(); ++i) {
          cout << *i << ",";
        }
    }

    cout << endl;
  }

}



