#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
int main(int argc, char** argv) {
  long parity = 0;
  ifstream* infiles;
  infiles = new ifstream[argc];
  for (int i = 1; i < argc; i++) {
    infiles[i].open(argv[i]);
    string parityline;
    getline(infiles[i], parityline);
    parity += stol(parityline.substr(7, parityline.length() - 8));
  }
  cout << "confs -;\n";
  cout << "parity " << parity << ";";

  long count = 0;
  long index = 0;
  for (int i = 1; i < argc; i++) {
    while (infiles[i].good()) {
      string line;
      getline(infiles[i], line);
      if (line.length() > 0) {
        cout << "\n";
        int firstspace = line.find(' ');
        index = stol(line.substr(0, firstspace)) + count;
        int thirdspace = line.find(' ', line.find(' ', firstspace + 1) + 1);
        string edges = line.substr(thirdspace + 1, line.length() - thirdspace - 2);
        cout << index << line.substr(firstspace, thirdspace - firstspace + 1);
        int c = 0;
        int cc = 0;
        while ((c = edges.find(',', cc)) != string::npos) {
          long edgeindex = stol(edges.substr(cc, c));
          cout << (edgeindex + count) << "|-,";
          cc = c + 1;
        }
        long edgeindex = stol(edges.substr(cc, edges.length() - cc));
        cout << (edgeindex + count) << "|-;";
      }
    }
    count = index + 1;
  }
  return 0;
}