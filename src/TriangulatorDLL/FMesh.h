#ifndef _FMESH_
#define _FMESH_
//---------------------------------------------------------------------------
#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include "my_assert.h"
#include <math.h>
#include "FStream.hpp"
#include "ptrArray.hpp"
#include "storeArray.hpp"
//---------------------------------------------------------------------------
void _ERROR(const char* format, ... );
//---------------------------------------------------------------------------
#pragma pack(1)
struct FVertex {
   int x, y, z;
   union {
      struct {
         int tx, ty;
      };
      struct {
         int u, v;
      };
   };
   union {
      struct {
         double r, g, b, a;
      };
      double rgba[4];
   };
   double w1, w2, w3;
};
#pragma pack()
//---------------------------------------------------------------------------
struct FEdge {
   int v0, v1;
   int contourId;
   bool isMark;
};
//---------------------------------------------------------------------------
struct FTriangle {
   int v0, v1, v2;
   int e0, e1, e2;
   bool isMark;
};
//---------------------------------------------------------------------------
class FMesh {
public:
   storeArray<FVertex> Vertices;
   storeArray<FEdge> Edges;
   storeArray<FTriangle> Triangles;

   int TextureIdx;
   int PaintedTriangles;
   int OpaquePixels;             // - число непрозрачных пикселей
   double Area;

   int MinX, MaxX;               // - размеры bbox-а
   int MinY, MaxY;               // - размеры bbox-а
   int DeltaTextX, DeltaTextY;   // - дельта для перевода в текстурные координаты
   FStream RleStream;            // - закодированная маска с помощью RLE в лок. коорд.

   int  SearchVertex(int x, int y);
   int  SearchEdge(int v0, int v1);
   int  SearchTriangle(int _v0, int _v1, int _v2);
   int  StoreVertex(int x, int y, int z = 0);
   int  StoreEdge(int x0, int y0, int x1, int y1, int contourId = -1);
   int  StoreTriangle(int v0, int v1, int v2);
   void MakeTriangles(void);
   void MarkEdge(int e);
   void SplitEdge(int e);
   void CalcArea(void);
   void ClearMesh(void);

   FMesh()
   {
      MinX = MinY = 0x7FFFFFFF;
      MaxX = MaxY = 0;
      DeltaTextX = DeltaTextY = 0;
      PaintedTriangles = 0;
      Area = 0;
      TextureIdx = -1;
      OpaquePixels = 0;
   }

   ~FMesh()
   {
   }
};
//---------------------------------------------------------------------------
#endif
