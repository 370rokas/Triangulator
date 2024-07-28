#ifndef FSCREEN_H
#define FSCREEN_H

#include <vcl.h>
#include <stdio.h>
#include <windows.h>
#include "FBitmap.h"

class FScreen {
public:
   Graphics::TBitmap* ScrBitmap;
   void* Scr;

   int Width, Height;
   static HDC hDC;

   void Clear(unsigned col = 0)
   {
      for(int i = 0; i < Width*Height; i++) ((unsigned*)Scr)[i] = col;
   }

   void Delete(void);

   inline unsigned GetPixel(int x, int y)
   {
      if(x < 0 || x >= Width || y < 0 || y >= Height) return 0;
      return ((unsigned*)Scr)[y*Width + x];
   }

   inline void PutPixel(int x, int y, unsigned c)
   {
      if(x < 0 || x >= Width || y < 0 || y >= Height) return;
      ((unsigned*)Scr)[y*Width + x] = c;
   }

   inline void AddPixel(int x, int y, unsigned c1)
   {
      if(x < 0 || x >= Width || y < 0 || y >= Height) return;
      unsigned c2 = ((unsigned*)Scr)[y*Width + x];

      int r1 = GetRValue(c1)*3/5;
      int g1 = GetGValue(c1)*3/5;
      int b1 = GetBValue(c1)*3/5;

      int r2 = (r1 + GetRValue(c2));
      int g2 = (g1 + GetGValue(c2));
      int b2 = (b1 + GetBValue(c2));

      if(r2 > 255) r2 = 255;
      if(g2 > 255) g2 = 255;
      if(b2 > 255) b2 = 255;

      ((unsigned*)Scr)[y*Width + x] = RGB(r2, g2, b2);
   }

   void PutLine(int xn, int yn, int xk, int yk, unsigned col);
   void PutTriangle(double x0, double y0,
                    double x1, double y1,
                    double x2, double y2,
                    unsigned col);
   void AddTriangle(double x0, double y0,
                    double x1, double y1,
                    double x2, double y2,
                    unsigned col);
   void PutTriangle(double x0, double y0,
                    double x1, double y1,
                    double x2, double y2,
                    double DeltaX, double DeltaY,
                    int Scale, FBitmap* Texture);
   void AddLine(int xn, int yn, int xk, int yk, unsigned col);

   FScreen(int width, int height, HDC hDC);
   ~FScreen();
};

#endif
