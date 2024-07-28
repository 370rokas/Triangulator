//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FFrame.h"
#include "myVector.h"
//---------------------------------------------------------------------------
// - обход препятствия, ожидаются _не_ диагональные точки пути и стены,
// в путь заносятся точки, исключая стартовую, включая терминальную
//---------------------------------------------------------------------------
int FFrame::GoAround(FNode* Contour, int x_p, int y_p, int x_w, int y_w, bool isRight)
{
   #define ADD_PATH_POINT(__x, __y) {\
      Bitmap->GetPixel(__x, __y).x = AlphaContourValue;\
      FNode* n = new FNode;\
      n->Next = Contour->Next;\
      Contour->Next = n;\
      n->x = __x;\
      n->y = __y;\
   }

   #define isWall(__x, __y) (Bitmap->GetPixel(__x, __y).x == AlphaMapValue)

   int x1, y1, x2, y2, p_len = 0;
   int x_p_last = x_p, y_p_last = y_p;

	// - сохраним начальную позицию
   int x_p_start = x_p;
   int y_p_start = y_p;
   int x_w_start = x_w;
   int y_w_start = y_w;

   #define checkStartPos() (x_p == x_p_start && y_p == y_p_start && \
                          	 x_w == x_w_start && y_w == y_w_start)

   // - установить очередную позицию точки стены
   #define assignWallCell(x, y) {\
   	x_w = x; y_w = y;\
      /* - выход при зацикливании */\
      if(checkStartPos()) break;\
   }

   // - установить очередную позицию точки пути
   #define assignPathCell(x, y) {\
   	x_p = x; y_p = y;\
      /* - выход при зацикливании */\
      if(checkStartPos()) break;\
      /* - добавить точку пути */\
      if(!(x_p_last == x_p && y_p_last == y_p))\
         ADD_PATH_POINT(x_p, y_p);\
      x_p_last = x_p; y_p_last = y_p;\
  		p_len++;\
   }

   while(1)
   {
		// - обход справа или слева, высчитываем очередные точки пути и стены
  		if(isRight)
      {
   		x1 = x_p - (y_w - y_p);
       	y1 = y_p - (x_p - x_w);
   		x2 = x_w - (y_w - y_p);
       	y2 = y_w - (x_p - x_w);
      }
  		else
      {
   		x1 = x_p + y_w - y_p;
     		y1 = y_p + x_p - x_w;
   		x2 = x_w + y_w - y_p;
     		y2 = y_w + x_p - x_w;
      }
      // - следующая за текущей точкой пути - стена?
  		if(isWall(x1, y1))
      {
         // - "поворачиваемся" на месте
         assignWallCell(x1, y1);
         continue;
      }
      // - следующая за текущей точкой пути - путь
      assignPathCell(x1, y1);
      // - следующая за точкой стены - стена?
      if(isWall(x2, y2))
     	{
         // - перемещаемся на шаг вперед вдоль ровной стены
         assignWallCell(x2, y2);
         continue;
     	}
      // - следующая за точкой стены - путь, огибаем поворот
      assignPathCell(x2, y2);
	}
 	return p_len;
}
//---------------------------------------------------------------------------
void FFrame::MinMaxClear(void)
{
   for(int j = 0; j < Bitmap->Height; j++)
   {
      MinBuf[j] = Bitmap->Width;
      MaxBuf[j] = -1;
   }
}
//---------------------------------------------------------------------------
void FFrame::MinMaxLine(int xn, int yn, int xk, int yk)
{
   int pxn = xn, pyn = yn;
   int dx, dy, s, sx, sy, kl, swap, incr1, incr2;

   #define PutPixLn(xn, yn) {\
      if(MinBuf[yn] > xn) MinBuf[yn] = xn;\
      if(MaxBuf[yn] < xn) MaxBuf[yn] = xn;\
   }

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

   PutPixLn(xn, yn); // - первая точка вектора
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
      PutPixLn(xn, yn); // - текущая точка вектора
   }
   #undef PutPixLn
}
//---------------------------------------------------------------------------
bool FFrame::CheckLine(int xn, int yn, int xk, int yk, FBitmap* bmp, bool isX)
{
   if(bmp == NULL) bmp = Bitmap;

   int pxn = xn, pyn = yn;
   int dx, dy, s, sx, sy, kl, swap, incr1, incr2;

   #define CheckPixLn(xn, yn) {\
      if(isX)\
      {\
         int x1 = xn;\
         int y1 = yn;\
         if(x1 >= bmp->Width) x1 -= bmp->Width;\
         if(x1 >= bmp->Width) return false;\
         if(y1 >= bmp->Height) y1 -= bmp->Height;\
         if(y1 >= bmp->Height) return false;\
         if(bmp->GetPixel(x1, y1).x) return false;\
      }\
      else\
      {\
         if(bmp->GetPixel(xn, yn).a) return false;\
      }\
   }

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

   CheckPixLn(xn, yn); // - первая точка вектора
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
      CheckPixLn(xn, yn); // - текущая точка вектора
   }
   #undef CheckPixLn
   return true;
}
//---------------------------------------------------------------------------
void FFrame::PutTriangle(double x0, double y0,
                         double x1, double y1,
                         double x2, double y2,
                         int MinX, int MinY,
                         int W, int H, char* Map)
{
   #define SWAP(a, b) { double t = a; a = b; b = t; }

   #define putpixel(x, y) {\
      int _xx = (x) - MinX;\
      int _yy = (y) - MinY;\
      if(_xx >= 0 && _xx < W && _yy >= 0 && _yy < H)\
        Map[_yy*W + _xx] = 1;\
   }

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

   if(fabs(y2 - y0) < 0.1) return;

   for(double y = y0; y < y2; y++)
   {
      double _x1 = x0 + (y - y0)*(x2 - x0)/(y2 - y0), _x2;
      if(y < y1)
         _x2 = x0 + (y - y0)*(x1 - x0)/(y1 - y0);
      else
      {
         if(fabs(y2 - y1) < 0.1)
            _x2 = x1;
         else
            _x2 = x1 + (y - y1)*(x2 - x1)/(y2 - y1);
      }
      if(_x1 > _x2) SWAP(_x1, _x2);
      for(double x = _x1; x < ceil(_x2); x++)
      {
         int xx = floor(x);
         int yy = floor(y);
         putpixel(xx, yy);
      }
   }
   #undef SWAP

   #define Line(y1, x1, y2, x2) {\
      int i;\
      int ystep, xstep;\
      int error;\
      int errorprev;\
      int y = y1, x = x1;\
      int ddy, ddx;\
      int dx = x2 - x1;\
      int dy = y2 - y1;\
      putpixel(y1, x1)\
      if(dy < 0)\
      {\
         ystep = -1;\
         dy = -dy;\
      }\
      else\
         ystep = 1;\
      if(dx < 0)\
      {\
         xstep = -1;\
         dx = -dx;\
      }\
      else\
         xstep = 1;\
      ddy = 2 * dy;\
      ddx = 2 * dx;\
      if(ddx >= ddy)\
      {\
         errorprev = error = dx;\
         for(i=0 ; i < dx ; i++)\
         {\
            x += xstep;\
            error += ddy;\
            if(error > ddx)\
            {\
               y += ystep;\
               error -= ddx;\
               if(error + errorprev < ddx)\
                  putpixel(y-ystep, x)\
               else\
               if(error + errorprev > ddx)\
                  putpixel(y, x-xstep)\
               else\
               {\
                  putpixel(y-ystep, x)\
                  putpixel(y, x-xstep)\
               }\
            }\
            putpixel(y, x)\
            errorprev = error;\
         }\
      }\
      else\
      {\
         errorprev = error = dy;\
         for(i=0 ; i < dy ; i++)\
         {\
            y += ystep;\
            error += ddx;\
            if(error > ddy)\
            {\
               x += xstep;\
               error -= ddy;\
               if(error + errorprev < ddy)\
                  putpixel(y, x-xstep)\
               else\
               if(error + errorprev > ddy)\
                  putpixel(y-ystep, x)\
               else\
               {\
                  putpixel(y, x-xstep)\
                  putpixel(y-ystep, x)\
               }\
            }\
            putpixel(y, x)\
            errorprev = error;\
         }\
      }\
      assert ((y == y2) && (x == x2));\
   }

   Line(x0, y0, x1, y1);
   Line(x1, y1, x2, y2);
   Line(x2, y2, x0, y0);
}
//---------------------------------------------------------------------------
void FFrame::PutTriangle(double x0, double y0,
                         double x1, double y1,
                         double x2, double y2,
                         double DeltaX, double DeltaY,
                         FBitmap* Texture)
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
         Bitmap->PutPixel(floor(x), floor(y), col);
      }
   }
   #undef SWAP
}
//---------------------------------------------------------------------------
// аффинное текстурирование, z-буфер, опциональная субпиксельная и
// субтексельная точность
//---------------------------------------------------------------------------
void FFrame::PutTexturedTriangle(FVertex* _a, FVertex* _b, FVertex* _c,
                                 FBitmap* Texture, FBitmap* bmp)
{
   #define F(x) (float)(x)

   // отсортируем вершины грани по sy
   FVertex *tmp_vertex;
   if(_a->y > _b->y) { tmp_vertex = _a; _a = _b; _b = tmp_vertex; }
   if(_a->y > _c->y) { tmp_vertex = _a; _a = _c; _c = tmp_vertex; }
   if(_b->y > _c->y) { tmp_vertex = _b; _b = _c; _c = tmp_vertex; }

   // грань нулевой высоты рисовать не будем
   if(_a->y == _c->y) return;

   struct VV {
      double x, y;
      double u, v;
      VV(int _x, int _y, double _u, double _v)
      {
         x = _x;
         y = _y;
         u = _u;
         v = _v;
      }
   };

   VV aa(_a->x, _a->y, _a->u, _a->v);
   VV bb(_b->x, _b->y, _b->u, _b->v);
   VV cc(_c->x, _c->y, _c->u, _c->v);

   VV* a = &aa;
   VV* b = &bb;
   VV* c = &cc;

   // посчитаем du/dsx, dv/dsx
   // считаем по самой длинной линии (т.е. проходящей через вершину B)
   double k = F(b->y - a->y)/F(c->y - a->y);
   double x_start = F(a->x) + F(c->x - a->x)*k;
   double u_start = F(a->u) + F(c->u - a->u)*k;
   double v_start = F(a->v) + F(c->v - a->v)*k;
   double x_end = b->x, u_end = b->u, v_end = b->v;
   double du = (u_start - u_end)/(x_start - x_end);
   double dv = (v_start - v_end)/(x_start - x_end);

   x_start = a->x;
   u_start = a->u;
   v_start = a->v;
   double dx_start = F(c->x - a->x)/F(c->y - a->y);
   double du_start = F(c->u - a->u)/F(c->y - a->y);
   double dv_start = F(c->v - a->v)/F(c->y - a->y);

   double dx_end, du_end, dv_end, x, u, v;
   if(b->y > a->y)
   {
      x_end = a->x;
      u_end = a->u;
      v_end = a->v;
      dx_end = F(b->x - a->x)/F(b->y - a->y);
      du_end = F(b->u - a->u)/F(b->y - a->y);
      dv_end = F(b->v - a->v)/F(b->y - a->y);
   }
   else
   {
      x_end = b->x;
      u_end = b->u;
      v_end = b->v;
      dx_end = F(c->x - b->x)/F(c->y - b->y);
      du_end = F(c->u - b->u)/F(c->y - b->y);
      dv_end = F(c->v - b->v)/F(c->y - b->y);
   }

   // построчная отрисовка грани
   for(int current_sy = a->y; current_sy < c->y; current_sy++)
   {
      if(current_sy == b->y)
      {
         x_end = b->x;
         u_end = b->u;
         v_end = b->v;
         dx_end = F(c->x - b->x)/F(c->y - b->y);
         du_end = F(c->u - b->u)/F(c->y - b->y);
         dv_end = F(c->v - b->v)/F(c->y - b->y);
      }

      // x_start должен находиться левее x_end
      int length;
      if(x_start > x_end)
      {
         x = x_end;
         u = u_end;
         v = v_end;
         length = floor(x_start) - floor(x_end);
      }
      else
      {
         x = x_start;
         u = u_start;
         v = v_start;
         length = floor(x_end) - floor(x_start);
      }

      // текстурируем строку
      int current_sx = (int)floor(x);

      if(length)
      {
         while(length--)
         {
            int tx = (int)floor(u);
            int ty = (int)floor(v);
            while(tx < 0) tx += Texture->Width;
            while(tx >= Texture->Width) tx -= Texture->Width;
            while(ty < 0) ty += Texture->Height;
            while(ty >= Texture->Height) ty -= Texture->Height;

            RGBAX& col = Texture->GetPixel(tx, ty);
            Bitmap->PutPixel(current_sx, current_sy, col);

            u += du;
            v += dv;
            current_sx++;
         }
      }

      // сдвигаем начальные и конечные значения x/u/v
      x_start += dx_start;
      u_start += du_start;
      v_start += dv_start;
      x_end += dx_end;
      u_end += du_end;
      v_end += dv_end;
   }
}
//---------------------------------------------------------------------------
void FFrame::PutCorrectedTriangle(FVertex* V0, FVertex* V1, FVertex* V2,
                                  int DeltaX, int DeltaY, FBitmap* Texture)
{
   #define SWAP(a, b) { double t = a; a = b; b = t; }

   double x0 = V0->x, y0 = V0->y;
   double x1 = V1->x, y1 = V1->y;
   double x2 = V2->x, y2 = V2->y;

   double z0r = V0->r, z0g = V0->g, z0b = V0->b;
   double z1r = V1->r, z1g = V1->g, z1b = V1->b;
   double z2r = V2->r, z2g = V2->g, z2b = V2->b;

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

   if(y2-y0 < 0.000001) return;

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
         int tx = x + DeltaX;
         int ty = y + DeltaY;

         while(tx < 0) tx += Texture->Width;
         while(tx >= Texture->Width) tx -= Texture->Width;
         while(ty < 0) ty += Texture->Height;
         while(ty >= Texture->Height) ty -= Texture->Height;

         RGBAX& col = Texture->GetPixel(tx, ty);
         RGBAX& _col = Bitmap->GetPixel(x, y);

         int r = 2*(_z1r + (x - _x1)*(_z2r - _z1r)/(_x2 - _x1));
         int g = 2*(_z1g + (x - _x1)*(_z2g - _z1g)/(_x2 - _x1));
         int b = 2*(_z1b + (x - _x1)*(_z2b - _z1b)/(_x2 - _x1));

         int rr = (int)col.r*r/255;
         int gg = (int)col.g*g/255;
         int bb = (int)col.b*b/255;

         if(rr > 0xFF) rr = 0xFF;
         if(gg > 0xFF) gg = 0xFF;
         if(bb > 0xFF) bb = 0xFF;

         _col.r = rr;
         _col.g = gg;
         _col.b = bb;
         _col.a = col.a;
      }
   }
   #undef SWAP
}
//---------------------------------------------------------------------------
bool FFrame::CheckTriangle(int x0, int y0, int x1, int y1, int x2, int y2)
{
   MinMaxClear();
   MinMaxLine(x0, y0, x1, y1);
   MinMaxLine(x1, y1, x2, y2);
   MinMaxLine(x2, y2, x0, y0);

   for(int j = 0; j < Bitmap->Height; j++)
   {
      if(MinBuf[j] < MaxBuf[j])
      {
         my_assert(MinBuf[j] >= 0 && MinBuf[j] < Bitmap->Width && MaxBuf[j] >= 0 && MaxBuf[j] < Bitmap->Width);
         for(int i = MinBuf[j]; i <= MaxBuf[j]; i++)
            if(Bitmap->GetPixel(i, j).a)
               return false;
      }
   }
   return true;
}
//---------------------------------------------------------------------------
void FFrame::FloodFill(int x, int y)
{
   struct _wrap {
      static int LineFill(FBitmap* Bitmap, int x, int y, int dir, int PrevXl, int PrevXr)
      {
         const int BorderColor = FFrame::AlphaContourValue;
         const int Color = FFrame::AlphaFillValue;

         #define getpixel(_x, _y) Bitmap->GetPixel(_x, _y).x
         #define putpixel(_x, _y) Bitmap->Data[(_y)*Bitmap->Width + (_x)].x = Color;

         int xl = x;
      	int xr = x;
      	int c;

      	// find line segment
      	do {
      		c = getpixel(--xl, y);
      	} while(xl > 0 && c != BorderColor && c != Color);

      	do {
      		c = getpixel(++xr, y);
      	} while(xr < Bitmap->Width-1 && c != BorderColor && c != Color);

      	xl++;
      	xr--;

         // fill segment
         for(x = xl; x <= xr; x++)
            putpixel(x, y);

         if(y < Bitmap->Height-1)
         {
         	// fill adjacent segments in the same direction
            for(x = xl; x <= xr; x++)
         	{
         		c = getpixel(x, y + dir);
         		if(c != BorderColor && c != Color)
         			x = LineFill(Bitmap, x, y + dir, dir, xl, xr);
         	}
         }

         if(y > 0)
         {
         	for(x = xl; x < PrevXl; x++)
         	{
         		c = getpixel(x, y - dir);
         		if(c != BorderColor && c != Color)
         			x = LineFill(Bitmap, x, y - dir, -dir, xl, xr);
         	}

         	for(x = PrevXr; x < xr; x++)
         	{
         		c = getpixel(x, y - dir);
         		if(c != BorderColor && c != Color)
         			x = LineFill(Bitmap, x, y - dir, -dir, xl, xr);
         	}
         }
      	return xr;
      }
   };

	_wrap::LineFill(Bitmap, x, y, 1, x, x);
}
//------------------------------------------------------------------------------
bool FindIntersect(double x11, double x12, double x21, double x22,
	                double y11, double y12, double y21, double y22,
                   double& x,  double& y)
{
   x = y = 0;
	bool s = 0;

   double t  = 0;
	double t1 = 0;
	double t2 = 0;

	double D  = (y12-y11)*(x21-x22) - (y21-y22)*(x12-x11);
	double D1 = (y12-y11)*(x21-x11) - (y21-y11)*(x12-x11);
	double D2 = (y21-y11)*(x21-x22) - (y21-y22)*(x21-x11);

	if(fabs(D) >= 0.000001)
	{
		t1 = D1/D;
		t2 = D2/D;
		if(t1 <= 1 && t1 >= 0 && t2 >= 0 && t2 <= 1)
		{
			s = true;
			x = x11 + (x12-x11)*t2;
			y = y11 + (y12-y11)*t2;
		}
		else
			s = false;
	}
   return s;
}
//------------------------------------------------------------------------------

