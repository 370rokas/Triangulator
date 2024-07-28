//---------------------------------------------------------------------------

#include "FMM\\FMM.h"
#include <vcl.h>
#pragma hdrstop
USEFORM("Unit1.cpp", Form1);
//---------------------------------------------------------------------------
void _ERROR(const char* format, ... );
//---------------------------------------------------------------------------
WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
   try
   {
       Application->Initialize();
       Application->CreateForm(__classid(TForm1), &Form1);
       Application->Run();
   }
   catch (Exception &exception)
   {
      _ERROR(exception.Message.c_str());
      Application->ShowException(&exception);
   }
   return 0;
}
//---------------------------------------------------------------------------
