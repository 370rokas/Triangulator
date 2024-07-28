//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FConvert.h"
//---------------------------------------------------------------------------
FBitmap* FConvert::SplitShadow(const FBitmap* bmp)
{
   return NULL;
   int W = bmp->Width;
   int H = bmp->Height;

   FBitmap* bmpS = new FBitmap(W, H);
   bmpS->Clear();

   // - найти точки тени
   for(int j = 1; j < H-1; j++)
   {
      for(int i = 1; i < W-1; i++)
      {
         int num = 0;
         for(int dy = -1; dy <= 1; dy++)
         {
            for(int dx = -1; dx <= 1; dx++)
            {
               RGBAX& col = bmp->GetPixel(i+dx, j+dy);
               if(col.r != 0 || col.g != 0 || col.b != 0 || col.a >= 0xE0 || col.n) num++;
            }
         }
         if(num == 0)
         {
            RGBAX& col = bmpS->GetPixel(i, j);
            col.x = 1;
         }
      }
   }

   // - окружить найденные точки тени под цветными пикселами
   for(int k = 0; k < 2; k++)
   {
      for(int j = 0; j < H; j++)
      {
         for(int i = 0; i < W; i++)
         {
            int x = bmpS->GetPixel(i, j).x;
            if(x)
            {
               RGBAX& col = bmp->GetPixel(i, j);
               col.x = x;
            }
         }
      }
      for(int j = 1; j < H-1; j++)
      {
         for(int i = 1; i < W-1; i++)
         {
            RGBAX col = bmp->GetPixel(i, j);
            if(col.x && col.a)
            {
               for(int dy = -1; dy <= 1; dy++)
               {
                  for(int dx = -1; dx <= 1; dx++)
                  {
                     if(col.a)
                     {
                        RGBAX& colS = bmpS->GetPixel(i+dx, j+dy);
                        if(colS.x == 0)
                           colS.x = k+2;
                     }
                  }
               }
            }
         }
      }
   }

   // - перенести тень
   for(int j = 0; j < H; j++)
   {
      for(int i = 0; i < W; i++)
      {
         RGBAX colS = bmpS->GetPixel(i, j);
         if(colS.x)
         {
            RGBAX col = bmp->GetPixel(i, j);
            bmpS->PutPixel(i, j, col);
            if(colS.x && col.a && col.r == 0 && col.g == 0 && col.b == 0)
               bmp->PutPixel(i, j, RGBAX());
         }
      }
   }

   FBitmap* _bmpS = new FBitmap((W+1)/2, (H+1)/2);
   _bmpS->Clear();

   // - уменьшение тени в два раза
   for(int j = 0; j < H-1; j+=2)
   {
      for(int i = 0; i < W-1; i+=2)
      {
         int r = 0, g = 0, b = 0, a = 0;
         for(int dy = 0; dy < 2; dy++)
         {
            for(int dx = 0; dx < 2; dx++)
            {
               RGBAX& col = bmpS->GetPixel(i+dx, j+dy);
               r += col.r;
               g += col.g;
               b += col.b;
               a += col.a;
            }
         }
         r >>= 2;
         g >>= 2;
         b >>= 2;
         a >>= 2;
         _bmpS->PutPixel(i/2, j/2, RGBAX(r, g, b, a, 0));
      }
   }
   delete bmpS;

   _bmpS->OriginX = bmp->OriginX;
   _bmpS->OriginY = bmp->OriginY;

   return _bmpS;
}
//---------------------------------------------------------------------------
void FConvert::Extract(FBitmap* bmp)
{
   bool isExtractKeyColor = true;
   bool isLeftTopPixelTransparent = true;
   int ShadowValue = 256*2;

   int Width = bmp->Width;
   int Height = bmp->Height;

   RGBAX KeyColor(0, 255, 0, 0, 0);
   RGBAX NationalColor(255, 0, 255, 0, 0);

   if(isExtractKeyColor && Width*Height && isLeftTopPixelTransparent)
      KeyColor = bmp->GetPixel(0, 0);

   //---------------------------------------------------------------------------
   for(int j = 0; j < Height; j++)
   {
      for(int i = 0; i < Width; i++)
      {
         //-------------- Extract background color --------------------------
         unsigned r = 0, g = 0, b = 0, a = 0;

         // - ключевой цвет
         double Kr = KeyColor.r;
         double Kg = KeyColor.g;
         double Kb = KeyColor.b;
         double Kd = sqrt(Kr*Kr + Kg*Kg + Kb*Kb);

         // - национальный цвет
         double Nr = NationalColor.r;
         double Ng = NationalColor.g;
         double Nb = NationalColor.b;
         double Nd = sqrt(Nr*Nr + Ng*Ng + Nb*Nb);

         // - цвет пиксела изображения
         RGBAX& col = bmp->GetPixel(i, j);
         double Ir = col.r;
         double Ig = col.g;
         double Ib = col.b;
         double Id = sqrt(Ir*Ir + Ig*Ig + Ib*Ib);

         bool isColK = false;
         bool isColN = false;
         bool isNColor = false;

         if(Id >= 0.000001 && Kd*Id >= 0.000001 && Nd*Id >= 0.000001)
         {
            if(isExtractKeyColor)
            {
               // - косинус угла между векторами K&I
               double aKI = fabs((Kr*Ir + Kg*Ig + Kb*Ib)/(Kd*Id));
               isColK = aKI >= 0.998;
            }
            // - косинус угла между векторами N&I
            double aNI = fabs((Nr*Ir + Ng*Ig + Nb*Ib)/(Nd*Id));
            isColN = aNI >= 0.998;
         }

         col.n = 0;
         if(isColK)
         {
            isNColor = false;
            a = (unsigned)((Kd-Id)*(double)ShadowValue/256.0);
         }
         else
         if(isColN)
         {
            isNColor = true;
            double aa = 1.0 - (Nd-Id)/sqrt(255*255*3);
            a = (unsigned)(245.0*aa);
            if(a > 255) a = 255;
         }
         else
         {
            r = (unsigned)Ir;
            g = (unsigned)Ig;
            b = (unsigned)Ib;
            a = 255;
         }
         if(a == 0) r = 0, g = 0, b = 0;
         if(a > 255) a = 255;

         if(isNColor)
         {
            int d = (r+g+b)/3 + 1;
            a /= d;
         }
         else
         if(!isExtractKeyColor)
            a = col.a;
         //------------------------------------------------------------------
         col.r = r;
         col.g = g;
         col.b = b;
         col.a = a;
         col.n = (int)isNColor;
      }
   }
}
//---------------------------------------------------------------------------
FBitmap* FConvert::Zoom(FBitmap* bmp, int delta)
{
   int W = bmp->Width;
   int H = bmp->Height;

   int x1 = 0, y1 = 0, x2 = W, y2 = H;
   //-------------------------- ищем bounding box ---------------------------
   {
      bool done = false;
      // - проверка вертикальных линий слева направо
      while(x1 < W)
      {
         done = false;
         for(int j = 0; j < H; j++)
         {
            if(bmp->Data[j*W + x1].a)
            {
               done = true;
               break;
            }
         }
         if(done) break;
         x1++;
      }
      if(!done)
      {
         x1 = 0, y1 = 0, x2 = 0, y2 = 0;
         goto a10;
      }
      // - проверка вертикальных линий справа налево
      while(x2 > 0)
      {
         done = false;
         for(int j = 0; j < H; j++)
         {
            if(bmp->Data[j*W + x2-1].a)
            {
               done = true;
               break;
            }
         }
         if(done) break;
         x2--;
      }
      // - проверка горизонтальных линий сверху вниз
      while(y1 < W)
      {
         done = false;
         for(int i = 0; i < W; i++)
         {
            if(bmp->Data[y1*W + i].a)
            {
               done = true;
               break;
            }
         }
         if(done) break;
         y1++;
      }
      // - проверка горизонтальных линий снизу вверх
      while(y2 > 0)
      {
         done = false;
         for(int i = 0; i < W; i++)
         {
            if(bmp->Data[(y2-1)*W + i].a)
            {
               done = true;
               break;
            }
         }
         if(done) break;
         y2--;
      }
   }
a10:
   //------------------------------------------------------------------------
   x1 -= x1%delta;
   y1 -= y1%delta;

   x2 = ((x2+delta-1)/delta)*delta;
   y2 = ((y2+delta-1)/delta)*delta;

   assert(x1 <= x2 && y1 <= y2);
   //------------------------------------------------------------------------

   int _width = (x2 - x1)/delta;
   int _height = (y2 - y1)/delta;

   FBitmap* _bmp = new FBitmap(_width, _height);

   for(int j = 0; j < _height; j++)
   {
      for(int i = 0; i < _width; i++)
      {
         int r = 0, g = 0, b = 0, a = 0, na = 0, s = delta*delta;
         if(!s) break;

         int NColors = 0;
         bool isShadow = false;

         for(int y = 0; y < delta; y++)
         {
            int yy = j*delta + y1 + y;
            for(int x = 0; x < delta; x++)
            {
               int xx = i*delta + x1 + x;

               int _a = 0, _r = 0, _g = 0, _b = 0, _n = 0;

               if(xx < W && yy < H)
               {
                  RGBAX& col = bmp->GetPixel(xx, yy);

                  _a = col.a;
                  _r = col.r;
                  _g = col.g;
                  _b = col.b;
                  _n = col.n;

                  if(_a < 220 && _r < 16 && _g < 16 && _b < 16)
                     isShadow = true;
               }
               if(_n)
               {
                  na += _a;   // - собираем яркость национального цвета
                  a += 255;
                  r += _r<<8;
                  g += _g<<8;
                  b += _b<<8;
                  NColors++;
               }
               else
               {
                  a += _a;
                  r += _r*_a;
                  g += _g*_a;
                  b += _b*_a;
               }
            }
         }
         if(a)
         {
            r /= a;
            g /= a;
            b /= a;
         }
         a /= s;

         if(r > 255) r = 255;
         if(g > 255) g = 255;
         if(b > 255) b = 255;
         if(a > 255) a = 255;

         if(NColors)
         {
            a = na/NColors + 0xF;
            if(a > 255) a = 255;
         }
         _bmp->PutPixel(i, j, RGBAX(r, g, b, a, 0, !!NColors));
      }
   }

   _bmp->OriginX = x1/delta;
   _bmp->OriginY = y1/delta;

   return _bmp;
}
//---------------------------------------------------------------------------
int FConvert::FindPalRGBElement(int _R, int _G, int _B)
{
   unsigned idx = 0;
   int min_d = 256*256*3;
   for(int i = ColorsNumber-1; i >= 0; i--)
   {
      int dr = _R - (int)PalRGB[i*4+0];
      int dg = _G - (int)PalRGB[i*4+1];
      int db = _B - (int)PalRGB[i*4+2];

      int dist = dr*dr + dg*dg + db*db;
      if(dist <= min_d)
      {
         min_d = dist;
         idx = i;
      }
   }
   return idx;
}
//---------------------------------------------------------------------------

