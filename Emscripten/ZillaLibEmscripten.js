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

var LibraryZLJS =
{
	$ZLJS:
	{
		initTime: null,
		settings_prefix: 'ZL',
		lock_flag: 0,
		do_lock: function(f)
		{
			ZLJS.lock_flag = f;
			var el = (f ? Module['canvas'] : document), rpl = 'requestPointerLock', epl = 'exitPointerLock', moz = 'moz', wk = 'webkit';
			var fs = (f ? (el[rpl] || el[moz+rpl] || el[wk+rpl]) : (el[epl] || el[moz+epl] || el[wk+epl]));
			if (fs) fs.apply(el, []);
		},
		malloc_string: function(s)
		{
			var i, s = unescape(encodeURIComponent(s));
			var buf = _malloc(s.length+1);
			for (i = 0; i < s.length; ++i) HEAP8[buf+i] = (s.charCodeAt(i) & 0xFF);
			HEAP8[buf+i] = 0;
			return buf;
		}
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
		Module.ctx = Browser.createContext(cnvs, true, true, {antialias:1});
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
		cnvs.addEventListener('mousedown',      function(e) { _ZLFNMouse(e.button,  true, (pointerlock ? mouse_lastx : mx(e)), (pointerlock ? mouse_lasty : my(e))); if (mousfocs) cancelEvent(e); if (ZLJS.lock_flag && !pointerlock) ZLJS.do_lock(1); }, true);
		cnvs.addEventListener('mouseup',        function(e) { _ZLFNMouse(e.button, false, (pointerlock ? mouse_lastx : mx(e)), (pointerlock ? mouse_lasty : my(e))); if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('mousewheel',     function(e) { _ZLFNWheel(e.wheelDelta); if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('DOMMouseScroll', function(e) { _ZLFNWheel(-e.detail*40); if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('mouseover',      function(e) { if (pointerlock||fullscreen) return; mousfocs = true;  _ZLFNWindow(1,0,0); }, true);
		cnvs.addEventListener('mouseout',       function(e) { if (pointerlock||fullscreen) return; mousfocs = false; mouse_lastx = mouse_lasty = -1; _ZLFNWindow(2,0,0); }, true);
		cnvs.addEventListener('touchstart',     function(e) { _ZLFNMouse(0, true, mouse_lastx = mx(e.touches[0]), mouse_lasty = my(e.touches[0])); if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('touchmove',      function(e) { var x = mx(e.touches[0]), y = my(e.touches[0]); _ZLFNMove(x, y, x-mouse_lastx, y-mouse_lasty); mouse_lastx = x; mouse_lasty = y; if (mousfocs) cancelEvent(e); }, true);
		cnvs.addEventListener('touchend',       function(e) { _ZLFNMouse(0, false, mouse_lastx, mouse_lasty); if (mousfocs) cancelEvent(e); }, true);
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
			var draw_func_ex = function() { if (ABORT) return; window.requestAnimationFrame(draw_func_ex); _ZLFNDraw(); }
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
		ZLJS.do_lock(flag);
	},

	ZLJS_AsyncLoad: function(url, impl, postdata, postlength)
	{
		var xhr = new XMLHttpRequest();
		xhr.open((postlength ? 'POST' : 'GET'), Pointer_stringify(url), true);
		xhr.responseType = 'arraybuffer';
		xhr.onload = function()
		{
			if (xhr.status == 200)
			{
				var b = allocate(new Uint8Array(xhr.response), 'i8', ALLOC_NORMAL)
				_ZLFNHTTP(impl, 200, b, xhr.response.byteLength);
				_free(b);
			}
			else _ZLFNHTTP(impl, xhr.status, 0, 0);
		};
		xhr.onerror = function(event)
		{
			_ZLFNHTTP(impl, xhr.status, 0, 0);
		};
		if (postlength) try { xhr.send(HEAPU8.subarray(postdata, postdata+postlength)); } catch (e) { xhr.send(HEAPU8.buffer.slice(postdata, postdata+postlength)); }
		else xhr.send(null);
	},

	ZLJS_Websocket__deps: ['$ZLJS'],
	ZLJS_Websocket: function(impl, cmd, param, len)
	{
		if (cmd == 1) { if (ZLJS.ws) ZLJS.ws.send(Pointer_stringify(param,len)); return; }
		if (cmd == 2) { if (ZLJS.ws) ZLJS.ws.send(HEAPU8.subarray(param, param+len)); return; }
		if (cmd >= 3) { ZLJS.ws.close(cmd-3, len?Pointer_stringify(param,len):undefined); ZLJS.ws = undefined; return; }
		var w = new WebSocket(Pointer_stringify(param));
		w.binaryType = 'arraybuffer';
		w.onopen = function() { _ZLFNWebSocket(impl, 1); }
		w.onmessage = function (evt)
		{
			var s = typeof evt.data === 'string', v = s ? evt.data : new Uint8Array(evt.data), b = s ? ZLJS.malloc_string(v) : allocate(v, 'i8', ALLOC_NORMAL);
			_ZLFNWebSocket(impl, s ? 2 : 3, b, v.length);
			_free(b);
		}
		w.onclose = function() { _ZLFNWebSocket(impl, 0); ZLJS.ws = undefined; }
		ZLJS.ws = w;
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
	ZLJS_OpenExternalUrl: function(url)
	{
		url = Pointer_stringify(url);
		if (Module['openUrl']) Module['openUrl'](url);
		else window.open(url, '_newtab');
	},

	ZLJS_SettingsInit__deps: ['$ZLJS'],
	ZLJS_SettingsInit: function(prefix)
	{
		ZLJS.settings_prefix = (Module['settingsPrefix'] ? Module['settingsPrefix'] : Pointer_stringify(prefix));
	},

	ZLJS_SettingsSet__deps: ['$ZLJS'],
	ZLJS_SettingsSet: function(key, val)
	{
		if (localStorage) localStorage[ZLJS.settings_prefix +' '+Pointer_stringify(key)] = Pointer_stringify(val);
	},

	ZLJS_SettingsDel__deps: ['$ZLJS'],
	ZLJS_SettingsDel: function(key)
	{
		if (localStorage) localStorage.removeItem(ZLJS.settings_prefix +' '+Pointer_stringify(key));
	},

	ZLJS_SettingsHas__deps: ['$ZLJS'],
	ZLJS_SettingsHas: function(key)
	{
		return (localStorage && localStorage[ZLJS.settings_prefix +' '+Pointer_stringify(key)] !== undefined);
	},

	ZLJS_SettingsGetMalloc__deps: ['$ZLJS'],
	ZLJS_SettingsGetMalloc: function(key)
	{
		key = Pointer_stringify(key);
		if (!localStorage || localStorage[ZLJS.settings_prefix +' '+key] === undefined) { return 0; }
		return ZLJS.malloc_string(localStorage[ZLJS.settings_prefix +' '+key]);
	},
};

autoAddDeps(LibraryZLJS, '$ZLJS');
mergeInto(LibraryManager.library, LibraryZLJS);
