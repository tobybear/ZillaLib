/*
  ZillaLib
  Copyright (C) 2010-2020 Bernhard Schelling

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

(function(){'use strict';

var ABORT = false;
var HEAP8, HEAPU8, HEAP16, HEAPU16, HEAP32, HEAPU32, HEAPF32;
var WASM_MEMORY, WASM_HEAP, WASM_HEAP_MAX = 256*1024*1024; //256mb;
ZL.print = ZL.print || function (msg) { console.log(msg); };
ZL.error = ZL.error || function (code, msg) { ZL.print('[ERROR] ' + code + ': ' + msg); };
ZL.started = ZL.started || function () { };

function abort(code, msg)
{
	ABORT = true;
	ZL.error(code, msg);
	throw 'abort';
}

function updateGlobalBufferViews()
{
	var buf = WASM_MEMORY.buffer;
	HEAP8 = new Int8Array(buf);
	HEAP16 = new Int16Array(buf);
	HEAP32 = new Int32Array(buf);
	HEAPU8 = new Uint8Array(buf);
	HEAPU16 = new Uint16Array(buf);
	HEAPU32 = new Uint32Array(buf);
	HEAPF32 = new Float32Array(buf);
}

function _sbrk(increment)
{
	var heapOld = WASM_HEAP, heapNew = heapOld + increment, heapGrow = heapNew - WASM_MEMORY.buffer.byteLength;
	//console.log('[SBRK] Increment: ' + increment + ' - HEAP: ' + heapOld + ' -> ' + heapNew + (heapGrow > 0 ? ' - GROW BY ' + heapGrow + ' (' + (heapGrow>>16) + ' pages)' : ''));
	if (heapNew > WASM_HEAP_MAX) abort('MEM', 'Out of memory');
	if (heapGrow > 0) { WASM_MEMORY.grow((heapGrow+65535)>>16); updateGlobalBufferViews(); }
	WASM_HEAP = heapNew;
	return heapOld|0;
}

function malloc_string(s)
{
	var i, s = unescape(encodeURIComponent(s));
	var ptr = ZL.asm.malloc(s.length+1);
	for (i = 0; i < s.length; ++i) HEAP8[ptr+i] = (s.charCodeAt(i) & 0xFF);
	HEAP8[ptr+i] = 0;
	return ptr;
}

function malloc_array(a)
{
	var ptr = ZL.asm.malloc(Math.max(a.length, 1));
	if (a.subarray || a.slice) HEAPU8.set(a, ptr);
	else HEAPU8.set(new Uint8Array(a), ptr);
	return ptr;
}

function stringToUTF8Array(str, outU8Array, outIdx, maxBytesToWrite)
{
	if(!(0<maxBytesToWrite))return 0;
	for(var e=str,r=outU8Array,f=outIdx,i=maxBytesToWrite,a=f,t=f+i-1,b=0;b<e.length;++b)
	{
		var k=e.charCodeAt(b);
		if(55296<=k&&k<=57343&&(k=65536+((1023&k)<<10)|1023&e.charCodeAt(++b)),k<=127){if(t<=f)break;r[f++]=k;}
		else if(k<=2047){if(t<=f+1)break;r[f++]=192|k>>6,r[f++]=128|63&k;}
		else if(k<=65535){if(t<=f+2)break;r[f++]=224|k>>12,r[f++]=128|k>>6&63,r[f++]=128|63&k;}
		else if(k<=2097151){if(t<=f+3)break;r[f++]=240|k>>18,r[f++]=128|k>>12&63,r[f++]=128|k>>6&63,r[f++]=128|63&k;}
		else if(k<=67108863){if(t<=f+4)break;r[f++]=248|k>>24,r[f++]=128|k>>18&63,r[f++]=128|k>>12&63,r[f++]=128|k>>6&63,r[f++]=128|63&k;}
		else{if(t<=f+5)break;r[f++]=252|k>>30,r[f++]=128|k>>24&63,r[f++]=128|k>>18&63,r[f++]=128|k>>12&63,r[f++]=128|k>>6&63,r[f++]=128|63&k;}
	}
	return r[f]=0,f-a;
}

function Pointer_stringify(ptr, length)
{
	if (length === 0 || !ptr) return '';
	for (var hasUtf = 0, t, i = 0;;)
	{
		t = HEAPU8[((ptr)+(i))>>0];
		hasUtf |= t;
		if (t == 0 && !length) break;
		i++;
		if (length && i == length) break;
	}
	if (!length) length = i;

	if (hasUtf & 128)
	{
		for(var r=HEAPU8,o=ptr,p=ptr+length,F=String.fromCharCode,e,f,i,n,C,t,a,g='';;)
		{
			if(o==p||(e=r[o++],!e)) return g;
			128&e?(f=63&r[o++],192!=(224&e)?(i=63&r[o++],224==(240&e)?e=(15&e)<<12|f<<6|i:(n=63&r[o++],240==(248&e)?e=(7&e)<<18|f<<12|i<<6|n:(C=63&r[o++],248==(252&e)?e=(3&e)<<24|f<<18|i<<12|n<<6|C:(t=63&r[o++],e=(1&e)<<30|f<<24|i<<18|n<<12|C<<6|t))),65536>e?g+=F(e):(a=e-65536,g+=F(55296|a>>10,56320|1023&a))):g+=F((31&e)<<6|f)):g+=F(e);
		}
	}

	// split up into chunks, because .apply on a huge string can overflow the stack
	for (var ret = '', curr; length > 0; ptr += 1024, length -= 1024)
		ret += String.fromCharCode.apply(String, HEAPU8.subarray(ptr, ptr + Math.min(length, 1024)));
	return ret;
}

function Base64Decode(B)
{
	var T=new Uint8Array(128),i,C=function(o){return T[B.charCodeAt(i+o)];};
	for (i=0;i<64;i++) T['ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'.charCodeAt(i)]=i;T[45]=62;T[95]=63;
	var L=B.length,PH=(B[L-2]==='='?2:(B[L-1]==='='?1:0)),a=new Uint8Array(L*3/4-PH),n=0,j=(PH>0?L-4:L),t=0;
	for (i=0;i<j;i+=4) { t = (C(0)<<18)|(C(1)<<12)|(C(2)<<6)|C(3); a[n++]=(t>>16)&255;a[n++]=(t>>8)&255;a[n++]=t&255; }
	if (PH===1) { t=(C(0)<<10)|(C(1)<<4)|(C(2)>>2); a[n]=(t>>8)&255;a[n+1]=t&255; }
	else if (PH===2) a[n]=((C(0)<<2)|(C(1)>>4))&255;
	return a;
}

function SYSCALLS_WASM_IMPORTS(env, wasi)
{
	var PAYLOAD_CURSOR = 0;
	var PAYLOAD = (ZL.files ? Base64Decode(ZL.files) : new Uint8Array(0));
	delete ZL.files;

	env.__sys_open = env.__syscall5 = function(which, varargs) // open (only used to open payload)
	{
		PAYLOAD_CURSOR = 0;
		//var getVarArgs;
		//function get(set) { return HEAP32[(((set ? getVarArgs=set+4 : getVarArgs+=4))-4)>>2]; }
		//var pathname = Pointer_stringify(get(varargs)), flags = get(), mode = get();
		//console.log('___syscall5 open: ' + pathname);
		return 1;
	};

	wasi.fd_read = function(fd, iov, iovcnt, pOutResult) //read from payload
	{
		var ret = 0;
		for (var i = 0; i < iovcnt; i++)
		{
			var ptr = HEAP32[(((iov)+(i*8))>>2)];
			var len = HEAP32[(((iov)+(i*8 + 4))>>2)];
			var curr = Math.min(len, PAYLOAD.length - PAYLOAD_CURSOR);
			//console.log('__wasi_fd_read - fd: ' + fd + ' - iov: ' + iov + ' - iovcnt: ' + iovcnt + ' - ptr: ' + ptr + ' - len: ' + len + ' - reading: ' + curr + ' (from ' + PAYLOAD_CURSOR + ' to ' + (PAYLOAD_CURSOR + curr) + ')');

			HEAPU8.set(PAYLOAD.subarray(PAYLOAD_CURSOR, PAYLOAD_CURSOR + curr), ptr);
			PAYLOAD_CURSOR += curr;

			if (curr < 0) return 5;
			ret += curr;
			if (curr < len) break; // nothing more to read
		}
		//console.log('__wasi_fd_read -     ret: ' + ret);
		HEAPU32[pOutResult>>2] = ret;
		return 0;
	};

	wasi.fd_seek = function(fd, offset_low, offset_high, whence, pOutResult) //seek in payload
	{
		if (whence == 0) PAYLOAD_CURSOR = offset_low; //set
		if (whence == 1) PAYLOAD_CURSOR += offset_low; //cur
		if (whence == 2) PAYLOAD_CURSOR = PAYLOAD.length - offset_low; //end
		if (PAYLOAD_CURSOR < 0) PAYLOAD_CURSOR = 0;
		if (PAYLOAD_CURSOR > PAYLOAD.length) PAYLOAD_CURSOR = PAYLOAD.length;
		HEAPU32[(pOutResult+0)>>2] = PAYLOAD_CURSOR;
		HEAPU32[(pOutResult+4)>>2] = 0;
		//console.log('__wasi_fd_seek - fd: ' + fd + ' - offset_high: ' + offset_high + ' - offset_low: ' + offset_low + ' - pOutResult: ' + pOutResult + ' - whence: ' +whence + ' - seek to: ' + PAYLOAD_CURSOR);
		return 0;
	};

	wasi.fd_write = function(fd, iov, iovcnt, pOutResult) //only used to write to stdout
	{
		var ret = 0, str = '';
		if (iovcnt == 0) return 0;
		for (var i = 0; i < iovcnt; i++)
		{
			var ptr = HEAP32[(((iov)+(i*8))>>2)];
			var len = HEAP32[(((iov)+(i*8 + 4))>>2)];
			if (len < 0) return -1;
			ret += len;
			str += Pointer_stringify(ptr, len);
			//console.log('__wasi_fd_write - fd: ' + fd + ' - ['+i+'][len:'+len+']: ' + Pointer_stringify(ptr, len));
		}
		ZL.print(str);
		HEAPU32[pOutResult>>2] = ret;
		return 0;
	};

	wasi.fd_close = function(fd)
	{
		//console.log('__wasi_fd_close - fd: ' + fd);
		return 0;
	};

	env.__sys_fcntl64 = env.__sys_ioctl = env.__syscall221 = env.__syscall54 = function(which, varargs) // fcntl64, ioctl
	{
		return 0;
	};
}

var GLsetupContext;
function GL_WASM_IMPORTS(env)
{
	var GLctx;
	var GLcounter = 1;
	var GLbuffers = [];
	var GLprograms = [];
	var GLframebuffers = [];
	var GLtextures = [];
	var GLuniforms = [];
	var GLshaders = [];
	var GLprogramInfos = {};
	var GLpackAlignment = 4;
	var GLunpackAlignment = 4;
	var GLMINI_TEMP_BUFFER_SIZE = 256;
	var GLminiTempBuffer = null;
	var GLminiTempBufferViews = [0];
	GLminiTempBuffer = new Float32Array(GLMINI_TEMP_BUFFER_SIZE);
	for (var i = 0; i < GLMINI_TEMP_BUFFER_SIZE; i++) GLminiTempBufferViews[i] = GLminiTempBuffer.subarray(0, i+1);

	GLsetupContext = function(canvas, attr)
	{
		var attr = { majorVersion: 1, minorVersion: 0, antialias: true, alpha: false };
		var errorInfo = '';
		try
		{
			let onContextCreationError = function(event) { errorInfo = event.statusMessage || errorInfo; };
			canvas.addEventListener('webglcontextcreationerror', onContextCreationError, false);
			try
			{
				//if (attr.majorVersion == 1 && attr.minorVersion == 0)
					GLctx = canvas.getContext('webgl', attr) || canvas.getContext('experimental-webgl', attr);
				//else if (attr.majorVersion == 2 && attr.minorVersion == 0)
				//	GLctx = canvas.getContext('webgl2', attr);
				//else
				//	throw 'Unsupported WebGL context version ' + majorVersion + '.' + minorVersion;
			}
			finally { canvas.removeEventListener('webglcontextcreationerror', onContextCreationError, false); }
			if (!GLctx) throw 'Could not create context';
		}
		catch (e)
		{
			abort('WEBGL', e + (errorInfo ? ' (' + errorInfo + ')' : ''));
		}

		/*
		// Detect the presence of a few extensions manually, this GL interop layer itself will need to know if they exist.
		if (attr.majorVersion < 2)
		{
			// Extension available from Firefox 26 and Google Chrome 30
			var instancedArraysExt = GLctx.getExtension('ANGLE_instanced_arrays');
			if (instancedArraysExt)
			{
				GLctx.vertexAttribDivisor = function(index, divisor) { instancedArraysExt.vertexAttribDivisorANGLE(index, divisor); };
				GLctx.drawArraysInstanced = function(mode, first, count, primcount) { instancedArraysExt.drawArraysInstancedANGLE(mode, first, count, primcount); };
				GLctx.drawElementsInstanced = function(mode, count, type, indices, primcount) { instancedArraysExt.drawElementsInstancedANGLE(mode, count, type, indices, primcount); };
			}

			// Extension available from Firefox 25 and WebKit
			var vaoExt = GLctx.getExtension('OES_vertex_array_object');
			if (vaoExt)
			{
				GLctx.createVertexArray = function() { return vaoExt.createVertexArrayOES(); };
				GLctx.deleteVertexArray = function(vao) { vaoExt.deleteVertexArrayOES(vao); };
				GLctx.bindVertexArray = function(vao) { vaoExt.bindVertexArrayOES(vao); };
				GLctx.isVertexArray = function(vao) { return vaoExt.isVertexArrayOES(vao); };
			}

			var drawBuffersExt = GLctx.getExtension('WEBGL_draw_buffers');
			if (drawBuffersExt)
			{
				GLctx.drawBuffers = function(n, bufs) { drawBuffersExt.drawBuffersWEBGL(n, bufs); };
			}
		}

		GLctx.disjointTimerQueryExt = GLctx.getExtension('EXT_disjoint_timer_query');
		*/

		var exts = GLctx.getSupportedExtensions();
		if (exts && exts.length > 0)
		{
			// These are the 'safe' feature-enabling extensions that don't add any performance impact related to e.g. debugging, and
			// should be enabled by default so that client GLES2/GL code will not need to go through extra hoops to get its stuff working.
			// As new extensions are ratified at http://www.khronos.org/registry/webgl/extensions/ , feel free to add your new extensions
			// here, as long as they don't produce a performance impact for users that might not be using those extensions.
			// E.g. debugging-related extensions should probably be off by default.
			var W = 'WEBGL_', O = 'OES_', E = 'EXT_', T = 'texture_', C = 'compressed_'+T;
			var automaticallyEnabledExtensions = [ // Khronos ratified WebGL extensions ordered by number (no debug extensions):
				O+T+'float', O+T+'half_float', O+'standard_derivatives',
				O+'vertex_array_object', W+C+'s3tc', W+'depth_texture',
				O+'element_index_uint', E+T+'filter_anisotropic', E+'frag_depth',
				W+'draw_buffers', 'ANGLE_instanced_arrays', O+T+'float_linear',
				O+T+'half_float_linear', E+'blend_minmax', E+'shader_texture_lod',
				// Community approved WebGL extensions ordered by number:
				W+C+'pvrtc', E+'color_buffer_half_float', W+'color_buffer_float',
				E+'sRGB', W+C+'etc1', E+'disjoint_timer_query',
				W+C+'etc', W+C+'astc', E+'color_buffer_float',
				W+C+'s3tc_srgb', E+'disjoint_timer_query_webgl2'];
			exts.forEach(function(ext)
			{
				if (automaticallyEnabledExtensions.indexOf(ext) != -1)
				{
					// Calling .getExtension enables that extension permanently, no need to store the return value to be enabled.
					GLctx.getExtension(ext);
				}
			});
		}

		return true;
	};
	function getNewId(table)
	{
		var ret = GLcounter++;
		for (var i = table.length; i < ret; i++) table[i] = null;
		return ret;
	}
	function getSource(shader, count, string, length)
	{
		var source = '';
		for (var i = 0; i < count; ++i)
		{
			var frag;
			if (length)
			{
				var len = HEAP32[(((length)+(i*4))>>2)];
				if (len < 0) frag = Pointer_stringify(HEAP32[(((string)+(i*4))>>2)]);
				else frag = Pointer_stringify(HEAP32[(((string)+(i*4))>>2)], len);
			}
			else frag = Pointer_stringify(HEAP32[(((string)+(i*4))>>2)]);
			source += frag;
		}
		return source;
	}
	function populateUniformTable(program)
	{
		var p = GLprograms[program];
		GLprogramInfos[program] =
		{
			uniforms: {},
			maxUniformLength: 0, // This is eagerly computed below, since we already enumerate all uniforms anyway.
			maxAttributeLength: -1, // This is lazily computed and cached, computed when/if first asked, '-1' meaning not computed yet.
			maxUniformBlockNameLength: -1 // Lazily computed as well
		};

		var ptable = GLprogramInfos[program];
		var utable = ptable.uniforms;

		// A program's uniform table maps the string name of an uniform to an integer location of that uniform.
		// The global GLuniforms map maps integer locations to WebGLUniformLocations.
		var numUniforms = GLctx.getProgramParameter(p, GLctx.ACTIVE_UNIFORMS);
		for (var i = 0; i < numUniforms; ++i)
		{
			var u = GLctx.getActiveUniform(p, i);

			var name = u.name;
			ptable.maxUniformLength = Math.max(ptable.maxUniformLength, name.length+1);

			// Strip off any trailing array specifier we might have got, e.g. '[0]'.
			if (name.indexOf(']', name.length-1) !== -1)
			{
				var ls = name.lastIndexOf('[');
				name = name.slice(0, ls);
			}

			// Optimize memory usage slightly: If we have an array of uniforms, e.g. 'vec3 colors[3];', then
			// only store the string 'colors' in utable, and 'colors[0]', 'colors[1]' and 'colors[2]' will be parsed as 'colors'+i.
			// Note that for the GLuniforms table, we still need to fetch the all WebGLUniformLocations for all the indices.
			var loc = GLctx.getUniformLocation(p, name);
			if (loc != null)
			{
				var id = getNewId(GLuniforms);
				utable[name] = [u.size, id];
				GLuniforms[id] = loc;

				for (var j = 1; j < u.size; ++j)
				{
					var n = name + '['+j+']';
					loc = GLctx.getUniformLocation(p, n);
					id = getNewId(GLuniforms);

					GLuniforms[id] = loc;
				}
			}
		}
	}
	function GLrecordError(err)
	{
		if (!GLlastError) GLlastError = err;
	}

	env.glActiveTexture = function(x0) { GLctx.activeTexture(x0); };
	env.glAttachShader = function(program, shader) { GLctx.attachShader(GLprograms[program], GLshaders[shader]); };
	env.glBindAttribLocation = function(program, index, name) { GLctx.bindAttribLocation(GLprograms[program], index, Pointer_stringify(name)); };
	env.glBindBuffer = function(target, buffer) { GLctx.bindBuffer(target, buffer ? GLbuffers[buffer] : null); };
	env.glBindFramebuffer = function(target, framebuffer) { GLctx.bindFramebuffer(target, framebuffer ? GLframebuffers[framebuffer] : null); };
	env.glBindTexture = function(target, texture) { GLctx.bindTexture(target, texture ? GLtextures[texture] : null); };
	env.glBlendFunc = function(x0, x1) { GLctx.blendFunc(x0, x1); };
	env.glBlendFuncSeparate = function(x0, x1, x2, x3) { GLctx.blendFuncSeparate(x0, x1, x2, x3); }
	env.glBlendColor = function(x0, x1, x2, x3) { GLctx.blendColor(x0, x1, x2, x3); }
	env.glBlendEquation = function(x0) { GLctx.blendEquation(x0); }
	env.glBlendEquationSeparate = function(x0, x1) { GLctx.blendEquationSeparate(x0, x1); }

	env.glBufferData = function(target, size, data, usage)
	{
		if (!data) GLctx.bufferData(target, size, usage);
		else GLctx.bufferData(target, HEAPU8.subarray(data, data+size), usage);
	};

	env.glBufferSubData = function(target, offset, size, data) { GLctx.bufferSubData(target, offset, HEAPU8.subarray(data, data+size)); };
	env.glClear = function(x0) { GLctx.clear(x0); };
	env.glClearColor = function(x0, x1, x2, x3) { GLctx.clearColor(x0, x1, x2, x3); };
	env.glColorMask = function(red, green, blue, alpha) { GLctx.colorMask(!!red, !!green, !!blue, !!alpha); };
	env.glCompileShader = function(shader) { GLctx.compileShader(GLshaders[shader]); };

	env.glCreateProgram = function()
	{
		var id = getNewId(GLprograms);
		var program = GLctx.createProgram();
		program.name = id;
		GLprograms[id] = program;
		return id;
	};

	env.glCreateShader = function(shaderType)
	{
		var id = getNewId(GLshaders);
		GLshaders[id] = GLctx.createShader(shaderType);
		return id;
	};

	env.glDeleteBuffers = function(n, buffers)
	{
		for (var i = 0; i < n; i++)
		{
			var id = HEAP32[(((buffers)+(i*4))>>2)];
			var buffer = GLbuffers[id];

			// From spec: "glDeleteBuffers silently ignores 0's and names that do not correspond to existing buffer objects."
			if (!buffer) continue;

			GLctx.deleteBuffer(buffer);
			buffer.name = 0;
			GLbuffers[id] = null;
		}
	};

	env.glDeleteFramebuffers = function(n, framebuffers)
	{
		for (var i = 0; i < n; ++i)
		{
			var id = HEAP32[(((framebuffers)+(i*4))>>2)];
			var framebuffer = GLframebuffers[id];
			if (!framebuffer) continue; // GL spec: "glDeleteFramebuffers silently ignores 0s and names that do not correspond to existing framebuffer objects".
			GLctx.deleteFramebuffer(framebuffer);
			framebuffer.name = 0;
			GLframebuffers[id] = null;
		}
	};

	env.glDeleteProgram = function(id)
	{
		if (!id) return;
		var program = GLprograms[id];
		if (!program) 
		{
			// glDeleteProgram actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
			GLrecordError(0x0501); // GL_INVALID_VALUE
			return;
		}
		GLctx.deleteProgram(program);
		program.name = 0;
		GLprograms[id] = null;
		GLprogramInfos[id] = null;
	};

	env.glDeleteShader = function(id)
	{
		if (!id) return;
		var shader = GLshaders[id];
		if (!shader)
		{
			// glDeleteShader actually signals an error when deleting a nonexisting object, unlike some other GL delete functions.
			GLrecordError(0x0501); // GL_INVALID_VALUE
			return;
		}
		GLctx.deleteShader(shader);
		GLshaders[id] = null;
	};

	env.glDeleteTextures = function(n, textures)
	{
		for (var i = 0; i < n; i++)
		{
			var id = HEAP32[(((textures)+(i*4))>>2)];
			var texture = GLtextures[id];
			if (!texture) continue; // GL spec: "glDeleteTextures silently ignores 0s and names that do not correspond to existing textures".
			GLctx.deleteTexture(texture);
			texture.name = 0;
			GLtextures[id] = null;
		}
	};

	env.glDepthFunc = function(x0) { GLctx.depthFunc(x0); };
	env.glDepthMask = function(flag) { GLctx.depthMask(!!flag); };
	env.glDetachShader = function(program, shader) { GLctx.detachShader(GLprograms[program], GLshaders[shader]); };

	env.glDisable = function(x0) { GLctx.disable(x0); };
	env.glDisableVertexAttribArray = function(index) { GLctx.disableVertexAttribArray(index); };
	env.glDrawArrays = function(mode, first, count) { GLctx.drawArrays(mode, first, count); };
	env.glDrawElements = function(mode, count, type, indices) { GLctx.drawElements(mode, count, type, indices); };
	env.glEnable = function(x0) { GLctx.enable(x0); };
	env.glEnableVertexAttribArray = function(index) { GLctx.enableVertexAttribArray(index); };
	env.glFramebufferTexture2D = function(target, attachment, textarget, texture, level) { GLctx.framebufferTexture2D(target, attachment, textarget, GLtextures[texture], level); };

	env.glGenBuffers = function(n, buffers)
	{
		for (var i = 0; i < n; i++)
		{
			var buffer = GLctx.createBuffer();
			if (!buffer)
			{
				GLrecordError(0x0502); // GL_INVALID_OPERATION
				while(i < n) HEAP32[(((buffers)+(i++*4))>>2)]=0;
				return;
			}
			var id = getNewId(GLbuffers);
			buffer.name = id;
			GLbuffers[id] = buffer;
			HEAP32[(((buffers)+(i*4))>>2)]=id;
		}
	};

	env.glGenFramebuffers = function(n, ids)
	{
		for (var i = 0; i < n; ++i)
		{
			var framebuffer = GLctx.createFramebuffer();
			if (!framebuffer)
			{
				GLrecordError(0x0502); // GL_INVALID_OPERATION
				while(i < n) HEAP32[(((ids)+(i++*4))>>2)]=0;
				return;
			}
			var id = getNewId(GLframebuffers);
			framebuffer.name = id;
			GLframebuffers[id] = framebuffer;
			HEAP32[(((ids)+(i*4))>>2)] = id;
		}
	};

	env.glGenTextures = function(n, textures)
	{
		for (var i = 0; i < n; i++)
		{
			var texture = GLctx.createTexture();
			if (!texture)
			{
				// GLES + EGL specs don't specify what should happen here, so best to issue an error and create IDs with 0.
				GLrecordError(0x0502); // GL_INVALID_OPERATION
				while(i < n) HEAP32[(((textures)+(i++*4))>>2)]=0;
				return;
			}
			var id = getNewId(GLtextures);
			texture.name = id;
			GLtextures[id] = texture;
			HEAP32[(((textures)+(i*4))>>2)]=id;
		}
	};

	env.glGetActiveUniform = function(program, index, bufSize, length, size, type, name)
	{
		program = GLprograms[program];
		var info = GLctx.getActiveUniform(program, index);
		if (!info) return; // If an error occurs, nothing will be written to length, size, type and name.

		if (bufSize > 0 && name)
		{
			var numBytesWrittenExclNull = stringToUTF8Array(info.name, HEAP8, name, bufSize);
			if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
		} else {
			if (length) HEAP32[((length)>>2)]=0;
		}

		if (size) HEAP32[((size)>>2)]=info.size;
		if (type) HEAP32[((type)>>2)]=info.type;
	};
	
	env.glGetAttribLocation = function(program, name)
	{
		program = GLprograms[program];
		name = Pointer_stringify(name);
		return GLctx.getAttribLocation(program, name);
	};

	function webGLGet(name_, p, type)
	{
		// Guard against user passing a null pointer.
		// Note that GLES2 spec does not say anything about how passing a null pointer should be treated.
		// Testing on desktop core GL 3, the application crashes on glGetIntegerv to a null pointer, but
		// better to report an error instead of doing anything random.
		if (!p) { GLrecordError(0x0501); return; } // GL_INVALID_VALUE

		var ret = undefined;
		switch(name_)
		{
			// Handle a few trivial GLES values
			case 0x8DFA: ret = 1; break; // GL_SHADER_COMPILER
			case 0x8DF8: // GL_SHADER_BINARY_FORMATS
				if (type !== 'Integer' && type !== 'Integer64') GLrecordError(0x0500); // GL_INVALID_ENUM
				return; // Do not write anything to the out pointer, since no binary formats are supported.
			case 0x8DF9: ret = 0; break; // GL_NUM_SHADER_BINARY_FORMATS
			case 0x86A2: // GL_NUM_COMPRESSED_TEXTURE_FORMATS
				// WebGL doesn't have GL_NUM_COMPRESSED_TEXTURE_FORMATS (it's obsolete since GL_COMPRESSED_TEXTURE_FORMATS returns a JS array that can be queried for length),
				// so implement it ourselves to allow C++ GLES2 code get the length.
				var formats = GLctx.getParameter(0x86A3); // GL_COMPRESSED_TEXTURE_FORMATS
				ret = formats.length;
				break;
		}

		if (ret === undefined)
		{
			var result = GLctx.getParameter(name_);
			switch (typeof(result))
			{
				case 'number':
					ret = result;
					break;
				case 'boolean':
					ret = result ? 1 : 0;
					break;
				case 'string':
					GLrecordError(0x0500); // GL_INVALID_ENUM
					return;
				case 'object':
					if (result === null)
					{
						// null is a valid result for some (e.g., which buffer is bound - perhaps nothing is bound), but otherwise
						// can mean an invalid name_, which we need to report as an error
						switch(name_)
						{
							case 0x8894: // ARRAY_BUFFER_BINDING
							case 0x8B8D: // CURRENT_PROGRAM
							case 0x8895: // ELEMENT_ARRAY_BUFFER_BINDING
							case 0x8CA6: // FRAMEBUFFER_BINDING
							case 0x8CA7: // RENDERBUFFER_BINDING
							case 0x8069: // TEXTURE_BINDING_2D
							case 0x8514: // TEXTURE_BINDING_CUBE_MAP
								ret = 0;
								break;
							default:
								GLrecordError(0x0500); // GL_INVALID_ENUM
								return;
						}
					}
					else if (result instanceof Float32Array || result instanceof Uint32Array || result instanceof Int32Array || result instanceof Array)
					{
						for (var i = 0; i < result.length; ++i) {
							switch (type)
							{
								case 'Integer': HEAP32[(((p)+(i*4))>>2)]=result[i]; break;
								case 'Float':   HEAPF32[(((p)+(i*4))>>2)]=result[i]; break;
								case 'Boolean': HEAP8[(((p)+(i))>>0)]=result[i] ? 1 : 0; break;
								default: abort('WEBGL', 'internal glGet error, bad type: ' + type);
							}
						}
						return;
					}
					else if (result instanceof WebGLBuffer || result instanceof WebGLProgram || result instanceof WebGLFramebuffer || result instanceof WebGLRenderbuffer || result instanceof WebGLTexture)
					{
						ret = result.name | 0;
					}
					else
					{
						GLrecordError(0x0500); // GL_INVALID_ENUM
						return;
					}
					break;
				default:
					GLrecordError(0x0500); // GL_INVALID_ENUM
					return;
			}
		}

		switch (type)
		{
			case 'Integer64': (tempI64 = [ret>>>0,(tempDouble=ret,(+(Math.abs(tempDouble))) >= 1.0 ? (tempDouble > 0.0 ? ((Math.min((+(Math.floor((tempDouble)/4294967296.0))), 4294967295.0))|0)>>>0 : (~~((+(Math.ceil((tempDouble - +(((~~(tempDouble)))>>>0))/4294967296.0)))))>>>0) : 0)],HEAP32[((p)>>2)]=tempI64[0],HEAP32[(((p)+(4))>>2)]=tempI64[1]); break;
			case 'Integer': HEAP32[((p)>>2)] = ret; break;
			case 'Float':   HEAPF32[((p)>>2)] = ret; break;
			case 'Boolean': HEAP8[((p)>>0)] = ret ? 1 : 0; break;
			default: abort('WEBGL', 'internal glGet error, bad type: ' + type);
		}
	}

	env.glGetError = function()
	{
		if (GLlastError)
		{
			var e = GLlastError;
			GLlastError = 0;
			return e;
		}
		return GLctx.getError();
	};

	env.glGetIntegerv = function(name_, p)
	{
		webGLGet(name_, p, 'Integer');
	};

	env.glGetProgramInfoLog = function(program, maxLength, length, infoLog)
	{
		var log = GLctx.getProgramInfoLog(GLprograms[program]);
		if (log === null) log = '(unknown error)';
		if (maxLength > 0 && infoLog)
		{
			var numBytesWrittenExclNull = stringToUTF8Array(log, HEAP8, infoLog, maxLength);
			if (length) HEAP32[((length)>>2)]=numBytesWrittenExclNull;
		}
		else if (length) HEAP32[((length)>>2)]=0;
	};

	env.glGetProgramiv = function(program, pname, p)
	{
		if (!p)
		{
			// GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
			// if p == null, issue a GL error to notify user about it.
			GLrecordError(0x0501); // GL_INVALID_VALUE
			return;
		}

		if (program >= GLcounter)
		{
			GLrecordError(0x0501); // GL_INVALID_VALUE
			return;
		}

		var ptable = GLprogramInfos[program];
		if (!ptable)
		{
			GLrecordError(0x0502); //GL_INVALID_OPERATION
			return;
		}

		if (pname == 0x8B84) // GL_INFO_LOG_LENGTH
		{
			var log = GLctx.getProgramInfoLog(GLprograms[program]);
			if (log === null) log = '(unknown error)';
			HEAP32[((p)>>2)] = log.length + 1;
		}
		else if (pname == 0x8B87) //GL_ACTIVE_UNIFORM_MAX_LENGTH
		{
			HEAP32[((p)>>2)] = ptable.maxUniformLength;
		}
		else if (pname == 0x8B8A) //GL_ACTIVE_ATTRIBUTE_MAX_LENGTH
		{
			if (ptable.maxAttributeLength == -1)
			{
				program = GLprograms[program];
				var numAttribs = GLctx.getProgramParameter(program, GLctx.ACTIVE_ATTRIBUTES);
				ptable.maxAttributeLength = 0; // Spec says if there are no active attribs, 0 must be returned.
				for (var i = 0; i < numAttribs; ++i)
				{
					var activeAttrib = GLctx.getActiveAttrib(program, i);
					ptable.maxAttributeLength = Math.max(ptable.maxAttributeLength, activeAttrib.name.length+1);
				}
			}
			HEAP32[((p)>>2)] = ptable.maxAttributeLength;
		}
		else if (pname == 0x8A35) //GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH
		{
			if (ptable.maxUniformBlockNameLength == -1)
			{
				program = GLprograms[program];
				var numBlocks = GLctx.getProgramParameter(program, GLctx.ACTIVE_UNIFORM_BLOCKS);
				ptable.maxUniformBlockNameLength = 0;
				for (var i = 0; i < numBlocks; ++i)
				{
					var activeBlockName = GLctx.getActiveUniformBlockName(program, i);
					ptable.maxUniformBlockNameLength = Math.max(ptable.maxUniformBlockNameLength, activeBlockName.length+1);
				}
			}
			HEAP32[((p)>>2)] = ptable.maxUniformBlockNameLength;
		}
		else
		{
			HEAP32[((p)>>2)] = GLctx.getProgramParameter(GLprograms[program], pname);
		}
	};

	env.glGetShaderInfoLog = function(shader, maxLength, length, infoLog)
	{
		var log = GLctx.getShaderInfoLog(GLshaders[shader]);
		if (log === null) log = '(unknown error)';
		if (maxLength > 0 && infoLog)
		{
			var numBytesWrittenExclNull = stringToUTF8Array(log, HEAP8, infoLog, maxLength);
			if (length) HEAP32[((length)>>2)] = numBytesWrittenExclNull;
		}
		else if (length) HEAP32[((length)>>2)] = 0;
	};

	env.glGetShaderiv = function(shader, pname, p)
	{
		if (!p)
		{
			// GLES2 specification does not specify how to behave if p is a null pointer. Since calling this function does not make sense
			// if p == null, issue a GL error to notify user about it.
			GLrecordError(0x0501); // GL_INVALID_VALUE
			return;
		}
		if (pname == 0x8B84) // GL_INFO_LOG_LENGTH
		{
			var log = GLctx.getShaderInfoLog(GLshaders[shader]);
			if (log === null) log = '(unknown error)';
			HEAP32[((p)>>2)] = log.length + 1;
		}
		else if (pname == 0x8B88) // GL_SHADER_SOURCE_LENGTH
		{
			var source = GLctx.getShaderSource(GLshaders[shader]);
			var sourceLength = (source === null || source.length == 0) ? 0 : source.length + 1;
			HEAP32[((p)>>2)] = sourceLength;
		}
		else HEAP32[((p)>>2)] = GLctx.getShaderParameter(GLshaders[shader], pname);
	};

	env.glGetUniformLocation = function(program, name)
	{
		name = Pointer_stringify(name);

		var arrayOffset = 0;
		if (name.indexOf(']', name.length-1) !== -1)
		{
			// If user passed an array accessor "[index]", parse the array index off the accessor.
			var ls = name.lastIndexOf('[');
			var arrayIndex = name.slice(ls+1, -1);
			if (arrayIndex.length > 0)
			{
				arrayOffset = parseInt(arrayIndex);
				if (arrayOffset < 0) return -1;
			}
			name = name.slice(0, ls);
		}

		var ptable = GLprogramInfos[program];
		if (!ptable) return -1;
		var utable = ptable.uniforms;
		var uniformInfo = utable[name]; // returns pair [ dimension_of_uniform_array, uniform_location ]
		if (uniformInfo && arrayOffset < uniformInfo[0])
		{
			// Check if user asked for an out-of-bounds element, i.e. for 'vec4 colors[3];' user could ask for 'colors[10]' which should return -1.
			return uniformInfo[1] + arrayOffset;
		}
		return -1;
	};

	env.glLineWidth = function(x0) { GLctx.lineWidth(x0); };

	env.glLinkProgram = function(program)
	{
		GLctx.linkProgram(GLprograms[program]);
		GLprogramInfos[program] = null; // uniforms no longer keep the same names after linking
		populateUniformTable(program);
	};

	env.glPixelStorei = function(pname, param)
	{
		if (pname == 0x0D05) GLpackAlignment = param; //GL_PACK_ALIGNMENT
		else if (pname == 0x0cf5) GLunpackAlignment = param; //GL_UNPACK_ALIGNMENT
		GLctx.pixelStorei(pname, param);
	};

	function webGLGetTexPixelData(type, format, width, height, pixels, internalFormat)
	{
		var sizePerPixel;
		var numChannels;
		switch(format)
		{
			case 0x1906: case 0x1909: case 0x1902: numChannels = 1; break; //GL_ALPHA, GL_LUMINANCE, GL_DEPTH_COMPONENT
			case 0x190A: numChannels = 2; break; //GL_LUMINANCE_ALPHA
			case 0x1907: case 0x8C40: numChannels = 3; break; //GL_RGB, GL_SRGB_EXT
			case 0x1908: case 0x8C42: numChannels = 4; break; //GL_RGBA, GL_SRGB_ALPHA_EXT
			default: GLrecordError(0x0500); return null; //GL_INVALID_ENUM
		}
		switch (type)
		{
			case 0x1401: sizePerPixel = numChannels*1; break; //GL_UNSIGNED_BYTE
			case 0x1403: case 0x8D61: sizePerPixel = numChannels*2; break; //GL_UNSIGNED_SHORT, GL_HALF_FLOAT_OES
			case 0x1405: case 0x1406: sizePerPixel = numChannels*4; break; //GL_UNSIGNED_INT, GL_FLOAT
			case 0x84FA: sizePerPixel = 4; break; //GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8
			case 0x8363: case 0x8033: case 0x8034: sizePerPixel = 2; break; //GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_5_5_5_1
			default: GLrecordError(0x0500); return null; //GL_INVALID_ENUM
		}

		function roundedToNextMultipleOf(x, y) { return Math.floor((x + y - 1) / y) * y; }
		var plainRowSize = width * sizePerPixel;
		var alignedRowSize = roundedToNextMultipleOf(plainRowSize, GLunpackAlignment);
		var bytes = (height <= 0 ? 0 : ((height - 1) * alignedRowSize + plainRowSize));

		switch(type)
		{
			case 0x1401: return HEAPU8.subarray((pixels),(pixels+bytes)); //GL_UNSIGNED_BYTE
			case 0x1406: return HEAPF32.subarray((pixels)>>2,(pixels+bytes)>>2); //GL_FLOAT
			case 0x1405: case 0x84FA: return HEAPU32.subarray((pixels)>>2,(pixels+bytes)>>2); //GL_UNSIGNED_INT, GL_UNSIGNED_INT_24_8_WEBGL/GL_UNSIGNED_INT_24_8
			case 0x1403: case 0x8363: case 0x8033: case 0x8034: case 0x8D61: return HEAPU16.subarray((pixels)>>1,(pixels+bytes)>>1); //GL_UNSIGNED_SHORT, GL_UNSIGNED_SHORT_5_6_5, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_5_5_5_1, GL_HALF_FLOAT_OES
			default: GLrecordError(0x0500); return null; //GL_INVALID_ENUM
		}
	}

	env.glReadPixels = function(x, y, width, height, format, type, pixels)
	{
		var pixelData = webGLGetTexPixelData(type, format, width, height, pixels, format);
		if (!pixelData) return GLrecordError(0x0500); // GL_INVALID_ENUM
		GLctx.readPixels(x, y, width, height, format, type, pixelData);
	};

	env.glScissor = function(x0, x1, x2, x3) { GLctx.scissor(x0, x1, x2, x3) };

	env.glShaderSource = function(shader, count, string, length)
	{
		var source = getSource(shader, count, string, length);
		GLctx.shaderSource(GLshaders[shader], source);
	};

	env.glTexImage2D = function(target, level, internalFormat, width, height, border, format, type, pixels)
	{
		var pixelData = null;
		if (pixels) pixelData = webGLGetTexPixelData(type, format, width, height, pixels, internalFormat);
		GLctx.texImage2D(target, level, internalFormat, width, height, border, format, type, pixelData);
	};

	env.glTexParameteri = function(x0, x1, x2)
	{
		GLctx.texParameteri(x0, x1, x2);
	};

	env.glTexSubImage2D = function(target, level, xoffset, yoffset, width, height, format, type, pixels)
	{
		var pixelData = null;
		if (pixels) pixelData = webGLGetTexPixelData(type, format, width, height, pixels, 0);
		GLctx.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixelData);
	};

	env.glUniform1f = function(loc, v0) { GLctx.uniform1f(GLuniforms[loc], v0); };
	env.glUniform1i = function(loc, v0) { GLctx.uniform1i(GLuniforms[loc], v0); };
	env.glUniform2f = function(loc, v0, v1) { GLctx.uniform2f(GLuniforms[loc], v0, v1); };
	env.glUniform3f = function(loc, v0, v1, v2) { GLctx.uniform3f(GLuniforms[loc], v0, v1, v2); };

	env.glUniform3fv = function(loc, count, value)
	{
		var view;
		if (3*count <= GLMINI_TEMP_BUFFER_SIZE)
		{
			// avoid allocation when uploading few enough uniforms
			view = GLminiTempBufferViews[3*count-1];
			for (var ptr = value>>2, i = 0; i != 3*count; i++)
			{
				view[i] = HEAPF32[ptr+i];
			}
		}
		else view = HEAPF32.subarray((value)>>2,(value+count*12)>>2);
		GLctx.uniform3fv(GLuniforms[loc], view);
	};

	env.glUniform4f = function(loc, v0, v1, v2, v3) { GLctx.uniform4f(GLuniforms[loc], v0, v1, v2, v3); };

	env.glUniformMatrix4fv = function(loc, count, transpose, value)
	{
		count<<=4;
		var view;
		if (count <= GLMINI_TEMP_BUFFER_SIZE)
		{
			// avoid allocation when uploading few enough uniforms
			view = GLminiTempBufferViews[count-1];
			for (var ptr = value>>2, i = 0; i != count; i += 4)
			{
				view[i  ] = HEAPF32[ptr+i  ];
				view[i+1] = HEAPF32[ptr+i+1];
				view[i+2] = HEAPF32[ptr+i+2];
				view[i+3] = HEAPF32[ptr+i+3];
			}
		}
		else view = HEAPF32.subarray((value)>>2,(value+count*4)>>2);
		GLctx.uniformMatrix4fv(GLuniforms[loc], !!transpose, view);
	};

	env.glUseProgram = function(program) { GLctx.useProgram(program ? GLprograms[program] : null); };
	env.glVertexAttrib4f = function(x0, x1, x2, x3, x4) { GLctx.vertexAttrib4f(x0, x1, x2, x3, x4); };
	env.glVertexAttrib4fv = function(index, v) { GLctx.vertexAttrib4f(index, HEAPF32[v>>2], HEAPF32[v+4>>2], HEAPF32[v+8>>2], HEAPF32[v+12>>2]); };
	env.glVertexAttribPointer = function(index, size, type, normalized, stride, ptr) { GLctx.vertexAttribPointer(index, size, type, !!normalized, stride, ptr); };
	env.glViewport = function(x0, x1, x2, x3) { GLctx.viewport(x0, x1, x2, x3); };
}

function ZLJS_WASM_IMPORTS(env)
{
	var initTime, settings_prefix = 'ZL', lock_flag = 0, ws, ls = {};
	try { ls = window.localStorage||{}; } catch (e) {}

	function findAlias(el, a, b, c) { return el[a+c] || el['moz'+b+c] || el['webkit'+b+c] || el['ms'+b+c]; }

	function do_lock(f)
	{
		lock_flag = f;
		var el = (f ? ZL.canvas : document), fs = (f ? findAlias(el,'r','R','equestPointerLock') : findAlias(el,'e','E','xitPointerLock'))
		if (fs) fs.apply(el, []);
	}

	env.ZLJS_CreateWindow = function(width, height, tpf_limit)
	{
		if (ZL.requestWidth)
		{
			height = (ZL.requestHeight ? ZL.requestHeight : ZL.requestWidth*height/width);
			width = ZL.requestWidth;
		}
		else if (ZL.requestHeight)
		{
			width = ZL.requestHeight*width/height;
			height = ZL.requestHeight;
		}

		var cnvs = ZL.canvas;
		var cnvsResetSize = function() { cnvs.width = width; cnvs.height = height; cnvs.height = cnvs.clientHeight; cnvs.width = cnvs.clientWidth; }
		cnvsResetSize();

		if (!GLsetupContext(cnvs)) return;

		var mousefocus = true, pointerlock = null, fullscreen = null;
		var cancelEvent = function(e) { if (e.preventDefault) e.preventDefault(true); else if (e.stopPropagation) e.stopPropagation(true); else e.stopped = true; };
		var cancelIfFocus = function(e) { if (mousefocus) cancelEvent(e); };
		var mx = function(e) { return (e.offsetX !== undefined ? e.offsetX : e.clientX - (fullscreen ? 0 : cnvs.getBoundingClientRect().left)) * cnvs.width  / cnvs.clientWidth;  };
		var my = function(e) { return (e.offsetY !== undefined ? e.offsetY : e.clientY - (fullscreen ? 0 : cnvs.getBoundingClientRect().top )) * cnvs.height / cnvs.clientHeight; };
		var eventAliases = function(t, f, evntf) { evntf('moz'+t, f, 1); evntf('webkit'+t, f, 1); evntf('ms'+t, f, 1); }
		var windEvent = function(t, f) { window.addEventListener(t, f, true); }
		var cnvsEvent = function(t, f) { cnvs.addEventListener(t, f, {capture:true,passive:false}); }
		var docuEvent = function(t, f, a) { document.addEventListener(t, f); if (!a) eventAliases(t, f, docuEvent); }
		windEvent('keydown', function(e)
		{
			ZL.asm.ZLFNKey(true, e.keyCode, e.shiftKey, e.ctrlKey, e.altKey, e.metaKey);
			var f;
			if (e.char !== undefined) { if (!e.char) cancelEvent(e); }
			else if (!e.keyIdentifier || (!(f = e.keyIdentifier.match(/^U\+(\S+)$/))) || ('0x'+f[1]|0) < 32) cancelEvent(e);
		});
		windEvent('keyup', function(e)
		{
			ZL.asm.ZLFNKey(false, e.keyCode, e.shiftKey, e.ctrlKey, e.altKey, e.metaKey);
			cancelEvent(e);
		});
		windEvent('keypress', function(e)
		{
			var chr = e.charCode || (e.char && e.char.charCodeAt(0));
			if (chr >= 32) ZL.asm.ZLFNText(chr);
			cancelEvent(e);
		});
		cnvsEvent('mousemove', function(e)
		{
			if (!pointerlock) ZL.asm.ZLFNMove(mx(e), my(e), 0, 0, 1);
			else ZL.asm.ZLFNMove(0, 0, (e.movementX|0), (e.movementY|0), 1);
			cancelIfFocus(e);
		});
		var mouseUpDown = function(e, v) { ZL.asm.ZLFNMouse(e.button,  v, mx(e), my(e), 1); cancelIfFocus(e); if (lock_flag && !pointerlock) do_lock(1); };
		var touchStartEnd = function(e, v) { for (var i = 0, t = e.changedTouches; i != t.length; i++) ZL.asm.ZLFNMouse(0, v, mx(t[i]), my(t[i]), 9+t[i].identifier); cancelIfFocus(e); };
		cnvsEvent('mousedown',      function(e) { mouseUpDown(e, true); });
		cnvsEvent('mouseup',        function(e) { mouseUpDown(e, false); });
		cnvsEvent('mousewheel',     function(e) { ZL.asm.ZLFNWheel(e.wheelDelta); cancelIfFocus(e); });
		cnvsEvent('DOMMouseScroll', function(e) { ZL.asm.ZLFNWheel(-e.detail*40); cancelIfFocus(e); });
		cnvsEvent('mouseover',      function(e) { if (pointerlock||fullscreen) return; mousefocus = true;  ZL.asm.ZLFNWindow(1,0,0); });
		cnvsEvent('mouseout',       function(e) { if (pointerlock||fullscreen) return; mousefocus = false; ZL.asm.ZLFNWindow(2,0,0); });
		cnvsEvent('touchstart',     function(e) { touchStartEnd(e, true); });
		cnvsEvent('touchmove',      function(e) { for (var i = 0, t = e.changedTouches; i != t.length; i++) ZL.asm.ZLFNMove(mx(t[i]), my(t[i]), 0, 0, 9+t[i].identifier); cancelIfFocus(e); });
		cnvsEvent('touchend',       function(e) { touchStartEnd(e, false); });
		cnvsEvent('touchcancel',    function(e) { touchStartEnd(e, false); });
		windEvent('focus',          function(e) { if (e.target == window) ZL.asm.ZLFNWindow(3,0,0); });
		windEvent('blur',           function(e) { if (e.target == window) ZL.asm.ZLFNWindow(4,0,0); });

		windEvent('resize', function(e)
		{
			if (cnvs.clientWidth<32 || cnvs.clientHeight<32 || (cnvs.width == cnvs.clientWidth && cnvs.height == cnvs.clientHeight)) return;
			cnvs.height = cnvs.clientHeight;
			cnvs.width = cnvs.clientWidth;
			ZL.asm.ZLFNWindow(6, cnvs.width, cnvs.height);
		});

		docuEvent('fullscreenchange', function()
		{
			fullscreen = findAlias(document,'f','F','ullscreenElement') || findAlias(document,'f','F','ullScreenElement');
			if (fullscreen) { cnvs.orgStyle = cnvs.style.cssText; cnvs.style.cssText = 'background:black'; cnvs.height = screen.height; cnvs.width = screen.width; }
			else { if (cnvs.orgStyle) cnvs.style.cssText = cnvs.orgStyle; cnvsResetSize(); }
			ZL.asm.ZLFNWindow((fullscreen ? 5 : 6), cnvs.width, cnvs.height);
		});

		docuEvent('pointerlockchange', function()
		{
			pointerlock = findAlias(document,'p','P','ointerLockElement');
			ZL.asm.ZLFNWindow((pointerlock ? 7 : 8),0,0);
		});

		initTime = Date.now();

		if (tpf_limit > (1000/55))
		{
			window.requestAnimationFrame(ZL.asm.ZLFNDraw);
			setInterval(function(){window.requestAnimationFrame(ZL.asm.ZLFNDraw);}, tpf_limit);
		}
		else
		{
			var draw_func_ex = function() { if (ABORT) return; window.requestAnimationFrame(draw_func_ex); ZL.asm.ZLFNDraw(); };
			window.requestAnimationFrame(draw_func_ex);
		}
	};

	env.ZLJS_GetWidth = function(type) { return ZL.canvas.width; };
	env.ZLJS_GetHeight = function(type) { return ZL.canvas.height; };
	env.ZLJS_GetTime = function(type) { return Date.now() - initTime; };

	env.ZLJS_SetFullscreen = ZL.SetFullscreen = function(f)
	{
		var el = (f ? ZL.canvas : document), fs;
		if (f) fs = findAlias(el,'r','R','equestFullscreen') || findAlias(el,'r','R','equestFullScreen');
		else fs = findAlias(el,'e','E','xitFullscreen') || findAlias(el,'c','C','ancelFullScreen');
		if (fs) fs.apply(el, []);
	};

	env.ZLJS_SetPointerLock = function(f)
	{
		do_lock(f);
	};

	env.ZLJS_AsyncLoad = function(url, impl, postdata, postlength, timeout)
	{
		var xhr = new XMLHttpRequest(), ZLFNHTTP = ZL.asm.ZLFNHTTP;
		xhr.open((postlength ? 'POST' : 'GET'), Pointer_stringify(url), true);
		xhr.responseType = 'arraybuffer';
		xhr.timeout = timeout;
		xhr.onload = function()
		{
			if (xhr.status == 200)
			{
				var b = malloc_array(new Uint8Array(xhr.response));
				ZLFNHTTP(impl, 200, b, xhr.response.byteLength);
				ZL.asm.free(b);
			}
			else ZLFNHTTP(impl, xhr.status, 0, 0);
		};
		xhr.ontimeout = xhr.onerror = function(event)
		{
			// this could be called synchronously by xhr.send() so force it to arrive a frame later
			setTimeout(function() { ZLFNHTTP(impl, xhr.status||-1, 0, 0); });
		};
		if (postlength) try { xhr.send(HEAPU8.subarray(postdata, postdata+postlength)); } catch (e) { xhr.send(HEAPU8.buffer.slice(postdata, postdata+postlength)); }
		else xhr.send(null);
	};

	env.ZLJS_Websocket = function(impl, cmd, param, len)
	{
		var w, ZLFNWebSocket = ZL.asm.ZLFNWebSocket;
		if (cmd == 1) { if (ws) ws.send(Pointer_stringify(param,len)); return; }
		if (cmd == 2) { if (ws) ws.send(HEAPU8.subarray(param, param+len)); return; }
		if (cmd >= 3) { if (ws) ws.close(cmd-3, len?Pointer_stringify(param,len):undefined); ws = undefined; return; }
		try { var w = new WebSocket(Pointer_stringify(param)); } catch (e) { setTimeout(function() { ZLFNWebSocket(impl, 1009); }); return; }
		w.binaryType = 'arraybuffer';
		w.onopen = function() { ZLFNWebSocket(impl, 0); };
		w.onmessage = function (evt)
		{
			var s = typeof evt.data === 'string', v = s ? evt.data : new Uint8Array(evt.data), b = s ? malloc_string(v) : malloc_array(v);
			ZLFNWebSocket(impl, s ? 1 : 2, b, v.length);
			ZL.asm.free(b);
		};
		w.onclose = function(evt) { ZLFNWebSocket(impl, 3+evt.code); ws = undefined; };
		ws = w;
	};

	env.ZLJS_StartAudio = function()
	{
		var audioCtx;
		try { audioCtx = new (findAlias(window,'','','AudioContext'))(); } catch (e) { }
		if (!audioCtx) { ZL.print('Warning: WebAudio not supported'); return; }
		var encTime = 0, audioSamples = 882, audioSecs = audioSamples/44100;
		var ptrTempBuf = 0, f32TempBuf = 0, audioBufs = [{'length':0}], audioBufIdx = 0;
		setInterval(function()
		{
			if (audioCtx.state == 'suspended') { audioCtx.resume(); if (audioCtx.state == 'suspended') return; }
			var ctxTime = audioCtx.currentTime;
			if (ctxTime == 0) encTime = 0;
			if (encTime - ctxTime > audioSecs) return;
			if (audioBufs[0].length != audioSamples)
			{
				ZL.asm.free(ptrTempBuf);
				f32TempBuf = ((ptrTempBuf = ZL.asm.malloc(audioSamples<<3))>>2); //2 channels, 4 byte per float sized sample
				for (var i = 0; i != 4; i++) audioBufs[i] = audioCtx.createBuffer(2, audioSamples, 44100);
			}
			if (ZL.asm.ZLFNAudio(ptrTempBuf, audioSamples))
			{
				var soundBuffer = audioBufs[audioBufIdx = ((audioBufIdx + 1) % 4)];
				soundBuffer.getChannelData(0).set(HEAPF32.subarray(f32TempBuf, f32TempBuf + audioSamples));
				soundBuffer.getChannelData(1).set(HEAPF32.subarray(f32TempBuf + audioSamples, f32TempBuf + (audioSamples<<1)));
				var source = audioCtx.createBufferSource();
				source.connect(audioCtx.destination);
				source.buffer = soundBuffer;
				source[source.start ? 'start' : 'noteOn'](0.005+encTime);
			}
			if (ctxTime > encTime && ctxTime > .5)
			{
				//ZL.print('Warning: Audio starved once by ' + (ctxTime - encTime) + ' seconds (ctxTime = ' + ctxTime + ', encTime = ' + encTime + ', document.hasFocus = ' + document.hasFocus() + ')');
				if (ctxTime - encTime < audioSecs * 10 && audioSamples < 11025 && document.hasFocus())
				{
					//only increase buffer when at least some time has passed (not directly after loading) and it's not a giant hickup
					audioSecs = (audioSamples += 441)/44100;
					ZL.print('Warning: Audio callback had starved sending audio by ' + (ctxTime - encTime) + ' seconds. (extending samples to: ' + audioSamples + ')');
				}
				encTime = ctxTime + (document.hasFocus() ? 0 : 1.5);
			}
			encTime += audioSecs;
		}, 10);
	};

	env.ZLJS_OpenExternalUrl = function(url)
	{
		url = Pointer_stringify(url);
		if (ZL.openUrl) ZL.openUrl(url);
		else window.open(url, '_newtab');
	};

	env.ZLJS_SettingsInit = function(prefix)
	{
		settings_prefix = (ZL.settingsPrefix ? ZL.settingsPrefix : Pointer_stringify(prefix));
	};

	env.ZLJS_SettingsSet = function(key, val)
	{
		ls[settings_prefix +' '+Pointer_stringify(key)] = Pointer_stringify(val);
	};

	env.ZLJS_SettingsDel = function(key)
	{
		delete ls[settings_prefix +' '+Pointer_stringify(key)];
	};

	env.ZLJS_SettingsHas = function(key)
	{
		return (ls[settings_prefix +' '+Pointer_stringify(key)] !== undefined);
	};

	env.ZLJS_SettingsGetMalloc = function(key)
	{
		key = Pointer_stringify(key);
		if (ls[settings_prefix +' '+key] === undefined) { return 0; }
		return malloc_string(ls[settings_prefix +' '+key]);
	};
}

var env =
{
	sbrk: _sbrk,
	time: function(ptr) { var ret = (Date.now()/1000)|0; if (ptr) HEAPU32[ptr>>2] = ret; return ret; },
	gettimeofday: function(ptr) { var now = Date.now(); HEAPU32[ptr>>2]=(now/1000)|0; HEAPU32[(ptr+4)>>2]=((now % 1000)*1000)|0; },
	__assert_fail:  function(condition, filename, line, func) { abort('CRASH', 'Assert ' + Pointer_stringify(condition) + ', at: ' + (filename ? Pointer_stringify(filename) : 'unknown filename'), line, (func ? Pointer_stringify(func) : 'unknown function')); },
	__cxa_uncaught_exception: function() { abort('CRASH', 'Uncaught exception!'); },
	__cxa_pure_virtual: function() { abort('CRASH', 'pure virtual'); },
	abort: function() { abort('CRASH', 'Abort called'); },
	longjmp: function() { abort('CRASH', 'Unsupported longjmp called'); },
}, wasi = {};
env.setjmp = env.__cxa_atexit = env.__lock = env.__unlock = function() {};
env.ceil  = env.ceilf  = Math.ceil;
env.exp  = env.expf  = Math.exp;
env.floor  = env.floorf  = Math.floor;
env.log  = env.logf  = Math.log;
env.pow  = env.powf  = Math.pow;
env.cos  = env.cosf  = Math.cos;
env.sin  = env.sinf  = Math.sin;
env.tan  = env.tanf  = Math.tan;
env.acos = env.acosf = Math.acos;
env.asin = env.asinf = Math.asin;
env.sqrt = env.sqrtf = Math.sqrt;
env.atan = env.atanf = Math.atan;
env.atan2 = env.atan2f = Math.atan2;
env.fabs = env.fabsf = env.abs = env.fabsl = Math.abs;
env.round = env.roundf = env.rint = env.rintf = Math.round;

ZLJS_WASM_IMPORTS(env);
GL_WASM_IMPORTS(env);
SYSCALLS_WASM_IMPORTS(env, wasi);

if (!ZL.wasm) abort('BOOT', 'Missing Wasm data');
var wasmBytes = Base64Decode(ZL.wasm);delete ZL.wasm;
//console.log('WASM BINARY LENGTH: ' + wasmBytes.length);
var wasmDataEnd, wasmStackTop, wasmHeapBase;
for (let i = 8, sectionEnd, type, length; i < wasmBytes.length; i = sectionEnd)
{
	function Get() { return wasmBytes[i++]; }
	function GetLEB() { for (var s=i,r=0,n=128; n&128; i++) r|=((n=wasmBytes[i])&127)<<((i-s)*7); return r; }
	type = GetLEB(), length = GetLEB(), sectionEnd = i + length;
	if (type < 0 || type > 11 || length <= 0 || sectionEnd > wasmBytes.length) break;
	//console.log('WASM SECTION TYPE: ' + type + ' - LENGTH: ' + length + ' - DATA START: ' + i);
	if (type == 6) //globals
	{
		let count = GetLEB(), gtype = Get(), mutable = Get(), opcode = GetLEB(), offset = GetLEB(), endcode = GetLEB();
		//console.log('    GLOBAL [' + 0 + '/' + 1 + '] gtype: ' + gtype + ' - mutable: ' + mutable + ' - opcode: ' + opcode + ' - offset: ' + offset + ' - endcode: ' + endcode);
		wasmHeapBase = offset;
		}
	if (type == 11) //data
	{
		for (let count = GetLEB(), j = 0; j != count && i < sectionEnd; j++)
		{
			let dindex = Get(), dopcode = GetLEB(), doffset = GetLEB(), dendcode = GetLEB(), dsize = GetLEB();
			//console.log('    DATA [' + j + '/' + count + '] RANGE: ' + doffset + ' ~ ' + (doffset + dsize) + ' (SIZE: ' + dsize + ')');
			wasmDataEnd = (doffset + dsize);
			wasmStackTop = (wasmDataEnd+15)>>4<<4;
			i += dsize;
		}
	}
}

//console.log('[WASM] wasmDataEnd: ' + wasmDataEnd + ' - wasmHeapBase: ' + wasmHeapBase);
//console.log('[WASM] STATIC DATA: [' + 0 + ' ~ ' + wasmDataEnd + '] - STACK: [' + wasmStackTop + ' ~ ' + (wasmHeapBase-1) + '] - HEAP: [' + wasmHeapBase + ' ~ ...]');
if (wasmDataEnd <= 0 || wasmHeapBase <= wasmStackTop) abort('BOOT', 'Invalid memory layout (' + wasmDataEnd + '/' + wasmStackTop + '/' + wasmHeapBase + ')');

var wasmMemInitial = 262144+((wasmHeapBase+65535)>>16<<16); //data + stack + 256kb
WASM_HEAP = wasmHeapBase;
WASM_MEMORY = env.memory = new WebAssembly.Memory({initial: wasmMemInitial>>16, maximum: WASM_HEAP_MAX>>16 });
updateGlobalBufferViews();

// Instantiate the wasm module by passing the prepared env and wasi objects containing import functions for the wasm module
WebAssembly.instantiate(wasmBytes, {env:env,wasi_unstable:wasi,wasi_snapshot_preview1:wasi,wasi:wasi}).then(function (output)
{
	// Store the list of the functions exported by the wasm module in ZL.asm
	ZL.asm = output.instance.exports;
	// Store the argument list with 1 entry at the far end of the stack to pass to main
	var argc = 1, argv = wasmStackTop, exe = 'zl';
	stringToUTF8Array(exe, HEAP8, (HEAPU32[(argv+0)>>2] = (argv+8)), 256);
	HEAPU32[(argv+4) >> 2] = 0;

	ZL.asm.__wasm_call_ctors();
	ZL.asm.main(argc, argv, 0);
	ZL.started();
})
.catch(function (err)
{
	// On an exception, if the err is 'abort' the error was already processed in the abort function above
	if (err !== 'abort') abort('BOOT', 'WASM instiantate error: ' + err + (err.stack ? "\n" + err.stack : ''));
});

})();
