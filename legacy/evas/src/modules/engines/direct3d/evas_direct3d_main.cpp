#include "evas_engine.h"


extern "C" {

int
evas_direct3d_masks_get(Outbuf *buf)
{
   D3DSURFACE_DESC sd;

   if (FAILED(buf->priv.d3d.texture->GetLevelDesc(0, &sd)))
     return 0;

   switch (sd.Format)
     {
     case D3DFMT_A8R8G8B8:
     case D3DFMT_X8R8G8B8:
       buf->priv.mask.r = 0x00ff0000;
       buf->priv.mask.g = 0x0000ff00;
       buf->priv.mask.b = 0x000000ff;
       break;
     case D3DFMT_R5G6B5:
       buf->priv.mask.r = 0xf800;
       buf->priv.mask.g = 0x07e0;
       buf->priv.mask.b = 0x001f;
       break;
     default:
       return 0;
     }

   return 1;
}

void *
evas_direct3d_lock(Outbuf *buf, int *d3d_width, int *d3d_height, int *d3d_pitch)
{
   D3DSURFACE_DESC sd;
   D3DLOCKED_RECT  d3d_rect;

   /* is that call needed / overkill ? */
   if (FAILED(buf->priv.d3d.texture->GetLevelDesc(0, &sd)))
     return NULL;

   if (FAILED(buf->priv.d3d.device->BeginScene()))
     return NULL;
  if (FAILED(buf->priv.d3d.sprite->Begin(D3DXSPRITE_DO_NOT_ADDREF_TEXTURE)))
    {
       buf->priv.d3d.device->EndScene();
       return NULL;
    }
  if (FAILED(buf->priv.d3d.texture->LockRect(0, &d3d_rect, NULL, D3DLOCK_DISCARD)))
    {
       buf->priv.d3d.sprite->End();
       buf->priv.d3d.device->EndScene();
       return NULL;
    }

  *d3d_width = sd.Width;
  *d3d_height = sd.Height;
  *d3d_pitch = d3d_rect.Pitch;

  return d3d_rect.pBits;
}

void
evas_direct3d_unlock(Outbuf *buf)
{
  if (FAILED(buf->priv.d3d.texture->UnlockRect(0)))
    return;

  if (FAILED(buf->priv.d3d.sprite->Draw(buf->priv.d3d.texture,
                                        NULL, NULL, NULL,
                                        D3DCOLOR_ARGB (255, 255, 255, 255))))
    return;
  if (FAILED(buf->priv.d3d.sprite->End()))
    return;

  if (FAILED(buf->priv.d3d.device->EndScene()))
    return;
  if (FAILED(buf->priv.d3d.device->Present(NULL, NULL, NULL, NULL)))
    return;
}

}
