//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FFrame.h"
//---------------------------------------------------------------------------
short FFrame::MinBuf[0x10000];
short FFrame::MaxBuf[0x10000];
//---------------------------------------------------------------------------
void FFrame::ReloadBitmap(const FBitmap* bmp, const FBitmap* bmpZ)
{
/*
   FBitmap _bmp(bmp->Width, bmp->Height);
   _bmp.Clear();

   const double cx = bmp->Width/2;//356;
   const double cy = bmp->Height/2;//277;
   for(int j = 0; j < bmp->Height; j++)
   {
      for(int i = 0; i < bmp->Width; i++)
      {
         RGBAX& colZ = bmpZ->GetPixel(i, j);

         if(!colZ.a) continue;

         // - перенос в начало координат
         double x = i - cx;
         double y = j - cy;
         double z = colZ.r-128;

         // - поворот относительно x
         const double xa = 30*M_PI/180;
         double x1 = x;
         double y1 = y*cos(xa) - z*sin(xa);
         double z1 = z*cos(xa) + y*sin(xa);

         // - поворот относительно y
         const double ya = (360.0/16.0)*M_PI/180;
         double x2 = x1*cos(ya) - z1*sin(ya);
         double y2 = y1;
         double z2 = z1*cos(ya) + x1*sin(ya);

         // - поворот относительно x
         double x3 = x2;
         double y3 = y2*cos(-xa) - z2*sin(-xa);
         double z3 = z2*cos(-xa) + y2*sin(-xa);

         int xx = x3 + cx;
         int yy = y3 + cy;

         int r = (x3-x)*2+128;
         int g = (y3-y)*2+128;
         int b = (z3-z)*2+128;

         if(r < 0) r = 0;
         if(g < 0) g = 0;
         if(b < 0) b = 0;
         if(r > 255) r = 255;
         if(g > 255) g = 255;
         if(b > 255) b = 255;

         _bmp.PutPixel(i, j, RGBAX(b, g, r, colZ.a, 0));
      }
   }
   _bmp.SaveToFile("!.tga");
*/

   storeArray<POINT> points;
   if(isCorrectLevels)
   {
      for(int t = 0; t < 16; t++)
      {
         int max_x = -1, max_y = -1, max_i = -1;
         for(int j = 0; j < bmp->Height; j++)
         {
            for(int i = 0; i < bmp->Width; i++)
            {
               RGBAX& col = bmp->GetPixel(i, j);
               if(!col.a) continue;

               int I = (int)col.r*(int)col.r + (int)col.g*(int)col.g + (int)col.b*(int)col.b;
               if(I > max_i)
               {
                  bool isFar = true;
                  for(int k = 0; k < points.sizeUsed; k++)
                  {
                     int dx = points[k].x - i;
                     int dy = points[k].y - j;
                     if(dx*dx + dy*dy < 32*32)
                     {
                        isFar = false;
                        break;
                     }
                  }
                  if(isFar)
                  {
                     max_i = I;
                     max_x = i;
                     max_y = j;
                  }
               }
            }
         }
         if(max_i > 0)
         {
            points.Add();
            points.last->x = max_x;
            points.last->y = max_y;
         }
      }

      for(int t = 0; t < 64; t++)
      {
         int min_x = -1, min_y = -1, min_i = 0x100000;
         for(int j = 0; j < bmp->Height; j++)
         {
            for(int i = 0; i < bmp->Width; i++)
            {
               RGBAX& col = bmp->GetPixel(i, j);
               if(!col.a) continue;

               int I = (int)col.r*(int)col.r + (int)col.g*(int)col.g + (int)col.b*(int)col.b;
               if(I < min_i)
               {
                  bool isFar = true;
                  for(int k = 0; k < points.sizeUsed; k++)
                  {
                     int dx = points[k].x - i;
                     int dy = points[k].y - j;
                     if(dx*dx + dy*dy < 32*32)
                     {
                        isFar = false;
                        break;
                     }
                  }
                  if(isFar)
                  {
                     min_i = I;
                     min_x = i;
                     min_y = j;
                  }
               }
            }
         }
         if(min_i != 0x100000)
         {
            points.Add();
            points.last->x = min_x;
            points.last->y = min_y;
         }
      }
   }
   if(!bmp) bmp = Bitmap;
   my_assert(bmp);

   if(Bitmap) delete Bitmap;
   Bitmap = NULL;

   // - отступ в результирующем битмапе
   int Delta = Smooth + 2;

   // - создаем битмап с пустыми границами
   int W = bmp->Width - bmp->OriginX;
   int H = bmp->Height - bmp->OriginY;
   Bitmap = new FBitmap(W + Delta*2, H + Delta*2);
   Bitmap->Clear();

   Bitmap->OriginX = bmp->OriginX - Delta;
   Bitmap->OriginY = bmp->OriginY - Delta;

   for(int j = 0; j < bmp->Height; j++)
   {
      for(int i = 0; i < bmp->Width; i++)
      {
         RGBAX& col = bmp->GetPixel(i, j);
         col.x = 0;
         if(bmpZ)
            col.z = bmpZ->GetPixel(i, j).r;
         Bitmap->PutPixel(i + Delta, j + Delta, col);
      }
   }

   int MapW = Bitmap->Width;
   int MapH = Bitmap->Height;

   unsigned char* Data1 = new unsigned char[MapW*MapH];
   unsigned char* Data2 = new unsigned char[MapW*MapH];

   // - просчет X-канала
   {
      memset(Data1, 0, MapW*MapH);
      memset(Data2, 0, MapW*MapH);

      // - просчет точного образа
      for(unsigned j = 0; j < MapH; j++)
      {
         for(unsigned i = 0; i < MapW; i++)
         {
            RGBAX col = Bitmap->GetPixel(i, j);
            Data1[j*MapW + i] = !!col.a;
            Data2[j*MapW + i] = !!col.a;
         }
      }

      // - просчет сглаженного образа
      for(int k = 0; k < Smooth/2; k++)
      {
         for(unsigned j = 0; j < MapH; j++)
         {
            for(unsigned i = 0; i < MapW; i++)
            {
               if(Data1[j*MapW + i])
               {
                  Data2[j*MapW + i] = 1;
                  Data2[j*MapW + i-1] = 1;
                  Data2[j*MapW + i+1] = 1;
                  Data2[(j-1)*MapW + i] = 1;
                  Data2[(j+1)*MapW + i] = 1;
                  if(k&1)
                  {
                     Data2[(j-1)*MapW + i-1] = 1;
                     Data2[(j-1)*MapW + i+1] = 1;
                     Data2[(j+1)*MapW + i-1] = 1;
                     Data2[(j+1)*MapW + i+1] = 1;
                  }
               }
            }
         }
         memcpy(Data1, Data2, MapW*MapH);
      }

      // - копируем в X-канал битмапа сглаженный образ
      for(int i = 0; i < MapW*MapH; i++)
         Bitmap->Data[i].x = Data1[i];
   }
   delete[] Data1;
   delete[] Data2;

   for(int i = 0; i < points.sizeUsed; i++)
   {
      int x = points[i].x + bmp->OriginX;
      int y = points[i].y + bmp->OriginY;
      UserSites.Add();
      UserSites.last->x = x;
      UserSites.last->y = y;
   }

}
//---------------------------------------------------------------------------
bool FFrame::ProcessContour(FNode** pContour)
{
   // - создание ребер
   OptimizeContour(pContour, true);
   Contours.pArray[Contours.sizeUsed-1] = Contours.last;
   Meshes.Add();
   CDT(*pContour);
   return ConstructMesh();
}
//---------------------------------------------------------------------------
int GlobalSmoothValue=4;
void FFrame::Triangulate(const FBitmap* bmp, const FBitmap* bmpZ, double scale)
{
   isZPresent = !!bmpZ;
   Scale = scale;
   //if(g_isAnimation)
      Smooth = GlobalSmoothValue;
   //else
   //   Smooth = 8;
BeginM:
   Meshes.RemoveAll();
   Contours.RemoveAll();

   // - создать битмап на основе данного
   ReloadBitmap(bmp, bmpZ);

   int MapW = Bitmap->Width;
   int MapH = Bitmap->Height;

   // - просчет контуров изображения
   for(int j = 1; j < MapH-1; j++)
   {
      for(int i = 1; i < MapW-1; i++)
      {
         bool ch0 = (Bitmap->GetPixel(i-1, j).x == AlphaMapValue);
         bool ch1 = (Bitmap->GetPixel(i, j-1).x == AlphaMapValue);
         bool ch2 = (Bitmap->GetPixel(i+1, j).x == AlphaMapValue);
         bool ch3 = (Bitmap->GetPixel(i, j+1).x == AlphaMapValue);

         if(Bitmap->GetPixel(i, j).x == 0 && (ch0 || ch1 || ch2 || ch3))
         {
            int x = i, y = j;
            if(ch0) x--;
            else
            if(ch1) y--;
            else
            if(ch2) x++;
            else
            if(ch3) y++;
            else
               my_assert(!"Error!");

            FNode* Contour = Contours.Add();
            Contour->Next = Contour;
            Contour->x = i;
            Contour->y = j;

            GoAround(Contour, i, j, x, y, true);
            // - clear the contour in the X-channel
            FloodFill(x, y);
            for(int k = 0; k < MapW*MapH; k++)
               if(Bitmap->Data[k].x == AlphaContourValue || Bitmap->Data[k].x == AlphaFillValue)
                  Bitmap->Data[k].x = AlphaNullValue;

            if(!ProcessContour(&Contours.last))
            {
               Smooth++;
               goto BeginM;
            }
         }
      }
   }
   MakeMeshes();
}
//---------------------------------------------------------------------------
void FFrame::Triangulate(FBitmap* bmpZ)
{
   Contours.RemoveAll();

   if(bmpZ)
   {
      for(int j = 0; j < bmpZ->Height; j++)
      {
         for(int i = 0; i < bmpZ->Width; i++)
         {
            int xx = i - Bitmap->OriginX;
            int yy = j - Bitmap->OriginY;

            if(xx >= 0 && xx < Bitmap->Width && yy >= 0 && yy < Bitmap->Height)
            {
               RGBAX& col = Bitmap->GetPixel(xx, yy);
               RGBAX& colZ = bmpZ->GetPixel(i, j);
               col.z = colZ.r;
            }
         }
      }
      isZPresent = true;
   }

   CDT();
   ConstructMesh();
}
//---------------------------------------------------------------------------

