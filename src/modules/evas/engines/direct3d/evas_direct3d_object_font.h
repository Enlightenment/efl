#ifndef __EVAS_DIRECT3D_OBJECT_FONT_H__
#define __EVAS_DIRECT3D_OBJECT_FONT_H__

#include "evas_engine.h"

#include "ref.h"
#include "array.h"

#include "evas_direct3d_object.h"

class D3DObjectFont : public D3DObject
{
public:

   class Glyph : public Referenc
   {
   public:
      Glyph(void *source)
         : _source(source), _width(0), _height(0) {};

      bool Compare(void *source)
      {
         return (_source == source);
      }
      void *Source()
      {
         return _source;
      }

   private:
      friend class D3DObjectFont;
      void *_source;
      TArray<BYTE> _data;
      int _width;
      int _height;
   };

public:
   D3DObjectFont(void *source, int image_id);
   ~D3DObjectFont();

   inline bool Compare(void *source);
   void CopyTo(D3DObjectFont *font);

   virtual void Draw(D3DDevice *d3d);
   static void EndCache(D3DDevice *d3d);

   inline void SetColor(DWORD color);

   Glyph *GetGlyph(void *source);
   Glyph *AddGlyph(D3DDevice *d3d, void *source, BYTE *data8, int width, int height, int pitch);
   void PushForDraw(Glyph *glyph, int x, int y);

protected:
   static void BeginCache(int image_id);

private:
   struct Vertex
   {
      FLOAT x, y;
      FLOAT u, v;
   };

   struct Color
   {
      union
      {
         struct
         {
            BYTE b, g, r, a;
         };
         DWORD color;
      };

      FLOAT Alpha() { return FLOAT(a) / 255.f; }
   };

   class Cache
   {
   public:
      Cache()
         : enabled(false), image_id(-1), width(0), height(0) {};
   public:
      TArray<DWORD> data;
      TArray<POINT> dirty;  // Start, End
      bool enabled;
      int image_id;
      int width;
      int height;
      RECT valid_rect;
   };

private:
   DWORD _color;
   void *_source;
   int _image_id;

   TArray<Ref<Glyph> > _glyphs;

   static Cache _cache;
};

bool D3DObjectFont::Compare(void *source)
{
   return (_source == source);
}

void D3DObjectFont::SetColor(DWORD color)
{
   _color = color;
}

#endif  // __EVAS_DIRECT3D_OBJECT_FONT_H__
