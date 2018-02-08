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

#include "ZL_Platform.h"
#ifndef ZL_DISABLE_DISPLAY3D

#include "ZL_Display3D.h"
#include "ZL_Impl.h"
#include "ZL_Texture_Impl.h"
#include "ZL_Display_Impl.h"
#include <assert.h>
#include <map>

#define ZL_DISPLAY3D_ENABLE_SHIFT_DEBUG_VIEW

struct ZL_ShaderIDs { GLuint Program; GLubyte UsedAttributeMask; GLint UniformMatrixModel, UniformMatrixNormal; };
static struct { ZL_ShaderIDs Shader; GLuint BoundTextureChksum, BoundTextures[4], IndexBuffer, VertexBuffer; GLubyte AttributeMask; GLenum Texture; } g_Active3D;
static std::vector<struct ZL_MaterialProgram*>* g_LoadedShaderVariations;
static GLubyte g_MaxLights;
static struct ZL_MaterialProgram* g_DebugColorMat;

static void (*g_SetupShadowMapProgram)(struct ZL_MaterialProgram* ShaderProgram, unsigned int MM, const char* CustomVertexCode);
static GLuint g_ShadowMap_FBO, g_ShadowMap_TEX;
static ZL_MaterialProgram *g_ShadowMapProgram, *g_ShadowMapMaskProgram;
#define SHADOWMAP_SIZE         2048
#define SHADOWMAP_SIZE_STRING "2048"

#ifdef ZL_VIDEO_WEAKCONTEXT
static std::vector<struct ZL_MaterialProgram*> *g_LoadedMaterialPrograms;
static std::vector<struct ZL_Mesh_Impl*> *g_LoadedMeshes;
#endif

namespace ZL_Display3D_Shaders
{
	#define Z3U_LIGHTDATA        ZL_SHADERVARNAME("uvl", "u_lightdata")     //light data (vec3 array)
	#define Z3L_LIGHTFACTOR      ZL_SHADERVARNAME("LF", "l_lightfactor")
	#define Z3L_LIGHTSCOLOR      ZL_SHADERVARNAME("LC", "l_lightscolor")
	#define Z3L_LIGHTSSHINE      ZL_SHADERVARNAME("LS", "l_lightsshine")
	#define Z3L_LIGHTSPECULAR    ZL_SHADERVARNAME("LP", "l_lightspecular")
	#define Z3L_SPECULAR         ZL_SHADERVARNAME("S", "l_specular")
	#define Z3L_PARALLAXDEPTH    ZL_SHADERVARNAME("PD", "l_parallaxdepth")
	#define Z3L_TANGENT          ZL_SHADERVARNAME("T", "l_tangent")
	#define Z3L_BITANGENT        ZL_SHADERVARNAME("B", "l_bitangent")
	#define Z3L_POS2CAMERA       ZL_SHADERVARNAME("PC", "l_pos2camera")
	#define Z3L_POS2LIGHT        ZL_SHADERVARNAME("PL", "l_pos2light")
	#define Z3L_LIGHTDISTANCE    ZL_SHADERVARNAME("LD", "l_lightdistance")
	#define Z3L_LIGHTDIM         ZL_SHADERVARNAME("LI", "l_lightdim")
	#define Z3L_DIRECTION2LIGHT  ZL_SHADERVARNAME("DL", "l_direction2light")
	#define Z3D_SHADOWMAP "SM"

	static const char S_VoidMain[] = "void main(){";

	static const char *S_AttributeList[] = { Z3A_POSITION, Z3A_NORMAL, Z3A_TEXCOORD, Z3A_TANGENT, Z3A_COLOR };

	using namespace ZL_MaterialModes;
	enum
	{
		MMDEF_USECUSTOMFRAGMENT = MM_DIFFUSEFUNC|MM_NORMALFUNC|MM_SPECULARFUNC,
		MMDEF_USECUSTOMVERTEX   = MM_VERTEXFUNC|MM_VERTEXCOLORFUNC|MM_POSITIONFUNC|MM_UVFUNC,
		MMDEF_USECUSTOMSHADER   = MMDEF_USECUSTOMFRAGMENT|MMDEF_USECUSTOMVERTEX,
		MMDEF_FRAGCOLORMODES    = MM_STATICCOLOR|MM_VERTEXCOLOR|MM_VERTEXCOLORFUNC|MM_DIFFUSEMAP|MM_DIFFUSEFUNC,
		MMDEF_USESLIT           = MM_SPECULARSTATIC|MM_SPECULARMAP|MM_SPECULARFUNC|MM_NORMALMAP|MM_NORMALFUNC,
		MMDEF_REQUESTS          = MR_POSITION|MR_TEXCOORD|MR_NORMAL|MR_CAMERATANGENT|MR_TIME,
		MMDEF_NOSHADERCODE      = MO_TRANSPARENCY|MO_ADDITIVE|MO_MODULATE|MO_CASTNOSHADOW|MO_IGNOREDEPTH,
		MMDEF_NODEPTHWRITE      = MO_TRANSPARENCY|MO_ADDITIVE|MO_MODULATE,

		MMUSE_VERTEXCOLOR   = MM_VERTEXCOLOR|MM_VERTEXCOLORFUNC,
		MMUSE_SPECULAR      = MM_SPECULARSTATIC|MM_SPECULARMAP|MM_SPECULARFUNC,
		MMUSE_CAMERATANGENT = MM_PARALLAXMAP|MR_CAMERATANGENT,
		MMUSE_BITANGENT     = MO_PRECISIONTANGENT|MM_NORMALMAP|MM_NORMALFUNC,
		MMUSE_TANGENT       = MMUSE_CAMERATANGENT|MMUSE_BITANGENT,
		MMUSE_TEXCOORD      = MM_PARALLAXMAP|MM_DIFFUSEMAP|MM_SPECULARMAP|MM_NORMALMAP|MR_TEXCOORD,
		MMUSE_NORMAL        = MMUSE_TANGENT|MM_PARALLAXMAP|MR_NORMAL,
		MMUSE_POSITION      = MMUSE_TANGENT|MR_POSITION,
		MMUSE_LATECOLORCALC = MM_PARALLAXMAP|MM_DIFFUSEFUNC,
	};

	enum { EXTERN_Varying_ShadowMap, EXTERN_VS_ShadowMap_Defs, EXTERN_FS_ShadowMap_Defs, EXTERN_VS_ShadowMap_Calc, EXTERN_FS_ShadowMap_Calc, _EXTERN_NUM };
	static const char* ExternalSource[_EXTERN_NUM];
	static char Const_NumLights[] = "const int " Z3S_NUMLIGHTS "=   ", *Const_NumLightsNumberPtr = Const_NumLights+COUNT_OF(Const_NumLights)-4;

	static const struct SourceRule { int MMUseIf, MMLimit; const char *Source; }
		SharedRules[] = {
			#ifdef ZL_VIDEO_OPENGL_ES2
			{ 0,0,ZLGLSL_LIST_HIGH_PRECISION_HEADER },
			#endif
			{ MMUSE_VERTEXCOLOR,                      0, "varying vec4 " Z3V_COLOR ";" },
			{ MMUSE_TEXCOORD,                         0, "varying vec2 " Z3V_TEXCOORD ";" },
			{ MMUSE_CAMERATANGENT,  MO_PRECISIONTANGENT, "varying vec3 " Z3V_CAMERATANGENT ";" },
			{ MMUSE_BITANGENT,                        0, "varying vec3 " Z3V_TANGENT ", " Z3V_BITANGENT ";" },
			{ MMUSE_NORMAL,                           0, "varying vec3 " Z3V_NORMAL ";" },
			{ MMUSE_POSITION,                         0, "varying vec3 " Z3V_POSITION ";" },
			{ 0,            MO_RECEIVENOSHADOW|MO_UNLIT, (const char *)&ExternalSource[EXTERN_Varying_ShadowMap] },
		},
		VSGlobalRules[] = {
			{ 0,                                      0, "uniform mat4 " Z3U_VIEW "," Z3U_MODEL ";attribute vec3 " Z3A_POSITION ";" },
			{ MMUSE_TEXCOORD,                         0, "attribute vec2 " Z3A_TEXCOORD ";" },
			{ MM_VERTEXCOLOR,                         0, "attribute vec4 " Z3A_COLOR ";" },
			{ MMUSE_NORMAL,                           0, "attribute vec3 " Z3A_NORMAL ";uniform mat4 " Z3U_NORMAL ";" },
			{ MMUSE_TANGENT,        MO_PRECISIONTANGENT, "attribute vec3 " Z3A_TANGENT ";uniform vec3 " Z3U_VIEWPOS ";" },
			{ MMUSE_TANGENT,       -MO_PRECISIONTANGENT, "attribute vec3 " Z3A_TANGENT ";" },
			{ MR_TIME,                                0, "uniform float " Z3U_TIME ";" },
		},
		VSRules[] = {
			{ 0,0,0 }, // <-- Custom Vertex Code
			{ 0,            MO_RECEIVENOSHADOW|MO_UNLIT, (const char *)&ExternalSource[EXTERN_VS_ShadowMap_Defs] },
			{ 0,0,S_VoidMain },
			{ MM_VERTEXFUNC,                          0, "Vertex();" },
			{ 0,          MM_POSITIONFUNC|MM_VERTEXFUNC, Z3O_POSITION " = " Z3U_MODEL " * vec4(" Z3A_POSITION ", 1);" },
			{ MM_POSITIONFUNC,            MM_VERTEXFUNC, Z3O_POSITION " = CalcPosition();" },
			{ MMUSE_POSITION,                         0, Z3V_POSITION " = " Z3O_POSITION ".xyz;" },
			{ 0,                                      0, Z3O_POSITION " = " Z3U_VIEW " * " Z3O_POSITION ";" },
			{ MMUSE_TEXCOORD,   MM_UVFUNC|MM_VERTEXFUNC, Z3V_TEXCOORD " = " Z3A_TEXCOORD ";" },
			{ MM_UVFUNC,                  MM_VERTEXFUNC, Z3V_TEXCOORD " = CalcUV();" },
			{ MM_VERTEXCOLOR,             MM_VERTEXFUNC, Z3V_COLOR " = " Z3A_COLOR ";" },
			{ MM_VERTEXCOLORFUNC,         MM_VERTEXFUNC, Z3V_COLOR " = CalcColor();" },
			{ MMUSE_NORMAL,               MM_VERTEXFUNC, Z3V_NORMAL " = normalize(vec3(" Z3U_NORMAL " * vec4(" Z3A_NORMAL ", 0)));" },
			{ MMUSE_TANGENT,        MO_PRECISIONTANGENT, "vec3 " Z3L_TANGENT " = normalize(vec3(" Z3U_NORMAL " * vec4(" Z3A_TANGENT ", 0)))," Z3L_BITANGENT " = cross(" Z3V_NORMAL ", " Z3L_TANGENT ");" },
			{ MMUSE_CAMERATANGENT,  MO_PRECISIONTANGENT, "vec3 " Z3L_POS2CAMERA " = " Z3U_VIEWPOS " - " Z3V_POSITION ";" Z3V_CAMERATANGENT " = normalize(vec3(dot(" Z3L_POS2CAMERA ", " Z3L_TANGENT "), dot(" Z3L_POS2CAMERA ", " Z3L_BITANGENT "), dot(" Z3L_POS2CAMERA ", " Z3V_NORMAL ")));" },
			{ MMUSE_BITANGENT,                        0, Z3V_TANGENT " = normalize(vec3(" Z3U_NORMAL " * vec4(" Z3A_TANGENT ", 0)));" Z3V_BITANGENT " = cross(" Z3V_NORMAL ", " Z3V_TANGENT ");" },
			{ 0,            MO_RECEIVENOSHADOW|MO_UNLIT, (const char *)&ExternalSource[EXTERN_VS_ShadowMap_Calc] },
			{ 0,0,"}" }
		},
		FSRules[] = {
			{ 0,                               MO_UNLIT, Const_NumLights },
			{ MM_STATICCOLOR,                         0, "uniform vec4 " Z3U_COLOR ";" },
			{ MM_DIFFUSEMAP,                          0, "uniform sampler2D " Z3U_DIFFUSEMAP ";" },
			{ MM_PARALLAXMAP,                         0, "uniform sampler2D " Z3U_PARALLAXMAP ";uniform float " Z3U_PARALLAXSCALE ";" },
			{ MMUSE_SPECULAR|MMUSE_CAMERATANGENT,     0, "uniform vec3 " Z3U_VIEWPOS ";" },
			{ 0,                               MO_UNLIT, "uniform vec3 " Z3U_LIGHTDATA "[1+3*" Z3S_NUMLIGHTS "];" },
			{ MM_NORMALMAP,                           0, "uniform sampler2D " Z3U_NORMALMAP ";" },
			{ MMUSE_SPECULAR,                         0, "uniform float " Z3U_SPECULAR ", " Z3U_SHININESS ";" },
			{ MM_SPECULARMAP,                         0, "uniform sampler2D " Z3U_SPECULARMAP ";" },
			{ MR_TIME,                                0, "uniform float " Z3U_TIME ";" },
			{ MMUSE_NORMAL,                           0, "vec3 " Z3S_NORMAL ";" },
			{ MMUSE_CAMERATANGENT,                    0, "vec3 " Z3S_CAMERATANGENT ";" },
			{ 0,            MO_RECEIVENOSHADOW|MO_UNLIT, (const char *)&ExternalSource[EXTERN_FS_ShadowMap_Defs] },
			{ 0,0,0 }, // <-- Custom Fragment Code
			{ 0,0,S_VoidMain },
			{ MMDEF_FRAGCOLORMODES,        MMUSE_LATECOLORCALC, Z3O_FRAGCOLOR " = " },
			{ MMDEF_FRAGCOLORMODES,        MMUSE_LATECOLORCALC, 0 }, // <-- FragColor Calculation
			{ MO_MASKED,                   MMUSE_LATECOLORCALC, "if (" Z3O_FRAGCOLOR ".a<.5)discard;" },
			{ MMUSE_CAMERATANGENT,         MO_PRECISIONTANGENT, Z3S_CAMERATANGENT " = normalize(" Z3V_CAMERATANGENT ");" },
			{ MMUSE_CAMERATANGENT,        -MO_PRECISIONTANGENT, "vec3 " Z3L_POS2CAMERA " = " Z3U_VIEWPOS " - " Z3V_POSITION ";"
			                                                    Z3S_CAMERATANGENT " = normalize(vec3(dot(" Z3L_POS2CAMERA ", " Z3V_TANGENT "), dot(" Z3L_POS2CAMERA ", " Z3V_BITANGENT "), dot(" Z3L_POS2CAMERA ", " Z3V_NORMAL ")));" },
			{ MM_PARALLAXMAP,                                0, "vec2 " Z3V_TEXCOORD " = " Z3V_TEXCOORD ";"
			                                                    "float " Z3L_PARALLAXDEPTH " = texture2D(" Z3U_PARALLAXMAP ", " Z3V_TEXCOORD ").r;"
			                                                    Z3V_TEXCOORD " += " Z3S_CAMERATANGENT ".xy * (" Z3L_PARALLAXDEPTH " * " Z3U_PARALLAXSCALE ");" },
			{ MM_NORMALMAP,                      MM_NORMALFUNC, Z3S_NORMAL " = texture2D(" Z3U_NORMALMAP ", " Z3V_TEXCOORD ").xyz * 2. - 1.;" },
			{ MM_NORMALFUNC,                      MM_NORMALMAP, Z3S_NORMAL " = CalcNormal();" },
			{ MM_NORMALMAP|MM_NORMALFUNC,                    0, Z3S_NORMAL " = normalize(" Z3V_NORMAL " * " Z3S_NORMAL ".z + " Z3V_TANGENT " * " Z3S_NORMAL ".x + " Z3V_BITANGENT " * " Z3S_NORMAL ".y);" },
			{ MMUSE_NORMAL,         MM_NORMALMAP|MM_NORMALFUNC, Z3S_NORMAL " = normalize(" Z3V_NORMAL ");" },
			{ MMDEF_FRAGCOLORMODES,       -MMUSE_LATECOLORCALC, Z3O_FRAGCOLOR " = " },
			{ MMDEF_FRAGCOLORMODES,       -MMUSE_LATECOLORCALC, 0 }, // <-- FragColor Calculation
			{ MO_MASKED,                  -MMUSE_LATECOLORCALC, "if (" Z3O_FRAGCOLOR ".a<.5)discard;" },
			{ MM_PARALLAXMAP,                                 0, Z3O_FRAGCOLOR ".rgb *= .5+" Z3L_PARALLAXDEPTH "*.5;" },
			{ MM_SPECULARMAP,MM_SPECULARFUNC|MM_SPECULARSTATIC, "float " Z3L_SPECULAR " = texture2D(" Z3U_SPECULARMAP ", " Z3V_TEXCOORD ").r * " Z3U_SPECULAR ";" },
			{ MM_SPECULARFUNC,MM_SPECULARMAP|MM_SPECULARSTATIC, "float " Z3L_SPECULAR " = CalcSpecular();" },
			{ MM_SPECULARSTATIC,MM_SPECULARMAP|MM_SPECULARFUNC, "float " Z3L_SPECULAR " = " Z3U_SPECULAR ";" },
			{ 0,                                      MO_UNLIT, "vec3 " Z3L_LIGHTSCOLOR " = vec3(0.)," Z3L_LIGHTSSHINE " = vec3(0.)," Z3L_DIRECTION2LIGHT ";"
			                                                    "for (int i = 0; i < " Z3S_NUMLIGHTS "; i++)"
			                                                    "{"
			                                                        "if (i > 0 && " Z3U_LIGHTDATA "[1+i*3].x > 3.3e+38) break;"
			                                                        "float " Z3L_LIGHTFACTOR "," Z3L_LIGHTDIM ";"
			                                                        "if (" Z3U_LIGHTDATA "[1+i*3+2].x < 0.)"
			                                                        "{"
			                                                            Z3L_LIGHTFACTOR " = max(dot(" Z3S_NORMAL ", " Z3U_LIGHTDATA "[1+i*3]), 0.);"
			                                                            Z3L_LIGHTDIM " = 1.;" },
			{ MMUSE_SPECULAR,                         MO_UNLIT,         "vec3 " Z3L_POS2LIGHT " =  " Z3U_LIGHTDATA "[1+i*3] * -" Z3U_LIGHTDATA "[1+i*3+2].x - " Z3V_POSITION ";"
			                                                            Z3L_DIRECTION2LIGHT " = " Z3L_POS2LIGHT " / length(" Z3L_POS2LIGHT ");" },
			{ 0,                                      MO_UNLIT,     "}"
			                                                        "else"
			                                                        "{"
			                                                            "vec3 " Z3L_POS2LIGHT " = " Z3U_LIGHTDATA "[1+i*3] - " Z3V_POSITION ";"
			                                                            "float " Z3L_LIGHTDISTANCE " = length(" Z3L_POS2LIGHT ");"
			                                                            Z3L_DIRECTION2LIGHT " = " Z3L_POS2LIGHT " / " Z3L_LIGHTDISTANCE ";"
			                                                            Z3L_LIGHTFACTOR " = max(dot(" Z3S_NORMAL ", " Z3L_DIRECTION2LIGHT "), 0.);"
			                                                            Z3L_LIGHTDIM " = max((" Z3U_LIGHTDATA "[1+i*3+2].x - " Z3L_LIGHTDISTANCE ") / " Z3U_LIGHTDATA "[1+i*3+2].x, 0.);"
			                                                        "}" },
			{ MMUSE_SPECULAR,                         MO_UNLIT,     "float " Z3L_LIGHTSPECULAR " = " Z3L_SPECULAR " * pow(max(dot(normalize(" Z3U_VIEWPOS " - " Z3V_POSITION "), reflect(-" Z3L_DIRECTION2LIGHT ", " Z3S_NORMAL ")), 0.), " Z3U_SHININESS ");"
			                                                        Z3O_FRAGCOLOR ".a = min(" Z3O_FRAGCOLOR ".a + " Z3L_LIGHTSPECULAR ", 1.);"
			                                                        Z3L_LIGHTFACTOR " += " Z3L_LIGHTSPECULAR ";" },
			{ 0,                   MO_RECEIVENOSHADOW|MO_UNLIT,     (const char *)&ExternalSource[EXTERN_FS_ShadowMap_Calc] },
			{ 0,                                      MO_UNLIT,     Z3L_LIGHTFACTOR " *= " Z3L_LIGHTDIM ";"
			                                                        Z3L_LIGHTSSHINE " += " Z3U_LIGHTDATA "[1+i*3+1] * max(" Z3L_LIGHTFACTOR "-1.,0.);"
			                                                        Z3L_LIGHTSCOLOR " += " Z3U_LIGHTDATA "[1+i*3+1] * " Z3L_LIGHTFACTOR ";"
			                                                    "}"
			                                                    //Z3O_FRAGCOLOR ".rgb = " Z3L_LIGHTSCOLOR ";return;"
			                                                    Z3O_FRAGCOLOR ".rgb = " Z3O_FRAGCOLOR ".rgb * (" Z3L_LIGHTSCOLOR "+" Z3U_LIGHTDATA "[0]) + " Z3L_LIGHTSSHINE ";"
			},
			{ 0,0,"}" }
		},
		ShadowMapFSRules[] = {
			{ MO_MASKED, 0, "uniform sampler2D " Z3U_DIFFUSEMAP ";" },
			{ 0,         0, S_VoidMain },
			{ MO_MASKED, 0,     "if (texture2D(" Z3U_DIFFUSEMAP ", " Z3V_TEXCOORD ").a<.5)discard;" },
			{ 0,         0, "}" }
		}
	;

	static GLsizei BuildList(const SourceRule* Rules, size_t RuleCount, unsigned int MM, const char** List, const char*const* NullReplacements = NULL)
	{
		const char** ListStart = List;
		for (const SourceRule *Rule = Rules, *RuleEnd = Rule+RuleCount; Rule != RuleEnd; Rule++)
		{
			if ((Rule->MMUseIf && !(MM & Rule->MMUseIf)) || (Rule->MMLimit > 0 && (MM & Rule->MMLimit)) || (Rule->MMLimit < 0 && !(MM & -Rule->MMLimit))) continue;
			const char* Source = Rule->Source;
			if (Source == NULL && NullReplacements) Source = *(NullReplacements++);
			else if ((void*)Source >= (void*)ExternalSource && (void*)Source < (void*)(ExternalSource+_EXTERN_NUM)) Source = *(const char**)Source;
			if (Source) *(List++) = Source;
		}
		return (GLsizei)(List - ListStart);
	}

	static const char FS_FragColor_StaticColor[] = Z3U_COLOR;
	static const char FS_FragColor_VertexColor[] = Z3V_COLOR;
	static const char FS_FragColor_DiffuseMap[]  = "texture2D(" Z3U_DIFFUSEMAP ", " Z3V_TEXCOORD ")";
	static const char FS_FragColor_DiffuseFunc[] = "CalcDiffuse()";
	SourceRule FragColorRules[] = {
		{ MM_STATICCOLOR,    0, FS_FragColor_StaticColor },
		{ MMUSE_VERTEXCOLOR, 0, FS_FragColor_VertexColor },
		{ MM_DIFFUSEMAP,     0, FS_FragColor_DiffuseMap  },
		{ MM_DIFFUSEFUNC,    0, FS_FragColor_DiffuseFunc },
	};
	enum { FRAGCOLOR_CALC_MAXLEN = COUNT_OF(FS_FragColor_StaticColor)+1+COUNT_OF(FS_FragColor_VertexColor)+1+COUNT_OF(FS_FragColor_DiffuseMap)+1+COUNT_OF(FS_FragColor_DiffuseFunc)+1 };
};

struct ZL_CameraBase_Impl : ZL_Impl
{
	ZL_Matrix VP;
	ZL_Vector3 Pos, Dir;
	GLuint UpdateCount;
	scalar Aspect, Size, Near, Far;
	ZL_CameraBase_Impl() : Dir(ZL_Vector3::Forward) { static GLushort gc; UpdateCount = gc++; }
};

struct ZL_Camera_Impl : public ZL_CameraBase_Impl
{
	ZL_Vector3 AmbientLightColor;
	bool IsOrtho;
	ZL_Camera_Impl() : IsOrtho(false) { Aspect = -1; Size = s(90); Near = s(.1); Far = s(1000); AmbientLightColor = s(.2); UpdateMatrix(); }
	void UpdateMatrix()
	{
		scalar ar = (Aspect > 0 ? Aspect : ZLWIDTH/ZLHEIGHT);
		if (IsOrtho) VP = ZL_Matrix::MakeCamera(Pos, Dir) * ZL_Matrix::MakeOrtho(-Size * ar, Size * ar, -Size, Size, Near, Far);
		else         VP = ZL_Matrix::MakeCamera(Pos, Dir) * ZL_Matrix::MakePerspectiveHorizontal(Size, ar, Near, Far);
		UpdateCount += 0x10000;
	}
};

struct ZL_Light_Impl : public ZL_CameraBase_Impl
{
	scalar Falloff, LightFactorOutside, ShadowBias;
	ZL_Vector3 Color;
	ZL_Matrix BiasedLightMatrix;
	ZL_Light_Impl() : Color(1,1,1) { Aspect = s(0); Size = s(0); Near = s(1); Far = s(50); Falloff = s(3.4e+37); LightFactorOutside = s(0); ShadowBias = s(.001); UpdateMatrix(); }
	void UpdateMatrix()
	{
		if (!Size)        VP = ZL_Matrix::MakeCamera(Pos, Dir);
		else if (!Aspect) VP = ZL_Matrix::MakeCamera(Pos, Dir) * ZL_Matrix::MakeOrtho(-Size, Size, -Size, Size, Near, Far);
		else              VP = ZL_Matrix::MakeCamera(Pos, Dir) * ZL_Matrix::MakePerspectiveHorizontal(Size, Aspect, Near, Far);
		BiasedLightMatrix = VP * ZL_Matrix(.5f, 0, 0, 0, 0, .5f, 0, 0, 0, 0, .5f, 0, .5f, .5f, .5f, 1);
		UpdateCount += 0x10000;
	}
	bool IsDirectionalLight() { return Size && VP.m[15] == 1; }
};

struct ZL_RenderSceneSetup
{
	enum { MAX_LIGHTS = 10 };
	ZL_RenderSceneSetup() : Light(NULL), LightDataCount(0), LightDataChkSum(0) {}
	ZL_RenderSceneSetup(ZL_CameraBase_Impl* Camera, ZL_Light_Impl* Light = NULL) : Camera(Camera), Light(Light), LightDataCount(0), LightDataChkSum(0) {}
	ZL_CameraBase_Impl* Camera;
	ZL_Light_Impl* Light;
	GLsizei LightDataCount;
	GLuint LightDataChkSum;
	ZL_Vector3 LightData[1+MAX_LIGHTS*3+1];
	void CalcLightDataChkSum() { ZL_STATIC_ASSERT(!(sizeof(ZL_Vector3)&3),BAD_ALIGN); LightDataChkSum = ZL_Checksum::Fast4(LightData, sizeof(*LightData)*LightDataCount); }
};

struct ZL_Material_Impl : ZL_Impl
{
	struct UniformArrayValue { scalar *Ptr; GLsizei Count; };
	struct sUniformSet
	{
		GLuint ValueNum, ValueChksum, TextureChksum; scalar* Values; ZL_Texture_Impl* TextureReferences[4];
		void CalcValueChksum()   { ZL_STATIC_ASSERT(!(sizeof(scalar)&3),BAD_ALIGN);            ValueChksum   = ZL_Checksum::Fast4(Values, sizeof(scalar) * ValueNum); }
		void CalcTextureChksum() { ZL_STATIC_ASSERT(!(sizeof(TextureReferences)&3),BAD_ALIGN); TextureChksum = ZL_Checksum::Fast4(TextureReferences, sizeof(TextureReferences)); }
	};
	struct ZL_MaterialProgram* ShaderProgram;
	unsigned int MaterialModes;
	sUniformSet UniformSet;

	static ZL_Material_Impl* GetMaterialReference(unsigned int MM, const char* CustomFragmentCode = NULL, const char* CustomVertexCode = NULL);

	ZL_Material_Impl(struct ZL_MaterialProgram* ShaderProgram, unsigned int MaterialModes) : ShaderProgram(ShaderProgram), MaterialModes(MaterialModes)
	{
		memset(&UniformSet, 0, sizeof(UniformSet));
	}

	~ZL_Material_Impl()
	{
		if (UniformSet.TextureReferences[0]) UniformSet.TextureReferences[0]->DelRef();
		if (UniformSet.TextureReferences[1]) UniformSet.TextureReferences[1]->DelRef();
		if (UniformSet.TextureReferences[2]) UniformSet.TextureReferences[2]->DelRef();
		if (UniformSet.TextureReferences[3]) UniformSet.TextureReferences[3]->DelRef();
		if (UniformSet.Values) free(UniformSet.Values);
	}

	void SetUniform1(GLint Offset, scalar val)
	{
		if (Offset < 0) return;
		UniformSet.Values[Offset] = val;
		UniformSet.CalcValueChksum();
	}
	void SetUniformX(GLint Offset, const scalar* val, int count)
	{
		if (Offset < 0) return;
		memcpy(UniformSet.Values+Offset, val, sizeof(scalar)*count);
		UniformSet.CalcValueChksum();
	}
	float GetUniform1(GLint Offset)
	{
		return (Offset >= 0 ? UniformSet.Values[Offset] : 0);
	}
	void GetUniformX(GLint Offset, scalar* val, int count)
	{
		if (Offset >= 0) memcpy(val, UniformSet.Values+Offset, sizeof(scalar)*count);
	}
	void SetTexture(int Num, ZL_Texture_Impl *Tex)
	{
		assert(Num >= 0 && Num < (int)COUNT_OF(UniformSet.TextureReferences));
		if (UniformSet.TextureReferences[Num] == Tex) return;
		ZL_Impl::CopyRef(Tex, (ZL_Impl*&)UniformSet.TextureReferences[Num]);
		UniformSet.CalcTextureChksum();
	}
	ZL_Texture_Impl* GetTexture(int Num)
	{
		assert(Num >= 0 && Num < (int)COUNT_OF(UniformSet.TextureReferences));
		return UniformSet.TextureReferences[Num];
	}
};

struct ZL_MaterialProgram : ZL_Material_Impl
{
	struct UniformEntry
	{
		bool operator<(const ZL_NameID& b) const { return (Name<b); }
		ZL_NameID Name; GLint Location, ValueOffset, Type;
		enum eType { TYPE_FLOAT = 1, TYPE_VEC2 = 2, TYPE_VEC3 = 3, TYPE_VEC4 = 4, TYPEFLAG_ARRAY = 8 };
	};

	ZL_ShaderIDs ShaderIDs;
	u64 VariationID;
	GLint UniformMatrixView, UniformVectorViewPos, UniformMatrixLight, UniformVectorLightData, UniformTime;
	GLuint UploadedCamera, UploadedLight, UploadedLightDataChkSum, UniformUploadedValueChksum, UniformNum;
	UniformEntry* UniformEntries;
	ZL_MaterialProgram* ShadowMapProgram;

	#ifdef ZL_VIDEO_WEAKCONTEXT
	ZL_String WeakVertexShaderSrc, WeakFragmentShaderSrc;
	void RecreateOnContextLost()
	{
		if (!ShaderIDs.Program) return;
		const char *vs = WeakVertexShaderSrc.c_str(), *fs = WeakFragmentShaderSrc.c_str();
		ShaderIDs.Program = ZLGLSL::CreateProgramFromVertexAndFragmentShaders(1, &vs, 1, &fs, COUNT_OF(ZL_Display3D_Shaders::S_AttributeList), ZL_Display3D_Shaders::S_AttributeList);
		UploadedCamera = UploadedLight = UploadedLightDataChkSum = UniformUploadedValueChksum = 0;
	}
	#endif

	static std::vector<struct ZL_MaterialProgram*>::iterator FindVariation(u64 VariationID)
	{
		struct SortFunc { static inline bool ByVariationID(ZL_MaterialProgram* a, u64 VariationID) { return a->VariationID < VariationID; } };
		return std::lower_bound(g_LoadedShaderVariations->begin(), g_LoadedShaderVariations->end(), VariationID, SortFunc::ByVariationID);
	}

	~ZL_MaterialProgram()
	{
		if (ShaderIDs.Program) glDeleteProgram(ShaderIDs.Program);
		if (VariationID) g_LoadedShaderVariations->erase(FindVariation(VariationID));
		if (ShadowMapProgram) ShadowMapProgram->DelRef();
		if (this == g_ShadowMapProgram) g_ShadowMapProgram = NULL;
		if (this == g_ShadowMapMaskProgram) g_ShadowMapMaskProgram = NULL;
		if (UniformEntries) free(UniformEntries);

		#ifdef ZL_VIDEO_WEAKCONTEXT
		if (ShaderIDs.Program) g_LoadedMaterialPrograms->erase(std::find(g_LoadedMaterialPrograms->begin(), g_LoadedMaterialPrograms->end(), this));
		#endif
	}

	ZL_MaterialProgram(GLsizei vertex_shader_srcs_count, const char **vertex_shader_srcs, GLsizei fragment_shader_srcs_count, const char **fragment_shader_srcs, unsigned int MaterialModes = 0) : ZL_Material_Impl(NULL, MaterialModes), VariationID(0), UploadedCamera(0), UploadedLight(0), UploadedLightDataChkSum(0), UniformUploadedValueChksum(0), UniformEntries(NULL), ShadowMapProgram(NULL)
	{
		ShaderProgram = this;
		if (!(ShaderIDs.Program = ZLGLSL::CreateProgramFromVertexAndFragmentShaders(vertex_shader_srcs_count, vertex_shader_srcs, fragment_shader_srcs_count, fragment_shader_srcs, COUNT_OF(ZL_Display3D_Shaders::S_AttributeList), ZL_Display3D_Shaders::S_AttributeList))) return;
		UniformMatrixView               = glGetUniformLocation(ShaderIDs.Program, Z3U_VIEW);
		ShaderIDs.UniformMatrixModel    = glGetUniformLocation(ShaderIDs.Program, Z3U_MODEL);
		ShaderIDs.UniformMatrixNormal   = glGetUniformLocation(ShaderIDs.Program, Z3U_NORMAL);
		UniformMatrixLight              = glGetUniformLocation(ShaderIDs.Program, Z3U_LIGHT);
		UniformVectorViewPos            = glGetUniformLocation(ShaderIDs.Program, Z3U_VIEWPOS);
		UniformVectorLightData          = glGetUniformLocation(ShaderIDs.Program, Z3U_LIGHTDATA);
		UniformTime                     = glGetUniformLocation(ShaderIDs.Program, Z3U_TIME);
		GLint UniformSamplerDiffuse     = glGetUniformLocation(ShaderIDs.Program, Z3U_DIFFUSEMAP);
		GLint UniformSamplerNormal      = glGetUniformLocation(ShaderIDs.Program, Z3U_NORMALMAP);
		GLint UniformSamplerSpecular    = glGetUniformLocation(ShaderIDs.Program, Z3U_SPECULARMAP);
		GLint UniformSamplerParallax    = glGetUniformLocation(ShaderIDs.Program, Z3U_PARALLAXMAP);
		GLint UniformSamplerShadow      = glGetUniformLocation(ShaderIDs.Program, Z3U_SHADOWMAP);
		GLint UniformFloatSpecular      = glGetUniformLocation(ShaderIDs.Program, Z3U_SPECULAR);
		GLint UniformFloatShininess     = glGetUniformLocation(ShaderIDs.Program, Z3U_SHININESS);
		GLint UniformFloatParallaxScale = glGetUniformLocation(ShaderIDs.Program, Z3U_PARALLAXSCALE);

		GLint AllUniformCount;
		glGetProgramiv(ShaderIDs.Program, GL_ACTIVE_UNIFORMS, &AllUniformCount);

		GLint ActiveProgram;
		glGetIntegerv(GL_CURRENT_PROGRAM, &ActiveProgram);
		glUseProgram(ShaderIDs.Program);
		if (UniformSamplerDiffuse  != -1) glUniform1i(UniformSamplerDiffuse,  0);
		if (UniformSamplerNormal   != -1) glUniform1i(UniformSamplerNormal,   1);
		if (UniformSamplerSpecular != -1) glUniform1i(UniformSamplerSpecular, 2);
		if (UniformSamplerParallax != -1) glUniform1i(UniformSamplerParallax, 3);
		if (UniformSamplerShadow   != -1) glUniform1i(UniformSamplerShadow,   4);

		std::vector<UniformEntry> Entries; std::vector<scalar> Values;
		for (int i = 0; i < AllUniformCount; i++)
		{
			GLsizei UniformNameLen; GLint UniformSize; GLenum UniformType; char UniformName[256];
			glGetActiveUniform(ShaderIDs.Program, i, 256, &UniformNameLen, &UniformSize, &UniformType, UniformName);
			GLint loc = glGetUniformLocation(ShaderIDs.Program, UniformName);
			if (loc == UniformMatrixView || loc == ShaderIDs.UniformMatrixModel || loc == ShaderIDs.UniformMatrixNormal || loc == UniformMatrixLight ||
			    loc == UniformVectorViewPos || loc == UniformVectorLightData || loc == UniformTime ||
			    loc == UniformSamplerDiffuse || loc == UniformSamplerNormal || loc == UniformSamplerSpecular || loc == UniformSamplerParallax || loc == UniformSamplerShadow) continue;
			UniformEntry e;
			scalar v[4] = {1, 1, 1, 1};
			if      (loc == UniformFloatSpecular )     v[0] = s(5);
			else if (loc == UniformFloatShininess)     v[0] = s(16);
			else if (loc == UniformFloatParallaxScale) v[0] = s(0.05);
			if      (UniformType == GL_FLOAT)          { e.Type = UniformEntry::TYPE_FLOAT; glUniform1(loc, v[0]); }
			else if (UniformType == GL_FLOAT_VEC2 )    { e.Type = UniformEntry::TYPE_VEC2;  glUniform2(loc, v[0], v[1]); }
			else if (UniformType == GL_FLOAT_VEC3 )    { e.Type = UniformEntry::TYPE_VEC3;  glUniform3(loc, v[0], v[1], v[2]); }
			else if (UniformType == GL_FLOAT_VEC4 )    { e.Type = UniformEntry::TYPE_VEC4;  glUniform4(loc, v[0], v[1], v[2], v[3]); }
			else continue; //unhandled type
			e.Name = UniformName;
			e.Location = loc;
			std::vector<UniformEntry>::iterator it = std::lower_bound(Entries.begin(), Entries.end(), e.Name);
			if (it == Entries.end() || it->Name != e.Name)
			{
				e.ValueOffset = (GLint)Values.size();
				if (UniformSize <= 1) Values.insert(Values.end(), v, v + (int)e.Type);
				else { Values.insert(Values.end(), (sizeof(UniformArrayValue)+sizeof(scalar)-1) / sizeof(scalar), 0); e.Type |= UniformEntry::TYPEFLAG_ARRAY; }
				Entries.insert(it, e);
				//ZL_LOG3("3D", "[%d] Uniform Name: %s - Type: %d", e.Location, UniformName, e.Type);
			}
		}
		UniformNum = (GLuint)Entries.size();
		if (UniformNum)
		{
			UniformSet.ValueNum = (GLuint)Values.size();
			UniformEntries = (UniformEntry*)malloc(sizeof(UniformEntry) * UniformNum);
			UniformSet.Values = (scalar*)malloc(sizeof(scalar) * UniformSet.ValueNum);
			memcpy(UniformEntries, &Entries[0], sizeof(UniformEntry) * UniformNum);
			memcpy(UniformSet.Values, &Values[0], sizeof(scalar) * UniformSet.ValueNum);
			UniformSet.CalcValueChksum();
			UniformUploadedValueChksum = UniformSet.ValueChksum;
		}

		glUseProgram(ActiveProgram);
		assert(glGetAttribLocation(ShaderIDs.Program, ZL_Display3D_Shaders::S_AttributeList[0]) == 0);
		ShaderIDs.UsedAttributeMask = 0;
		for (GLint loc = 1; loc < (GLint)COUNT_OF(ZL_Display3D_Shaders::S_AttributeList); loc++)
			if (glGetAttribLocation(ShaderIDs.Program, ZL_Display3D_Shaders::S_AttributeList[loc]) != -1) { ShaderIDs.UsedAttributeMask |= 1<<loc; assert(glGetAttribLocation(ShaderIDs.Program, ZL_Display3D_Shaders::S_AttributeList[loc]) == loc); }
		//ZL_LOG4("3D", "Compiled Shader - Program ID: %d - VS Parts: %d - FS Parts: %d - AttrMask: %d", ShaderIDs.Program, vertex_shader_srcs_count, fragment_shader_srcs_count, ShaderIDs.UsedAttributeMask);

		#ifdef ZL_VIDEO_WEAKCONTEXT
		for (GLsizei ivs = 0; ivs < vertex_shader_srcs_count; ivs++) WeakVertexShaderSrc += vertex_shader_srcs[ivs];
		for (GLsizei ifs = 0; ifs < fragment_shader_srcs_count; ifs++) WeakFragmentShaderSrc += fragment_shader_srcs[ifs];
		if (!g_LoadedMaterialPrograms) g_LoadedMaterialPrograms = new std::vector<struct ZL_MaterialProgram*>();
		g_LoadedMaterialPrograms->push_back(this);
		#endif
	}

	void Activate(const sUniformSet& Override, const ZL_RenderSceneSetup& Scene)
	{
		if (g_Active3D.Shader.Program != ShaderIDs.Program)
		{
			g_Active3D.Shader = ShaderIDs;
			glUseProgram(g_Active3D.Shader.Program);
			if (UniformTime != -1) glUniform1(UniformTime, ZLSECONDS);
			if (UploadedCamera != Scene.Camera->UpdateCount)
			{
				glUniformMatrix4v(UniformMatrixView, 1, GL_FALSE, Scene.Camera->VP.m);
				if (UniformVectorViewPos    != -1) glUniform3(UniformVectorViewPos, Scene.Camera->Pos.x, Scene.Camera->Pos.y, Scene.Camera->Pos.z);
				UploadedCamera = Scene.Camera->UpdateCount;
			}
			if (Scene.Light && UploadedLight != Scene.Light->UpdateCount)
			{
				if (UniformMatrixLight      != -1) glUniformMatrix4v(UniformMatrixLight, 1, GL_FALSE, Scene.Light->BiasedLightMatrix.m);
				UploadedLight = Scene.Light->UpdateCount;
			}
			if (UploadedLightDataChkSum != Scene.LightDataChkSum)
			{
				if (UniformVectorLightData  != -1) glUniform3v(UniformVectorLightData, Scene.LightDataCount, (GLscalar*)Scene.LightData);
				UploadedLightDataChkSum = Scene.LightDataChkSum;
			}
		}
		if (UniformUploadedValueChksum != Override.ValueChksum)
		{
			assert(Override.ValueNum >= UniformSet.ValueNum);
			for (UniformEntry* e = UniformEntries, *eEnd = e + UniformNum; e != eEnd; e++)
			{
				const scalar* v = Override.Values + e->ValueOffset;
				switch (e->Type)
				{
					case UniformEntry::TYPE_FLOAT: glUniform1(e->Location, v[0]); break;
					case UniformEntry::TYPE_VEC2:  glUniform2(e->Location, v[0], v[1]); break;
					case UniformEntry::TYPE_VEC3:  glUniform3(e->Location, v[0], v[1], v[2]); break;
					case UniformEntry::TYPE_VEC4:  glUniform4(e->Location, v[0], v[1], v[2], v[3]); break;
					case UniformEntry::TYPE_VEC3|UniformEntry::TYPEFLAG_ARRAY: glUniform3v(e->Location, ((UniformArrayValue*)v)->Count, ((UniformArrayValue*)v)->Ptr); break;
					default: assert(0);
				}
			}
			UniformUploadedValueChksum = Override.ValueChksum;
		}
		if (g_Active3D.BoundTextureChksum != Override.TextureChksum)
		{
			if (Override.TextureReferences[0] && Override.TextureReferences[0]->gltexid != g_Active3D.BoundTextures[0]) { if (g_Active3D.Texture != GL_TEXTURE0) glActiveTexture(g_Active3D.Texture = GL_TEXTURE0); glBindTexture(GL_TEXTURE_2D, Override.TextureReferences[0]->gltexid); }
			if (Override.TextureReferences[1] && Override.TextureReferences[1]->gltexid != g_Active3D.BoundTextures[1]) { if (g_Active3D.Texture != GL_TEXTURE1) glActiveTexture(g_Active3D.Texture = GL_TEXTURE1); glBindTexture(GL_TEXTURE_2D, Override.TextureReferences[1]->gltexid); }
			if (Override.TextureReferences[2] && Override.TextureReferences[2]->gltexid != g_Active3D.BoundTextures[2]) { if (g_Active3D.Texture != GL_TEXTURE2) glActiveTexture(g_Active3D.Texture = GL_TEXTURE2); glBindTexture(GL_TEXTURE_2D, Override.TextureReferences[2]->gltexid); }
			if (Override.TextureReferences[3] && Override.TextureReferences[3]->gltexid != g_Active3D.BoundTextures[3]) { if (g_Active3D.Texture != GL_TEXTURE3) glActiveTexture(g_Active3D.Texture = GL_TEXTURE3); glBindTexture(GL_TEXTURE_2D, Override.TextureReferences[3]->gltexid); }
			g_Active3D.BoundTextureChksum = Override.TextureChksum;
		}
	}

	inline GLint GetUniformOffset(ZL_NameID Name, UniformEntry::eType Type, bool IsArray = false) const
	{
		UniformEntry* e = std::lower_bound(UniformEntries, UniformEntries+UniformNum, Name);
		if (e == UniformEntries+UniformNum || e->Name != Name) return -1;
		assert(e->Type == (Type | (IsArray ? UniformEntry::TYPEFLAG_ARRAY : 0)));
		return UniformEntries[e - UniformEntries].ValueOffset;
	}
};

struct ZL_MaterialInstance : ZL_Material_Impl
{
	ZL_MaterialInstance(ZL_Material_Impl *Base, unsigned int MaterialModes, bool ResetUniforms) : ZL_Material_Impl(Base->ShaderProgram, MaterialModes)
	{
		assert((Base->MaterialModes & ~ZL_Display3D_Shaders::MMDEF_NOSHADERCODE) == (MaterialModes & ~ZL_Display3D_Shaders::MMDEF_NOSHADERCODE));
		ShaderProgram->AddRef();
		UniformSet.ValueNum = Base->UniformSet.ValueNum;
		UniformSet.Values = (scalar*)malloc(sizeof(scalar) * UniformSet.ValueNum);
		if (!ResetUniforms)
		{
			memcpy(UniformSet.Values, Base->UniformSet.Values, sizeof(scalar) * ShaderProgram->UniformSet.ValueNum);
			UniformSet.ValueChksum = Base->UniformSet.ValueChksum;
			for (int i = 0; i < (int)COUNT_OF(UniformSet.TextureReferences); i++) SetTexture(i, Base->GetTexture(i));
		}
		else
		{
			for (ZL_MaterialProgram::UniformEntry *e = ShaderProgram->UniformEntries, *eEnd = e + ShaderProgram->UniformNum; e != eEnd; e++)
			{
				scalar* v = UniformSet.Values + e->ValueOffset;
				switch (e->Type)
				{
					case ZL_MaterialProgram::UniformEntry::TYPE_VEC4: v[0] = v[1] = v[2] = v[3] = 1; break;
					case ZL_MaterialProgram::UniformEntry::TYPE_VEC3: v[0] = v[1] = v[2] = 1; break;
					case ZL_MaterialProgram::UniformEntry::TYPE_VEC2: v[0] = v[1] = 1; break;
					case ZL_MaterialProgram::UniformEntry::TYPE_FLOAT:
						if      (e->Name == Z3U_SPECULAR)      v[0] = s(5);
						else if (e->Name == Z3U_SHININESS)     v[0] = s(16);
						else if (e->Name == Z3U_PARALLAXSCALE) v[0] = s(0.05);
						else                                   v[0] = 1;
				}
			}
			UniformSet.CalcValueChksum();
		}
	}

	~ZL_MaterialInstance() { ShaderProgram->DelRef(); }
};

ZL_Material_Impl* ZL_Material_Impl::GetMaterialReference(unsigned int MM, const char* CustomFragmentCode, const char* CustomVertexCode)
{
	using namespace ZL_Display3D_Shaders;
	assert(!CustomFragmentCode || (MM & MMDEF_USECUSTOMFRAGMENT));
	assert(!CustomVertexCode || (MM & MMDEF_USECUSTOMVERTEX));
	assert(CustomFragmentCode || CustomVertexCode || !(MM & MMDEF_REQUESTS)); //requests only valid for custom code
	assert(MM & MMDEF_FRAGCOLORMODES); //need at least one material mode defining the output fragment color
	assert(!(MM & MO_UNLIT) || !(MM & MMDEF_USESLIT)); //confirm that if things that need lit are set lit is also set
	if (!(MM & MO_RECEIVENOSHADOW) && !(MM & MO_UNLIT) && !g_SetupShadowMapProgram) MM |= MO_RECEIVENOSHADOW;
	if (!(MM & MMDEF_FRAGCOLORMODES)) MM |= MM_STATICCOLOR;
	if (MM & MO_UNLIT) MM &= ~MMDEF_USESLIT;

	u64 VariationID = (MM & ~MMDEF_NOSHADERCODE); //normalize MaterialModes
	if ((MM & MR_POSITION)      && (MM & (MMUSE_POSITION     ^MR_POSITION     ))) VariationID ^= MR_POSITION;
	if ((MM & MR_TEXCOORD)      && (MM & (MMUSE_TEXCOORD     ^MR_TEXCOORD     ))) VariationID ^= MR_TEXCOORD;
	if ((MM & MR_NORMAL)        && (MM & (MMUSE_NORMAL       ^MR_NORMAL       ))) VariationID ^= MR_NORMAL;
	if ((MM & MR_CAMERATANGENT) && (MM & (MMUSE_CAMERATANGENT^MR_CAMERATANGENT))) VariationID ^= MR_CAMERATANGENT;
	if (CustomFragmentCode) VariationID ^= (((u64)(ZL_NameID(CustomFragmentCode).IDValue)) << 32);
	if (CustomVertexCode  ) VariationID ^= (((u64)(ZL_NameID(CustomVertexCode  ).IDValue)) << 32);

	ZL_Material_Impl* res;
	std::vector<ZL_MaterialProgram*>::iterator it;
	if (!g_LoadedShaderVariations) g_LoadedShaderVariations = new std::vector<struct ZL_MaterialProgram*>();
	if ((it = ZL_MaterialProgram::FindVariation(VariationID)) != g_LoadedShaderVariations->end() && (*it)->VariationID == VariationID)
	{
		res = new ZL_MaterialInstance(*it, MM, true);
	}
	else
	{
		char FS_FragColorCalc[FRAGCOLOR_CALC_MAXLEN+1], *FS_FragColorCalcPtr = FS_FragColorCalc;
		for (size_t i = 0; i < COUNT_OF(FragColorRules); i++)
			if (MM & FragColorRules[i].MMUseIf)
				FS_FragColorCalcPtr += sprintf(FS_FragColorCalcPtr, "%s%s", (FS_FragColorCalcPtr != FS_FragColorCalc ? "*" : ""), FragColorRules[i].Source);
		FS_FragColorCalcPtr[0] = ';'; FS_FragColorCalcPtr[1] = '\0';

		const unsigned int MMRules = (MM & MO_UNLIT ? MM : MM | MR_POSITION | MR_NORMAL);
		const char* FSNullReplacements[] = { CustomFragmentCode, FS_FragColorCalc };
		const char *VS[COUNT_OF(SharedRules)+COUNT_OF(VSGlobalRules)+COUNT_OF(VSRules)], *FS[COUNT_OF(SharedRules)+COUNT_OF(FSRules)];
		GLsizei VSCount  = BuildList(SharedRules,   COUNT_OF(SharedRules),   MMRules, &VS[      0]);
		        VSCount += BuildList(VSGlobalRules, COUNT_OF(VSGlobalRules), MMRules, &VS[VSCount]);
		        VSCount += BuildList(VSRules,       COUNT_OF(VSRules),       MMRules, &VS[VSCount], &CustomVertexCode);
		GLsizei FSCount  = BuildList(SharedRules,   COUNT_OF(SharedRules),   MMRules, &FS[      0]);
		        FSCount += BuildList(FSRules,       COUNT_OF(FSRules),       MMRules, &FS[FSCount], FSNullReplacements);
		res = new ZL_MaterialProgram(VSCount, VS, FSCount, FS, MM);
		if (!res->ShaderProgram->ShaderIDs.Program) { res->ShaderProgram->MaterialModes = 0; delete res; return NULL; }
		((ZL_MaterialProgram*)res)->VariationID = VariationID;

		g_LoadedShaderVariations->insert(it, res->ShaderProgram);
	}

	if (!res->ShaderProgram->ShadowMapProgram && g_SetupShadowMapProgram && !(MM & MO_CASTNOSHADOW))
		g_SetupShadowMapProgram(res->ShaderProgram, MM, CustomVertexCode);

	return res;
}

struct ZL_Mesh_Impl : ZL_Impl
{
	enum { VA_POS = 0, VA_NORMAL = 1, VAMASK_NORMAL = 2, VA_TEXCOORD = 2, VAMASK_TEXCOORD = 4, VA_TANGENT = 3, VAMASK_TANGENT = 8, VA_COLOR = 4, VAMASK_COLOR = 16 };

	GLuint IndexBufferObject, VertexBufferObject;
	GLubyte ProvideAttributeMask;
	GLsizei Stride;
	GLvoid *NormalOffsetPtr, *TexCoordOffsetPtr, *ColorOffsetPtr, *TangentOffsetPtr;
	struct MeshPart
	{
		ZL_NameID Name; GLsizei IndexCount; GLushort* IndexOffsetPtr; ZL_Material_Impl* Material;
		MeshPart(ZL_NameID Name, GLsizei IndexCount, GLushort* IndexOffsetPtr,  ZL_Material_Impl* Material, bool AddMatRef) : Name(Name), IndexCount(IndexCount), IndexOffsetPtr(IndexOffsetPtr), Material(Material) { if (Material && AddMatRef) Material->AddRef(); }
		bool operator==(ZL_NameID n) { return Name == n; }
	};
	MeshPart *Parts, *PartsEnd;

	#ifdef ZL_VIDEO_WEAKCONTEXT
	bool WeakIsAnimatedMesh;
	GLushort* WeakIndices;  GLsizei WeakIndicesSize;
	GLvoid*   WeakVertData; GLsizei WeakVertDataSize;
	void RecreateOnContextLost()
	{
		glGenBuffers(2, &IndexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, WeakIndicesSize, WeakIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, WeakVertDataSize, WeakVertData, GL_STATIC_DRAW);
	}
	#endif

	ZL_Mesh_Impl(GLubyte AttributeMask) : IndexBufferObject(0), VertexBufferObject(0), ProvideAttributeMask(AttributeMask)
	{
		Stride = sizeof(GLscalar) * (3 + ((AttributeMask & VAMASK_NORMAL) ? 3 : 0) + ((AttributeMask & VAMASK_TEXCOORD) ? 2 : 0) + ((AttributeMask & VAMASK_TANGENT) ? 3 : 0) + ((AttributeMask & VAMASK_COLOR) ? 1 : 0));
		int ScalarOffset = 3;
		if (AttributeMask & VAMASK_NORMAL)   { NormalOffsetPtr   = (GLvoid*)(ScalarOffset * sizeof(GLscalar)); ScalarOffset += 3; }
		if (AttributeMask & VAMASK_TEXCOORD) { TexCoordOffsetPtr = (GLvoid*)(ScalarOffset * sizeof(GLscalar)); ScalarOffset += 2; }
		if (AttributeMask & VAMASK_TANGENT)  { TangentOffsetPtr  = (GLvoid*)(ScalarOffset * sizeof(GLscalar)); ScalarOffset += 3; }
		if (AttributeMask & VAMASK_COLOR)    { ColorOffsetPtr    = (GLvoid*)(ScalarOffset * sizeof(GLscalar)); ScalarOffset += 1; }
	}

	static ZL_Mesh_Impl* Make(GLubyte AttributeMask, const GLushort* Indices, GLsizei IndicesCount, const GLvoid* VertData, GLsizei VertCount, ZL_Material_Impl* Program)
	{
		ZL_Mesh_Impl* res = new ZL_Mesh_Impl(AttributeMask);
		res->CreateAndFillBufferData(Indices, IndicesCount, VertData, VertCount);
		res->Parts = (MeshPart*)malloc(sizeof(MeshPart));
		res->Parts[0] = MeshPart(ZL_NameID(), IndicesCount, NULL, Program, true);
		res->PartsEnd = res->Parts+1;
		return res;
	}

	~ZL_Mesh_Impl()
	{
		if (IndexBufferObject) glDeleteBuffers(2, &IndexBufferObject);
		for (MeshPart* it = Parts; it != PartsEnd; ++it) it->Material->DelRef();
		free(Parts);

		#ifdef ZL_VIDEO_WEAKCONTEXT
		if (IndexBufferObject) { free(WeakIndices); free(WeakVertData); g_LoadedMeshes->erase(std::find(g_LoadedMeshes->begin(), g_LoadedMeshes->end(), this)); }
		#endif
	}

	#if defined(ZILLALOG) && !defined(ZL_VIDEO_OPENGL_ES2)
	void DrawDebug(const ZL_Matrix& Matrix, const ZL_Camera& cam)
	{
		if (!VertexBufferObject || !ProvideAttributeMask) return;

		GLint BufferSize;
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &BufferSize);
		GLvoid* StoredVertData = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		assert((BufferSize%Stride) == 0);assert(StoredVertData);
		glBindBuffer(GL_ARRAY_BUFFER, g_Active3D.VertexBuffer);

		for (GLubyte *v = (GLubyte*)StoredVertData, *vEnd = v + BufferSize; v != vEnd; v += Stride)
		{
			ZL_Vector3 *pos = (ZL_Vector3*)v, tpos = Matrix.TransformPosition(*pos), nrml = ZL_Vector3::Up; GLscalar *ScalarOffset = (GLscalar*)(pos+1);
			if (ProvideAttributeMask & VAMASK_NORMAL)   { nrml = Matrix.TransformDirection(*((ZL_Vector3*)ScalarOffset)); ZL_Display3D::DrawLine(cam, tpos, tpos + nrml * 0.1f, ZL_Color::Green, 0.001f); ScalarOffset += 3; }
			if (ProvideAttributeMask & VAMASK_TEXCOORD) { ZL_Display3D::DrawPlane(cam, tpos, nrml, ZL_Vector(0.05f,0.05f), ZL_Color(ScalarOffset[0], ScalarOffset[1], 0)); ScalarOffset += 2; }
			if (ProvideAttributeMask & VAMASK_TANGENT)  { ZL_Display3D::DrawLine(cam, tpos, tpos + Matrix.TransformDirection(*((ZL_Vector3*)ScalarOffset)) * 0.1f, ZL_Color::Red, 0.001f); ScalarOffset += 3; }
		}

		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glUnmapBuffer(GL_ARRAY_BUFFER);
		glBindBuffer(GL_ARRAY_BUFFER, g_Active3D.VertexBuffer);
	}
	#endif

	void CreateAndFillBufferData(const GLushort* Indices, GLsizei IndicesCount, const GLvoid* VertData, GLsizei VertCount)
	{
		glGenBuffers(2, &IndexBufferObject);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IndexBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * IndicesCount, Indices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_Active3D.IndexBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, Stride * VertCount, VertData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, g_Active3D.VertexBuffer);
		#if defined(ZILLALOG) && (0||ZL_DISPLAY3D_ASSERT_NORMALS_AND_TANGENTS)
		assert(!(IndicesCount%3));
		if (ProvideAttributeMask & VAMASK_NORMAL) for (const GLushort *a = Indices, *aEnd = a+IndicesCount; a != aEnd; a += 3)
		{
			ZL_Vector3 *p1 = (ZL_Vector3*)((char*)VertData+Stride*a[0]), *p2 = (ZL_Vector3*)((char*)VertData+Stride*a[1]), *p3 = (ZL_Vector3*)((char*)VertData+Stride*a[2]), *n1 = p1+1, *n2 = p2+1, *n3 = p3+1;
			ZL_Vector3 Edge21 = (*p2-*p1), Edge31 = (*p3-*p2), EdgeCross = (Edge21 ^ Edge31);
			if (EdgeCross.AlmostZero(KINDA_SMALL_NUMBER)) continue;
			ZL_Vector3 CalcNormal = EdgeCross.VecNormUnsafe(), AvgInpNormal = (*n1+*n2+*n3)/3.0f; scalar AvgInpNormalLen = (n1->GetLength()+n2->GetLength()+n3->GetLength())/3.0f;
			assert((CalcNormal | AvgInpNormal) > s(0.0));
			assert(sabs(AvgInpNormalLen-1.0f) < KINDA_SMALL_NUMBER);
			if ((ProvideAttributeMask & VAMASK_TEXCOORD) && (ProvideAttributeMask & VAMASK_TANGENT))
			{
				ZL_Vector *uv1 = (ZL_Vector*)(n1+1), *uv2 = (ZL_Vector*)(n2+1), *uv3 = (ZL_Vector*)(n3+1); ZL_Vector3 *t1 = (ZL_Vector3*)(uv1+1), *t2 = (ZL_Vector3*)(uv2+1), *t3 = (ZL_Vector3*)(uv3+1);
				ZL_Vector UVDelta21 = *uv2-*uv1, UVDelta31 = *uv3-*uv2;
				float r = 1.0f / (UVDelta21.x * UVDelta31.y - UVDelta21.y * UVDelta31.x);
				ZL_Vector3 tangent = ((Edge21 * UVDelta31.y - Edge31 * UVDelta21.y)*r);
				ZL_Vector3 bitangent = ((Edge31 * UVDelta21.x - Edge21 * UVDelta31.x)*r);
				ZL_Vector3 CalcTangent = tangent.VecNormUnsafe(), AvgInpTangent = (*t1+*t2+*t3)/3.0f; scalar AvgInpTangentLen = (t1->GetLength()+t2->GetLength()+t3->GetLength())/3.0f;
				assert((CalcTangent | AvgInpTangent) > s(0.5));
				assert(sabs(AvgInpTangentLen-1.0f) < KINDA_SMALL_NUMBER);
			}
		}
		#endif

		#ifdef ZL_VIDEO_WEAKCONTEXT
		WeakIsAnimatedMesh = false;
		WeakIndices  = (GLushort*)malloc((WeakIndicesSize  = (sizeof(GLushort) * IndicesCount))); memcpy(WeakIndices,  Indices,  WeakIndicesSize );
		WeakVertData = (GLvoid*  )malloc((WeakVertDataSize = (Stride * VertCount             ))); memcpy(WeakVertData, VertData, WeakVertDataSize);
		if (!g_LoadedMeshes) g_LoadedMeshes = new std::vector<struct ZL_Mesh_Impl*>();
		g_LoadedMeshes->push_back(this);
		#endif
	}

	static unsigned char* ReadMeshFile(const ZL_FileLink& file)
	{
		ZL_File f = file.Open();
		ZL_File_Impl* fileimpl = ZL_ImplFromOwner<ZL_File_Impl>(f);
		if (!fileimpl || !fileimpl->src || fileimpl->src->size() < 2) return NULL;
		unsigned char magic[2];
		fileimpl->src->read(magic, 1, 2);
		fileimpl->src->seektell(0, RW_SEEK_SET);
		if (magic[0] == 'P' && magic[1] == 'K') return ZL_RWopsZIP::ReadSingle(fileimpl->src);
		size_t file_size = fileimpl->src->size();
		unsigned char* ply_buffer = (unsigned char*)malloc(file_size);
		fileimpl->src->read(ply_buffer, 1, file_size);
		return ply_buffer;
	}

	static bool AdvanceLine(unsigned char*& line_end, unsigned char*& cursor)
	{
		while (*line_end < 32) { if (!*line_end) return false; line_end++; }
		cursor = line_end;
		while (*(++line_end) > 31);
		return true;
	}

	static ZL_Mesh_Impl* LoadAny(const ZL_FileLink& file, ZL_Material_Impl* Material)
	{
		unsigned char *buffer = ReadMeshFile(file);
		if (!buffer) return NULL;
		if (buffer[0] == 'p' && buffer[1] == 'l' && buffer[2] == 'y') return PLYLoad(file, Material, buffer);
		return OBJLoad(file, Material, buffer);
	}

	static ZL_Mesh_Impl* PLYLoad(const ZL_FileLink& file, ZL_Material_Impl* Material, unsigned char *ply_buffer = NULL)
	{
		if (!ply_buffer) ply_buffer = ReadMeshFile(file);
		assert(ply_buffer);
		if (!ply_buffer) { ZL_LOG0("3D", "PLY Loader failed - No input file"); return NULL; }
		enum { PROP_X, PROP_Y, PROP_Z, PROP_NX, PROP_NY, PROP_NZ, PROP_S, PROP_T, PROP_RED, PROP_GREEN, PROP_BLUE, PROP_INT_RED, PROP_INT_GREEN, PROP_INT_BLUE, PROP_UNKNOWN };
		std::vector<unsigned char> PropTypes;
		ZL_Mesh_Impl* res = NULL;
		GLvoid *VertData = NULL; GLubyte *VertDataCursor = NULL, *VertDataCursorEnd = NULL;
		unsigned char AttributeMask = 0;
		std::vector<GLushort> Indices;
		GLsizei prop_count = 0, vertex_count = 0, face_count = 0;
		enum { PARSEMODE_HEADER, PARSEMODE_VERTICES, PARSEMODE_INDICES } Mode = PARSEMODE_HEADER;
		for (unsigned char *line_end = ply_buffer, *cursor; AdvanceLine(line_end, cursor);)
		{
			if (Mode == PARSEMODE_HEADER)
			{
				size_t line_length = line_end - cursor;
				if      (line_length > 15 && cursor[0] == 'e' && cursor[8] == 'v') vertex_count = atoi((char*)cursor+15); //element vertex
				else if (line_length > 13 && cursor[0] == 'e' && cursor[8] == 'f') face_count = atoi((char*)cursor+13); //element face
				else if (line_length > 10 && cursor[0] == 'p' && cursor[9] != 'l') //property
				{
					unsigned char PropType = PROP_UNKNOWN, *p = cursor + 9;
					bool is_int = (*p != 'f' && *p != 'd');
					while (*p > ' ') p++;
					if (*p < ' ') {}
					else if (p[1] == 'x' || p[1] == 'y' || p[1] == 'z') PropType = (PROP_X + (p[1] - 'x'));
					else if (p[1] == 'n' && (p[2] == 'x' || p[2] == 'y' || p[2] == 'z')) PropType = (PROP_NX + (p[2] - 'x')), AttributeMask |= VAMASK_NORMAL;
					else if (p[1] == 's' || p[1] == 't') PropType = (PROP_S + (p[1] - 's')), AttributeMask |= VAMASK_TEXCOORD;
					else if (p[1] == 'r') PropType = (is_int ? PROP_INT_RED : PROP_RED), AttributeMask |= VAMASK_COLOR;
					else if (p[1] == 'g') PropType = (is_int ? PROP_INT_GREEN : PROP_GREEN), AttributeMask |= VAMASK_COLOR;
					else if (p[1] == 'b') PropType = (is_int ? PROP_INT_BLUE : PROP_BLUE), AttributeMask |= VAMASK_COLOR;
					PropTypes.push_back(PropType);
					prop_count++;
				}
				else if (line_length == 10 && cursor[0] == 'e' && cursor[1] == 'n') //end_header
				{
					if (!prop_count || !vertex_count || !face_count) break;
					res = new ZL_Mesh_Impl(AttributeMask);
					VertDataCursor = (GLubyte*)(VertData = malloc(res->Stride * vertex_count));
					VertDataCursorEnd = VertDataCursor + res->Stride * vertex_count;
					Mode = PARSEMODE_VERTICES;
				}
			}
			else if (Mode == PARSEMODE_VERTICES)
			{
				GLscalar* OutXYZ  = (GLscalar*)VertDataCursor;
				GLscalar* OutNXYZ = (GLscalar*)(VertDataCursor+(size_t)res->NormalOffsetPtr);
				GLscalar* OutUV   = (GLscalar*)(VertDataCursor+(size_t)res->TexCoordOffsetPtr);
				GLubyte* OutRGBA  =  (GLubyte*)(VertDataCursor+(size_t)res->ColorOffsetPtr);
				VertDataCursor += res->Stride;
				for (GLsizei i = 0; i < prop_count; i++)
				{
					while (cursor != line_end && *cursor <= ' ') cursor++;
					if (cursor == line_end) break;
					double n = strtod((char*)cursor, (char**)&cursor);
					int PropType = PropTypes[i];
					if      (PropType <= PROP_Z) OutXYZ[PropType] = (GLscalar)n;
					else if (PropType <= PROP_NZ) OutNXYZ[PropType-PROP_NX] = (GLscalar)n;
					else if (PropType <= PROP_T) OutUV[PropType-PROP_S] = (GLscalar)n;
					else if (PropType <= PROP_BLUE) OutRGBA[PropType-PROP_RED] = (GLubyte)(n*255.9999);
					else if (PropType <= PROP_INT_BLUE) OutRGBA[PropType-PROP_INT_RED] = (GLubyte)n;
				}
				if (AttributeMask & VAMASK_COLOR) OutRGBA[3] = 255;
				if (VertDataCursor == VertDataCursorEnd) Mode = PARSEMODE_INDICES;
			}
			else if (Mode == PARSEMODE_INDICES)
			{
				GLushort pvind[3];
				for (int i = 0, icount = atoi((char*)cursor); i < icount && cursor != line_end; i++)
				{
					while (cursor != line_end && *cursor != ' ') cursor++;
					if (i > 2) pvind[1] = pvind[2];
					pvind[i > 1 ? 2 : i] = atoi((char*)cursor);
					if (i > 1) Indices.insert(Indices.end(), pvind, pvind+3);
					if (cursor != line_end) cursor++;
				}
				if (--face_count == 0) break;
			}
		}
		if (Indices.empty()) { ZL_LOG0("3D", "PLY Loader failed - No vertex data");  free(VertData); delete res; return NULL; }
		res->CreateAndFillBufferData(&Indices[0], (GLsizei)Indices.size(), VertData, vertex_count);
		free(VertData);
		res->Parts = (MeshPart*)malloc(sizeof(MeshPart));
		res->Parts[0] = MeshPart(ZL_NameID(), (GLsizei)Indices.size(), NULL, Material, true);
		res->PartsEnd = res->Parts+1;
		ZL_LOG3("3D", "Loaded PLY - Verts: %d - Indices: %d - Parts: %d", vertex_count, Indices.size(), 1);
		return res;
	}

	static u64 OBJReadNextCombinedIndex(unsigned char*& cursor)
	{
		while (*cursor > ' ') cursor++;
		u64 ind = 0;
		for (int i = 40; (i==40 && *cursor == ' ') || (i>=0 && *cursor == '/'); i-=20) { ind |= ((u64)atoi((char*)++cursor)) << i; while (*cursor > '/') cursor++; }
		return ind;
	}

	static ZL_Mesh_Impl* OBJLoad(const ZL_FileLink& file, ZL_Material_Impl* Material, unsigned char *obj_buffer = NULL, ZL_Mesh_Impl* (*MakeFunc)(GLubyte AttributeMask) = NULL, std::vector<unsigned int>* used_indices = NULL)
	{
		if (!obj_buffer) obj_buffer = ReadMeshFile(file);
		if (!obj_buffer) return NULL;

		std::vector<u64> unique_indices;
		unsigned int index_counter = 0;
		for (unsigned char* f = obj_buffer; (f = (unsigned char*)strstr((char*)f, "\nf ")) && *++f;)
		{
			for (u64 ind; (ind = OBJReadNextCombinedIndex(f)); index_counter++)
			{
				std::vector<u64>::iterator it = std::lower_bound(unique_indices.begin(), unique_indices.end(), ind);
				if (it != unique_indices.end() && *it == ind) continue;
				if (used_indices) used_indices->insert(used_indices->begin() + (it - unique_indices.begin()), index_counter);
				unique_indices.insert(it, ind);
			}
		}
		assert(!unique_indices.empty()); if (unique_indices.empty()) { free(obj_buffer); return NULL; }

		std::vector<GLushort> Indices;
		std::vector<GLscalar> Positions, Normals, TexCoords;
		std::vector<MeshPart> Parts;
		MeshPart NextPart(ZL_NameID(), 0, NULL, NULL, false);
		for (unsigned char *line_end = obj_buffer, *cursor; AdvanceLine(line_end, cursor);)
		{
			if (cursor[0] == 'u' && cursor[1] == 's' && Indices.size() != (size_t)(NextPart.IndexOffsetPtr - (GLushort*)NULL)) //usemtl end
			{
				NextPart.IndexCount = (GLsizei)(Indices.size() - (size_t)(NextPart.IndexOffsetPtr - (GLushort*)NULL));
				Parts.push_back(NextPart);
				NextPart.IndexOffsetPtr += NextPart.IndexCount;
			}
			if (cursor[0] == 'u' && cursor[1] == 's' && line_end > cursor + 7) NextPart.Name = ZL_NameID((char*)cursor + 7, line_end - cursor - 7); //usemtl start
			else if (cursor[0] == 'f' && cursor[1] == ' ') //face
			{
				GLushort pvind[3], vcount = 0;
				for (u64 ind; (ind = OBJReadNextCombinedIndex(cursor)); vcount++)
				{
					if (vcount > 2) pvind[1] = pvind[2];
					pvind[vcount > 1 ? 2 : vcount] = (GLushort)(std::lower_bound(unique_indices.begin(), unique_indices.end(), ind) - unique_indices.begin());
					if (vcount > 1) Indices.insert(Indices.end(), pvind, pvind+3);
				}
			}
			else if (cursor[0] == 'v' && (cursor[1] == ' ' || cursor[1] == 'n' || cursor[1] == 't')) //vertex data (position, normal or texcoord)
			{
				cursor += (cursor[1] == ' ' ? 2 : 3);
				std::vector<GLscalar>& Vector = (cursor[-2] == 'n' ? Normals : (cursor[-2] == 't' ? TexCoords : Positions));
				for (int floatcount = (cursor[-2] == 't' ? 2 : 3); *cursor >= ' ' && floatcount--;)
					Vector.push_back((GLscalar)strtod((char*)cursor, (char**)&cursor));
			}
		}
		if (Indices.size() != (size_t)(NextPart.IndexOffsetPtr - (GLushort*)NULL))
		{
			NextPart.IndexCount = (GLsizei)(Indices.size() - (size_t)(NextPart.IndexOffsetPtr - (GLushort*)NULL));
			Parts.push_back(NextPart);
		}
		free(obj_buffer);
		assert(!Indices.empty() && !Positions.empty()); if (Indices.empty() || Positions.empty()) return NULL;

		bool HasNormals = !Normals.empty(), HasTexCoords = !TexCoords.empty();
		GLubyte AttributeMask = ((HasNormals ? VAMASK_NORMAL : 0) | (HasTexCoords ? VAMASK_TEXCOORD : 0));
		ZL_Mesh_Impl* res = (MakeFunc ? MakeFunc(AttributeMask) : new ZL_Mesh_Impl(AttributeMask));
		GLvoid* VertData = malloc(res->Stride * unique_indices.size());
		GLscalar *Out = (GLscalar*)VertData, Empties[3] = {0,0,0}, *SrcPosition = &Positions[0], *SrcNormal = (HasNormals ? &Normals[0] : 0), *SrcTexCoord = (HasTexCoords ? &TexCoords[0] : 0);
		for (std::vector<u64>::iterator it = unique_indices.begin(); it != unique_indices.end(); ++it)
		{
			            GLsizei Idx = ((*it>>40)        ); memcpy(Out, (Idx ? SrcPosition + (Idx-1)*3 : Empties), sizeof(GLscalar)*3); Out += 3;
			if (HasNormals)   { Idx = ((*it    )&0xFFFFF); memcpy(Out, (Idx ? SrcNormal   + (Idx-1)*3 : Empties), sizeof(GLscalar)*3); Out += 3; }
			if (HasTexCoords) { Idx = ((*it>>20)&0xFFFFF); memcpy(Out, (Idx ? SrcTexCoord + (Idx-1)*2 : Empties), sizeof(GLscalar)*2); Out += 2; }
		}
		res->CreateAndFillBufferData(&Indices[0], (GLsizei)Indices.size(), VertData, (GLsizei)unique_indices.size());
		free(VertData);
		res->Parts = (MeshPart*)malloc(sizeof(MeshPart) * Parts.size());
		memcpy(res->Parts, &Parts[0], sizeof(MeshPart) * Parts.size());
		res->PartsEnd = res->Parts + Parts.size();
		for (MeshPart* itMP = res->Parts; itMP != res->PartsEnd; ++itMP) { Material->AddRef(); itMP->Material = Material; }
		ZL_LOG3("3D", "Loaded OBJ - Verts: %d - Indices: %d - Parts: %d", unique_indices.size(), Indices.size(), Parts.size());
		return res;
	}

	void LoadMTLFile(unsigned char* buf)
	{
		ZL_MaterialInstance *mat = NULL;
		for (unsigned char *line_end = buf, *line, *p; AdvanceLine(line_end, line);)
		{
			size_t line_length = line_end - line;
			if (line_length > 7 && !memcmp(line, "newmtl ", 7))
			{
				ZL_Mesh_Impl::MeshPart* it = std::find(Parts, PartsEnd, ZL_NameID((char*)line+7, line_length-7));
				if (it == PartsEnd) mat = NULL;
				else { mat = new ZL_MaterialInstance(it->Material, it->Material->MaterialModes, false); it->Material->DelRef(); it->Material = mat; }
				continue;
			}
			if (!mat) continue;
			const GLint OffSpecular = mat->ShaderProgram->GetUniformOffset(Z3U_SPECULAR, ZL_MaterialProgram::UniformEntry::TYPE_FLOAT);
			const GLint OffColor = mat->ShaderProgram->GetUniformOffset(Z3U_COLOR, ZL_MaterialProgram::UniformEntry::TYPE_VEC4);

			GLscalar param[4];
			size_t param_count = 0;
			for (p = line; p != line_end && *p > ' ';) p++;
			for (unsigned char *pEnd; param_count < 4; p = pEnd, param_count++)
			{
				while (p != line_end && *p <= ' ') p++;
				if (p == line_end) break;
				param[param_count] = (GLscalar)strtod((char*)p, (char**)&pEnd);
				if (pEnd == p) break;
			}
			if      (param_count == 1 && line[0] == 'N' && line[1] == 's') mat->SetUniform1(OffSpecular, param[0]*0.01f);
			else if (param_count == 3 && line[0] == 'K' && line[1] == 'd') { ZL_Color c = ZL_Color(param[0], param[1], param[2], mat->GetUniform1(OffColor+3)); mat->SetUniformX(OffColor, (scalar*)&c, 4); }
			else if (param_count == 1 && line[0] == 'd' && line[1] == ' ') { ZL_Color c; mat->GetUniformX(OffColor, (scalar*)&c, 4); c.a = param[0]; mat->SetUniformX(OffColor, (scalar*)&c, 4); }
		}
	}

	void DrawPart(MeshPart* p, const ZL_Matrix& ModelMatrix, const ZL_Matrix& NormalMatrix) const
	{
		if (g_Active3D.Shader.UniformMatrixModel  != -1) glUniformMatrix4v(g_Active3D.Shader.UniformMatrixModel, 1, GL_FALSE, ModelMatrix.m);
		if (g_Active3D.Shader.UniformMatrixNormal != -1) glUniformMatrix4v(g_Active3D.Shader.UniformMatrixNormal, 1, GL_FALSE, NormalMatrix.m);
		GLuint RenderAttributeMask = (g_Active3D.Shader.UsedAttributeMask & ProvideAttributeMask), ChangeMask = (RenderAttributeMask ^ g_Active3D.AttributeMask);
		if (ChangeMask)
		{
			if (ChangeMask & VAMASK_NORMAL  ) { if (RenderAttributeMask & VAMASK_NORMAL  ) glEnableVertexAttribArray(VA_NORMAL  ); else glDisableVertexAttribArray(VA_NORMAL  ); }
			if (ChangeMask & VAMASK_TEXCOORD) { if (RenderAttributeMask & VAMASK_TEXCOORD) glEnableVertexAttribArray(VA_TEXCOORD); else glDisableVertexAttribArray(VA_TEXCOORD); }
			if (ChangeMask & VAMASK_TANGENT ) { if (RenderAttributeMask & VAMASK_TANGENT ) glEnableVertexAttribArray(VA_TANGENT ); else glDisableVertexAttribArray(VA_TANGENT ); }
			if (ChangeMask & VAMASK_COLOR   ) { if (RenderAttributeMask & VAMASK_COLOR   ) glEnableVertexAttribArray(VA_COLOR   ); else glDisableVertexAttribArray(VA_COLOR   ); }
			g_Active3D.AttributeMask = RenderAttributeMask;
			goto UpdateVertexBuffer;
		}
		if (g_Active3D.VertexBuffer != VertexBufferObject)
		{
			UpdateVertexBuffer:
			glBindBuffer(GL_ARRAY_BUFFER, (g_Active3D.VertexBuffer = VertexBufferObject));
			                                           glVertexAttribPointer(VA_POS,      3, GL_SCALAR,        GL_FALSE, Stride, NULL);
			if (RenderAttributeMask & VAMASK_NORMAL  ) glVertexAttribPointer(VA_NORMAL,   3, GL_SCALAR,        GL_FALSE, Stride, NormalOffsetPtr);
			if (RenderAttributeMask & VAMASK_TEXCOORD) glVertexAttribPointer(VA_TEXCOORD, 2, GL_SCALAR,        GL_FALSE, Stride, TexCoordOffsetPtr);
			if (RenderAttributeMask & VAMASK_TANGENT ) glVertexAttribPointer(VA_TANGENT,  3, GL_SCALAR,        GL_FALSE, Stride, TangentOffsetPtr);
			if (RenderAttributeMask & VAMASK_COLOR   ) glVertexAttribPointer(VA_COLOR,    4, GL_UNSIGNED_BYTE, GL_TRUE,  Stride, ColorOffsetPtr);
		}
		if (g_Active3D.IndexBuffer != IndexBufferObject)
		{
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, (g_Active3D.IndexBuffer = IndexBufferObject));
		}
		glDrawElements(GL_TRIANGLES, p->IndexCount, GL_UNSIGNED_SHORT, p->IndexOffsetPtr);
	}
};

struct ZL_MeshAnimated_Impl : public ZL_Mesh_Impl
{
	std::vector<GLuint> FrameVertexBufferObjects;
	
	#ifdef ZL_VIDEO_WEAKCONTEXT
	std::vector<GLvoid*> WeakFramesVertData;
	void RecreateOnContextLost()
	{
		if (FrameVertexBufferObjects.empty()) return;
		unsigned int ActiveFrameIndex = 0;
		for (size_t i = 0; i < FrameVertexBufferObjects.size(); i++)
			if (VertexBufferObject == FrameVertexBufferObjects[i]) { ActiveFrameIndex = (unsigned int)i; break; }
		ZL_Mesh_Impl::RecreateOnContextLost();
		FrameVertexBufferObjects[0] = VertexBufferObject;
		for (size_t j = 1; j < FrameVertexBufferObjects.size(); j++)
			{ glGenBuffers(1, &FrameVertexBufferObjects[j]); glBindBuffer(GL_ARRAY_BUFFER, FrameVertexBufferObjects[j]); glBufferData(GL_ARRAY_BUFFER, WeakVertDataSize, WeakFramesVertData[j-1], GL_STATIC_DRAW); }
	}
	#endif

	ZL_MeshAnimated_Impl(GLubyte AttributeMask) : ZL_Mesh_Impl(AttributeMask) {}

	~ZL_MeshAnimated_Impl()
	{
		if (FrameVertexBufferObjects.empty()) return;
		VertexBufferObject = FrameVertexBufferObjects[0]; //reset to frame 0 for ~ZL_Mesh_Impl()
		if (FrameVertexBufferObjects.size() > 1) glDeleteBuffers((GLsizei)(FrameVertexBufferObjects.size() - 1), &FrameVertexBufferObjects[1]);

		#ifdef ZL_VIDEO_WEAKCONTEXT
		for (size_t i = 0; i < WeakFramesVertData.size(); i++) free(WeakFramesVertData[i]);
		#endif
	}

	void AddAndFreeFrameData(GLvoid* VertData, GLsizei VertCount)
	{
		GLuint FrameVertexBufferObject;
		glGenBuffers(1, &FrameVertexBufferObject);
		glBindBuffer(GL_ARRAY_BUFFER, FrameVertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, Stride * VertCount, VertData, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, g_Active3D.VertexBuffer);
		FrameVertexBufferObjects.push_back(FrameVertexBufferObject);

		#ifdef ZL_VIDEO_WEAKCONTEXT
		assert(g_LoadedMeshes);assert(IndexBufferObject);assert(WeakVertDataSize == Stride * VertCount);
		WeakFramesVertData.push_back(VertData);
		#else
		free(VertData);
		#endif
	}

	static ZL_Mesh_Impl* OBJLoadAnimation(const ZL_FileLink& file, ZL_Material_Impl* Material)
	{
		ZL_File f = file.Open();
		ZL_File_Impl* fileimpl = ZL_ImplFromOwner<ZL_File_Impl>(f);
		if (!fileimpl || !fileimpl->src || fileimpl->src->size() < 2) return NULL;

		std::vector<unsigned int> used_indices;
		ZL_MeshAnimated_Impl* res = NULL;
		unsigned char *buf, *bufmtl = NULL;
		for (unsigned int FileIndex = 0; (buf = ZL_RWopsZIP::ReadSingle(fileimpl->src, FileIndex)); FileIndex++)
		{
			if (strstr((char*)buf, "newmtl "))
			{
				bufmtl = buf;
				continue;
			}

			if (FileIndex == 0 || (bufmtl && FileIndex == 1))
			{
				struct MakeFunc { static ZL_Mesh_Impl* Make(GLubyte AttributeMask) { return new ZL_MeshAnimated_Impl(AttributeMask); } };
				res = static_cast<ZL_MeshAnimated_Impl*>(OBJLoad(file, Material, buf, &MakeFunc::Make, &used_indices));
				if (!res) return NULL;
				res->FrameVertexBufferObjects.push_back(res->VertexBufferObject);
				continue;
			}

			std::vector<GLscalar> Positions, Normals, TexCoords;
			std::vector<u64> all_indices;
			for (unsigned char *line_end = buf, *cursor; AdvanceLine(line_end, cursor);)
			{
				if (cursor[0] == 'f' && cursor[1] == ' ') //face
				{
					for (u64 ind; (ind = OBJReadNextCombinedIndex(cursor));) all_indices.push_back(ind);
				}
				else if (cursor[0] == 'v' && (cursor[1] == ' ' || cursor[1] == 'n' || cursor[1] == 't')) //vertex data (position, normal or texcoord)
				{
					cursor += (cursor[1] == ' ' ? 2 : 3);
					std::vector<GLscalar>& Vector = (cursor[-2] == 'n' ? Normals : (cursor[-2] == 't' ? TexCoords : Positions));
					for (int floatcount = (cursor[-2] == 't' ? 2 : 3); *cursor >= ' ' && floatcount--;)
						Vector.push_back((GLscalar)strtod((char*)cursor, (char**)&cursor));
				}
			}

			bool HasNormals = !Normals.empty(), HasTexCoords = !TexCoords.empty();
			assert(HasNormals == !!(res->ProvideAttributeMask & VAMASK_NORMAL) && HasTexCoords == !!(res->ProvideAttributeMask & VAMASK_TEXCOORD));
			if (HasNormals == !!(res->ProvideAttributeMask & VAMASK_NORMAL) && HasTexCoords == !!(res->ProvideAttributeMask & VAMASK_TEXCOORD))
			{
				GLvoid* VertData = malloc(res->Stride * used_indices.size());
				GLscalar *Out = (GLscalar*)VertData, Empties[3] = {0,0,0}, *SrcPosition = &Positions[0], *SrcNormal = (HasNormals ? &Normals[0] : 0), *SrcTexCoord = (HasTexCoords ? &TexCoords[0] : 0);
				for (std::vector<unsigned int>::iterator it = used_indices.begin(); it != used_indices.end(); ++it)
				{
					u64 ind = all_indices[*it];
					            GLsizei Idx = ((ind>>40)        ); memcpy(Out, (Idx ? SrcPosition + (Idx-1)*3 : Empties), sizeof(GLscalar)*3); Out += 3;
					if (HasNormals)   { Idx = ((ind    )&0xFFFFF); memcpy(Out, (Idx ? SrcNormal   + (Idx-1)*3 : Empties), sizeof(GLscalar)*3); Out += 3; }
					if (HasTexCoords) { Idx = ((ind>>20)&0xFFFFF); memcpy(Out, (Idx ? SrcTexCoord + (Idx-1)*2 : Empties), sizeof(GLscalar)*2); Out += 2; }
				}
				res->AddAndFreeFrameData(VertData, (GLsizei)used_indices.size());
			}

			free(buf);
		}
		if (bufmtl)
		{
			if (res) res->LoadMTLFile(bufmtl);
			free(bufmtl);
		}
		return res;
	}

	void SetFrame(unsigned int FrameIndex)
	{
		if (FrameIndex >= FrameVertexBufferObjects.size()) return;
		VertexBufferObject = FrameVertexBufferObjects[FrameIndex];
	}
};

struct ZL_ParticleEmitter_Impl : public ZL_Mesh_Impl
{
	struct sParticleMeta { scalar SpawnTime; ZL_Vector3 Velocity; sParticleMeta() : SpawnTime(s(-1)) {} };
	struct sEmitter { scalar LifeTime; ZL_Vector3 Gravity, VelocityMins, VelocityMaxs; ZL_Vector3 ColorMin, ColorMax; scalar AlphaMin, AlphaMax, SizeMin, SizeMax; int TileCount, TileCols; bool TileAnimate, ColorOverTime, AlphaOverTime, SizeOverTime; };

	std::vector<sParticleMeta> ParticleMeta;
	std::vector<ZL_Vector3> ShaderData;
	sEmitter Emitter;
	size_t ActiveCount, MaxCount;

	enum { S_COUNT = 70, SD_HEADER = 3, SD_BODY = 3, SD_COUNT = SD_HEADER + S_COUNT * SD_BODY, P_INDEXES = 6 };
	#define SD_COUNT_STR "213" //needs to be updated when changing above

	ZL_ParticleEmitter_Impl(scalar LifeTime, size_t MaxParticles, ZL_MaterialModes::Blending BlendMode) : ZL_Mesh_Impl(0), ActiveCount(0), MaxCount(MaxParticles)
	{
		memset(&Emitter, 0, sizeof(Emitter));
		Emitter.LifeTime = LifeTime;
		Emitter.ColorMin = Emitter.ColorMax = ZL_Vector3::One;
		Emitter.SizeMin = Emitter.SizeMax = Emitter.AlphaMin = Emitter.AlphaMax = s(1);
		GLscalar Verts[S_COUNT*4*3];
		GLushort Indices[S_COUNT*P_INDEXES];
		for (GLushort i = 0; i < S_COUNT; i++)
		{
			Verts[i*4*3 + 3*0 + 0] = -HALF; Verts[i*4*3 + 3*0 + 1] =  HALF; Verts[i*4*3 + 3*0 + 2] = 3+i*3+HALF;
			Verts[i*4*3 + 3*1 + 0] = -HALF; Verts[i*4*3 + 3*1 + 1] = -HALF; Verts[i*4*3 + 3*1 + 2] = 3+i*3+HALF;
			Verts[i*4*3 + 3*2 + 0] =  HALF; Verts[i*4*3 + 3*2 + 1] = -HALF; Verts[i*4*3 + 3*2 + 2] = 3+i*3+HALF;
			Verts[i*4*3 + 3*3 + 0] =  HALF; Verts[i*4*3 + 3*3 + 1] =  HALF; Verts[i*4*3 + 3*3 + 2] = 3+i*3+HALF;
			Indices[i*P_INDEXES + 0] = i*4 + 0; Indices[i*P_INDEXES + 1] = i*4 + 1; Indices[i*P_INDEXES + 2] = i*4 + 2;
			Indices[i*P_INDEXES + 3] = i*4 + 0; Indices[i*P_INDEXES + 4] = i*4 + 2; Indices[i*P_INDEXES + 5] = i*4 + 3;
		}

		#if defined(ZILLALOG)
		GLint MAX_VERTEX_UNIFORM_VECTORS, MAX_FRAGMENT_UNIFORM_VECTORS;
		glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, &MAX_VERTEX_UNIFORM_VECTORS);
		glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, &MAX_FRAGMENT_UNIFORM_VECTORS);
		//ZL_LOG2("[3D]", "MAX_VERTEX_UNIFORM_VECTORS: %d - MAX_FRAGMENT_UNIFORM_VECTORS: %d", MAX_VERTEX_UNIFORM_VECTORS, MAX_FRAGMENT_UNIFORM_VECTORS);
		assert(MAX_VERTEX_UNIFORM_VECTORS > SD_COUNT+10);
		assert(MAX_FRAGMENT_UNIFORM_VECTORS > SD_COUNT+10);
		#endif

		using namespace ZL_MaterialModes;
		ZL_Material_Impl* Program = ZL_Material_Impl::GetMaterialReference(MM_VERTEXCOLOR | MM_DIFFUSEMAP | MM_VERTEXFUNC | MO_UNLIT | MO_CASTNOSHADOW | BlendMode, NULL, 
			"uniform vec3 sd[" SD_COUNT_STR "];"
			"void Vertex()"
			"{"
				"int i = int(" Z3A_POSITION ".z);"
				"float sz = sd[i+2].z;"
				Z3O_POSITION " = vec4(sd[i] + sd[0] * (" Z3A_POSITION ".x*sz) + sd[1] * (" Z3A_POSITION ".y*sz), 1);"
				"\n#ifndef " Z3D_SHADOWMAP "\n"
				Z3V_COLOR " = vec4(sd[i+1], sd[i+2].x);"
				"float m = sd[i+2].y, n = sd[2].z;"
				Z3V_TEXCOORD " = vec2((" Z3A_POSITION ".x + .5 + mod(m, n)) * sd[2].x, (" Z3A_POSITION ".y + .5 + (n-1.-floor(m/n))) * sd[2].y);"
				"\n#endif\n"
			"}"
		);

		CreateAndFillBufferData(Indices, S_COUNT*P_INDEXES, Verts, S_COUNT*4);
		Parts = (MeshPart*)malloc(sizeof(MeshPart));
		Parts[0] = MeshPart(ZL_NameID(), 0, NULL, Program, false);
		PartsEnd = Parts+1;
		
		GLint Off = Parts[0].Material->ShaderProgram->GetUniformOffset("sd[0]", ZL_MaterialProgram::UniformEntry::TYPE_VEC3, true);
		reinterpret_cast<ZL_Material_Impl::UniformArrayValue*>(Program->UniformSet.Values+Off)->Count = SD_COUNT;
	}

	void SetTexture(ZL_Texture_Impl* Tex, int NumTilesCols, int NumTilesRows, bool Animate)
	{
		for (ZL_Mesh_Impl::MeshPart* it = Parts; it != PartsEnd; ++it)
			it->Material->SetTexture(0, Tex);
		Emitter.TileCount = NumTilesCols * NumTilesRows;
		Emitter.TileCols = NumTilesCols;
		Emitter.TileAnimate = Animate;
	}

	void Spawn(const ZL_Vector3& Pos)
	{
		if (ActiveCount == MaxCount) return;

		sParticleMeta* m;
		if (ActiveCount == ParticleMeta.size())
		{
			size_t NewPartIdx = ActiveCount / S_COUNT, NewPartCount = NewPartIdx + 1;
			if (NewPartIdx > 0)
			{
				Parts = (MeshPart*)realloc(Parts, sizeof(MeshPart) * NewPartCount);
				Parts[NewPartIdx] = Parts[0];
				Parts[NewPartIdx].Material = new ZL_MaterialInstance(Parts[0].Material, Parts[0].Material->MaterialModes, false);
				Parts[NewPartIdx].Material->UniformSet.ValueChksum = (GLuint)NewPartIdx + 1;
				PartsEnd = Parts + NewPartCount;
			}
			else Parts[0].IndexCount = S_COUNT*P_INDEXES;

			ParticleMeta.resize(NewPartCount * S_COUNT);
			ShaderData.resize(NewPartCount * SD_COUNT);
			GLint Off = Parts[0].Material->ShaderProgram->GetUniformOffset("sd[0]", ZL_MaterialProgram::UniformEntry::TYPE_VEC3, true);
			for (size_t Part = 0; Part < NewPartCount; Part++)
				reinterpret_cast<ZL_Material_Impl::UniformArrayValue*>(Parts[Part].Material->UniformSet.Values+Off)->Ptr = (scalar*)&ShaderData[SD_COUNT * Part];
			m = &ParticleMeta[ActiveCount];
		}
		else
		{
			m = &ParticleMeta[0];
			for (sParticleMeta* mEnd = m + ParticleMeta.size(); m != mEnd; m++)
				if (m->SpawnTime < 0)
					break;
		}

		size_t i = (m - &ParticleMeta[0]);
		ZL_Vector3* p = &ShaderData[SD_HEADER + ((i / S_COUNT) * SD_COUNT) + ((i % S_COUNT) * SD_BODY)];
		m->SpawnTime = ZLSECONDS;
		m->Velocity = ZLV3(RAND_RANGE(Emitter.VelocityMins.x,Emitter.VelocityMaxs.x), RAND_RANGE(Emitter.VelocityMins.y,Emitter.VelocityMaxs.y), RAND_RANGE(Emitter.VelocityMins.z,Emitter.VelocityMaxs.z));
		p[0] = Pos;
		if (!Emitter.ColorOverTime) p[1] = ZL_Vector3::Lerp(Emitter.ColorMin, Emitter.ColorMax, RAND_FACTOR);
		if (!Emitter.AlphaOverTime) p[2].x = ZL_Math::Lerp(Emitter.AlphaMin, Emitter.AlphaMax, RAND_FACTOR);
		if (!Emitter.TileAnimate)   p[2].y = s(RAND_INT_MAX(Emitter.TileCount-1));
		if (!Emitter.SizeOverTime)  p[2].z = ZL_Math::Lerp(Emitter.SizeMin, Emitter.SizeMax, RAND_FACTOR);
		ActiveCount++;
	}

	void Update(const ZL_Camera& Camera)
	{
		if (!ActiveCount) return;

		scalar TimeNow = ZLSECONDS, TimeElapsed = ZLELAPSED;
		ZL_Vector3 CamRight = Camera.GetRightDirection();
		ZL_Vector3 CamUp = Camera.GetUpDirection();
		ZL_Vector3 SpriteSize = ZLV3(s(1)/Emitter.TileCols, s(1)/(Emitter.TileCount/Emitter.TileCols), Emitter.TileCols);
		ZL_Vector3* p = &ShaderData[0];
		sParticleMeta* m = &ParticleMeta[0];
		for (size_t i = 0; i < ParticleMeta.size(); i++, m++, p+=3)
		{
			if (!(i % S_COUNT))
			{
				*(p++) = CamRight;
				*(p++) = CamUp;
				*(p++) = SpriteSize;
			}
			if (m->SpawnTime < 0) continue;

			scalar t = (TimeNow - m->SpawnTime) / Emitter.LifeTime;
			if (t > 1)
			{
				ActiveCount--;
				m->SpawnTime = s(-1);
				p[0].x = S_MAX;
				p[2].z = 0;
				continue;
			}

			m->Velocity += Emitter.Gravity * TimeElapsed;
			p[0] += m->Velocity * TimeElapsed;
			if (Emitter.ColorOverTime) p[1] = ZL_Vector3::Lerp(Emitter.ColorMin, Emitter.ColorMax, t);
			if (Emitter.AlphaOverTime) p[2].x = ZL_Math::Lerp(Emitter.AlphaMin, Emitter.AlphaMax, t);
			if (Emitter.TileAnimate)   p[2].y = s((int)(t * Emitter.TileCount));
			if (Emitter.SizeOverTime)  p[2].z = ZL_Math::Lerp(Emitter.SizeMin, Emitter.SizeMax, t);
		}
		Parts[0].Material->UniformSet.ValueChksum ^= 1;
	}
};

struct ZL_RenderList_Impl : public ZL_Impl
{
	struct MeshEntry { ZL_Mesh_Impl* Mesh; ZL_Matrix ModelMatrix, NormalMatrix; MeshEntry(ZL_Mesh_Impl* Mesh, const ZL_Matrix& ModelMatrix, const ZL_Matrix& NormalMatrix) : Mesh(Mesh), ModelMatrix(ModelMatrix), NormalMatrix(NormalMatrix) {} };
	struct PartEntry { ZL_Mesh_Impl::MeshPart* Part; ZL_Material_Impl* Material; GLushort MeshIndex; GLuint Checksum; PartEntry(ZL_Mesh_Impl::MeshPart* Part, ZL_Material_Impl* Material, GLushort MeshIndex, GLuint Checksum) : Part(Part), Material(Material), MeshIndex(MeshIndex), Checksum(Checksum) {} };
	std::vector<ZL_Mesh_Impl*> ReferencedMeshes;
	std::vector<MeshEntry> Meshes;
	std::vector<PartEntry> Parts;

	void Reset()
	{
		for (std::vector<ZL_Mesh_Impl*>::iterator it = ReferencedMeshes.begin(); it != ReferencedMeshes.end(); ++it) (*it)->DelRef();
		ReferencedMeshes.clear();
		Meshes.clear();
		Parts.clear();
	}

	void Add(ZL_Mesh_Impl* mesh, const ZL_Matrix& matrix, ZL_Material_Impl* OverrideMaterial)
	{
		GLushort MeshIndex = (GLushort)Meshes.size();
		bool UseNormalMatrix = false;
		for (ZL_Mesh_Impl::MeshPart *p = mesh->Parts; p != mesh->PartsEnd; p++)
		{	
			ZL_Material_Impl* m = (OverrideMaterial ? OverrideMaterial : p->Material);
			if (m->ShaderProgram->ShaderIDs.UniformMatrixNormal != -1) UseNormalMatrix = true;
			Parts.push_back(PartEntry(p, m, MeshIndex, (m->ShaderProgram->ShaderIDs.Program ^ m->UniformSet.ValueChksum ^ m->UniformSet.TextureChksum)));
		}
		Meshes.push_back(MeshEntry(mesh, matrix, (UseNormalMatrix ? matrix.GetInverseTransposed() : ZL_Matrix(ZL_NoInit))));
	}

	void AddReferenced(ZL_Mesh_Impl* mesh, const ZL_Matrix& matrix)
	{
		mesh->AddRef();
		ReferencedMeshes.push_back(mesh);
		Add(mesh, matrix, NULL);
	}

	void Sort()
	{
		struct Func { static inline bool SortByMaterial(const PartEntry& a, const PartEntry& b)
		{
			const ZL_Material_Impl *am = a.Material, *bm = b.Material;
			if ((am->MaterialModes & ZL_Display3D_Shaders::MMDEF_NODEPTHWRITE) != (bm->MaterialModes & ZL_Display3D_Shaders::MMDEF_NODEPTHWRITE)) return !(am->MaterialModes & ZL_Display3D_Shaders::MMDEF_NODEPTHWRITE);
			const ZL_MaterialProgram *ap = am->ShaderProgram, *bp = bm->ShaderProgram;
			return (ap->ShadowMapProgram == bp->ShadowMapProgram ? ap < bp : ap->ShadowMapProgram < bp->ShadowMapProgram);
		}};
		std::sort(Parts.begin(), Parts.end(), Func::SortByMaterial);
	}

	void RenderShadowMap(const ZL_RenderSceneSetup& Scene)
	{
		GLuint ActiveChecksum = 0;
		for (ZL_RenderList_Impl::PartEntry *e = (Parts.empty() ? NULL : &Parts[0]), *eEnd = e + Parts.size(); e != eEnd; e++)
		{
			if (e->Material->MaterialModes & ZL_MaterialModes::MO_CASTNOSHADOW) continue;
			if (ActiveChecksum != e->Checksum)
			{
				ActiveChecksum = e->Checksum;
				e->Material->ShaderProgram->ShadowMapProgram->Activate(e->Material->UniformSet, Scene);
			}
			ZL_RenderList_Impl::MeshEntry& me = Meshes[e->MeshIndex];
			me.Mesh->DrawPart(e->Part, me.ModelMatrix, me.NormalMatrix);
		}
	}

	void RenderColor(const ZL_RenderSceneSetup& Scene)
	{
		using namespace ZL_Display3D_Shaders; using namespace ZL_MaterialModes;
		GLuint ActiveChecksum = 0, MaterialOptions = 0;
		for (ZL_RenderList_Impl::PartEntry *e = (Parts.empty() ? NULL : &Parts[0]), *eEnd = e + Parts.size(); e != eEnd; e++)
		{
			enum { MMDEF_WATCHOPTIONS = MMDEF_NODEPTHWRITE|MO_ADDITIVE|MO_MODULATE|MO_IGNOREDEPTH };
			if (MaterialOptions != (e->Material->MaterialModes & MMDEF_WATCHOPTIONS))
			{
				GLuint NewOptions = (e->Material->MaterialModes & MMDEF_WATCHOPTIONS);
				if ((NewOptions & MMDEF_NODEPTHWRITE) && !(MaterialOptions & MMDEF_NODEPTHWRITE)) glDepthMask(GL_FALSE);
				if (!(NewOptions & MMDEF_NODEPTHWRITE) && (MaterialOptions & MMDEF_NODEPTHWRITE)) glDepthMask(GL_TRUE);
				if ((NewOptions & MO_IGNOREDEPTH) && !(MaterialOptions & MO_IGNOREDEPTH)) glDisable(GL_DEPTH_TEST);
				if (!(NewOptions & MO_IGNOREDEPTH) && (MaterialOptions & MO_IGNOREDEPTH)) glEnable(GL_DEPTH_TEST);
				if ((NewOptions & MO_ADDITIVE) && !(MaterialOptions & MO_ADDITIVE)) glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				if ((NewOptions & MO_MODULATE) && !(MaterialOptions & MO_MODULATE)) glBlendFunc(GL_DST_COLOR, GL_ZERO);
				if (!(NewOptions & MO_ADDITIVE) && (MaterialOptions & MO_ADDITIVE)) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				if (!(NewOptions & MO_MODULATE) && (MaterialOptions & MO_MODULATE)) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				MaterialOptions = NewOptions;
			}
			if (ActiveChecksum != e->Checksum)
			{
				ActiveChecksum = e->Checksum;
				e->Material->ShaderProgram->Activate(e->Material->UniformSet, Scene);
			}
			ZL_RenderList_Impl::MeshEntry& me = Meshes[e->MeshIndex];
			me.Mesh->DrawPart(e->Part, me.ModelMatrix, me.NormalMatrix);
		}
		if (MaterialOptions & MMDEF_NODEPTHWRITE) glDepthMask(GL_TRUE);
		if (MaterialOptions & MO_IGNOREDEPTH) glEnable(GL_DEPTH_TEST);
		if (MaterialOptions & (MO_ADDITIVE|MO_MODULATE)) glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
};

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Material)
ZL_Material::ZL_Material(unsigned int MaterialModes) : impl(ZL_Material_Impl::GetMaterialReference(MaterialModes)) { }
ZL_Material::ZL_Material(unsigned int MaterialModes, const char* CustomFragmentCode, const char* CustomVertexCode) : impl(ZL_Material_Impl::GetMaterialReference(MaterialModes, CustomFragmentCode, CustomVertexCode)) { }
ZL_Material ZL_Material::MakeNewMaterialInstance() { return (impl ? ZL_ImplMakeOwner<ZL_Material>(new ZL_MaterialInstance(impl, impl->MaterialModes, false), false) : ZL_Material()); }
ZL_Material& ZL_Material::SetUniformFloat(ZL_NameID Name, scalar val)           { if (impl) impl->SetUniform1(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_FLOAT), val); return *this; }
ZL_Material& ZL_Material::SetUniformVec2(ZL_NameID Name, const ZL_Vector& val)  { if (impl) impl->SetUniformX(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_VEC2), (scalar*)&val, 2); return *this; }
ZL_Material& ZL_Material::SetUniformVec3(ZL_NameID Name, const ZL_Vector3& val) { if (impl) impl->SetUniformX(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_VEC3), (scalar*)&val, 3); return *this; }
ZL_Material& ZL_Material::SetUniformVec3(ZL_NameID Name, const ZL_Color& val)   { if (impl) impl->SetUniformX(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_VEC3), (scalar*)&val, 3); return *this; }
ZL_Material& ZL_Material::SetUniformVec4(ZL_NameID Name, const ZL_Color& val)   { if (impl) impl->SetUniformX(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_VEC4), (scalar*)&val, 4); return *this; }
ZL_Material& ZL_Material::SetDiffuseTexture(const ZL_Surface& srf)  { if (impl) impl->SetTexture(0, (srf ? ZL_ImplFromOwner<ZL_Surface_Impl>(srf)->tex : NULL)); return *this; }
ZL_Material& ZL_Material::SetNormalTexture(const ZL_Surface& srf)   { if (impl) impl->SetTexture(1, (srf ? ZL_ImplFromOwner<ZL_Surface_Impl>(srf)->tex : NULL)); return *this; }
ZL_Material& ZL_Material::SetSpecularTexture(const ZL_Surface& srf) { if (impl) impl->SetTexture(2, (srf ? ZL_ImplFromOwner<ZL_Surface_Impl>(srf)->tex : NULL)); return *this; }
ZL_Material& ZL_Material::SetParallaxTexture(const ZL_Surface& srf) { if (impl) impl->SetTexture(3, (srf ? ZL_ImplFromOwner<ZL_Surface_Impl>(srf)->tex : NULL)); return *this; }
scalar ZL_Material::GetUniformFloat(ZL_NameID Name)    { return (impl ? impl->GetUniform1(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_FLOAT)) : 0); }
ZL_Vector ZL_Material::GetUniformVec2(ZL_NameID Name)  { ZL_Vector r;  if (impl) impl->GetUniformX(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_FLOAT), (scalar*)&r, 2); return r; }
ZL_Vector3 ZL_Material::GetUniformVec3(ZL_NameID Name) { ZL_Vector3 r; if (impl) impl->GetUniformX(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_FLOAT), (scalar*)&r, 3); return r; }
ZL_Color ZL_Material::GetUniformVec4(ZL_NameID Name)   { ZL_Color r;   if (impl) impl->GetUniformX(impl->ShaderProgram->GetUniformOffset(Name, ZL_MaterialProgram::UniformEntry::TYPE_FLOAT), (scalar*)&r, 4); return r; }
ZL_Surface ZL_Material::GetDiffuseTexture()  { return ZL_ImplMakeOwner<ZL_Surface>(new ZL_Surface_Impl(impl->GetTexture(0)), false); }
ZL_Surface ZL_Material::GetNormalTexture()   { return ZL_ImplMakeOwner<ZL_Surface>(new ZL_Surface_Impl(impl->GetTexture(1)), false); }
ZL_Surface ZL_Material::GetSpecularTexture() { return ZL_ImplMakeOwner<ZL_Surface>(new ZL_Surface_Impl(impl->GetTexture(2)), false); }
ZL_Surface ZL_Material::GetParallaxTexture() { return ZL_ImplMakeOwner<ZL_Surface>(new ZL_Surface_Impl(impl->GetTexture(3)), false); }

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Mesh)
ZL_Mesh::ZL_Mesh(const ZL_FileLink& ModelFile, const ZL_Material& Material) : impl(ZL_Mesh_Impl::LoadAny(ModelFile, ZL_ImplFromOwner<ZL_Material_Impl>(Material))) {}
ZL_Mesh ZL_Mesh::FromPLY(const ZL_FileLink& PLYFile, const ZL_Material& Material) { ZL_Mesh m; m.impl = ZL_Mesh_Impl::PLYLoad(PLYFile, ZL_ImplFromOwner<ZL_Material_Impl>(Material)); return m; }
ZL_Mesh ZL_Mesh::FromOBJ(const ZL_FileLink& OBJFile, const ZL_Material& Material) { ZL_Mesh m; m.impl = ZL_Mesh_Impl::OBJLoad(OBJFile, ZL_ImplFromOwner<ZL_Material_Impl>(Material)); return m; }
#if defined(ZILLALOG) && !defined(ZL_VIDEO_OPENGL_ES2)
void ZL_Mesh::DrawDebug(const ZL_Matrix& Matrix, const struct ZL_Camera& Camera) { if (impl) impl->DrawDebug(Matrix, Camera); }
#endif

ZL_Material ZL_Mesh::GetMaterial(unsigned int PartNumber) const
{
	if (!impl || impl->Parts + PartNumber >= impl->PartsEnd) return ZL_Material();
	return ZL_ImplMakeOwner<ZL_Material>(impl->Parts[PartNumber].Material, true);
}

ZL_Material ZL_Mesh::GetMaterial(ZL_NameID PartName) const
{
	ZL_Mesh_Impl::MeshPart* it;
	if (!impl || (it = std::find(impl->Parts, impl->PartsEnd, PartName)) == impl->PartsEnd) return ZL_Material();
	return ZL_ImplMakeOwner<ZL_Material>(it->Material, true);
}

ZL_Mesh& ZL_Mesh::SetMaterial(unsigned int PartNumber, const ZL_Material& Material)
{
	assert(!impl || impl->Parts + PartNumber < impl->PartsEnd);
	if (!impl || impl->Parts + PartNumber >= impl->PartsEnd || !ZL_ImplFromOwner<ZL_Material_Impl>(Material)) return *this;
	ZL_Impl::CopyRef(ZL_ImplFromOwner<ZL_Material_Impl>(Material), (ZL_Impl*&)impl->Parts[PartNumber].Material);
	return *this;
}

ZL_Mesh& ZL_Mesh::SetMaterial(ZL_NameID PartName, const ZL_Material& Material)
{
	ZL_Mesh_Impl::MeshPart* it;
	if (!impl || (it = std::find(impl->Parts, impl->PartsEnd, PartName)) == impl->PartsEnd || !ZL_ImplFromOwner<ZL_Material_Impl>(Material)) return *this;
	ZL_Impl::CopyRef(ZL_ImplFromOwner<ZL_Material_Impl>(Material), (ZL_Impl*&)it->Material);
	return *this;
}

ZL_Mesh& ZL_Mesh::SetMaterial(const ZL_Material& Material)
{
	if (!impl || !ZL_ImplFromOwner<ZL_Material_Impl>(Material)) return *this;
	for (ZL_Mesh_Impl::MeshPart* it = impl->Parts; it != impl->PartsEnd; ++it)
		ZL_Impl::CopyRef(ZL_ImplFromOwner<ZL_Material_Impl>(Material), (ZL_Impl*&)it->Material);
	return *this;
}

ZL_Mesh ZL_Mesh::BuildPlane(const ZL_Vector& Extents, const ZL_Material& Material, const ZL_Vector3& Normal, const ZL_Vector3& Offset, const ZL_Vector& UVMapMax)
{
	ZL_Vector3 Tangent = (Normal ^ ZL_Vector3::Up).VecNorm(), Bitangent = -(Tangent ^ Normal).VecNorm(), X = Tangent * Extents.x, Y = Bitangent * Extents.y;
	GLscalar Verts[] = {
		Offset.x + X.x + Y.x, Offset.y + X.y + Y.y, Offset.z + X.z + Y.z, Normal.x,Normal.y,Normal.z, UVMapMax.x, UVMapMax.y, Tangent.x,Tangent.y,Tangent.z,
		Offset.x - X.x + Y.x, Offset.y - X.y + Y.y, Offset.z - X.z + Y.z, Normal.x,Normal.y,Normal.z,          0, UVMapMax.y, Tangent.x,Tangent.y,Tangent.z,
		Offset.x - X.x - Y.x, Offset.y - X.y - Y.y, Offset.z - X.z - Y.z, Normal.x,Normal.y,Normal.z,          0,          0, Tangent.x,Tangent.y,Tangent.z,
		Offset.x + X.x - Y.x, Offset.y + X.y - Y.y, Offset.z + X.z - Y.z, Normal.x,Normal.y,Normal.z, UVMapMax.x,          0, Tangent.x,Tangent.y,Tangent.z
	};
	GLushort Indices[] = { 0,1,2,0,2,3 };

	/*
	for (size_t indind = 0; indind < COUNT_OF(Indices); indind+=3)
	{
		GLushort a = 11*Indices[indind+0], b = 11*Indices[indind+1], c = 11*Indices[indind+2];
		ZL_Vector3 edge1 = ZL_Vector3(Verts[b+0],Verts[b+1],Verts[b+2]) - ZL_Vector3(Verts[a+0],Verts[a+1],Verts[a+2]);
		ZL_Vector3 edge2 = ZL_Vector3(Verts[c+0],Verts[c+1],Verts[c+2]) - ZL_Vector3(Verts[a+0],Verts[a+1],Verts[a+2]);
		ZL_Vector  deltaUV1 = ZL_Vector(Verts[b+6],Verts[b+7]) - ZL_Vector(Verts[a+6],Verts[a+7]);
		ZL_Vector  deltaUV2 = ZL_Vector(Verts[c+6],Verts[c+7]) - ZL_Vector(Verts[a+6],Verts[a+7]);
		GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
		ZL_Vector3 tangent;
		tangent.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
		tangent.Norm();
		ZL_Vector3 bitangent;
		bitangent.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
		bitangent.Norm();
		assert(Verts[a+8]==tangent.x && Verts[a+9]==tangent.y && Verts[a+10]==tangent.z);
		assert(Verts[b+8]==tangent.x && Verts[b+9]==tangent.y && Verts[b+10]==tangent.z);
		assert(Verts[c+8]==tangent.x && Verts[c+9]==tangent.y && Verts[c+10]==tangent.z);
	}
	*/

	return ZL_ImplMakeOwner<ZL_Mesh>(ZL_Mesh_Impl::Make((ZL_Mesh_Impl::VAMASK_NORMAL|ZL_Mesh_Impl::VAMASK_TEXCOORD|ZL_Mesh_Impl::VAMASK_TANGENT), Indices, 6, Verts, 4, ZL_ImplFromOwner<ZL_Material_Impl>(Material)), false);
}

ZL_Mesh ZL_Mesh::BuildBox(const ZL_Vector3& e, const ZL_Material& Material, const ZL_Vector3& o, const ZL_Vector& uv)
{
	const GLscalar ex = e.x, ey = e.y, ez = e.z, ox = o.x, oy = o.y, oz = o.z, u = uv.x/MAX(ex,ey), v = uv.y/MAX(ey,ez), ux = u*ex, uy = u*ey, vy = 1-v*ey, vz = 1-v*ez, Verts[11*6*4] = {
		ox+ex,oy+ey,oz+ez,  0, 0, 1,    ux, 1    ,  1, 0, 0 , ox-ex,oy+ey,oz+ez,  0, 0, 1,    0 ,1,     1, 0, 0 , ox-ex,oy-ey,oz+ez,  0, 0, 1,     0,vy   ,  1, 0, 0 , ox+ex,oy-ey,oz+ez,  0, 0, 1,    ux,vy   ,  1, 0, 0,
		ox+ex,oy+ey,oz-ez,  0, 0,-1,     0, 1    ,  0, 1, 0 , ox+ex,oy-ey,oz-ez,  0, 0,-1,    0,vy,     0, 1, 0 , ox-ex,oy-ey,oz-ez,  0, 0,-1,    ux,vy   ,  0, 1, 0 , ox-ex,oy+ey,oz-ez,  0, 0,-1,    ux, 1   ,  0, 1, 0,
		ox+ex,oy+ey,oz+ez,  1, 0, 0,    uy, 1    ,  0, 1, 0 , ox+ex,oy-ey,oz+ez,  1, 0, 0,    0, 1,     0, 1, 0 , ox+ex,oy-ey,oz-ez,  1, 0, 0,     0,vz   ,  0, 1, 0 , ox+ex,oy+ey,oz-ez,  1, 0, 0,    uy,vz   ,  0, 1, 0,
		ox-ex,oy+ey,oz+ez, -1, 0, 0,     0, 1    ,  0,-1, 0 , ox-ex,oy+ey,oz-ez, -1, 0, 0,    0,vz,     0,-1, 0 , ox-ex,oy-ey,oz-ez, -1, 0, 0,    uy,vz   ,  0,-1, 0 , ox-ex,oy-ey,oz+ez, -1, 0, 0,    uy, 1   ,  0,-1, 0,
		ox+ex,oy+ey,oz+ez,  0, 1, 0,     0, 1    , -1, 0, 0 , ox+ex,oy+ey,oz-ez,  0, 1, 0,    0,vz,    -1, 0, 0 , ox-ex,oy+ey,oz-ez,  0, 1, 0,    ux,vz   , -1, 0, 0 , ox-ex,oy+ey,oz+ez,  0, 1, 0,    ux, 1   , -1, 0, 0,
		ox+ex,oy-ey,oz+ez,  0,-1, 0,    ux, 1    ,  1, 0, 0 , ox-ex,oy-ey,oz+ez,  0,-1, 0,    0, 1,     1, 0, 0 , ox-ex,oy-ey,oz-ez,  0,-1, 0,     0,vz  ,  1, 0, 0 , ox+ex,oy-ey,oz-ez,  0,-1, 0,     ux,vz   ,  1, 0, 0,
	};
	const GLushort Indices[] = { 0,1,2,0,2,3 , 4,5,6,4,6,7 , 8,9,10,8,10,11 , 12,13,14,12,14,15 , 16,17,18,16,18,19 , 20,21,22,20,22,23 };
	return ZL_ImplMakeOwner<ZL_Mesh>(ZL_Mesh_Impl::Make((ZL_Mesh_Impl::VAMASK_NORMAL|ZL_Mesh_Impl::VAMASK_TEXCOORD|ZL_Mesh_Impl::VAMASK_TANGENT), Indices, COUNT_OF(Indices), Verts, COUNT_OF(Verts)/11, ZL_ImplFromOwner<ZL_Material_Impl>(Material)), false);
}

ZL_Mesh ZL_Mesh::BuildLandscape(const ZL_Material& Material)
{
	struct NoiseFuncs
	{
		static scalar Noise(unsigned int x) { x = (x << 13) ^ x; return (1.0f - ((x * (x * x * 15731) + 1376312589) & 0x7fffffff) / 1073741824.0f); }
		static scalar PerlinNoise2D(unsigned int seed, scalar persistence, scalar octave, scalar x, scalar y)
		{
			scalar freq = spow(s(2), s(octave)), amp = spow(persistence, s(octave)), tx = x * freq, ty = y * freq;
			int txi = (int)tx, tyi = (int)ty;
			scalar fracX = tx - txi, fracY = ty - tyi;
			scalar v1 = Noise(txi + tyi * 57 + seed), v2 = Noise(txi + 1 + tyi * 57 + seed), v3 = Noise(txi + (tyi + 1) * 57 + seed), v4 = Noise(txi + 1 + (tyi + 1) * 57 + seed);
			return ZL_Math::Lerp(ZL_Math::Lerp(v1, v2, fracX), ZL_Math::Lerp(v3, v4, fracX), fracY) * amp;
		}
	};
	std::vector<GLscalar> Verts;
	std::vector<GLushort> Indices;
	size_t SZ = 129, SZMAX = SZ-1;
	for (scalar y = 0, XYEND = s(SZ)-s(0.1), SZHALF = s(SZMAX)*s(.5); y < XYEND; y++) for (scalar x = 0; x < XYEND; x++)
	{
		GLscalar vs[] = { x-SZHALF,NoiseFuncs::PerlinNoise2D(0, s(2), s(2), ZLSECONDS*.1f+x/s(20), y/s(20))*0.1f,y-SZHALF , 0,1,0 , x/s(16),-y/s(16) , 1,0,0 };
		Verts.insert(Verts.end(), vs, vs+COUNT_OF(vs));
		if (!y || !x) continue;
		GLushort d = (GLushort)(Verts.size()/11-1), c = d - 1, b = c - (GLushort)SZ, a = b+1, is[] = { a,b,c , a,c,d };
		Indices.insert(Indices.end(), is, is+COUNT_OF(is));
	}
	for (size_t iy = 0; iy < SZ; iy++) for (size_t ix = 0; ix < SZ; ix++)
	{
		ZL_Vector3 *c = (ZL_Vector3*)&Verts[(iy*SZ+ix)*11], *n = c+1,
			*xa = (!ix ? c : (ZL_Vector3*)&Verts[(iy*SZ+ix- 1)*11]), *xb = (ix==SZMAX ? c : (ZL_Vector3*)&Verts[(iy*SZ+ix+ 1)*11]),
			*ya = (!iy ? c : (ZL_Vector3*)&Verts[(iy*SZ+ix-SZ)*11]), *yb = (iy==SZMAX ? c : (ZL_Vector3*)&Verts[(iy*SZ+ix+SZ)*11]);
		ZL_Vector NormX = ZL_Vector::FromAngle(satan2(xb->y - xa->y, 2)+PIHALF);
		ZL_Vector NormY = ZL_Vector::FromAngle(satan2(yb->y - ya->y, 2)+PIHALF);
		*n = ZL_Vector3(NormX.x, (NormX.y + NormY.y) * .5f, NormY.x).VecNorm();
	}
	return ZL_ImplMakeOwner<ZL_Mesh>(ZL_Mesh_Impl::Make((ZL_Mesh_Impl::VAMASK_NORMAL|ZL_Mesh_Impl::VAMASK_TEXCOORD|ZL_Mesh_Impl::VAMASK_TANGENT), &Indices[0], (GLsizei)Indices.size(), &Verts[0], (GLsizei)(Verts.size()/11), ZL_ImplFromOwner<ZL_Material_Impl>(Material)), false);
}

ZL_Mesh ZL_Mesh::BuildSphere(scalar Radius, int Segments, bool Inside, const ZL_Material& Material)
{
	std::vector<GLscalar> Verts;
	std::vector<GLushort> Indices;
	int stack, stacks = Segments/2+1, slice, slices = Segments+1;
	if (Inside) Radius *= -1;
	for (stack = 0; stack < stacks; stack++)
	{
		GLscalar u = (GLscalar)stack / (stacks-1), phi = u * PI, sinphi = ssin(phi), cosphi = scos(phi); //, phiu = u * PI, sinphiu = scos(phiu), cosphiu = scos(phiu);
		if (Inside) sinphi *= s(-1), cosphi *= s(-1);
		for (slice = 0; slice < slices; slice++)
		{
			GLscalar v = (GLscalar)slice / (slices-1),  theta = v * 2 * PI;
			GLscalar V[11] = { 0,0,0 , scos(theta)*sinphi,ssin(theta)*sinphi,cosphi , u,v , 0,0,0 };
			V[0] = V[3]*Radius, V[1] = V[4]*Radius, V[2] = V[5]*Radius;
			Verts.insert(Verts.end(), V, V + COUNT_OF(V));
		}
	}
	for (stack = 0; stack < stacks; stack++)
	{
		for (slice = 0; slice < slices; slice++)
		{
			GLushort a = stack*slices + slice, b = stack*slices + ((slice+1) % slices), c = ((stack+1) % stacks)*slices + ((slice+1) % slices), d = ((stack+1) % stacks)*slices + slice;

			ZL_Vector3 *va = (ZL_Vector3*)&Verts[11*a], *vb = (ZL_Vector3*)&Verts[11*b], *vd = (ZL_Vector3*)&Verts[11*d];
			ZL_Vector *uva = (ZL_Vector*)(va+2), *uvb = (ZL_Vector*)(vb+2), *uvd = (ZL_Vector*)(vd+2);
			ZL_Vector3 edge1 = *vb - *va, edge2 = *vd - *va;
			ZL_Vector deltaUV1 = *uvb - *uva, deltaUV2 = *uvd - *uva;
			GLfloat f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);
			ZL_Vector3 *ta = (ZL_Vector3*)(uva+1);
			ta->x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
			ta->y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
			ta->z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
			ta->Norm();

			if (Inside) { GLushort is[] = { a,b,c , a,c,d }; Indices.insert(Indices.end(), is, is + COUNT_OF(is)); }
			else        { GLushort is[] = { c,b,a , d,c,a }; Indices.insert(Indices.end(), is, is + COUNT_OF(is)); }
		}
	}
	return ZL_ImplMakeOwner<ZL_Mesh>(ZL_Mesh_Impl::Make((ZL_Mesh_Impl::VAMASK_NORMAL|ZL_Mesh_Impl::VAMASK_TEXCOORD|ZL_Mesh_Impl::VAMASK_TANGENT), &Indices[0], (GLsizei)Indices.size(), &Verts[0], (GLsizei)(Verts.size()/11), ZL_ImplFromOwner<ZL_Material_Impl>(Material)), false);
}

ZL_Mesh ZL_Mesh::BuildExtrudePixels(scalar Scale, scalar Depth, const ZL_FileLink& ImgFile, const ZL_Material& Material, bool KeepAlpha, bool AlphaDepth, scalar AlphaDepthAlign, const ZL_Matrix& Transform)
{
	unsigned char* Pixels; int w, h;
	if (!ZL_Texture_Impl::LoadPixelsRGBA(ImgFile.Open(), &Pixels, &w, &h)) return ZL_Mesh();

	const GLscalar CubeVerts[12*6*4] = {
		 s(.5), s(.5), s(.5),  0, 0, 1, 1,1,  1, 0, 0, 0 , -s(.5), s(.5), s(.5),  0, 0, 1, 0,1,  1, 0, 0, 0 , -s(.5),-s(.5), s(.5),  0, 0, 1, 0,0,  1, 0, 0, 0 ,  s(.5),-s(.5), s(.5),  0, 0, 1, 1,0,  1, 0, 0, 0, 
		 s(.5), s(.5),-s(.5),  0, 0,-1, 1,1, -1, 0, 0, 0 ,  s(.5),-s(.5),-s(.5),  0, 0,-1, 1,0, -1, 0, 0, 0 , -s(.5),-s(.5),-s(.5),  0, 0,-1, 0,0, -1, 0, 0, 0 , -s(.5), s(.5),-s(.5),  0, 0,-1, 0,1, -1, 0, 0, 0,
		 s(.5), s(.5), s(.5),  1, 0, 0, 1,1,  0, 1, 0, 0 ,  s(.5),-s(.5), s(.5),  1, 0, 0, 0,1,  0, 1, 0, 0 ,  s(.5),-s(.5),-s(.5),  1, 0, 0, 0,0,  0, 1, 0, 0 ,  s(.5), s(.5),-s(.5),  1, 0, 0, 1,0,  0, 1, 0, 0, 
		-s(.5), s(.5), s(.5), -1, 0, 0, 1,1,  0,-1, 0, 0 , -s(.5), s(.5),-s(.5), -1, 0, 0, 1,0,  0,-1, 0, 0 , -s(.5),-s(.5),-s(.5), -1, 0, 0, 0,0,  0,-1, 0, 0 , -s(.5),-s(.5), s(.5), -1, 0, 0, 0,1,  0,-1, 0, 0,
		 s(.5), s(.5), s(.5),  0, 1, 0, 1,1,  0, 0, 1, 0 ,  s(.5), s(.5),-s(.5),  0, 1, 0, 0,1,  0, 0, 1, 0 , -s(.5), s(.5),-s(.5),  0, 1, 0, 0,0,  0, 0, 1, 0 , -s(.5), s(.5), s(.5),  0, 1, 0, 1,0,  0, 0, 1, 0, 
		 s(.5),-s(.5), s(.5),  0,-1, 0, 1,1,  0, 0,-1, 0 , -s(.5),-s(.5), s(.5),  0,-1, 0, 1,0,  0, 0,-1, 0 , -s(.5),-s(.5),-s(.5),  0,-1, 0, 0,0,  0, 0,-1, 0 ,  s(.5),-s(.5),-s(.5),  0,-1, 0, 0,1,  0, 0,-1, 0,
	};
	const GLushort CubeIndices[] = { 0,1,2,0,2,3 , 4,5,6,4,6,7 , 8,9,10,8,10,11 , 12,13,14,12,14,15 , 16,17,18,16,18,19 , 20,21,22,20,22,23 };

	std::vector<GLscalar> Verts;
	std::vector<GLushort> Indices;
	unsigned char* pPixels = Pixels;
	for (GLscalar y = -h * .5f + .5f, yEnd = y + h - .5f; y < yEnd; y += 1)
	{
		for (GLscalar x = -w * .5f + .5f, xEnd = x + w - .5f; x < xEnd; x += 1, pPixels += 4)
		{
			if (pPixels[3] == 0) continue;
			GLscalar PixelDepth = (AlphaDepth ? pPixels[3] / s(255) : s(1)), PixelDepthShift = (1.f - PixelDepth) * (AlphaDepthAlign - .5f);
			if (!KeepAlpha) pPixels[3] = 255;
			Verts.resize(Verts.size() + COUNT_OF(CubeVerts));
			Indices.resize(Indices.size() + COUNT_OF(CubeIndices));
			GLscalar PixelColorAsScalar; memcpy(&PixelColorAsScalar, pPixels, 4);
			GLscalar* pVerts   = &Verts  [Verts.size()   - COUNT_OF(CubeVerts)  ]; memcpy(pVerts,   CubeVerts,   sizeof(CubeVerts  ));
			GLushort* pIndices = &Indices[Indices.size() - COUNT_OF(CubeIndices)]; memcpy(pIndices, CubeIndices, sizeof(CubeIndices));
			for (GLscalar* pVertsEnd = pVerts + COUNT_OF(CubeVerts); pVerts != pVertsEnd; pVerts += 12)
			{
				pVerts[0] = (x + pVerts[0]) * Scale;
				pVerts[1] = (y + pVerts[1]) * Scale;
				pVerts[2] = (pVerts[2] * PixelDepth + PixelDepthShift) * Depth;
				*(ZL_Vector3*)&pVerts[0] = Transform.TransformPosition(*(ZL_Vector3*)&pVerts[0]);
				*(ZL_Vector3*)&pVerts[3] = Transform.TransformPosition(*(ZL_Vector3*)&pVerts[3]);
				*(ZL_Vector3*)&pVerts[8] = Transform.TransformPosition(*(ZL_Vector3*)&pVerts[8]);
				pVerts[11] = PixelColorAsScalar;
			}
			for (GLushort IndexOffset = (GLushort)((Verts.size() - COUNT_OF(CubeVerts)) / 12), *pIndicesEnd = pIndices + COUNT_OF(CubeIndices); pIndices != pIndicesEnd; pIndices++)
				*pIndices += IndexOffset;
		}
	}
	free(Pixels);
	return ZL_ImplMakeOwner<ZL_Mesh>(ZL_Mesh_Impl::Make((ZL_Mesh_Impl::VAMASK_NORMAL|ZL_Mesh_Impl::VAMASK_TEXCOORD|ZL_Mesh_Impl::VAMASK_TANGENT|ZL_Mesh_Impl::VAMASK_COLOR), &Indices[0], (GLsizei)Indices.size(), &Verts[0], (GLsizei)(Verts.size()/12), ZL_ImplFromOwner<ZL_Material_Impl>(Material)), false);
}

ZL_IMPL_OWNER_INHERITED_IMPLEMENTATIONS(ZL_MeshAnimated)
ZL_MeshAnimated::ZL_MeshAnimated(const ZL_FileLink& ModelFile, const ZL_Material& Material) { impl = ZL_MeshAnimated_Impl::OBJLoadAnimation(ModelFile, ZL_ImplFromOwner<ZL_Material_Impl>(Material)); }
ZL_MeshAnimated& ZL_MeshAnimated::SetFrame(unsigned int FrameIndex) { if (impl) static_cast<ZL_MeshAnimated_Impl*>(impl)->SetFrame(FrameIndex); return *this; }

ZL_IMPL_OWNER_INHERITED_IMPLEMENTATIONS(ZL_ParticleEmitter)
#define PIMPL static_cast<ZL_ParticleEmitter_Impl*>(impl)
ZL_ParticleEmitter::ZL_ParticleEmitter(scalar LifeTime, size_t MaxParticles, ZL_MaterialModes::Blending BlendMode) { impl = new ZL_ParticleEmitter_Impl(LifeTime, MaxParticles, BlendMode); }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetLifeTime(scalar LifeTime)                                             { PIMPL->Emitter.LifeTime = LifeTime; return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetGravity(const ZL_Vector3& Gravity)                                    { PIMPL->Emitter.Gravity = Gravity; return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetSpawnVelocity(const ZL_Vector3& Velocity)                             { PIMPL->Emitter.VelocityMins =       PIMPL->Emitter.VelocityMaxs = Velocity; return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetSpawnVelocityRanges(const ZL_Vector3& Mins, const ZL_Vector3& Maxs)   { PIMPL->Emitter.VelocityMins = Mins; PIMPL->Emitter.VelocityMaxs = Maxs;     return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetColor(const ZL_Color& Color, bool Alpha)                              { PIMPL->Emitter.ColorMin =        PIMPL->Emitter.ColorMax = Color; PIMPL->Emitter.ColorOverTime = false; if (Alpha) { PIMPL->Emitter.AlphaMin =          PIMPL->Emitter.AlphaMax = Color.a; PIMPL->Emitter.AlphaOverTime = false; } return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetSpawnColorRange(const ZL_Color& Min, const ZL_Color& Max, bool Alpha) { PIMPL->Emitter.ColorMin = Min;   PIMPL->Emitter.ColorMax = Max;   PIMPL->Emitter.ColorOverTime = false; if (Alpha) { PIMPL->Emitter.AlphaMin = Min.a;   PIMPL->Emitter.AlphaMax = Max.a;   PIMPL->Emitter.AlphaOverTime = false; } return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetLifetimeColor(const ZL_Color& Start, const ZL_Color& End, bool Alpha) { PIMPL->Emitter.ColorMin = Start; PIMPL->Emitter.ColorMax = End;   PIMPL->Emitter.ColorOverTime = true;  if (Alpha) { PIMPL->Emitter.AlphaMin = Start.a; PIMPL->Emitter.AlphaMax = End.a;   PIMPL->Emitter.AlphaOverTime = true;  } return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetAlpha(scalar Alpha)                                                   { PIMPL->Emitter.AlphaMin =        PIMPL->Emitter.AlphaMax = Alpha; PIMPL->Emitter.AlphaOverTime = false; return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetSpawnAlphaRange(scalar Min, scalar Max)                               { PIMPL->Emitter.AlphaMin = Min;   PIMPL->Emitter.AlphaMax = Max;   PIMPL->Emitter.AlphaOverTime = false; return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetLifetimeAlpha(scalar Start, scalar End)                               { PIMPL->Emitter.AlphaMin = Start; PIMPL->Emitter.AlphaMax = End;   PIMPL->Emitter.AlphaOverTime = true;  return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetSize(scalar Size)                                                     { PIMPL->Emitter.SizeMin =        PIMPL->Emitter.SizeMax = Size; PIMPL->Emitter.SizeOverTime = false; return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetSpawnSizeRange(scalar Min, scalar Max)                                { PIMPL->Emitter.SizeMin = Min;   PIMPL->Emitter.SizeMax = Max;  PIMPL->Emitter.SizeOverTime = false; return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetLifetimeSize(scalar Start, scalar End)                                { PIMPL->Emitter.SizeMin = Start; PIMPL->Emitter.SizeMax = End;  PIMPL->Emitter.SizeOverTime = true;  return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetAnimationSheet(const ZL_Surface& srf, int Cols, int Rows)             { PIMPL->SetTexture((srf ? ZL_ImplFromOwner<ZL_Surface_Impl>(srf)->tex : NULL), Cols, Rows, true); return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::SetTexture(const ZL_Surface& srf, int Cols, int Rows)                    { PIMPL->SetTexture((srf ? ZL_ImplFromOwner<ZL_Surface_Impl>(srf)->tex : NULL), Cols, Rows, false); return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::Spawn(const ZL_Vector3& Pos) { PIMPL->Spawn(Pos); return *this; }
ZL_ParticleEmitter& ZL_ParticleEmitter::Update(const ZL_Camera& Camera) { PIMPL->Update(Camera); return *this; }
#undef PIMPL

ZL_IMPL_OWNER_NONULLCON_IMPLEMENTATIONS(ZL_Camera)
ZL_Camera::ZL_Camera() : impl(new ZL_Camera_Impl) { }
ZL_Camera& ZL_Camera::SetPosition(const ZL_Vector3& pos) { impl->Pos = pos; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetDirection(const ZL_Vector3& dir) { impl->Dir = dir; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetLookAt(const ZL_Vector3& pos, const ZL_Vector3& targ) { impl->Pos = pos; impl->Dir = (targ - pos).VecNorm(); impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetFOV(scalar fov) { impl->IsOrtho = false; impl->Size = fov; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetOrtho(scalar size) { impl->IsOrtho = true; impl->Size = size; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetAspectRatio(scalar ar) { impl->Aspect = ar; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetClipPlane(scalar znear, scalar zfar) { impl->Near = znear; impl->Far = zfar; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetAmbientLightColor(const ZL_Color& color) { impl->AmbientLightColor = ZL_Vector3(color.r*color.a, color.g*color.a, color.b*color.a); return *this; }
ZL_Vector3 ZL_Camera::GetPosition() const { return impl->Pos; }
ZL_Vector3 ZL_Camera::GetDirection() const { return impl->Dir; }
ZL_Vector3 ZL_Camera::GetUpDirection() const { return impl->VP.GetInverted().GetAxisY().Norm(); }
ZL_Vector3 ZL_Camera::GetRightDirection() const { return impl->VP.GetInverted().GetAxisX().Norm(); }
scalar ZL_Camera::GetFOV() const { return impl->Size; }
scalar ZL_Camera::GetAspectRatio() const { return (impl->Aspect > 0 ? impl->Aspect : ZLWIDTH/ZLHEIGHT); }
scalar ZL_Camera::GetNearClipPlane() const { return impl->Near; }
scalar ZL_Camera::GetFarClipPlane() const { return impl->Far; }
ZL_Color ZL_Camera::GetAmbientLightColor() const { return ZL_Color(impl->AmbientLightColor.x, impl->AmbientLightColor.y, impl->AmbientLightColor.z); }
ZL_Vector ZL_Camera::WorldToScreen(const ZL_Vector3& WorldPosition)
{
	return (impl->VP.PerspectiveTransformPositionTo2D(WorldPosition) + ZLV(1., 1.)) * ZL_Display::Center();
}
ZL_Vector ZL_Camera::WorldToScreen(const ZL_Vector3& WorldPosition, bool* pIsOutsideOfView)
{
	ZL_Vector3 v = impl->VP.PerspectiveTransformPosition(WorldPosition);
	if (pIsOutsideOfView) *pIsOutsideOfView = (v.z < s(-1) || v.z > s(1));
	return ZL_Vector((v.x + s(1)) * ZLHALFW, (v.y + s(1)) * ZLHALFH);
}
ZL_Vector3 ZL_Camera::ScreenToWorld(const ZL_Vector& ScreenPosition, scalar Depth /*= -1*/)
{
	const ZL_Vector Screenspace = (ScreenPosition - ZL_Display::Center()) / ZL_Display::Center();
	return impl->VP.GetInverted().PerspectiveTransformPosition(ZL_Vector3(Screenspace, Depth));
}
void ZL_Camera::ScreenToWorld(const ZL_Vector& ScreenPosition, ZL_Vector3* WorldRayStart, ZL_Vector3* WorldRayEnd)
{
	const ZL_Vector Screenspace = (ScreenPosition - ZL_Display::Center()) / ZL_Display::Center();
	const ZL_Matrix Inverted = impl->VP.GetInverted();
	if (WorldRayStart) *WorldRayStart = Inverted.PerspectiveTransformPosition(ZL_Vector3(Screenspace, -1));
	if (WorldRayEnd) *WorldRayEnd = Inverted.PerspectiveTransformPosition(ZL_Vector3(Screenspace,  1));
}

ZL_IMPL_OWNER_NONULLCON_IMPLEMENTATIONS(ZL_Light)
ZL_Light::ZL_Light() : impl(new ZL_Light_Impl) { }
ZL_Light& ZL_Light::SetPosition(const ZL_Vector3& pos) { impl->Pos = pos; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetDirection(const ZL_Vector3& dir) { impl->Dir = dir; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetLookAt(const ZL_Vector3& pos, const ZL_Vector3& targ) { impl->Pos = pos; impl->Dir = (targ - pos).VecNorm(); impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetPointLight() { impl->Size = 0; impl->Aspect = 0; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetDirectionalLight(scalar area) { impl->Size = area; impl->Aspect = 0; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetSpotLight(scalar angle_degree, scalar aspect) { impl->Size = angle_degree; impl->Aspect = aspect; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetRange(scalar range_far, scalar range_near) { impl->Far = range_far; impl->Near = range_near; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetFalloff(scalar distance) { impl->Falloff = distance; return *this; }
ZL_Light& ZL_Light::SetOutsideLit(bool OutsideLit) { impl->LightFactorOutside = (OutsideLit ? s(1) : s(0)); return *this; }
ZL_Light& ZL_Light::SetShadowBias(scalar bias) { impl->ShadowBias = bias; return *this; }
ZL_Light& ZL_Light::SetColor(const ZL_Color& color) { impl->Color = ZL_Vector3(color.r*color.a, color.g*color.a, color.b*color.a); return *this; }
ZL_Vector3 ZL_Light::GetPosition() const { return impl->Pos; }
ZL_Vector3 ZL_Light::GetDirection() const { return impl->Dir; }
ZL_Color ZL_Light::GetColor() const { return ZL_Color(impl->Color.x, impl->Color.y, impl->Color.z); }

ZL_IMPL_OWNER_NONULLCON_IMPLEMENTATIONS(ZL_RenderList)
ZL_RenderList::ZL_RenderList() : impl(new ZL_RenderList_Impl) { }
void ZL_RenderList::Reset() { impl->Reset(); }
void ZL_RenderList::Add(const ZL_Mesh& Mesh, const ZL_Matrix& Matrix) { impl->Add(ZL_ImplFromOwner<ZL_Mesh_Impl>(Mesh), Matrix, NULL); }
void ZL_RenderList::Add(const ZL_Mesh& Mesh, const ZL_Matrix& Matrix, const ZL_Material& OverrideMaterial) { impl->Add(ZL_ImplFromOwner<ZL_Mesh_Impl>(Mesh), Matrix, ZL_ImplFromOwner<ZL_Material_Impl>(OverrideMaterial)); }
void ZL_RenderList::AddReferenced(const ZL_Mesh& Mesh, const ZL_Matrix& Matrix) { impl->AddReferenced(ZL_ImplFromOwner<ZL_Mesh_Impl>(Mesh), Matrix); }

#if defined(ZILLALOG) && !defined(ZL_VIDEO_OPENGL_ES2)
void ZL_RenderList::DebugDump()
{
	ZL_LOG0("[RENDERLIST]", "----------------------------------------------------------------");
	ZL_LOG1("[RENDERLIST]", "ReferencedMeshes Count: %d", impl->ReferencedMeshes.size());
	ZL_LOG1("[RENDERLIST]", "Meshes Count: %d", impl->Meshes.size());
	ZL_LOG1("[RENDERLIST]", "SortedParts Count: %d", impl->Parts.size());
	for (ZL_RenderList_Impl::PartEntry *e = (impl->Parts.empty() ? NULL : &impl->Parts[0]), *eBegin = e, *eEnd = e + impl->Parts.size(); e != eEnd; e++)
	{
		ZL_LOG4("[RENDERLIST]", "[Part #%d] ShadowMapProgram: %d - Program: %d - MM: %X", e-eBegin, (e->Part->Material->ShaderProgram->ShadowMapProgram ? e->Part->Material->ShaderProgram->ShadowMapProgram->ShaderIDs.Program : 0), e->Part->Material->ShaderProgram->ShaderIDs.Program, e->Part->Material->MaterialModes);
	}
}
#endif

static void ZL_Display3D_BuildDebugColorMat()
{
	assert(!g_DebugColorMat);
	using namespace ZL_Display3D_Shaders;
	const char *VS[COUNT_OF(SharedRules)+COUNT_OF(VSGlobalRules)+COUNT_OF(VSRules)], *FS[COUNT_OF(SharedRules)+COUNT_OF(FSRules)], *FSNullReplacements[] = { NULL, Z3U_COLOR ";" };
	GLsizei VSCount  = BuildList(SharedRules,   COUNT_OF(SharedRules),   MM_STATICCOLOR|MO_UNLIT, &VS[      0]);
	        VSCount += BuildList(VSGlobalRules, COUNT_OF(VSGlobalRules), MM_STATICCOLOR|MO_UNLIT, &VS[VSCount]);
	        VSCount += BuildList(VSRules,       COUNT_OF(VSRules),       MM_STATICCOLOR|MO_UNLIT, &VS[VSCount]);
	GLsizei FSCount  = BuildList(SharedRules,   COUNT_OF(SharedRules),   MM_STATICCOLOR|MO_UNLIT, &FS[      0]);
	        FSCount += BuildList(FSRules,       COUNT_OF(FSRules),       MM_STATICCOLOR|MO_UNLIT, &FS[FSCount], FSNullReplacements);
	g_DebugColorMat = new ZL_MaterialProgram(VSCount, VS, FSCount, FS);
	assert(g_DebugColorMat->ShaderIDs.Program);
}

static void ZL_Display3D_DrawDebugMesh(ZL_Mesh_Impl* DbgMesh, const ZL_Matrix& Matrix, const ZL_Camera& Camera, const ZL_Color& color)
{
	g_DebugColorMat->SetUniformX(0, (scalar*)&color, 4);
	g_DebugColorMat->Activate(g_DebugColorMat->UniformSet, ZL_RenderSceneSetup(ZL_ImplFromOwner<ZL_Camera_Impl>(Camera)));
	DbgMesh->DrawPart(DbgMesh->Parts, Matrix, Matrix);
}

static void ZL_Display3D_InitGL3D(bool RecreateContext)
{
	(void)RecreateContext;
	glDepthMask(GL_TRUE);

	#ifdef ZL_VIDEO_WEAKCONTEXT
	if (RecreateContext)
	{
		if (g_ShadowMap_FBO) { g_ShadowMap_FBO = 0; ZL_Display3D::InitShadowMapping(); }
		if (g_LoadedMaterialPrograms)
			for (std::vector<struct ZL_MaterialProgram*>::iterator itp = g_LoadedMaterialPrograms->begin(); itp != g_LoadedMaterialPrograms->end(); ++itp)
				(*itp)->RecreateOnContextLost();
		if (g_LoadedMeshes)
			for (std::vector<struct ZL_Mesh_Impl*>::iterator itm = g_LoadedMeshes->begin(); itm != g_LoadedMeshes->end(); ++itm)
				((*itm)->WeakIsAnimatedMesh ? static_cast<ZL_MeshAnimated_Impl*>(*itm)->RecreateOnContextLost() : (*itm)->RecreateOnContextLost());
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}
	#endif
}

bool ZL_Display3D::Init(size_t MaxLights)
{
	#ifdef ZL_REQUIRE_INIT3DGLEXTENSIONENTRIES
	ZL_Init3DGLExtensionEntries();
	#endif
	ZL_Display3D_InitGL3D(false);
	funcInitGL3D = ZL_Display3D_InitGL3D;
	assert(MaxLights < 100 && MaxLights < ZL_RenderSceneSetup::MAX_LIGHTS);
	g_MaxLights = (GLubyte)MIN(MaxLights, ZL_RenderSceneSetup::MAX_LIGHTS);
	sprintf(ZL_Display3D_Shaders::Const_NumLightsNumberPtr, "%d;", (int)MaxLights);
	return true;
}

bool ZL_Display3D::InitShadowMapping()
{
	if (g_ShadowMap_FBO) return false;

	using namespace ZL_Display3D_Shaders;
	#define Z3SM_LIGHTSPACE        ZL_SHADERVARNAME("ms", "sm_lightspace") //position in light space (vec3 or vec4)
	#define Z3SM_TXL               ZL_SHADERVARNAME("mt", "sm_txl")
	#define Z3SM_BIASDEPTH         ZL_SHADERVARNAME("mb", "sm_biaseddepth")
	#define Z3SM_LIGHTUNPROJECTED  ZL_SHADERVARNAME("mu", "sm_lightunprojected")
	//#define ZL_DISPLAY3D_ORTHOLIGHTSONLY

	#ifdef ZL_DISPLAY3D_ORTHOLIGHTSONLY
	ExternalSource[EXTERN_Varying_ShadowMap] = "varying vec3 " Z3SM_LIGHTSPACE ";";
	ExternalSource[EXTERN_VS_ShadowMap_Calc] = Z3SM_LIGHTSPACE " = vec3(" Z3U_LIGHT " * vec4(" Z3V_POSITION ",1));";
	#else
	ExternalSource[EXTERN_Varying_ShadowMap] = "varying vec4 " Z3SM_LIGHTUNPROJECTED ";";
	ExternalSource[EXTERN_VS_ShadowMap_Calc] = Z3SM_LIGHTUNPROJECTED " = " Z3U_LIGHT " * vec4(" Z3V_POSITION ",1);";
	#endif
	ExternalSource[EXTERN_VS_ShadowMap_Defs] = "uniform mat4 " Z3U_LIGHT ";";
	ExternalSource[EXTERN_FS_ShadowMap_Defs] =
			"uniform sampler2D " Z3U_SHADOWMAP ";"
			"const float " Z3SM_TXL " = 1./" SHADOWMAP_SIZE_STRING ".;";
	ExternalSource[EXTERN_FS_ShadowMap_Calc] =
			"if (i == 0)" 
			"{" 
				#ifndef ZL_DISPLAY3D_ORTHOLIGHTSONLY
				"vec3 " Z3SM_LIGHTSPACE " = " Z3SM_LIGHTUNPROJECTED ".xyz / " Z3SM_LIGHTUNPROJECTED ".w;"
				#endif
				"if (" Z3SM_LIGHTSPACE ".z < 1. && " Z3SM_LIGHTSPACE ".x >= 0. && " Z3SM_LIGHTSPACE ".x <= 1. && " Z3SM_LIGHTSPACE ".y >= 0. && " Z3SM_LIGHTSPACE ".y <= 1.)"
				"{"
					"float " Z3SM_BIASDEPTH " = " Z3SM_LIGHTSPACE ".z - " Z3U_LIGHTDATA "[3].z;"
					Z3L_LIGHTFACTOR " *= .2 * (step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy).r)"
					                         "+step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy + vec2(-" Z3SM_TXL ",-" Z3SM_TXL ")).r)"
					                         "+step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy + vec2( " Z3SM_TXL ",-" Z3SM_TXL ")).r)"
					                         "+step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy + vec2(-" Z3SM_TXL ", " Z3SM_TXL ")).r)"
					                         "+step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy + vec2( " Z3SM_TXL ", " Z3SM_TXL ")).r));"
				"}"
				"else " Z3L_LIGHTFACTOR " *= " Z3U_LIGHTDATA "[3].y;"
			"}";

	struct Func
	{
		static void SetupShadowMapProgram(ZL_MaterialProgram* ShaderProgram, unsigned int MM, const char* CustomVertexCode = NULL)
		{
			using namespace ZL_Display3D_Shaders;
			bool UseMasked = (MM&MM_DIFFUSEMAP) && (MM&(MO_MASKED|MO_TRANSPARENCY)), UseCustomPosition = !!(MM&(MM_VERTEXFUNC|MM_POSITIONFUNC)), UseCustomMask = (UseMasked && (MM&(MM_UVFUNC)));
			ZL_MaterialProgram*& UseShadowMapProgram = (UseCustomPosition || UseCustomMask ? ShaderProgram->ShadowMapProgram : (UseMasked ? g_ShadowMapMaskProgram : g_ShadowMapProgram));
			if (UseShadowMapProgram) UseShadowMapProgram->AddRef();
			else
			{
				unsigned int MMSMRules = MO_UNLIT;
				if (UseMasked)         MMSMRules |= MO_MASKED | (UseCustomMask ? (MM&(MM_UVFUNC|MM_DIFFUSEMAP|MM_DIFFUSEFUNC)) : MM_DIFFUSEMAP);
				if (UseCustomPosition) MMSMRules |= (MM&(MM_VERTEXFUNC|MM_POSITIONFUNC));
				unsigned int MMSMGlobal = MMSMRules | (UseCustomPosition|UseCustomMask ? (MM & MMDEF_REQUESTS) : 0);
				const char *VS[1+COUNT_OF(SharedRules)+COUNT_OF(VSGlobalRules)+COUNT_OF(VSRules)], *FS[COUNT_OF(SharedRules)+COUNT_OF(FSRules)];
				GLsizei VSCount = 0;
				if (MMSMRules & MM_VERTEXFUNC) VS[VSCount++] = "#define " Z3D_SHADOWMAP "\n";
				        VSCount += BuildList(SharedRules,      COUNT_OF(SharedRules),      MMSMRules,  &VS[VSCount]);
				        VSCount += BuildList(VSGlobalRules,    COUNT_OF(VSGlobalRules),    MMSMGlobal, &VS[VSCount]);
				        VSCount += BuildList(VSRules,          COUNT_OF(VSRules),          MMSMRules,  &VS[VSCount], &CustomVertexCode);
				GLsizei FSCount  = BuildList(SharedRules,      COUNT_OF(SharedRules),      MMSMRules,  &FS[      0]);
				        FSCount += BuildList(ShadowMapFSRules, COUNT_OF(ShadowMapFSRules), MMSMRules,  &FS[FSCount]);
				UseShadowMapProgram = new ZL_MaterialProgram(VSCount, VS, FSCount, FS);
				if (!UseShadowMapProgram->ShaderIDs.Program) { delete UseShadowMapProgram; UseShadowMapProgram = NULL; }
			}
			ShaderProgram->ShadowMapProgram = UseShadowMapProgram;
		}
	};

	glGenTextures(1, &g_ShadowMap_TEX);
	glBindTexture(GL_TEXTURE_2D, g_ShadowMap_TEX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_SIZE, SHADOWMAP_SIZE, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, NULL); //GL_DEPTH_COMPONENT32F and GL_FLOAT not available on __WEBAPP__
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); //GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //GL_REPEAT);

	glGenFramebuffers(1, &g_ShadowMap_FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, g_ShadowMap_FBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_ShadowMap_TEX, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, g_ShadowMap_TEX);
	glActiveTexture(GL_TEXTURE0);

	g_SetupShadowMapProgram = &Func::SetupShadowMapProgram;

	#if defined(ZILLALOG) && (0||ZL_DISPLAY3D_TEST_VARIOUS_SHADER_TEMPLATES)
	using namespace ZL_MaterialModes;
	ZL_Material(MM_STATICCOLOR|MO_UNLIT);
	ZL_Material(MM_STATICCOLOR|MM_VERTEXCOLOR|MM_DIFFUSEMAP|MO_UNLIT);
	ZL_Material(MM_STATICCOLOR|MO_RECEIVENOSHADOW);
	ZL_Material(MM_DIFFUSEMAP|MM_SPECULARSTATIC|MM_NORMALMAP);
	ZL_Material(MM_STATICCOLOR|MM_PARALLAXMAP|MO_UNLIT);
	ZL_Material(MM_DIFFUSEFUNC|MM_STATICCOLOR|MR_POSITION|MO_UNLIT, "vec4 CalcDiffuse() { return vec4(1.); }");
	ZL_Material(MM_DIFFUSEFUNC|MM_STATICCOLOR|MR_TEXCOORD|MO_UNLIT, "vec4 CalcDiffuse() { return vec4(1.); }");
	ZL_Material(MM_DIFFUSEFUNC|MM_STATICCOLOR|MR_NORMAL|MO_UNLIT, "vec4 CalcDiffuse() { return vec4(1.); }");
	ZL_Material(MM_DIFFUSEFUNC|MM_STATICCOLOR|MR_CAMERATANGENT|MO_UNLIT, "vec4 CalcDiffuse() { return vec4(1.); }");
	ZL_Material(MM_DIFFUSEFUNC|MM_STATICCOLOR|MR_CAMERATANGENT|MO_PRECISIONTANGENT|MO_UNLIT, "vec4 CalcDiffuse() { return vec4(1.); }");
	#endif

	return true;
}

void ZL_Display3D::BeginRendering()
{
	if (ZLGLSL::ActiveProgram == ZLGLSL::TEXTURE) glDisableVertexAttribArrayUnbuffered(ZLGLSL::ATTR_TEXCOORD);
	ZLGLSL::ActiveProgram = ZLGLSL::DISPLAY3D;

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);
}

void ZL_Display3D::FinishRendering()
{
	if (g_Active3D.AttributeMask & ZL_Mesh_Impl::VAMASK_NORMAL  ) glDisableVertexAttribArray(ZL_Mesh_Impl::VA_NORMAL  );
	if (g_Active3D.AttributeMask & ZL_Mesh_Impl::VAMASK_TEXCOORD) glDisableVertexAttribArray(ZL_Mesh_Impl::VA_TEXCOORD);
	if (g_Active3D.AttributeMask & ZL_Mesh_Impl::VAMASK_TANGENT ) glDisableVertexAttribArray(ZL_Mesh_Impl::VA_TANGENT );
	if (g_Active3D.AttributeMask & ZL_Mesh_Impl::VAMASK_COLOR   ) glDisableVertexAttribArray(ZL_Mesh_Impl::VA_COLOR   );
	if (g_Active3D.Texture && g_Active3D.Texture != GL_TEXTURE0) glActiveTexture(GL_TEXTURE0);
	if (g_Active3D.IndexBuffer) glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	if (g_Active3D.VertexBuffer) glBindBuffer(GL_ARRAY_BUFFER, 0);
	memset(&g_Active3D, 0, sizeof(g_Active3D));
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	ZLGLSL::DisableProgram();

	#if defined(ZILLALOG) && (0||ZL_DISPLAY3D_TEST_SHOWSHADOWMAP)
	glBindTexture(GL_TEXTURE_2D, g_ShadowMap_TEX);
	GLscalar halfscreen[8] = { 0,.5f , .5f/ZLASPECTR,.5f , 0,0 , .5f/ZLASPECTR,0 };
	GLscalar fullbox[8] = { 0,1 , 1,1 , 0,0 , 1,0 };
	GLscalar matrix[16] = { 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, -1, 0, -1, -1, 0, 1 };
	ZLGLSL::_TEXTURE_PROGRAM_ACTIVATE();
	glVertexAttrib4(ZLGLSL::ATTR_COLOR, 3, 3, 3, 1);
	glUniformMatrix4v(ZLGLSL::UNI_MVP, 1, GL_FALSE, matrix);
	glEnableVertexAttribArrayUnbuffered(ZLGLSL::ATTR_TEXCOORD);
	glVertexAttribPointerUnbuffered(ZLGLSL::ATTR_POSITION, 2, GL_SCALAR, GL_FALSE, 0, halfscreen);
	glVertexAttribPointerUnbuffered(ZLGLSL::ATTR_TEXCOORD, 2, GL_SCALAR, GL_FALSE, 0, fullbox);
	glDrawArraysUnbuffered(GL_TRIANGLE_STRIP, 0, 4);
	ZLGLSL::ActiveProgram = ZLGLSL::NONE; //reset matrix on next draw
	#endif
}

void ZL_Display3D::DrawLists(const ZL_RenderList*const* RenderLists, size_t NumLists, const ZL_Camera& Camera)
{
	for (size_t i = 0; i != NumLists; i++) ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[i])->Sort();
	bool TemporaryRendering = (ZLGLSL::ActiveProgram != ZLGLSL::DISPLAY3D);
	if (TemporaryRendering) BeginRendering();
	ZL_RenderSceneSetup Scene(ZL_ImplFromOwner<ZL_Camera_Impl>(Camera));
	for (size_t j = 0; j != NumLists; j++)
		ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[j])->RenderColor(Scene);
	if (TemporaryRendering) FinishRendering();
}

void ZL_Display3D::DrawListsWithLights(const ZL_RenderList*const* RenderLists, size_t NumLists, const ZL_Camera& Camera, const ZL_Light*const* Lights, size_t NumLights)
{
	for (size_t i = 0; i != NumLists; i++) ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[i])->Sort();
	assert(NumLights <= g_MaxLights);
	bool TemporaryRendering = (ZLGLSL::ActiveProgram != ZLGLSL::DISPLAY3D);
	if (TemporaryRendering) BeginRendering();

	ZL_RenderSceneSetup Scene;
	Scene.LightDataCount = 1;
	Scene.LightData[0] = ZL_ImplFromOwner<ZL_Camera_Impl>(Camera)->AmbientLightColor;
	if (NumLights)
	{
		if (g_ShadowMap_FBO)
		{
			glBindFramebuffer(GL_FRAMEBUFFER, g_ShadowMap_FBO);
			glViewport(0, 0, SHADOWMAP_SIZE, SHADOWMAP_SIZE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glClear(GL_DEPTH_BUFFER_BIT);

			Scene.Camera = ZL_ImplFromOwner<ZL_Light_Impl>(*Lights[0]);
			for (size_t i = 0; i != NumLists; i++)
				ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[i])->RenderShadowMap(Scene);

			glBindFramebuffer(GL_FRAMEBUFFER, active_framebuffer);
			glViewport(active_viewport[0], active_viewport[1], active_viewport[2], active_viewport[3]);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}

		Scene.Light = ZL_ImplFromOwner<ZL_Light_Impl>(*Lights[0]);
		for (size_t i = 0, iEnd = MIN(NumLights, ZL_RenderSceneSetup::MAX_LIGHTS); i != iEnd; i++)
		{
			ZL_Light_Impl* Light = ZL_ImplFromOwner<ZL_Light_Impl>(*Lights[i]);
			if (Light->IsDirectionalLight() && g_ShadowMap_FBO)
			{
				Scene.LightData[Scene.LightDataCount++] = -Light->Dir;
				Scene.LightData[Scene.LightDataCount++] = Light->Color;
				Scene.LightData[Scene.LightDataCount++] = ZL_Vector3(-Light->Pos.GetLength(), Light->LightFactorOutside, Light->ShadowBias);
			}
			else
			{
				Scene.LightData[Scene.LightDataCount++] = Light->Pos;
				Scene.LightData[Scene.LightDataCount++] = Light->Color;
				Scene.LightData[Scene.LightDataCount++] = ZL_Vector3(Light->Falloff, Light->LightFactorOutside, Light->ShadowBias);
			}
		}
	}
	Scene.LightData[Scene.LightDataCount++].x = 3.4e+38f; //list end terminator
	Scene.CalcLightDataChkSum();

	Scene.Camera = ZL_ImplFromOwner<ZL_Camera_Impl>(Camera);

	#if defined(ZILLALOG) && defined(ZL_DISPLAY3D_ENABLE_SHIFT_DEBUG_VIEW)
	if (ZL_Display::KeyDown[ZLK_LSHIFT])
	{
		static ZL_Camera DebugCam;
		static float DebugDist = 0.0f;
		static float DebugLastMouseY = ZL_Display::PointerY;
		if (ZL_Display::KeyDown[ZLK_LCTRL]) DebugDist += 0.1f*(DebugLastMouseY - ZL_Display::PointerY);
		#if !defined(ZL_VIDEO_OPENGL_ES2)
		if (ZL_Display::KeyDown[ZLK_LALT]) { glDisable(GL_CULL_FACE); glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
		#endif
		DebugLastMouseY = ZL_Display::PointerY;
		DebugCam.SetDirection(ZL_Vector3::Forward.
			VecRotate(ZL_Vector3::Right, -(ZL_Display::PointerY-ZLHALFH)/ZLHALFH*PIHALF*0.99f).
			VecRotate(ZL_Vector3::Up,     (ZL_Display::PointerX-ZLHALFW)/ZLHALFW*PI));
		DebugCam.SetPosition(Camera.GetPosition() + Camera.GetDirection() * 5.0f + DebugCam.GetDirection() * -(5.0f+DebugDist));
		Scene.Camera = ZL_ImplFromOwner<ZL_Camera_Impl>(DebugCam);

		for (size_t j = 0; j != NumLists; j++)
			ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[j])->RenderColor(Scene);

		#if 1
		if (ZL_Display::KeyDown[ZLK_LCTRL]) glClear(GL_DEPTH_BUFFER_BIT);
		ZL_Display3D::DrawLine(DebugCam, ZL_Vector3(0,0,0), ZL_Vector3(100,0,0), ZL_Color::Red,   .02f);
		ZL_Display3D::DrawLine(DebugCam, ZL_Vector3(0,0,0), ZL_Vector3(0,100,0), ZL_Color::Green, .02f);
		ZL_Display3D::DrawLine(DebugCam, ZL_Vector3(0,0,0), ZL_Vector3(0,0,100), ZL_Color::Blue,  .02f);
		ZL_Display3D::DrawFrustum(DebugCam, ZL_ImplFromOwner<ZL_Camera_Impl>(Camera)->VP, ZL_Color::Magenta);
		ZL_Display3D::DrawLine(DebugCam, ZL_ImplFromOwner<ZL_Camera_Impl>(Camera)->Pos, ZL_ImplFromOwner<ZL_Camera_Impl>(Camera)->Pos + Camera.GetRightDirection() * 100, ZL_Color::Red, .02f);
		ZL_Display3D::DrawLine(DebugCam, ZL_ImplFromOwner<ZL_Camera_Impl>(Camera)->Pos, ZL_ImplFromOwner<ZL_Camera_Impl>(Camera)->Pos + Camera.GetUpDirection() * 100, ZL_Color::Blue, .02f);
		if (NumLights && g_ShadowMap_FBO) ZL_Display3D::DrawFrustum(DebugCam, ZL_ImplFromOwner<ZL_Light_Impl>(*Lights[0])->VP, ZL_Color::Yellow);
		for (size_t k = 0; k != NumLights; k++) ZL_Display3D::DrawLine(DebugCam, Lights[k]->GetPosition(), Lights[k]->GetPosition() + Lights[k]->GetDirection(), ZL_Color::Yellow, 0.02f);
		for (size_t k = (g_ShadowMap_FBO ? 1 : 0); k != NumLights; k++) for (int dir = 0; dir < 3; dir++) ZL_Display3D::DrawPlane(DebugCam, Lights[k]->GetPosition(), (!dir?ZL_Vector3::Up:dir==1?ZL_Vector3::Right:ZL_Vector3::Forward), ZLV(1.5,1.5), ZL_Color::Yellow);
		//for (size_t j = 0; j != NumLists; j++) for (ZL_RenderList_Impl::MeshEntry m : ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[j])->Meshes) m.Mesh->DrawDebug(m.ModelMatrix, DebugCam);
		#endif
		#if !defined(ZL_VIDEO_OPENGL_ES2)
		if (ZL_Display::KeyDown[ZLK_LALT]) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); glEnable(GL_CULL_FACE); }
		#endif
	}
	else
	#endif
	for (size_t j = 0; j != NumLists; j++)
		ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[j])->RenderColor(Scene);
	if (TemporaryRendering) FinishRendering();
}

void ZL_Display3D::DrawLine(const ZL_Camera& cam, const ZL_Vector3& a, const ZL_Vector3& b, const ZL_Color& color, scalar width)
{
	static ZL_Mesh_Impl* DbgMesh;
	if (!DbgMesh)
	{
		if (!g_DebugColorMat) ZL_Display3D_BuildDebugColorMat();
		ZL_Vector3 w(s(.5),0,0), v(0,s(.5),0), fwd(0,0,1), verts[] = { w, fwd+w, fwd-w, -w, v, fwd+v, fwd-v, -v };
		GLushort indices[] = { 0,1,2 , 0,2,3 , 2,1,0 , 3,2,0 , 4,5,6 , 4,6,7 , 6,5,4 , 7,6,4 };
		DbgMesh = ZL_Mesh_Impl::Make(0, indices, COUNT_OF(indices), verts, COUNT_OF(verts), g_DebugColorMat);
	}
	ZL_Vector3 delta = (b-a); scalar dist = delta.GetLength();
	if (dist > SMALL_NUMBER) ZL_Display3D_DrawDebugMesh(DbgMesh, ZL_Matrix::MakeRotateTranslateScale(delta / dist, a, ZL_Vector3(width, width, dist)), cam, color);
}

void ZL_Display3D::DrawPlane(const ZL_Camera& cam, const ZL_Vector3& pos, const ZL_Vector3& normal, const ZL_Vector& extents, const ZL_Color& color)
{
	static ZL_Mesh_Impl* DbgMesh;
	if (!DbgMesh)
	{
		if (!g_DebugColorMat) ZL_Display3D_BuildDebugColorMat();
		ZL_Vector3 verts[] = { ZL_Vector3(1,-1,0), ZL_Vector3(-1,-1,0), ZL_Vector3(-1,1,0), ZL_Vector3(1,1,0) };
		GLushort indices[] = { 0,1,2 , 0,2,3 , 2,1,0 , 3,2,0 };
		DbgMesh = ZL_Mesh_Impl::Make(0, indices, COUNT_OF(indices), verts, COUNT_OF(verts), g_DebugColorMat);
	}
	ZL_Display3D_DrawDebugMesh(DbgMesh, ZL_Matrix::MakeRotateTranslateScale(normal, pos, ZL_Vector3(extents.x, extents.y, 1)), cam, color);
}

void ZL_Display3D::DrawFrustum(const ZL_Camera& cam, const ZL_Matrix& VPMatrix, const ZL_Color& color, scalar width)
{
	ZL_Matrix VPI = VPMatrix.GetInverted();
	ZL_Vector3 Points[8] = { VPI.PerspectiveTransformPosition(ZL_Vector3(-1,-1,-1)), VPI.PerspectiveTransformPosition(ZL_Vector3(-1,-1, 1)),
	                         VPI.PerspectiveTransformPosition(ZL_Vector3( 1,-1,-1)), VPI.PerspectiveTransformPosition(ZL_Vector3( 1,-1, 1)),
	                         VPI.PerspectiveTransformPosition(ZL_Vector3( 1, 1,-1)), VPI.PerspectiveTransformPosition(ZL_Vector3( 1, 1, 1)),
	                         VPI.PerspectiveTransformPosition(ZL_Vector3(-1, 1,-1)), VPI.PerspectiveTransformPosition(ZL_Vector3(-1, 1, 1)) };
	int Edges[12*2] = { 0,1 , 2,3 , 4,5 , 6,7 , 0,2 , 0,6 , 1,7 , 1,3 , 2,4 , 3,5 , 4,6 , 5,7 };
	for (int i = 0; i < 12*2; i+=2) DrawLine(cam, Points[Edges[i]], Points[Edges[i+1]], color, width);
}

#endif
