//---------------------------------------------------------------------------
#include "FMM\\FMM.h"
#include "FFrame.h"
//---------------------------------------------------------------------------
extern int MAX_TEXTURE_SIZE;
//---------------------------------------------------------------------------
void FFrame::MakeMeshes(int InitialNum)
{
   int Num = Meshes.sizeUsed-1;
   for(int i = Num; i >= 0; i--)
      SplitMesh(i, InitialNum);

   for(int i = 0; i < Meshes.sizeUsed; i++)
   {
      Meshes[i]->CalcArea();
      if(Meshes[i]->MaxX - Meshes[i]->MinX >= MAX_TEXTURE_SIZE-4 ||
         Meshes[i]->MaxY - Meshes[i]->MinY >= MAX_TEXTURE_SIZE-4)
      {
         SplitBigMesh(i);
         i = 0;
      }
   }

   for(int i = 0; i < Meshes.sizeUsed; i++)
   {
      assert(!(Meshes[i]->MaxX - Meshes[i]->MinX >= MAX_TEXTURE_SIZE ||
         Meshes[i]->MaxY - Meshes[i]->MinY >= MAX_TEXTURE_SIZE));
      Meshes[i]->CalcArea();
      MakeMeshMap(Meshes[i]);
   }
}
//---------------------------------------------------------------------------
bool FFrame::SplitMesh(int m, int Num)
{
   FMesh* M = Meshes[m];

   __int64 MinD = Bitmap->Width*Bitmap->Height+1;
   int MinE = -1, StartE = -1;

   // - цикл выбора ребра деления
   for(int i = 0; i < M->Edges.sizeUsed; i++)
   {
      // - выбираем не контурное ребро для деления
      FEdge* E = &M->Edges[i];
      if(E->contourId == -1)
      {
         // - пометим выбранное ребро
         M->ClearMesh();
         int contourId = E->contourId, startE = -1;
         E->contourId = 0;
         E->isMark = true;
         // - выбор любого внутреннего ребра, не равного разделительному
         for(int k = 0; k < M->Edges.sizeUsed; k++)
         {
            if(k != i && M->Edges[k].contourId == -1)
            {
               startE = k;
               // - раскрасим треугольники
               M->MarkEdge(startE);
               break;
            }
         }

         if(M->PaintedTriangles > Num &&
            M->Triangles.sizeUsed - M->PaintedTriangles > Num)
         {
            __int64 dx = M->Vertices[E->v1].x - M->Vertices[E->v0].x;
            __int64 dy = M->Vertices[E->v1].y - M->Vertices[E->v0].y;
            __int64 dd = dx*dx + dy*dy;
            if(dd <= MinD)
            {
               MinD = dd;
               MinE = i;
               StartE = startE;
            }
         }
         E->contourId = contourId;
      }
   }

   // - если есть минимальное ребро деления, то делим
   if(MinE != -1)
   {
      // - раскрасим треугольники
      FEdge* E = &M->Edges[MinE];
      M->ClearMesh();
      E->contourId = 0;
      E->isMark = true;
      M->MarkEdge(StartE);

      // - делим меш
      FMesh* M1 = new FMesh;
      FMesh* M2 = new FMesh;

      // - поместим новую границу явно
      M1->StoreEdge(M->Vertices[E->v0].x, M->Vertices[E->v0].y,
                    M->Vertices[E->v1].x, M->Vertices[E->v1].y, 0);
      M2->StoreEdge(M->Vertices[E->v0].x, M->Vertices[E->v0].y,
                    M->Vertices[E->v1].x, M->Vertices[E->v1].y, 0);

      // - распихиваем треугольники по двум мешам
      for(int i = 0; i < M->Triangles.sizeUsed; i++)
      {
         FVertex* V0 = &M->Vertices[M->Triangles[i].v0];
         FVertex* V1 = &M->Vertices[M->Triangles[i].v1];
         FVertex* V2 = &M->Vertices[M->Triangles[i].v2];

         if(M->Triangles[i].isMark)
         {
            M1->StoreEdge(V0->x, V0->y, V1->x, V1->y);
            M1->StoreEdge(V1->x, V1->y, V2->x, V2->y);
            M1->StoreEdge(V2->x, V2->y, V0->x, V0->y);
         }
         else
         {
            M2->StoreEdge(V0->x, V0->y, V1->x, V1->y);
            M2->StoreEdge(V1->x, V1->y, V2->x, V2->y);
            M2->StoreEdge(V2->x, V2->y, V0->x, V0->y);
         }
      }
      M1->MakeTriangles();
      M2->MakeTriangles();
      delete Meshes.pArray[m];
      Meshes.pArray[m] = M1;
      if(m == Meshes.sizeUsed-1) Meshes.last = M1;
      Meshes.Add(M2);

      int m1 = m;
      int m2 = Meshes.sizeUsed-1;

      SplitMesh(m1, Num+1);
      SplitMesh(m2, Num+1);

      return true;
   }
   return false;
}
//---------------------------------------------------------------------------
bool FFrame::SplitBigMesh(int m)
{
   FMesh* M = Meshes[m];
   M->ClearMesh();
   M->CalcArea();

   for(int i = 0; i < M->Edges.sizeUsed; i++)
      M->Edges[i].contourId = -1;

   int AllMarked = 0;
   int MeshNum = 0;
   while(1)
   {
      // - создадим новый меш
      FMesh* M1 = new FMesh;

      int Marked = 0;
      // - пометим один треугольник и его ребра для затравки
      for(int t = 0; t < M->Triangles.sizeUsed; t++)
      {
         FTriangle* T = &M->Triangles[t];
         if(!T->isMark)
         {
            int x0 = M->Vertices[T->v0].x;
            int y0 = M->Vertices[T->v0].y;
            int x1 = M->Vertices[T->v1].x;
            int y1 = M->Vertices[T->v1].y;
            int x2 = M->Vertices[T->v2].x;
            int y2 = M->Vertices[T->v2].y;

            int v0 = M1->StoreVertex(x0, y0);
            int v1 = M1->StoreVertex(x1, y1);
            int v2 = M1->StoreVertex(x2, y2);
            M1->StoreEdge(x0, y0, x1, y1);
            M1->StoreEdge(x1, y1, x2, y2);
            M1->StoreEdge(x2, y2, x0, y0);
            M1->StoreTriangle(v0, v1, v2);

            T->isMark = true;
            M->Edges[T->e0].contourId = MeshNum;
            M->Edges[T->e1].contourId = MeshNum;
            M->Edges[T->e2].contourId = MeshNum;
            AllMarked++;
            Marked++;
            break;
         }
      }
      if(Marked == 0)
      {
         delete M1;
         return false;
      }

      // - выгрызаем кусок из меша
      for(int t = 0; t < M->Triangles.sizeUsed; t++)
      {
         M1->CalcArea();
         if(M1->MaxX - M1->MinX >= MAX_TEXTURE_SIZE ||
            M1->MaxY - M1->MinY >= MAX_TEXTURE_SIZE) break;

         FTriangle* T = &M->Triangles[t];
         FEdge* E0 = &M->Edges[T->e0];
         FEdge* E1 = &M->Edges[T->e1];
         FEdge* E2 = &M->Edges[T->e2];

         if(!T->isMark &&
            (E0->contourId == MeshNum ||
             E1->contourId == MeshNum ||
             E2->contourId == MeshNum))
         {
            int x0 = M->Vertices[T->v0].x;
            int y0 = M->Vertices[T->v0].y;
            int x1 = M->Vertices[T->v1].x;
            int y1 = M->Vertices[T->v1].y;
            int x2 = M->Vertices[T->v2].x;
            int y2 = M->Vertices[T->v2].y;

            int v0 = M1->StoreVertex(x0, y0);
            int v1 = M1->StoreVertex(x1, y1);
            int v2 = M1->StoreVertex(x2, y2);
            M1->StoreEdge(x0, y0, x1, y1);
            M1->StoreEdge(x1, y1, x2, y2);
            M1->StoreEdge(x2, y2, x0, y0);
            M1->StoreTriangle(v0, v1, v2);


            // - пометим треугольник и его ребра
            T->isMark = true;
            E0->contourId = MeshNum;
            E1->contourId = MeshNum;
            E2->contourId = MeshNum;

            Marked++;
            AllMarked++;
            MeshNum++;
            t = 0;
         }
      }
      if(AllMarked == M->Triangles.sizeUsed)
      {
         delete M;
         Meshes.pArray[m] = M1;
         if(m == Meshes.sizeUsed-1)
            Meshes.last = M1;
         break;
      }
      Meshes.AddNULL();
      Meshes.last = Meshes.pArray[Meshes.sizeUsed-1] = M1;
   }
   return true;
}
//---------------------------------------------------------------------------
FFrame* FFrame::CloneSplitted(void)
{
   FFrame* F = new FFrame;

   F->Smooth = Smooth;
   F->Scale = Scale;

   F->Bitmap = new FBitmap(Bitmap->Width, Bitmap->Height);
   F->Bitmap->OriginX = Bitmap->OriginX;
   F->Bitmap->OriginY = Bitmap->OriginY;
   memcpy(F->Bitmap->Data, Bitmap->Data, Bitmap->Width*Bitmap->Height*sizeof(Bitmap->Data[0]));

   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      M->TextureIdx = -1;
      for(int t = 0; t < M->Triangles.sizeUsed; t++)
      {
         FTriangle* T = &M->Triangles[t];
         F->Meshes.Add();
         F->Meshes.last->OpaquePixels = M->OpaquePixels;
         int v0 = F->Meshes.last->StoreVertex(M->Vertices[T->v0].x, M->Vertices[T->v0].y);
         int v1 = F->Meshes.last->StoreVertex(M->Vertices[T->v1].x, M->Vertices[T->v1].y);
         int v2 = F->Meshes.last->StoreVertex(M->Vertices[T->v2].x, M->Vertices[T->v2].y);
         F->Meshes.last->StoreTriangle(v0, v1, v2);
         F->MakeMeshes();
      }
   }
   return F;
}
//---------------------------------------------------------------------------
FMesh* FFrame::RepairMesh(void)
{
   FMesh* M1 = new FMesh;

   // - создаем ребра меша
   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      for(int e = 0; e < M->Edges.sizeUsed; e++)
      {
         int v0 = M->Edges[e].v0;
         int v1 = M->Edges[e].v1;

         int x0 = M->Vertices[v0].x;
         int y0 = M->Vertices[v0].y;
         int x1 = M->Vertices[v1].x;
         int y1 = M->Vertices[v1].y;

         // - нуль используем как начальное значение счетчика
         M1->StoreEdge(x0, y0, x1, y1, 0);
      }
   }

   // - создаем треугольники меша
   for(int m = 0; m < Meshes.sizeUsed; m++)
   {
      FMesh* M = Meshes[m];
      for(int t = 0; t < M->Triangles.sizeUsed; t++)
      {
         int v0 = M->Triangles[t].v0;
         int v1 = M->Triangles[t].v1;
         int v2 = M->Triangles[t].v2;

         int x0 = M->Vertices[v0].x;
         int y0 = M->Vertices[v0].y;
         int x1 = M->Vertices[v1].x;
         int y1 = M->Vertices[v1].y;
         int x2 = M->Vertices[v2].x;
         int y2 = M->Vertices[v2].y;

         v0 = M1->SearchVertex(x0, y0);
         v1 = M1->SearchVertex(x1, y1);
         v2 = M1->SearchVertex(x2, y2);

         M1->StoreTriangle(v0, v1, v2);
      }
   }

   // - используем contourId как счетчик соседей
   for(int t = 0; t < M1->Triangles.sizeUsed; t++)
   {
      FTriangle* T = &M1->Triangles[t];
      M1->Edges[T->e0].contourId++;
      M1->Edges[T->e1].contourId++;
      M1->Edges[T->e2].contourId++;
   }

   // - выделяем ребра контура
   for(int e = 0; e < M1->Edges.sizeUsed; e++)
   {
      FEdge* E = &M1->Edges[e];
      if(E->contourId == 1)
         E->contourId = 0;
      else
         E->contourId = -1;
   }
   return M1;
}
//---------------------------------------------------------------------------
bool FFrame::ConstructMesh(void)
{
   FMesh* M = Meshes.last;
   M->MakeTriangles();

   if(M->Edges.sizeUsed < 3)
      return false;

   // - ищем верхнее ребро
   for(int i = 0; i < M->Edges.sizeUsed; i++)
   {
      int v0 = M->Edges[i].v0;
      int v1 = M->Edges[i].v1;
      if((M->Vertices[v0].x == 1 + Bitmap->OriginX &&
          M->Vertices[v0].y == 1 + Bitmap->OriginY &&
          M->Vertices[v1].x == Bitmap->Width-2 + Bitmap->OriginX &&
          M->Vertices[v1].y == 1 + Bitmap->OriginY) ||
         (M->Vertices[v1].x == 1 + Bitmap->OriginX &&
          M->Vertices[v1].y == 1 + Bitmap->OriginY &&
          M->Vertices[v0].x == Bitmap->Width-2 + Bitmap->OriginX &&
          M->Vertices[v0].y == 1 + Bitmap->OriginY))
      {
         // - помечаем все ребра вне контуров
         M->ClearMesh();
         M->MarkEdge(i);
         break;
      }
   }

   // - создание нового меша без лишних ребер
   FMesh* M1 = new FMesh;
   for(int i = 0; i < M->Edges.sizeUsed; i++)
   {
      int v0 = M->Edges[i].v0;
      int v1 = M->Edges[i].v1;

      int x0 = M->Vertices[v0].x;
      int y0 = M->Vertices[v0].y;
      int x1 = M->Vertices[v1].x;
      int y1 = M->Vertices[v1].y;

      if(!M->Edges[i].isMark)
         M1->StoreEdge(x0, y0, x1, y1, M->Edges[i].contourId);
   }
   M1->MakeTriangles();

   delete M;
   M = Meshes.pArray[Meshes.sizeUsed-1] = Meshes.last = M1;

   if(M->Triangles.sizeUsed == 0)
      return false;

   return true;
}
//---------------------------------------------------------------------------
int FFrame::GetZ(int x, int y)
{
   x -= Bitmap->OriginX;
   y -= Bitmap->OriginY;

   if(!(x >= 0 && x < Bitmap->Width && y >= 0 && y < Bitmap->Height))
      return 0;

   const int InitD = 0;
   int D = InitD;

findZ:
   int MaxZ = 0;
   int num = 0;
   for(int j = -D; j <= D; j++)
   {
      for(int i = -D; i <= D; i++)
      {
         int xx = x + i;
         int yy = y + j;

         if(xx >= 0 && xx < Bitmap->Width && yy >= 0 && yy < Bitmap->Height)
         {
            RGBAX& col = Bitmap->GetPixel(xx, yy);
            if(col.a > 100)
            {
               MaxZ += col.z;
               num++;
            }
         }
      }
   }
   if(num <= 4)
   {
      if(D >= Bitmap->Height/8 || D >= Bitmap->Width/8)
         return 0;
      D++;
      goto findZ;
   }

   int top = Smooth+2;
   while(top < Bitmap->Height && Bitmap->GetPixel(Smooth+2, top).z == 0) top++;
   int bottom = top;
   while(bottom < Bitmap->Height && Bitmap->GetPixel(Smooth+2, bottom).z < 255) bottom++;

   if(top < bottom)
      return (float)((float)MaxZ/(float)num)*((float)(bottom-top)/tan(M_PI*30/180))/256.0;

   return MaxZ/num;
}
//---------------------------------------------------------------------------
void FFrame::SubdivideMesh(FMesh* M)
{
/*
   const double MinD = 4;
   const double MinDZ = 4;

   int Passes = 0;
subdB:
   CDT();
   ConstructMesh();
   M = Meshes.last;

   if(Passes++ < 100)
   {
      for(int e = 0; e < M->Edges.sizeUsed; e++)
      {
         if(M->Edges[e].contourId != -1) continue;

         int v0 = M->Edges[e].v0;
         int v1 = M->Edges[e].v1;

         int x0 = M->Vertices[v0].x;
         int y0 = M->Vertices[v0].y;
         int z0 = GetZ(x0, y0);
         int x1 = M->Vertices[v1].x;
         int y1 = M->Vertices[v1].y;
         int z1 = GetZ(x1, y1);

         double dx = x1 - x0;
         double dy = y1 - y0;
         double dz = z1 - z0;

         double D = sqrt(dx*dx + dy*dy + dz*dz);
         if(D >= 3*MinD)
         {
            double kx = dx/D;
            double ky = dy/D;
            double kz = dz/D;

            double x = x0 + kx*MinD;
            double y = y0 + ky*MinD;
            double z = z0 + kz*MinD;

            int max_dz = 0, max_x = 0, max_y = 0;
            int N = (int)(D - 2*MinD);
            for(int i = 0; i < N; i++, x += kx, y += ky, z += kz)
            {
               double zz = GetZ(x + Bitmap->OriginX, y + Bitmap->OriginY);
               double dzz = fabs(z - zz);
               if(dzz > max_dz)
               {
                  max_dz = dzz;
                  max_x = x;
                  max_y = y;
               }
            }
            if(max_dz >= MinDZ)
            {
               UserSites.putint(max_x + Bitmap->OriginX);
               UserSites.putint(max_y + Bitmap->OriginY);
//               goto subdB;
            }
         }
      }
   }
   MakeMeshes();
*/   
}
//---------------------------------------------------------------------------


