#
#  ZillaLib
#  Copyright (C) 2010-2018 Bernhard Schelling
#
#  This software is provided 'as-is', without any express or implied
#  warranty.  In no event will the authors be held liable for any damages
#  arising from the use of this software.
#
#  Permission is granted to anyone to use this software for any purpose,
#  including commercial applications, and to alter it and redistribute it
#  freely, subject to the following restrictions:
#
#  1. The origin of this software must not be misrepresented; you must not
#     claim that you wrote the original software. If you use this software
#     in a product, an acknowledgment in the product documentation would be
#     appreciated but is not required.
#  2. Altered source versions must be plainly marked as such, and must not be
#     misrepresented as being the original software.
#  3. This notice may not be removed or altered from any source distribution.
#

import sys,re,os,string,subprocess,fnmatch,SimpleHTTPServer,SocketServer
emscripten_dir = os.path.dirname(os.path.realpath(sys.argv.pop(0))).replace('\\', '/')
def get_makefile_str(param,m='Makefile',d=''): return ((re.findall(param+'\s*=\s*(.*?)\s*(?:\n|\r|$)',open(m).read())or[d])[0] if os.path.exists(m) else d)
def get_makefile_int(param,m='Makefile',d=0): return (int((re.findall(param+'\s*=\s*(\d+)',open(m).read())or[str(d)])[0]) if os.path.exists(m) else d)
cmd = ''
for ln in sys.argv:
	if ln[0]!='-':cmd = sys.argv.pop(sys.argv.index(ln)).upper();break

if cmd == 'RUN' or cmd == 'WEB':
	outdir = 'Debug-emscripten'
	for ln in sys.argv:
		if re.match('-REL', ln, re.I): outdir = 'Release-emscripten';sys.argv.pop(sys.argv.index(ln));break
	if sys.argv and os.path.exists(sys.argv[0]): outdir = sys.argv.pop(0)
	htmlfile = (fnmatch.filter(os.walk(outdir).next()[2], "*.htm*") or [''])[0]
	if sys.argv and os.path.exists(outdir+os.sep+sys.argv[0]): htmlfile = sys.argv.pop(0)

	if cmd == 'WEB':
		HTTPRequestCount = 0
		class OurHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
			def send_header(self, keyword, value):
				if keyword == 'Content-type' and value == 'application/x-gzip': value = 'application/javascript'; self.send_header('Content-Encoding', 'gzip')
				SimpleHTTPServer.SimpleHTTPRequestHandler.send_header(self, keyword, value)
			def translate_path(self, path):
				return outdir+path
			def send_head(self):
				if self.path == '/favicon.ico': self.send_response(200);self.end_headers();return
				global HTTPRequestCount
				HTTPRequestCount += 1
				return SimpleHTTPServer.SimpleHTTPRequestHandler.send_head(self)

		from random import randint
		httpdport = randint(6103,7102)
		httpd = SocketServer.TCPServer(("", httpdport), OurHTTPRequestHandler)
		htmlurl = 'http://localhost:'+str(httpdport)+'/'+htmlfile
	else:
		htmlurl = os.path.abspath(outdir+os.sep+htmlfile)

	browsercmd = get_makefile_str('BROWSER', emscripten_dir+'/ZillaAppLocalConfig.mk');
	if browsercmd == '':
		def tryurl(p): a=1;exec "try:subprocess.call([p, htmlurl])\nexcept:a=0";return a
		try:os.startfile(htmlurl)
		except:(tryurl('xdg-open') or tryurl('gnome-open') or tryurl('exo-open') or tryurl('open'))
	else:
		try:os.spawnv(os.P_NOWAIT, browsercmd, ['"'+browsercmd+'"', htmlurl, '--incognito'])
		except:
			sys.stderr.write("\nCould not run browser with command line '"+browsercmd+"'\nSet custom path in "+emscripten_dir+"/ZillaAppLocalConfig.mk with BROWSER = "+('D:' if sys.platform == 'win32' else '')+"/path/to/browser/browser"+('.exe' if sys.platform == 'win32' else '')+"\n\n")
			sys.exit(1)
	print "Opening",htmlurl,"with",browsercmd

	if cmd == 'WEB':
		MaxRequestCount = (2 if outdir == 'Release-emscripten' and get_makefile_int('ZLEMSCRIPTEN_ASSETS_EMBED') else 3)
		print 'Serving', MaxRequestCount, 'files from directory', outdir, 'at port', httpdport
		try:
			while HTTPRequestCount < MaxRequestCount: httpd.handle_request()
		except:pass
	sys.exit(0)

else:
	print "No command in arguments specified."
	print "Valid commands are: RUN, WEB"
	print "RUN: run browser to output HTML"
	print "WEB: start webserver in output dir run browser to output HTML"
