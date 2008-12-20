#ifndef __EVAS_DIRECT3D_SHADER_PACK_H__
#define __EVAS_DIRECT3D_SHADER_PACK_H__

#include "evas_engine.h"

#include "ref.h"
#include "array.h"

class D3DDevice;

class D3DShaderPack : virtual public Referenc
{
public:
   ~D3DShaderPack();

   static D3DShaderPack *Current();
   static void SetCurrent(D3DShaderPack *obj);

   bool Initialize(D3DDevice *d3d);
   void Uninitialize();

   bool SetVDecl(D3DDevice *d3d, int id);
   bool SetVS(D3DDevice *d3d, int id);
   bool SetPS(D3DDevice *d3d, int id);

public:
   enum VDECL
   {
      VDECL_XYC = 0,
      VDECL_XYUV,
      VDECL_XYUVC,
      VDECL_XYZUVC,

      VDECL_NUM
   };

   enum VS
   {
      VS_COPY_COLOR = 0,
      VS_COPY_UV,
      VS_COPY_UV_COLOR,
      VS_COPY_UV_COLOR_Z,

      VS_NUM
   };

   enum PS
   {
      PS_COLOR = 0,
      PS_TEX,
      PS_TEX_COLOR_FILTER,
      PS_TEX_2,

      PS_NUM
   };

private:
   D3DShaderPack();

   bool InitVertexDeclarations(D3DDevice *d3d);
   bool InitVertexShaders(D3DDevice *d3d);
   bool InitPixelShaders(D3DDevice *d3d);

   void *CompileShader(D3DDevice *d3d, bool make_vs, const char *name,
      const char *buf, int size);

private:
   TArray<LPDIRECT3DVERTEXDECLARATION9> _vdecl;
   TArray<LPDIRECT3DVERTEXSHADER9> _vs;
   TArray<LPDIRECT3DPIXELSHADER9> _ps;

   static Ref<D3DShaderPack> _this;
};

#endif  // __EVAS_DIRECT3D_SHADER_PACK_H__
