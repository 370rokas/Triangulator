#ifndef _STOREARRAY_H
#define _STOREARRAY_H

// - динамический массив с хранением непосредственно объектов

#include "FMM\\FMM.h"
#include <memory.h>
#include <stdio.h>
#include <assert.h>

template <class T>
class storeArray {
//-----------------------------------------------------------------------------------
   int curDelta;
public:
   int sizeUsed;
   int sizeFull;
   T* last;
   T* pArray;
   //-----------------------------------------------------------------------
   storeArray()
   {
      curDelta = 128;
      sizeUsed = 0;
      sizeFull = 0;
      last = NULL;
      pArray = NULL;
   }
   //-----------------------------------------------------------------------
   ~storeArray()
   {
      RemoveAll();
   }
   //-----------------------------------------------------------------------
   T& operator[](int pos) const
   {
      if(!(pos < sizeUsed && pos >= 0))
         assert(pos < sizeUsed && pos >= 0);
      return pArray[pos];
   }
   //-----------------------------------------------------------------------
   bool Reset(int Size, int Delta)
   {
      RemoveAll();
      curDelta = Delta;
      return Expand(Size);
   }
   //-----------------------------------------------------------------------
   bool Expand(void)
   {
	   if(sizeUsed >= sizeFull)
         return Expand(curDelta + 1);
      return true;
   }
   //-----------------------------------------------------------------------
   bool Expand(unsigned delta)
   {
  		int newSize = sizeFull + delta;
  		T* p = new T[newSize];
  		if(!p) return false;
  		memcpy(p, pArray, sizeof(T)*sizeUsed);
  		delete[] (void*)pArray;
  		pArray = p;
  		sizeFull = newSize;
      return true;
   }
   //-----------------------------------------------------------------------
   T* Add(void)
   {
      if(!Expand()) return NULL;
		last = &pArray[sizeUsed++];
   	return last;
   }
   //-----------------------------------------------------------------------
   T* Add(T* pointer)
   {
      if(!Expand()) return NULL;
		last = &pArray[sizeUsed++];
      memcpy(last, pointer, sizeof(T));
   	return last;
   }
   //-----------------------------------------------------------------------
   T* GetAt(int pos)
   {
      if(pos < 0 || sizeUsed <= pos) return NULL;
      return &pArray[pos];
   }
   //-----------------------------------------------------------------------
   bool InsertAt(int pos, T *pointer)
   {
      int i;
      if(pos < sizeUsed)
      {
         if(!Expand()) return false;

         for(i = sizeUsed; i > pos; i--)
   		  memcpy((pArray+i), (pArray+i-1), sizeof(T));

         memcpy(&pArray[pos], pointer, sizeof(T));
         sizeUsed++;
      }
      else
      if(pos == sizeUsed)
      {
         return Add(pointer);
      }
      else return false;
      return true;
   }
   //-----------------------------------------------------------------------
   bool Remove(int pos)
   {
      if(pos < 0 || sizeUsed <= pos) return false;

		memmove(&pArray[pos], &pArray[pos+1], sizeof(T)*(sizeUsed-pos));

      sizeUsed--;
      return true;
   }
   //-----------------------------------------------------------------------
   bool Remove(T *pointer)
   {
      int pos = Find(pointer, 0);
      if(pos == -1) return false;

      return Remove(pos);
   }
   //-----------------------------------------------------------------------
   void RemoveAll()
   {
      if(pArray) delete[] pArray;
      pArray = NULL;
      sizeFull = 0;
      sizeUsed = 0;
   }
   //-----------------------------------------------------------------------
   int Find(T *pointer, int first = 0)
   {
	   for(int i = first; i < sizeUsed; i++)
   	{
   		if(memcmp(&pArray[i], pointer, sizeof(T)) == 0) return i;
   	}
   	return -1;
   }
   //-----------------------------------------------------------------------
};
#endif
