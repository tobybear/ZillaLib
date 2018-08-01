/*
  ZillaLib
  Copyright (C) 2010-2018 Bernhard Schelling

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/

#ifndef __ZL_DISPLAY3D__
#define __ZL_DISPLAY3D__

#include "ZL_Display.h"
#include "ZL_Surface.h"
#include "ZL_Math3D.h"
#include "ZL_Data.h"

namespace ZL_MaterialModes { enum
{
	//Material modes
	MM_STATICCOLOR = 1<<0, MM_VERTEXCOLOR = 1<<1, MM_DIFFUSEMAP = 1<<2, MM_DIFFUSEFUNC = 1<<3,
	MM_SPECULARSTATIC = 1<<4, MM_SPECULARMAP = 1<<5, MM_SPECULARFUNC = 1<<6, MM_NORMALMAP = 1<<7, MM_NORMALFUNC = 1<<8, MM_PARALLAXMAP = 1<<9,
	MM_VERTEXFUNC = 1<<10, MM_VERTEXCOLORFUNC = 1<<11, MM_POSITIONFUNC = 1<<12, MM_UVFUNC = 1<<13,
	//Material requests for custom shader code
	MR_POSITION = 1<<16, MR_TEXCOORD = 1<<17, MR_NORMAL = 1<<18, MR_CAMERATANGENT = 1<<19, MR_TIME = 1<<20,
	//Material options
	MO_UNLIT = 1<<22, MO_RECEIVENOSHADOW = 1<<23, MO_MASKED = 1<<24, MO_PRECISIONTANGENT = 1<<25,
	MO_TRANSPARENCY = 1<<26, MO_ADDITIVE = 1<<27, MO_MODULATE = 1<<28, MO_CASTNOSHADOW = 1<<29, MO_IGNOREDEPTH = 1<<30
};
enum Blending
{
	OP_OPAQUE = 0, OP_MASKED = MO_MASKED, OP_TRANSPARENT = MO_TRANSPARENCY, OP_ADDITIVE = MO_ADDITIVE, OP_MODULATE = MO_MODULATE
};}

//Shader code variable names (attributes, for vertex shader)
#define Z3A_POSITION         ZL_SHADERVARNAME("ap", "a_position")       //input mesh vertex position (vec3)
#define Z3A_NORMAL           ZL_SHADERVARNAME("ab", "a_normal")         //input mesh normal vector (vec3)
#define Z3A_TEXCOORD         ZL_SHADERVARNAME("at", "a_texcoord")       //input mesh texture coordinate (vec2)
#define Z3A_TANGENT          ZL_SHADERVARNAME("ag", "a_tangent")        //input mesh tangent vector (vec3)
#define Z3A_COLOR            ZL_SHADERVARNAME("ac", "a_color")          //input mesh vertex color (vec4)

//Shader code variable names (uniforms) (default values are 1 if not specified)
#define Z3U_VIEW             ZL_SHADERVARNAME("umv", "u_view")          //view matrix (camera+projection) (mat4)
#define Z3U_MODEL            ZL_SHADERVARNAME("umm", "u_model")         //model matrix (mat4)
#define Z3U_NORMAL           ZL_SHADERVARNAME("umn", "u_normal")        //normal matrix (inversed transposed model) (mat4)
#define Z3U_LIGHT            ZL_SHADERVARNAME("uml", "u_light")         //light transformation matrix (mat4)
#define Z3U_VIEWPOS          ZL_SHADERVARNAME("uvv", "u_viewpos")       //camera position (vec3)
#define Z3U_DIFFUSEMAP       ZL_SHADERVARNAME("usd", "u_diffuse")       //diffuse texture (sampler2D)
#define Z3U_NORMALMAP        ZL_SHADERVARNAME("usn", "u_normalmap")     //normal texture (sampler2D)
#define Z3U_SPECULARMAP      ZL_SHADERVARNAME("uss", "u_specularmap")   //specular texture (sampler2D)
#define Z3U_PARALLAXMAP      ZL_SHADERVARNAME("usp", "u_parallaxmap")   //parallax texture (sampler2D)
#define Z3U_SHADOWMAP        ZL_SHADERVARNAME("ush", "u_shadowmap")     //calculated shadow map (sampler2D)
#define Z3U_COLOR            ZL_SHADERVARNAME("uvc", "u_color")         //static color (vec4)
#define Z3U_SPECULAR         ZL_SHADERVARNAME("ufs", "u_specular")      //specular amount (float, default 5)
#define Z3U_SHININESS        ZL_SHADERVARNAME("ufh", "u_shininess")     //specular shininess factor (float, default 16)
#define Z3U_PARALLAXSCALE    ZL_SHADERVARNAME("ufp", "u_parallaxscale") //parallax mapping amount (float, default 0.05)
#define Z3U_TIME             ZL_SHADERVARNAME("ut", "u_time")           //time passed in seconds (float)

//Shader code variable names (varyings, set up by vertex shader, used in fragment shader)
#define Z3V_COLOR            ZL_SHADERVARNAME("co", "v_color")          //color as calculated by the vertex shader (vec4)
#define Z3V_TEXCOORD         ZL_SHADERVARNAME("t",  "v_texcoord")       //fragment texture coordinate (vec2)
#define Z3V_NORMAL           ZL_SHADERVARNAME("n",  "v_normal")         //fragment normal (vec3)
#define Z3V_POSITION         ZL_SHADERVARNAME("p",  "v_position")       //fragment position (vec3)
#define Z3V_CAMERATANGENT    ZL_SHADERVARNAME("ct", "v_cameratangent")  //camera tangent relative to fragment (vec3)
#define Z3V_TANGENT          ZL_SHADERVARNAME("nt",  "v_tangent")       //fragment tangent (vec3)
#define Z3V_BITANGENT        ZL_SHADERVARNAME("nb",  "v_bitangent")     //fragment binormal (vec3)

//Shader code variable names (statics, available for custom functions)
#define Z3S_NORMAL           ZL_SHADERVARNAME("N", "s_normal")          //final normal vector inside fragment shader (vec3)
#define Z3S_CAMERATANGENT    ZL_SHADERVARNAME("CT", "s_cameratangent")  //calculated camera tangent inside fragment shader (vec3)
#define Z3S_NUMLIGHTS        ZL_SHADERVARNAME("L", "s_numlights")       //max number of lights handled (int)

//Shader code variable names (output)
#define Z3O_POSITION  "gl_Position"  //result of vertex shader (Z3U_VIEW * WORLDPOSITION) (vec4)
#define Z3O_FRAGCOLOR "gl_FragColor" //result color of fragment shader (vec4)

struct ZL_Material
{
	ZL_Material();
	ZL_Material(unsigned int MaterialModes);
	ZL_Material(unsigned int MaterialModes, const char* CustomFragmentCode, const char* CustomVertexCode = NULL);
	~ZL_Material();
	ZL_Material(const ZL_Material &source);
	ZL_Material &operator=(const ZL_Material &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Material &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Material &b) const { return (impl!=b.impl); }

	ZL_Material MakeNewMaterialInstance();
	ZL_Material& SetUniformFloat(ZL_NameID Name, scalar val);
	ZL_Material& SetUniformVec2(ZL_NameID Name, const ZL_Vector& val);
	ZL_Material& SetUniformVec3(ZL_NameID Name, const ZL_Vector3& val);
	ZL_Material& SetUniformVec3(ZL_NameID Name, const ZL_Color& val);
	ZL_Material& SetUniformVec4(ZL_NameID Name, const ZL_Color& val);
	ZL_Material& SetDiffuseTexture(const ZL_Surface& srf);
	ZL_Material& SetNormalTexture(const ZL_Surface& srf);
	ZL_Material& SetSpecularTexture(const ZL_Surface& srf);
	ZL_Material& SetParallaxTexture(const ZL_Surface& srf);

	scalar GetUniformFloat(ZL_NameID Name);
	ZL_Vector GetUniformVec2(ZL_NameID Name);
	ZL_Vector3 GetUniformVec3(ZL_NameID Name);
	ZL_Color GetUniformVec4(ZL_NameID Name);
	ZL_Surface GetDiffuseTexture();
	ZL_Surface GetNormalTexture();
	ZL_Surface GetSpecularTexture();
	ZL_Surface GetParallaxTexture();

	private: struct ZL_Material_Impl* impl;
};

struct ZL_Mesh
{
	ZL_Mesh();
	ZL_Mesh(const ZL_FileLink& ModelFile, const ZL_Material& Material = DefaultMaterial());
	~ZL_Mesh();
	ZL_Mesh(const ZL_Mesh &source);
	ZL_Mesh &operator=(const ZL_Mesh &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Mesh &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Mesh &b) const { return (impl!=b.impl); }

	ZL_Material GetMaterial(unsigned int PartNumber = 0) const;
	ZL_Material GetMaterial(ZL_NameID PartName) const;
	ZL_Mesh& SetMaterial(unsigned int PartNumber, const ZL_Material& Material);
	ZL_Mesh& SetMaterial(ZL_NameID PartName, const ZL_Material& Material);
	ZL_Mesh& SetMaterial(const ZL_Material& Material);

	static ZL_Mesh FromPLY(const ZL_FileLink& PLYFile, const ZL_Material& Material = DefaultMaterial());
	static ZL_Mesh FromOBJ(const ZL_FileLink& OBJFile, const ZL_Material& Material = DefaultMaterial());

	enum VertDataMode { VDM_POS = 0, VDM_POS_NORMAL = 2, VDM_POS_NORMAL_TEXCOORD = 6, VDM_POS_NORMAL_TEXCOORD_TANGENT = 14, VDM_POS_NORMAL_TEXCOORD_TANGENT_COLOR = 30, VDM_POS_NORMAL_TEXCOORD_COLOR = 22, VDM_POS_NORMAL_TANGENT = 10, VDM_POS_NORMAL_TANGENT_COLOR = 26, VDM_POS_NORMAL_COLOR = 18, VDM_POS_TEXCOORD = 4, VDM_POS_TEXCOORD_TANGENT = 12, VDM_POS_TEXCOORD_TANGENT_COLOR = 28, VDM_POS_TEXCOORD_COLOR = 20, VDM_POS_TANGENT = 8, VDM_POS_TANGENT_COLOR = 24, VDM_POS_COLOR = 16 };
	static ZL_Mesh BuildMesh(const unsigned short* Indices, size_t NumIndices, const void* Vertices, size_t NumVertices, VertDataMode Content, const ZL_Material& Material = DefaultMaterial());
	static ZL_Mesh BuildQuads(const void* Vertices, size_t NumQuads, VertDataMode Content, const ZL_Material& Material = DefaultMaterial());
	static ZL_Mesh BuildPlane(const ZL_Vector& Extents, const ZL_Material& Material = DefaultMaterial(), const ZL_Vector3& Normal = ZL_Vector3::Up, const ZL_Vector3& Offset = ZL_Vector3::Zero, const ZL_Vector& UVMapMax = ZL_Vector::One);
	static ZL_Mesh BuildBox(const ZL_Vector3& Extents, const ZL_Material& Material = DefaultMaterial(), const ZL_Vector3& Offset = ZL_Vector3::Zero, const ZL_Vector& UVMapMax = ZL_Vector::One);
	static ZL_Mesh BuildLandscape(const ZL_Material& Material = DefaultMaterial());
	static ZL_Mesh BuildSphere(scalar Radius, int Segments, bool Inside = false, const ZL_Material& Material = DefaultMaterial());
	static ZL_Mesh BuildExtrudePixels(scalar Scale, scalar Depth, const ZL_FileLink& ImgFile, const ZL_Material& Material = DefaultMaterial(), bool KeepAlpha = true, bool AlphaDepth = false, scalar AlphaDepthAlign = .5f, const ZL_Matrix& Transform = ZL_Matrix::Identity);

	enum IntersectMode { IM_ODD = 0, IM_NONZERO = 1, IM_POSITIVE = 2, IM_NEGATIVE = 3, IM_ABS_GEQ_TWO = 4 };
	static ZL_Mesh BuildExtrudeContour(const ZL_Vector3* p, size_t pnum,                              scalar Depth, const ZL_Material& Material = DefaultMaterial(), IntersectMode selfintersect = IM_NONZERO);
	static ZL_Mesh BuildExtrudeContour(const std::vector<ZL_Vector3>& contour,                        scalar Depth, const ZL_Material& Material = DefaultMaterial(), IntersectMode selfintersect = IM_NONZERO);
	static ZL_Mesh BuildExtrudeContours(const ZL_Vector3*const* ps, const size_t* pnums, size_t cnum, scalar Depth, const ZL_Material& Material = DefaultMaterial(), IntersectMode intersect     = IM_NONZERO);
	static ZL_Mesh BuildExtrudeContours(const std::vector<ZL_Vector3>*const* cs, size_t cnum,         scalar Depth, const ZL_Material& Material = DefaultMaterial(), IntersectMode intersect     = IM_NONZERO);

	#if defined(ZILLALOG) && !defined(ZL_VIDEO_OPENGL_ES2)
	//Debug drawing is only available on desktop debug builds
	void DrawDebug(const ZL_Matrix& Matrix, const struct ZL_Camera& Camera);
	#endif

	protected: struct ZL_Mesh_Impl* impl;
	inline static ZL_Material DefaultMaterial() { return ZL_Material(ZL_MaterialModes::MM_STATICCOLOR); }
};

struct ZL_MeshAnimated : public ZL_Mesh
{
	ZL_MeshAnimated();
	ZL_MeshAnimated(const ZL_FileLink& AnimZipFile, const ZL_Material& Material = DefaultMaterial());
	~ZL_MeshAnimated();
	ZL_MeshAnimated(const ZL_MeshAnimated &source);
	ZL_MeshAnimated &operator=(const ZL_MeshAnimated &source);

	ZL_MeshAnimated& SetFrame(unsigned int FrameIndex);
};

struct ZL_ParticleEmitter : public ZL_Mesh
{
	ZL_ParticleEmitter();
	ZL_ParticleEmitter(scalar LifeTime, size_t MaxParticles = 500, ZL_MaterialModes::Blending BlendMode = ZL_MaterialModes::OP_ADDITIVE);
	~ZL_ParticleEmitter();
	ZL_ParticleEmitter(const ZL_ParticleEmitter &source);
	ZL_ParticleEmitter &operator=(const ZL_ParticleEmitter &source);

	ZL_ParticleEmitter& SetLifeTime(scalar LifeTime);
	ZL_ParticleEmitter& SetGravity(const ZL_Vector3& Gravity);
	ZL_ParticleEmitter& SetSpawnChance(float Chance);
	ZL_ParticleEmitter& SetSpawnVelocity(const ZL_Vector3& Velocity);
	ZL_ParticleEmitter& SetSpawnVelocityRanges(const ZL_Vector3& VelocityMin, const ZL_Vector3& VelocityMax);
	ZL_ParticleEmitter& SetColor(const ZL_Color& Color, bool IncludeAlpha = true);
	ZL_ParticleEmitter& SetSpawnColorRange(const ZL_Color& ColorMin, const ZL_Color& ColorMax, bool IncludeAlpha = true);
	ZL_ParticleEmitter& SetLifetimeColor(const ZL_Color& ColorStart, const ZL_Color& ColorEnd, bool IncludeAlpha = true);
	ZL_ParticleEmitter& SetAlpha(scalar Alpha);
	ZL_ParticleEmitter& SetSpawnAlphaRange(scalar AlphaMin, scalar AlphaMax);
	ZL_ParticleEmitter& SetLifetimeAlpha(scalar AlphaStart, scalar AlphaEnd);
	ZL_ParticleEmitter& SetSize(scalar Size);
	ZL_ParticleEmitter& SetSpawnSizeRange(scalar SizeMin, scalar SizeMax);
	ZL_ParticleEmitter& SetLifetimeSize(scalar SizeStart, scalar SizeEnd);
	ZL_ParticleEmitter& SetTile(int TileIndex);
	ZL_ParticleEmitter& SetSpawnTileRange(int TileIndexMin, int TileIndexMax);
	ZL_ParticleEmitter& SetLifetimeTile(int TileIndexStart, int TileIndexEnd);
	ZL_ParticleEmitter& SetTexture(const ZL_Surface& srf, int NumTilesCols = 1, int NumTilesRows = 1); //auto sets SpawnTile range
	ZL_ParticleEmitter& SetAnimationSheet(const ZL_Surface& srf, int NumTilesCols, int NumTilesRows); //auto sets LifetimeTile range

	ZL_ParticleEmitter& Spawn(const ZL_Vector3& Pos);
	ZL_ParticleEmitter& Update(const struct ZL_Camera& Camera);
};

struct ZL_Camera
{
	ZL_Camera();
	~ZL_Camera();
	ZL_Camera(const ZL_Camera &source);
	ZL_Camera &operator=(const ZL_Camera &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Camera &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Camera &b) const { return (impl!=b.impl); }

	ZL_Camera& SetPosition(const ZL_Vector3& pos);
	ZL_Camera& SetDirection(const ZL_Vector3& dir);
	ZL_Camera& SetLookAt(const ZL_Vector3& pos, const ZL_Vector3& target);
	ZL_Camera& SetFOV(scalar fov); //default 90
	ZL_Camera& SetOrtho(scalar size); //default off
	ZL_Camera& SetAspectRatio(scalar ar); //default -1 (use display ratio)
	ZL_Camera& SetClipPlane(scalar znear, scalar zfar); //default (0.1, 1000.0)
	ZL_Camera& SetAmbientLightColor(const ZL_Color& color); //default (0.2, 0.2, 0.2)
	ZL_Vector3 GetPosition() const;
	ZL_Vector3 GetDirection() const;
	ZL_Vector3 GetUpDirection() const;
	ZL_Vector3 GetRightDirection() const;
	scalar GetFOV() const;
	scalar GetAspectRatio() const;
	scalar GetNearClipPlane() const;
	scalar GetFarClipPlane() const;
	ZL_Color GetAmbientLightColor() const;
	ZL_Vector WorldToScreen(const ZL_Vector3& WorldPosition);
	ZL_Vector WorldToScreen(const ZL_Vector3& WorldPosition, bool* pIsOutsideOfView);
	ZL_Vector3 ScreenToWorld(const ZL_Vector& ScreenPosition, scalar Depth = -1);
	void ScreenToWorld(const ZL_Vector& ScreenPosition, ZL_Vector3* WorldRayStart, ZL_Vector3* WorldRayEnd = NULL);

	private: struct ZL_Camera_Impl* impl;
};

struct ZL_Light
{
	ZL_Light();
	ZL_Light(const ZL_Vector3& pos);
	~ZL_Light();
	ZL_Light(const ZL_Light &source);
	ZL_Light &operator=(const ZL_Light &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_Light &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_Light &b) const { return (impl!=b.impl); }

	ZL_Light& SetPosition(const ZL_Vector3& pos);
	ZL_Light& SetDirection(const ZL_Vector3& dir);
	ZL_Light& SetLookAt(const ZL_Vector3& pos, const ZL_Vector3& target);
	ZL_Light& SetPointLight(); //default on
	ZL_Light& SetDirectionalLight(scalar area); //for shadowmapping, default off
	ZL_Light& SetSpotLight(scalar angle_degree, scalar aspect = 1); //for shadowmapping, default off
	ZL_Light& SetRange(scalar range_far, scalar range_near = 1); //for shadowmapping, default 50.0, 1.0
	ZL_Light& SetOutsideLit(bool OutsideLit = true); //for shadowmapping, default false
	ZL_Light& SetShadowBias(scalar bias); //for shadowmapping, default 0.001
	ZL_Light& SetFalloff(scalar distance); //default infinite
	ZL_Light& SetColor(const ZL_Color& color); //default white
	ZL_Vector3 GetPosition() const;
	ZL_Vector3 GetDirection() const;
	ZL_Color GetColor() const;

	private: struct ZL_Light_Impl* impl;
};

struct ZL_RenderList
{
	ZL_RenderList();
	~ZL_RenderList();
	ZL_RenderList(const ZL_RenderList &source);
	ZL_RenderList &operator=(const ZL_RenderList &source);
	operator bool () const { return (impl!=NULL); }
	bool operator==(const ZL_RenderList &b) const { return (impl==b.impl); }
	bool operator!=(const ZL_RenderList &b) const { return (impl!=b.impl); }

	void Reset();
	void Add(const ZL_Mesh& Mesh, const ZL_Matrix& Matrix);
	void Add(const ZL_Mesh& Mesh, const ZL_Matrix& Matrix, const ZL_Material& OverrideMaterial);
	void AddReferenced(const ZL_Mesh& Mesh, const ZL_Matrix& Matrix); //keep mesh reference in list

	#if defined(ZILLALOG) && !defined(ZL_VIDEO_OPENGL_ES2)
	//Debug functionality is only available on desktop debug builds
	void DebugDump();
	#endif

	private: struct ZL_RenderList_Impl* impl;
};

struct ZL_Display3D
{
	//Initialize 3D rendering features
	static bool Init(size_t MaxLights = 1);
	static bool InitShadowMapping();

	//Draw rendering lists
	static inline void DrawList(const ZL_RenderList& RenderList, const ZL_Camera& Camera) { const ZL_RenderList* r = &RenderList; DrawLists(&r, 1, Camera); }
	static void DrawLists(const ZL_RenderList*const* RenderLists, size_t NumLists, const ZL_Camera& Camera);
	static inline void DrawListWithLight(const ZL_RenderList& RenderList, const ZL_Camera& Camera, const ZL_Light& Light) { const ZL_RenderList* r = &RenderList; const ZL_Light* l = &Light; DrawListsWithLights(&r, 1, Camera, &l, 1); }
	static inline void DrawListWithLights(const ZL_RenderList& RenderList, const ZL_Camera& Camera, const ZL_Light*const* Lights, size_t NumLights) { const ZL_RenderList* r = &RenderList; DrawListsWithLights(&r, 1, Camera, Lights, NumLights); }
	static inline void DrawListsWithLight(const ZL_RenderList*const* RenderLists, size_t NumLists, const ZL_Camera& Camera, const ZL_Light& Light) { const ZL_Light* l = &Light; DrawListsWithLights(RenderLists, NumLists, Camera, &l, 1); }
	static void DrawListsWithLights(const ZL_RenderList*const* RenderLists, size_t NumLists, const ZL_Camera& Camera, const ZL_Light*const* Lights, size_t NumLights);

	//Manual render state handling when issueing multiple draw list calls or when using the simple shape rendering functions below
	static void BeginRendering();
	static void FinishRendering();

	//Simple shape rendering
	static void DrawLine(const ZL_Camera& cam, const ZL_Vector3& a, const ZL_Vector3& b, const ZL_Color& color = ZL_Color::White, scalar width = 0.01f);
	static void DrawPlane(const ZL_Camera& cam, const ZL_Vector3& pos, const ZL_Vector3& normal, const ZL_Vector& extents, const ZL_Color& color = ZL_Color::White);
	static void DrawFrustum(const ZL_Camera& cam, const ZL_Matrix& VPMatrix, const ZL_Color& color = ZL_Color::White, scalar width = 0.03f);
	static void DrawSphere(const ZL_Camera& cam, const ZL_Vector3& pos, scalar radius, const ZL_Color& color = ZL_Color::White);
};

//returns glsl function 'float snoise(vec2)'
#define ZL_GLSL_IMPORTSNOISE() "vec3 snoiseprm(vec3 x){x*=((x*34.0)+1.0);return x-floor(x*(1.0/289.0))*289.0;}float snoise(vec2 v){const vec4 C=vec4(0.211324865405187,0.366025403784439,-0.577350269189626,0.024390243902439);vec2 i=floor(v+dot(v,C.yy)),y=v-i+dot(i,C.xx),j=((y.x>y.y)?vec2(1.0,0.0):vec2(0.0,1.0));i-=floor(i*(1.0/289.0))*289.0;vec3 p=snoiseprm(snoiseprm(i.y+vec3(0.0,j.y,1.0))+i.x+vec3(0.0,j.x,1.0));vec3 x=2.0*fract(p*C.www)-1.0,h=abs(x)-0.5,a=x-floor(x+0.5);vec4 z=vec4(y.x+C.x-j.x,y.y+C.x-j.y,y.x+C.z,y.y+C.z);vec3 m=max(0.5-vec3(dot(y,y),dot(z.xy,z.xy),dot(z.zw,z.zw)),0.0);m*=m;m*=m;m*=1.79284291400159-0.85373472095314*(a*a+h*h);return 0.5+65.0*dot(m,vec3(a.x*y.x+h.x*y.y,a.y*z.x+h.y*z.y,a.z*z.z+h.z*z.w));}"

#ifdef ZILLALOG
#define ZL_SHADERVARNAME(release_name, debug_name) debug_name
#else
#define ZL_SHADERVARNAME(release_name, debug_name) release_name
#endif

#endif //__ZL_DISPLAY3D__
