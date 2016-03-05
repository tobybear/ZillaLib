//
// File which forces the linker to actually link to libGoogleAdMobAds.a
//
#if defined(__APPLE__)
#include "AvailabilityMacros.h"
#endif
#ifdef MAC_OS_X_VERSION_10_3
#include "TargetConditionals.h"
#endif
#if TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
@interface GADBannerView : NSObject @end
static NSInteger gadbannerviewver = [GADBannerView version];
@interface GADRequest : NSObject @end
static NSInteger gadrequest = [GADRequest version];
#endif

static UIView* ZL_IOSAdBannerView = nil;

//------------------------------------------------------------------------------------------------

#define GAD_SIZE_320x50 CGSizeMake(320, 50)
@class GADRequestError;
@class GADBannerView;
#define GAD_SIMULATOR_ID @"Simulator"
typedef enum { kGADGenderUnknown, kGADGenderMale, kGADGenderFemale } GADGender;
@interface GADRequest : NSObject <NSCopying>
+ (GADRequest *)request;
@property (nonatomic, retain) NSDictionary *additionalParameters;
+ (NSString *)sdkVersion;
@property (nonatomic, retain) NSArray *testDevices;
@property (nonatomic, assign) GADGender gender;
@property (nonatomic, retain) NSDate *birthday;
- (void)setBirthdayWithMonth:(NSInteger)m day:(NSInteger)d year:(NSInteger)y;
- (void)setLocationWithLatitude:(CGFloat)latitude longitude:(CGFloat)longitude accuracy:(CGFloat)accuracyInMeters;
- (void)setLocationWithDescription:(NSString *)locationDescription;
@property (nonatomic, retain) NSMutableArray *keywords;
- (void)addKeyword:(NSString *)keyword;
@property (nonatomic, getter=isTesting) BOOL testing;
@end
@protocol GADBannerViewDelegate <NSObject>
@optional
- (void)adViewDidReceiveAd:(GADBannerView *)view;
- (void)adView:(GADBannerView *)view didFailToReceiveAdWithError:(GADRequestError *)error;
- (void)adViewWillPresentScreen:(GADBannerView *)adView;
- (void)adViewWillDismissScreen:(GADBannerView *)adView;
- (void)adViewDidDismissScreen:(GADBannerView *)adView;
- (void)adViewWillLeaveApplication:(GADBannerView *)adView;
@end
@interface GADBannerView : UIView
@property (nonatomic, copy) NSString *adUnitID;
@property (nonatomic, assign) UIViewController *rootViewController;
@property (nonatomic, assign) NSObject<GADBannerViewDelegate> *delegate;
- (void)loadRequest:(GADRequest *)request;
@property (nonatomic, readonly) BOOL hasAutoRefreshed;
@end

//------------------------------------------------------------------------------------------------

void ZL_IOSAdDisplayShow(bool show, short pos, short x, short y)
{
	if (ZL_IOS_viewcontroller == nil || ZL_IOSAdBannerView == nil) return;
	if (show && pos >= 0)
	{	
		CGRect frm;
		frm.size = GAD_SIZE_320x50;
		while (frm.size.width*csf < ZL_IOS_WindowWidth/2) { frm.size.width *= 2; frm.size.height *= 2; }
		if (frm.size.width > GAD_SIZE_320x50.width)
		{
			ZL_IOSAdBannerView.transform = CGAffineTransformMakeScale(frm.size.width/GAD_SIZE_320x50.width, frm.size.height/GAD_SIZE_320x50.height);
		}
		frm.origin.x = (pos == 0 ? (CGFloat)x/csf : (pos % 3 == 1 ? 0 : (ZL_IOS_WindowWidth/csf-frm.size.width) / (pos % 3 == 2 ? 2 : 1)));
		frm.origin.y = (pos == 0 ? (CGFloat)y/csf : (pos >= 7     ? 0 : (ZL_IOS_WindowHeight/csf-frm.size.height) / (pos >= 4 ? 2 : 1)));
		[ZL_IOSAdBannerView setFrame:frm];
	}

	if ([ZL_IOSAdBannerView isHidden] != (show ? NO : YES))
	{
		[ZL_IOSAdBannerView setBackgroundColor:[UIColor magentaColor]];
		[ZL_IOSAdBannerView setHidden:(show ? NO : YES)];
		if (show)
		{
			assert(NSClassFromString(@"GADRequest")); //needs libGoogleAdMobAds.a
			GADRequest *request = [NSClassFromString(@"GADRequest") request];
			request.testDevices = [NSArray arrayWithObjects:GAD_SIMULATOR_ID,/*@"aaa",@"bbb",*/nil];
			[(GADBannerView*)ZL_IOSAdBannerView loadRequest:request];
		}
		else
		{
			[(GADBannerView*)ZL_IOSAdBannerView loadRequest:nil];
		}
	}
}

int ZL_IOSAdDisplayInit(const ZL_String& admob_unit_id, bool show, short pos, short x, short y)
{
	if (ZL_IOS_viewcontroller == nil || ZL_IOSAdBannerView != nil) return GAD_SIZE_320x50.width*csf;
	assert(NSClassFromString(@"GADBannerView")); //needs libGoogleAdMobAds.a
	ZL_IOSAdBannerView = [[NSClassFromString(@"GADBannerView") alloc] initWithFrame:CGRectMake(0,0,GAD_SIZE_320x50.width,GAD_SIZE_320x50.height)];
	[(GADBannerView*)ZL_IOSAdBannerView setRootViewController:ZL_IOS_viewcontroller];
	[(GADBannerView*)ZL_IOSAdBannerView setAdUnitID:[NSString stringWithCString:admob_unit_id.c_str() encoding:NSUTF8StringEncoding]];
	[ZL_IOSAdBannerView setHidden:YES];
	ZL_IOSAdDisplayShow(show, pos, x, y);
	[ZL_IOS_viewcontroller.view addSubview:ZL_IOSAdBannerView];
	return ZL_IOSAdBannerView.frame.size.width*csf;
}
