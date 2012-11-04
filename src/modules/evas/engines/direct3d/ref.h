#ifndef __REF_H__
#define __REF_H__

//////////////////////////////////////////////////////////////////////////////
// class Referenc
// Desc: Base class enabling reference interface
// Note: Class should derive as virtual
//
class Referenc
{
public:
   Referenc()
      : refs_count(0) {};

   inline int AddRef()
   {
      return ++refs_count;
   }
   inline int RemRef()
   {
      return --refs_count;
   }
   inline int RefCount()
   {
      return refs_count;
   }

private:
   int refs_count;
};


//////////////////////////////////////////////////////////////////////////////
// template Ref
// Desc: Holder in smart-pointers system.
// Important: Only Referenc subclasses may be used as template param.
//

template <class T>
class Ref
{
public:
   // Constructors & destructor
   Ref();
   //Ref(Ref<T> &ref);
   Ref(const Ref<T> &ref);
   Ref(T *ptr);
   Ref(const T *ptr);
   ~Ref();

   Ref<T> &operator =(Ref<T> &ref);
   Ref<T> &operator =(T *ptr);

   inline T *Addr();
   inline T *Addr() const;
   inline int RefCount();
   inline bool IsNull();

   inline T *operator ->();
   inline operator const T *() const;
   inline operator T *();

//private:
   void RemRef();

private:
   T *m_ptr;
};

//////////////////////////////////////////////////////////////////////////////
// Constructors & destructor
template <class T> Ref<T>::Ref()
: m_ptr(NULL)
{
}

//template <class T> Ref<T>::Ref(Ref<T> &ref)
//: m_ptr(NULL)
//{
//   if (ref.Addr() != NULL)
//   {
//      m_ptr = ref.Addr();
//      ((Referenc *)m_ptr)->AddRef();
//   }
//}

template <class T> Ref<T>::Ref(const Ref<T> &ref)
: m_ptr(NULL)
{
   if (ref.Addr() != NULL)
   {
      m_ptr = ref.Addr();
      ((Referenc *)m_ptr)->AddRef();
   }
}

template <class T> Ref<T>::Ref(T *ptr)
: m_ptr(NULL)
{
   if (ptr != NULL)
   {
      m_ptr = ptr;
      ((Referenc *)m_ptr)->AddRef();
   }
}

template <class T> Ref<T>::Ref(const T *ptr)
: m_ptr(NULL)
{
   if (ptr != NULL)
   {
      m_ptr = ptr;
      ((Referenc *)m_ptr)->AddRef();
   }
}

template <class T> Ref<T>::~Ref()
{
   if (m_ptr == NULL)
      return;
   RemRef();
}

// Check pointer on correctness
template <class T> bool Ref<T>::IsNull()
{
   return (m_ptr == NULL);
}

//////////////////////////////////////////////////////////////////////////////
// Operators

template <class T> Ref<T> &Ref<T>::operator =(T *ptr)
{
   if (ptr != NULL)
   {
      if (m_ptr != ptr)
      {
         RemRef();
         m_ptr = ptr;
         ((Referenc *)m_ptr)->AddRef();
      }
   }
   else if (m_ptr != NULL)
      RemRef();
   return *this;
}

template <class T> Ref<T> &Ref<T>::operator =(Ref<T> &ref)
{
   if (ref.Addr() != NULL)
   {
      if (m_ptr != ref.Addr())
      {
         RemRef();
         m_ptr = ref.Addr();
         ((Referenc *)m_ptr)->AddRef();
      }
   }
   else if (m_ptr != NULL)
      RemRef();
   return *this;
}

// Get pointer
template <class T> T *Ref<T>::Addr()
{
   return m_ptr;
}

template <class T> T *Ref<T>::Addr() const
{
   return m_ptr;
}

// Get refs count
template <class T> int Ref<T>::RefCount()
{
   if (m_ptr == NULL)
      return 0;
   return ((Referenc *)m_ptr)->RefCount();
}

// Remove ref to the object and delete it if necessary
// WARNING: arrays cannot be deleted
template <class T> void Ref<T>::RemRef()
{
   if (m_ptr == NULL)
      return;
   if (((Referenc *)m_ptr)->RemRef() == 0)
      delete m_ptr;
   m_ptr = NULL;
}

template <class T> T *Ref<T>::operator ->()
{
   return m_ptr;
}

template <class T> Ref<T>::operator const T *() const
{
   return m_ptr;
}

template <class T> Ref<T>::operator T *()
{
   return m_ptr;
}

#endif  // __REF_H__
