/*
  ZillaLib
  Copyright (C) 2010-2019 Bernhard Schelling

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

#if defined(__APPLE__)
#include "AvailabilityMacros.h"
#endif
#ifdef MAC_OS_X_VERSION_10_3
#include "TargetConditionals.h"
#endif
#if TARGET_OS_IPHONE
#include "ZL_Platform.h"
#include "ZL_Application.h"
#include "ZL_Display.h"
#include "ZL_Display_Impl.h"

#import <UIKit/UIKit.h>
#import <OpenGLES/ES1/glext.h>
#import <AudioToolbox/AudioToolbox.h>
#import <QuartzCore/QuartzCore.h>

enum IOSTOUCH_ACTION { IOSTOUCH_DOWN = 0, IOSTOUCH_UP = 1, IOSTOUCH_MOVE = 2 };
struct ZL_IOSTouch { int lastx, lasty; UITouch *touch; };

@interface ZL_UIKitDelegate : NSObject<UIApplicationDelegate> @end
@interface ZL_UIViewController : UIViewController @end
@class ZL_UIKitView;

static void ZL_IOS_Activate();
static void ZL_IOS_Deactivate();

#define MAX_SIMULTANEOUS_TOUCHES 10
static ZL_IOSTouch ZL_IOS_touch[MAX_SIMULTANEOUS_TOUCHES];
static ZL_JoystickData* ZL_IOS_joysticks[2] = { NULL, NULL };
static UITextField* ZL_IOS_TextField;
static BOOL ZL_IOS_KeyboardVisible = NO;
static BOOL ZL_IOS_WantsLandscape = NO;
static unsigned int ZL_IOS_WindowFlags = ZL_WINDOW_FULLSCREEN | ZL_WINDOW_INPUT_FOCUS | ZL_WINDOW_MOUSE_FOCUS;
static int ZL_IOS_WindowWidth = 320, ZL_IOS_WindowHeight = 480;
static CGFloat csf = 1.0;
static AudioUnit ZL_IOS_AudioUnit = NULL;
static UIWindow* ZL_IOS_uiwindow = nil;
static ZL_UIViewController* ZL_IOS_viewcontroller = nil;
static ZL_UIKitView* ZL_IOS_view = nil; //
static double lastfps = 0;
static int fpscount = 0;
static bool bActive = false;
static bool bMainLoopRunning = false;
static id displayLink = nil;
static BOOL ZL_IOS_HasUIKitDisplayLink = NO;
static ZL_UIKitDelegate* ZL_IOS_UIKitDelegate = nil;
static id lastActiveAudioPlayer = nil;
static NSUserDefaults* standardUserDefaults = nil;

static void ZL_WindowEvent(unsigned char event, int data1 = 0, int data2 = 0)
{
	if (pZL_WindowFlags != &ZL_IOS_WindowFlags) return; //make sure window has been fully created
	ZL_Event e; e.type = ZL_EVENT_WINDOW;
	e.window.event = event; e.window.data1 = data1; e.window.data2 = data2;
	ZL_Display_Process_Event(e);
}

static void ZL_TouchEvent(ZL_IOSTouch* t, CGPoint location, IOSTOUCH_ACTION action, int DeviceIndex)
{
	location.x *= csf;
	location.y *= csf;
	if (action == IOSTOUCH_UP)
	{
		//ios touch panels send x/y coordinates with the up-event that are extrapolated from the last
		//touch event and the direction the finger is moving away from the screen
		//downscale these extrapolations as they seem too much and other OS/devices won't do that.
		if (t->lastx) location.x = (t->lastx*3+location.x)/4;
		if (t->lasty) location.y = (t->lasty*3+location.y)/4;
	}
	ZL_Event em;
	em.type = ZL_EVENT_MOUSEMOTION;
	em.motion.which = DeviceIndex;
	em.motion.state = (action == IOSTOUCH_DOWN ? 0 : 1);
	em.motion.x = location.x;
	em.motion.y = location.y;
	em.motion.xrel = (t->lastx >= 0 ? location.x - t->lastx : 0);
	em.motion.yrel = (t->lasty >= 0 ? location.y - t->lasty : 0);
	if (em.motion.xrel || em.motion.yrel) ZL_Display_Process_Event(em);
	t->lastx = location.x;
	t->lasty = location.y;
	if (action == IOSTOUCH_DOWN || action == IOSTOUCH_UP)
	{
		ZL_Event eb;
		eb.type = (action == IOSTOUCH_DOWN ? ZL_EVENT_MOUSEBUTTONDOWN : ZL_EVENT_MOUSEBUTTONUP);
		eb.button.which = DeviceIndex;
		eb.button.button = 1;
		eb.button.is_down = (action == IOSTOUCH_DOWN);
		eb.button.x = location.x;
		eb.button.y = location.y;
		ZL_Display_Process_Event(eb);
	}
	if (action == IOSTOUCH_UP) t->lastx = t->lasty = -1;
}

static void ZL_KeyEvent(ZL_Key key, bool is_down, int modstate)
{
	ZL_Event e;
	e.type = (is_down ? ZL_EVENT_KEYDOWN : ZL_EVENT_KEYUP);
	e.key.is_down = is_down;
	e.key.key = key;
	e.key.mod = modstate;
	ZL_Display_Process_Event(e);
}

@interface ZL_UIKitView : UIView<UITextFieldDelegate>
{
	EAGLContext *context;
	GLuint viewRenderbuffer, viewFramebuffer; // OpenGL names for the renderbuffer and framebuffers used to render to this view
	GLuint depthRenderbuffer; // OpenGL name for the depth buffer that is attached to viewFramebuffer, if it exists (0 if it does not exist)
}
@end

@implementation ZL_UIKitView
+ (Class)layerClass { return [CAEAGLLayer class]; }

- (id)initWithFrame:(CGRect)frame
{
	self = [super initWithFrame: frame];
	if (!self) return nil;

	if ([self respondsToSelector:@selector(contentScaleFactor)])
	{
		csf = self.contentScaleFactor = [[UIScreen mainScreen] scale];
		self.contentMode = UIViewContentModeScaleToFill;
	}

	ZL_IOS_TextField = [[[UITextField alloc] initWithFrame: CGRectZero] autorelease];
	ZL_IOS_TextField.delegate = self;
	ZL_IOS_TextField.text = @" "; //can be backspaced
	ZL_IOS_TextField.autocapitalizationType = UITextAutocapitalizationTypeNone;
	ZL_IOS_TextField.autocorrectionType = UITextAutocorrectionTypeNo;
	ZL_IOS_TextField.enablesReturnKeyAutomatically = NO;
	ZL_IOS_TextField.keyboardAppearance = UIKeyboardAppearanceAlert;
	ZL_IOS_TextField.keyboardType = UIKeyboardTypeASCIICapable;
	ZL_IOS_TextField.returnKeyType = UIReturnKeyDefault;
	ZL_IOS_TextField.secureTextEntry = NO;
	ZL_IOS_TextField.hidden = YES;
	ZL_IOS_KeyboardVisible = NO;
	[self addSubview: ZL_IOS_TextField];

	self.multipleTouchEnabled = YES;
	memset(ZL_IOS_touch, 0, sizeof(ZL_IOS_touch));
	for (int i = 0; i < MAX_SIMULTANEOUS_TOUCHES; i++) ZL_IOS_touch[i].lastx = ZL_IOS_touch[i].lasty = -1;

	return self;
}

- (void)dealloc
{
	if (viewFramebuffer) { glDeleteFramebuffersOES(1, &viewFramebuffer); viewFramebuffer = 0; }
	if (viewRenderbuffer) { glDeleteRenderbuffersOES(1, &viewRenderbuffer); viewRenderbuffer = 0; }
	if (depthRenderbuffer) { glDeleteRenderbuffersOES(1, &depthRenderbuffer); depthRenderbuffer = 0; }
	if ([EAGLContext currentContext] == context) [EAGLContext setCurrentContext:nil];
	[context release];
	[ZL_IOS_TextField release];
	[super dealloc];
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
	NSEnumerator *enumerator = [touches objectEnumerator];
	UITouch *touch =(UITouch*)[enumerator nextObject];
	for (int i = 0; touch && i < MAX_SIMULTANEOUS_TOUCHES; i++)
	{
		if (ZL_IOS_touch[i].touch != nil) continue;
		ZL_IOS_touch[i].touch = [touch retain];
		ZL_TouchEvent(&ZL_IOS_touch[i], [touch locationInView: self], IOSTOUCH_DOWN, i);
		touch =(UITouch*)[enumerator nextObject];
	}
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch *touch in touches)
	{
		for (int i = 0; i < MAX_SIMULTANEOUS_TOUCHES; i++)
		{
			if (ZL_IOS_touch[i].touch != touch) continue;
			[ZL_IOS_touch[i].touch release];
			ZL_IOS_touch[i].touch = nil;
			ZL_TouchEvent(&ZL_IOS_touch[i], [touch locationInView: self], IOSTOUCH_UP, i);
			break;
		}
	}
}

- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
	for (UITouch *touch in touches)
	{
		for (int i = 0; i < MAX_SIMULTANEOUS_TOUCHES; i++)
		{
			if (ZL_IOS_touch[i].touch != touch) continue;
			ZL_TouchEvent(&ZL_IOS_touch[i], [touch locationInView: self], IOSTOUCH_MOVE, i);
		}
	}
}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
	[self touchesEnded: touches withEvent: event];
}

- (BOOL)textField:(UITextField *)_textField shouldChangeCharactersInRange:(NSRange)range replacementString:(NSString *)string
{
	struct UIKitKeyInfo { ZL_Key key; int mod; };
	static const UIKitKeyInfo unicharToUIKeyInfoTable[] =
	{
		/*   0 */ { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 },
		/*  10 */ { ZLK_RETURN, 0 },
		/*  11 */ { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 }, { ZLK_UNKNOWN, 0 },
		/*  32 */ { ZLK_SPACE, 0 },
		/*  33 */ { ZLK_1,	ZLKMOD_SHIFT }, //'!'
		/*  34 */ { ZLK_APOSTROPHE, ZLKMOD_SHIFT }, //'"'
		/*  35 */ { ZLK_3, ZLKMOD_SHIFT }, //'#'
		/*  36 */ { ZLK_4, ZLKMOD_SHIFT }, //'$'
		/*  37 */ { ZLK_5, ZLKMOD_SHIFT }, //'%'
		/*  38 */ { ZLK_7, ZLKMOD_SHIFT }, //'&'
		/*  39 */ { ZLK_APOSTROPHE, 0 },
		/*  40 */ { ZLK_9, ZLKMOD_SHIFT }, //'('
		/*  41 */ { ZLK_0, ZLKMOD_SHIFT }, //')'
		/*  42 */ { ZLK_8, ZLKMOD_SHIFT }, //'*'
		/*  43 */ { ZLK_EQUALS, ZLKMOD_SHIFT }, //'+'
		/*  44 */ { ZLK_COMMA, 0 }, { ZLK_MINUS, 0 }, { ZLK_PERIOD, 0 }, { ZLK_SLASH, 0 },
		/*  48 */ { ZLK_0, 0 }, { ZLK_1, 0 }, { ZLK_2, 0 }, { ZLK_3, 0 }, { ZLK_4, 0 }, { ZLK_5, 0 }, { ZLK_6, 0 }, { ZLK_7, 0 }, { ZLK_8, 0 }, { ZLK_9, 0 },
		/*  58 */ { ZLK_SEMICOLON, ZLKMOD_SHIFT }, //';'
		/*  59 */ { ZLK_SEMICOLON, 0 },
		/*  60 */ { ZLK_COMMA, ZLKMOD_SHIFT }, //'<'
		/*  61 */ { ZLK_EQUALS, 0 },
		/*  62 */ { ZLK_PERIOD, ZLKMOD_SHIFT }, //'>'
		/*  63 */ { ZLK_SLASH, ZLKMOD_SHIFT }, //'?'
		/*  64 */ { ZLK_2, ZLKMOD_SHIFT }, //'@'
		/*  65 */ { ZLK_A, ZLKMOD_SHIFT }, { ZLK_B, ZLKMOD_SHIFT }, { ZLK_C, ZLKMOD_SHIFT }, { ZLK_D, ZLKMOD_SHIFT }, { ZLK_E, ZLKMOD_SHIFT }, { ZLK_F, ZLKMOD_SHIFT }, { ZLK_G, ZLKMOD_SHIFT }, { ZLK_H, ZLKMOD_SHIFT }, { ZLK_I, ZLKMOD_SHIFT }, { ZLK_J, ZLKMOD_SHIFT }, { ZLK_K, ZLKMOD_SHIFT }, { ZLK_L, ZLKMOD_SHIFT }, { ZLK_M, ZLKMOD_SHIFT }, { ZLK_N, ZLKMOD_SHIFT }, { ZLK_O, ZLKMOD_SHIFT }, { ZLK_P, ZLKMOD_SHIFT }, { ZLK_Q, ZLKMOD_SHIFT }, { ZLK_R, ZLKMOD_SHIFT }, { ZLK_S, ZLKMOD_SHIFT }, { ZLK_T, ZLKMOD_SHIFT }, { ZLK_U, ZLKMOD_SHIFT }, { ZLK_V, ZLKMOD_SHIFT }, { ZLK_W, ZLKMOD_SHIFT }, { ZLK_X, ZLKMOD_SHIFT }, { ZLK_Y, ZLKMOD_SHIFT }, { ZLK_Z, ZLKMOD_SHIFT },
		/*  91 */ { ZLK_LEFTBRACKET, 0 },
		/*  92 */ { ZLK_BACKSLASH, 0 },
		/*  93 */ { ZLK_RIGHTBRACKET, 0 },
		/*  94 */ { ZLK_6, ZLKMOD_SHIFT }, //'^'
		/*  95 */ { ZLK_MINUS, ZLKMOD_SHIFT }, //'_'
		/*  96 */ { ZLK_GRAVE, ZLKMOD_SHIFT },
		/*  97 */ { ZLK_A, 0	}, { ZLK_B, 0 }, { ZLK_C, 0 }, { ZLK_D, 0 }, { ZLK_E, 0 }, { ZLK_F, 0 }, { ZLK_G, 0 }, { ZLK_H, 0 }, { ZLK_I, 0 }, { ZLK_J, 0 }, { ZLK_K, 0 }, { ZLK_L, 0 }, { ZLK_M, 0 }, { ZLK_N, 0 }, { ZLK_O, 0 }, { ZLK_P, 0 }, { ZLK_Q, 0 }, { ZLK_R, 0 }, { ZLK_S, 0 }, { ZLK_T, 0 }, { ZLK_U, 0 }, { ZLK_V, 0 }, { ZLK_W, 0 }, { ZLK_X, 0 }, { ZLK_Y, 0 }, { ZLK_Z, 0 },
		/* 123 */ { ZLK_LEFTBRACKET, ZLKMOD_SHIFT }, //'{'
		/* 124 */ { ZLK_BACKSLASH, ZLKMOD_SHIFT }, //'|'
		/* 125 */ { ZLK_RIGHTBRACKET, ZLKMOD_SHIFT }, //'}'
		/* 126 */ { ZLK_GRAVE, ZLKMOD_SHIFT }, //'~'
		/* 127 */ { ZLK_DELETE, ZLKMOD_SHIFT }
	};
	if ([string length] == 0)
	{
		ZL_KeyEvent(ZLK_BACKSPACE, true, 0);
		ZL_Event e;
		e.type = ZL_EVENT_TEXTINPUT;
		e.text.text[0] = '\b'; e.text.text[1] = 0;
		ZL_Display_Process_Event(e);
		ZL_KeyEvent(ZLK_BACKSPACE, false, 0);
	}
	else
	{
		for (int i = 0; i < [string length]; i++)
		{
			unichar c = [string characterAtIndex: i];
			const UIKitKeyInfo& keyinfo = unicharToUIKeyInfoTable[c < 128 ? c : 0];
			if (keyinfo.mod & ZLKMOD_SHIFT) ZL_KeyEvent(ZLK_LSHIFT, true, 0);
			ZL_KeyEvent(keyinfo.key, true, keyinfo.mod);
		}
		const char* pcText = [string UTF8String];
		ZL_Event e;
		e.type = ZL_EVENT_TEXTINPUT;
		size_t len = strlen(pcText);
		size_t maxlen = (len >= sizeof(e.text.text) ? sizeof(e.text.text)-1: len);
		memcpy(e.text.text, pcText, maxlen);
		e.text.text[maxlen] = 0;
		ZL_Display_Process_Event(e);
		for (int i = 0; i < [string length]; i++)
		{
			unichar c = [string characterAtIndex: i];
			const UIKitKeyInfo& keyinfo = unicharToUIKeyInfoTable[c < 128 ? c : 0];
			ZL_KeyEvent(keyinfo.key, false, keyinfo.mod);
			if (keyinfo.mod & ZLKMOD_SHIFT) ZL_KeyEvent(ZLK_LSHIFT, false, 0);
		}
	}
	return NO; // never edit original text
}

- (BOOL)textFieldShouldReturn:(UITextField*)_textField
{
	[self textField:_textField shouldChangeCharactersInRange:NSMakeRange(1, 0) replacementString:@"\n"];
	return YES;
}

- (void)setCurrentContext { [EAGLContext setCurrentContext:context]; }
- (void)swapBuffers { glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer); [context presentRenderbuffer:GL_RENDERBUFFER_OES]; }

- (void)initGLContext
{
	NSString *colorFormat = kEAGLColorFormatRGBA8; //kEAGLColorFormatRGB565 faster
	BOOL retained = YES;

	CAEAGLLayer *eaglLayer = (CAEAGLLayer *)self.layer;

	eaglLayer.opaque = YES;
	eaglLayer.drawableProperties = [NSDictionary dictionaryWithObjectsAndKeys:[NSNumber numberWithBool:retained], kEAGLDrawablePropertyRetainedBacking, colorFormat, kEAGLDrawablePropertyColorFormat, nil];
	#if defined(ZL_VIDEO_OPENGL_ES2)
	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES2];
	#elif defined(ZL_VIDEO_OPENGL_ES1)
	context = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1];
	#else
	#error NO ZL GLES version defined
	#endif
	if (!context || ![EAGLContext setCurrentContext:context]) return;

	//Framebuffer
	glGenFramebuffersOES(1, &viewFramebuffer);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, viewFramebuffer);

	//Renderbuffer
	glGenRenderbuffersOES(1, &viewRenderbuffer);

	if (ZL_IOS_WindowFlags & ZL_WINDOW_DEPTHBUFFER)
	{
		glGenRenderbuffersOES(1, &depthRenderbuffer);
	}
}

- (void)initGLRenderbuffer
{
	//Renderbuffer
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, viewRenderbuffer);
	[context renderbufferStorage:GL_RENDERBUFFER_OES fromDrawable:(CAEAGLLayer*)self.layer];
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES, viewRenderbuffer);

	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_WIDTH_OES, &ZL_IOS_WindowWidth);
	glGetRenderbufferParameterivOES(GL_RENDERBUFFER_OES, GL_RENDERBUFFER_HEIGHT_OES, &ZL_IOS_WindowHeight);
	//printf("[ZL_UIKitView initGLRenderbuffer] Setting GL Renderbuffer to size %d x %d (%s)\n", ZL_IOS_WindowWidth, ZL_IOS_WindowHeight, (ZL_IOS_WindowWidth > ZL_IOS_WindowHeight ? "Landscape" : "Portrait"));

	if (ZL_IOS_WindowFlags & ZL_WINDOW_DEPTHBUFFER)
	{
		glBindRenderbufferOES(GL_RENDERBUFFER_OES, depthRenderbuffer);
		glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES, ZL_IOS_WindowWidth, ZL_IOS_WindowHeight); //GL_DEPTH_COMPONENT24_OES
		glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES, depthRenderbuffer);
	}
}

- (void)layoutSubviews
{
	[super layoutSubviews];

	int width  = (int)(self.bounds.size.width * csf);
	int height = (int)(self.bounds.size.height * csf);
	//printf("[ZL_UIKitView layoutSubviews] layoutSubviews - Current Size: %4d,%4d - Last Size: %4d,%4d\n", width, height, ZL_IOS_WindowWidth, ZL_IOS_WindowHeight);

	if (width != ZL_IOS_WindowWidth || height != ZL_IOS_WindowHeight)
	{
		[self initGLRenderbuffer];
		ZL_WindowEvent(ZL_WINDOWEVENT_RESIZED, ZL_IOS_WindowWidth, ZL_IOS_WindowHeight);
	}
}

@end

@implementation ZL_UIViewController
- (BOOL)shouldAutorotate
{
	return YES;
}
- (NSUInteger)supportedInterfaceOrientations
{
	return ((ZL_IOS_WindowFlags & ZL_WINDOW_ALLOWANYORIENTATION) ? UIInterfaceOrientationMaskAll : (ZL_IOS_WantsLandscape ? UIInterfaceOrientationMaskLandscape : (UIInterfaceOrientationMaskPortrait | UIInterfaceOrientationMaskPortraitUpsideDown)));
}
-(BOOL) shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)toInterfaceOrientation
{
	return ((ZL_IOS_WindowFlags & ZL_WINDOW_ALLOWANYORIENTATION) ? YES : (ZL_IOS_WantsLandscape ? UIInterfaceOrientationIsLandscape(toInterfaceOrientation) : UIInterfaceOrientationIsPortrait(toInterfaceOrientation)));
}
- (void)presentModalViewController:(UIViewController *)modalViewController animated:(BOOL)animated
{
	ZL_IOS_Deactivate();
	[super presentModalViewController:modalViewController animated:animated];
}
- (void)dismissModalViewControllerAnimated:(BOOL)animated
{
	[super dismissModalViewControllerAnimated:animated];
	ZL_IOS_Activate();
}
@end

bool ZL_CreateWindow(const char* windowtitle, int width, int height, int displayflags)
{
	if (ZL_IOS_view != nil) return true;
	ZL_IOS_WantsLandscape = (width > height ? 1 : 0);
	if (displayflags & ZL_DISPLAY_ALLOWANYORIENTATION) ZL_IOS_WindowFlags |= ZL_WINDOW_ALLOWANYORIENTATION;
	if (displayflags & ZL_DISPLAY_DEPTHBUFFER) ZL_IOS_WindowFlags |= ZL_WINDOW_DEPTHBUFFER;
	ZL_IOS_WindowWidth = ZL_IOS_WindowHeight = 0;

	ZL_IOS_viewcontroller = [[ZL_UIViewController alloc] init];
	[ZL_IOS_uiwindow setRootViewController:ZL_IOS_viewcontroller];

	CGRect rec = ZL_IOS_uiwindow.bounds;
	UIInterfaceOrientation orientation = [[UIApplication sharedApplication] statusBarOrientation];
	if (UIInterfaceOrientationIsPortrait(orientation) == (rec.size.width > rec.size.height)) { CGFloat tmp = rec.size.width; rec.size.width = rec.size.height; rec.size.height = tmp; }

	ZL_IOS_view = [[ZL_UIKitView alloc] initWithFrame:rec];
	ZL_IOS_view.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;
	ZL_IOS_viewcontroller.view = ZL_IOS_view;
	[ZL_IOS_view initGLContext]; //will set window width and height to correct values according to required orientation
	[ZL_IOS_view initGLRenderbuffer];

	////For debugging layout issues
	//ZL_IOS_uiwindow.backgroundColor = [UIColor redColor];
	//ZL_IOS_view.backgroundColor = [UIColor greenColor];

	pZL_WindowFlags = &ZL_IOS_WindowFlags;
	return true;
}

static void ZL_IOS_Activate()
{
	if (ZL_IOS_WindowFlags & ZL_WINDOW_MINIMIZED)
	{
		ZL_IOS_WindowFlags &= ~ZL_WINDOW_MINIMIZED;
		ZL_WindowEvent(ZL_WINDOWEVENT_RESTORED);
	}
	if (ZL_IOS_AudioUnit) AudioOutputUnitStart(ZL_IOS_AudioUnit);
	if (lastActiveAudioPlayer) [(id)lastActiveAudioPlayer performSelector:@selector(play)];
	ZL_LastFPSTicks = ZL_Application::Ticks = ZL_GetTicks();
	if (ZL_IOS_HasUIKitDisplayLink)
	{
		displayLink = [NSClassFromString(@"CADisplayLink") displayLinkWithTarget:ZL_IOS_UIKitDelegate selector:@selector(drawView)];
		[displayLink setFrameInterval:(ZL_TPF_Limit < 30 ? 1 : (int)(ZL_TPF_Limit/15+0.4999f))];
		[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
	}
	else [ZL_IOS_UIKitDelegate performSelector:@selector(mainLoop) withObject:nil afterDelay:0.0];
	bActive = bMainLoopRunning = true;
}

static void ZL_IOS_Deactivate()
{
	bActive = false;
	if (ZL_IOS_HasUIKitDisplayLink) { [displayLink invalidate]; displayLink = nil; }
	ZL_IOS_WindowFlags |= ZL_WINDOW_MINIMIZED;
	ZL_WindowEvent(ZL_WINDOW_MINIMIZED);
	if (ZL_IOS_AudioUnit) AudioOutputUnitStop(ZL_IOS_AudioUnit);
	if (lastActiveAudioPlayer) [(id)lastActiveAudioPlayer performSelector:@selector(pause)];
	if (ZL_IOS_view != nil) [ZL_IOS_view swapBuffers];
}

@implementation ZL_UIKitDelegate
- (void)applicationDidFinishLaunching:(UIApplication *)application
{
	NSString *reqSysVer = @"3.1";
	NSString *currSysVer = [[UIDevice currentDevice] systemVersion];
	ZL_IOS_HasUIKitDisplayLink = ([currSysVer compare:reqSysVer options:NSNumericSearch] != NSOrderedAscending);
	[[NSFileManager defaultManager] changeCurrentDirectoryPath: [[NSBundle mainBundle] resourcePath]];
	[[UIApplication sharedApplication] setIdleTimerDisabled:YES];

	//printf("Setting Window to frame %f x %f\n", [UIScreen mainScreen].bounds.size.width, [UIScreen mainScreen].bounds.size.height);
	ZL_IOS_uiwindow = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
	ZL_IOS_uiwindow.autoresizingMask = UIViewAutoresizingFlexibleWidth | UIViewAutoresizingFlexibleHeight;

	ZillaLibInit(0, NULL);
	ZL_MainApplication->Frame();

	[ZL_IOS_uiwindow makeKeyAndVisible];
	ZL_IOS_UIKitDelegate = self;
}

- (void)mainLoop
{
	while (!(ZL_MainApplicationFlags & ZL_APPLICATION_DONE) && bActive)
	{
		for(; CFRunLoopRunInMode(kCFRunLoopDefaultMode, .003, TRUE) == kCFRunLoopRunHandledSource;) {}
		ZL_MainApplication->Frame();
		[ZL_IOS_view swapBuffers];

	}
	bMainLoopRunning = false;
}

- (void) drawView
{
	if ([displayLink frameInterval] == 6)
	{
		[displayLink setFrameInterval:(ZL_TPF_Limit < 30 ? 1 : ZL_TPF_Limit/15)];
		lastfps = [displayLink timestamp];
		fpscount = 1;
	}
	else if (fpscount == 10)
	{
		int tpf = ([displayLink timestamp] - lastfps) / 0.010;
		if (lastfps && tpf > 17 && tpf > (int)ZL_TPF_Limit + 1) [displayLink setFrameInterval:6];
		else { lastfps = [displayLink timestamp]; fpscount = 1; }
	}
	else fpscount++;
	ZL_MainApplication->Frame();
	[ZL_IOS_view swapBuffers];
}

- (void) applicationDidBecomeActive:(UIApplication*)application { ZL_IOS_Activate(); }
- (void) applicationWillResignActive:(UIApplication*)application { ZL_IOS_Deactivate(); }
- (void) applicationWillTerminate:(UIApplication *)application
{
	if (ZL_IOS_view != nil) [ZL_IOS_view release];
	if (ZL_IOS_viewcontroller != nil) [ZL_IOS_viewcontroller release];
	[ZL_IOS_uiwindow release];
	bActive = false;
	ZL_Event e; e.type = ZL_EVENT_QUIT; ZL_Display_Process_Event(e);
}
@end

int main(int argc, char **argv)
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	UIApplicationMain(argc, argv, NULL, @"ZL_UIKitDelegate");
	[pool release];
}

void ZL_GetWindowSize(int *w, int *h)
{
	*w = ZL_IOS_WindowWidth;
	*h = ZL_IOS_WindowHeight;
}

ZL_String ZL_DeviceUniqueID()
{
	NSString *uid = NULL;
	if ([[UIDevice currentDevice] respondsToSelector:@selector(identifierForVendor)])
		uid = [[[UIDevice currentDevice] performSelector:@selector(identifierForVendor)] performSelector:@selector(UUIDString)];
	else
		uid = [[UIDevice currentDevice] performSelector:@selector(uniqueIdentifier)];
	return (uid ? ZL_String([[uid stringByReplacingOccurrencesOfString:@"-" withString:@""] UTF8String]) : ZL_String());
}

void ZL_SettingsInit(const char*)
{
	standardUserDefaults = [NSUserDefaults standardUserDefaults];
}
const ZL_String ZL_SettingsGet(const char* Key)
{
	NSString *nskey = [NSString stringWithUTF8String:Key], *nsval = @"";
	if ([standardUserDefaults objectForKey:nskey]) nsval = [standardUserDefaults objectForKey:nskey];
	return ZL_String([nsval cStringUsingEncoding:NSUTF8StringEncoding]);
}
void ZL_SettingsSet(const char* Key, const char* Value)
{
	[standardUserDefaults setObject:[NSString stringWithUTF8String:Value] forKey:[NSString stringWithUTF8String:Key]];
}
void ZL_SettingsDel(const char* Key)
{
	[standardUserDefaults removeObjectForKey:[NSString stringWithUTF8String:Key]];
}
bool ZL_SettingsHas(const char* Key)
{
	return (nil != [standardUserDefaults objectForKey:[NSString stringWithUTF8String:Key]]);
}
void ZL_SettingsSynchronize()
{
	[standardUserDefaults synchronize];
}

void ZL_SoftKeyboardToggle()
{
	(ZL_IOS_KeyboardVisible ? ZL_SoftKeyboardHide() : ZL_SoftKeyboardShow());
}
void ZL_SoftKeyboardShow()
{
	if (ZL_IOS_KeyboardVisible) return;
	ZL_IOS_KeyboardVisible = YES;
	[ZL_IOS_TextField becomeFirstResponder];
}
void ZL_SoftKeyboardHide()
{
	if (!ZL_IOS_KeyboardVisible) return;
	ZL_IOS_KeyboardVisible = NO;
	[ZL_IOS_TextField resignFirstResponder];
}
bool ZL_SoftKeyboardIsShown() { return (bool)ZL_IOS_KeyboardVisible; }

void ZL_DeviceVibrate(int duration)
{
	AudioServicesPlaySystemSound(kSystemSoundID_Vibrate);
}

void ZL_OpenExternalUrl(const char* url)
{
	[[UIApplication sharedApplication] openURL:[NSURL URLWithString:[NSString stringWithCString:url encoding:NSUTF8StringEncoding]]];
}

//void ZillaLib_UIKitFilterJoyAxisEvent(ZL_JoyAxisEvent *jaxis)
//{
//	if (jaxis->axis != 1 || jaxis->which != 0 || ZL_IOS_viewcontroller.interfaceOrientation == UIInterfaceOrientationLandscapeLeft || ZL_IOS_viewcontroller.interfaceOrientation == UIInterfaceOrientationPortrait) return;
//	//jaxis->value *= -1;
//}

int ZL_NumJoysticks()
{
	return 0;
}
ZL_JoystickData* ZL_JoystickHandleOpen(int index)
{
	return NULL;
}
void ZL_JoystickHandleClose(ZL_JoystickData* joystick)
{
}

static OSStatus ZL_AudioOutputCallback(void *inRefCon, AudioUnitRenderActionFlags * ioActionFlags, const AudioTimeStamp * inTimeStamp, UInt32 inBusNumber, UInt32 inNumberFrames, AudioBufferList * ioDataList)
{
	if (ioDataList->mNumberBuffers != 1) return noErr;
	AudioBuffer *ioData = &ioDataList->mBuffers[0];
	ZL_PlatformAudioMix((short*)ioData->mData, ioData->mDataByteSize);
	return noErr;
}

bool ZL_AudioOpen(unsigned int /*buffer_length*/)
{
	if (ZL_IOS_AudioUnit) return true; // cannot restart on IOS
	AudioComponentDescription desc;
	memset(&desc, 0, sizeof(AudioComponentDescription));
	desc.componentType = kAudioUnitType_Output;
	desc.componentSubType = kAudioUnitSubType_RemoteIO;
	desc.componentManufacturer = kAudioUnitManufacturer_Apple;

	AudioStreamBasicDescription strdesc;
	memset(&strdesc, 0, sizeof(AudioStreamBasicDescription));
	strdesc.mFormatID = kAudioFormatLinearPCM;
	strdesc.mFormatFlags = kLinearPCMFormatFlagIsPacked|kLinearPCMFormatFlagIsSignedInteger; //|kLinearPCMFormatFlagIsBigEndian; //|kLinearPCMFormatFlagIsFloat
	strdesc.mChannelsPerFrame = 2;
	strdesc.mSampleRate = 44100;
	strdesc.mFramesPerPacket = 1;
	strdesc.mBitsPerChannel = 16;
	strdesc.mBytesPerFrame = strdesc.mBitsPerChannel * strdesc.mChannelsPerFrame / 8;
	strdesc.mBytesPerPacket = strdesc.mBytesPerFrame * strdesc.mFramesPerPacket;

	AudioComponent comp;
	if (!(comp = AudioComponentFindNext(NULL, &desc))) return false;
	if (AudioComponentInstanceNew(comp, &ZL_IOS_AudioUnit) != noErr) return false;
	unsigned int enableInput = 0, enableOutput = 1;
	if (AudioUnitSetProperty(ZL_IOS_AudioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Input, 1, &enableInput, sizeof(enableInput)) != noErr) return false;
	if (AudioUnitSetProperty(ZL_IOS_AudioUnit, kAudioOutputUnitProperty_EnableIO, kAudioUnitScope_Output, 0, &enableOutput, sizeof(enableOutput)) != noErr) return false;
	if (AudioUnitSetProperty(ZL_IOS_AudioUnit, kAudioUnitProperty_StreamFormat, kAudioUnitScope_Input, 0, &strdesc, sizeof(strdesc)) != noErr) return false;

	// set audio callback
	AURenderCallbackStruct callback;
	memset(&callback, 0, sizeof(AURenderCallbackStruct));
	callback.inputProc = ZL_AudioOutputCallback;
	callback.inputProcRefCon = (void*)1;
	if (AudioUnitSetProperty(ZL_IOS_AudioUnit, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback)) != noErr) return false;

	//start audio
	if (AudioUnitInitialize(ZL_IOS_AudioUnit) != noErr) return false;
	if (AudioOutputUnitStart(ZL_IOS_AudioUnit) != noErr) return false;

	return true;
}

/*
void ZL_AudioClose()
{
	ZL_PlatformAudioClose();
	AudioOutputUnitStop(au);
	AURenderCallbackStruct callback;
	memset(&callback, 0, sizeof(AURenderCallbackStruct));
	AudioUnitSetProperty(au, kAudioUnitProperty_SetRenderCallback, kAudioUnitScope_Input, 0, &callback, sizeof(callback));
}
*/

#include "ZL_File.h"
#include "ZL_File_Impl.h"
#import <AVFoundation/AVAudioPlayer.h>

void* ZL_AudioPlayerOpen(ZL_String filename)
{
	assert(NSClassFromString(@"AVAudioPlayer")); //streaming audio needs AVFoundation.framework
	AVAudioPlayer* ap = [[NSClassFromString(@"AVAudioPlayer") alloc] initWithContentsOfURL:[NSURL fileURLWithPath:[NSString stringWithUTF8String:filename.c_str()]] error:nil];
	[ap setEnableRate:YES];
	[ap prepareToPlay];
	return ap;
}
void ZL_AudioPlayerPlay(void* audioPlayer, bool looped)
{
	[(id)audioPlayer setNumberOfLoops:(looped ? -1 : 0)];
	[(id)audioPlayer setCurrentTime:0];
	[(id)audioPlayer play];
	lastActiveAudioPlayer = (id)audioPlayer;
}
void ZL_AudioPlayerPause(void* audioPlayer) { [(id)audioPlayer pause]; if (audioPlayer == lastActiveAudioPlayer) lastActiveAudioPlayer = nil; }
void ZL_AudioPlayerStop(void* audioPlayer) { [(id)audioPlayer stop]; if (audioPlayer == lastActiveAudioPlayer) lastActiveAudioPlayer = nil; }
void ZL_AudioPlayerResume(void* audioPlayer) { [(id)audioPlayer play]; lastActiveAudioPlayer = (id)audioPlayer; }
void ZL_AudioPlayerRelease(void* audioPlayer) { [(id)audioPlayer release]; if (audioPlayer == lastActiveAudioPlayer) lastActiveAudioPlayer = nil; }
void ZL_AudioPlayerRate(void* audioPlayer, float rate) { [(id)audioPlayer setRate:rate]; lastActiveAudioPlayer = (id)audioPlayer; }
void ZL_AudioPlayerVolume(void* audioPlayer, float vol) { [(id)audioPlayer setVolume:vol]; lastActiveAudioPlayer = (id)audioPlayer; }

#endif
