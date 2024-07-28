//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include <windows.h>
#include <dir.h>
#include "FConvert.h"
//---------------------------------------------------------------------------
BOOL WINAPI DllMain(HINSTANCE hinstDLL,  // handle to DLL module
                    DWORD fdwReason,     // reason for calling function
                    LPVOID lpReserved)   // reserved
{
   // Perform actions based on the reason for calling.
   switch(fdwReason)
   {
      case DLL_PROCESS_ATTACH:
         // Initialize once for each new process.
         // Return FALSE to fail DLL load.
         break;
      case DLL_THREAD_ATTACH:
         // Do thread-specific initialization.
         break;
      case DLL_THREAD_DETACH:
         // Do thread-specific cleanup.
         break;
      case DLL_PROCESS_DETACH:
         // Perform any necessary cleanup.
         break;
   }
   return TRUE;  // Successful DLL_PROCESS_ATTACH.
}
//---------------------------------------------------------------------------
FConvert Convert;
//---------------------------------------------------------------------------
extern "C" bool __declspec(dllexport) Install(void(*ProgressMax)(int), void(*ProgressCur)(int))
{
   return Convert.Reset(ProgressMax, ProgressCur);
}
//---------------------------------------------------------------------------
extern "C" bool __declspec(dllexport) Add(const char* FileName, const char* Parameters)
{
   return Convert.AddProjectItem(FileName, Parameters);
}
//---------------------------------------------------------------------------
extern "C" bool __declspec(dllexport) Process(void)
{
   return Convert.Process();
}
//---------------------------------------------------------------------------
extern "C" bool __declspec(dllexport) SaveToFile(const char* FileName)
{
   static char drive[MAXDRIVE];
   static char dir[MAXDIR];
   static char file[MAXFILE];
   static char ext[MAXEXT];
   static char FileNameS[MAXPATH];

   fnsplit(FileName, drive, dir, file, ext);
   fnmerge(FileNameS, drive, dir, file, "S.g2d");

   if(!Convert.SaveToFile(FileName, &Convert.Triangulator)) return false;
//   if(!Convert.SaveToFile(FileName, &Convert.TriangulatorS)) return false;

   return true;
}
//---------------------------------------------------------------------------

