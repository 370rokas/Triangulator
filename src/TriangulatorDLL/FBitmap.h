#ifndef FBITMAP_H
#define FBITMAP_H

#include <mem.h>

#pragma pack(1)
struct RGBAX {
   union {
      struct { unsigned char r, g, b, a; };
      unsigned col;
      unsigned char rgb[3];
   };
   unsigned char n, x, z, _;
   RGBAX(int _r, int _g, int _b, int _a, int _x, int _n = 0, int _z = 0)
   {
      r = _r;
      g = _g;
      b = _b;
      a = _a;
      x = _x;
      n = _n;
      z = _z;
      _ = 0;
   }
   RGBAX()
   {
      r = g = b = a = 0;
      x = z = n = _ = 0;
   }
};
#pragma pack()

class FBitmap {
//-----------------------------------------------------------------------------------
   bool LoadBMP(const char* FileName);
   bool SaveBMP(const char* FileName);
   bool LoadTGA(const char* FileName);
   bool SaveTGA(const char* FileName);
//-----------------------------------------------------------------------------------
public:
	RGBAX* Data;
	int Width, Height;
	int OriginX, OriginY;   // - вспомогательная дельта (к левому верхнему углу)

	bool LoadFromFile(const char* fname);
	bool SaveToFile(const char* FileName);

   inline RGBAX& GetPixel(int x, int y)
   {
      return Data[y*Width + x];
   }

   inline void PutPixel(int x, int y, RGBAX& c)
   {
      Data[y*Width + x] = c;
   }

   void FBitmap::Clear(void)
   {
      memset(Data, 0, sizeof(Data[0])*Width*Height);
   }

	FBitmap(void)
   {
	   Data = NULL;
   	Width = Height = 0;
      OriginX = OriginY = 0;
   }

	FBitmap(int w, int h)
   {
	   Data = new RGBAX[w*h];
   	Width = w;
      Height = h;
      OriginX = OriginY = 0;
   }

   ~FBitmap()
   {
   	if(Data) delete[] Data;
   	Data = NULL;
   }
};

#endif
