#ifndef _FTRIANGULATOR_
#define _FTRIANGULATOR_
//---------------------------------------------------------------------------
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "my_assert.h"
#include <math.h>
#include "FBitmap.h"
#include "FStream.hpp"
#include "ptrArray.hpp"
#include "storeArray.hpp"
#include "FMesh.h"
#include "FFrame.h"
#include "FPerformance.h"

extern int MAX_TEXTURE_SIZE;
extern bool g_isAnimation;
//---------------------------------------------------------------------------
class FTriangulator {
   int OpaquePixels, AllPixels;
public:

   char Suffix[_MAX_PATH+1];
   ptrArray<FBitmap> Textures;
   ptrArray<FFrame> Frames;

   bool isCorrectLevels;
   double msPackedTime;

   void Initialize(char* Suffix);
   void AddFrame(const FBitmap* bmp, const FBitmap* bmpZ, double Scale = 1.0);
   void PackFrames(void(*ProgressHookFn)(int) = NULL);
   bool PackMeshes(FFrame* F);
   FBitmap* CreateTexture(int Mul = 0);
   void SaveTexture(int t);

   FTriangulator()
   {
      isCorrectLevels = false;
      OpaquePixels = 0, AllPixels = 0;
      Suffix[0] = 0;
      msPackedTime = 0;
   }

   ~FTriangulator()
   {
      Textures.RemoveAll();
      Frames.RemoveAll();
   }
};
//---------------------------------------------------------------------------
void _ERROR(const char* format, ... );
//---------------------------------------------------------------------------
#endif
