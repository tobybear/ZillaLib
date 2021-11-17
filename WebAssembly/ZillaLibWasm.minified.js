!function(){'use strict';function e(e,t){throw x=!0,ZL.error(e,t),'abort'}function t(){var e=m.buffer;f=new Int8Array(e),new Int16Array(e),l=new Int32Array(e),u=new Uint8Array(e),c=new Uint16Array(e),s=new Uint32Array(e),g=new Float32Array(e)}function n(e){var t,n;for(e=unescape(encodeURIComponent(e)),n=ZL.asm.malloc(e.length+1),t=0;t<e.length;++t)f[n+t]=255&e.charCodeAt(t);return f[n+t]=0,n}function r(e){var t=ZL.asm.malloc(Math.max(e.length,1));return e.subarray||e.slice?u.set(e,t):u.set(new Uint8Array(e),t),t}function a(e,t,n,r){var a,o,i,f,u,c,l;if(0>=r)return 0;for(a=e,o=t,f=i=n,u=i+r-1,c=0;c<a.length;++c)if((l=a.charCodeAt(c))>=55296&&57343>=l&&(l=65536+((1023&l)<<10)|1023&a.charCodeAt(++c)),l>127)if(l>2047)if(l>65535)if(l>2097151)if(l>67108863){if(i+5>=u)break;o[i++]=252|l>>30,o[i++]=128|l>>24&63,o[i++]=128|l>>18&63,o[i++]=128|l>>12&63,o[i++]=128|l>>6&63,o[i++]=128|63&l}else{if(i+4>=u)break;o[i++]=248|l>>24,o[i++]=128|l>>18&63,o[i++]=128|l>>12&63,o[i++]=128|l>>6&63,o[i++]=128|63&l}else{if(i+3>=u)break;o[i++]=240|l>>18,o[i++]=128|l>>12&63,o[i++]=128|l>>6&63,o[i++]=128|63&l}else{if(i+2>=u)break;o[i++]=224|l>>12,o[i++]=128|l>>6&63,o[i++]=128|63&l}else{if(i+1>=u)break;o[i++]=192|l>>6,o[i++]=128|63&l}else{if(i>=u)break;o[i++]=l}return o[i]=0,i-f}function o(e,t){var n,r,a,o,i,f,c,l,s,g,m,d,h,L;if(0===t||!e)return'';for(n=0,a=0;n|=r=u[e+a>>0],(0!=r||t)&&(a++,!t||a!=t););if(t||(t=a),128&n)for(o=u,i=e,f=e+t,c=String.fromCharCode,h='';;){if(i==f||!(l=o[i++]))return h;128&l?(s=63&o[i++],192!=(224&l)?(a=63&o[i++],224==(240&l)?l=(15&l)<<12|s<<6|a:(g=63&o[i++],240==(248&l)?l=(7&l)<<18|s<<12|a<<6|g:(m=63&o[i++],l=248==(252&l)?(3&l)<<24|s<<18|a<<12|g<<6|m:(1&l)<<30|s<<24|a<<18|g<<12|m<<6|(r=63&o[i++]))),h+=65536>l?c(l):c(55296|(d=l-65536)>>10,56320|1023&d)):h+=c((31&l)<<6|s)):h+=c(l)}for(L='';t>0;e+=1024,t-=1024)L+=String.fromCharCode.apply(String,u.subarray(e,e+Math.min(t,1024)));return L}function i(e){var t,n,r,a,o,i,f,u=new Uint8Array(128),c=function(n){return u[e.charCodeAt(t+n)]};for(t=0;64>t;t++)u['ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'.charCodeAt(t)]=t;for(u[45]=62,u[95]=63,n=e.length,r='='===e[n-2]?2:'='===e[n-1]?1:0,a=new Uint8Array(3*n/4-r),o=0,i=r>0?n-4:n,f=0,t=0;i>t;t+=4)f=c(0)<<18|c(1)<<12|c(2)<<6|c(3),a[o++]=f>>16&255,a[o++]=f>>8&255,a[o++]=255&f;return 1===r?(f=c(0)<<10|c(1)<<4|c(2)>>2,a[o]=f>>8&255,a[o+1]=255&f):2===r&&(a[o]=255&(c(0)<<2|c(1)>>4)),a}var f,u,c,l,s,g,m,d,h,L,b,v,w,_,Z,y,x=!1;ZL.print=ZL.print||function(e){console.log(e)},ZL.error=ZL.error||function(e,t){ZL.print('[ERROR] '+e+': '+t)},ZL.started=ZL.started||function(){},b={},(L={sbrk:function(n){var r=d,a=r+n,o=a-m.buffer.byteLength;return a>268435456&&e('MEM','Out of memory'),o>0&&(m.grow(o+65535>>16),t()),d=a,0|r},time:function(e){var t=Date.now()/1e3|0;return e&&(s[e>>2]=t),t},gettimeofday:function(e){var t=Date.now();s[e>>2]=t/1e3|0,s[e+4>>2]=t%1e3*1e3|0},__assert_fail:function(t,n,r,a){e('CRASH','Assert '+o(t)+', at: '+(n?o(n):'unknown filename'),a&&o(a))},__cxa_uncaught_exception:function(){e('CRASH','Uncaught exception!')},__cxa_pure_virtual:function(){e('CRASH','pure virtual')},abort:function(){e('CRASH','Abort called')},longjmp:function(){e('CRASH','Unsupported longjmp called')}}).setjmp=L.__cxa_atexit=L.__lock=L.__unlock=function(){},L.ceil=L.ceilf=Math.ceil,L.exp=L.expf=Math.exp,L.floor=L.floorf=Math.floor,L.log=L.logf=Math.log,L.pow=L.powf=Math.pow,L.cos=L.cosf=Math.cos,L.sin=L.sinf=Math.sin,L.tan=L.tanf=Math.tan,L.acos=L.acosf=Math.acos,L.asin=L.asinf=Math.asin,L.sqrt=L.sqrtf=Math.sqrt,L.atan=L.atanf=Math.atan,L.atan2=L.atan2f=Math.atan2,L.fabs=L.fabsf=L.abs=L.fabsl=Math.abs,L.round=L.roundf=L.rint=L.rintf=Math.round,function(e){function t(e,t,n,r){return e[t+r]||e['moz'+n+r]||e['webkit'+n+r]||e['ms'+n+r]}function a(e){l=e;var n=e?ZL.canvas:document,r=e?t(n,'r','R','equestPointerLock'):t(n,'e','E','xitPointerLock');r&&r.call(n)}var i,f,c='ZL',l=0,s={};try{s=window.localStorage||{}}catch(e){}e.ZLJS_CreateWindow=function(e,n,r){var o,f,u,c,s,g,m,d,L,b,v,w,_,Z,y,p;ZL.requestWidth?(n=ZL.requestHeight?ZL.requestHeight:ZL.requestWidth*n/e,e=ZL.requestWidth):ZL.requestHeight&&(e=ZL.requestHeight*e/n,n=ZL.requestHeight),o=ZL.canvas,(f=function(){o.width=e,o.height=n,o.height=o.clientHeight,o.width=o.clientWidth})(),h(o)&&(u=!0,c=null,s=null,g=function(e){e.preventDefault?e.preventDefault(!0):e.stopPropagation?e.stopPropagation(!0):e.stopped=!0},m=function(e){u&&g(e)},d=function(e){return(void 0!==e.offsetX?e.offsetX:e.clientX-(s?0:o.getBoundingClientRect().left))*o.width/o.clientWidth},L=function(e){return(void 0!==e.offsetY?e.offsetY:e.clientY-(s?0:o.getBoundingClientRect().top))*o.height/o.clientHeight},b=function(e,t,n){n('moz'+e,t,1),n('webkit'+e,t,1),n('ms'+e,t,1)},w=function(e,t){o.addEventListener(e,t,{capture:!0,passive:!1})},_=function(e,t,n){document.addEventListener(e,t),n||b(e,t,_)},(v=function(e,t){window.addEventListener(e,t,!0)})('keydown',function(e){var t;ZL.asm.ZLFNKey(!0,e.keyCode,e.shiftKey,e.ctrlKey,e.altKey,e.metaKey),void 0!==e.char?e.char||g(e):e.keyIdentifier&&(t=e.keyIdentifier.match(/^U\+(\S+)$/))&&('0x'+t[1]|0)>=32||g(e)}),v('keyup',function(e){ZL.asm.ZLFNKey(!1,e.keyCode,e.shiftKey,e.ctrlKey,e.altKey,e.metaKey),g(e)}),v('keypress',function(e){var t=e.charCode||e.char&&e.char.charCodeAt(0);32>t||ZL.asm.ZLFNText(t),g(e)}),w('mousemove',function(e){c?ZL.asm.ZLFNMove(0,0,0|e.movementX,0|e.movementY,1):ZL.asm.ZLFNMove(d(e),L(e),0,0,1),m(e)}),Z=function(e,t){ZL.asm.ZLFNMouse(e.button,t,d(e),L(e),1),m(e),l&&!c&&a(1)},y=function(e,t){for(var n=0,r=e.changedTouches;n!=r.length;n++)ZL.asm.ZLFNMouse(0,t,d(r[n]),L(r[n]),9+r[n].identifier);m(e)},w('mousedown',function(e){Z(e,!0)}),w('mouseup',function(e){Z(e,!1)}),w('mousewheel',function(e){ZL.asm.ZLFNWheel(e.wheelDelta),m(e)}),w('DOMMouseScroll',function(e){ZL.asm.ZLFNWheel(40*-e.detail),m(e)}),w('mouseover',function(){c||s||(u=!0,ZL.asm.ZLFNWindow(1,0,0))}),w('mouseout',function(){c||s||(u=!1,ZL.asm.ZLFNWindow(2,0,0))}),w('touchstart',function(e){y(e,!0)}),w('touchmove',function(e){for(var t=0,n=e.changedTouches;t!=n.length;t++)ZL.asm.ZLFNMove(d(n[t]),L(n[t]),0,0,9+n[t].identifier);m(e)}),w('touchend',function(e){y(e,!1)}),w('touchcancel',function(e){y(e,!1)}),v('focus',function(e){e.target==window&&ZL.asm.ZLFNWindow(3,0,0)}),v('blur',function(e){e.target==window&&ZL.asm.ZLFNWindow(4,0,0)}),v('resize',function(){32>o.clientWidth||32>o.clientHeight||o.width==o.clientWidth&&o.height==o.clientHeight||(o.height=o.clientHeight,o.width=o.clientWidth,ZL.asm.ZLFNWindow(6,o.width,o.height))}),_('fullscreenchange',function(){(s=t(document,'f','F','ullscreenElement')||t(document,'f','F','ullScreenElement'))?(o.orgStyle=o.style.cssText,o.style.cssText='background:black',o.height=screen.height,o.width=screen.width):(o.orgStyle&&(o.style.cssText=o.orgStyle),f()),ZL.asm.ZLFNWindow(s?5:6,o.width,o.height)}),_('pointerlockchange',function(){c=t(document,'p','P','ointerLockElement'),ZL.asm.ZLFNWindow(c?7:8,0,0)}),i=Date.now(),r>1e3/55?(window.requestAnimationFrame(ZL.asm.ZLFNDraw),setInterval(function(){window.requestAnimationFrame(ZL.asm.ZLFNDraw)},r)):(p=function(){x||(window.requestAnimationFrame(p),ZL.asm.ZLFNDraw())},window.requestAnimationFrame(p)))},e.ZLJS_GetWidth=function(){return ZL.canvas.width},e.ZLJS_GetHeight=function(){return ZL.canvas.height},e.ZLJS_GetTime=function(){return Date.now()-i},e.ZLJS_SetFullscreen=ZL.SetFullscreen=function(e){var n,r=e?ZL.canvas:document;(n=e?t(r,'r','R','equestFullscreen')||t(r,'r','R','equestFullScreen'):t(r,'e','E','xitFullscreen')||t(r,'c','C','ancelFullScreen'))&&n.call(r)},e.ZLJS_SetPointerLock=function(e){a(e)},e.ZLJS_AsyncLoad=function(e,t,n,a,i){var f=new XMLHttpRequest,c=ZL.asm.ZLFNHTTP;if(f.open(a?'POST':'GET',o(e),!0),f.responseType='arraybuffer',f.timeout=i,f.onload=function(){if(200==f.status){var e=r(new Uint8Array(f.response));c(t,200,e,f.response.byteLength),ZL.asm.free(e)}else c(t,f.status,0,0)},f.ontimeout=f.onerror=function(){setTimeout(function(){c(t,f.status||-1,0,0)})},a)try{f.send(u.subarray(n,n+a))}catch(e){f.send(u.buffer.slice(n,n+a))}else f.send(null)},e.ZLJS_Websocket=function(e,t,a,i){var c,l=ZL.asm.ZLFNWebSocket;if(1!=t)if(2!=t){if(t>=3)return f&&f.close(t-3,i?o(a,i):void 0),void(f=void 0);try{c=new WebSocket(o(a))}catch(t){return void setTimeout(function(){l(e,1009)})}c.binaryType='arraybuffer',c.onopen=function(){l(e,0)},c.onmessage=function(t){var a='string'==typeof t.data,o=a?t.data:new Uint8Array(t.data),i=a?n(o):r(o);l(e,a?1:2,i,o.length),ZL.asm.free(i)},c.onclose=function(t){l(e,3+t.code),f=void 0},f=c}else f&&f.send(u.subarray(a,a+i));else f&&f.send(o(a,i))},e.ZLJS_StartAudio=function(){var e,n,r,a,o,i,f,u;try{e=new(t(window,'','','AudioContext'))}catch(e){}e?(n=0,a=(r=882)/44100,o=0,i=0,f=[{length:0}],u=0,setInterval(function(){var t,c,l,s;if(('suspended'!=e.state||(e.resume(),'suspended'!=e.state))&&(0==(t=e.currentTime)&&(n=0),a>=n-t)){if(f[0].length!=r)for(ZL.asm.free(o),i=(o=ZL.asm.malloc(r<<3))>>2,c=0;4!=c;c++)f[c]=e.createBuffer(2,r,44100);ZL.asm.ZLFNAudio(o,r)&&((l=f[u=(u+1)%4]).getChannelData(0).set(g.subarray(i,i+r)),l.getChannelData(1).set(g.subarray(i+r,i+(r<<1))),(s=e.createBufferSource()).connect(e.destination),s.buffer=l,s[s.start?'start':'noteOn'](.005+n)),t>n&&t>.5&&(10*a>t-n&&11025>r&&document.hasFocus()&&(a=(r+=441)/44100,ZL.print('Warning: Audio callback had starved sending audio by '+(t-n)+' seconds. (extending samples to: '+r+')')),n=t+(document.hasFocus()?0:1.5)),n+=a}},10)):ZL.print('Warning: WebAudio not supported')},e.ZLJS_OpenExternalUrl=function(e){e=o(e),ZL.openUrl?ZL.openUrl(e):window.open(e,'_newtab')},e.ZLJS_SettingsInit=function(e){c=ZL.settingsPrefix?ZL.settingsPrefix:o(e)},e.ZLJS_SettingsSet=function(e,t){s[c+' '+o(e)]=o(t)},e.ZLJS_SettingsDel=function(e){delete s[c+' '+o(e)]},e.ZLJS_SettingsHas=function(e){return void 0!==s[c+' '+o(e)]},e.ZLJS_SettingsGetMalloc=function(e){return e=o(e),void 0===s[c+' '+e]?0:n(s[c+' '+e])}}(L),function(t){function n(e){var t,n=L++;for(t=e.length;n>t;t++)e[t]=null;return n}function r(e){GLlastError||(GLlastError=e)}function i(e,t,n,a,o){var i,f,l,m,d;switch(t){case 6406:case 6409:case 6402:f=1;break;case 6410:f=2;break;case 6407:case 35904:f=3;break;case 6408:case 35906:f=4;break;default:return r(1280),null}switch(e){case 5121:i=1*f;break;case 5123:case 36193:i=2*f;break;case 5125:case 5126:i=4*f;break;case 34042:i=4;break;case 33635:case 32819:case 32820:i=2;break;default:return r(1280),null}switch(l=n*i,d=p,m=a>0?Math.floor((l+d-1)/d)*d*(a-1)+l:0,e){case 5121:return u.subarray(o,o+m);case 5126:return g.subarray(o>>2,o+m>>2);case 5125:case 34042:return s.subarray(o>>2,o+m>>2);case 5123:case 33635:case 32819:case 32820:case 36193:return c.subarray(o>>1,o+m>>1);default:return r(1280),null}}var m,d,L=1,b=[],v=[],w=[],_=[],Z=[],y=[],x={},p=4,A=null,S=[0];for(A=new Float32Array(256),d=0;256>d;d++)S[d]=A.subarray(0,d+1);h=function(t,n){var r,a,o,i,f,u,c,l;n={majorVersion:1,minorVersion:0,antialias:!0,alpha:!1},r='';try{let a=function(e){r=e.statusMessage||r};t.addEventListener('webglcontextcreationerror',a,!1);try{m=t.getContext('webgl',n)||t.getContext('experimental-webgl',n)}finally{t.removeEventListener('webglcontextcreationerror',a,!1)}if(!m)throw'Could not create context'}catch(t){e('WEBGL',t+(r?' ('+r+')':''))}return(a=m.getSupportedExtensions())&&a.length>0&&(l=[(i='OES_')+(u='texture_')+'float',i+u+'half_float',i+'standard_derivatives',i+'vertex_array_object',(o='WEBGL_')+(c='compressed_'+u)+'s3tc',o+'depth_texture',i+'element_index_uint',(f='EXT_')+u+'filter_anisotropic',f+'frag_depth',o+'draw_buffers','ANGLE_instanced_arrays',i+u+'float_linear',i+u+'half_float_linear',f+'blend_minmax',f+'shader_texture_lod',o+c+'pvrtc',f+'color_buffer_half_float',o+'color_buffer_float',f+'sRGB',o+c+'etc1',f+'disjoint_timer_query',o+c+'etc',o+c+'astc',f+'color_buffer_float',o+c+'s3tc_srgb',f+'disjoint_timer_query_webgl2'],a.forEach(function(e){-1!=l.indexOf(e)&&m.getExtension(e)})),!0},t.glActiveTexture=function(e){m.activeTexture(e)},t.glAttachShader=function(e,t){m.attachShader(v[e],y[t])},t.glBindAttribLocation=function(e,t,n){m.bindAttribLocation(v[e],t,o(n))},t.glBindBuffer=function(e,t){m.bindBuffer(e,t?b[t]:null)},t.glBindFramebuffer=function(e,t){m.bindFramebuffer(e,t?w[t]:null)},t.glBindTexture=function(e,t){m.bindTexture(e,t?_[t]:null)},t.glBlendFunc=function(e,t){m.blendFunc(e,t)},t.glBlendFuncSeparate=function(e,t,n,r){m.blendFuncSeparate(e,t,n,r)},t.glBlendColor=function(e,t,n,r){m.blendColor(e,t,n,r)},t.glBlendEquation=function(e){m.blendEquation(e)},t.glBlendEquationSeparate=function(e,t){m.blendEquationSeparate(e,t)},t.glBufferData=function(e,t,n,r){n?m.bufferData(e,u.subarray(n,n+t),r):m.bufferData(e,t,r)},t.glBufferSubData=function(e,t,n,r){m.bufferSubData(e,t,u.subarray(r,r+n))},t.glClear=function(e){m.clear(e)},t.glClearColor=function(e,t,n,r){m.clearColor(e,t,n,r)},t.glColorMask=function(e,t,n,r){m.colorMask(!!e,!!t,!!n,!!r)},t.glCompileShader=function(e){m.compileShader(y[e])},t.glCreateProgram=function(){var e=n(v),t=m.createProgram();return t.name=e,v[e]=t,e},t.glCreateShader=function(e){var t=n(y);return y[t]=m.createShader(e),t},t.glDeleteBuffers=function(e,t){var n,r,a;for(n=0;e>n;n++)r=l[t+4*n>>2],(a=b[r])&&(m.deleteBuffer(a),a.name=0,b[r]=null)},t.glDeleteFramebuffers=function(e,t){var n,r,a;for(n=0;e>n;++n)r=l[t+4*n>>2],(a=w[r])&&(m.deleteFramebuffer(a),a.name=0,w[r]=null)},t.glDeleteProgram=function(e){if(e){var t=v[e];t?(m.deleteProgram(t),t.name=0,v[e]=null,x[e]=null):r(1281)}},t.glDeleteShader=function(e){if(e){var t=y[e];t?(m.deleteShader(t),y[e]=null):r(1281)}},t.glDeleteTextures=function(e,t){var n,r,a;for(n=0;e>n;n++)r=l[t+4*n>>2],(a=_[r])&&(m.deleteTexture(a),a.name=0,_[r]=null)},t.glDepthFunc=function(e){m.depthFunc(e)},t.glDepthMask=function(e){m.depthMask(!!e)},t.glDetachShader=function(e,t){m.detachShader(v[e],y[t])},t.glDisable=function(e){m.disable(e)},t.glDisableVertexAttribArray=function(e){m.disableVertexAttribArray(e)},t.glDrawArrays=function(e,t,n){m.drawArrays(e,t,n)},t.glDrawElements=function(e,t,n,r){m.drawElements(e,t,n,r)},t.glEnable=function(e){m.enable(e)},t.glEnableVertexAttribArray=function(e){m.enableVertexAttribArray(e)},t.glFramebufferTexture2D=function(e,t,n,r,a){m.framebufferTexture2D(e,t,n,_[r],a)},t.glGenBuffers=function(e,t){var a,o,i;for(a=0;e>a;a++){if(!(o=m.createBuffer())){for(r(1282);e>a;)l[t+4*a++>>2]=0;return}i=n(b),o.name=i,b[i]=o,l[t+4*a>>2]=i}},t.glGenFramebuffers=function(e,t){var a,o,i;for(a=0;e>a;++a){if(!(o=m.createFramebuffer())){for(r(1282);e>a;)l[t+4*a++>>2]=0;return}i=n(w),o.name=i,w[i]=o,l[t+4*a>>2]=i}},t.glGenTextures=function(e,t){var a,o,i;for(a=0;e>a;a++){if(!(o=m.createTexture())){for(r(1282);e>a;)l[t+4*a++>>2]=0;return}i=n(_),o.name=i,_[i]=o,l[t+4*a>>2]=i}},t.glGetActiveUniform=function(e,t,n,r,o,i,u){var c,s;e=v[e],(c=m.getActiveUniform(e,t))&&(n>0&&u?(s=a(c.name,f,u,n),r&&(l[r>>2]=s)):r&&(l[r>>2]=0),o&&(l[o>>2]=c.size),i&&(l[i>>2]=c.type))},t.glGetAttribLocation=function(e,t){return e=v[e],t=o(t),m.getAttribLocation(e,t)},t.glGetError=function(){if(GLlastError){var e=GLlastError;return GLlastError=0,e}return m.getError()},t.glGetIntegerv=function(e,t){!function(e,t){var n,a,o;if(t){switch(n=void 0,e){case 36346:n=1;break;case 36344:return;case 36345:n=0;break;case 34466:n=m.getParameter(34467).length}if(void 0===n)switch(typeof(a=m.getParameter(e))){case'number':n=a;break;case'boolean':n=a?1:0;break;case'string':return void r(1280);case'object':if(null===a)switch(e){case 34964:case 35725:case 34965:case 36006:case 36007:case 32873:case 34068:n=0;break;default:return void r(1280)}else{if(a instanceof Float32Array||a instanceof Uint32Array||a instanceof Int32Array||a instanceof Array){for(o=0;o<a.length;++o)l[t+4*o>>2]=a[o];return}if(!(a instanceof WebGLBuffer||a instanceof WebGLProgram||a instanceof WebGLFramebuffer||a instanceof WebGLRenderbuffer||a instanceof WebGLTexture))return void r(1280);n=0|a.name}break;default:return void r(1280)}l[t>>2]=n}else r(1281)}(e,t)},t.glGetProgramInfoLog=function(e,t,n,r){var o,i=m.getProgramInfoLog(v[e]);null===i&&(i='(unknown error)'),t>0&&r?(o=a(i,f,r,t),n&&(l[n>>2]=o)):n&&(l[n>>2]=0)},t.glGetProgramiv=function(e,t,n){var a,o,i,f,u,c,s;if(n)if(L>e)if(a=x[e])if(35716==t)null===(o=m.getProgramInfoLog(v[e]))&&(o='(unknown error)'),l[n>>2]=o.length+1;else if(35719==t)l[n>>2]=a.maxUniformLength;else if(35722==t){if(-1==a.maxAttributeLength)for(e=v[e],i=m.getProgramParameter(e,m.ACTIVE_ATTRIBUTES),a.maxAttributeLength=0,f=0;i>f;++f)u=m.getActiveAttrib(e,f),a.maxAttributeLength=Math.max(a.maxAttributeLength,u.name.length+1);l[n>>2]=a.maxAttributeLength}else if(35381==t){if(-1==a.maxUniformBlockNameLength)for(e=v[e],c=m.getProgramParameter(e,m.ACTIVE_UNIFORM_BLOCKS),a.maxUniformBlockNameLength=0,f=0;c>f;++f)s=m.getActiveUniformBlockName(e,f),a.maxUniformBlockNameLength=Math.max(a.maxUniformBlockNameLength,s.length+1);l[n>>2]=a.maxUniformBlockNameLength}else l[n>>2]=m.getProgramParameter(v[e],t);else r(1282);else r(1281);else r(1281)},t.glGetShaderInfoLog=function(e,t,n,r){var o,i=m.getShaderInfoLog(y[e]);null===i&&(i='(unknown error)'),t>0&&r?(o=a(i,f,r,t),n&&(l[n>>2]=o)):n&&(l[n>>2]=0)},t.glGetShaderiv=function(e,t,n){var a,o,i;n?35716==t?(null===(a=m.getShaderInfoLog(y[e]))&&(a='(unknown error)'),l[n>>2]=a.length+1):35720==t?(i=null===(o=m.getShaderSource(y[e]))||0==o.length?0:o.length+1,l[n>>2]=i):l[n>>2]=m.getShaderParameter(y[e],t):r(1281)},t.glGetUniformLocation=function(e,t){var n,r,a,i,f;if(n=0,-1!==(t=o(t)).indexOf(']',t.length-1)){if(r=t.lastIndexOf('['),(a=t.slice(r+1,-1)).length>0&&0>(n=parseInt(a)))return-1;t=t.slice(0,r)}return(i=x[e])&&(f=i.uniforms[t])&&n<f[0]?f[1]+n:-1},t.glLineWidth=function(e){m.lineWidth(e)},t.glLinkProgram=function(e){m.linkProgram(v[e]),x[e]=null,function(e){var t,r,a,o,i,f,u,c,l,s,g,d=v[e];for(x[e]={uniforms:{},maxUniformLength:0,maxAttributeLength:-1,maxUniformBlockNameLength:-1},r=(t=x[e]).uniforms,a=m.getProgramParameter(d,m.ACTIVE_UNIFORMS),o=0;a>o;++o)if(f=(i=m.getActiveUniform(d,o)).name,t.maxUniformLength=Math.max(t.maxUniformLength,f.length+1),-1!==f.indexOf(']',f.length-1)&&(u=f.lastIndexOf('['),f=f.slice(0,u)),null!=(c=m.getUniformLocation(d,f)))for(l=n(Z),r[f]=[i.size,l],Z[l]=c,s=1;s<i.size;++s)g=f+'['+s+']',c=m.getUniformLocation(d,g),l=n(Z),Z[l]=c}(e)},t.glPixelStorei=function(e,t){3333==e||3317==e&&(p=t),m.pixelStorei(e,t)},t.glReadPixels=function(e,t,n,a,o,f,u){var c=i(f,o,n,a,u);if(!c)return r(1280);m.readPixels(e,t,n,a,o,f,c)},t.glScissor=function(e,t,n,r){m.scissor(e,t,n,r)},t.glShaderSource=function(e,t,n,r){var a=function(e,t,n,r){var a,i,f='';for(a=0;t>a;++a)f+=r?0>(i=l[r+4*a>>2])?o(l[n+4*a>>2]):o(l[n+4*a>>2],i):o(l[n+4*a>>2]);return f}(0,t,n,r);m.shaderSource(y[e],a)},t.glTexImage2D=function(e,t,n,r,a,o,f,u,c){var l=null;c&&(l=i(u,f,r,a,c)),m.texImage2D(e,t,n,r,a,o,f,u,l)},t.glTexParameteri=function(e,t,n){m.texParameteri(e,t,n)},t.glTexSubImage2D=function(e,t,n,r,a,o,f,u,c){var l=null;c&&(l=i(u,f,a,o,c)),m.texSubImage2D(e,t,n,r,a,o,f,u,l)},t.glUniform1f=function(e,t){m.uniform1f(Z[e],t)},t.glUniform1i=function(e,t){m.uniform1i(Z[e],t)},t.glUniform2f=function(e,t,n){m.uniform2f(Z[e],t,n)},t.glUniform3f=function(e,t,n,r){m.uniform3f(Z[e],t,n,r)},t.glUniform3fv=function(e,t,n){var r,a,o;if(3*t>256)r=g.subarray(n>>2,n+12*t>>2);else for(r=S[3*t-1],a=n>>2,o=0;o!=3*t;o++)r[o]=g[a+o];m.uniform3fv(Z[e],r)},t.glUniform4f=function(e,t,n,r,a){m.uniform4f(Z[e],t,n,r,a)},t.glUniformMatrix4fv=function(e,t,n,r){var a,o,i;if((t<<=4)>256)a=g.subarray(r>>2,r+4*t>>2);else for(a=S[t-1],o=r>>2,i=0;i!=t;i+=4)a[i]=g[o+i],a[i+1]=g[o+i+1],a[i+2]=g[o+i+2],a[i+3]=g[o+i+3];m.uniformMatrix4fv(Z[e],!!n,a)},t.glUseProgram=function(e){m.useProgram(e?v[e]:null)},t.glVertexAttrib4f=function(e,t,n,r,a){m.vertexAttrib4f(e,t,n,r,a)},t.glVertexAttrib4fv=function(e,t){m.vertexAttrib4f(e,g[t>>2],g[t+4>>2],g[t+8>>2],g[t+12>>2])},t.glVertexAttribPointer=function(e,t,n,r,a,o){m.vertexAttribPointer(e,t,n,!!r,a,o)},t.glViewport=function(e,t,n,r){m.viewport(e,t,n,r)}}(L),function(e,t){var n=0,r=ZL.files?i(ZL.files):new Uint8Array(0);delete ZL.files,e.__sys_open=e.__syscall_open=e.__syscall5=function(){return n=0,1},t.fd_read=function(e,t,a,o){var i,f,c,g,m=0;for(i=0;a>i;i++){if(f=l[t+8*i>>2],c=l[t+(8*i+4)>>2],g=Math.min(c,r.length-n),u.set(r.subarray(n,n+g),f),n+=g,0>g)return 5;if(m+=g,c>g)break}return s[o>>2]=m,0},t.fd_seek=function(e,t,a,o,i){return 0==o&&(n=t),1==o&&(n+=t),2==o&&(n=r.length-t),0>n&&(n=0),n>r.length&&(n=r.length),s[i+0>>2]=n,s[i+4>>2]=0,0},t.fd_write=function(e,t,n,r){var a,i,f,u=0,c='';if(0==n)return 0;for(a=0;n>a;a++){if(i=l[t+8*a>>2],0>(f=l[t+(8*a+4)>>2]))return-1;u+=f,c+=o(i,f)}return ZL.print(c),s[r>>2]=u,0},t.fd_close=function(){return 0},e.__sys_fcntl64=e.__syscall_fcntl64=e.__sys_ioctl=e.__syscall_ioctl=e.__syscall221=e.__syscall54=function(){return 0}}(L,b),ZL.wasm||e('BOOT','Missing Wasm data'),v=i(ZL.wasm),delete ZL.wasm;for(let e,t,n,r=8;r<v.length;r=e){function p(){return v[r++]}function A(){for(var e=r,t=0,n=128;128&n;r++)t|=(127&(n=v[r]))<<7*(r-e);return t}if(t=A(),n=A(),e=r+n,0>t||t>11||0>=n||e>v.length)break;if(6==t){A(),p(),p(),A();let e=A();A(),Z=e}if(11==t)for(let t=A(),n=0;n!=t&&e>r;n++){p(),A();let e=A(),t=(A(),A());_=(w=e+t)+15>>4<<4,r+=t}}w>0&&Z>_||e('BOOT','Invalid memory layout ('+w+'/'+_+'/'+Z+')'),y=262144+(Z+65535>>16<<16),d=Z,m=L.memory=new WebAssembly.Memory({initial:y>>16,maximum:4096}),t(),WebAssembly.instantiate(v,{env:L,wasi_unstable:b,wasi_snapshot_preview1:b,wasi:b}).then(function(e){ZL.asm=e.instance.exports;var t=_;a('zl',f,s[t+0>>2]=t+8,256),s[t+4>>2]=0,ZL.asm.__wasm_call_ctors(),ZL.asm.main(1,t,0),ZL.started()}).catch(function(t){'abort'!==t&&e('BOOT','WASM instiantate error: '+t+(t.stack?'\n'+t.stack:''))})}();