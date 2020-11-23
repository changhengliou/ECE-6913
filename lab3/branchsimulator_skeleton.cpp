#include <iostream>
#include <fstream>
#include <cmath>
#include <vector>
#include <sstream>

using namespace std;

// Author: ChangHeng Liou <cl5533@nyu.edu>
// #define __DEBUG__

int main (int argc, char** argv) {
	ifstream config;
#ifdef __DEBUG__
	string configPath = "/Users/line/Desktop/nyu/Computer architecture ECE-GY 6913/lab3/config.txt";
	string tracePath = "/Users/line/Desktop/nyu/Computer architecture ECE-GY 6913/lab3/trace.txt";
#else
	string configPath = argv[1];
	string tracePath = argv[2];
#endif

  config.open(configPath);

	int m, k;
#ifdef __DEBUG__
  if (!argv[1]) {
    config >> m;
  } else {
    m = atoi(argv[1]);;
  }
	k = 2;
#else
	config >> m >> k;
#endif

	config.close();

	ofstream out;
	string out_file_name = tracePath + ".out";
	out.open(out_file_name.c_str());
	
	ifstream trace;
	trace.open(tracePath);

	string line;
	vector<bitset<2>> counter(pow(2, m), bitset<2>(11));
	while (getline(trace, line)) {
		unsigned long pc; bool taken;
		istringstream iss(line);
		iss >> std::hex >> pc >> taken;

		// TODO: branch prediction here
    auto lsb = pc & static_cast<unsigned long>(pow(2, m) - 1);

    out << counter[lsb][1] << endl;

    if (taken) {
      counter[lsb] = min(static_cast<int>(counter[lsb].to_ulong() + 1), 3);
    } else {
      counter[lsb] = max(static_cast<int>(counter[lsb].to_ulong() - 1), 0);
    }
	}
	 
	trace.close();	
	out.close();
	return 0;
}
