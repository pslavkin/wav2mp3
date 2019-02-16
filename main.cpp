#include <unistd.h>
#include <semaphore.h>
#include <dirent.h>
#include <lame/lame.h>

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include <queue>

typedef std::vector<std::string> String_Vector;
//-----------------------------------------------
String_Vector Files;
std::string   Dir;
unsigned int Cores=std::thread::hardware_concurrency();

sem_t Task_Sem;
std::mutex Print_Mutex;
//-----------------------------------------------
void Find_Wav_Files             ( const std::string& dir, String_Vector& files );
void Convert_Wav_To_Mp3         ( String_Vector& Wav_Files                     );
int Convert_One_File_Wav_To_Mp3 ( std::string Wav_File_Name                    );
//-----------------------------------------------
int main(int argc, char** argv)
{
   if(argc < 2 )
      exit(1);
   else {
      std::cout << "cores=" << Cores << std::endl;
      sem_init(&Task_Sem,0,Cores);
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
         }
      }
   }
   closedir(dirp);
}

void Convert_Wav_To_Mp3(String_Vector& Wav_Files)
{
   for(auto F: Wav_Files) {
      sem_wait(&Task_Sem);
      std::thread Thread=std::thread(Convert_One_File_Wav_To_Mp3,(Dir + F));
      Thread.detach();
   }
   for(int Pending=0;Pending<Cores;sem_getvalue(&Task_Sem,&Pending))
      ;
}
int Convert_One_File_Wav_To_Mp3(std::string Wav_File_Name)
{
   std::string Mp3_File_Name=Wav_File_Name.substr ( 0,Wav_File_Name.find_last_of(".")+1) + "mp3";
   int read, write;

   FILE *pcm = fopen(Wav_File_Name.c_str(), "rb");
   FILE *mp3 = fopen(Mp3_File_Name.c_str(), "wb");

   const int PCM_SIZE = 8192;
   const int MP3_SIZE = 8192;

   short int pcm_buffer[PCM_SIZE*2];
   unsigned char mp3_buffer[MP3_SIZE];

   lame_t lame = lame_init();
   lame_set_in_samplerate(lame, 44100);
   lame_set_VBR(lame, vbr_default);
   lame_init_params(lame);

   Print_Mutex.lock();
      std::cout<< "Begin coding " << Wav_File_Name << " -> " << Mp3_File_Name << std::endl; //debug
   Print_Mutex.unlock();
   do {
      read = fread(pcm_buffer, 2*sizeof(short int), PCM_SIZE, pcm);
      if (read == 0)
         write = lame_encode_flush(lame, mp3_buffer, MP3_SIZE);
      else
         write = lame_encode_buffer_interleaved(lame, pcm_buffer, read, mp3_buffer, MP3_SIZE);
      fwrite(mp3_buffer, write, 1, mp3);
   } while (read != 0);

   lame_close(lame);
   fclose(mp3);
   fclose(pcm);
   Print_Mutex.lock();
      std::cout<< "End coding " << Wav_File_Name << " -> " << Mp3_File_Name << std::endl; //debug
   Print_Mutex.unlock();
   sem_post(&Task_Sem);
   return 0;
}

