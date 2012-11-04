#ifndef __ARRAY_H__
#define __ARRAY_H__

#include "ref.h"
#include <assert.h>

template <class T>
class TArray : virtual public Referenc
{
public:
   TArray();
   TArray(const TArray<T> &arr)
   {
      data = NULL;
      size = num = 0;
      block_size = arr.block_size;
      keep_order = arr.keep_order;

      //assert(0 && "Direct assignment for arrays is NOT allowed");
      // risky probably, but anyway
      arr.CopyTo(*this);
   }
   ~TArray();

   bool Allocate(int new_num);
   bool Resize(int new_size = 0);
   bool Add(T &el);
   bool Add(const T &el);

   inline T &operator[](int i);
   inline const T &operator[](int i) const;
   inline const TArray<T> &operator =(const TArray<T> &arr)
   {
      block_size = arr.block_size;
      keep_order = arr.keep_order;

      //assert(0 && "Direct assignment for arrays is NOT allowed");
      // risky probably, but anyway
      arr.CopyTo(*this);
      return *this;
   }

   T *Last()
   {
      if (num > 0)
         return &data[num - 1];
      return NULL;
   }

   inline int Length() const
   {
      return num;
   }

   inline int Size() const
   {
      return size;
   }

   inline int BlockSize() const
   {
      return block_size;
   }

   inline T *Data()
   {
      return data;
   }

   inline T **DataPtr()
   {
      return &data;
   }

   inline const T *Data() const
   {
      return data;
   }

   inline void SetKeepOrder(bool enable)
   {
      keep_order = enable;
   }

   bool Find(const T &el);

   bool Add(TArray<T> &arr);
   bool CopyTo(TArray<T> &dest) const;
   bool Init(const T *arr, int len);

   void Swap(int to, int from);
   void Replace(int i);

   bool SetBlockSize(int new_size);
   void Set(T &el);
   void Set(const T &el);

protected:
   T     *data;
   int   size;
   int   num;
   int   block_size;
   // Some operations restricted, order of the elements is fixed
   bool  keep_order;

};

namespace Array
{
   const int default_block_size = 16;
   const int max_array_size = 0xffffff;
}


template <class T> TArray<T>::TArray()
: data(NULL), size(0), num(0), block_size(Array::default_block_size), keep_order(false)
{
}

template <class T> TArray<T>::~TArray()
{
   if (data != NULL)
      Resize();
}

template <class T> bool TArray<T>::Allocate(int new_num)
{
   assert(new_num >= 0 && new_num <= Array::max_array_size);
   if (new_num > size)
   {
      if (!Resize(new_num))
         return false;
   }
   num = new_num;
   return true;
}

template <class T> bool TArray<T>::Resize(int new_size)
{
   assert(new_size >= 0 && new_size <= Array::max_array_size);
   if (new_size == 0)
   {
      delete[] data;
      data = NULL;
      size = 0;
      num = 0;
      return true;
   }
   if (new_size == size)
      return true;

   T *new_data = new T[new_size];
   if (new_data == NULL)
      return false;

   if (data != NULL && num > 0)
   {
      //CopyMemory(new_data, data, num * sizeof(T));
      for (int i = 0; i < num && i < new_size; i++)
         new_data[i] = data[i];
   }
   delete[] data;

   data = new_data;
   size = new_size;
   return true;
}

template <class T> bool TArray<T>::Add(T &el)
{
   if (data == NULL)
      Resize(1);

   if (num < size)
   {
      data[num++] = el;
      return true;
   }
   // num >= size
   int new_size = size + block_size;
   if (!Resize(new_size))
      return false;

   data[num++] = el;
   return true;
}

template <class T> bool TArray<T>::Add(const T &el)
{
   if (data == NULL)
      Resize(1);

   if (num < size)
   {
      data[num++] = *(T *)&el;
      return true;
   }
   // num >= size
   int new_size = size + block_size;
   if (!Resize(new_size))
      return false;

   data[num++] = *(T *)&el;
   return true;
}

template <class T> bool TArray<T>::Add(TArray<T> &arr)
{
   if (arr.Length() == 0)
      return true;
   int numf = num;
   if (!Allocate(Length() + arr.Length()))
      return false;
   CopyMemory(&data[numf], arr.Data(), arr.Length() * sizeof(T));
   return true;
}

template <class T> T &TArray<T>::operator [](int i)
{
   assert(i >= 0 && i < num);
   return data[i];
}

template <class T> const T &TArray<T>::operator [](int i) const
{
   assert(i >= 0 && i < num);
   return data[i];
}

template <class T> bool TArray<T>::SetBlockSize(int new_size)
{
   assert(new_size >= 0 && new_size <= Array::max_array_size);
   block_size = new_size;
   return true;
}

template <class T> void TArray<T>::Set(T &el)
{
   for (int i = 0; i < num; i++)
      data[i] = el;
}

template <class T> void TArray<T>::Set(const T &el)
{
   for (int i = 0; i < num; i++)
      data[i] = el;
}

template <class T> bool TArray<T>::CopyTo(TArray<T> &dest) const
{
   if (!dest.Resize(size))
      return false;
   dest.num = 0;
   for (int i = 0; i < num; i++)
      dest.Add(data[i]);

   return true;
}

template <class T> bool TArray<T>::Init(const T *arr, int len)
{
   assert(arr != NULL);
   if (!Resize(len))
      return false;
   num = 0;
   for (int i = 0; i < len; i++)
      Add((T)arr[i]);

   return true;
}

template <class T> void TArray<T>::Swap(int to, int from)
{
   assert(to >= 0 && to < num && from >= 0 && from < num);
   if (keep_order)
      return;
   T t = data[to];
   data[to] = data[from];
   data[from] = t;
}

template <class T> void TArray<T>::Replace(int i)
{
   assert(i >= 0 && i < num);
   if (keep_order)
      return;
   if (num >= 1)
   {
      data[i] = data[num - 1];
      num--;
   }
}

// operator == for type T should be defined
template <class T> bool TArray<T>::Find(const T &el)
{
   for (int i = 0; i < num; i++)
   {
      if (data[i] == el)
         return true;
   }
   return false;
}

#endif  // __ARRAY_H__
