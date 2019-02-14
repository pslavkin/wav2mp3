#include <sys/types.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <dirent.h>
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <iterator>
#include <thread>
#include <mutex>

typedef std::vector<std::string> String_Vector;
std::string   Dir;
String_Vector Files;
std::thread Threads[4];
std::mutex Mutexes[4];
//-----------------------------------------------
void Find_Wav_Files(const std::string& dir, String_Vector& files);
void Convert_Wav_To_Mp3(String_Vector& Wav_Files);
void Convert_One_File_Wav_To_Mp3(std::string Wav_File_Name,char i);
//-----------------------------------------------
int main(int argc, char** argv)
{
   if(argc < 2 )
      exit(1);
   else {
      Dir = argv[1];
      Find_Wav_Files(Dir,Files);
      Convert_Wav_To_Mp3(Files);
   }
   return 0;
}
//---------------------------------------
void Find_Wav_Files(const std::string& dir, String_Vector& files)
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

void Convert_Wav_To_Mp3(String_Vector& Wav_Files)
{
   bool On_Work;
   for(auto F: Wav_Files) {
      On_Work=false;
      while(On_Work==false) {
         for(int i=0; i<4 ; i++) {
            if(Mutexes[i].try_lock()) {
               if(Threads[i].joinable())
                  Threads[i].join();
               Threads[i]=std::thread(Convert_One_File_Wav_To_Mp3,(Dir + F),i);
               On_Work=true;
               break;
            }
         }
      }
   }
   for(int i=0; i<4 ; i++) {
      if(Threads[i].joinable())
         Threads[i].join();
   }
}
void Convert_One_File_Wav_To_Mp3(std::string Wav_File_Name,char i)
{
   std::string S;
   std::ofstream Mp3_File;
   std::ifstream Wav_File;
   std::string Mp3_File_Name=Wav_File_Name.substr ( 0,Wav_File_Name.find_last_of(".")+1) +
      "mp3";
   Wav_File.open(Wav_File_Name);
   Mp3_File.open(Mp3_File_Name);
   while(std::getline(Wav_File,S)) {
      Mp3_File << S << std::endl;
   }
   std::cout<< "convirtiendo " << Wav_File_Name << " -> " << Mp3_File_Name << std::endl;
   sleep(1);
   Wav_File.close();
   Mp3_File.close();
   Mutexes[i].unlock();
}
