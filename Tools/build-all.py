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

import os, sys, subprocess, zipfile, shutil, re

#read MSBUILD_PATH, OUT_DIR and ANDROID_* variables for signing APK files from external file 'build-all.cfg' (not checked into version control)
MSBUILD_PATH = 'C:/Program Files (x86)/MSBuild/12.0/Bin/MSBuild.exe' #default
OUT_DIR = 'Builds'
buildall_dir = os.path.dirname(os.path.realpath(sys.argv.pop(0))).replace('\\', '/')
WEB_GZ = False
exec (file(buildall_dir+'/build-all.cfg').read() if os.path.exists(buildall_dir+'/build-all.cfg') else '')
WEB_GZ = ('.gz' if WEB_GZ else '')
if 'ANDROID_SIGN_KEYSTORE' in vars() and '..' in ANDROID_SIGN_KEYSTORE: ANDROID_SIGN_KEYSTORE = buildall_dir + '/' + ANDROID_SIGN_KEYSTORE
if 'OUT_DIR' in vars() and '..' in OUT_DIR: OUT_DIR = buildall_dir + '/' + OUT_DIR

proj_name = ((re.findall('ZillaApp\s*=\s*(.*)\s',open('Makefile').read()) if os.path.exists('Makefile') else []) or [os.path.basename(os.getcwd())])[0]
if not os.path.exists(proj_name+'-vs.vcxproj'): print 'Could not find ',proj_name+'-vs.vcxproj',' (Makefile needs ZillaApp definition or directory name has to be project name)'; sys.exit(1)
if not os.path.exists('Makefile'): print 'Could not find Makefile'; sys.exit(1)

if 'clean' in sys.argv:
	for p in [ 'Builds', 'Debug-vc6', 'Release-vc6', 'Debug-vs2012', 'Debug-vs2012x64', 'Debug-vs2013', 'Debug-vs2013x64', 'Debug-vs2015', 'Debug-vs2015x64', 'Release-vs2012', 'Release-vs2012x64', 'Release-vs2013', 'Release-vs2013x64', 'Release-vs2015', 'Release-vs2015x64', 'Debug-linux', 'Release-linux', 'Debug-nacl', 'Release-nacl', 'Debug-emscripten', 'Release-emscripten', proj_name+'-iOS.xcodeproj/Debug', proj_name+'-iOS.xcodeproj/Release', proj_name+'-OSX.xcodeproj/Debug', proj_name+'-OSX.xcodeproj/Release' ]:
		if os.path.exists(p): shutil.rmtree(p, True)
	sys.exit(0)

zl_dir = buildall_dir + '/..'
if sys.platform == 'win32': os.environ['PATH'] += os.pathsep+zl_dir.replace('/', os.sep)+os.sep+'Tools'
linux_cpu_type = ('x86_64' if sys.maxsize > 2**32 else 'x86_32')
is_rebuild = 'rebuild' in sys.argv
select_targets = [k for k in sys.argv if k in ['emscripten','nacl','android','win32','win64','linux','osx']]
warnerrors = []
if not os.path.exists(OUT_DIR): os.makedirs(OUT_DIR)

try:
	def buildheader(typ):
		print '---------------------------------------------------------------------------------------------------------------------------------------------------------------------'
		print '[' + typ + '] Building ',proj_name,':'
	def buildfooter():
		print '---------------------------------------------------------------------------------------------------------------------------------------------------------------------'
		print ''
	def building(pargs):
		print '    ****    Executing:',pargs,'...'
		p = subprocess.Popen(pargs, bufsize=1, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
		while True:
			l = p.stdout.readline()
			if not l: break
			if not l.strip(): continue
			sys.stderr.write('            ' + l.rstrip()[0:180] + "\n")
			sys.stderr.flush()
			if re.search(': (warn|error)', l, re.I): warnerrors.append(l)
		pret = p.wait()
		assert pret == 0, '    BUILD RETURNED ERROR, ABORTING'
	def buildcopy(src, trg):
		print '    ****    Copying',src,'to',trg,'...'
		shutil.copy2(src, OUT_DIR+'/'+trg)
	def buildzip(trgzip, src, trg):
		print '    ****    Zipping',src,'into',OUT_DIR+'/'+trgzip,'as',trg,'...'
		z = zipfile.ZipFile(OUT_DIR+'/'+trgzip,'w',zipfile.ZIP_DEFLATED);
		z.write(src, trg);[z.write(r+os.sep+f, r.replace(src, trg, 1)+os.sep+f) for r,d,fs in os.walk(src) for f in fs]
		z.close()
	def buildcheck(name, trg, outdir):
		if select_targets and name not in select_targets: return False
		if is_rebuild and os.path.exists(outdir): shutil.rmtree(outdir, True)
		return is_rebuild or not os.path.exists(OUT_DIR+'/'+trg)
	def getoutputexecutable(dir,  basename = proj_name, ext = ''):
		return (dir + '/' + basename + '_WithData' + ext if os.path.exists(dir + '/' + basename + '_WithData' + ext) else dir + '/' + basename + ext)
	if sys.platform == 'win32':
		if buildcheck('emscripten', proj_name + '.js'+WEB_GZ, 'Release-emscripten'):
			buildheader('EMSCRIPTEN')
			building(['make', '-j', '4', 'emscripten-release' ])
			buildcopy(getoutputexecutable('Release-emscripten', proj_name, '.js'+WEB_GZ), proj_name + '.js'+WEB_GZ)
			buildfooter()

		if buildcheck('nacl', proj_name + '.pexe'+WEB_GZ, 'Release-nacl'):
			buildheader('NACL')
			building(['make', '-j', '4', 'nacl-release' ])
			buildcopy(getoutputexecutable('Release-nacl', proj_name, '.pexe'+WEB_GZ), proj_name + '.pexe'+WEB_GZ)
			buildfooter()

		if buildcheck('android', proj_name + '.apk', 'Android/obj'):
			buildheader('ANDROID')
			building(['make', '-j', '4', 'android-release' ])
			building(['make', 'android-sign', 'SIGN_OUTAPK=' + OUT_DIR + '/' + proj_name + '.apk', 'SIGN_KEYSTORE='+ANDROID_SIGN_KEYSTORE, 'SIGN_STOREPASS='+ANDROID_SIGN_STOREPASS, 'SIGN_KEYALIAS='+ANDROID_SIGN_KEYALIAS, 'SIGN_KEYPASS='+ANDROID_SIGN_KEYPASS])
			buildfooter()

		if buildcheck('win32', proj_name + '_Win32.zip', 'Release-vs2013'):
			buildheader('WIN32')
			building('"'+MSBUILD_PATH+'" /p:Configuration=Release;Platform=Win32 '+proj_name+'-vs.vcxproj')
			buildzip(proj_name + '_Win32.zip', getoutputexecutable('Release-vs2013', proj_name, '.exe'), proj_name + '.exe')
			buildfooter()

		if buildcheck('win64', proj_name + '_Win64.zip', 'Release-vs2013x64'):
			buildheader('WIN64')
			building('"'+MSBUILD_PATH+'" /p:Configuration=Release;Platform=x64 '+proj_name+'-vs.vcxproj')
			buildzip(proj_name + '_Win64.zip', getoutputexecutable('Release-vs2013x64', proj_name, '.exe'), proj_name + '.exe')
			buildfooter()

	if sys.platform == 'linux2':
		if buildcheck('linux', proj_name + '_linux_' + linux_cpu_type + '.zip', 'Release-linux'):
			buildheader('LINUX')
			building(['make', '-j', '4', 'linux-release' ])
			buildzip(proj_name + '_linux_' + linux_cpu_type + '.zip', getoutputexecutable('Release-linux', proj_name+'_' + linux_cpu_type), proj_name)
			buildfooter()

	if sys.platform == 'darwin':
		if buildcheck('osx', proj_name + '_osx.zip', proj_name+'-OSX.xcodeproj/Release'):
			buildheader('OSX')
			building(['make', '-j', '4', 'osx-release' ])
			buildzip(proj_name + '_osx.zip', proj_name+'-OSX.xcodeproj/Release/'+proj_name+'.app', proj_name + '.app')
			buildfooter()

except: import traceback; traceback.print_exception(sys.exc_info()[0], sys.exc_info()[1], sys.exc_info()[2]);

if warnerrors:
	warnerrors = list(set(map((lambda s: s.strip()), warnerrors)))
	warnerrors.sort()
	print 'WARNINGS/ERRORS:\n'+'\n'.join(warnerrors)
