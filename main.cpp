#include <unistd.h>
#include <semaphore.h>
#include <dirent.h>
//on linux it's on /usr/lib/lane dir. so I copy the header locally to prevent os niss it
#include "lame.h"

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>

#define DEBUG       1         //0 doesn't print nathing. 1 print when a file was converted
#define PCM_SIZE    8192
#define MP3_SIZE    8192
#define SAMPLE_RATE 44100

//----------GLOBALS---------------------------
typedef std::vector<std::string> String_Vector;                             // to save file list
unsigned int                     Cores=std::thread::hardware_concurrency(); // cores number
sem_t                            Task_Sem;                                  // one sem 4 each task
std::mutex                       Print_Mutex;                               // to print concurrently
//----------PROTOTIPES---------------------------
void Find_Wav_Files              ( const std::string& dir, String_Vector& files     );
int  Convert_One_File_Wav_To_Mp3 ( std::string Wav_File_Name                        );
void Convert_Wav_To_Mp3          ( const std::string& Dir, String_Vector& Wav_Files );
//----------MAIN---------------------------------
int main(int argc, char** argv)
{
   if(argc < 2 )
      exit(1);
   else {
#if DEBUG==1
      std::cout << "wav2mp3 1.0 working with " << Cores << " cores" << std::endl;
#endif
      std::string   Dir_Name=argv[1];              //expect the first parameter was the dir
      String_Vector Files;
      if(Dir_Name.back()!='/')
         Dir_Name+='/';                            //add the slash if the user doesn't include that (like me..)
      sem_init           ( &Task_Sem,0,Cores );    //one sem per core (dinamically allocate Cores sem)
      Find_Wav_Files     ( Dir_Name,Files    );    //fill files names in Files string vector
      Convert_Wav_To_Mp3 ( Dir_Name,Files    );    //justo do the job miltitasking
   }
   return 0;
}
//----------FUNCS------------------------
//Read dir and fill Files with all wav (case insensitive) files inside Dir
void Find_Wav_Files(const std::string& Dir, String_Vector& Files)
{
   DIR* Dirp = opendir(Dir.c_str());
   struct dirent* Dp;
   while ((Dp = readdir(Dirp)) != NULL) {
      std::string File_Name=Dp->d_name;
      if(File_Name.length() > 4 ) {
         std::string File_Extension = File_Name.substr(File_Name.find_last_of(".") + 1);
         std::transform(File_Extension.begin(),File_Extension.end(), File_Extension.begin(), ::tolower);
         if(File_Extension=="wav")
            Files.push_back(File_Name);
      }
   }
   closedir(Dirp);
}
//-----------------------------------------------------------------
//dispach one task per core to convert wav2mp3 in detached mode, so I don't need to 
//keep a structure with task availables. I use a counting semphr for that 
void Convert_Wav_To_Mp3(const std::string& Dir, String_Vector& Wav_Files)
{
   for(auto F: Wav_Files) {
      sem_wait(&Task_Sem);

      std::thread Thread=std::thread(Convert_One_File_Wav_To_Mp3,(Dir + F));
      Thread.join();
      //Thread.detach();
   }
   //when all wav in his own task, wait for all sems being post but without overload the core
   //that runs main
   //for ( int Pending=0;Pending<Cores;sem_getvalue(&Task_Sem,&Pending ))
   //   ;
   //sleep(0.1);
}
//-----------------------------------------------------------------
//it's a C snippet from lame doc to read a file and convert to mp3 with rasonable settings
//I added a debug message when the file was ready or an error when something happend
int Convert_One_File_Wav_To_Mp3(std::string Wav_File_Name)
{
   int read, write;
   bool Ok=false;

   std::string Mp3_File_Name=Wav_File_Name.substr ( 0,Wav_File_Name.find_last_of(".")+1) + "mp3";

   FILE *pcm = fopen(Wav_File_Name.c_str(), "rb");
   FILE *mp3 = fopen(Mp3_File_Name.c_str(), "wb");
   if(pcm != NULL && mp3 != NULL) {
      Ok=true;                                  //just a minimal error check
      short int     pcm_buffer[ PCM_SIZE*2 ];
      unsigned char mp3_buffer[ MP3_SIZE   ];

      lame_t lame = lame_init();
      lame_set_in_samplerate ( lame, SAMPLE_RATE );
      lame_set_VBR           ( lame, vbr_default );
      lame_init_params       ( lame              );

      do {
         read = fread(pcm_buffer, 2*sizeof(short int), PCM_SIZE, pcm);
         if (read == 0)
            write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
         else
            write = lame_encode_buffer_interleaved(lame, pcm_buffer, read, mp3_buffer, MP3_SIZE);
         fwrite(mp3_buffer, write, 1, mp3);
      } while (read != 0);

      lame_close ( lame );
      fclose     ( mp3  );
      fclose     ( pcm  );
   }
#if DEBUG==1
   if(Ok == true) {
      Print_Mutex.lock();
         std::cout<< Wav_File_Name << " -> " << Mp3_File_Name << std::endl; //debug
      Print_Mutex.unlock();
   }
#endif
   sem_post(&Task_Sem); //free the sem
   return 0;            //it's the same as pthread_exit(0)
}

