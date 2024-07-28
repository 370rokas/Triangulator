#ifndef FConvertH
#define FConvertH

#include "FTriangulator.h"

class FConvert {
   int Scale, ColorsNumber;
   int Sharp, Contrast, Bright, ColorCorrect, ShadowValue;
   char PaletteFileName[MAX_PATH];
   unsigned char PalRGB[1024];

   void Extract(FBitmap* bmp);
   FBitmap* Zoom(FBitmap* bmp, int Scale);
   FBitmap* SplitShadow(const FBitmap* bmp);
   FBitmap* ConvertA24(const char* FileName);
   int FindPalRGBElement(int _R, int _G, int _B);

   bool LoadFromList(const char* FileName, const char* Parameters);
   bool LoadFromG16(const char* FileName, const char* Parameters);
   bool LoadFromG17(const char* FileName, const char* Parameters);

   void(*ProgressMax)(int);
   void(*ProgressCur)(int);
public:
   int Directions;
   bool isCorrectLevels;
   FTriangulator Triangulator;
   FTriangulator TriangulatorS;

   bool AddProjectItem(const char* LstFileName, const char* Parameters);
   bool AddFrame(const char* FileName, const char* FileNameZ = NULL);
   bool AddFrame(const FBitmap* bmp, const FBitmap* bmpZ = NULL);

   char* GetTextureName(int t, const char* Suffix);
   bool Export(FTriangulator* tr);
   bool SaveToFile(const char* FileName, FTriangulator* tr);
   bool LoadFromFile(const char* FileName, FTriangulator* tr);

   bool Reset(void(*_ProgressMax)(int) = NULL, void(*_ProgressCur)(int) = NULL)
   {
      Directions = 1;
      isCorrectLevels = false;
      Scale = 4;
      ColorsNumber = -1;
      Sharp = Contrast = Bright = ColorCorrect = 0;
      ShadowValue = 512;

      ProgressMax = _ProgressMax;
      ProgressCur = _ProgressCur;

      Triangulator.Initialize("");
      TriangulatorS.Initialize("S");
      return true;
   }

   bool Process(void);

   FConvert(void)
   {
      Reset();
   }
};

void _ERROR(const char* format, ... );

#endif
