//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FFrame.h"
#include "myVector.h"
//---------------------------------------------------------------------------
void FFrame::MakeMeshMap(FMesh* M)
{
   // - ширина bbox-а
   const int D = 1;
   int W = M->MaxX - M->MinX + 2*D;
   int H = M->MaxY - M->MinY + 2*D;

   // - карта для отрисовки образа
   char* Map = new char[W*H];
   memset(Map, 0, W*H);

   // - отрисовываем треугольники меша в Map размером с ББокс
   for(int i = 0; i < M->Triangles.sizeUsed; i++)
   {
      int v0 = M->Triangles[i].v0;
      int v1 = M->Triangles[i].v1;
      int v2 = M->Triangles[i].v2;

      int x0 = M->Vertices[v0].x;
      int y0 = M->Vertices[v0].y;
      int x1 = M->Vertices[v1].x;
      int y1 = M->Vertices[v1].y;
      int x2 = M->Vertices[v2].x;
      int y2 = M->Vertices[v2].y;

      PutTriangle(x0, y0, x1, y1, x2, y2, M->MinX - D, M->MinY - D, W, H, Map);
   }

   // - паковка образа с помощью RLE
   M->OpaquePixels = 0;
   M->RleStream.reload(H*3*4*3);
   for(int j = 0; j < H; j++)
   {
      int Len = 0, ch = 0, x = 0, y = j;
      for(int i = 0; i < W; i++)
      {
         RGBAX& col = Bitmap->GetPixel(x + M->MinX - D - Bitmap->OriginX,
                                       y + M->MinY - D - Bitmap->OriginY);
         M->OpaquePixels += !!col.a;
         if(!!Map[j*W + i] != ch)
         {
            if(ch)
            {
               M->RleStream.putint(x + M->MinX - D - Bitmap->OriginX);
               M->RleStream.putint(y + M->MinY - D - Bitmap->OriginY);
               M->RleStream.putint(Len);
            }
            ch ^= 1;
            Len = 0;
            x = i;
         }
         Len++;
      }
      if(ch)
      {
         M->RleStream.putint(x + M->MinX - D - Bitmap->OriginX);
         M->RleStream.putint(y + M->MinY - D - Bitmap->OriginY);
         M->RleStream.putint(Len);
      }
   }
   delete[] Map;
}
//---------------------------------------------------------------------------
int FFrame::CheckMeshMap(FBitmap* bmp, int x0, int y0, FMesh* M)
{
   #define CHECK(ii, jj) {\
      int x1 = x0 + ii;\
      int y1 = y0 + jj;\
      if(x1 < 0) x1 += W;\
      if(y1 < 0) y1 += H;\
      if(x1 < 0 || y1 < 0) return -1;\
      if(W >= 512 && H >= 512)\
      {\
         if(x1 >= W) x1 -= W;\
         if(y1 >= H) y1 -= H;\
      }\
      if(x1 >= W) return -1;\
      if(y1 >= H) return -1;\
      RGBAX& col = bmp->Data[y1*W + x1];\
      if(!(col.x == 0 || col.col == Bitmap->Data[(jj)*Bitmap->Width + ii].col)) return -1;\
   }

   int W = bmp->Width;
   int H = bmp->Height;

   int Opaques = 0;

   int Num = M->RleStream.Size/(4*3);
   M->RleStream.rewind();
   for(int k = 0; k < Num; k++)
   {
      int i = M->RleStream._getint();
      int j = M->RleStream._getint();
      int l = M->RleStream._getint();
      my_assert(l);

      CHECK(i, j);
      CHECK(i+l-1, j);
      Opaques += l;
   }

   M->RleStream.rewind();
   for(int k = 0; k < Num; k++)
   {
      int i = M->RleStream._getint();
      int j = M->RleStream._getint();
      int l = M->RleStream._getint();
      my_assert(l);

      for(int t = 0; t < l; t++)
         CHECK(i+t, j);
   }
   #undef CHECK

   for(int j = M->MinY; j < M->MaxY; j++)
   {
      for(int i = M->MinX; i < M->MaxX; i++)
      {
         int x1 = x0 + i;
         int y1 = y0 + j;
         if(x1 >= 0 && y1 >= 0)
         {
            if(x1 >= W) x1 -= W;
            if(x1 >= W) return -1;
            if(y1 >= H) y1 -= H;
            if(y1 >= H) return -1;
            RGBAX& col = bmp->Data[y1*W + x1];
            Opaques += !!col.x;
         }
      }
   }
   return Opaques;
}
//---------------------------------------------------------------------------
void FFrame::PutMeshMap(FBitmap* bmp, int x0, int y0, FMesh* M)
{
/*   #define PUT(ii, jj) {\
      int x1 = x0 + ii;\
      int y1 = y0 + jj;\
      if(x1 < 0) x1 += bmp->Width;\
      if(y1 < 0) y1 += bmp->Height;\
      if(x1 >= bmp->Width) x1 -= bmp->Width;\
      my_assert(x1 < bmp->Width);\
      if(y1 >= bmp->Height) y1 -= bmp->Height;\
      my_assert(y1 < bmp->Height);\
      RGBAX col = Bitmap->Data[(jj)*Bitmap->Width + ii];\
      col.x = 1;\
      bmp->PutPixel(x1, y1, col);\
   }
*/
   int Num = M->RleStream.Size/(4*3);
   M->RleStream.rewind();
   for(int k = 0; k < Num; k++)
   {
      int i = M->RleStream._getint();
      int j = M->RleStream._getint();
      int l = M->RleStream._getint();
      my_assert(l);

      for(int t = 0; t < l; t++)
      {
         //PUT(i+t, j);
         int ii = i+t, jj = j;

         int x1 = x0 + ii;
         int y1 = y0 + jj;
         if(x1 < 0) x1 += bmp->Width;
         if(y1 < 0) y1 += bmp->Height;
         if(x1 >= bmp->Width) x1 -= bmp->Width;
         my_assert(x1 < bmp->Width);
         if(y1 >= bmp->Height) y1 -= bmp->Height;
         my_assert(y1 < bmp->Height);
         RGBAX col = Bitmap->Data[(jj)*Bitmap->Width + ii];
         col.x = 1;
         bmp->PutPixel(x1, y1, col);
      }
   }
   #undef PUT
}
//---------------------------------------------------------------------------
int FFrame::TryToPut(FBitmap* bmp, int& max_x, int& max_y, FMesh* M)
{
   const int D1 = 1;
   const int D2 = 4;
   for(int j = 0; j < bmp->Width; j += D1)
   {
      for(int i = 0; i < bmp->Height; i += D1)
      {
         int MaxK = -1;
         max_x = max_y = 0;
         for(int t1 = 0; t1 < D2; t1++)
         {
            for(int t2 = 0; t2 < D2; t2++)
            {
               int x = i - M->MinX - Smooth + t1;
               int y = j - M->MinY - Smooth + t2;

//               if(x < -M->MinX) x = -M->MinX;
//               if(y < -M->MinY) y = -M->MinY;

               int K = CheckMeshMap(bmp, x, y, M);
               if(K != -1 && K > MaxK)
               {
                  MaxK = K;
                  max_x = x;
                  max_y = y;
               }
            }
         }
         if(MaxK >= 0) return MaxK;
      }
   }
   return -1;
}
//---------------------------------------------------------------------------
bool FFrame::PackMeshes(FBitmap* bmp)
{
   struct _wrap {
      static int meshes_sort_function(const void *a, const void *b)
      {
         FMesh* M1 = *(FMesh**)a;
         FMesh* M2 = *(FMesh**)b;
         if(M1->Area > M2->Area) return -1;
         if(M1->Area < M2->Area) return 1;
         return 0;
      }
   };
   qsort(Meshes.pArray, Meshes.sizeUsed, 4, _wrap::meshes_sort_function);

   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      M->DeltaTextX = 0x7FFFFFFF;
      M->DeltaTextY = 0x7FFFFFFF;
   }

   FBitmap* backupBmp = new FBitmap(bmp->Width, bmp->Height);
   memcpy(backupBmp->Data, bmp->Data, sizeof(bmp->Data[0])*bmp->Width*bmp->Height);

   srand(0x1234567);
   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      int x = 0, y = 0;
      int K = TryToPut(bmp, x, y, M);
      if(K < 0)
      {
//         for(int m = 0; m < Meshes.sizeUsed; m++)
//         {
//            FMesh* M = Meshes[m];
//            if(M->DeltaTextX != 0x7FFFFFFF && M->DeltaTextY != 0x7FFFFFFF)
//               UnPutMeshMap(bmp, M->DeltaTextX, M->DeltaTextY, M);
//         }
         memcpy(bmp->Data, backupBmp->Data, sizeof(bmp->Data[0])*bmp->Width*bmp->Height);
         delete backupBmp;
         return false;
      }
      PutMeshMap(bmp, x, y, M);
      M->DeltaTextX = x;
      M->DeltaTextY = y;
   }

   // - окончательный просчет всех координат
   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];

      for(int i = 0; i < M->Vertices.sizeUsed; i++)
      {
         int x = M->Vertices[i].x;
         int y = M->Vertices[i].y;

         int tx = x + M->DeltaTextX - Bitmap->OriginX;
         int ty = y + M->DeltaTextY - Bitmap->OriginY;

         M->Vertices[i].tx = tx;
         M->Vertices[i].ty = ty;

         M->Vertices[i].x = x = (int)((double)x*Scale);
         M->Vertices[i].y = y = (int)((double)y*Scale);
         M->Vertices[i].z = GetZ(x, y);

         if(Width < x) Width = x;
         if(Height < y) Height = y;
      }
   }
   delete backupBmp;
//   delete Bitmap;
//   Bitmap = NULL;

   return true;
}
//---------------------------------------------------------------------------
void BaryCoords(double ax, double ay, double bx, double by, double cx, double cy,
  					 double ptX, double ptY, Vector& res)
{
	double acx	= ax - cx;
	double acy	= ay - cy;
	double bcx	= bx - cx;
	double bcy	= by - cy;
	double pcx	= ptX - cx;
	double pcy	= ptY - cy;
	double m00 	= acx*acx + acy*acy;
	double m01 	= acx*bcx + acy*bcy;
	double m11 	= bcx*bcx + bcy*bcy;
	double r0  	= acx*pcx + acy*pcy;
	double r1  	= bcx*pcx + bcy*pcy;
	double det 	= m00*m11 - m01*m01;
//	my_assert(fabs(det) > 0.0);
	double invDet = 1.0/det;

	res.x = (m11*r0 - m01*r1)*invDet;
	res.y = (m00*r1 - m01*r0)*invDet;
	res.z = 1.0 - res.x - res.y;
}
//---------------------------------------------------------------------------
bool FFrame::CheckITriangle(FBitmap* bmp, FVertex* V0, FVertex* V1, FVertex* V2, int rgb, Vector* W)
{
   bool isIntersect = false;
   double x0 = V0->x, y0 = V0->y, z0 = V0->rgba[rgb];
   double x1 = V1->x, y1 = V1->y, z1 = V1->rgba[rgb];
   double x2 = V2->x, y2 = V2->y, z2 = V2->rgba[rgb];

   /* - проверяем весь треугольник на пересечение */

   #define SWAP(a, b) { double t = a; a = b; b = t; }

   /* - сортировка по y */
   if(y0 > y1)
   {
      SWAP(x0, x1);
      SWAP(y0, y1);
      SWAP(z0, z1);
   }
   if(y0 > y2)
   {
      SWAP(x0, x2);
      SWAP(y0, y2);
      SWAP(z0, z2);
   }
   if(y1 > y2)
   {
      SWAP(x1, x2);
      SWAP(y1, y2);
      SWAP(z1, z2);
   }

   W->x = W->y = W->z = 0;
   if(y2-y0 >= 0.000001)
   {
      for(double y = y0; y < y2; y++)
      {
         /* - пересечение с длиннейшим ребром */
         double _x1 = x0 + (y - y0)*(x2 - x0)/(y2 - y0);
         double _z1 = z0 + (y - y0)*(z2 - z0)/(y2 - y0);

         /* - пересечение с коротким ребром */
         double _x2, _z2;
         if(y < y1)
         {
            _x2 = x0 + (y - y0)*(x1 - x0)/(y1 - y0);
            _z2 = z0 + (y - y0)*(z1 - z0)/(y1 - y0);
         }
         else
         {
            if(y2 == y1)
            {
               _x2 = x1;
               _z2 = z1;
            }
            else
            {
               _x2 = x1 + (y - y1)*(x2 - x1)/(y2 - y1);
               _z2 = z1 + (y - y1)*(z2 - z1)/(y2 - y1);
            }
         }

         /* - обмен если надо */
         if(_x1 > _x2)
         {
            SWAP(_x1, _x2);
            SWAP(_z1, _z2);
         }

         /* - проверка линии */
         int start_x = floor(_x1);
         int end_x = floor(_x2);
         for(int x = start_x; x < end_x; x++)
         {
            int tx = x - bmp->OriginX;
            int ty = y - bmp->OriginY;

            RGBAX& col = bmp->GetPixel(tx, ty);

            int z = _z1 + (x - _x1)*(_z2 - _z1)/(_x2 - _x1);
            int d = (int)col.rgb[rgb] - z;
            if(d >= 0)
            {
               static Vector w;
               BaryCoords(V0->x, V0->y, V1->x, V1->y, V2->x, V2->y, x, y, w);

               W->x += w.x*d;
               W->y += w.y*d;
               W->z += w.z*d;
            }
            if(z < col.rgb[rgb])
               isIntersect = true;
         }
      }
   }
   #undef SWAP
   return isIntersect;
}
//---------------------------------------------------------------------------
void FFrame::CorrectLevels(void)
{
   srand(0x1234567);
   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      for(int v = 0; v < M->Vertices.sizeUsed; v++)
      {
         M->Vertices[v].r = 0;
         M->Vertices[v].g = 0;
         M->Vertices[v].b = 0;

         M->Vertices[v].w1 = 0;
         M->Vertices[v].w2 = 0;
         M->Vertices[v].w3 = 0;
      }
   }

   for(int k = 0;; k++)
   {
      bool isIntersect = false;
      double MaxW1 = 0, MaxW2 = 0, MaxW3 = 0;
      for(int m = 0; m < Meshes.sizeUsed; m++)
      {
         FMesh* M = Meshes[m];
         for(int t = 0; t < M->Triangles.sizeUsed; t++)
         {
            FTriangle* T = &M->Triangles[t];
            FVertex* V0 = &M->Vertices[T->v0];
            FVertex* V1 = &M->Vertices[T->v1];
            FVertex* V2 = &M->Vertices[T->v2];

            Vector W;
            if(CheckITriangle(Bitmap, V0, V1, V2, 0, &W))
            {
               V0->w1 += W.x;
               V1->w1 += W.y;
               V2->w1 += W.z;
               isIntersect = true;
            }
            if(CheckITriangle(Bitmap, V0, V1, V2, 1, &W))
            {
               V0->w2 += W.x;
               V1->w2 += W.y;
               V2->w2 += W.z;
               isIntersect = true;
            }
            if(CheckITriangle(Bitmap, V0, V1, V2, 2, &W))
            {
               V0->w3 += W.x;
               V1->w3 += W.y;
               V2->w3 += W.z;
               isIntersect = true;
            }
            if(isIntersect)
            {
               if(V0->w1 > MaxW1) MaxW1 = V0->w1;
               if(V1->w1 > MaxW1) MaxW1 = V1->w1;
               if(V2->w1 > MaxW1) MaxW1 = V2->w1;
               if(V0->w2 > MaxW2) MaxW2 = V0->w2;
               if(V1->w2 > MaxW2) MaxW2 = V1->w2;
               if(V2->w2 > MaxW2) MaxW2 = V2->w2;
               if(V0->w3 > MaxW3) MaxW3 = V0->w3;
               if(V1->w3 > MaxW3) MaxW3 = V1->w3;
               if(V2->w3 > MaxW3) MaxW3 = V2->w3;
            }
         }
      }
      if(!isIntersect) break;

      // - normalize and correct rgb
      for(int m = 0; m < Meshes.sizeUsed; m++)
      {
         FMesh* M = Meshes[m];
         for(int v = 0; v < M->Vertices.sizeUsed; v++)
         {
            FVertex* V = &M->Vertices[v];

            const double K = 8;
            const double D = 0.1;
            if(MaxW1 > 0)
            {
               double d = K*V->w1/MaxW1;
               if(d < D) d = D;
               V->r += d;
            }
            if(MaxW2 > 0)
            {
               double d = K*V->w2/MaxW2;
               if(d < D) d = D;
               V->g += d;
            }
            if(MaxW3 > 0)
            {
               double d = K*V->w3/MaxW3;
               if(d < D) d = D;
               V->b += d;
            }

            if(V->r > 511) V->r = 511;
            if(V->r < 0)   V->r = 0;
            if(V->g > 511) V->g = 511;
            if(V->g < 0)   V->g = 0;
            if(V->b > 511) V->b = 511;
            if(V->b < 0)   V->b = 0;

            V->w1 = V->w2 = V->w3 = 0;
         }
      }
   }

   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      for(int v = 0; v < M->Vertices.sizeUsed; v++)
      {
         FVertex* V = &M->Vertices[v];
         V->r = ((int)V->r>>1)&0xFF;
         V->g = ((int)V->g>>1)&0xFF;
         V->b = ((int)V->b>>1)&0xFF;
      }
   }

//   FBitmap bmp(Bitmap->Width, Bitmap->Height);
//   bmp.Clear();

   #define SWAP(a, b) { double t = a; a = b; b = t; }
   // - отрисовка меша
   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      for(int t = 0; t < M->Triangles.sizeUsed; t++)
      {
         FTriangle* T = &M->Triangles[t];
         FVertex* V0 = &M->Vertices[T->v0];
         FVertex* V1 = &M->Vertices[T->v1];
         FVertex* V2 = &M->Vertices[T->v2];

         double x0 = V0->x, y0 = V0->y, z0r = V0->r, z0g = V0->g, z0b = V0->b;
         double x1 = V1->x, y1 = V1->y, z1r = V1->r, z1g = V1->g, z1b = V1->b;
         double x2 = V2->x, y2 = V2->y, z2r = V2->r, z2g = V2->g, z2b = V2->b;

         // - сортировка по y
         if(y0 > y1)
         {
            SWAP(x0, x1);
            SWAP(y0, y1);
            SWAP(z0r, z1r);
            SWAP(z0g, z1g);
            SWAP(z0b, z1b);
         }
         if(y0 > y2)
         {
            SWAP(x0, x2);
            SWAP(y0, y2);
            SWAP(z0r, z2r);
            SWAP(z0g, z2g);
            SWAP(z0b, z2b);
         }
         if(y1 > y2)
         {
            SWAP(x1, x2);
            SWAP(y1, y2);
            SWAP(z1r, z2r);
            SWAP(z1g, z2g);
            SWAP(z1b, z2b);
         }

         if(y2-y0 < 0.000001) continue;

         int start_y = floor(y0);
         int end_y = floor(y2);
         for(int y = start_y; y < end_y; y++)
         {
            // - пересечение с длиннейшим ребром
            double _x1 = x0 + (y - y0)*(x2 - x0)/(y2 - y0);
            double _z1r = z0r + (y - y0)*(z2r - z0r)/(y2 - y0);
            double _z1g = z0g + (y - y0)*(z2g - z0g)/(y2 - y0);
            double _z1b = z0b + (y - y0)*(z2b - z0b)/(y2 - y0);

            // - пересечение с коротким ребром
            double _x2, _z2r, _z2g, _z2b;
            if(y < y1)
            {
               _x2 = x0 + (y - y0)*(x1 - x0)/(y1 - y0);
               _z2r = z0r + (y - y0)*(z1r - z0r)/(y1 - y0);
               _z2g = z0g + (y - y0)*(z1g - z0g)/(y1 - y0);
               _z2b = z0b + (y - y0)*(z1b - z0b)/(y1 - y0);
            }
            else
            {
               if(y2 == y1)
               {
                  _x2 = x1;
                  _z2r = z1r;
                  _z2g = z1g;
                  _z2b = z1b;
               }
               else
               {
                  _x2 = x1 + (y - y1)*(x2 - x1)/(y2 - y1);
                  _z2r = z1r + (y - y1)*(z2r - z1r)/(y2 - y1);
                  _z2g = z1g + (y - y1)*(z2g - z1g)/(y2 - y1);
                  _z2b = z1b + (y - y1)*(z2b - z1b)/(y2 - y1);
               }
            }

            // - обмен если надо
            if(_x1 > _x2)
            {
               SWAP(_x1, _x2);
               SWAP(_z1r, _z2r);
               SWAP(_z1g, _z2g);
               SWAP(_z1b, _z2b);
            }

            int start_x = floor(_x1);
            int end_x = floor(_x2);
            // - отрисовка линии
            for(int x = start_x; x < end_x; x++)
            {
               int tx = x - Bitmap->OriginX;
               int ty = y - Bitmap->OriginY;

               RGBAX& col = Bitmap->GetPixel(tx, ty);
//               RGBAX& _col = bmp.GetPixel(tx, ty);

               int r = 2*(_z1r + (x - _x1)*(_z2r - _z1r)/(_x2 - _x1));
               int g = 2*(_z1g + (x - _x1)*(_z2g - _z1g)/(_x2 - _x1));
               int b = 2*(_z1b + (x - _x1)*(_z2b - _z1b)/(_x2 - _x1));

               int rr = rand()%10, gg = rand()%10, bb = rand()%10;

               if(r) rr += (int)col.r*0xF0/r;
               if(g) gg += (int)col.g*0xF0/g;
               if(b) bb += (int)col.b*0xF0/b;

               if(rr > 0xF0) rr = 0xF0;
               if(gg > 0xF0) gg = 0xF0;
               if(bb > 0xF0) bb = 0xF0;

               col.r = rr&0xF0;
               col.g = gg&0xF0;
               col.b = bb&0xF0;

               rr = (int)col.r*r/255;
               gg = (int)col.g*g/255;
               bb = (int)col.b*b/255;

               if(rr > 0xFF) rr = 0xFF;
               if(gg > 0xFF) gg = 0xFF;
               if(bb > 0xFF) bb = 0xFF;

//               _col.r = rr;
//               _col.g = gg;
//               _col.b = bb;
            }
         }
      }
   }
   #undef SWAP
/*
   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      for(int v = 0; v < M->Vertices.sizeUsed; v++)
      {
         M->Vertices[v].z = (M->Vertices[v].r + M->Vertices[v].g + M->Vertices[v].b)/3;
      }
   }
*/
//   bmp.SaveToFile("!.tga");
}
//---------------------------------------------------------------------------

