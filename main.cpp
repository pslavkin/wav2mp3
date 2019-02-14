#include <sys/types.h>
#include <getopt.h>
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>

typedef std::vector<std::string> String_Vector;

void List_Dirs(const std::string& dir, String_Vector& files)
{
 DIR* dirp = opendir(dir.c_str());
    struct dirent* dp;
    while ((dp = readdir(dirp)) != NULL) {
       files.push_back(dp->d_name);
       std::cout << dp->d_name << std::endl;
    }
    closedir(dirp);
}


int main(int argc, char** argv)
{
   if(argc < 2 )
      exit(1);
   else {
      std::string Dir;
      String_Vector Files;
      Dir = argv[1];
      List_Dirs(Dir,Files);
   }
   return 0;
}

