#
#  ZillaLib
#  Copyright (C) 2010-2016 Bernhard Schelling
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

elif cmd == 'BUILD':
	nacl_sdk = get_makefile_str('NACL_SDK', nacl_dir+'/ZillaAppLocalConfig.mk');
	if nacl_sdk != '' and (not os.path.exists(nacl_sdk+'/tools/getos.py') or not os.path.exists(nacl_sdk+'/toolchain')):
		sys.stderr.write(" \nSubdirectory 'tools' and subdirectory 'toolchain' not found inside "+nacl_sdk+"\n")
		nacl_sdk = ''
	if nacl_sdk == '':
		sys.stderr.write(" \nPlease create the file "+nacl_dir+os.sep+"ZillaAppLocalConfig.mk with the following definitions:\n \n")
		sys.stderr.write("NACL_SDK = "+('d:' if sys.platform == 'win32' else '')+"/path/to/nacl_sdk/pepper18\n \n")
		sys.exit(0)
	os.environ['PATH'] = nacl_sdk+os.sep+'tools' + os.pathsep + os.environ['PATH'] + os.pathsep + os.path.dirname(sys.executable)
	pargs = ['make', '-f', nacl_dir+'/ZillaLibNACL.mk', '-j', '4', 'PYTHON='+sys.executable]
	targs = []
	vc = False;appname = None
	for ln in sys.argv:
		if re.match('-CLEAN', ln, re.I) and targs.count('clean') == 0: targs.insert(0, 'clean')
		elif re.match('-B', ln, re.I): pargs.append('-B')
		elif re.match('-REL', ln, re.I): pargs.append('BUILD=RELEASE')
		elif re.match('-VC', ln, re.I): vc = True
		else: appname = ln
	if appname:pargs.append('ZillaApp='+appname)
	if appname and get_makefile_int('ZLNACL_ASSETS_EMBED'): pargs.append('ASSETS_EMBED=1')
	if appname and get_makefile_str('ZLNACL_ASSETS_OUTFILE'): pargs.append('ASSETS_OUTFILE='+get_makefile_str('ZLNACL_ASSETS_OUTFILE'))
	if len(targs) == 0: targs.append('')
	ret = 0
	for targ in targs:
		if targ: pargs.append(targ)
		#sys.stderr.write("# " + string.join(pargs, " ") + "\n");sys.stderr.flush()
		if not vc:
			ret |= subprocess.Popen(pargs).wait()
		else:
			r1=re.compile(':(\\d+):')
			r2=re.compile('\\xe2\\x80[\\x98\\x99]|\\r|\\n')
			r3=re.compile(re.escape('/cygdrive/'+os.getcwd().replace('\\','/').replace(':','')+'/'),re.IGNORECASE)
			r4=re.compile('/cygdrive/(.)')
			p = subprocess.Popen(pargs, shell=True, bufsize=1, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
			while True:
				l = p.stdout.readline()
				if not l: break
				sys.stderr.write(r4.sub('\\1:', r3.sub('',r2.sub('',r1.sub('(\\1) :',l)))) + "\n")
				sys.stderr.flush()
			ret |= p.wait()
		if targ: del pargs[len(pargs)-1]
	sys.stderr.write("\n")
	sys.exit(ret)

else:
	print "No command in arguments specified."
	print "Valid commands are: BUILD, RUN, WEB"
	print "BUILD: compile, switches: -vc, -B, -rel, -clean"
	print "RUN/WEB: start webserver in output dir run chrome browser to it"
