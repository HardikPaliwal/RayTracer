#include <iostream>
#include "scene_lua.hpp"
#include <chrono>
#include <time.h>
#include <stdio.h>

int main(int argc, char** argv)
{
  std::string filename = "simple.lua";
  bool isMultiThreaded = true;
  int numberOfThreads = 4;
  int numberOfSamples = 10;
  if (argc >= 2) {
    filename = argv[1];
  }

  time_t start,end;
  time (&start);
  if (!run_lua(filename)) {
    std::cerr << "Could not open " << filename <<
                 ". Try running the executable from inside of" <<
                 " the Assets/ directory" << std::endl;
    return 1;
  }
  
  time (&end);
  double dif = difftime (end,start);
  printf ("Runtime: %.2lf seconds.\n", dif );
}
