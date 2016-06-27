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
#if defined(ZL_VIDEO_USE_GLSL)
#include <ZL_Application.h>
#include "ZL_Texture_Impl.h"
#include "ZL_Math.h"
#include <assert.h>

namespace ZLGLSL
{
	std::vector<GLSLscalar> mvp_matrix_store(16);
	GLSLscalar *mvp_matrix_start = &mvp_matrix_store[0];
	GLSLscalar *mvp_matrix_last = mvp_matrix_start;
	GLSLscalar *mvp_matrix_ = mvp_matrix_start;
	eActiveProgram ActiveProgram = NONE;

	void MatrixApply();

	void MatrixPush()
	{
		if (mvp_matrix_ == mvp_matrix_last)
		{
			//ZL_LOG1("ZLGLSL", "Resize matrix store to %d", 1+diff/16);
			size_t sz = (mvp_matrix_last-mvp_matrix_start);
			mvp_matrix_store.resize(sz+32);
			mvp_matrix_start = &mvp_matrix_store[0];
			mvp_matrix_ = mvp_matrix_last = mvp_matrix_start+sz+16;
		}
		else mvp_matrix_ += 16;
		memcpy(mvp_matrix_, mvp_matrix_-16, 16*sizeof(GLSLscalar));
	}

	void MatrixPop()
	{
		if (mvp_matrix_ == mvp_matrix_start) return;
		mvp_matrix_ -= 16;
		MatrixApply();
	}

	void MatrixIdentity()
	{
		memset(mvp_matrix_, 0, sizeof(GLSLscalar)*16);
		mvp_matrix_[0] = mvp_matrix_[5] = mvp_matrix_[10] = mvp_matrix_[15] = 1.0f;
		MatrixApply();
	}

	void MatrixTranslate(GLSLscalar tx, GLSLscalar ty)
	{
		mvp_matrix_[12] += (mvp_matrix_[0] * tx) + (mvp_matrix_[4] * ty);
		mvp_matrix_[13] += (mvp_matrix_[1] * tx) + (mvp_matrix_[5] * ty);
		//mvp_matrix_[14] += (mvp_matrix_[2] * tx) + (mvp_matrix_[6] * ty);
		//mvp_matrix_[15] += (mvp_matrix_[3] * tx) + (mvp_matrix_[7] * ty);
		MatrixApply();
	}

	void MatrixScale(GLSLscalar sx, GLSLscalar sy)
	{
		mvp_matrix_[0] *= sx;
		mvp_matrix_[1] *= sx;
		//mvp_matrix_[2] *= sx;
		//mvp_matrix_[3] *= sx;
		mvp_matrix_[4] *= sy;
		mvp_matrix_[5] *= sy;
		//mvp_matrix_[6] *= sy;
		//mvp_matrix_[7] *= sy;
		MatrixApply();
	}

	void MatrixRotate(GLSLscalar angle_rad)
	{
		GLSLscalar sinAngle = ssin(angle_rad), cosAngle = scos(angle_rad);
		for (int i = 0; i < /*4*/ 2; i++)
		{
			GLSLscalar tmp = mvp_matrix_[i];
			mvp_matrix_[  i] = (mvp_matrix_[4+i] * sinAngle) + (tmp * cosAngle);
			mvp_matrix_[4+i] = (mvp_matrix_[4+i] * cosAngle) - (tmp * sinAngle);
			mvp_matrix_[8+i] *= cosAngle;
		}
		MatrixApply();
	}

	void MatrixRotate(GLSLscalar rcos, GLSLscalar rsin)
	{
		for (int i = 0; i < /*4*/ 2; i++)
		{
			GLSLscalar tmp = mvp_matrix_[i];
			mvp_matrix_[  i] = (mvp_matrix_[4+i] * rsin) + (tmp * rcos);
			mvp_matrix_[4+i] = (mvp_matrix_[4+i] * rcos) - (tmp * rsin);
			mvp_matrix_[8+i] *= rcos;
		}
		MatrixApply();
	}

	void MatrixTransform(GLSLscalar tx, GLSLscalar ty, GLSLscalar rcos, GLSLscalar rsin)
	{
		mvp_matrix_[12] += (mvp_matrix_[0] * tx) + (mvp_matrix_[4] * ty);
		mvp_matrix_[13] += (mvp_matrix_[1] * tx) + (mvp_matrix_[5] * ty);
		//mvp_matrix_[14] += (mvp_matrix_[2] * tx) + (mvp_matrix_[6] * ty);
		//mvp_matrix_[15] += (mvp_matrix_[3] * tx) + (mvp_matrix_[7] * ty);
		for (int i = 0; i < /*4*/ 2; i++)
		{
			GLSLscalar tmp = mvp_matrix_[i];
			mvp_matrix_[  i] = (mvp_matrix_[4+i] * rsin) + (tmp * rcos);
			mvp_matrix_[4+i] = (mvp_matrix_[4+i] * rcos) - (tmp * rsin);
			mvp_matrix_[8+i] *= rcos;
		}
		MatrixApply();
	}

	void MatrixTransformReverse(GLSLscalar tx, GLSLscalar ty, GLSLscalar rcos, GLSLscalar rsin)
	{
		for (int i = 0; i < /*4*/ 2; i++)
		{
			GLSLscalar tmp = mvp_matrix_[i];
			mvp_matrix_[  i] = (tmp * rcos) - (mvp_matrix_[4+i] * rsin);
			mvp_matrix_[4+i] = (tmp * rsin) + (mvp_matrix_[4+i] * rcos);
			mvp_matrix_[8+i] *= rcos;
		}
		mvp_matrix_[12] -= (mvp_matrix_[0] * tx) + (mvp_matrix_[4] * ty);
		mvp_matrix_[13] -= (mvp_matrix_[1] * tx) + (mvp_matrix_[5] * ty);
		//mvp_matrix_[14] -= (mvp_matrix_[2] * tx) + (mvp_matrix_[6] * ty);
		//mvp_matrix_[15] -= (mvp_matrix_[3] * tx) + (mvp_matrix_[7] * ty);
		MatrixApply();
	}

	void MatrixOrtho(GLSLscalar left, GLSLscalar right, GLSLscalar bottom, GLSLscalar top)
	{
		scalar deltaX = right - left, deltaY = top - bottom;
		if ((deltaX == 0.0f) || (deltaY == 0.0f)) return;
		memset(mvp_matrix_, 0, sizeof(GLSLscalar)*16);
		mvp_matrix_[0] = 2.0f / deltaX;
		mvp_matrix_[12] = -(right + left) / deltaX;
		mvp_matrix_[5] = 2.0f / deltaY;
		mvp_matrix_[13] = -(top + bottom) / deltaY;
		mvp_matrix_[10] = -1.0f;
		mvp_matrix_[15] = 1.0f;
		MatrixApply();
	}

	void Project(GLSLscalar& x, GLSLscalar& y)
	{
		GLSLscalar oldx = x;
		x = (   x * mvp_matrix_[0/*M11*/]) + (y * mvp_matrix_[4/*M21*/]) + mvp_matrix_[12/*M41*/];
		y = (oldx * mvp_matrix_[1/*M12*/]) + (y * mvp_matrix_[5/*M22*/]) + mvp_matrix_[13/*M42*/];
	}

	void Unproject(GLSLscalar& x, GLSLscalar& y)
	{
		GLSLscalar det8 = -mvp_matrix_[10] * mvp_matrix_[12];
		GLSLscalar det10 = -mvp_matrix_[10] * mvp_matrix_[13];
		GLSLscalar invdetmatrix = 1.0f / ((mvp_matrix_[ 0] * mvp_matrix_[ 5] - mvp_matrix_[ 1] * mvp_matrix_[ 4])*mvp_matrix_[10]);
		GLSLscalar invm11 = ( mvp_matrix_[ 5]*mvp_matrix_[10]) * invdetmatrix;
		GLSLscalar invm12 = (-mvp_matrix_[ 1]*mvp_matrix_[10]) * invdetmatrix;
		GLSLscalar invm21 = (-mvp_matrix_[ 4]*mvp_matrix_[10]) * invdetmatrix;
		GLSLscalar invm22 = ( mvp_matrix_[ 0]*mvp_matrix_[10]) * invdetmatrix;
		GLSLscalar invm41 = (-mvp_matrix_[ 4]*det10 + mvp_matrix_[ 5]*det8) * invdetmatrix;
		GLSLscalar invm42 = ( mvp_matrix_[ 0]*det10 - mvp_matrix_[ 1]*det8) * invdetmatrix;
		GLSLscalar oldx = x;
		x = (   x * invm11) + (y * invm21) + invm41;
		y = (oldx * invm12) + (y * invm22) + invm42;
	}
}

#ifndef ZL_VIDEO_DIRECT3D
#include <ZL_Display.h>
#include "ZL_Impl.h"

static const char* baseattribs_vertex_shader_src = //"#ifdef GL_ES\nprecision lowp float;\n#endif\n" //lowp vec4
	"uniform mat4 u_mvpMatrix;"
	"attribute vec4 a_position;"
	"attribute vec4 a_color;"
	"varying vec4 v_color;";

static const char* baseattribs_fragment_shader_src =
	"varying vec4 v_color;";

static const char* color_vertex_shader_src =
	"void main()"
	"{"
		"v_color = a_color;"
		"gl_PointSize = 1.0;"
		"gl_Position = u_mvpMatrix * a_position;"
	"}";

static const char* color_fragment_shader_src =
	"void main()"
	"{"
		"gl_FragColor = v_color;"
	"}";

static const char* texture_vertex_shader_src =
	"attribute vec2 a_texcoord;"
	"varying vec2 v_texcoord;"
	"void main()"
	"{"
		"v_color = a_color;"
		"if (v_color.a > 1.) { v_color.a = 1.; }"
		"v_texcoord = a_texcoord;"
		"gl_Position = u_mvpMatrix * a_position;"
	"}";

static const char* texture_fragment_shader_src =
	"uniform sampler2D u_texture;"
	"varying vec2 v_texcoord;"
	"void main()"
	"{"
		"gl_FragColor = v_color * texture2D(u_texture, v_texcoord);"
	"}";

static const char *srcs_texture_vertex[] = { ZLGLSL_LIST_LOW_PRECISION_HEADER baseattribs_vertex_shader_src, texture_vertex_shader_src };
static const char *srcs_texture_fragment[] = { ZLGLSL_LIST_LOW_PRECISION_HEADER baseattribs_fragment_shader_src, texture_fragment_shader_src };

namespace ZLGLSL
{
	GLuint UNI_MVP = 0;

	GLuint _COLOR_PROGRAM = 0;
	GLuint COLOR_UNI_MVP;

	GLuint _TEXTURE_PROGRAM = 0;
	GLuint TEXTURE_UNI_MVP;

	GLuint CreateShaderOfType(GLenum type, GLsizei count, const char **shader_src)
	{
		GLuint shader;
		GLint compiled;
		shader = glCreateShader(type); //Create the shader object
		if (shader == 0) return 0;

		#if defined(ZILLALOG)
		//Format the source code for debugging purposes
		ZL_String src, indent;
		for (GLsizei i = 0, justnl = 1, bracket_level = 0; i < count; i++, justnl = 1)
			for (const char *p = shader_src[i]; *p; p++)
				if (justnl && *p <= ' ') { } //skip whitespaces after newline
				else if (*p == '{') { src << '\n' << indent << '{' << '\n' << indent << '\t'; indent += '\t'; justnl = 1; }
				else if (*p == '}') { if (!indent.empty()) { indent.resize(indent.size()-1); if (justnl && src.size()) src.resize(src.size()-1); }src << '}' << '\n' << indent; justnl = 1; }
				else if (*p == '\n') { src << '\n' << indent; justnl = 1;  }
				else if (*p == ';' && !bracket_level) { src << ';' << '\n' << indent; justnl = 1; }
				else { if (*p == '(') bracket_level++; else if (*p == ')') bracket_level--; src << *p; justnl = 0; }
		const char* psrc = src.c_str();
		shader_src = &psrc;
		count = 1;
		#endif

		//Load and compile the shader source
		glShaderSource(shader, count, shader_src, NULL);
		glCompileShader(shader);

		//Check the compile status
		glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
		if (compiled == 0)
		{
			#if defined(ZILLALOG)
			GLint info_len = 0;
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
			if (info_len > 1)
			{
				char* info_log = reinterpret_cast<char*>(malloc(sizeof(char) * info_len));
				glGetShaderInfoLog(shader, info_len, NULL, info_log);
				ZL_LOG1("ZLGLSL", "Error compiling shader: %s", info_log);
				ZL_LOG0("ZLGLSL", "Shader Source:");
				int i = 1; for (const char *lb = src.c_str(), *le; (le = strchr(lb, '\n')); lb = le+1) { ZL_LOG3("ZLGLSL", "%3d: %.*s", i++, le-lb, lb); }
				free(info_log);
				assert(false);
			}
			#endif
			glDeleteShader(shader);
			return 0;
		}

		return shader;
	}

	GLuint CreateProgramFromVertexAndFragmentShaders(GLsizei vertex_shader_srcs_count, const char **vertex_shader_srcs, GLsizei fragment_shader_srcs_count, const char **fragment_shader_srcs, GLsizei bind_attribs_count, const char **bind_attribs)
	{
		GLuint vertex_shader;
		GLuint fragment_shader;
		GLuint program_object;
		GLint linked;

		// Load the vertex/fragment shaders
		if (!(vertex_shader = CreateShaderOfType(GL_VERTEX_SHADER, vertex_shader_srcs_count, vertex_shader_srcs))) return 0;
		if (!(fragment_shader = CreateShaderOfType(GL_FRAGMENT_SHADER, fragment_shader_srcs_count, fragment_shader_srcs))) { glDeleteShader(vertex_shader); return 0; }

		// Create the program object and attach the shaders.
		if (!(program_object = glCreateProgram())) { glDeleteShader(vertex_shader); glDeleteShader(fragment_shader); return 0; }
		glAttachShader(program_object, vertex_shader);
		glAttachShader(program_object, fragment_shader);

		//attribute bound to index 0 must always be an enabled array attribute - so force position to it
		for (GLsizei i = 0; i < bind_attribs_count; i++) glBindAttribLocation(program_object, i, bind_attribs[i]);

		// Link the program
		glLinkProgram(program_object);

		// Check the link status
		glGetProgramiv(program_object, GL_LINK_STATUS, &linked);
		if (linked == 0)
		{
			#if defined(ZILLALOG)
			GLint info_len = 0;
			glGetProgramiv(program_object, GL_INFO_LOG_LENGTH, &info_len);
			if (info_len > 1)
			{
				char* info_log = reinterpret_cast<char*>(malloc(info_len));
				glGetProgramInfoLog(program_object, info_len, NULL, info_log);
				ZL_LOG1("ZLGLSL", "Error linking program: %s", info_log);
				free(info_log);
				assert(false);
			}
			#endif
			glDeleteProgram(program_object);
			return 0;
		}

		#ifndef ZILLALOG
		// Detach and delete these here because they are linked into the program object (leave them in debug builds for debugging purposes)
		glDetachShader(program_object, vertex_shader); glDeleteShader(vertex_shader);
		glDetachShader(program_object, fragment_shader); glDeleteShader(fragment_shader);
		#endif

		return program_object;
	}

	bool CreateShaders()
	{
		// Load the shaders and get a linked program object
		const char *srcs_color_vertex[] = { ZLGLSL_LIST_LOW_PRECISION_HEADER baseattribs_vertex_shader_src, color_vertex_shader_src };
		const char *srcs_color_fragment[] = { ZLGLSL_LIST_LOW_PRECISION_HEADER baseattribs_fragment_shader_src, color_fragment_shader_src };
		const char *attr_color[] = { "a_position", "a_color" };
		if (!(_COLOR_PROGRAM = CreateProgramFromVertexAndFragmentShaders(COUNT_OF(srcs_color_vertex), srcs_color_vertex, COUNT_OF(srcs_color_fragment), srcs_color_fragment, COUNT_OF(attr_color), attr_color))) return false;
		COLOR_UNI_MVP       = (GLuint)glGetUniformLocation(_COLOR_PROGRAM, "u_mvpMatrix");

		// Load the shaders and get a linked program object
		const char *attr_texture[] = { "a_position", "a_color", "a_texcoord" };
		if (!(_TEXTURE_PROGRAM = CreateProgramFromVertexAndFragmentShaders(COUNT_OF(srcs_texture_vertex), srcs_texture_vertex, COUNT_OF(srcs_texture_fragment), srcs_texture_fragment, COUNT_OF(attr_texture), attr_texture))) return false;
		TEXTURE_UNI_MVP       = (GLuint)glGetUniformLocation(_TEXTURE_PROGRAM, "u_mvpMatrix");

		glEnableVertexAttribArrayUnbuffered(0); //0 = POSITION = always required

		return true;
	}

	void MatrixApply()
	{
		if (ActiveProgram != NONE) glUniformMatrix4v(ZLGLSL::UNI_MVP, 1, GL_FALSE, mvp_matrix_);
	}

	void _COLOR_PROGRAM_ACTIVATE()
	{
		ActiveProgram = COLOR;
		glDisableVertexAttribArrayUnbuffered(ATTR_TEXCOORD);
		glUseProgram(_COLOR_PROGRAM);
		UNI_MVP = COLOR_UNI_MVP;
		MatrixApply();
	}

	void _TEXTURE_PROGRAM_ACTIVATE()
	{
		ActiveProgram = TEXTURE;
		glUseProgram(_TEXTURE_PROGRAM);
		UNI_MVP = TEXTURE_UNI_MVP;
		glEnableVertexAttribArrayUnbuffered(ATTR_TEXCOORD); //always required
		MatrixApply();
	}
}

#ifdef ZL_VIDEO_WEAKCONTEXT
#include "ZL_String.h"
static std::vector<struct ZL_Shader_Impl*> *pLoadedShaders = NULL;
static std::vector<struct ZL_PostProcess_Impl*> *pLoadedPostProcesses = NULL;
#endif

struct ZL_Shader_Impl : ZL_Impl
{
	GLuint PROGRAM;
	GLint UNI_MVP, UNI_1, UNI_2;
	#ifdef ZL_VIDEO_WEAKCONTEXT
	ZL_String strCodeVertex, strCodeFragment;
	#endif

	ZL_Shader_Impl(const char* code_fragment, const char *code_vertex, const char* name_uniform_float_1, const char* name_uniform_float_2)
	{
		const char **srcs_vertex, **srcs_fragment; GLsizei srcs_vertex_count, srcs_fragment_count;
		if (code_vertex) srcs_vertex = &code_vertex, srcs_vertex_count = 1; else srcs_vertex = srcs_texture_vertex, srcs_vertex_count = COUNT_OF(srcs_texture_vertex);
		if (code_fragment) srcs_fragment = &code_fragment, srcs_fragment_count = 1; else srcs_fragment = srcs_texture_fragment, srcs_fragment_count = COUNT_OF(srcs_texture_fragment);
		const char *attr[] = { "a_position", "a_color", "a_texcoord" };
		if (!(PROGRAM = ZLGLSL::CreateProgramFromVertexAndFragmentShaders(srcs_vertex_count, srcs_vertex, srcs_fragment_count, srcs_fragment, COUNT_OF(attr), attr))) return;
		UNI_MVP       = (GLuint)glGetUniformLocation(PROGRAM, "u_mvpMatrix");
		if (name_uniform_float_1) UNI_1 = (GLuint)glGetUniformLocation(PROGRAM, name_uniform_float_1);
		if (name_uniform_float_2) UNI_2 = (GLuint)glGetUniformLocation(PROGRAM, name_uniform_float_2);
		#ifdef ZL_VIDEO_WEAKCONTEXT
		if (code_vertex) strCodeVertex = code_vertex;
		if (code_fragment) strCodeFragment = code_fragment;
		if (!pLoadedShaders) pLoadedShaders = new std::vector<ZL_Shader_Impl*>();
		pLoadedShaders->push_back(this);
		#endif
	}

	~ZL_Shader_Impl()
	{
		if (PROGRAM) glDeleteProgram(PROGRAM);
		#ifdef ZL_VIDEO_WEAKCONTEXT
		pLoadedShaders->erase(std::find(pLoadedShaders->begin(), pLoadedShaders->end(), this));
		#endif
	}
};

ZL_Shader::ZL_Shader(const char* code_fragment, const char *code_vertex, const char* name_uniform_float_1, const char* name_uniform_float_2) : impl(new ZL_Shader_Impl(code_fragment, code_vertex, name_uniform_float_1, name_uniform_float_2))
{ if (!impl->PROGRAM) { delete impl; impl = NULL; } }

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_Shader)

void ZL_Shader::Activate()
{
	if (!impl) return;
	ZLGLSL::ActiveProgram = ZLGLSL::CUSTOM;
	glUseProgram(impl->PROGRAM);
	ZLGLSL::UNI_MVP = impl->UNI_MVP;
	glEnableVertexAttribArrayUnbuffered(ZLGLSL::ATTR_POSITION);
	glEnableVertexAttribArrayUnbuffered(ZLGLSL::ATTR_TEXCOORD);
	ZLGLSL::MatrixApply();
}
void ZL_Shader::SetUniform(scalar uni1) { if (impl) glUniform1(impl->UNI_1, (GLscalar)uni1); }
void ZL_Shader::SetUniform(scalar uni1, scalar uni2) { if (impl) { glUniform1(impl->UNI_1, (GLscalar)uni1); glUniform1(impl->UNI_2, (GLscalar)uni2); } }

void ZL_Shader::Deactivate()
{
	if (!impl) return;
	ZLGLSL::ActiveProgram = ZLGLSL::NONE;
}

static const char *postprocess_vertex_shader_srcs[] = { ZLGLSL_LIST_LOW_PRECISION_HEADER "attribute vec4 a_position;attribute vec2 a_texcoord;varying vec2 v_texcoord; void main(){v_texcoord=a_texcoord;gl_Position=a_position;}" };

struct ZL_PostProcess_Impl : ZL_Impl
{
	GLuint PROGRAM;
	GLint UNI_1, UNI_2;
	ZL_Texture_Impl *tex;
	#ifdef ZL_VIDEO_WEAKCONTEXT
	std::string strCodeFragment;
	#endif

	ZL_PostProcess_Impl(const char* code_fragment, bool use_alpha, const char* name_uniform_float_1, const char* name_uniform_float_2) : tex(NULL)
	{
		const char *attr[] = { "a_position", "a_texcoord" };
		if (!(PROGRAM = ZLGLSL::CreateProgramFromVertexAndFragmentShaders(COUNT_OF(postprocess_vertex_shader_srcs), postprocess_vertex_shader_srcs, 1, &code_fragment, COUNT_OF(attr), attr))) return;
		if (name_uniform_float_1) UNI_1 = (GLuint)glGetUniformLocation(PROGRAM, name_uniform_float_1);
		if (name_uniform_float_2) UNI_2 = (GLuint)glGetUniformLocation(PROGRAM, name_uniform_float_2);
		tex = new ZL_Texture_Impl(window_viewport[2], window_viewport[3], use_alpha);
		if (!tex->gltexid) { tex->DelRef(); tex = NULL; return; }
		#ifdef ZL_VIDEO_WEAKCONTEXT
		strCodeFragment = code_fragment;
		if (!pLoadedPostProcesses) pLoadedPostProcesses = new std::vector<ZL_PostProcess_Impl*>();
		pLoadedPostProcesses->push_back(this);
		#endif
	}

	~ZL_PostProcess_Impl()
	{
		if (PROGRAM) glDeleteProgram(PROGRAM);
		if (tex) tex->DelRef();
		#ifdef ZL_VIDEO_WEAKCONTEXT
		for (std::vector<ZL_PostProcess_Impl*>::iterator it = pLoadedPostProcesses->begin(); it != pLoadedPostProcesses->end(); ++it)
			if (*it == this) { pLoadedPostProcesses->erase(it); break; }
		#endif
	}

	void Start(bool clear)
	{
		tex->FrameBufferBegin(clear);
	}

	void Apply(scalar *uni1, scalar *uni2)
	{
		tex->FrameBufferEnd();
		const GLscalar vec_fullbox[8] = { -1,1 , 1,1 , -1,-1 , 1,-1 };
		const GLscalar tex_fullbox[8] = {  0,1 , 1,1 ,  0, 0 , 1, 0 };
		glDisableVertexAttribArrayUnbuffered(2);
		glUseProgram(PROGRAM);
		glEnableVertexAttribArrayUnbuffered(1);
		glBindTexture(GL_TEXTURE_2D, tex->gltexid);
		glVertexAttribPointerUnbuffered(0, 2, GL_SCALAR, GL_FALSE, 0, vec_fullbox);
		glVertexAttribPointerUnbuffered(1, 2, GL_SCALAR, GL_FALSE, 0, tex_fullbox);
		if (uni1) glUniform1(UNI_1, (GLscalar)*uni1);
		if (uni2) glUniform1(UNI_2, (GLscalar)*uni2);
		//glDisable(GL_BLEND);
		glDrawArraysUnbuffered(GL_TRIANGLE_STRIP, 0, 4);
		//glEnable(GL_BLEND);
		glDisableVertexAttribArrayUnbuffered(1);
		ZLGLSL::ActiveProgram = ZLGLSL::NONE;
	}
};

ZL_PostProcess::ZL_PostProcess(const char* code_fragment, bool use_alpha, const char* name_uniform_float_1, const char* name_uniform_float_2) : impl(new ZL_PostProcess_Impl(code_fragment, use_alpha, name_uniform_float_1, name_uniform_float_2))
{ if (!impl->PROGRAM || !impl->tex) { delete impl; impl = NULL; } }

ZL_IMPL_OWNER_DEFAULT_IMPLEMENTATIONS(ZL_PostProcess)

void ZL_PostProcess::Start(bool clear) { if (impl) impl->Start(clear); }
void ZL_PostProcess::Apply() { if (impl) impl->Apply(NULL, NULL); }
void ZL_PostProcess::Apply(scalar uni1) { if (impl) impl->Apply(&uni1, NULL); }
void ZL_PostProcess::Apply(scalar uni1, scalar uni2) { if (impl) impl->Apply(&uni1, &uni2); }

#ifdef ZL_VIDEO_WEAKCONTEXT
bool CheckProgramsIfContextLost()
{
	//return true; //DEBUG
	return (ZLGLSL::_COLOR_PROGRAM != ZLGLSL::_TEXTURE_PROGRAM && !glIsProgram(ZLGLSL::_TEXTURE_PROGRAM));
}
void RecreateAllProgramsOnContextLost()
{
	//glDeleteProgram(ZLGLSL::_COLOR_PROGRAM); glDeleteProgram(ZLGLSL::_TEXTURE_PROGRAM); if (pLoadedShaders) for (std::vector<ZL_Shader_Impl*>::iterator itx = pLoadedShaders->begin(); itx != pLoadedShaders->end(); ++itx) glDeleteProgram((*itx)->PROGRAM); if (pLoadedPostProcesses) for (std::vector<ZL_PostProcess_Impl*>::iterator ity = pLoadedPostProcesses->begin(); ity != pLoadedPostProcesses->end(); ++ity) glDeleteProgram((*ity)->PROGRAM); //DEBUG
	ZL_LOG1("TEXTURE", "RecreateAllProgramsIfContextLost with %d programs loaded", 2+(pLoadedShaders ? pLoadedShaders->size() : 0)+(pLoadedPostProcesses ? pLoadedPostProcesses->size() : 0));
	ZLGLSL::CreateShaders();
	if (pLoadedShaders)
		for (std::vector<ZL_Shader_Impl*>::iterator its = pLoadedShaders->begin(); its != pLoadedShaders->end(); ++its)
		{
			const char *code_vertex = ((*its)->strCodeVertex.length() ? (*its)->strCodeVertex.c_str() : NULL), *code_fragment = ((*its)->strCodeFragment.length() ? (*its)->strCodeFragment.c_str() : NULL), **srcs_vertex, **srcs_fragment; GLsizei srcs_vertex_count, srcs_fragment_count;
			if (code_vertex  ) srcs_vertex   = &code_vertex,   srcs_vertex_count   = 1; else srcs_vertex   = srcs_texture_vertex,   srcs_vertex_count   = COUNT_OF(srcs_texture_vertex);
			if (code_fragment) srcs_fragment = &code_fragment, srcs_fragment_count = 1; else srcs_fragment = srcs_texture_fragment, srcs_fragment_count = COUNT_OF(srcs_texture_fragment);
			const char *attr[] = { "a_position", "a_color", "a_texcoord" };
			(*its)->PROGRAM = ZLGLSL::CreateProgramFromVertexAndFragmentShaders(srcs_vertex_count, srcs_vertex, srcs_fragment_count, srcs_fragment, COUNT_OF(attr), attr);
		}
	if (pLoadedPostProcesses)
		for (std::vector<ZL_PostProcess_Impl*>::iterator itp = pLoadedPostProcesses->begin(); itp != pLoadedPostProcesses->end(); ++itp)
			{ const char *attr[] = { "a_position", "a_texcoord" }; const char* pf = (*itp)->strCodeFragment.c_str(); (*itp)->PROGRAM = ZLGLSL::CreateProgramFromVertexAndFragmentShaders(COUNT_OF(postprocess_vertex_shader_srcs), postprocess_vertex_shader_srcs, 1, &pf, COUNT_OF(attr), attr); }
	ZLGLSL::ActiveProgram = ZLGLSL::NONE; //reset matrix on next draw
}
#endif

#endif //ZL_VIDEO_DIRECT3D

#endif //ZL_VIDEO_USE_GLSL
