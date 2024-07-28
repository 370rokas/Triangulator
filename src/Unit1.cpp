//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include <vcl.h>
#include <dir.h>
#include <signal.h>
#include <process.h>
#pragma hdrstop

#include "Unit1.h"
#include "FScreen.h"
#include "FConvert.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
FConvert Convert;
AnsiString CurrentFileName;
//---------------------------------------------------------------------------
void DrawFrame(HDC Handle, FFrame* F, FFrame* FS, FBitmap* Texture, bool isMeshDraw, bool isZDraw);
int DrawTexture(HDC Handle, FBitmap* bmp);
//---------------------------------------------------------------------------
bool g_isAnimation = !true; // - переключение режима анимаци€/здание-дерево
//---------------------------------------------------------------------------
FScreen* myScreen = NULL;
//---------------------------------------------------------------------------
void _ERROR(const char* format, ... )
{
	static char buffer[1024];
	va_list argList;
	va_start(argList, format);
	vsprintf(buffer, format, argList);
	va_end(argList);
	strcat(buffer, "\n");

   FILE* log = fopen("c:\\trLog.log", "ab");
	fputs(buffer, log);
   fprintf(log, "File: %s\n", CurrentFileName.c_str());
   fclose(log);

   _cexit();
   TerminateProcess(GetCurrentProcess(), 1);
}
//---------------------------------------------------------------------------
int CurrentFrameIdx = 0;
int CurrentTextureIdx = 0;
int CurrentX = 0;
int CurrentY = 0;
//---------------------------------------------------------------------------
int _matherr(struct _exception *a)
{
   _ERROR(a->name);
   return 0;
}
//---------------------------------------------------------------------------
int _matherrl(struct _exception *a)
{
   _ERROR(a->name);
   return 0;
}
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
   : TForm(Owner)
{
}
//---------------------------------------------------------------------------
int DrawScreen(bool isShow = true)
{
   int AlphaNulls = 0;
   if(Form1->PageControl1->ActivePageIndex == 0)
   {
      if(CurrentFrameIdx >= 0 && CurrentFrameIdx < Convert.Triangulator.Frames.sizeUsed)
      {
         FFrame* F = Convert.Triangulator.Frames[CurrentFrameIdx];
         FFrame* FS = NULL;
         if(Convert.Triangulator.Frames.sizeUsed == Convert.TriangulatorS.Frames.sizeUsed)
            FS = Convert.TriangulatorS.Frames[CurrentFrameIdx];

         DrawFrame(Form1->Canvas->Handle, F, FS,
            NULL,//Convert.Triangulator.Textures[F->TextureIdx],
            Form1->ToolButtonTri->Down, Form1->ToolButtonZ->Down);

         int TrianglesNumber = 0;
         for(int i = 0; i < F->Meshes.sizeUsed; i++)
            TrianglesNumber += F->Meshes[i]->Triangles.sizeUsed;

         Form1->StatusBar1->Panels->Items[1]->Text =
            IntToStr(F->Meshes.sizeUsed) + " meshes, " +
            IntToStr(TrianglesNumber) + " triangles. " +
            "Packed in " + FloatToStrF(Convert.Triangulator.msPackedTime, ffGeneral, 3, 0) + " ms";
      }
   }
   else
   {
      int TNum = Convert.Triangulator.Textures.sizeUsed-1;
      int TNumS = Convert.TriangulatorS.Textures.sizeUsed-1;

      if(CurrentTextureIdx >= 0)
      {
         if(CurrentTextureIdx < TNum)
         {
            FBitmap* T = Convert.Triangulator.Textures[CurrentTextureIdx];
            AlphaNulls = DrawTexture(Form1->Canvas->Handle, T);
            Form1->StatusBar1->Panels->Items[1]->Text =
               FloatToStrF(100.0-(float)AlphaNulls*100/((float)T->Width*(float)T->Height), ffGeneral, 3, 0) + "% used";
         }
         else
         if(CurrentTextureIdx - TNum < TNumS)
         {
            FBitmap* T = Convert.TriangulatorS.Textures[CurrentTextureIdx - TNum];
            AlphaNulls = DrawTexture(Form1->Canvas->Handle, T);
            Form1->StatusBar1->Panels->Items[1]->Text =
               FloatToStrF(100.0-(float)AlphaNulls*100/((float)T->Width*(float)T->Height), ffGeneral, 3, 0) + "% used";
         }
      }
   }
   if(isShow)
   {
      if(myScreen && myScreen->ScrBitmap)
      {
         Form1->MainPaintBox->Canvas->Draw(0, 0, myScreen->ScrBitmap);

         TRect rect;
        	Form1->MainPaintBox->Canvas->Brush->Color = clMenu;

         rect.Left = myScreen->Width;
         rect.Top = 0;
         rect.Right = Form1->Width;
         rect.Bottom = Form1->Height;
         Form1->MainPaintBox->Canvas->FillRect(rect);

         rect.Left = 0;
         rect.Top = myScreen->Height;
         rect.Right = myScreen->Width;
         rect.Bottom = Form1->Height;
         Form1->MainPaintBox->Canvas->FillRect(rect);
      }
   }

   return AlphaNulls;
}
//---------------------------------------------------------------------------
void ProgressMax(int p)
{
   Form1->ProgressBar1->Max = p;
   Form1->ProgressBar1->Position = 0;
   Application->ProcessMessages();
}
//---------------------------------------------------------------------------
void ProgressCur(int p)
{
   Form1->ProgressBar1->Position = p;
   Application->ProcessMessages();
}
//---------------------------------------------------------------------------
extern int GlobalSmoothValue;
extern int LastFrameEfficiency;
void __fastcall TForm1::ImportFramesProc(AnsiString FileName)
{
   GlobalSmoothValue=4;
   bool failed=false;
   do{
      ProgressBar1->Visible = true;
      ListBoxFrames->Items->Clear();
      ListBoxTextures->Items->Clear();
      Form1->StatusBar1->Panels->Items[0]->Text = "Loading frames";
      Refresh();

      // - reset the encoder
      Convert.Reset(ProgressMax, ProgressCur);
      Convert.isCorrectLevels = CorrectLevels1->Checked;
      //Convert.UserSites.release();

      // - adding a list file to the encoder
      Convert.AddProjectItem(FileName.c_str(),
         "IDXSTORE NONCOMPRESSED 1 0 100.000000 90.000000 4.000000 0 0 0 0 0 0 1 32 \"\" 0 0 1 1 0 1 0 256 2");

      // - pack it!
      Form1->StatusBar1->Panels->Items[0]->Text = "Pack frames";
      Convert.Process();

      ProgressBar1->Position = 0;
      ProgressBar1->Visible = false;
      Refresh();

      // - save it!
      AnsiString OutFileName = ChangeFileExt(FileName, ".gp2");
      static char drive[MAXDRIVE];
      static char dir[MAXDIR];
      static char file[MAXFILE];
      static char ext[MAXEXT];
      static char FileNameS[MAXPATH];

      fnsplit(OutFileName.c_str(), drive, dir, file, ext);
      fnmerge(FileNameS, drive, dir, file, "S.gp2");

      Convert.SaveToFile(OutFileName.c_str(), &Convert.Triangulator);

      if(LastFrameEfficiency<20000){
         failed=true;
      }

      for(int i = 0; i < Convert.Triangulator.Frames.sizeUsed; i++)
         ListBoxFrames->Items->Add(AnsiString("Frame") + IntToStr(i));

      for(int i = 0; i < Convert.Triangulator.Textures.sizeUsed-1; i++)
         ListBoxTextures->Items->Add(AnsiString("Texture") + IntToStr(i));

      Form1->StatusBar1->Panels->Items[0]->Text = "Success";
      DrawScreen();
      Refresh();
      GlobalSmoothValue--;
   }while(GlobalSmoothValue>1 && failed);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::MainPaintBoxPaint(TObject *Sender)
{
   if(!myScreen || !myScreen->ScrBitmap) DrawScreen();
   if(myScreen && myScreen->ScrBitmap)
      MainPaintBox->Canvas->Draw(0, 0, myScreen->ScrBitmap);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormClose(TObject *Sender, TCloseAction &Action)
{
   if(myScreen) delete myScreen;
   myScreen = NULL;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ListBoxTexturesClick(TObject *Sender)
{
   CurrentTextureIdx = ListBoxTextures->ItemIndex;
   DrawScreen();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ListBoxFramesMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   if(Shift.Contains(ssLeft))
      ActionSelectFrameExecute(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ListBoxTexturesMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   if(Shift.Contains(ssLeft))
      ListBoxTexturesClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::PageControl1Change(TObject *Sender)
{
   DrawScreen();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Splitter1Paint(TObject *Sender)
{
   StatusBar1->Panels->Items[0]->Width = Splitter1->Left;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::MainPaintBoxMouseMove(TObject *Sender,
      TShiftState Shift, int X, int Y)
{
   StatusBar1->Panels->Items[2]->Text = IntToStr(X) + ", " + IntToStr(Y);
   CurrentX = X;
   CurrentY = Y;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::MyIdleHandler(TObject *Sender, bool &Done)
{
   if(_argc >= 2)
   {
      ImportFramesProc(AnsiString(_argv[1]));
      if(_argc >= 3 && stricmp(_argv[2], "-d") == 0) unlink(_argv[1]);
      Application->Terminate();
   }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormCreate(TObject *Sender)
{
   Application->OnIdle = MyIdleHandler;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ActionOpenExecute(TObject *Sender)
{
   if(OpenDialogG2D->Execute())
   {
      ListBoxFrames->Items->BeginUpdate();
      ListBoxTextures->Items->BeginUpdate();

      ListBoxFrames->Items->Clear();
      ListBoxTextures->Items->Clear();

      CurrentFileName = OpenDialogG2D->FileName;
      Convert.LoadFromFile(CurrentFileName.c_str(), &Convert.Triangulator);

      for(int i = 0; i < Convert.Triangulator.Frames.sizeUsed; i++)
         ListBoxFrames->Items->Add(AnsiString("Frame") + IntToStr(i));

      for(int i = 0; i < Convert.Triangulator.Textures.sizeUsed-1; i++)
         ListBoxTextures->Items->Add(AnsiString("Texture") + IntToStr(i));

      ListBoxFrames->Items->EndUpdate();
      ListBoxTextures->Items->EndUpdate();

      CurrentFrameIdx = 0;
      CurrentTextureIdx = 0;

      DrawScreen();
      Refresh();
   }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ActionImportFramesExecute(TObject *Sender)
{
   if(OpenDialogLst->Execute())
   {
      CurrentFileName = OpenDialogLst->FileName;
      ImportFramesProc(CurrentFileName);
   }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ActionShowMeshExecute(TObject *Sender)
{
   DrawScreen();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ActionEnterEditModeExecute(TObject *Sender)
{
   ListBoxFrames->Enabled = false;
   ToolButtonOkEditMode->Visible = true;
   ToolButtonCancelEditMode->Visible = true;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ActionExitEditModeExecute(TObject *Sender)
{
   AnsiString FileName = OpenDialogLst->FileName;
   if(FileName.IsEmpty())
      FileName = OpenDialogG2D->FileName;

   ProgressBar1->Visible = true;
   ListBoxFrames->Items->Clear();
   ListBoxTextures->Items->Clear();

   // - pack it!
   Form1->StatusBar1->Panels->Items[0]->Text = "Pack frames";
   FFrame* F = Convert.Triangulator.Frames[CurrentFrameIdx];
   F->MakeMeshes();
   Convert.Process();

   ProgressBar1->Position = 0;
   ProgressBar1->Visible = false;
   Refresh();

   // - save it!
   AnsiString OutFileName = ChangeFileExt(FileName, ".gp2");
   Convert.SaveToFile(OutFileName.c_str(), &Convert.Triangulator);

   for(int i = 0; i < Convert.Triangulator.Frames.sizeUsed; i++)
      ListBoxFrames->Items->Add(AnsiString("Frame") + IntToStr(i));

   for(int i = 0; i < Convert.Triangulator.Textures.sizeUsed-1; i++)
      ListBoxTextures->Items->Add(AnsiString("Texture") + IntToStr(i));

   Form1->StatusBar1->Panels->Items[0]->Text = "Success";
   DrawScreen();
   Refresh();

   ListBoxFrames->Enabled = true;
   ToolButtonOkEditMode->Visible = false;
   ToolButtonCancelEditMode->Visible = false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ActionSelectFrameExecute(TObject *Sender)
{
   CurrentFrameIdx = ListBoxFrames->ItemIndex;
   DrawScreen();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ActionClickPaintBoxExecute(TObject *Sender)
{
   if(ToolButtonOkEditMode->Visible &&
      CurrentFrameIdx >= 0 &&
      CurrentFrameIdx < Convert.Triangulator.Frames.sizeUsed)
   {
      FFrame* F = Convert.Triangulator.Frames[CurrentFrameIdx];
      int x = CurrentX + F->Bitmap->OriginX;
      int y = CurrentY + F->Bitmap->OriginY;
      bool isFound = false;
      for(int i = 0; i < F->UserSites.sizeUsed; i++)
      {
         if(abs(F->UserSites[i].x - x) <= 1 && abs(F->UserSites[i].y - y) <= 1)
         {
            F->UserSites.Remove(i);
            isFound = true;
            break;
         }
      }
      if(!isFound)
      {
         F->UserSites.Add();
         F->UserSites.last->x = x;
         F->UserSites.last->y = y;
      }
      if(!F->isZPresent && OpenDialogZBuffer->Execute())
      {
         FBitmap* bmpZ = new FBitmap;
         bmpZ->LoadFromFile(OpenDialogZBuffer->FileName.c_str());
         F->Triangulate(bmpZ);
         delete bmpZ;
      }
      else
         F->Triangulate(NULL);
      DrawScreen();
   }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ActionExportExecute(TObject *Sender)
{
   Convert.Export(&Convert.Triangulator);
   for(int i = 0; i < Convert.Triangulator.Textures.sizeUsed; i++)
      Convert.Triangulator.Textures[i]->SaveToFile(Convert.GetTextureName(i, ""));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormKeyDown(TObject *Sender, WORD &Key,
      TShiftState Shift)
{
   if(Key == 'W' || Key == 'w')
   {
      ToolButtonTri->Down ^= 1;
      DrawScreen();
   }
   if(Key == 'Z' || Key == 'z')
   {
      ToolButtonZ->Down ^= 1;
      DrawScreen();
   }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ToolButtonZClick(TObject *Sender)
{
   DrawScreen();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CorrectLevels1Click(TObject *Sender)
{
   CorrectLevels1->Checked ^= 1;
   ImportFramesProc(CurrentFileName);
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Dumpforg171Click(TObject *Sender)
{
   Form1->StatusBar1->Panels->Items[0]->Text = "Dumping for .g17";

   // Turn off the mesh & no texture view
   ToolButtonTri->Down = 0;
   ToolButtonZ->Down = 0;

   // Create a folder for storing the output data
   AnsiString FolderName = ChangeFileExt(CurrentFileName, "");
   mkdir(FolderName.c_str());

   // For each frame
   for (int CurrentFrameIdx = 0; CurrentFrameIdx < Convert.Triangulator.Frames.sizeUsed; CurrentFrameIdx++)
   {
      FFrame* F = Convert.Triangulator.Frames[CurrentFrameIdx];
      FFrame* FS = NULL;
      if(Convert.Triangulator.Frames.sizeUsed == Convert.TriangulatorS.Frames.sizeUsed)
         FS = Convert.TriangulatorS.Frames[CurrentFrameIdx];

      
      // Create a new screen
      if(myScreen)
      {
         delete myScreen;
         myScreen = NULL;
      }
      myScreen = new FScreen(F->Width - F->Bitmap->OriginX*2, F->Height - F->Bitmap->OriginY*2, Form1->Canvas->Handle);

      // Clear the screen
      myScreen->Clear(RGB(128, 128, 128));

      // Draw the frame
      DrawFrame(Form1->Canvas->Handle, F, FS, NULL, Form1->ToolButtonTri->Down, Form1->ToolButtonZ->Down);

      // Save the frame inside the folder
      AnsiString FileName = FolderName + "\\Frame_" + IntToStr(CurrentFrameIdx) + ".bmp";
      myScreen->ScrBitmap->SaveToFile(FileName);
   }

   // Dump all the textures
   for (int CurrentTextureIdx = 0; CurrentTextureIdx < Convert.Triangulator.Textures.sizeUsed-1; CurrentTextureIdx++) {
      FBitmap* T = Convert.Triangulator.Textures[CurrentTextureIdx];
      AnsiString FileName = FolderName + "\\Texture_" + IntToStr(CurrentTextureIdx) + ".tga";
      T->SaveToFile(FileName.c_str());
   }

   Form1->StatusBar1->Panels->Items[0]->Text = "Dump successful!";
}
//---------------------------------------------------------------------------

