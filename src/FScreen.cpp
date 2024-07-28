//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FScreen.h"
#include "myVector.h"
//---------------------------------------------------------------------------
HWND FScreen::hDC;
//---------------------------------------------------------------------------
FScreen::FScreen(int width, int height, HDC _hDC = NULL)
{
   ScrBitmap = NULL;
   Scr = NULL;

   Width = width;
   Height = height;
   if(_hDC)
      hDC = _hDC;

   tagBITMAPINFO BInfo;

   // Создание DIB
   BInfo.bmiHeader.biSize = sizeof(tagBITMAPINFOHEADER);
   BInfo.bmiHeader.biWidth = Width;
   BInfo.bmiHeader.biHeight = -(int)Height;
   BInfo.bmiHeader.biPlanes = 1;
   BInfo.bmiHeader.biBitCount = 32;
   BInfo.bmiHeader.biCompression = BI_RGB;

   ScrBitmap = new Graphics::TBitmap();
  	ScrBitmap->Handle = CreateDIBSection(
      hDC,
      &BInfo,
      DIB_RGB_COLORS,
      &Scr,
      0,
      0);
   if(ScrBitmap->Handle == NULL)
   {
      char str[1024];
      sprintf(str, "Cannot create DIB section: %ux%u", Width, Height);
      MessageBox(NULL, str, "ERROR!", MB_OK);
      Delete();
      return;
   }
   memset(Scr, 0xf, Width*Height*4);
}
//-------------------------------------------------------------------------
FScreen::~FScreen()
{
   Delete();
}
//-------------------------------------------------------------------------
void FScreen::Delete(void)
{
   if(ScrBitmap)
   {
      ScrBitmap->FreeImage();
      delete ScrBitmap;
   	ScrBitmap = NULL;
   }
}
//---------------------------------------------------------------------------
void FScreen::PutLine(int xn, int yn, int xk, int yk, unsigned col)
{
   int pxn = xn, pyn = yn;
   int dx, dy, s, sx, sy, kl, swap, incr1, incr2;

   // - вычисление приращений и шагов
   sx = 0;
   dx = xk - xn;
   if(dx < 0)
   {
      dx = -dx;
      sx--;
   }
   else
   if(dx > 0) sx++;
   sy = 0;
   dy = yk - yn;
   if(dy < 0)
   {
      dy = -dy;
      sy--;
   }
   else
   if(dy > 0) sy++;

   // - учет наклона
   swap = 0;
   kl = dx;
   s = dy;
   if(kl < s)
   {
      dx = s;
      dy = kl;
      kl = s;
      swap++;
   }
   incr1 = dy << 1;
   s = incr1 - dx;
   incr2 = dx << 1;

   PutPixel(xn, yn, col); // - первая точка вектора
   while(--kl >= 0)
   {
      if(s >= 0)
      {
         if(swap) xn += sx;
         else yn += sy;
         s -= incr2;
      }
      if(swap) yn += sy;
      else xn += sx;
      s += incr1;
      PutPixel(xn, yn, col); // - текущая точка вектора
   }
}
//-------------------------------------------------------------------------
void FScreen::AddLine(int xn, int yn, int xk, int yk, unsigned col)
{
   int pxn = xn, pyn = yn;
   int dx, dy, s, sx, sy, kl, swap, incr1, incr2;

   // - вычисление приращений и шагов
   sx = 0;
   dx = xk - xn;
   if(dx < 0)
   {
      dx = -dx;
      sx--;
   }
   else
   if(dx > 0) sx++;
   sy = 0;
   dy = yk - yn;
   if(dy < 0)
   {
      dy = -dy;
      sy--;
   }
   else
   if(dy > 0) sy++;

   // - учет наклона
   swap = 0;
   kl = dx;
   s = dy;
   if(kl < s)
   {
      dx = s;
      dy = kl;
      kl = s;
      swap++;
   }
   incr1 = dy << 1;
   s = incr1 - dx;
   incr2 = dx << 1;

   AddPixel(xn, yn, col); // - первая точка вектора
   while(--kl >= 0)
   {
      if(s >= 0)
      {
         if(swap) xn += sx;
         else yn += sy;
         s -= incr2;
      }
      if(swap) yn += sy;
      else xn += sx;
      s += incr1;
      AddPixel(xn, yn, col); // - текущая точка вектора
   }
}
//-------------------------------------------------------------------------
void FScreen::PutTriangle(double x0, double y0,
                          double x1, double y1,
                          double x2, double y2,
                          unsigned col)
{
   #define SWAP(a, b) { double t = a; a = b; b = t; }

   // - сортировка по y
   if(y0 > y1)
   {
      SWAP(x0, x1);
      SWAP(y0, y1);
   }
   if(y0 > y2)
   {
      SWAP(x0, x2);
      SWAP(y0, y2);
   }
   if(y1 > y2)
   {
      SWAP(x1, x2);
      SWAP(y1, y2);
   }

   if(fabs(y2-y0) < 0.000001) return;

   for(double y = y0; y < y2; y++)
   {
      double _x1 = 0, _x2 = 0;

      _x1 = x0 + (y - y0)*(x2 - x0)/(y2 - y0);
      if(y < y1)
         _x2 = x0 + (y - y0)*(x1 - x0)/(y1 - y0);
      else
      {
         if(y2 == y1)
            _x2 = x1;
         else
            _x2 = x1 + (y - y1)*(x2 - x1)/(y2 - y1);
      }
      if(_x1 > _x2) SWAP(_x1, _x2);
      for(double x = _x1; x < _x2; x++)
      {
         int i = floor(x), j = floor(y);
         if(i >= 0 && i < Width && j >= 0 && j < Height)
            PutPixel(i, j, col);
      }
   }
   #undef SWAP
}
//---------------------------------------------------------------------------
void FScreen::PutTriangle(double x0, double y0,
                          double x1, double y1,
                          double x2, double y2,
                          double DeltaX, double DeltaY,
                          int Scale, FBitmap* Texture)
{
   #define SWAP(a, b) { double t = a; a = b; b = t; }

   // - сортировка по y
   if(y0 > y1)
   {
      SWAP(x0, x1);
      SWAP(y0, y1);
   }
   if(y0 > y2)
   {
      SWAP(x0, x2);
      SWAP(y0, y2);
   }
   if(y1 > y2)
   {
      SWAP(x1, x2);
      SWAP(y1, y2);
   }

   if(fabs(y2-y0) < 0.000001) return;

   for(double y = y0; y < y2; y++)
   {
      double _x1 = 0, _x2 = 0;

      _x1 = x0 + (y - y0)*(x2 - x0)/(y2 - y0);
      if(y < y1)
         _x2 = x0 + (y - y0)*(x1 - x0)/(y1 - y0);
      else
      {
         if(y2 == y1)
            _x2 = x1;
         else
            _x2 = x1 + (y - y1)*(x2 - x1)/(y2 - y1);
      }
      if(_x1 > _x2) SWAP(_x1, _x2);
      for(double x = _x1; x < _x2; x++)
      {
         int tx = floor(x) + DeltaX;
         int ty = floor(y) + DeltaY;

         while(tx < 0) tx += Texture->Width;
         while(tx >= Texture->Width) tx -= Texture->Width;
         while(ty < 0) ty += Texture->Height;
         while(ty >= Texture->Height) ty -= Texture->Height;

         RGBAX& col = Texture->GetPixel(tx, ty);

         int r = col.r;
         int g = col.g;
         int b = col.b;
         int a = col.a;
         int n = col.n;

         if(n)
         {
            r = 0;
            g = 0xE0*a/0xE0;
            b = 0;
            a = 255;
         }
         if(a != 255)
         {
            int foneB = 128;
            int foneG = 128;
            int foneR = 128;

            r = foneR-(a*(foneR-r)>>8);
            g = foneG-(a*(foneG-g)>>8);
            b = foneB-(a*(foneB-b)>>8);
            if(r > 255) r = 255;
            if(g > 255) g = 255;
            if(b > 255) b = 255;
         }
         PutPixel(floor(x), floor(y), RGB(r, g, b));
      }
   }
   #undef SWAP
}
//---------------------------------------------------------------------------
void FScreen::AddTriangle(double x0, double y0,
                          double x1, double y1,
                          double x2, double y2,
                          unsigned col)
{
   #define SWAP(a, b) { double t = a; a = b; b = t; }

   // - сортировка по y
   if(y0 > y1)
   {
      SWAP(x0, x1);
      SWAP(y0, y1);
   }
   if(y0 > y2)
   {
      SWAP(x0, x2);
      SWAP(y0, y2);
   }
   if(y1 > y2)
   {
      SWAP(x1, x2);
      SWAP(y1, y2);
   }

   if(fabs(y2-y0) < 0.000001) return;

   for(double y = y0; y < y2; y++)
   {
      double _x1 = 0, _x2 = 0;

      _x1 = x0 + (y - y0)*(x2 - x0)/(y2 - y0);
      if(y < y1)
         _x2 = x0 + (y - y0)*(x1 - x0)/(y1 - y0);
      else
      {
         if(y2 == y1)
            _x2 = x1;
         else
            _x2 = x1 + (y - y1)*(x2 - x1)/(y2 - y1);
      }
      if(_x1 > _x2) SWAP(_x1, _x2);
      for(double x = _x1; x < _x2; x++)
      {
         int i = floor(x), j = floor(y);
         if(i >= 0 && i < Width && j >= 0 && j < Height)
            AddPixel(i, j, col);
      }
   }
   #undef SWAP
}
//---------------------------------------------------------------------------

