// Force the layered windows APIs to be visible.
#define _WIN32_WINNT 0x0500

#include "evas_engine.h"
#include <assert.h>

#include <d3dx9.h>

#include "evas_direct3d_device.h"
#include "evas_direct3d_context.h"
#include "evas_direct3d_shader_pack.h"
#include "evas_direct3d_scene.h"
#include "evas_direct3d_image_cache.h"
#include "evas_direct3d_object_line.h"
#include "evas_direct3d_object_rect.h"
#include "evas_direct3d_object_image.h"
#include "evas_direct3d_vertex_buffer_cache.h"
#include "evas_direct3d_object_font.h"

// Internal structure that joins two types of objects
struct ImagePtr
{
   Ref<D3DObjectImage> ref;
   RGBA_Image *img;
};

struct DevicePtr
{
   Ref<D3DDevice> device;
   Ref<D3DScene> scene;
   Ref<D3DContext> context;
   Ref<D3DImageCache> image_cache;
   Ref<D3DShaderPack> shader_pack;
   Ref<D3DVertexBufferCache> vb_cache;

   int fonts_buffer_image_id;

   // Layered windows cannot render D3D in the normal way
   bool layered;

   // Window shape mask
   struct
   {
      // Width and height may be different from target size
      int width;
      int height;
      // Pointer to external memory location, dont do anything with it
      unsigned char *mask;
   } shape;

   // GDI output target
   struct
   {
      HBITMAP image;
      HDC hdc;
      BITMAPINFO info;
      BYTE *data;
   } dib;

};

DevicePtr *SelectDevice(Direct3DDeviceHandler d3d)
{
   DevicePtr *dev_ptr = (DevicePtr *)d3d;
   D3DImageCache::SetCurrent(dev_ptr->image_cache);
   D3DShaderPack::SetCurrent(dev_ptr->shader_pack);
   D3DVertexBufferCache::SetCurrent(dev_ptr->vb_cache);
   return dev_ptr;
}

void DeleteDIBObjects(DevicePtr *dev_ptr)
{
   assert(dev_ptr != NULL);
   if (dev_ptr->dib.image != NULL)
      DeleteObject(dev_ptr->dib.image);
   if (dev_ptr->dib.hdc != NULL)
      DeleteDC(dev_ptr->dib.hdc);
   ZeroMemory(&dev_ptr->dib, sizeof(dev_ptr->dib));
}

bool CreateDIBObjects(DevicePtr *dev_ptr)
{
   assert(dev_ptr != NULL);
   if ((dev_ptr->dib.hdc = CreateCompatibleDC(NULL)) == NULL)
   {
      Log("Failed to create compatible DC");
      return false;
   }
   ZeroMemory(&dev_ptr->dib.info, sizeof(dev_ptr->dib.info));
   dev_ptr->dib.info.bmiHeader.biSize = sizeof(dev_ptr->dib.info.bmiHeader);
   dev_ptr->dib.info.bmiHeader.biBitCount = 32;
   dev_ptr->dib.info.bmiHeader.biWidth = dev_ptr->device->GetWidth();
   dev_ptr->dib.info.bmiHeader.biHeight = -dev_ptr->device->GetHeight();
   dev_ptr->dib.info.bmiHeader.biCompression = BI_RGB;
   dev_ptr->dib.info.bmiHeader.biPlanes = 1;
   if ((dev_ptr->dib.image = CreateDIBSection(dev_ptr->dib.hdc, &dev_ptr->dib.info,
      DIB_RGB_COLORS, (void **)&dev_ptr->dib.data, NULL, 0)) == NULL)
   {
      Log("Failed to create dib section");
      DeleteDIBObjects(dev_ptr);
      return false;
   }
   assert(dev_ptr->dib.data != NULL);
   GdiFlush();
   return true;
}




extern "C" {

Direct3DDeviceHandler evas_direct3d_init(HWND window, int depth, int fullscreen)
{
   Ref<D3DDevice> device = new D3DDevice();
   if (!device->Init(window, depth, fullscreen == 1))
      return NULL;

   D3DImageCache::SetCurrent(NULL);
   D3DShaderPack::SetCurrent(NULL);
   D3DVertexBufferCache::SetCurrent(NULL);

   if (!D3DShaderPack::Current()->Initialize(device))
   {
      Log("Failed to build shader pack");
      device->Destroy();
      return NULL;
   }

   DevicePtr *dev_ptr = new DevicePtr;
   ZeroMemory(dev_ptr, sizeof(DevicePtr));
   dev_ptr->device = device;
   dev_ptr->scene = new D3DScene();
   dev_ptr->context = new D3DContext();
   dev_ptr->image_cache = D3DImageCache::Current();
   dev_ptr->shader_pack = D3DShaderPack::Current();
   dev_ptr->vb_cache = D3DVertexBufferCache::Current();
   dev_ptr->fonts_buffer_image_id = -1;

   D3DImageCache::CacheEntryInfo info;
   if (!D3DImageCache::Current()->CreateImage(device, device->GetWidth(), device->GetHeight(),
      true, info))
   {
      Log("Failed to create fonts image buffer");
      return NULL;
   }
   dev_ptr->fonts_buffer_image_id = info.id;
   D3DImageCache::Current()->SetImageStage(info.id, 1);

   return (Direct3DDeviceHandler)dev_ptr;
}

void
evas_direct3d_free(Direct3DDeviceHandler d3d)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);

   DeleteDIBObjects(dev_ptr);

   dev_ptr->context = NULL;
   dev_ptr->scene = NULL;
   dev_ptr->image_cache = NULL;
   dev_ptr->shader_pack = NULL;
   D3DShaderPack::Current()->Uninitialize();
   D3DImageCache::Current()->Uninitialize();
   D3DVertexBufferCache::Current()->Uninitialize();
   D3DShaderPack::SetCurrent(NULL);
   D3DImageCache::SetCurrent(NULL);
   D3DVertexBufferCache::SetCurrent(NULL);

   dev_ptr->device = NULL;
   delete dev_ptr;

   Log("uninitialized");
}

void
evas_direct3d_resize(Direct3DDeviceHandler d3d, int width, int height)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   if (!device->Reset(width, height, -1))
   {
      Log("Failed to resize");
      return;
   }
   if (!D3DImageCache::Current()->ResizeImage(device, width, height,
      dev_ptr->fonts_buffer_image_id))
   {
      Log("Failed to resize fonts image buffer");
   }
   if (dev_ptr->layered)
   {
      DeleteDIBObjects(dev_ptr);
      if (!CreateDIBObjects(dev_ptr))
         Log("Failed to create dib objects");
   }
}

void
evas_direct3d_set_fullscreen(Direct3DDeviceHandler d3d, int width, int height, int fullscreen)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;

   if (width < 0)
      width = ::GetSystemMetrics(SM_CXSCREEN);
   if (height < 0)
      height = ::GetSystemMetrics(SM_CYSCREEN);

   if (!device->Reset(width, height, fullscreen))
   {
      Log("Failed to resize");
      return;
   }
   if (!D3DImageCache::Current()->ResizeImage(device, width, height,
      dev_ptr->fonts_buffer_image_id))
   {
      Log("Failed to resize fonts image buffer");
   }

   if (fullscreen == 0)
      InvalidateRect(HWND_DESKTOP, NULL, TRUE);
}

void
evas_direct3d_set_layered(Direct3DDeviceHandler d3d, int layered,
   int mask_width, int mask_height, unsigned char *mask)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   dev_ptr->layered = (layered != 0);
   dev_ptr->shape.width = mask_width;
   dev_ptr->shape.height = mask_height;
   dev_ptr->shape.mask = mask;

   if (dev_ptr->layered && dev_ptr->dib.data == NULL)
      CreateDIBObjects(dev_ptr);
   else if (!dev_ptr->layered)
      DeleteDIBObjects(dev_ptr);
}

void
evas_direct3d_context_color_set(Direct3DDeviceHandler d3d, int r, int g, int b, int a)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   dev_ptr->context->color = ((a & 0xff) << 24) | ((r & 0xff) << 16) |
      ((g & 0xff) << 8) | (b & 0xff);
}

void
evas_direct3d_context_set_multiplier(Direct3DDeviceHandler d3d, int r, int g, int b, int a)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   dev_ptr->context->color_mul = ((a & 0xff) << 24) | ((r & 0xff) << 16) |
      ((g & 0xff) << 8) | (b & 0xff);
}

void
evas_direct3d_render_all(Direct3DDeviceHandler d3d)
{
   Log("render");
   assert(d3d != NULL);
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   D3DScene *scene = dev_ptr->scene;

   if (!device->Begin())
      return;

   device->GetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
   device->GetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
   device->GetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

   //device->GetDevice()->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
   //device->GetDevice()->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);

   D3DObjectLine::BeginCache();
   D3DObjectRect::BeginCache();
   D3DObjectImage::BeginCache();
   scene->DrawAll(device);
   D3DObjectLine::EndCache(device);
   D3DObjectRect::EndCache(device);
   D3DObjectImage::EndCache(device);
   D3DObjectFont::EndCache(device);

   device->End();

   if (dev_ptr->layered && !device->GetFullscreen() && dev_ptr->dib.data != NULL)
   {
      HDC hdc = GetDC(device->GetWindow());
      if (hdc != NULL)
      {
         POINT dest = {0, 0};
         POINT src = {0, 0};
         SIZE client = {device->GetWidth(), device->GetHeight()};
         BLENDFUNCTION blend_func = {AC_SRC_OVER, 0, 255, AC_SRC_ALPHA};

         if (device->GetRenderData().Length() == client.cx * client.cy)
         {
            CopyMemory(dev_ptr->dib.data, device->GetRenderData().Data(),
               sizeof(DWORD) * client.cx * client.cy);
         }

         for (int i = 0; i < client.cy; i++)
         {
            for (int j = 0; j < client.cx; j++)
            {
               int mask_i = int(dev_ptr->shape.height * float(i) / float(client.cy));
               int mask_j = int(dev_ptr->shape.width * float(j) / float(client.cx));
               if (mask_i < 0)
                  mask_i = 0;
               else if (mask_i >= dev_ptr->shape.height)
                  mask_i = dev_ptr->shape.height - 1;
               if (mask_j < 0)
                  mask_j = 0;
               else if (mask_j >= dev_ptr->shape.width)
                  mask_j = dev_ptr->shape.width - 1;
               BYTE mask_b = dev_ptr->shape.mask[mask_i * dev_ptr->shape.width + mask_j];
               float alpha = float(mask_b) / 255.f;

               dev_ptr->dib.data[j * 4 + 0 + i * 4 * client.cx] = BYTE(float(dev_ptr->dib.data[j * 4 + 0 + i * 4 * client.cx]) * alpha);
               dev_ptr->dib.data[j * 4 + 1 + i * 4 * client.cx] = BYTE(float(dev_ptr->dib.data[j * 4 + 1 + i * 4 * client.cx]) * alpha);
               dev_ptr->dib.data[j * 4 + 2 + i * 4 * client.cx] = BYTE(float(dev_ptr->dib.data[j * 4 + 2 + i * 4 * client.cx]) * alpha);
               dev_ptr->dib.data[j * 4 + 3 + i * 4 * client.cx] = mask_b;
            }
         }
         HGDIOBJ prev_obj = SelectObject(dev_ptr->dib.hdc, dev_ptr->dib.image);
         ClientToScreen(device->GetWindow(), &dest);

         UpdateLayeredWindow(device->GetWindow(), hdc, &dest, &client,
            dev_ptr->dib.hdc, &src, 0, &blend_func, ULW_ALPHA);

         SelectObject(dev_ptr->dib.hdc, prev_obj);
         ReleaseDC(device->GetWindow(), hdc);
      }
   }
   scene->FreeObjects();
}

void evas_direct3d_line_draw(Direct3DDeviceHandler d3d, int x1, int y1, int x2, int y2)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   D3DScene *scene = dev_ptr->scene;
   D3DContext *context = dev_ptr->context;

   Ref<D3DObjectLine> line = scene->GetFreeObject<D3DObjectLine>();
   if (line == NULL)
   {
      line = new D3DObjectLine();
      scene->AddObject(line);
      Log("New line object (total objects: %d)", scene->GetObjectCount());
   }
   else
   {
      line->SetFree(false);
      Log("Line reused (object: %p)", line.Addr());
   }

   line->Setup(
      2.f * float(x1) / float(device->GetWidth()) - 1.f,
      2.f * (1.f - float(y1) / float(device->GetHeight())) - 1.f,
      2.f * float(x2) / float(device->GetWidth()) - 1.f,
      2.f * (1.f - float(y2) / float(device->GetHeight())) - 1.f,
      context->color);
}

void evas_direct3d_rectangle_draw(Direct3DDeviceHandler d3d, int x, int y, int w, int h)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   D3DScene *scene = dev_ptr->scene;
   D3DContext *context = dev_ptr->context;

   Ref<D3DObjectRect> rect = scene->GetFreeObject<D3DObjectRect>();
   if (rect == NULL)
   {
      rect = new D3DObjectRect();
      scene->AddObject(rect);
      Log("New rect object (total objects: %d)", scene->GetObjectCount());
   }
   else
   {
      rect->SetFree(false);
      Log("Rect reused (object: %p)", rect.Addr());
   }

   rect->Setup(
      2.f * float(x) / float(device->GetWidth()) - 1.f,
      2.f * (1.f - float(y) / float(device->GetHeight())) - 1.f,
      2.f * float(w) / float(device->GetWidth()),
      -2.f * float(h) / float(device->GetHeight()),
      context->color);
}

Direct3DImageHandler evas_direct3d_image_load(Direct3DDeviceHandler d3d,
   const char *file, const char *key, int *error, Evas_Image_Load_Opts *lo)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   D3DScene *scene = dev_ptr->scene;

   RGBA_Image *evas_image = evas_common_load_image_from_file(file, key, lo);
   if (evas_image == NULL)
   {
      Log("Failed to load image from %s", file);
      return NULL;
   }
   int image_width = evas_image->cache_entry.w;
   int image_height = evas_image->cache_entry.h;
   assert(image_width > 0 && image_height > 0);

   D3DImageCache::CacheEntryInfo info;
   ZeroMemory(&info, sizeof(info));
   info.id = -1;

   Ref<D3DObjectImage> image = new D3DObjectImage();
   image->Init(info.u, info.v, info.du, info.dv, info.id,
      info.width, info.height, evas_image->cache_entry.file);
   image->SetFree(true);
   scene->AddObject(image);

   Log("New image object (total objects: %d)", scene->GetObjectCount());

   ImagePtr *ptr = new ImagePtr;
   ptr->ref = image;
   ptr->img = evas_image;

   return (Direct3DImageHandler)ptr;
}

Direct3DImageHandler evas_direct3d_image_new_from_data(Direct3DDeviceHandler d3d,
   int w, int h, DWORD *image_data, int alpha, int cspace)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   D3DScene *scene = dev_ptr->scene;

   int image_width = w;
   int image_height = h;
   assert(image_width > 0 && image_height > 0);

   Ref<D3DObjectImage> image = new D3DObjectImage();

   D3DImageCache::CacheEntryInfo info;
   ZeroMemory(&info, sizeof(info));
   if (!D3DImageCache::Current()->InsertImage(device, image_data,
      image_width, image_height, info))
   {
      Log("Couldnt add image to the cache");
      return NULL;
   }
   char buf[64];
   sprintf(buf, "%p", image_data);
   image->Init(info.u, info.v, info.du, info.dv, info.id,
      info.width, info.height, buf);

   image->SetFree(true);
   scene->AddObject(image);

   Log("New image object (total objects: %d)", scene->GetObjectCount());

   ImagePtr *ptr = new ImagePtr;
   ptr->ref = image;
   ptr->img = NULL;

   return (Direct3DImageHandler)ptr;
}

Direct3DImageHandler evas_direct3d_image_new_from_copied_data(Direct3DDeviceHandler d3d,
   int w, int h, DWORD *image_data, int alpha, int cspace)
{
   return evas_direct3d_image_new_from_data(d3d, w, h, image_data, alpha, cspace);
}

void evas_direct3d_image_free(Direct3DDeviceHandler d3d, Direct3DImageHandler image)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   D3DScene *scene = dev_ptr->scene;

   ImagePtr *ptr = (ImagePtr *)image;

   Ref<D3DObjectImage> image_ref = ptr->ref;
   assert(!image_ref.IsNull());
   scene->DeleteObject(image_ref);

   delete ptr;
}

void evas_direct3d_image_data_put(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   DWORD *image_data)
{
   ImagePtr *ptr = (ImagePtr *)image;
   Ref<D3DObjectImage> image_ref = ptr->ref;
   //assert(!image_ref.IsNull());
   if (image_ref.IsNull())
      return;

   if (!image_ref->UpdateImageData(image_data))
      Log("Failed to update image data");
}

void evas_direct3d_image_data_get(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   int to_write, DATA32 **image_data)
{
   ImagePtr *ptr = (ImagePtr *)image;
   Ref<D3DObjectImage> image_ref = ptr->ref;
   if (image_ref.IsNull())
      return;
   if (image_data == NULL)
      return;
   assert(sizeof(DATA32) == sizeof(DWORD));
   *image_data = (DATA32 *)image_ref->GetImageData();
}

void evas_direct3d_image_draw(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   int src_x, int src_y, int src_w, int src_h,
   int dst_x, int dst_y, int dst_w, int dst_h, int smooth)
{
   ImagePtr *ptr = (ImagePtr *)image;
   Ref<D3DObjectImage> image_ref = ptr->ref;
   RGBA_Image *evas_image = ptr->img;
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   D3DScene *scene = dev_ptr->scene;
   //assert(!image_ref.IsNull());
   if (image_ref.IsNull())
      return;

   if (!image_ref->IsValid())
   {
      TArray<D3DObjectImage *> images;
      scene->GetObjectsOfType<D3DObjectImage>(images);
      bool found = false;
      for (int i = 0; i < images.Length(); i++)
      {
         if (images[i]->IsValid() &&
            _stricmp(images[i]->GetSource(), image_ref->GetSource()) == 0)
         {
            images[i]->CopyTo(image_ref);
            found = true;
            Log("Image object info reused, source: \"%s\"", image_ref->GetSource());
            break;
         }
      }
      if (!found && evas_image != NULL)
         evas_cache_image_load_data(&evas_image->cache_entry);
   }

   // If the image object wasnt initialized yet
   if (evas_image != NULL && evas_image->image.data != NULL && !image_ref->IsValid())
   {
      D3DImageCache::CacheEntryInfo info;
      ZeroMemory(&info, sizeof(info));
      if (!D3DImageCache::Current()->InsertImage(device, (DWORD *)evas_image->image.data,
         evas_image->cache_entry.w, evas_image->cache_entry.h, info))
      {
         Log("Couldnt add image to the cache");
         return;
      }
      image_ref->Init(info.u, info.v, info.du, info.dv, info.id,
         info.width, info.height, evas_image->cache_entry.file);
   }

   // Do not draw invalid objects
   if (!image_ref->IsValid())
   {
      image_ref->SetFree(true);
      return;
   }

   image_ref->Setup(
      2.f * float(dst_x) / float(device->GetWidth()) - 1.f,
      2.f * (1.f - float(dst_y) / float(device->GetHeight())) - 1.f,
      2.f * float(dst_w) / float(device->GetWidth()),
      -2.f * float(dst_h) / float(device->GetHeight()),
      src_x, src_y, src_w, src_h);

   image_ref->SetupColorFilter(dev_ptr->context->color_mul);

   image_ref->SetFree(false);
}

void evas_direct3d_image_size_get(Direct3DImageHandler image, int *w, int *h)
{
   ImagePtr *ptr = (ImagePtr *)image;
   if (ptr == NULL)
      return;
   if (ptr->img != NULL)
   {
      if (w != NULL)
         *w = ptr->img->cache_entry.w;
      if (h != NULL)
         *h = ptr->img->cache_entry.h;
   }
   else if (!ptr->ref.IsNull())
   {
      if (w != NULL)
         *w = ptr->ref->GetWidth();
      if (h != NULL)
         *h = ptr->ref->GetHeight();
   }
}

void evas_direct3d_image_border_set(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   int l, int r, int t, int b)
{
   ImagePtr *ptr = (ImagePtr *)image;
   Ref<D3DObjectImage> image_ref = ptr->ref;
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   if (image_ref.IsNull())
      return;

   int im_w, im_h;
   if (ptr->img != NULL)
   {
      im_w = ptr->img->cache_entry.w;
      im_h = ptr->img->cache_entry.h;
   }
   else
   {
      im_w = image_ref->GetWidth();
      im_h = image_ref->GetHeight();
   }

   image_ref->SetupBorder(
      D3DXVECTOR4(
         2.f * float(l) / float(device->GetWidth()),
         -2.f * float(t) / float(device->GetHeight()),
         2.f * float(r) / float(device->GetWidth()),
         -2.f * float(b) / float(device->GetHeight())),
      D3DXVECTOR4(float(l) / float(im_w),
         float(t) / float(im_h),
         float(r) / float(im_w),
         float(b) / float(im_h)));
}

void evas_direct3d_image_border_get(Direct3DDeviceHandler d3d, Direct3DImageHandler image,
   int *l, int *r, int *t, int *b)
{
   ImagePtr *ptr = (ImagePtr *)image;
   Ref<D3DObjectImage> image_ref = ptr->ref;
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DDevice *device = dev_ptr->device;
   if (image_ref.IsNull())
      return;
   assert(l != NULL && r != NULL && b != NULL && t != NULL);
   *l = (int)(0.5f * image_ref->GetBorderLeft() * device->GetWidth());
   *r = (int)(0.5f * image_ref->GetBorderRight() * device->GetWidth());
   *t = (int)(-0.5f * image_ref->GetBorderTop() * device->GetHeight());
   *b = (int)(-0.5f * image_ref->GetBorderBottom() * device->GetHeight());
}


Direct3DFontGlyphHandler evas_direct3d_font_texture_new(Direct3DDeviceHandler d3d,
   RGBA_Font_Glyph *fg)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DContext *context = dev_ptr->context;
   D3DDevice *device = dev_ptr->device;

   if (context->font.IsNull())
      return NULL;

   D3DObjectFont *font = (D3DObjectFont *)context->font.Addr();
   // This is not reliable
   //D3DObjectFont::Glyph *glyph = (D3DObjectFont::Glyph *)fg->ext_dat;
   D3DObjectFont::Glyph *glyph = font->GetGlyph(fg);
   if (glyph != NULL)
   {
      assert(glyph->Compare(fg));
      return glyph;
   }
   glyph = font->AddGlyph(device, fg, fg->glyph_out->bitmap.buffer,
      fg->glyph_out->bitmap.width, fg->glyph_out->bitmap.rows, fg->glyph_out->bitmap.pitch);
   return (Direct3DFontGlyphHandler)glyph;
}

void evas_direct3d_font_texture_free(Direct3DFontGlyphHandler ft)
{
   if (ft == NULL)
      return;
   D3DObjectFont::Glyph *glyph = (D3DObjectFont::Glyph *)ft;
   RGBA_Font_Glyph *fg = (RGBA_Font_Glyph *)glyph->Source();
   fg->ext_dat = NULL;
}

void evas_direct3d_font_texture_draw(Direct3DDeviceHandler d3d, void *, void *,
   RGBA_Font_Glyph *fg, int x, int y)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DContext *context = dev_ptr->context;
   D3DDevice *device = dev_ptr->device;

   if (context->font.IsNull())
      return;

   D3DObjectFont *font = (D3DObjectFont *)context->font.Addr();
   D3DObjectFont::Glyph *glyph = (D3DObjectFont::Glyph *)fg->ext_dat;
   if (glyph == NULL)
      return;
   assert(glyph->Compare(fg));

   font->SetColor(context->color);
   font->PushForDraw(glyph, x, y);
   font->SetFree(false);
}

void evas_direct3d_select_or_create_font(Direct3DDeviceHandler d3d, void *font)
{
   // The Plan
   // 1. Create D3DObjectFont with source string = "%p" (font)
   // 2. Or find this object in the scene
   // 3. On each texture_new call - fill internal texture with glyph, remember the
   //    glyph pointer - we'll use it.

   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DContext *context = dev_ptr->context;

   if (!context->font.IsNull() && ((D3DObjectFont *)context->font.Addr())->Compare(font))
      return;

   D3DScene *scene = dev_ptr->scene;

   static TArray<D3DObjectFont *> fonts;
   scene->GetObjectsOfType<D3DObjectFont>(fonts);
   for (int i = 0; i < fonts.Length(); i++)
   {
      if (fonts[i]->Compare(font))
      {
         context->font = fonts[i];
         return;
      }
   }

   D3DDevice *device = dev_ptr->device;

   assert(dev_ptr->fonts_buffer_image_id >= 0);

   Ref<D3DObjectFont> new_font = new D3DObjectFont(font, dev_ptr->fonts_buffer_image_id);
   scene->AddObject(new_font);
   context->font = new_font;
}

void evas_direct3d_font_free(Direct3DDeviceHandler d3d, void *font)
{
   DevicePtr *dev_ptr = SelectDevice(d3d);
   D3DContext *context = dev_ptr->context;
   D3DScene *scene = dev_ptr->scene;

   if (context->font.IsNull() || !((D3DObjectFont *)context->font.Addr())->Compare(font))
   {
      D3DScene *scene = dev_ptr->scene;

      static TArray<D3DObjectFont *> fonts;
      scene->GetObjectsOfType<D3DObjectFont>(fonts);
      for (int i = 0; i < fonts.Length(); i++)
      {
         if (fonts[i]->Compare(font))
         {
            context->font = fonts[i];
            break;
         }
      }
   }

   scene->DeleteObject(context->font);
   context->font = NULL;
}


}  // extern "C"
