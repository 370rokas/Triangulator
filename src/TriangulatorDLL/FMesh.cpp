//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FMesh.h"
#include "FTriangulator.h"
//---------------------------------------------------------------------------
int FMesh::SearchEdge(int v0, int v1)
{
   // - поиск ребра по его вершинам
   int e = -1;
   for(int i = 0; i < Edges.sizeUsed; i++)
   {
      if((Edges[i].v0 == v0 && Edges[i].v1 == v1) ||
         (Edges[i].v0 == v1 && Edges[i].v1 == v0))
      {
         e = i;
         break;
      }
   }
   return e;
}
//---------------------------------------------------------------------------
int FMesh::SearchVertex(int x, int y)
{
   int v = -1;
   for(int i = 0; i < Vertices.sizeUsed; i++)
   {
      if(Vertices[i].x == x && Vertices[i].y == y)
      {
         v = i;
         break;
      }
   }
   return v;
}
//---------------------------------------------------------------------------
int FMesh::SearchTriangle(int _v0, int _v1, int _v2)
{
   int t = -1;
   for(int i = 0; i < Triangles.sizeUsed; i++)
   {
      int v0 = Triangles[i].v0;
      int v1 = Triangles[i].v1;
      int v2 = Triangles[i].v2;

      if((v0 == _v0 && v1 == _v1 && v2 == _v2) ||
         (v0 == _v0 && v1 == _v2 && v2 == _v1) ||
         (v0 == _v1 && v1 == _v0 && v2 == _v2) ||
         (v0 == _v1 && v1 == _v2 && v2 == _v0) ||
         (v0 == _v2 && v1 == _v0 && v2 == _v1) ||
         (v0 == _v2 && v1 == _v1 && v2 == _v0))
      {
         t = i;
         break;
      }
   }
   return t;
}
//---------------------------------------------------------------------------
int FMesh::StoreVertex(int x, int y, int z)
{
   int v = SearchVertex(x, y);
   if(v == -1)
   {
      v = Vertices.sizeUsed;
      Vertices.Add();
      Vertices.last->x = x;
      Vertices.last->y = y;
      Vertices.last->z = z;
      Vertices.last->tx = x;
      Vertices.last->ty = y;
      Vertices.last->r = 0;
      Vertices.last->g = 0;
      Vertices.last->b = 0;
      Vertices.last->w1 = 0;
      Vertices.last->w2 = 0;
      Vertices.last->w3 = 0;
   }
   return v;
}
//---------------------------------------------------------------------------
int FMesh::StoreEdge(int x0, int y0, int x1, int y1, int contourId)
{
   if(x0 == x1 && y0 == y1)
      return -1;

   // - поиск вершин ребра
   int v0 = StoreVertex(x0, y0);
   int v1 = StoreVertex(x1, y1);

   // - поиск ребра по его вершинам
   int e = SearchEdge(v0, v1);

   // - найдено ребро?
   if(e == -1)
   {
      e = Edges.sizeUsed;
      Edges.Add();
      Edges.last->v0 = v0;
      Edges.last->v1 = v1;
      Edges.last->contourId = contourId;
      Edges.last->isMark = false;
   }
   else
   {
      // - если ребро внутреннее, то возможно что оно станет внешним
      if(Edges[e].contourId == -1)
         Edges[e].contourId = contourId;
   }
   return e;
}
//---------------------------------------------------------------------------
int FMesh::StoreTriangle(int _v0, int _v1, int _v2)
{
   if(_v0 == _v1 || _v0 == _v2 || _v1 == _v2)
      return -1;
   
   int t = SearchTriangle(_v0, _v1, _v2);
   if(t == -1)
   {
      Triangles.Add();
      Triangles.last->isMark = false;
      Triangles.last->e0 = SearchEdge(_v0, _v1);
      Triangles.last->e1 = SearchEdge(_v1, _v2);
      Triangles.last->e2 = SearchEdge(_v2, _v0);

      int ax = Vertices[_v0].x - Vertices[_v1].x;
      int ay = Vertices[_v0].y - Vertices[_v1].y;
      int bx = Vertices[_v2].x - Vertices[_v1].x;
      int by = Vertices[_v2].y - Vertices[_v1].y;

      if(ax*by - ay*bx < 0)
      {
         Triangles.last->v0 = _v0;
         Triangles.last->v1 = _v2;
         Triangles.last->v2 = _v1;
      }
      else
      {
         Triangles.last->v0 = _v0;
         Triangles.last->v1 = _v1;
         Triangles.last->v2 = _v2;
      }
   }
   return t;
}
//---------------------------------------------------------------------------
int sort_angle_x0 = 0, sort_angle_y0 = 0;
//------------------------------------------------------------------------------
void FMesh::MakeTriangles(void)
{
   struct _wrap {
      static int sort_angle_function(const void *a, const void *b)
      {
         int* p1 = (int*)a;
         int* p2 = (int*)b;

         int dx1 = p1[0]-sort_angle_x0;
         int dy1 = p1[1]-sort_angle_y0;
         int dx2 = p2[0]-sort_angle_x0;
         int dy2 = p2[1]-sort_angle_y0;

         if((dx1 == 0 && dy1 == 0) || (dx2 == 0 && dy2 == 0))
            my_assert(!((dx1 == 0 && dy1 == 0) || (dx2 == 0 && dy2 == 0)));
         double a1 = atan2(dx1, dy1) + M_PI;
         double a2 = atan2(dx2, dy2) + M_PI;

         if(a1 > a2) return -1;
         if(a1 < a2) return 1;
         return 0;
      }
   };

   // - цикл по всем вершинам
   for(int v = 0; v < Vertices.sizeUsed; v++)
   {
      // - текущая вершина
      FVertex* V = &Vertices[v];
      sort_angle_x0 = Vertices[v].x;
      sort_angle_y0 = Vertices[v].y;

      // - создадим массив ребер, которые прилегают к данной вершине
      FStream EdgesFan;
      for(int e = 0; e < Edges.sizeUsed; e++)
      {
         int v0 = Edges[e].v0;
         int v1 = Edges[e].v1;

         if(v0 == v)
         {
            EdgesFan.putint(Vertices[v1].x);
            EdgesFan.putint(Vertices[v1].y);
         }
         else
         if(v1 == v)
         {
            EdgesFan.putint(Vertices[v0].x);
            EdgesFan.putint(Vertices[v0].y);
         }
      }
      my_assert(EdgesFan.Size);
      int Num = EdgesFan.Size/(4*2);

      // - сортировка по углу
      qsort(EdgesFan.Data, Num, 4*2, _wrap::sort_angle_function);

      EdgesFan.putint(EdgesFan.uiData[0]);
      EdgesFan.putint(EdgesFan.uiData[1]);
      EdgesFan.rewind();

      for(int i = 0; i < Num; i++)
      {
         int x0 = sort_angle_x0;
         int y0 = sort_angle_y0;
         int x1 = EdgesFan.uiData[i*2+0];
         int y1 = EdgesFan.uiData[i*2+1];
         int x2 = EdgesFan.uiData[i*2+2];
         int y2 = EdgesFan.uiData[i*2+3];

         my_assert(!((x1-x0 == 0 && y1-y0 == 0) || (x2-x0 == 0 && y2-y0 == 0)));

         double a1 = atan2(x1-x0, y1-y0) + M_PI;
         double a2 = atan2(x2-x0, y2-y0) + M_PI;

         double da = a2 - a1;
         if(da < 0) da += 2*M_PI;
         my_assert(da >= 0);

         if(a2 - a1 < M_PI)
         {
            int v0 = StoreVertex(x0, y0);
            int v1 = StoreVertex(x1, y1);
            int v2 = StoreVertex(x2, y2);

            if(SearchEdge(v0, v1) != -1 && SearchEdge(v1, v2) != -1 && SearchEdge(v2, v0) != -1)
            {
               StoreTriangle(v0, v1, v2);
            }
         }
      }
   }
}
//---------------------------------------------------------------------------
void FMesh::MarkEdge(int e)
{
   FEdge* E = &Edges[e];
   E->isMark = true;

   int num = 0;
   for(int i = 0; i < Triangles.sizeUsed; i++)
   {
      FTriangle* T = &Triangles[i];
      if(!T->isMark)
      {
         int e0 = T->e0;
         int e1 = T->e1;
         int e2 = T->e2;
         my_assert(e0 != -1 && e1 != -1 && e2 != -1);

         if(e0 == e || e1 == e || e2 == e)
         {
            T->isMark = true;
            PaintedTriangles++;
            FEdge* E0 = &Edges[e0];
            FEdge* E1 = &Edges[e1];
            FEdge* E2 = &Edges[e2];

            if(E0->contourId == -1 && !E0->isMark)
               MarkEdge(e0);
            if(E1->contourId == -1 && !E1->isMark)
               MarkEdge(e1);
            if(E2->contourId == -1 && !E2->isMark)
               MarkEdge(e2);
            num++;

//            if(!(num <= 2))
//               my_assert(num <= 2);
         }
      }
   }
}
//---------------------------------------------------------------------------
void FMesh::CalcArea(void)
{
   Area = 0;
   for(int i = 0; i < Triangles.sizeUsed; i++)
   {
      FTriangle* T = &Triangles[i];
      FVertex* V0 = &Vertices[T->v0];
      FVertex* V1 = &Vertices[T->v1];
      FVertex* V2 = &Vertices[T->v2];

      double ax = V1->x - V0->x;
      double ay = V1->y - V0->y;
      double ad = sqrt(ax*ax + ay*ay);

      double bx = V2->x - V1->x;
      double by = V2->y - V1->y;
      double bd = sqrt(bx*bx + by*by);

      if(ad > 0.0000001 && bd > 0.0000001)
      {
         // - угол между векторами
         double angle = acos((ax*bx + ay*by)/(ad*bd));

         // - площадь треугольника
         double S = 0.5*ad*bd*sin(angle);
         Area += S;
      }

      int min_x = V0->x < V1->x ? V0->x : V1->x;
      if(V2->x < min_x) min_x = V2->x;
      int max_x = V0->x > V1->x ? V0->x : V1->x;
      if(V2->x > max_x) max_x = V2->x;
      int min_y = V0->y < V1->y ? V0->y : V1->y;
      if(V2->y < min_y) min_y = V2->y;
      int max_y = V0->y > V1->y ? V0->y : V1->y;
      if(V2->y > max_y) max_y = V2->y;

      if(max_x-min_x >= MAX_TEXTURE_SIZE || max_y-min_y >= MAX_TEXTURE_SIZE)
      {
         if(MAX_TEXTURE_SIZE < 512/2)
            MAX_TEXTURE_SIZE <<= 1;
         else
            _ERROR("Image is too big! %ux%u", max_x-min_x, max_y-min_y);
      }
   }

   for(int i = 0; i < Vertices.sizeUsed; i++)
   {
      int x = Vertices[i].x;
      int y = Vertices[i].y;

      if(MaxX < x) MaxX = x;
      if(MinX > x) MinX = x;

      if(MaxY < y) MaxY = y;
      if(MinY > y) MinY = y;
   }
}
//---------------------------------------------------------------------------
void FMesh::ClearMesh(void)
{
   for(int i = 0; i < Triangles.sizeUsed; i++)
      Triangles[i].isMark = false;
   for(int i = 0; i < Edges.sizeUsed; i++)
      Edges[i].isMark = false;
   PaintedTriangles = 0;
}
//---------------------------------------------------------------------------

