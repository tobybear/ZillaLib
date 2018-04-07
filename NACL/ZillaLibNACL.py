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
nacl_dir = os.path.dirname(os.path.realpath(sys.argv.pop(0))).replace('\\', '/')
def get_makefile_str(param,m='Makefile',d=''): return ((re.findall(param+'\s*=\s*(.*)\s',open(m).read())or[d])[0] if os.path.exists(m) else d)
def get_makefile_int(param,m='Makefile',d=0): return (int((re.findall(param+'\s*=\s*(\d+)',open(m).read())or[str(d)])[0]) if os.path.exists(m) else d)
cmd = ''
for ln in sys.argv:
	if ln[0]!='-':cmd = sys.argv.pop(sys.argv.index(ln)).upper();break

if cmd == 'RUN' or cmd == 'WEB':
	outdir = 'Debug-nacl'
	for ln in sys.argv:
		if re.match('-REL', ln, re.I): outdir = 'Release-nacl';sys.argv.pop(sys.argv.index(ln));break
	if sys.argv and os.path.exists(sys.argv[0]): outdir = sys.argv.pop(0)
	htmlfile = (fnmatch.filter(os.walk(outdir).next()[2], "*.htm*") or [''])[0]
	if sys.argv and os.path.exists(outdir+os.sep+sys.argv[0]): htmlfile = sys.argv.pop(0)

	HTTPRequestCount = 0
	class OurHTTPRequestHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
		def send_header(self, keyword, value):
			if keyword == 'Content-type' and value == 'application/x-gzip': value = 'text/plain'; self.send_header('Content-Encoding', 'gzip')
			SimpleHTTPServer.SimpleHTTPRequestHandler.send_header(self, keyword, value)
		def translate_path(self, path):
			return outdir+path
		def send_head(self):
			if self.path == '/favicon.ico': self.send_response(200);self.end_headers();return
			global HTTPRequestCount
			HTTPRequestCount += 1
			return SimpleHTTPServer.SimpleHTTPRequestHandler.send_head(self)
		def log_error(self, *args):
			if 'Request timed out' not in str(args): apply(self.log_message, args)
	class TimeoutHTTPServer(SocketServer.TCPServer):
		def server_bind(self):
			SocketServer.TCPServer.server_bind(self)
			self.socket.settimeout(1)
		def get_request(self):
			sock, addr = self.socket.accept()
			sock.settimeout(3)
			return (sock, addr)

	from random import randint
	httpdport = randint(5103,6102)
	httpd = TimeoutHTTPServer(("", httpdport), OurHTTPRequestHandler)
	htmlurl = 'http://localhost:'+str(httpdport)+'/'+htmlfile

	chromecmd = get_makefile_str('CHROME', nacl_dir+'/ZillaAppLocalConfig.mk', 'chrome');
	if chromecmd == 'chrome':
		if os.path.exists(os.getenv('ProgramFiles'     ,'')+'\chromium\chrome.exe'): chromecmd = os.getenv('ProgramFiles'     ,'')+'\chromium\chrome.exe'
		if os.path.exists(os.getenv('ProgramFiles(x86)','')+'\chromium\chrome.exe'): chromecmd = os.getenv('ProgramFiles(x86)','')+'\chromium\chrome.exe'
		if os.path.exists(os.getenv('ProgramFiles'     ,'')+  '\chrome\chrome.exe'): chromecmd = os.getenv('ProgramFiles'     ,'')+  '\chrome\chrome.exe'
		if os.path.exists(os.getenv('ProgramFiles(x86)','')+  '\chrome\chrome.exe'): chromecmd = os.getenv('ProgramFiles(x86)','')+  '\chrome\chrome.exe'
	try:os.spawnv(os.P_NOWAIT, chromecmd, ['"'+chromecmd+'"', '--incognito', htmlurl])
	except:
		sys.stderr.write("\nCould not run Chrome/Chromium with command line '"+chromecmd+"'\nSet custom path in "+nacl_dir+"/ZillaAppLocalConfig.mk with CHROME = "+('D:' if sys.platform == 'win32' else '')+"/path/to/Chromium/chrome"+('.exe' if sys.platform == 'win32' else '')+"\n\n")
		sys.exit(1)
	print "Opening",htmlurl,"with",chromecmd

	MaxRequestCount = (2 if outdir == 'Release-nacl' and get_makefile_int('ZLNACL_ASSETS_EMBED') else 3)
	print 'Serving', MaxRequestCount, 'files from directory', outdir, 'at port', httpdport
	try:
		for i in xrange(20):
			httpd.handle_request()
			if HTTPRequestCount == MaxRequestCount: break
		for i in xrange(3):
			httpd.handle_request()
	except:pass
	sys.exit(0)

else:
	print "No command in arguments specified."
	print "Valid commands are: RUN, WEB"
	print "RUN/WEB: start webserver in output dir run chrome browser to it"
