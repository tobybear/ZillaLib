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

var LibraryZLJS =
{
	$ZLJS:
	{
		initTime: null,
		settings_prefix: 'ZL',
		fromutf8: function(v)
		{
			var s = '';
			for (var i = 0, sz = 1, len = v.length; i < len && v[i]; i+=sz)
				s += String.fromCharCode(v[i] < 0xc2 ? (sz=1,v[i]) :
					(v[i] > 0xdf && v[i] < 0xf0 ? (sz=(v[i+1] && v[i+2] ? 3 : 1),((v[i]&0x1f)<<12) + ((v[i+1]&0x7f)<<6) + (v[i+2]&0x7f)) :
					(v[i] > 0xc1 && v[i] < 0xe0 ? (sz=(v[i+1] ? 2 : 1),((v[i]&0x3f)<<6) + (v[i+1]&0x7f)) : (sz=4,0))));
			return s;
		},
	},

	ZLJS_CreateWindow__deps: ['$Browser', '$ZLJS'],
	ZLJS_CreateWindow: function(width, height, tpf_limit)
	{
		if (Module['requestWidth'])
		{
			height = (Module['requestHeight'] ? Module['requestHeight'] : Module['requestWidth']*height/width);
			width = Module['requestWidth'];
		}
		else if (Module['requestHeight'])
		{
			width = Module['requestHeight']*width/height;
			height = Module['requestHeight'];
		}

		var cnvs = Module['canvas'];
		cnvs.width = width;
		cnvs.height = height;
		cnvs.height = cnvs['clientHeight'];
		cnvs.width = cnvs['clientWidth'];
		Module.ctx = Browser.createContext(cnvs, true, true);
		if (!Module.ctx) return;

  		var mousfocs = true, pointerlock = null, fullscreen = null, mouse_lastx = -1, mouse_lasty = -1;
		var cancelEvent = function(e) { if (e.preventDefault) e.preventDefault(true); else if (e.stopPropagation) e.stopPropagation(true); else e.stopped = true; };
		var mx = function(e) { return (e['offsetX'] !== undefined ? e['offsetX'] : (e['layerX'] !== undefined ? e['layerX'] : e['pageX']) - (fullscreen ? 0 : cnvs['offsetLeft'])) * cnvs.width / cnvs['clientWidth']; };
		var my = function(e) { return (e['offsetY'] !== undefined ? e['offsetY'] : (e['layerY'] !== undefined ? e['layerY'] : e['pageY']) - (fullscreen ? 0 : cnvs['offsetTop'])) * cnvs.height / cnvs['clientHeight']; };
		window.addEventListener('keydown', function(e)
		{
			_ZLFNKey(true, e['keyCode'], e['shiftKey'], e['ctrlKey'], e['altKey'], e['metaKey']);
			var f;
			if (e['char'] !== undefined) { if (!e['char']) cancelEvent(e); }
			else if (!e['keyIdentifier'] || (!(f = e['keyIdentifier'].match(/^U\+(\S+)$/))) || Math.floor("0x"+f[1]) < 32) cancelEvent(e);
		}, true);
		window.addEventListener('keyup', function(e)
		{
			_ZLFNKey(false, e['keyCode'], e['shiftKey'], e['ctrlKey'], e['altKey'], e['metaKey']);
			cancelEvent(e);
		}, true);
		window.addEventListener('keypress', function(e)
		{
			var chr = e['charCode'] || (e['char'] && e['char'].charCodeAt(0));
			if (chr >= 32) _ZLFNText(chr);
			if (e['char'] == "\b")
			{
				//internet explorer super special handling which only works sometimes
				_ZLFNKey(true, 8, e['shiftKey'], e['ctrlKey'], e['altKey'], e['metaKey']);
				_ZLFNKey(false, 8, e['shiftKey'], e['ctrlKey'], e['altKey'], e['metaKey']);
			}
			cancelEvent(e);
		}, true);
		cnvs.addEventListener('mousemove', function(e)
		{
			if (pointerlock)
			{
				if (mouse_lastx == -1) mouse_lastx = mouse_lasty = 200;
				var relx = (e['movementX']||e['mozMovementX']||e['webkitMovementX']||0), rely = (e['movementY']||e['mozMovementY']||e['webkitMovementY']||0);
				if (relx || rely)
				{
					mouse_lastx += relx;
					mouse_lasty += rely;
					if      (mouse_lastx <             0) mouse_lastx = 0;
					else if (mouse_lastx >  cnvs.width-1) mouse_lastx = cnvs.width-1;
					if      (mouse_lasty <             0) mouse_lasty = 0;
					else if (mouse_lasty > cnvs.height-1) mouse_lasty = cnvs.height-1;
					_ZLFNMove(mouse_lastx, mouse_lasty, relx, rely);
				}
			}
			else
			{
				var x = mx(e), y = my(e);
				if (mouse_lastx == -1 || x != mouse_lastx || y != mouse_lasty)
				{
					if (mouse_lastx == -1) { mouse_lastx = x; mouse_lasty = y; }
					_ZLFNMove(x, y, x-mouse_lastx, y-mouse_lasty);
					mouse_lastx = x; mouse_lasty = y;
				}
			}
			if (mousfocs) cancelEvent(e);
		}, true);
		cnvs.addEventListener('mousedown',      function(e) { _ZLFNMouse(e.button,  true, (pointerlock ? mouse_lastx : mx(e)), (pointerlock ? mouse_lasty : my(e))); if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('mouseup',        function(e) { _ZLFNMouse(e.button, false, (pointerlock ? mouse_lastx : mx(e)), (pointerlock ? mouse_lasty : my(e))); if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('mousewheel',     function(e) { _ZLFNWheel(e.wheelDelta); if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('DOMMouseScroll', function(e) { _ZLFNWheel(-e.detail*40); if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('mouseover',      function(e) { if (pointerlock||fullscreen) return; mousfocs = true;  _ZLFNWindow(1,0,0); }, true);
		cnvs.addEventListener('mouseout',       function(e) { if (pointerlock||fullscreen) return; mousfocs = false; mouse_lastx = mouse_lasty = -1; _ZLFNWindow(2,0,0); }, true);
		window.addEventListener('focus',        function(e) { if (e.target == window) _ZLFNWindow(3,0,0); }, true);
		window.addEventListener('blur',         function(e) { if (e.target == window) _ZLFNWindow(4,0,0); }, true);

		window.addEventListener('resize', function(e)
		{
			if (fullscreen || cnvs['clientWidth']<32 || cnvs['clientHeight']<32 || (cnvs.width == cnvs['clientWidth'] && cnvs.height == cnvs['clientHeight'])) return;
			cnvs.height = cnvs['clientHeight'];
			cnvs.width = cnvs['clientWidth'];
			_ZLFNWindow(6, cnvs.width, cnvs.height);
		}, true);

		var onFullScreenChange = function()
		{
			fullscreen = ((document['webkitFullScreenElement'] || document['webkitFullscreenElement'] || document['mozFullScreenElement'] || document['mozFullscreenElement'] || document['fullScreenElement'] || document['fullscreenElement']));
			if (fullscreen) { cnvs.orgStyle = cnvs.style.cssText; cnvs.style.cssText = 'background:black'; cnvs.height = screen["height"]; cnvs.width = screen["width"]; }
			else { if (cnvs.orgStyle) cnvs.style.cssText = cnvs.orgStyle; cnvs.height = cnvs['clientHeight']; cnvs.width = cnvs['clientWidth']; }
			_ZLFNWindow((fullscreen ? 5 : 6), cnvs.width, cnvs.height);
		};
		document.addEventListener('fullscreenchange', onFullScreenChange, false);
		document.addEventListener('mozfullscreenchange', onFullScreenChange, false);
		document.addEventListener('webkitfullscreenchange', onFullScreenChange, false);

		var onPointerLockChange = function()
		{
			pointerlock = (document['pointerLockElement'] || document['mozPointerLockElement'] || document['webkitPointerLockElement']);
			_ZLFNWindow((pointerlock ? 7 : 8),0,0);
		};
		document.addEventListener('pointerlockchange', onPointerLockChange, false);
		document.addEventListener('mozpointerlockchange', onPointerLockChange, false);
		document.addEventListener('webkitpointerlockchange', onPointerLockChange, false);

		ZLJS.initTime = Date.now();

		Module['noExitRuntime'] = true;
		if (tpf_limit > (1000/55))
		{
			Browser.requestAnimationFrame(_ZLFNDraw);
			setInterval(function(){window.requestAnimationFrame(_ZLFNDraw)}, tpf_limit);
		}
		else
		{
			var draw_func_ex = function() { window.requestAnimationFrame(draw_func_ex); _ZLFNDraw(); }
			Browser.requestAnimationFrame(draw_func_ex);
		}
	},

	ZLJS_GetWidth: function(type) { return Module['canvas'].width; },
	ZLJS_GetHeight: function(type) { return Module['canvas'].height; },

	ZLJS_GetTime__deps: ['$ZLJS'],
	ZLJS_GetTime: function(type) { return Date.now() - ZLJS.initTime; },

	ZLJS_SetFullscreen: function(flag)
	{
		var el = (flag ? Module['canvas'] : document);
		var fs = null;
		if (flag) fs = el['requestFullscreen'] || el['mozRequestFullscreen'] || el['mozRequestFullScreen'] || (el['webkitRequestFullScreen'] ? function() { el['webkitRequestFullScreen'](Element['ALLOW_KEYBOARD_INPUT']) } : null);
		else fs = el['exitFullscreen'] || el['cancelFullScreen'] || el['mozCancelFullScreen'] || el['webkitCancelFullScreen'];
		if (fs) fs.apply(el, []);
	},

	ZLJS_SetPointerLock: function(flag)
	{
		var el = (flag ? Module['canvas'] : document);
		var fs = null;
		if (flag) fs = el['requestPointerLock'] || el['mozRequestPointerLock'] || el['webkitRequestPointerLock']; 
		else fs = el['exitPointerLock'] || el['mozExitPointerLock'] || el['webkitExitPointerLock'];
		if (fs) fs.apply(el, []);
	},

	ZLJS_AsyncLoad: function(url, impl, postdata, postlength)
	{
		url = Pointer_stringify(url);
		var xhr = new XMLHttpRequest();
		xhr.open((postlength ? 'POST' : 'GET'), url, true);
		xhr.responseType = 'arraybuffer';
		xhr.onload = function()
		{
			if (xhr.status == 200)
			{
				_ZLFNHTTPLoad(impl, allocate(new Uint8Array(xhr.response), 'i8', ALLOC_STACK), xhr.response.byteLength);
			}
			else
			{
				_ZLFNHTTPError(impl, xhr.status);
			}
		};
		xhr.onerror = function(event)
		{
			_ZLFNHTTPError(impl, xhr.status);
		};
		if (postlength) try { xhr.send(HEAPU8.subarray(postdata, postdata+postlength)); } catch (e) { xhr.send(HEAPU8.buffer.slice(postdata, postdata+postlength)); }
		else xhr.send(null);
	},

	ZLJS_StartAudio: function()
	{
		var audiobuffersize = 10240<<2; //2 channels, 2 byte per short sized sample
		var audiobuffer = _malloc(audiobuffersize);
		var audioCtx;
		try { audioCtx = new (window['AudioContext'] || window['webkitAudioContext'])(); } catch (e) { return; }
		var canMutateBufferSource = (function(){try { audioCtx['createBufferSource']()['buffer'] = null; } catch (e) { return true; } return false; })();
		if (canMutateBufferSource)
		{
			var outputBuffer = audioCtx['createBuffer'](2, 0x1000, 44100);
			var outputSource = audioCtx['createBufferSource']();
			outputSource['connect'](audioCtx['destination']);
			outputSource['buffer'] = outputBuffer;
			outputSource['loop'] = true;
			outputSource[outputSource['start'] ? 'start' : 'noteOn']();
			var audioOutBufL = outputBuffer['getChannelData'](0), audioOutBufR = outputBuffer['getChannelData'](1);
			var audioPrevSample = 0;
			setInterval(function()
			{
				var cursample = (audioCtx.currentTime * 44100)|0;
				var numnewbytes = (cursample - audioPrevSample)<<2;
				if (!numnewbytes) return;
				if (numnewbytes > audiobuffersize) { numnewbytes = audiobuffersize; }
				audioPrevSample = cursample;
				_ZLFNAudio(audiobuffer, numnewbytes);
				var sample_end = (cursample & 0x0fff);
				var sample_start = sample_end - (numnewbytes>>2);
				var i, ptr = audiobuffer >> 1;
				if (sample_start < 0)
				{
					for (i = 0x1000+sample_start; i < 0x1000; ++i) { audioOutBufL[i] = HEAP16[ptr++] / 32768.0; audioOutBufR[i] = HEAP16[ptr++] / 32768.0; }
					sample_start = 0;
				}
				for (i = sample_start; i < sample_end; ++i) { audioOutBufL[i] = HEAP16[ptr++] / 32768.0; audioOutBufR[i] = HEAP16[ptr++] / 32768.0; }
			}, 5);
		}
		else
		{
			var audioCallSampleCount = 512;
			var audioNextPlayTime = 0;
			var audioCaller = function()
			{
				var curtime = audioCtx['currentTime'];
				var numnewbytes = audioCallSampleCount << 2;
				_ZLFNAudio(audiobuffer, numnewbytes);
				var soundBuffer = audioCtx['createBuffer'](2, audioCallSampleCount, 44100);
				var bufl = soundBuffer['getChannelData'](0), bufr = soundBuffer['getChannelData'](1);
				for (var ptr = audiobuffer >> 1, i = 0; i < audioCallSampleCount; ++i) { bufl[i] = HEAP16[ptr++] / 32768.0; bufr[i] = HEAP16[ptr++] / 32768.0; }
				
				var playtime = audioNextPlayTime / 44100;
				var source = audioCtx['createBufferSource']();
				source['connect'](audioCtx['destination']);
				source['buffer'] = soundBuffer;
				source[source['start'] ? 'start' : 'noteOn'](0.005+playtime);
				audioNextPlayTime += audioCallSampleCount;

				if (curtime > playtime && playtime && audioCallSampleCount < 10240)
				{
					audioCallSampleCount += 256;
  					Module['print']('warning: Audio callback had starved sending audio by ' + (curtime - audioNextPlayTime) + ' seconds. (extending samples to: ' + audioCallSampleCount + ')');
				}

				var nexttimeMs = ((playtime - curtime - (audioCallSampleCount / 44100.0)) * 1000.0);
				if (nexttimeMs < 4) audioCaller();
				else setTimeout(audioCaller, nexttimeMs);
			};
			setTimeout(audioCaller, 1);
		}
	},

	ZLJS_OpenExternalUrl__deps: ['$ZLJS'],
	ZLJS_OpenExternalUrl: function(url, urllen)
	{
		url = ZLJS.fromutf8(HEAPU8.subarray(url, url+urllen));
		if (Module['openUrl']) Module['openUrl'](url);
		else window.open(url, '_newtab');
	},

	ZLJS_SettingsInit__deps: ['$ZLJS'],
	ZLJS_SettingsInit: function(prefix, prefixlen)
	{
		ZLJS.settings_prefix = (Module['settingsPrefix'] ? Module['settingsPrefix'] : ZLJS.fromutf8(HEAPU8.subarray(prefix, prefix+prefixlen)));
	},

	ZLJS_SettingsSet__deps: ['$ZLJS'],
	ZLJS_SettingsSet: function(key, keylen, val, vallen)
	{
		key = ZLJS.fromutf8(HEAPU8.subarray(key, key+keylen));
		val = ZLJS.fromutf8(HEAPU8.subarray(val, val+vallen));
		if (localStorage) localStorage[ZLJS.settings_prefix +' '+key] = val;
	},

	ZLJS_SettingsDel__deps: ['$ZLJS'],
	ZLJS_SettingsDel: function(key, keylen)
	{
		key = ZLJS.fromutf8(HEAPU8.subarray(key, key+keylen));
		if (localStorage) localStorage.removeItem(ZLJS.settings_prefix +' '+key);
	},

	ZLJS_SettingsHas__deps: ['$ZLJS'],
	ZLJS_SettingsHas: function(key, keylen)
	{
		key = ZLJS.fromutf8(HEAPU8.subarray(key, key+keylen));
		return (localStorage && localStorage[ZLJS.settings_prefix +' '+key] !== undefined);
	},

	ZLJS_SettingsGetMalloc__deps: ['$ZLJS'],
	ZLJS_SettingsGetMalloc: function(key, keylen)
	{
		key = ZLJS.fromutf8(HEAPU8.subarray(key, key+keylen));
		if (!localStorage || localStorage[ZLJS.settings_prefix +' '+key] === undefined) { return 0; }
		var i, s = unescape(encodeURIComponent(localStorage[ZLJS.settings_prefix +' '+key]));
		var buf = _malloc(s.length+1);
		for (i = 0; i < s.length; ++i) HEAP8[buf+i] = (s.charCodeAt(i) & 0xFF);
		HEAP8[buf+i] = 0;
		return buf;
	},
};

autoAddDeps(LibraryZLJS, '$ZLJS');
mergeInto(LibraryManager.library, LibraryZLJS);
