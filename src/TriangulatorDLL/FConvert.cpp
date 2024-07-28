//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include <dir.h>
#include <process.h>
#include "FConvert.h"
//---------------------------------------------------------------------------
bool FConvert::AddFrame(const FBitmap* bmp, const FBitmap* bmpZ)
{
   Triangulator.isCorrectLevels = isCorrectLevels;
   TriangulatorS.isCorrectLevels = isCorrectLevels;

   Triangulator.AddFrame(bmp, bmpZ, 1);

   FBitmap* bmpS = SplitShadow(bmp);
   if(bmpS)
   {
      TriangulatorS.AddFrame(bmpS, NULL, 2);
      delete bmpS;
   }
   return true;
}
//---------------------------------------------------------------------------
bool FConvert::AddFrame(const char* FileName, const char* FileNameZ)
{
   static char drive[MAXDRIVE];
   static char dir[MAXDIR];
   static char file[MAXFILE];
   static char ext[MAXEXT];
   static char fname[MAXPATH];

   FBitmap* bmp = NULL;
   FBitmap* bmpZ = NULL;

   if(FileName == NULL) return false;

   // - подготовим битмап
   fnsplit(FileName, drive, dir, file, ext);
   if(stricmp(ext, ".a24") == 0)
   {
      FBitmap* _bmp = ConvertA24(FileName);
      if(!_bmp) return false;
      Extract(_bmp);
      my_assert(Scale > 0);
      bmp = Zoom(_bmp, Scale);
      delete _bmp;
   }
   else
   {
      bmp = new FBitmap;
      if(!bmp->LoadFromFile(FileName))
      {
         delete bmp;
         return false;
      }
   }

   // - пробуем загрузить Z-канал
   if(FileNameZ == NULL)
   {
      strcat(file, "Z");
      fnmerge(fname, drive, dir, file, ext);
      FileNameZ = fname;
   }

   bmpZ = new FBitmap;
   if(!bmpZ->LoadFromFile(FileNameZ))
   {
      delete bmpZ;
      bmpZ = NULL;
   }

   // - добавить кадр
   AddFrame(bmp, bmpZ);

   // - удалить битмапы
   delete bmp;
   return true;
}
//---------------------------------------------------------------------------
bool FConvert::LoadFromList(const char* LstFileName, const char* Parameters)
{
   // - settings
   static char str1[32];
   static char str2[32];
   static char pal_str[MAXPATH+2];

   // - значения записи
   int directions = 0, FPSegm = 0, alpha_delta_dist = 0, All_Alpha_Color = 0;
   int sharp = 20, contrast = 8, bright = 10, color_correct = 25;
   int isFileCompress = 0, colors_number = 32;
   int isOldG16Format = 1, isOldG16FormatCompress = 1;
   int numX = 0, numY = 0, dX = 0, dY = 0, Pitch = 0;
   int shadowValue = 256, G15_G16_G18_Font = 1;
   float quality = 0, opacity = 0, scale = 0;

   // - сканирование записи
   sscanf(Parameters, "%s %s %u %u %f %f %f %u %u %u %u %u %u %u %u %s %u %u %u %u %u %u %u %u %u",
                str1, str2,
                &directions, &FPSegm,
                &quality, &opacity, &scale,
                &All_Alpha_Color, &alpha_delta_dist,
                &sharp, &contrast, &bright,
                &color_correct, &isFileCompress, &colors_number,
                pal_str,
                &isOldG16Format, &isOldG16FormatCompress,
                &numX, &numY, &dX, &dY, &Pitch, &shadowValue,
                &G15_G16_G18_Font);

   // - корректировка имени файла палитры
   int pal_str_start = 0;
   int pal_str_end = strlen(pal_str);
   while(pal_str[pal_str_start] == '"')
   {
      pal_str[pal_str_start] = 0;
      pal_str_start++;
   }
   while(pal_str_end && pal_str[pal_str_end-1] == '"')
   {
      pal_str_end--;
      pal_str[pal_str_end] = 0;
   }

   // - сохраним нужные параметры
   Directions       = directions;
   Scale            = scale;
//   ColorsNumber     = colors_number;
   Sharp            = sharp;
   Contrast         = contrast;
   Bright           = bright;
   ColorCorrect     = color_correct;
   ShadowValue      = shadowValue;
   strcpy(PaletteFileName, pal_str + pal_str_start);

   // - загрузка lst-файла
   FILE* in = fopen(LstFileName, "rt");
   if(!in)
   {
      _ERROR("Cannot open file: %s", LstFileName);
      return false;
   }

   static char drive[MAXDRIVE];
   static char dir[MAXDIR];
   static char file[MAXFILE];
   static char ext[MAXEXT];
   static char SourcePath[MAXPATH];

   fnsplit(LstFileName, drive, dir, file, ext);
   fnmerge(SourcePath, drive, dir, "", "");

   // - грузим имена исходных файлов в список
   storeArray<char*> _Lst;
   while(1)
   {
      static char str[MAXPATH+1];
      static char SourceFile[MAXPATH];
      if(!fgets(str, MAXPATH, in)) break;

      char* _str = str + strlen(str);
      while(_str > str && (*_str == 0 ||
                           *_str == '\n' ||
                           *_str == '\r' ||
                           *_str == ' ' ||
                           *_str == '"' ||
                           *_str == '*')) _str[0] = 0, _str--;

      _str = str;
      while(*_str == '*' || *_str == '"' || *_str == ' ' || *_str == '\n') _str++;

      fnsplit(_str, drive, dir, file, ext);
      strcpy(SourceFile, SourcePath);
      strcat(SourceFile, file);
      if(ext[0] == 0)
         strcat(SourceFile, ".a24");
      else
         strcat(SourceFile, ext);

      *_Lst.Add() = strdup(SourceFile);
   }

   // - переупорядочиваем список по направлениям
	int pos = 0;
   storeArray<char*> Lst;
	if(_Lst.sizeUsed < Directions) Directions = 1;
	while(Lst.sizeUsed < _Lst.sizeUsed)
	{
      *Lst.Add() = _Lst[pos];
      _Lst[pos] = NULL;
     	pos += Directions;
	   if(pos >= _Lst.sizeUsed) pos -= _Lst.sizeUsed-1;
	}

   if(ProgressMax) ProgressMax(Lst.sizeUsed);

   // - загрузка файлов
   for(int i = 0; i < Lst.sizeUsed; i++)
   {
      AddFrame(Lst[i], NULL);
      free(Lst[i]);
      Lst[i] = NULL;
      if(ProgressCur) ProgressCur(i);
   }
   fclose(in);
   return true;
}
//---------------------------------------------------------------------------
bool FConvert::AddProjectItem(const char* FileName, const char* Parameters)
{
   static char drive[MAXDRIVE];
   static char dir[MAXDIR];
   static char file[MAXFILE];
   static char ext[MAXEXT];

   fnsplit(FileName, drive, dir, file, ext);

   if(stricmp(ext, ".tga") == 0)
      return AddFrame(FileName);

   if(stricmp(ext, ".lst") == 0)
      return LoadFromList(FileName, Parameters);

   if(stricmp(ext, ".g16") == 0)
      return LoadFromG16(FileName, Parameters);

   if(stricmp(ext, ".g17") == 0)
      return LoadFromG17(FileName, Parameters);

   return false;
}
//---------------------------------------------------------------------------
bool FConvert::Process(void)
{
   if(ProgressMax) ProgressMax(Triangulator.Frames.sizeUsed);
   Triangulator.PackFrames(ProgressCur);
   //TriangulatorS.PackFrames(ProgressCur);
   return true;
}
//---------------------------------------------------------------------------

