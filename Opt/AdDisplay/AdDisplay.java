package org.zillalib;

import android.util.TypedValue;
import android.view.Gravity;
import android.view.View;
import android.widget.FrameLayout;
import android.widget.FrameLayout.LayoutParams;
import com.google.ads.AdRequest;
import com.google.ads.AdSize;
import com.google.ads.AdView;

public class AdDisplay
{
	private static AdView adView = null;
	private static LayoutParams adViewLP = null;
	static int adWidth = 0;
	private static RunnableDoShow DoShow = null;
	
	static void Init(final ZillaActivity activity, final String admob_unit_id, boolean show, short pos_like_numpad, short x, short y)
	{
		if (adView != null) return;
		DoShow = new RunnableDoShow();
	    adWidth = (int)TypedValue.applyDimension(TypedValue.COMPLEX_UNIT_DIP, 320, activity.getResources().getDisplayMetrics());
	    adViewLP = new LayoutParams(adWidth, adWidth*50/320, Gravity.TOP | Gravity.LEFT);
		DoShow.show = show;
	    DoShow.dolayout = DoPrepareLayout(pos_like_numpad, x, y);
	    DoShow.isvisible = false;
		activity.runOnUiThread(new Runnable()
		{
			@Override public void run()
			{
			    adView = new com.google.ads.AdView(activity, AdSize.BANNER, admob_unit_id);
			    if (DoShow.show) DoShow.run();
			    else adView.setVisibility(View.GONE);
			    ((FrameLayout)activity.mView.getParent()).addView(adView);
			}
		});
	}
	
	static void Show(ZillaActivity activity, boolean show,  short pos_like_numpad, short x, short y)
	{
		DoShow.dolayout = (show ? DoPrepareLayout(pos_like_numpad, x, y) : false);
		if (DoShow.dolayout || DoShow.isvisible != show) { DoShow.show = show; activity.runOnUiThread(DoShow); }
	}
	
	static private boolean DoPrepareLayout(short pos_like_numpad, short x, short y)
	{
		if (pos_like_numpad < 0) return false;
		if (pos_like_numpad == 0)
		{
			if (adViewLP.gravity == (Gravity.TOP | Gravity.LEFT ) && adViewLP.leftMargin == x && adViewLP.topMargin == y) return false; 
			adViewLP.gravity = Gravity.TOP | Gravity.LEFT;
			adViewLP.leftMargin = x;
			adViewLP.topMargin = y;
		}
		else if (pos_like_numpad > 0)	
		{
			int g = ((pos_like_numpad >= 7 ? Gravity.TOP : (pos_like_numpad >= 4 ? Gravity.CENTER_VERTICAL : Gravity.BOTTOM))
		           | (pos_like_numpad % 3 == 1 ? Gravity.LEFT : (pos_like_numpad % 3 == 2 ? Gravity.CENTER_HORIZONTAL : Gravity.RIGHT)));
			if (adViewLP.gravity == g && adViewLP.leftMargin == 0 && adViewLP.topMargin == 0) return false;
			adViewLP.leftMargin = adViewLP.topMargin = 0;
			adViewLP.gravity = g; 
		}
		return true;
	}
	
	private static class RunnableDoShow implements Runnable
	{
		public boolean show, isvisible, dolayout;
		@Override public void run()
		{
			if (dolayout) adView.setLayoutParams(adViewLP);
			if (isvisible != show)
			{
				adView.setVisibility(show ? View.VISIBLE : View.GONE);
				if (show)
				{
					AdRequest adRequest = new AdRequest();
					adRequest.addTestDevice(AdRequest.TEST_EMULATOR);
					adRequest.addTestDevice("41E2D3E199AAE94BA3585F20C10C0460");
					adView.loadAd(adRequest);
				}
				else adView.stopLoading();
				isvisible = show;
			}
		}
	}
}
