#ifndef _FFRAME_
#define _FFRAME_
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
#include "myVector.h"
#include "FMesh.h"
//---------------------------------------------------------------------------
struct FNode {
   double x, y;
   FNode* Next;
};
//---------------------------------------------------------------------------
// В каждом фрейме работа ведется в мировых координатах
//---------------------------------------------------------------------------
class FFrame {
public:
   ptrArray<FNode> Contours;
   static short MinBuf[0x10000];
   static short MaxBuf[0x10000];

   int Smooth;             // - сглаживание границ битмапа
   double Scale;           // - масштаб

   void  ReloadBitmap(const FBitmap* bmp, const FBitmap* bmpZ);
   void  CDT(FNode* Contour);
   void  CDT(void);
   int   GoAround(FNode* Contour, int x_p, int y_p, int x_w, int y_w, bool isRight);
   bool  CheckLine(int xn, int yn, int xk, int yk, FBitmap* bmp = NULL, bool isX = false);
   bool  CheckITriangle(FBitmap* bmp, FVertex* V0, FVertex* V1, FVertex* V2, int rgb, Vector* W);
   void  MinMaxLine(int xn, int yn, int xk, int yk);
   void  MinMaxClear(void);
   void  FloodFill(int x, int y);
   void  PutTriangle(double x0, double y0, double x1, double y1, double x2, double y2, int MinX, int MinY, int W, int H, char* Map);
   bool  CheckTriangle(int x0, int y0, int x1, int y1, int x2, int y2);
   bool  ProcessContour(FNode** pContour);
   void  OptimizeContour(FNode** pContour, bool isBending);
   void  OptimizeContourByTriangles(FNode** pContour, double Area);
   void  OptimizeContourByAngles(FNode** pContour, double Angle);
   void  Bending(FNode* Path, double val, int Num);
   bool  SplitMesh(int m, int Num);
   bool  SplitBigMesh(int m);
   void  MakeMeshMap(FMesh* M);
   int   CheckMeshMap(FBitmap* bmp, int x0, int y0, FMesh* M);
   void  PutMeshMap(FBitmap* bmp, int x0, int y0, FMesh* M);
   void  UnPutMeshMap(FBitmap* bmp, int x0, int y0, FMesh* M);
   int   TryToPut(FBitmap* bmp, int& max_x, int& max_y, FMesh* M);
   bool  PackMeshes(FBitmap* bmp);
   int   GetZ(int x, int y);
   bool  ConstructMesh(void);
   FMesh* RepairMesh(void);

   enum { AlphaNullValue, AlphaMapValue, AlphaContourValue, AlphaFillValue };
   enum { MaxEdgeLen = 64 };

   bool isInternalSitesLoaded;
   bool isZPresent;
   FBitmap* Bitmap;        // - исходное изображение

   bool isCorrectLevels;
   void CorrectLevels();

   void MakeMeshes(int InitialNum = 0);
   void PutTriangle(double x0, double y0,
                    double x1, double y1,
                    double x2, double y2,
                    double DeltaX, double DeltaY,
                    FBitmap* Texture);
   void PutCorrectedTriangle(FVertex* V0, FVertex* V1, FVertex* V2,
                             int DeltaX, int DeltaY, FBitmap* Texture);
   void PutTexturedTriangle(FVertex* a, FVertex* b, FVertex* c,
                            FBitmap* Texture, FBitmap* bmp);

   // - установленные пользователем точки
   storeArray<POINT> UserSites;

   ptrArray<FMesh> Meshes;

   int Width, Height;      // - размеры кадра

   void    Triangulate(const FBitmap* bmp, const FBitmap* bmpZ, double Scale = 1);
   void    Triangulate(FBitmap* bmpZ);
   void    SubdivideMesh(FMesh* M);
   FFrame* CloneSplitted(void);

   FFrame()
   {
      isInternalSitesLoaded = false;
      isZPresent = false;
      isCorrectLevels = false;
      Bitmap = NULL;
      Smooth = 4;
      Scale = 1;
      Width = Height = 0;
   }

   ~FFrame()
   {
      if(Bitmap) delete Bitmap;
   }
};
//---------------------------------------------------------------------------
extern bool g_isAnimation;
//---------------------------------------------------------------------------
#endif
