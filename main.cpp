#include <sys/types.h>
#include <string.h>
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
       std::string File_Name=dp->d_name;
       if(File_Name.length() > 4 ) {
          std::string File_Extension = File_Name.substr(File_Name.find_last_of(".") + 1);
          std::transform(File_Extension.begin(),File_Extension.end(), File_Extension.begin(), ::tolower);
          if(File_Extension=="wav") {
             files.push_back(File_Name);
             std::cout << File_Name << std::endl; //debug
          }
       }
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

