//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FFrame.h"
//---------------------------------------------------------------------------
void FFrame::Bending(FNode* Contour, double val, int Num)
{
   FNode* p0 = Contour;
   FNode* p1 = p0->Next;
   FNode* p2 = p1->Next;

   if(p0 == p1 || p1 == p2 || p2 == p0) return;

   int num = 0;
   while(num < Num)
   {
      double ax = p0->x - p1->x;
      double ay = p0->y - p1->y;
      double da = sqrt(ax*ax + ay*ay) + 0.0000001;
      ax /= da; ay /= da;

      double bx = p2->x - p1->x;
      double by = p2->y - p1->y;
      double db = sqrt(bx*bx + by*by) + 0.0000001;
      bx /= db; by /= db;

      // - угол между векторами
      double _cos = (ax*bx + ay*by)/(da*db);

      double nx = ax + bx;
      double ny = ay + by;
      double dn = sqrt(nx*nx + ny*ny) + 0.0000001;
      nx /= dn; ny /= dn;

      if(ax*by - ay*bx > 0)
      {
         nx = -nx;
         ny = -ny;
      }

      // - получили координаты конца вектора отступа
      FNode pt;
      pt.x = p1->x + val*nx;
      pt.y = p1->y + val*ny;

      // - проводим линию от новой точки к старой
      if(CheckLine(p0->x, p0->y, pt.x, pt.y) &&
         CheckLine(pt.x, pt.y, p0->x, p0->y) &&
         CheckLine(p2->x, p2->y, pt.x, pt.y) &&
         CheckLine(pt.x, pt.y, p2->x, p2->y))
      {
         p1->x = pt.x;
         p1->y = pt.y;
      }
      p0 = p1;
      p1 = p2;
      p2 = p2->Next;

      if(p0 == Contour) num++;
   }
}
//---------------------------------------------------------------------------
void FFrame::OptimizeContourByTriangles(FNode** pContour, double Area)
{
   FNode* n0 = *pContour;
   FNode* n1 = n0->Next;
   FNode* n2 = n1->Next;

   while(n0 != n1 && n1 != n2 && n2 != n0)
   {
      double ax = n1->x - n0->x;
      double ay = n1->y - n0->y;
      double ad = sqrt(ax*ax + ay*ay) + 0.000001;

      double bx = n2->x - n1->x;
      double by = n2->y - n1->y;
      double bd = sqrt(bx*bx + by*by) + 0.000001;

      // - угол между векторами
      double angle = acos((ax*bx + ay*by)/(ad*bd));

      // - площадь треугольника
      double S = 0.5*ad*bd*sin(angle);

      int x0 = n0->x, y0 = n0->y;
      int x1 = n1->x, y1 = n1->y;
      int x2 = n2->x, y2 = n2->y;
//      if(CheckLine(x0, y0, x2, y2) && CheckLine(x2, y2, x0, y0) && S < Area)
      if(S < Area && CheckTriangle(x0, y0, x1, y1, x2, y2) && ad < MaxEdgeLen && bd < MaxEdgeLen)
      {
         n0->Next = n2;
         if(n1 == *pContour && n1 != n1->Next)
         {
            delete n1;
            *pContour = n2;
         }
         else
            delete n1;
      }
      else
         n0 = n1;

      n1 = n2;
      n2 = n2->Next;

      if(n0 == *pContour) break;
   }
}
//---------------------------------------------------------------------------
void FFrame::OptimizeContourByAngles(FNode** pContour, double Angle)
{
   FNode* n0 = *pContour;
   FNode* n1 = n0->Next;
   FNode* n2 = n1->Next;

   while(n0 != n1 && n1 != n2 && n2 != n0)
   {
      double ax = n1->x - n0->x;
      double ay = n1->y - n0->y;
      double ad = sqrt(ax*ax + ay*ay) + 0.000000001;

      double bx = n2->x - n1->x;
      double by = n2->y - n1->y;
      double bd = sqrt(bx*bx + by*by) + 0.000000001;

      // - угол между векторами
      double angle = acos((ax*bx + ay*by)/(ad*bd));

      int x0 = n0->x, y0 = n0->y;
      int x1 = n1->x, y1 = n1->y;
      int x2 = n2->x, y2 = n2->y;
//      if(CheckLine(n0->x, n0->y, n2->x, n2->y) &&
//         CheckLine(n2->x, n2->y, n0->x, n0->y) &&
//         angle < Angle)
      if(angle < Angle && CheckTriangle(x0, y0, x1, y1, x2, y2) && ad < MaxEdgeLen && bd < MaxEdgeLen)
      {
         n0->Next = n2;
         if(n1 == *pContour && n1 != n1->Next)
         {
            delete n1;
            *pContour = n2;
         }
         else
            delete n1;
      }
      else
         n0 = n1;

      n1 = n2;
      n2 = n2->Next;

      if(n0 == *pContour) break;
   }
}
//---------------------------------------------------------------------------
void FFrame::OptimizeContour(FNode** pContour, bool isBending)
{
   double k;
   //if(g_isAnimation)
      k = sqrt(Bitmap->Width*Bitmap->Height)/150.0;
   //else
   //   k = sqrt(Bitmap->Width*Bitmap->Height)/80.0;
   for(int i = 0; i < 150; i++)
   {
      OptimizeContourByTriangles(pContour, k*0.06*(double)i);
      OptimizeContourByAngles(pContour, k*(double)i/4000);
   }

   if(isBending)
   {
      Bending(*pContour, 0.1, 20);
      OptimizeContourByTriangles(pContour, 0.13*(double)250);
      OptimizeContourByAngles(pContour, (double)250/1000);
   }

   // - обход контура
   FNode* n0 = *pContour;
   FNode* n1 = n0->Next;
   while(1)
   {
      n0->x = int(n0->x) + Bitmap->OriginX;
      n0->y = int(n0->y) + Bitmap->OriginY;
      n1->x = int(n1->x);
      n1->y = int(n1->y);

      n0 = n1;
      n1 = n1->Next;
      if(n0 == *pContour) break;
   }
}
//---------------------------------------------------------------------------

