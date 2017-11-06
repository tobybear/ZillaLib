/*
  ZillaLib
  Copyright (C) 2010-2016 Bernhard Schelling

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

//#define ZL_DISPLAY3D_ALLOW_SHIFT_DEBUG_VIEW

struct ZL_ShaderIDs { GLuint Program; GLubyte UsedAttributeMask; GLint UniformMatrixModel, UniformMatrixNormal; };
static struct { ZL_ShaderIDs Shader; GLuint BoundTextureChksum, BoundTextures[4], IndexBuffer, VertexBuffer; GLubyte AttributeMask; GLenum Texture; } g_Active3D;
static std::vector<struct ZL_MaterialProgram*>* g_LoadedShaderVariations;
static GLubyte g_MaxLights;
static ZL_MaterialProgram* g_DebugColorMat;

#ifdef ZL_VIDEO_WEAKCONTEXT
static std::vector<struct ZL_MaterialProgram*> *g_LoadedMaterialPrograms;
static std::vector<struct ZL_Mesh_Impl*> *g_LoadedMeshes;
#endif

static GLuint g_ShadowMap_FBO, g_ShadowMap_TEX;
static ZL_MaterialProgram *g_ShadowMapProgram, *g_ShadowMapMaskProgram;
#define SHADOWMAP_SIZE         2048
#define SHADOWMAP_SIZE_STRING "2048"

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
	#define Z3L_DIRECTION2LIGHT  ZL_SHADERVARNAME("DL", "l_direction2light")

	static const char S_VoidMain[] = "void main(){";

	static const char *S_AttributeList[] = { Z3A_POSITION, Z3A_NORMAL, Z3A_TEXCOORD, Z3A_TANGENT, Z3A_COLOR };

	using namespace ZL_MaterialModes;
	enum
	{
		MMDEF_USECUSTOMFRAGMENT         = MM_DIFFUSEFUNC|MM_NORMALFUNC|MM_SPECULARFUNC,
		MMDEF_USECUSTOMVERTEX           = MM_VERTEXCOLORFUNC|MM_POSITIONFUNC,
		MMDEF_USECUSTOMSHADER           = MMDEF_USECUSTOMFRAGMENT|MMDEF_USECUSTOMVERTEX,
		MMDEF_FRAGCOLORMODES            = MM_STATICCOLOR|MM_VERTEXCOLOR|MM_VERTEXCOLORFUNC|MM_DIFFUSEMAP|MM_DIFFUSEFUNC,
		MMDEF_USESLIT                   = MM_SPECULARSTATIC|MM_SPECULARMAP|MM_SPECULARFUNC|MM_NORMALMAP|MM_NORMALFUNC|MM_SHADOWMAP,
		MMDEF_VSGLOBALREQUESTS          = MR_POSITION|MR_TEXCOORD|MR_NORMALS|MR_CAMERATANGENT|MR_PRECISIONTANGENT|MR_TIME,
		MMDEF_RULEWITHEXTERNALSOURCEPTR = MM_SHADOWMAP,
		MMDEF_WITHOUTOPTIONS            = MO_TRANSPARENCY-1,

		MMUSE_VERTEXCOLOR      = MM_VERTEXCOLOR|MM_VERTEXCOLORFUNC,
		MMUSE_SPECULAR         = MM_SPECULARSTATIC|MM_SPECULARMAP|MM_SPECULARFUNC,
		MMUSE_CAMERATANGENT    = MM_PARALLAXMAP|MR_CAMERATANGENT,
		MMUSE_TANGENT          = MMUSE_CAMERATANGENT,
		MMUSE_DYNTEXCOORD      = MM_PARALLAXMAP,
		MMUSE_TEXCOORD         = MMUSE_DYNTEXCOORD|MM_DIFFUSEMAP|MM_SPECULARMAP|MM_NORMALMAP|MR_TEXCOORD,
		MMUSE_NORMAL           = MMUSE_TANGENT|MM_LIT|MM_PARALLAXMAP|MR_NORMALS,
		MMUSE_POSITION         = MMUSE_TANGENT|MM_LIT|MR_POSITION,
	};

	static const char* Varying_ShadowMap, *VS_ShadowMap_Defs, *FS_ShadowMap_Defs, *VS_ShadowMap_Calc, *FS_ShadowMap_Calc;
	static char Const_NumLights[] = "const int " Z3S_NUMLIGHTS "=   ", *Const_NumLightsNumberPtr = Const_NumLights+COUNT_OF(Const_NumLights)-4;

	static const struct SourceRule { int MMUseIf, MMLimit; const char *Source; }
		SharedRules[] = {
			#ifdef ZL_VIDEO_OPENGL_ES2
			{ 0,0,ZLGLSL_LIST_HIGH_PRECISION_HEADER },
			#endif
			{ MMUSE_VERTEXCOLOR,                      0, "varying vec4 " Z3V_COLOR ";" },
			{ MMUSE_TEXCOORD,                         0, "varying vec2 " Z3V_TEXCOORD ";" },
			{ MMUSE_CAMERATANGENT,  MR_PRECISIONTANGENT, "varying vec3 " Z3V_CAMERATANGENT ";" },
			{ MMUSE_TANGENT,       -MR_PRECISIONTANGENT, "varying vec3 " Z3V_TANGENT ", " Z3V_BITANGENT ";" },
			{ MMUSE_NORMAL,                           0, "varying vec3 " Z3V_NORMAL ";" },
			{ MMUSE_POSITION,                         0, "varying vec3 " Z3V_POSITION ";" },
			{ MM_SHADOWMAP,                           0, (const char *)&Varying_ShadowMap },
		},
		VSGlobalRules[] = {
			{ 0,                                      0, "uniform mat4 " Z3U_VIEW "," Z3U_MODEL ";attribute vec3 " Z3A_POSITION ";" },
			{ MMUSE_TEXCOORD,                         0, "attribute vec2 " Z3A_TEXCOORD ";" },
			{ MM_VERTEXCOLOR,                         0, "attribute vec4 " Z3A_COLOR ";" },
			{ MMUSE_NORMAL,                           0, "attribute vec3 " Z3A_NORMAL ";uniform mat4 " Z3U_NORMAL ";" },
			{ MMUSE_TANGENT,        MR_PRECISIONTANGENT, "attribute vec3 " Z3A_TANGENT ";uniform vec3 " Z3U_VIEWPOS ";" },
			{ MMUSE_TANGENT,       -MR_PRECISIONTANGENT, "attribute vec3 " Z3A_TANGENT ";" },
			{ MR_TIME,                                0, "uniform float " Z3U_TIME ";" },
		},
		VSRules[] = {
			{ 0,0,0 }, // <-- Custom Vertex Code
			{ MM_SHADOWMAP,                           0, (const char *)&VS_ShadowMap_Defs },
			{ 0,0,S_VoidMain },
			{ MM_POSITIONFUNC,                        0, Z3O_POSITION " = CalcPosition();" },
			{ 0,                        MM_POSITIONFUNC, Z3O_POSITION " = " Z3U_MODEL " * vec4(" Z3A_POSITION ", 1);" },
			{ MMUSE_POSITION,                         0, Z3V_POSITION " = " Z3O_POSITION ".xyz;" },
			{ 0,                                      0, Z3O_POSITION " = " Z3U_VIEW " * " Z3O_POSITION ";" },
			{ MMUSE_TEXCOORD,                         0, Z3V_TEXCOORD " = " Z3A_TEXCOORD ";" },
			{ MMUSE_NORMAL,                           0, Z3V_NORMAL " = normalize(vec3(" Z3U_NORMAL " * vec4(" Z3A_NORMAL ", 0)));" },
			{ MMUSE_TANGENT,        MR_PRECISIONTANGENT, "vec3 " Z3L_TANGENT " = normalize(vec3(" Z3U_NORMAL " * vec4(" Z3A_TANGENT ", 0)))," Z3L_BITANGENT " = cross(" Z3V_NORMAL ", " Z3L_TANGENT ");" },
			{ MMUSE_CAMERATANGENT,  MR_PRECISIONTANGENT, "vec3 " Z3L_POS2CAMERA " = " Z3U_VIEWPOS " - " Z3V_POSITION ";" Z3V_CAMERATANGENT " = normalize(vec3(dot(" Z3L_POS2CAMERA ", " Z3L_TANGENT "), dot(" Z3L_POS2CAMERA ", " Z3L_BITANGENT "), dot(" Z3L_POS2CAMERA ", " Z3V_NORMAL ")));" },
			{ MMUSE_TANGENT,       -MR_PRECISIONTANGENT, Z3V_TANGENT " = normalize(vec3(" Z3U_NORMAL " * vec4(" Z3A_TANGENT ", 0)));" Z3V_BITANGENT " = cross(" Z3V_NORMAL ", " Z3V_TANGENT ");" },
			{ MM_VERTEXCOLOR,                         0, Z3V_COLOR " = " Z3A_COLOR ";" },
			{ MM_VERTEXCOLORFUNC,                     0, Z3V_COLOR " = CalcColor();" },
			{ MM_SHADOWMAP,                           0, (const char *)&VS_ShadowMap_Calc },
			{ 0,0,"}" }
		},
		FSRules[] = {
			{ MM_LIT,                                 0, Const_NumLights },
			{ MM_STATICCOLOR,                         0, "uniform vec4 " Z3U_COLOR ";" },
			{ MM_DIFFUSEMAP,                          0, "uniform sampler2D " Z3U_DIFFUSEMAP ";" },
			{ MM_PARALLAXMAP,                         0, "uniform sampler2D " Z3U_PARALLAXMAP ";uniform float " Z3U_PARALLAXSCALE ";" },
			{ MMUSE_SPECULAR|MMUSE_CAMERATANGENT,     0, "uniform vec3 " Z3U_VIEWPOS ";" },
			{ MM_LIT,                                 0, "uniform vec3 " Z3U_LIGHTDATA "[1+3*" Z3S_NUMLIGHTS "];" },
			{ MM_NORMALMAP,                           0, "uniform sampler2D " Z3U_NORMALMAP ";" },
			{ MMUSE_SPECULAR,                         0, "uniform float " Z3U_SPECULAR ", " Z3U_SHININESS ";" },
			{ MM_SPECULARMAP,                         0, "uniform sampler2D " Z3U_SPECULARMAP ";" },
			{ MR_TIME,                                0, "uniform float " Z3U_TIME ";" },
			{ MMUSE_NORMAL,                           0, "vec3 " Z3S_NORMAL ";" },
			{ MMUSE_CAMERATANGENT,                    0, "vec3 " Z3S_CAMERATANGENT ";" },
			{ MM_SHADOWMAP,                           0, (const char *)&FS_ShadowMap_Defs },
			{ 0,0,0 }, // <-- Custom Fragment Code
			{ 0,0,S_VoidMain },
			{ 0,                                   MM_DIFFUSEFUNC, Z3O_FRAGCOLOR " = " },
			{ 0,                                   MM_DIFFUSEFUNC, 0 }, // <-- FragColor Calculation
			{ MM_MASKED,                           MM_DIFFUSEFUNC, "if (" Z3O_FRAGCOLOR ".a<.5)discard;" },
			{ MMUSE_CAMERATANGENT,            MR_PRECISIONTANGENT, Z3S_CAMERATANGENT " = normalize(" Z3V_CAMERATANGENT ");" },
			{ MMUSE_CAMERATANGENT,           -MR_PRECISIONTANGENT, "vec3 " Z3L_POS2CAMERA " = " Z3U_VIEWPOS " - " Z3V_POSITION ";"
			                                                       Z3S_CAMERATANGENT " = normalize(vec3(dot(" Z3L_POS2CAMERA ", " Z3V_TANGENT "), dot(" Z3L_POS2CAMERA ", " Z3V_BITANGENT "), dot(" Z3L_POS2CAMERA ", " Z3V_NORMAL ")));" },
			{ MMUSE_DYNTEXCOORD,                                0, "vec2 " Z3V_TEXCOORD " = " Z3V_TEXCOORD ";" },
			{ MM_PARALLAXMAP,                                   0, "float " Z3L_PARALLAXDEPTH " = texture2D(" Z3U_PARALLAXMAP ", " Z3V_TEXCOORD ").r;"
			                                                       Z3V_TEXCOORD " += " Z3S_CAMERATANGENT ".xy * (" Z3L_PARALLAXDEPTH " * " Z3U_PARALLAXSCALE ");" },
			{ MM_NORMALMAP,                         MM_NORMALFUNC, Z3S_NORMAL " = texture2D(" Z3U_NORMALMAP ", " Z3V_TEXCOORD ").xyz * 2.0 - 1.0;" },
			{ MM_NORMALFUNC,                         MM_NORMALMAP, Z3S_NORMAL " = CalcNormal();" },
			{ MMUSE_NORMAL,            MM_NORMALMAP|MM_NORMALFUNC, Z3S_NORMAL " = normalize(" Z3V_NORMAL ");" },
			{ 0,                                  -MM_DIFFUSEFUNC, Z3O_FRAGCOLOR " = " },
			{ 0,                                  -MM_DIFFUSEFUNC, 0 }, // <-- FragColor Calculation
			{ MM_MASKED,                          -MM_DIFFUSEFUNC, "if (" Z3O_FRAGCOLOR ".a<.5)discard;" },
			{ MM_PARALLAXMAP,                                   0, Z3O_FRAGCOLOR ".rgb *= .5+" Z3L_PARALLAXDEPTH "*.5;" },
			{ MM_SPECULARMAP,      MMUSE_SPECULAR&~MM_SPECULARMAP, "float " Z3L_SPECULAR " = texture2D(" Z3U_SPECULARMAP ", " Z3V_TEXCOORD ").r * " Z3U_SPECULAR ";" },
			{ MM_SPECULARFUNC,    MMUSE_SPECULAR&~MM_SPECULARFUNC, "float " Z3L_SPECULAR " = CalcSpecular();" },
			{ MM_SPECULARSTATIC,MMUSE_SPECULAR&~MM_SPECULARSTATIC, "float " Z3L_SPECULAR " = " Z3U_SPECULAR ";" },
			{ MM_LIT,                                           0, "vec3 " Z3L_LIGHTSCOLOR " = vec3(0.)," Z3L_LIGHTSSHINE " = vec3(0.);"
			                                                       "for (int i = 0; i < " Z3S_NUMLIGHTS "; i++)" //"const int i = 0;"
			                                                       "{"
			                                                           "if (i > 0 && " Z3U_LIGHTDATA "[1+i*3].x > 3.3e+38) break;"
			                                                           "vec3 " Z3L_POS2LIGHT " = " Z3U_LIGHTDATA "[1+i*3] - " Z3V_POSITION ";"
			                                                           "float " Z3L_LIGHTDISTANCE " = length(" Z3L_POS2LIGHT ");"
			                                                           "vec3 " Z3L_DIRECTION2LIGHT " = " Z3L_POS2LIGHT " / " Z3L_LIGHTDISTANCE ";"
			                                                           "float " Z3L_LIGHTFACTOR " = max(dot(" Z3S_NORMAL ", " Z3L_DIRECTION2LIGHT "), 0.);" },
			{ MMUSE_SPECULAR,                                   0,     "float " Z3L_LIGHTSPECULAR " = " Z3L_SPECULAR " * pow(max(dot(normalize(" Z3U_VIEWPOS " - " Z3V_POSITION "), reflect(-" Z3L_DIRECTION2LIGHT ", " Z3S_NORMAL ")), 0.0), " Z3U_SHININESS ");"
			                                                           Z3O_FRAGCOLOR ".a = min(" Z3O_FRAGCOLOR ".a + " Z3L_LIGHTSPECULAR ", 1.);"
			                                                           Z3L_LIGHTFACTOR " += " Z3L_LIGHTSPECULAR ";" },
			{ MM_SHADOWMAP,                                     0,     (const char *)&FS_ShadowMap_Calc },
			{ MM_LIT,                                           0,     Z3L_LIGHTFACTOR " *= max((" Z3U_LIGHTDATA "[1+i*3+2].x - " Z3L_LIGHTDISTANCE ") / " Z3U_LIGHTDATA "[1+i*3+2].x, 0.);"
			                                                           Z3L_LIGHTSSHINE " += " Z3U_LIGHTDATA "[1+i*3+1] * max(" Z3L_LIGHTFACTOR "-1.,0.);"
			                                                           Z3L_LIGHTSCOLOR " += " Z3U_LIGHTDATA "[1+i*3+1] * " Z3L_LIGHTFACTOR ";"
			                                                       "}"
			                                                       //Z3O_FRAGCOLOR ".rgb = " Z3L_LIGHTSCOLOR ";"
			                                                       Z3O_FRAGCOLOR ".rgb = " Z3O_FRAGCOLOR ".rgb*(" Z3L_LIGHTSCOLOR "+" Z3U_LIGHTDATA "[0]) + " Z3L_LIGHTSSHINE ";"
			},
			{ 0,0,"}" }
		},
		ShadowMapFSRules[] = {
			{ MM_MASKED, 0, "uniform sampler2D " Z3U_DIFFUSEMAP ";" },
			{ 0,         0, S_VoidMain },
			{ MM_MASKED, 0, "if (texture2D(" Z3U_DIFFUSEMAP ", " Z3V_TEXCOORD ").a<.5)discard;" },
			{ 0,         0, "}" }
		}
	;

	static GLsizei BuildList(const SourceRule* Rules, size_t RuleCount, unsigned int MM, const char** List, const char*const* NullReplacements = NULL)
	{
		const char** ListStart = List;
		for (const SourceRule *Rule = Rules, *RuleEnd = Rule+RuleCount; Rule != RuleEnd; Rule++)
		{
			if ((Rule->MMUseIf && !(MM & Rule->MMUseIf)) || (Rule->MMLimit > 0 && (MM & Rule->MMLimit)) || (Rule->MMLimit < 0 && !(MM & -Rule->MMLimit))) continue;
			if (Rule->MMUseIf & MMDEF_RULEWITHEXTERNALSOURCEPTR) *(List++) = *(const char**)Rule->Source;
			else if (Rule->Source) *(List++) = Rule->Source;
			else if (NullReplacements) { if (*NullReplacements) { *(List++) = *NullReplacements; } NullReplacements++; }
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
	ZL_CameraBase_Impl() : Dir(0,0,1) { static GLushort gc; UpdateCount = gc++; }
};

struct ZL_Camera_Impl : public ZL_CameraBase_Impl
{
	scalar FOV, AspectRatio, ZNear, ZFar;
	ZL_Vector3 AmbientLightColor;
	ZL_Camera_Impl() : FOV(s(90)), AspectRatio(-1), ZNear(s(.1)), ZFar(s(1000)), AmbientLightColor(s(.2)) { UpdateMatrix(); }
	void UpdateMatrix() { VP = ZL_Matrix::MakeCamera(Pos, Dir) * ZL_Matrix::MakePerspectiveHorizontal(FOV, (AspectRatio > 0 ? AspectRatio : ZLWIDTH/ZLHEIGHT), ZNear, ZFar); UpdateCount += 0x10000; }
};

struct ZL_Light_Impl : public ZL_CameraBase_Impl
{
	ZL_Matrix BiasedLightMatrix;
	scalar Aspect, Size, Near, Far, Falloff, LightFactorOutside;
	ZL_Vector3 Color;
	ZL_Light_Impl() : Aspect(0), Size(2), Near(1), Far(50), Falloff(3.4e+37f), LightFactorOutside(0), Color(1,1,1) { UpdateMatrix(); }
	void UpdateMatrix()
	{
		if (!Aspect) VP = ZL_Matrix::MakeCamera(Pos, Dir) * ZL_Matrix::MakeOrtho(-Size, Size, -Size, Size, Near, Far);
		else         VP = ZL_Matrix::MakeCamera(Pos, Dir) * ZL_Matrix::MakePerspectiveHorizontal(Size, Aspect, Near, Far);
		BiasedLightMatrix = VP * ZL_Matrix(.5f, 0, 0, 0, 0, .5f, 0, 0, 0, 0, .5f, 0, .5f, .5f, .5f, 1);
		UpdateCount += 0x10000;
	}
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
	struct UniformValue
	{
		bool operator<(const ZL_NameID& b) const { return (Name<b); }
		ZL_NameID Name; GLint Location;
		enum { TYPE_FLOAT, TYPE_VEC2, TYPE_VEC3, TYPE_VEC4 } Type;
		union { scalar Float, Vec2[2], Vec3[3], Vec4[4]; } Value;
	};
	struct sUniformSet
	{
		GLuint Num; UniformValue* Values; GLuint ValueChksum, TextureChksum; ZL_Texture_Impl* TextureReferences[4];
		void CalcValueChksum()   { ZL_STATIC_ASSERT(!(sizeof(UniformValue)&3),BAD_ALIGN);      ValueChksum   = ZL_Checksum::Fast4(Values, sizeof(UniformValue) * Num); }
		void CalcTextureChksum() { ZL_STATIC_ASSERT(!(sizeof(TextureReferences)&3),BAD_ALIGN); TextureChksum = ZL_Checksum::Fast4(TextureReferences, sizeof(TextureReferences)); }
	} UniformSet;
	struct ZL_MaterialProgram* ShaderProgram;
	unsigned int MaterialModes;

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

	void SetUniformFloat(ZL_NameID Name, scalar val)
	{
		UniformValue* v = std::lower_bound(UniformSet.Values, UniformSet.Values+UniformSet.Num, Name);
		if (v == UniformSet.Values+UniformSet.Num || v->Name != Name) return;
		assert(v->Type == UniformValue::TYPE_FLOAT);ZL_STATIC_ASSERT(sizeof(val)==sizeof(v->Value.Float),NEEDS_TO_MATCH);
		v->Value.Float = val;
		UniformSet.CalcValueChksum();
	}
	void SetUniformVec2(ZL_NameID Name, const ZL_Vector& val)
	{
		UniformValue* v = std::lower_bound(UniformSet.Values, UniformSet.Values+UniformSet.Num, Name);
		if (v == UniformSet.Values+UniformSet.Num || v->Name != Name) return;
		assert(v->Type == UniformValue::TYPE_VEC2);ZL_STATIC_ASSERT(sizeof(val)==sizeof(v->Value.Vec2),NEEDS_TO_MATCH);
		memcpy(v->Value.Vec2, &val, sizeof(val));
		UniformSet.CalcValueChksum();
	}
	void SetUniformVec3(ZL_NameID Name, const ZL_Vector3& val)
	{
		UniformValue* v = std::lower_bound(UniformSet.Values, UniformSet.Values+UniformSet.Num, Name);
		if (v == UniformSet.Values+UniformSet.Num || v->Name != Name) return;
		assert(v->Type == UniformValue::TYPE_VEC3);ZL_STATIC_ASSERT(sizeof(val)==sizeof(v->Value.Vec3),NEEDS_TO_MATCH);
		memcpy(v->Value.Vec3, &val, sizeof(val));
		UniformSet.CalcValueChksum();
	}
	void SetUniformVec4(ZL_NameID Name, const ZL_Color& val)
	{
		UniformValue* v = std::lower_bound(UniformSet.Values, UniformSet.Values+UniformSet.Num, Name);
		if (v == UniformSet.Values+UniformSet.Num || v->Name != Name) return;
		assert(v->Type == UniformValue::TYPE_VEC4);ZL_STATIC_ASSERT(sizeof(val)==sizeof(v->Value.Vec4),NEEDS_TO_MATCH);
		memcpy(v->Value.Vec4, &val, sizeof(val));
		UniformSet.CalcValueChksum();
	}
	void SetTexture(int Num, ZL_Surface& Surface)
	{
		assert(Num >= 0 && Num < (int)COUNT_OF(UniformSet.TextureReferences));
		ZL_Surface_Impl *srfi = ZL_ImplFromOwner<ZL_Surface_Impl>(Surface);
		ZL_Texture_Impl *texi = (srfi ? srfi->tex : NULL);
		ZL_Impl::CopyRef(texi, (ZL_Impl*&)UniformSet.TextureReferences[Num]);
		UniformSet.CalcTextureChksum();
	}
	ZL_Color GetUniformVec4(ZL_NameID Name)
	{
		UniformValue* v = std::lower_bound(UniformSet.Values, UniformSet.Values+UniformSet.Num, Name);
		if (v == UniformSet.Values+UniformSet.Num || v->Name != Name) return ZL_Color::White;
		assert(v->Type == UniformValue::TYPE_VEC4);
		return ZL_Color(v->Value.Vec4[0], v->Value.Vec4[1], v->Value.Vec4[2], v->Value.Vec4[3]);
	}
};

struct ZL_MaterialInstance : ZL_Material_Impl
{
	ZL_MaterialInstance(ZL_Material_Impl *Base, unsigned int MaterialModes) : ZL_Material_Impl(Base->ShaderProgram, MaterialModes)
	{
		assert((Base->MaterialModes & ZL_Display3D_Shaders::MMDEF_WITHOUTOPTIONS) == (MaterialModes & ZL_Display3D_Shaders::MMDEF_WITHOUTOPTIONS));
		ShaderProgram = Base->ShaderProgram;
		reinterpret_cast<ZL_Material_Impl*>(ShaderProgram)->AddRef();
		UniformSet = Base->UniformSet;
		UniformSet.Values = (UniformValue*)malloc(sizeof(UniformValue) * UniformSet.Num);
		memcpy(UniformSet.Values, Base->UniformSet.Values, sizeof(UniformValue) * UniformSet.Num);
	}

	~ZL_MaterialInstance() { reinterpret_cast<ZL_Material_Impl*>(ShaderProgram)->DelRef(); }
};

struct ZL_MaterialProgram : ZL_Material_Impl
{
	ZL_ShaderIDs ShaderIDs;
	GLint UniformMatrixView, UniformVectorViewPos, UniformMatrixLight, UniformVectorLightData, UniformTime;
	GLuint UploadedCamera, UploadedLight, UploadedLightDataChkSum, UniformUploadedValueChksum;
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

	static std::vector<struct ZL_MaterialProgram*>::iterator FindVariation(unsigned int MaterialModes)
	{
		struct SortFunc { static inline bool ByMaterialMode(ZL_MaterialProgram* a, unsigned int MM) { return (a->MaterialModes & ZL_Display3D_Shaders::MMDEF_WITHOUTOPTIONS) < MM; } };
		return std::lower_bound(g_LoadedShaderVariations->begin(), g_LoadedShaderVariations->end(), (MaterialModes & ZL_Display3D_Shaders::MMDEF_WITHOUTOPTIONS), SortFunc::ByMaterialMode);
	}

	~ZL_MaterialProgram()
	{
		if (ShaderIDs.Program) glDeleteProgram(ShaderIDs.Program);
		if (MaterialModes && !(MaterialModes & (ZL_Display3D_Shaders::MMDEF_USECUSTOMSHADER)))
			g_LoadedShaderVariations->erase(FindVariation(MaterialModes));
		if (ShadowMapProgram) ShadowMapProgram->DelRef();
		if (this == g_ShadowMapProgram) g_ShadowMapProgram = NULL;
		if (this == g_ShadowMapMaskProgram) g_ShadowMapMaskProgram = NULL;

		#ifdef ZL_VIDEO_WEAKCONTEXT
		if (ShaderIDs.Program) g_LoadedMaterialPrograms->erase(std::find(g_LoadedMaterialPrograms->begin(), g_LoadedMaterialPrograms->end(), this));
		#endif
	}

	ZL_MaterialProgram(GLsizei vertex_shader_srcs_count, const char **vertex_shader_srcs, GLsizei fragment_shader_srcs_count, const char **fragment_shader_srcs, unsigned int MaterialModes = 0) : ZL_Material_Impl(NULL, MaterialModes), UploadedCamera(0), UploadedLight(0), UploadedLightDataChkSum(0), UniformUploadedValueChksum(0), ShadowMapProgram(NULL)
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
		if (UniformSamplerDiffuse  != -1) glUniform1i(UniformSamplerDiffuse , 0);
		if (UniformSamplerNormal   != -1) glUniform1i(UniformSamplerNormal  , 1);
		if (UniformSamplerSpecular != -1) glUniform1i(UniformSamplerSpecular, 2);
		if (UniformSamplerParallax != -1) glUniform1i(UniformSamplerParallax, 3);
		if (UniformSamplerShadow   != -1) glUniform1i(UniformSamplerShadow  , 4);

		std::vector<UniformValue> Values;
		for (int i = 0; i < AllUniformCount; i++)
		{
			GLsizei UniformNameLen; GLint UniformSize; GLenum UniformType; char UniformName[256];
			glGetActiveUniform(ShaderIDs.Program, i, 256, &UniformNameLen, &UniformSize, &UniformType, UniformName);
			GLint loc = glGetUniformLocation(ShaderIDs.Program, UniformName);
			if (loc == UniformMatrixView || loc == ShaderIDs.UniformMatrixModel || loc == ShaderIDs.UniformMatrixNormal || loc == UniformMatrixLight ||
			    loc == UniformVectorViewPos || loc == UniformVectorLightData || loc == UniformTime ||
			    loc == UniformSamplerDiffuse || loc == UniformSamplerNormal || loc == UniformSamplerSpecular || loc == UniformSamplerParallax || loc == UniformSamplerShadow) continue;
			UniformValue v;
			memset(&v.Value, 0, sizeof(v.Value));
			if      (loc == UniformFloatSpecular )     { v.Type = UniformValue::TYPE_FLOAT; glUniform1(loc, (v.Value.Float = s(5))); }
			else if (loc == UniformFloatShininess)     { v.Type = UniformValue::TYPE_FLOAT; glUniform1(loc, (v.Value.Float = s(16))); }
			else if (loc == UniformFloatParallaxScale) { v.Type = UniformValue::TYPE_FLOAT; glUniform1(loc, (v.Value.Float = s(0.05))); }
			else if (UniformType == GL_FLOAT)          { v.Type = UniformValue::TYPE_FLOAT; glUniform1(loc, (v.Value.Float = 1)); }
			else if (UniformType == GL_FLOAT_VEC2 )    { v.Type = UniformValue::TYPE_VEC2;  glUniform2(loc, (v.Value.Vec2[0] = 1), (v.Value.Vec2[1] = 1)); }
			else if (UniformType == GL_FLOAT_VEC3 )    { v.Type = UniformValue::TYPE_VEC3;  glUniform3(loc, (v.Value.Vec3[0] = 1), (v.Value.Vec3[1] = 1), (v.Value.Vec3[2] = 1)); }
			else if (UniformType == GL_FLOAT_VEC4 )    { v.Type = UniformValue::TYPE_VEC4;  glUniform4(loc, (v.Value.Vec4[0] = 1), (v.Value.Vec4[1] = 1), (v.Value.Vec4[2] = 1), (v.Value.Vec4[3] = 1)); }
			else continue; //unhandled type
			v.Name = UniformName;
			v.Location = loc;
			std::vector<UniformValue>::iterator it = std::lower_bound(Values.begin(), Values.end(), v.Name);
			if (it == Values.end() || it->Name != v.Name) Values.insert(it, v);
			ZL_LOG3("3D", "[%d] Uniform Name: %s - Type: %d", v.Location, UniformName, v.Type);
		}
		if (!Values.empty())
		{
			UniformSet.Num = (GLuint)Values.size();
			UniformSet.Values = (UniformValue*)malloc(sizeof(UniformValue) * UniformSet.Num);
			memcpy(UniformSet.Values, &Values[0], sizeof(UniformValue) * UniformSet.Num);
			UniformSet.CalcValueChksum();
			UniformUploadedValueChksum = UniformSet.ValueChksum;
		}

		glUseProgram(ActiveProgram);
		assert(glGetAttribLocation(ShaderIDs.Program, ZL_Display3D_Shaders::S_AttributeList[0]) == 0);
		ShaderIDs.UsedAttributeMask = 0;
		for (GLint loc = 1; loc < (GLint)COUNT_OF(ZL_Display3D_Shaders::S_AttributeList); loc++)
			if (glGetAttribLocation(ShaderIDs.Program, ZL_Display3D_Shaders::S_AttributeList[loc]) != -1) { ShaderIDs.UsedAttributeMask |= 1<<loc; assert(glGetAttribLocation(ShaderIDs.Program, ZL_Display3D_Shaders::S_AttributeList[loc]) == loc); }
		ZL_LOG4("3D", "Compiled Shader - Program ID: %d - VS Parts: %d - FS Parts: %d - AttrMask: %d", ShaderIDs.Program, vertex_shader_srcs_count, fragment_shader_srcs_count, ShaderIDs.UsedAttributeMask);

		#ifdef ZL_VIDEO_WEAKCONTEXT
		for (GLsizei ivs = 0; ivs < vertex_shader_srcs_count; ivs++) WeakVertexShaderSrc += vertex_shader_srcs[ivs];
		for (GLsizei ifs = 0; ifs < fragment_shader_srcs_count; ifs++) WeakFragmentShaderSrc += fragment_shader_srcs[ifs];
		if (!g_LoadedMaterialPrograms) g_LoadedMaterialPrograms = new std::vector<struct ZL_MaterialProgram*>();
		g_LoadedMaterialPrograms->push_back(this);
		#endif
	}

	static ZL_Material_Impl* GetReference(unsigned int MM, const char* CustomFragmentCode = NULL, const char* CustomVertexCode = NULL)
	{
		using namespace ZL_Display3D_Shaders;
		assert(!CustomFragmentCode || (MM & MMDEF_USECUSTOMFRAGMENT));
		assert(!CustomVertexCode   || (MM & MMDEF_USECUSTOMVERTEX));
		assert(MM & MMDEF_FRAGCOLORMODES); //need at least one material mode defining the output fragment color
		assert((MM & MM_LIT) || !(MM & MMDEF_USESLIT));
		assert(!(MM & MM_SHADOWMAP) || Varying_ShadowMap); //check if InitShadowMapping has been called

		//normalize MaterialModes
		const unsigned int MMRequests = (MM & MMDEF_VSGLOBALREQUESTS);
		if (!(MM & MMDEF_FRAGCOLORMODES)) MM |= MM_STATICCOLOR;
		if (!(MM & MM_LIT)) MM &= ~MMDEF_USESLIT;
		if ((MM & MM_SHADOWMAP)     && !Varying_ShadowMap                           ) MM ^= MM_SHADOWMAP;
		if ((MM & MR_CAMERATANGENT) && (MM & (MMUSE_CAMERATANGENT^MR_CAMERATANGENT))) MM ^= MR_CAMERATANGENT;
		if ((MM & MR_TEXCOORD)      && (MM & (MMUSE_TEXCOORD     ^MR_TEXCOORD     ))) MM ^= MR_TEXCOORD;
		if ((MM & MR_NORMALS)       && (MM & (MMUSE_NORMAL       ^MR_NORMALS      ))) MM ^= MR_NORMALS;
		if ((MM & MR_POSITION)      && (MM & (MMUSE_POSITION     ^MR_POSITION     ))) MM ^= MR_POSITION;

		const bool bStoreVariation = (!(MM & MMDEF_USECUSTOMSHADER));
		std::vector<ZL_MaterialProgram*>::iterator it;
		ZL_Material_Impl* res;
		if (!g_LoadedShaderVariations) g_LoadedShaderVariations = new std::vector<struct ZL_MaterialProgram*>();
		if (bStoreVariation && (it = FindVariation(MM)) != g_LoadedShaderVariations->end() && ((*it)->MaterialModes&MMDEF_WITHOUTOPTIONS) == (MM&MMDEF_WITHOUTOPTIONS))
		{
			res = new ZL_MaterialInstance(*it, MM);
		}
		else
		{
			char FS_FragColorCalc[FRAGCOLOR_CALC_MAXLEN+1], *FS_FragColorCalcPtr = FS_FragColorCalc;
			for (size_t i = 0; i < COUNT_OF(FragColorRules); i++)
				if (MM & FragColorRules[i].MMUseIf)
					FS_FragColorCalcPtr += sprintf(FS_FragColorCalcPtr, "%s%s", (FS_FragColorCalcPtr != FS_FragColorCalc ? "*" : ""), FragColorRules[i].Source);
			FS_FragColorCalcPtr[0] = ';'; FS_FragColorCalcPtr[1] = '\0';

			const char* FSNullReplacements[] = { CustomFragmentCode, FS_FragColorCalc };
			const char *VS[COUNT_OF(SharedRules)+COUNT_OF(VSGlobalRules)+COUNT_OF(VSRules)], *FS[COUNT_OF(SharedRules)+COUNT_OF(FSRules)];
			GLsizei VSCount  = BuildList(SharedRules,   COUNT_OF(SharedRules),   MM, &VS[      0]);
			        VSCount += BuildList(VSGlobalRules, COUNT_OF(VSGlobalRules), MM, &VS[VSCount]);
			        VSCount += BuildList(VSRules,       COUNT_OF(VSRules),       MM, &VS[VSCount], &CustomVertexCode);
			GLsizei FSCount  = BuildList(SharedRules,   COUNT_OF(SharedRules),   MM, &FS[      0]);
			        FSCount += BuildList(FSRules,       COUNT_OF(FSRules),       MM, &FS[FSCount], FSNullReplacements);

			res = new ZL_MaterialProgram(VSCount, VS, FSCount, FS, MM);
			if (!res->ShaderProgram->ShaderIDs.Program) { res->ShaderProgram->MaterialModes = 0; delete res; return NULL; }

			if (bStoreVariation) g_LoadedShaderVariations->insert(it, res->ShaderProgram);
		}

		if (Varying_ShadowMap && !(MM & MO_NOSHADOW) && !res->ShaderProgram->ShadowMapProgram)
		{
			ZL_MaterialProgram*& UseShadowMapProgram = (MM & MM_POSITIONFUNC ? res->ShaderProgram->ShadowMapProgram : (!(MM&(MM_MASKED|MM_DIFFUSEMAP)) ? g_ShadowMapProgram : g_ShadowMapMaskProgram));
			if (UseShadowMapProgram) UseShadowMapProgram->AddRef();
			else
			{
				unsigned int MMSMRules =  (MM & MM_POSITIONFUNC ? MM_POSITIONFUNC : (!(MM&(MM_MASKED|MM_DIFFUSEMAP)) ? 0 : MM_MASKED|MM_DIFFUSEMAP));
				unsigned int MMSMGlobal = (MMSMRules == MM_POSITIONFUNC ? MMRequests : MMSMRules);
				const char *VS[COUNT_OF(SharedRules)+COUNT_OF(VSGlobalRules)+COUNT_OF(VSRules)], *FS[COUNT_OF(SharedRules)+COUNT_OF(FSRules)];
				GLsizei VSCount  = BuildList(SharedRules,      COUNT_OF(SharedRules),      MMSMRules,  &VS[      0]);
				        VSCount += BuildList(VSGlobalRules,    COUNT_OF(VSGlobalRules),    MMSMGlobal, &VS[VSCount]);
				        VSCount += BuildList(VSRules,          COUNT_OF(VSRules),          MMSMRules,  &VS[VSCount], &CustomVertexCode);
				GLsizei FSCount  = BuildList(SharedRules,      COUNT_OF(SharedRules),      MMSMRules,  &FS[      0]);
				        FSCount += BuildList(ShadowMapFSRules, COUNT_OF(ShadowMapFSRules), MMSMRules,  &FS[FSCount]);
				UseShadowMapProgram = new ZL_MaterialProgram(VSCount, VS, FSCount, FS);
				if (!UseShadowMapProgram->ShaderIDs.Program) { delete UseShadowMapProgram; UseShadowMapProgram = NULL; }
			}
			res->ShaderProgram->ShadowMapProgram = UseShadowMapProgram;
		}

		return res;
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
			assert(Override.Num >= UniformSet.Num);
			for (UniformValue* v = Override.Values, *vEnd = v + UniformSet.Num; v != vEnd; ++v)
			{
				switch (v->Type)
				{
					case UniformValue::TYPE_FLOAT: glUniform1(v->Location, v->Value.Float); break;
					case UniformValue::TYPE_VEC2:  glUniform2(v->Location, v->Value.Vec2[0], v->Value.Vec2[1]); break;
					case UniformValue::TYPE_VEC3:  glUniform3(v->Location, v->Value.Vec3[0], v->Value.Vec3[1], v->Value.Vec3[2]); break;
					case UniformValue::TYPE_VEC4:  glUniform4(v->Location, v->Value.Vec4[0], v->Value.Vec4[1], v->Value.Vec4[2], v->Value.Vec4[3]); break;
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
};

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
		MeshPart(ZL_NameID Name, GLsizei IndexCount, GLushort* IndexOffsetPtr,  ZL_Material_Impl* Material) : Name(Name), IndexCount(IndexCount), IndexOffsetPtr(IndexOffsetPtr), Material(Material) { if (Material) Material->AddRef(); }
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
		res->Parts[0] = MeshPart(ZL_NameID(), IndicesCount, NULL, Program);
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
		ZL_Camera_Impl CamModelImpl = *ZL_ImplFromOwner<ZL_Camera_Impl>(cam);
		CamModelImpl.VP = Matrix * CamModelImpl.VP;
		CamModelImpl.UpdateCount ^= (GLuint)-1;
		const ZL_Camera_Impl* CamModelImplPtr = &CamModelImpl;
		const ZL_Camera& CamModel = *(const ZL_Camera*)&CamModelImplPtr;

		GLint BufferSize;
		glBindBuffer(GL_ARRAY_BUFFER, VertexBufferObject);
		glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &BufferSize);
		GLvoid* StoredVertData = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		assert((BufferSize%Stride) == 0);assert(StoredVertData);
		glBindBuffer(GL_ARRAY_BUFFER, g_Active3D.VertexBuffer);

		for (GLubyte *v = (GLubyte*)StoredVertData, *vEnd = v + BufferSize; v != vEnd; v += Stride)
		{
			ZL_Vector3 *pos = (ZL_Vector3*)v; GLscalar *ScalarOffset = (GLscalar*)(pos+1);
			if (ProvideAttributeMask & VAMASK_NORMAL)   { ZL_Display3D::DrawLine(CamModel, *pos, *pos + *((ZL_Vector3*)ScalarOffset) * 0.1f, ZL_Color::White, 0.001f); ScalarOffset += 3; }
			if (ProvideAttributeMask & VAMASK_TEXCOORD) { ZL_Display3D::DrawPlane(CamModel, *pos, ZL_Vector3(0,0,1), ZL_Vector(0.05f,0.05f), ZL_Color(ScalarOffset[0], ScalarOffset[1], 0)); ScalarOffset += 2; }
			if (ProvideAttributeMask & VAMASK_TANGENT)  { ZL_Display3D::DrawLine(CamModel, *pos, *pos + *((ZL_Vector3*)ScalarOffset) * 0.1f, ZL_Color::Red, 0.001f); ScalarOffset += 3; }
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
		res->Parts[0] = MeshPart(ZL_NameID(), (GLsizei)Indices.size(), NULL, Material);
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
		MeshPart NextPart(ZL_NameID(), 0, NULL, NULL);
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
				else { mat = new ZL_MaterialInstance(it->Material, it->Material->MaterialModes); it->Material->DelRef(); it->Material = mat; }
				continue;
			}
			if (!mat) continue;

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
			if      (param_count == 1 && line[0] == 'N' && line[1] == 's') mat->SetUniformFloat(Z3U_SPECULAR, param[0]*0.01f);
			else if (param_count == 3 && line[0] == 'K' && line[1] == 'd') mat->SetUniformVec4(Z3U_COLOR, ZL_Color(param[0], param[1], param[2], mat->GetUniformVec4(Z3U_COLOR).a));
			else if (param_count == 1 && line[0] == 'd' && line[1] == ' ') { ZL_Color c = mat->GetUniformVec4(Z3U_COLOR); c.a = param[0]; mat->SetUniformVec4(Z3U_COLOR, c); }
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

struct ZL_RenderList_Impl : public ZL_Impl
{
	struct MeshEntry { ZL_Mesh_Impl* Mesh; ZL_Matrix ModelMatrix, NormalMatrix; MeshEntry(ZL_Mesh_Impl* Mesh, const ZL_Matrix& ModelMatrix, const ZL_Matrix& NormalMatrix) : Mesh(Mesh), ModelMatrix(ModelMatrix), NormalMatrix(NormalMatrix) {} };
	struct PartEntry { ZL_Mesh_Impl::MeshPart* Part; GLushort MeshIndex, UniformSetIndex; };
	std::vector<ZL_Mesh_Impl*> ReferencedMeshes;
	std::vector<MeshEntry> Meshes;
	std::vector<PartEntry> SortedParts, CustomVertexShaderParts;
	std::vector<ZL_Material_Impl::UniformValue> UniformValues;
	std::vector<ZL_Material_Impl::sUniformSet> UniformSets;

	static inline bool SortMeshPartByMaterial(const PartEntry& a, const PartEntry& b)
	{
		const ZL_Material_Impl *am = a.Part->Material, *bm = b.Part->Material;
		if ((am->MaterialModes & ZL_MaterialModes::MO_TRANSPARENCY) != (bm->MaterialModes & ZL_MaterialModes::MO_TRANSPARENCY)) return !(am->MaterialModes & ZL_MaterialModes::MO_TRANSPARENCY);
		const ZL_MaterialProgram *ap = am->ShaderProgram, *bp = bm->ShaderProgram;
		return (ap->ShadowMapProgram == bp->ShadowMapProgram ? ap < bp : ap->ShadowMapProgram < bp->ShadowMapProgram);
	}

	void Reset()
	{
		for (std::vector<ZL_Mesh_Impl*>::iterator it = ReferencedMeshes.begin(); it != ReferencedMeshes.end(); ++it) (*it)->DelRef();
		ReferencedMeshes.clear();
		Meshes.clear();
		SortedParts.clear();
		UniformValues.clear();
		UniformSets.clear();
	}

	void Add(ZL_Mesh_Impl* mesh, const ZL_Matrix& matrix)
	{
		GLushort MeshIndex = (GLushort)Meshes.size();
		MeshEntry me(mesh, matrix, matrix.GetInverseTransposed());
		Meshes.push_back(me);
		for (ZL_Mesh_Impl::MeshPart *p = mesh->Parts; p != mesh->PartsEnd; p++)
		{
			PartEntry e = { p, MeshIndex };
			std::vector<PartEntry>::iterator it = std::upper_bound(SortedParts.begin(), SortedParts.end(), e, SortMeshPartByMaterial);
			if (it != SortedParts.begin())
			{
				PartEntry *PrevEntry = &*(it-1);
				if (PrevEntry->Part->Material->ShaderProgram == p->Material->ShaderProgram)
				{
					ZL_Material_Impl::sUniformSet& PrevUniformSet = UniformSets[PrevEntry->UniformSetIndex];
					if (PrevUniformSet.ValueChksum == p->Material->UniformSet.ValueChksum && PrevUniformSet.TextureChksum == p->Material->UniformSet.TextureChksum)
					{
						e.UniformSetIndex = PrevEntry->UniformSetIndex;
						goto InsertStoredPart;
					}
				}
			}
			e.UniformSetIndex = (GLushort)UniformSets.size();
			UniformSets.push_back(p->Material->UniformSet);
			if (p->Material->UniformSet.Num)
			{
				char* OldPtr = (char*)(UniformValues.empty() ? NULL : &UniformValues[0]);
				UniformValues.resize(UniformValues.size() + p->Material->UniformSet.Num);
				ptrdiff_t PtrDiff = (char*)&UniformValues[0] - OldPtr;
				if (PtrDiff && OldPtr)
					for (std::vector<ZL_Material_Impl::sUniformSet>::iterator its = UniformSets.begin(); its != UniformSets.end(); ++its)
						*(char**)&its->Values += PtrDiff;
				ZL_Material_Impl::sUniformSet& UniformSet = UniformSets.back();
				UniformSet.Values = &UniformValues[UniformValues.size() - p->Material->UniformSet.Num];
				memcpy(UniformSet.Values, p->Material->UniformSet.Values, sizeof(ZL_Material_Impl::UniformValue) * p->Material->UniformSet.Num);
			}
			InsertStoredPart:
			SortedParts.insert(it, e);
		}
	}

	void AddReferenced(ZL_Mesh_Impl* mesh, const ZL_Matrix& matrix)
	{
		mesh->AddRef();
		ReferencedMeshes.push_back(mesh);
		Add(mesh, matrix);
	}
};

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Material)
ZL_Material::ZL_Material(unsigned int MaterialModes) : impl(ZL_MaterialProgram::GetReference(MaterialModes)) { }
ZL_Material::ZL_Material(unsigned int MaterialModes, const char* CustomFragmentCode, const char* CustomVertexCode) : impl(ZL_MaterialProgram::GetReference(MaterialModes, CustomFragmentCode, CustomVertexCode)) { }
ZL_Material ZL_Material::MakeNewMaterialInstance() { return (impl ? ZL_ImplMakeOwner<ZL_Material>(new ZL_MaterialInstance(impl, impl->MaterialModes), false) : ZL_Material()); }
ZL_Material& ZL_Material::SetUniformFloat(ZL_NameID Name, scalar val) { if (impl) impl->SetUniformFloat(Name, val); return *this; }
ZL_Material& ZL_Material::SetUniformVec2(ZL_NameID Name, const ZL_Vector& val) { if (impl) impl->SetUniformVec2(Name, val); return *this; }
ZL_Material& ZL_Material::SetUniformVec3(ZL_NameID Name, const ZL_Vector3& val) { if (impl) impl->SetUniformVec3(Name, val); return *this; }
ZL_Material& ZL_Material::SetUniformVec3(ZL_NameID Name, const ZL_Color& val) { if (impl) impl->SetUniformVec3(Name, (const ZL_Vector3&)val); return *this; }
ZL_Material& ZL_Material::SetUniformVec4(ZL_NameID Name, const ZL_Color& val) { if (impl) impl->SetUniformVec4(Name, val); return *this; }
ZL_Material& ZL_Material::SetDiffuseTexture(ZL_Surface& srf) { if (impl) impl->SetTexture(0, srf); return *this; }
ZL_Material& ZL_Material::SetNormalTexture(ZL_Surface& srf) { if (impl) impl->SetTexture(1, srf); return *this; }
ZL_Material& ZL_Material::SetSpecularTexture(ZL_Surface& srf) { if (impl) impl->SetTexture(2, srf); return *this; }
ZL_Material& ZL_Material::SetParallaxTexture(ZL_Surface& srf) { if (impl) impl->SetTexture(3, srf); return *this; }

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

ZL_Mesh ZL_Mesh::BuildPlane(const ZL_Vector& Extents, const ZL_Material& Material, const ZL_Vector& UVMapMax)
{
	GLscalar Verts[] = {
		 Extents.x,  Extents.y, 0, 0,0,1, UVMapMax.x, UVMapMax.y, 1,0,0,
		-Extents.x,  Extents.y, 0, 0,0,1,          0, UVMapMax.y, 1,0,0,
		-Extents.x, -Extents.y, 0, 0,0,1,          0,          0, 1,0,0,
		 Extents.x, -Extents.y, 0, 0,0,1, UVMapMax.x,          0, 1,0,0
	};

	GLushort Indices[] = { 0,1,2,0,2,3 };

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
	return ZL_ImplMakeOwner<ZL_Mesh>(ZL_Mesh_Impl::Make((ZL_Mesh_Impl::VAMASK_NORMAL|ZL_Mesh_Impl::VAMASK_TEXCOORD|ZL_Mesh_Impl::VAMASK_TANGENT), Indices, 6, Verts, 4, ZL_ImplFromOwner<ZL_Material_Impl>(Material)), false);
}

ZL_Mesh ZL_Mesh::BuildBox(const ZL_Vector3& ex, const ZL_Material& Material)
{
	const GLscalar Verts[11*6*4] = {
		 ex.x, ex.y, ex.z,  0, 0, 1, 1,1 ,  1, 0, 0, -ex.x, ex.y, ex.z,  0, 0, 1, 0,1,  1, 0, 0 , -ex.x,-ex.y, ex.z,  0, 0, 1, 0,0,  1, 0, 0 ,  ex.x,-ex.y, ex.z,  0, 0, 1, 1,0,  1, 0, 0 , 
		 ex.x, ex.y,-ex.z,  0, 0,-1, 1,1 , -1, 0, 0,  ex.x,-ex.y,-ex.z,  0, 0,-1, 1,0, -1, 0, 0 , -ex.x,-ex.y,-ex.z,  0, 0,-1, 0,0, -1, 0, 0 , -ex.x, ex.y,-ex.z,  0, 0,-1, 0,1, -1, 0, 0 ,
		 ex.x, ex.y, ex.z,  1, 0, 0, 1,1 ,  0, 1, 0,  ex.x,-ex.y, ex.z,  1, 0, 0, 0,1,  0, 1, 0 ,  ex.x,-ex.y,-ex.z,  1, 0, 0, 0,0,  0, 1, 0 ,  ex.x, ex.y,-ex.z,  1, 0, 0, 1,0,  0, 1, 0 , 
		-ex.x, ex.y, ex.z, -1, 0, 0, 1,1 ,  0,-1, 0, -ex.x, ex.y,-ex.z, -1, 0, 0, 1,0,  0,-1, 0 , -ex.x,-ex.y,-ex.z, -1, 0, 0, 0,0,  0,-1, 0 , -ex.x,-ex.y, ex.z, -1, 0, 0, 0,1,  0,-1, 0 ,
		 ex.x, ex.y, ex.z,  0, 1, 0, 1,1 ,  0, 0, 1,  ex.x, ex.y,-ex.z,  0, 1, 0, 0,1,  0, 0, 1 , -ex.x, ex.y,-ex.z,  0, 1, 0, 0,0,  0, 0, 1 , -ex.x, ex.y, ex.z,  0, 1, 0, 1,0,  0, 0, 1 , 
		 ex.x,-ex.y, ex.z,  0,-1, 0, 1,1 ,  0, 0,-1, -ex.x,-ex.y, ex.z,  0,-1, 0, 1,0,  0, 0,-1 , -ex.x,-ex.y,-ex.z,  0,-1, 0, 0,0,  0, 0,-1 ,  ex.x,-ex.y,-ex.z,  0,-1, 0, 0,1,  0, 0,-1 ,
	};
	const GLushort Indices[] = { 0,1,2,0,2,3 , 4,5,6,4,6,7 , 8,9,10,8,10,11 , 12,13,14,12,14,15 , 16,17,18,16,18,19 , 20,21,22,20,22,23 };
	return ZL_ImplMakeOwner<ZL_Mesh>(ZL_Mesh_Impl::Make((ZL_Mesh_Impl::VAMASK_NORMAL|ZL_Mesh_Impl::VAMASK_TEXCOORD|ZL_Mesh_Impl::VAMASK_TANGENT), Indices, COUNT_OF(Indices), Verts, COUNT_OF(Verts), ZL_ImplFromOwner<ZL_Material_Impl>(Material)), false);
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
			for (GLushort IndexOffset = (Verts.size() - COUNT_OF(CubeVerts)) / 12, *pIndicesEnd = pIndices + COUNT_OF(CubeIndices); pIndices != pIndicesEnd; pIndices++)
				*pIndices += IndexOffset;
		}
	}
	free(Pixels);
	return ZL_ImplMakeOwner<ZL_Mesh>(ZL_Mesh_Impl::Make((ZL_Mesh_Impl::VAMASK_NORMAL|ZL_Mesh_Impl::VAMASK_TEXCOORD|ZL_Mesh_Impl::VAMASK_TANGENT|ZL_Mesh_Impl::VAMASK_COLOR), &Indices[0], (GLsizei)Indices.size(), &Verts[0], (GLsizei)(Verts.size()/12), ZL_ImplFromOwner<ZL_Material_Impl>(Material)), false);
}

ZL_IMPL_OWNER_INHERITED_IMPLEMENTATIONS(ZL_MeshAnimated)
ZL_MeshAnimated::ZL_MeshAnimated(const ZL_FileLink& ModelFile, const ZL_Material& Material) { impl = ZL_MeshAnimated_Impl::OBJLoadAnimation(ModelFile, ZL_ImplFromOwner<ZL_Material_Impl>(Material)); }
ZL_MeshAnimated& ZL_MeshAnimated::SetFrame(unsigned int FrameIndex) { if (impl) static_cast<ZL_MeshAnimated_Impl*>(impl)->SetFrame(FrameIndex); return *this; }

ZL_IMPL_OWNER_NONULLCON_IMPLEMENTATIONS(ZL_Camera)
ZL_Camera::ZL_Camera() : impl(new ZL_Camera_Impl) { }
ZL_Camera& ZL_Camera::SetPosition(const ZL_Vector3& pos)      { impl->Pos = pos; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetDirection(const ZL_Vector3& dir)     { impl->Dir = dir; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetFOV(scalar fov)                      { impl->FOV = fov; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetAspectRatio(scalar ar)               { impl->AspectRatio = ar; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetClipPlane(scalar znear, scalar zfar) { impl->ZNear = znear; impl->ZFar = zfar; impl->UpdateMatrix(); return *this; }
ZL_Camera& ZL_Camera::SetAmbientLightColor(const ZL_Color& color) { impl->AmbientLightColor = ZL_Vector3(color.r*color.a, color.g*color.a, color.b*color.a); return *this; }
ZL_Matrix& ZL_Camera::GetVPMatrix() { return impl->VP; }
const ZL_Matrix& ZL_Camera::GetVPMatrix() const { return impl->VP; }
ZL_Vector3 ZL_Camera::GetPosition() const { return impl->Pos; }
ZL_Vector3 ZL_Camera::GetDirection() const { return impl->Dir; }
scalar ZL_Camera::GetFOV() const { return impl->FOV; }
scalar ZL_Camera::GetNearClipPlane() const { return impl->ZNear; }
scalar ZL_Camera::GetFarClipPlane() const { return impl->ZFar; }
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
ZL_Light& ZL_Light::SetDirectionalLight(scalar area) { impl->Size = area; impl->Aspect = 0; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetSpotLight(scalar angle, scalar aspect) { impl->Size = angle; impl->Aspect = aspect; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetRange(scalar range_far, scalar range_near) { impl->Far = range_far; impl->Near = range_near; impl->UpdateMatrix(); return *this; }
ZL_Light& ZL_Light::SetFalloff(scalar distance) { impl->Falloff = distance; return *this; }
ZL_Light& ZL_Light::SetOutsideLit(bool OutsideLit) { impl->LightFactorOutside = (OutsideLit ? s(1) : s(0)); return *this; }
ZL_Light& ZL_Light::SetColor(const ZL_Color& color) { impl->Color = ZL_Vector3(color.r*color.a, color.g*color.a, color.b*color.a); return *this; }
ZL_Matrix& ZL_Light::GetVPMatrix() { return impl->VP; }
const ZL_Matrix& ZL_Light::GetVPMatrix() const { return impl->VP; }
ZL_Vector3 ZL_Light::GetPosition() const { return impl->Pos; }
ZL_Vector3 ZL_Light::GetDirection() const { return impl->Dir; }
ZL_Color ZL_Light::GetColor() const { return ZL_Color(impl->Color.x, impl->Color.y, impl->Color.z); }

ZL_IMPL_OWNER_NONULLCON_IMPLEMENTATIONS(ZL_RenderList)
ZL_RenderList::ZL_RenderList() : impl(new ZL_RenderList_Impl) { }
void ZL_RenderList::Reset() { impl->Reset(); }
void ZL_RenderList::Add(const ZL_Mesh& Mesh, const ZL_Matrix& Matrix) { impl->Add(ZL_ImplFromOwner<ZL_Mesh_Impl>(Mesh), Matrix); }
void ZL_RenderList::AddReferenced(const ZL_Mesh& Mesh, const ZL_Matrix& Matrix) { impl->AddReferenced(ZL_ImplFromOwner<ZL_Mesh_Impl>(Mesh), Matrix); }

#if defined(ZILLALOG) && !defined(ZL_VIDEO_OPENGL_ES2)
void ZL_RenderList::DebugDump()
{
	ZL_LOG0("[RENDERLIST]", "----------------------------------------------------------------");
	ZL_LOG1("[RENDERLIST]", "ReferencedMeshes Count: %d", impl->ReferencedMeshes.size());
	ZL_LOG1("[RENDERLIST]", "Meshes Count: %d", impl->Meshes.size());
	ZL_LOG1("[RENDERLIST]", "SortedParts Count: %d", impl->SortedParts.size());
	ZL_LOG1("[RENDERLIST]", "UniformValues Count: %d", impl->UniformValues.size());
	ZL_LOG1("[RENDERLIST]", "UniformSets Count: %d", impl->UniformSets.size());
	for (ZL_RenderList_Impl::PartEntry *e = (impl->SortedParts.empty() ? NULL : &impl->SortedParts[0]), *eBegin = e, *eEnd = e + impl->SortedParts.size(); e != eEnd; e++)
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
	GLsizei VSCount  = BuildList(SharedRules,   COUNT_OF(SharedRules),   MM_STATICCOLOR, &VS[      0]);
		    VSCount += BuildList(VSGlobalRules, COUNT_OF(VSGlobalRules), MM_STATICCOLOR, &VS[VSCount]);
		    VSCount += BuildList(VSRules,       COUNT_OF(VSRules),       MM_STATICCOLOR, &VS[VSCount]);
	GLsizei FSCount  = BuildList(SharedRules,   COUNT_OF(SharedRules),   MM_STATICCOLOR, &FS[      0]);
		    FSCount += BuildList(FSRules,       COUNT_OF(FSRules),       MM_STATICCOLOR, &FS[FSCount], FSNullReplacements);
	g_DebugColorMat = new ZL_MaterialProgram(VSCount, VS, FSCount, FS);
}

static void ZL_Display3D_DrawDebugMesh(ZL_Mesh_Impl* DbgMesh, const ZL_Matrix& Matrix, const ZL_Camera& Camera, const ZL_Color& color)
{
	g_DebugColorMat->SetUniformVec4(g_DebugColorMat->UniformSet.Values->Name, color);
	g_DebugColorMat->Activate(g_DebugColorMat->UniformSet, ZL_RenderSceneSetup(ZL_ImplFromOwner<ZL_Camera_Impl>(Camera)));
	DbgMesh->DrawPart(DbgMesh->Parts, Matrix, Matrix);
}

template<bool IsLighting> static void ZL_Display3D_RenderShaded(ZL_RenderList_Impl* List, const ZL_RenderSceneSetup& Scene)
{
	if (List->SortedParts.empty()) return;
	GLushort ActiveUniformSetIndex = (GLushort)-1;
	for (ZL_RenderList_Impl::PartEntry *e = &List->SortedParts[0], *eEnd = e + List->SortedParts.size(); e != eEnd; e++)
	{
		ZL_RenderList_Impl::MeshEntry& me = List->Meshes[e->MeshIndex];
		if (IsLighting && !e->Part->Material->ShaderProgram->ShadowMapProgram) continue;
		if (ActiveUniformSetIndex != e->UniformSetIndex)
		{
			ActiveUniformSetIndex = e->UniformSetIndex;
			(IsLighting ? e->Part->Material->ShaderProgram->ShadowMapProgram : e->Part->Material->ShaderProgram)->Activate(List->UniformSets[ActiveUniformSetIndex], Scene);
		}
		me.Mesh->DrawPart(e->Part, me.ModelMatrix, me.NormalMatrix);
	}
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
	Varying_ShadowMap = "varying vec3 " Z3SM_LIGHTSPACE ";";
	VS_ShadowMap_Calc = Z3SM_LIGHTSPACE " = vec3(" Z3U_LIGHT " * vec4(" Z3V_POSITION ",1));";
	#else
	Varying_ShadowMap = "varying vec4 " Z3SM_LIGHTUNPROJECTED ";";
	VS_ShadowMap_Calc = Z3SM_LIGHTUNPROJECTED " = " Z3U_LIGHT " * vec4(" Z3V_POSITION ",1);";
	#endif
	VS_ShadowMap_Defs = "uniform mat4 " Z3U_LIGHT ";";
	FS_ShadowMap_Defs = "uniform sampler2D " Z3U_SHADOWMAP ";";
	FS_ShadowMap_Calc =
			"const float " Z3SM_TXL " = 1./" SHADOWMAP_SIZE_STRING ".;"
			"if (i == 0)" 
			"{" 
				#ifndef ZL_DISPLAY3D_ORTHOLIGHTSONLY
				"vec3 " Z3SM_LIGHTSPACE " = " Z3SM_LIGHTUNPROJECTED ".xyz / " Z3SM_LIGHTUNPROJECTED ".w;"
				#endif
				"if (" Z3SM_LIGHTSPACE ".z < 1. && " Z3SM_LIGHTSPACE ".x >= 0. && " Z3SM_LIGHTSPACE ".x <= 1. && " Z3SM_LIGHTSPACE ".y >= 0. && " Z3SM_LIGHTSPACE ".y <= 1.)"
				"{"
					"float " Z3SM_BIASDEPTH " = " Z3SM_LIGHTSPACE ".z - .005;" //- (.002 + .004 * " Z3L_DIFFUSE ");" (Z3L_DIFFUSE is Z3L_LIGHTFACTOR before specular)
					"const float " Z3SM_TXL " = 1./" SHADOWMAP_SIZE_STRING ".;"
					Z3L_LIGHTFACTOR " *= .4+.12 * (step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy).r)"
					                         "+step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy + vec2(-" Z3SM_TXL ",-" Z3SM_TXL ")).r)"
					                         "+step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy + vec2( " Z3SM_TXL ",-" Z3SM_TXL ")).r)"
					                         "+step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy + vec2(-" Z3SM_TXL ", " Z3SM_TXL ")).r)"
					                         "+step(" Z3SM_BIASDEPTH ", texture2D(" Z3U_SHADOWMAP ", " Z3SM_LIGHTSPACE ".xy + vec2( " Z3SM_TXL ", " Z3SM_TXL ")).r));"
				"}"
				"else " Z3L_LIGHTFACTOR " *= " Z3U_LIGHTDATA "[3].y;"
			"}";

	glGenTextures(1, &g_ShadowMap_TEX);
	glBindTexture(GL_TEXTURE_2D, g_ShadowMap_TEX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_SIZE, SHADOWMAP_SIZE, 0, GL_DEPTH_COMPONENT, /*GL_FLOAT*/ GL_UNSIGNED_INT, NULL);
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

	#if defined(ZILLALOG) && (0||ZL_DISPLAY3D_TEST_VARIOUS_SHADER_TEMPLATES)
	using namespace ZL_MaterialModes;
	ZL_Material(MM_STATICCOLOR);
	ZL_Material(MM_STATICCOLOR|MM_VERTEXCOLOR|MM_DIFFUSEMAP);
	ZL_Material(MM_STATICCOLOR|MM_LIT);
	ZL_Material(MM_DIFFUSEMAP|MM_LIT|MM_SHADOWMAP|MM_SPECULARSTATIC|MM_NORMALMAP);
	ZL_Material(MM_STATICCOLOR|MM_PARALLAXMAP);
	ZL_Material(MM_STATICCOLOR|MR_POSITION);
	ZL_Material(MM_STATICCOLOR|MR_TEXCOORD);
	ZL_Material(MM_STATICCOLOR|MR_NORMALS);
	ZL_Material(MM_STATICCOLOR|MR_CAMERATANGENT);
	ZL_Material(MM_STATICCOLOR|MR_PRECISIONTANGENT);
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

	#if !defined(ZL_VIDEO_OPENGL_ES2) && defined(ZILLALOG) && (0||ZL_DISPLAY3D_TEST_WIREFRAMEMODE)
	if (ZL_Display::KeyDown[ZLK_LCTRL])
	{
		glDisable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (ZL_Display::KeyDown[ZLK_LALT]) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	#endif
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
	glVertexAttrib4(ZLGLSL::ATTR_COLOR, .5, .5, .5, 1);
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
	bool TemporaryRendering = (ZLGLSL::ActiveProgram != ZLGLSL::DISPLAY3D);
	if (TemporaryRendering) BeginRendering();
	ZL_RenderSceneSetup Scene(ZL_ImplFromOwner<ZL_Camera_Impl>(Camera));
	for (size_t i = 0; i != NumLists; i++)
		ZL_Display3D_RenderShaded<false>(ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[i]), Scene);
	if (TemporaryRendering) FinishRendering();
}

void ZL_Display3D::DrawListsWithLights(const ZL_RenderList*const* RenderLists, size_t NumLists, const ZL_Camera& Camera, const ZL_Light*const* Lights, size_t NumLights)
{
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
			assert(g_ShadowMap_FBO); //check if InitShadowMapping has been called
			glBindFramebuffer(GL_FRAMEBUFFER, g_ShadowMap_FBO);
			glViewport(0, 0, SHADOWMAP_SIZE, SHADOWMAP_SIZE);
			glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
			glClear(GL_DEPTH_BUFFER_BIT);

			Scene.Camera = ZL_ImplFromOwner<ZL_Light_Impl>(*Lights[0]);
			for (size_t i = 0; i != NumLists; i++)
				ZL_Display3D_RenderShaded<true>(ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[i]), Scene);

			glBindFramebuffer(GL_FRAMEBUFFER, active_framebuffer);
			glViewport(active_viewport[0], active_viewport[1], active_viewport[2], active_viewport[3]);
			glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		}

		Scene.Light = ZL_ImplFromOwner<ZL_Light_Impl>(*Lights[0]);
		for (size_t i = 0, iEnd = MIN(NumLights, ZL_RenderSceneSetup::MAX_LIGHTS); i != iEnd; i++)
		{
			ZL_Light_Impl* Light = ZL_ImplFromOwner<ZL_Light_Impl>(*Lights[i]);
			Scene.LightData[Scene.LightDataCount++] = Light->Pos;
			Scene.LightData[Scene.LightDataCount++] = Light->Color;
			Scene.LightData[Scene.LightDataCount++] = ZL_Vector3(Light->Falloff, Light->LightFactorOutside);
		}
	}
	Scene.LightData[Scene.LightDataCount++].x = 3.4e+38f;
	Scene.CalcLightDataChkSum();

	Scene.Camera = ZL_ImplFromOwner<ZL_Camera_Impl>(Camera);

	#if defined(ZILLALOG) && defined(ZL_DISPLAY3D_ALLOW_SHIFT_DEBUG_VIEW)
	if (ZL_Display::KeyDown[ZLK_LSHIFT])
	{
		static ZL_Camera DebugCam;
		static float DebugDist = 0.0f;
		static float DebugLastMouseY = ZL_Display::PointerY;
		if (ZL_Display::KeyDown[ZLK_LCTRL]) DebugDist += 0.1f*(DebugLastMouseY - ZL_Display::PointerY);
		if (ZL_Display::KeyDown[ZLK_LALT]) { glDisable(GL_CULL_FACE); glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); }
		DebugLastMouseY = ZL_Display::PointerY;
		DebugCam.SetDirection(-ZL_Vector3::FromRotation((ZL_Display::PointerX-ZLHALFW)/ZLHALFW*PI, -(ZL_Display::PointerY-ZLHALFH)/ZLHALFH*PIHALF*0.99f));
		DebugCam.SetPosition(Camera.GetPosition() + Camera.GetDirection() * 5.0f + DebugCam.GetDirection() * -(5.0f+DebugDist));
		Scene.Camera = ZL_ImplFromOwner<ZL_Camera_Impl>(DebugCam);

		for (size_t j = 0; j != NumLists; j++)
			ZL_Display3D_RenderShaded<false>(ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[j]), Scene);

		#if 1
		if (ZL_Display::KeyDown[ZLK_LCTRL]) glClear(GL_DEPTH_BUFFER_BIT);
		ZL_Display3D::DrawLine(DebugCam, ZL_Vector3(0,0,0), ZL_Vector3(100,0,0), ZL_Color::Red,   .02f);
		ZL_Display3D::DrawLine(DebugCam, ZL_Vector3(0,0,0), ZL_Vector3(0,100,0), ZL_Color::Green, .02f);
		ZL_Display3D::DrawLine(DebugCam, ZL_Vector3(0,0,0), ZL_Vector3(0,0,100), ZL_Color::Blue,  .02f);
		ZL_Display3D::DrawFrustum(DebugCam, Camera.GetVPMatrix(), ZL_Color::Magenta);
		if (NumLights && g_ShadowMap_FBO) ZL_Display3D::DrawFrustum(DebugCam, Lights[0]->GetVPMatrix(), ZL_Color::Yellow);
		for (size_t k = 0; k != NumLights; k++) ZL_Display3D::DrawLine(DebugCam, Lights[k]->GetPosition(), Lights[k]->GetPosition() + Lights[k]->GetDirection(), ZL_Color::Yellow, 0.02f);
		#endif
		if (ZL_Display::KeyDown[ZLK_LALT]) { glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); glEnable(GL_CULL_FACE); }
	}
	else
	#endif
	for (size_t j = 0; j != NumLists; j++)
		ZL_Display3D_RenderShaded<false>(ZL_ImplFromOwner<ZL_RenderList_Impl>(*RenderLists[j]), Scene);
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
