#include "evas_direct3d_shader_pack.h"
#include "evas_direct3d_device.h"
#include <assert.h>

Ref<D3DShaderPack> D3DShaderPack::_this;

D3DShaderPack::D3DShaderPack()
{
}

D3DShaderPack::~D3DShaderPack()
{
   Uninitialize();
}

D3DShaderPack *D3DShaderPack::Current()
{
   if (_this.IsNull())
      _this = new D3DShaderPack();
   return _this;
}

void D3DShaderPack::SetCurrent(D3DShaderPack *obj)
{
   _this = obj;
}


bool D3DShaderPack::Initialize(D3DDevice *d3d)
{
   bool res = true;
   if (!(res = InitVertexDeclarations(d3d) && res))
      Log("Failed to create vdecl set");
   if (!(res = InitVertexShaders(d3d) && res))
      Log("Failed to create vs set");
   if (!(res = InitPixelShaders(d3d) && res))
      Log("Failed to create ps set");
   return res;
}

void D3DShaderPack::Uninitialize()
{
   for (int i = 0; i < _vdecl.Length(); i++)
   {
      if (_vdecl[i] != NULL)
      {
         _vdecl[i]->Release();
         _vdecl[i] = NULL;
      }
   }

   for (int i = 0; i < _vs.Length(); i++)
   {
      if (_vs[i] != NULL)
      {
         _vs[i]->Release();
         _vs[i] = NULL;
      }
   }

   for (int i = 0; i < _ps.Length(); i++)
   {
      if (_ps[i] != NULL)
      {
         _ps[i]->Release();
         _ps[i] = NULL;
      }
   }
}

bool D3DShaderPack::InitVertexDeclarations(D3DDevice *d3d)
{
   _vdecl.Allocate(VDECL_NUM);
   _vdecl.Set(NULL);

   LPDIRECT3DVERTEXDECLARATION9 vdecl = NULL;
   {
      D3DVERTEXELEMENT9 elements[] = {
         {0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, 8, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
         D3DDECL_END()
         };
      if (FAILED(d3d->GetDevice()->CreateVertexDeclaration(elements, &vdecl)))
         return false;
      if (vdecl == NULL)
         return false;
   }
   _vdecl[VDECL_XYC] = vdecl;
   vdecl = NULL;
   {
      D3DVERTEXELEMENT9 elements[] = {
         {0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         D3DDECL_END()
         };
      if (FAILED(d3d->GetDevice()->CreateVertexDeclaration(elements, &vdecl)))
         return false;
      if (vdecl == NULL)
         return false;
   }
   _vdecl[VDECL_XYUV] = vdecl;
   vdecl = NULL;
   {
      D3DVERTEXELEMENT9 elements[] = {
         {0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, 8, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
         D3DDECL_END()
         };
      if (FAILED(d3d->GetDevice()->CreateVertexDeclaration(elements, &vdecl)))
         return false;
      if (vdecl == NULL)
         return false;
   }
   _vdecl[VDECL_XYUVC] = vdecl;
   vdecl = NULL;
   {
      D3DVERTEXELEMENT9 elements[] = {
         {0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
         {0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
         {0, 20, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0},
         D3DDECL_END()
         };
      if (FAILED(d3d->GetDevice()->CreateVertexDeclaration(elements, &vdecl)))
         return false;
      if (vdecl == NULL)
         return false;
   }
   _vdecl[VDECL_XYZUVC] = vdecl;

   return true;
}

bool D3DShaderPack::InitVertexShaders(D3DDevice *d3d)
{
   _vs.Allocate(VS_NUM);
   _vs.Set(NULL);

   {
      char buf[] =
         "struct VsInput {	float2 pos : POSITION; float4 col : COLOR; };\n"
         "struct VsOutput { float4 pos : POSITION; float4 col : COLOR0; };\n"
         "VsOutput main(VsInput vs_in) {\n"
         "VsOutput vs_out;\n"
	      "vs_out.pos = float4(vs_in.pos, 0, 1);\n"
         "vs_out.col = vs_in.col;\n"
	      "return vs_out;}";

      _vs[VS_COPY_COLOR] = (LPDIRECT3DVERTEXSHADER9)
         CompileShader(d3d, true, "CopyColor", buf, sizeof(buf) - 1);
      if (_vs[VS_COPY_COLOR] == NULL)
         return false;
   }

   {
      char buf[] =
         "struct VsInput {	float2 pos : POSITION; float2 tex : TEXCOORD0; };\n"
         "struct VsOutput { float4 pos : POSITION; float2 tex : TEXCOORD0; };\n"
         "VsOutput main(VsInput vs_in) {\n"
         "VsOutput vs_out;\n"
	      "vs_out.pos = float4(vs_in.pos, 0, 1);\n"
         "vs_out.tex = vs_in.tex;\n"
	      "return vs_out;}";

      _vs[VS_COPY_UV] = (LPDIRECT3DVERTEXSHADER9)
         CompileShader(d3d, true, "CopyUV", buf, sizeof(buf) - 1);
      if (_vs[VS_COPY_UV] == NULL)
         return false;
   }

   {
      char buf[] =
         "struct VsInput {	float2 pos : POSITION; float2 tex : TEXCOORD0; float4 col : COLOR; };\n"
         "struct VsOutput { float4 pos : POSITION; float2 tex : TEXCOORD0; float4 col : COLOR0; };\n"
         "VsOutput main(VsInput vs_in) {\n"
         "VsOutput vs_out;\n"
	      "vs_out.pos = float4(vs_in.pos, 0, 1);\n"
         "vs_out.tex = vs_in.tex;\n"
         "vs_out.col = vs_in.col;\n"
	      "return vs_out;}";

      _vs[VS_COPY_UV_COLOR] = (LPDIRECT3DVERTEXSHADER9)
         CompileShader(d3d, true, "CopyUVColor", buf, sizeof(buf) - 1);
      if (_vs[VS_COPY_UV_COLOR] == NULL)
         return false;
   }

   {
      char buf[] =
         "struct VsInput {	float3 pos : POSITION; float2 tex : TEXCOORD0; float4 col : COLOR; };\n"
         "struct VsOutput { float4 pos : POSITION; float2 tex : TEXCOORD0; float4 col : COLOR0; };\n"
         "VsOutput main(VsInput vs_in) {\n"
         "VsOutput vs_out;\n"
	      "vs_out.pos = float4(vs_in.pos, 1);\n"
         "vs_out.tex = vs_in.tex;\n"
         "vs_out.col = vs_in.col;\n"
	      "return vs_out;}";

      _vs[VS_COPY_UV_COLOR_Z] = (LPDIRECT3DVERTEXSHADER9)
         CompileShader(d3d, true, "CopyUVColorZ", buf, sizeof(buf) - 1);
      if (_vs[VS_COPY_UV_COLOR_Z] == NULL)
         return false;
   }

   return true;
}

bool D3DShaderPack::InitPixelShaders(D3DDevice *d3d)
{
   _ps.Allocate(PS_NUM);
   _ps.Set(NULL);

   {
      char buf[] =
         "struct VsOutput { float4 pos : POSITION; float4 col : COLOR0; };\n"
         "float4 main(VsOutput ps_in) : COLOR0 {\n"
         "return ps_in.col;}";

      _ps[PS_COLOR] = (LPDIRECT3DPIXELSHADER9)
         CompileShader(d3d, false, "Color", buf, sizeof(buf) - 1);
      if (_ps[PS_COLOR] == NULL)
         return false;
   }

   {
      char buf[] =
         "sampler Texture : register(s0);\n"
         "struct VsOutput { float4 pos : POSITION; float2 tex : TEXCOORD0; };\n"
         "float4 main(VsOutput ps_in) : COLOR0 {\n"
         "return tex2D(Texture, ps_in.tex);}";

      _ps[PS_TEX] = (LPDIRECT3DPIXELSHADER9)
         CompileShader(d3d, false, "Tex", buf, sizeof(buf) - 1);
      if (_ps[PS_TEX] == NULL)
         return false;
   }

   {
      char buf[] =
         "sampler Texture : register(s0);\n"
         "struct VsOutput { float4 pos : POSITION; float2 tex : TEXCOORD0; float4 col : COLOR0; };\n"
         "float4 main(VsOutput ps_in) : COLOR0 {\n"
         "return tex2D(Texture, ps_in.tex) * ps_in.col;}";

      _ps[PS_TEX_COLOR_FILTER] = (LPDIRECT3DPIXELSHADER9)
         CompileShader(d3d, false, "TexColorFilter", buf, sizeof(buf) - 1);
      if (_ps[PS_TEX_COLOR_FILTER] == NULL)
         return false;
   }

   {
      char buf[] =
         "sampler Texture : register(s1);\n"
         "struct VsOutput { float4 pos : POSITION; float2 tex : TEXCOORD0; };\n"
         "float4 main(VsOutput ps_in) : COLOR0 {\n"
         "return tex2D(Texture, ps_in.tex);}";

      _ps[PS_TEX_2] = (LPDIRECT3DPIXELSHADER9)
         CompileShader(d3d, false, "Tex2", buf, sizeof(buf) - 1);
      if (_ps[PS_TEX_2] == NULL)
         return false;
   }

   return true;
}

void *D3DShaderPack::CompileShader(D3DDevice *d3d, bool make_vs,
   const char *name, const char *buf, int size)
{
   LPD3DXBUFFER compiled_res = NULL;
   LPD3DXBUFFER error_msgs = NULL;

   HRESULT res = D3DXCompileShader(buf, size, NULL, NULL,
      "main", make_vs ? "vs_2_0" : "ps_2_0",  // ?
      0, &compiled_res, &error_msgs, NULL);

   if (FAILED(res))
   {
      Log("Shader %s compilation failed, code = %X", name, res);
      if (error_msgs == NULL)
         return NULL;
      const char *mess = (const char *)error_msgs->GetBufferPointer();
      Log("Error output:\n%s", mess);
      error_msgs->Release();
      return NULL;
   }

   if (error_msgs != NULL)
      error_msgs->Release();

   void *res_ptr = NULL;
   if (make_vs)
   {
      LPDIRECT3DVERTEXSHADER9 vs;
      res = d3d->GetDevice()->CreateVertexShader((DWORD *)compiled_res->GetBufferPointer(), &vs);
      res_ptr = (void *)vs;
   }
   else
   {
      LPDIRECT3DPIXELSHADER9 ps;
      res = d3d->GetDevice()->CreatePixelShader((DWORD *)compiled_res->GetBufferPointer(), &ps);
      res_ptr = (void *)ps;
   }

   compiled_res->Release();

   if (FAILED(res))
   {
      Log("Shader %s creation failed, code = %X", name, res);
      return NULL;
   }
   return res_ptr;
}

bool D3DShaderPack::SetVDecl(D3DDevice *d3d, int id)
{
   if (id < 0 || id >= _vdecl.Length() || _vdecl[id] == NULL)
      return false;
   assert(d3d != NULL);
   d3d->GetDevice()->SetVertexDeclaration(_vdecl[id]);
   return true;
}

bool D3DShaderPack::SetVS(D3DDevice *d3d, int id)
{
   if (id < 0 || id >= _vs.Length() || _vs[id] == NULL)
      return false;
   assert(d3d != NULL);
   d3d->GetDevice()->SetVertexShader(_vs[id]);
   return true;
}

bool D3DShaderPack::SetPS(D3DDevice *d3d, int id)
{
   if (id < 0 || id >= _ps.Length() || _ps[id] == NULL)
      return false;
   assert(d3d != NULL);
   d3d->GetDevice()->SetPixelShader(_ps[id]);
   return true;
}
