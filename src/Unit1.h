//---------------------------------------------------------------------------
#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include <ComCtrls.hpp>
#include <CheckLst.hpp>
#include <ToolWin.hpp>
#include <ImgList.hpp>
#include "FStream.hpp"
#include <ActnList.hpp>
#include <Buttons.hpp>
//---------------------------------------------------------------------------
class TForm1 : public TForm
{
__published:	// IDE-managed Components
   TMainMenu *MainMenu1;
   TMenuItem *File1;
   TMenuItem *ImportFrames;
   TOpenDialog *OpenDialogLst;
   TProgressBar *ProgressBar1;
   TSplitter *Splitter1;
   TPageControl *PageControl1;
   TTabSheet *TabSheet1;
   TTabSheet *TabSheet2;
   TListBox *ListBoxFrames;
   TListBox *ListBoxTextures;
   TStatusBar *StatusBar1;
   TBevel *Bevel1;
   TPaintBox *MainPaintBox;
   TToolBar *ToolBar1;
   TToolButton *ToolButtonTri;
   TImageList *ImageList1;
   TMenuItem *Open;
   TOpenDialog *OpenDialogG2D;
   TToolButton *ToolButton1;
   TActionList *ActionList1;
   TAction *ActionOpen;
   TAction *ActionImportFrames;
   TAction *ActionShowMesh;
   TAction *ActionEnterEditMode;
   TAction *ActionExitEditMode;
   TAction *ActionSelectFrame;
   TAction *ActionClickPaintBox;
   TAction *ActionExport;
   TMenuItem *Export1;
   TSpeedButton *ToolButtonCancelEditMode;
   TSpeedButton *ToolButtonOkEditMode;
   TOpenDialog *OpenDialogZBuffer;
   TToolButton *ToolButtonZ;
   TMenuItem *Tools1;
   TMenuItem *CorrectLevels1;
        TMenuItem *Dumptga1;
   void __fastcall MainPaintBoxPaint(TObject *Sender);
   void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
   void __fastcall PageControl1Change(TObject *Sender);
   void __fastcall ListBoxTexturesClick(TObject *Sender);
   void __fastcall ListBoxTexturesMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
   void __fastcall ListBoxFramesMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
   void __fastcall Splitter1Paint(TObject *Sender);
   void __fastcall MainPaintBoxMouseMove(TObject *Sender,
          TShiftState Shift, int X, int Y);
   void __fastcall MyIdleHandler(TObject *Sender, bool &Done);
   void __fastcall FormCreate(TObject *Sender);
   void __fastcall ActionOpenExecute(TObject *Sender);
   void __fastcall ActionImportFramesExecute(TObject *Sender);
   void __fastcall ActionShowMeshExecute(TObject *Sender);
   void __fastcall ActionEnterEditModeExecute(TObject *Sender);
   void __fastcall ActionExitEditModeExecute(TObject *Sender);
   void __fastcall ActionSelectFrameExecute(TObject *Sender);
   void __fastcall ActionClickPaintBoxExecute(TObject *Sender);
   void __fastcall ActionExportExecute(TObject *Sender);
   void __fastcall FormKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
   void __fastcall ToolButtonZClick(TObject *Sender);
   void __fastcall CorrectLevels1Click(TObject *Sender);
        void __fastcall Dumptga1Click(TObject *Sender);
private:	// User declarations
   void __fastcall ImportFramesProc(AnsiString str);
public:		// User declarations
   __fastcall TForm1(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern AnsiString CurrentFileName;
extern PACKAGE TForm1 *Form1;
//---------------------------------------------------------------------------
#endif
