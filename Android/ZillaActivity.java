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

package org.zillalib;

import android.content.Context;
import android.view.WindowManager;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.hardware.Sensor;
import android.hardware.SensorManager;

public class ZillaActivity extends android.app.Activity implements SurfaceHolder.Callback
{
	native void NativeOnCreate(String apkPath);
	static native void NativeOnResume();
	static native void NativeOnPause(boolean isFinishing);
	static native void NativeOnDestroy();
	static native void NativeSetSurface(Surface surface, int w, int h);
	static native void NativeTouch(int x, int y, int action, int pointerId, int pressure, int radius);
	static native void NativeKey(int keyCode, int down);
	static native void NativeText(String text);
	static native void NativeGyroscope(float x, float y, float z);
	static native void NativeAccelerometer(float x, float y, float z);

	android.view.SurfaceView mView = null;
	android.content.SharedPreferences mSettings = null;
	android.content.SharedPreferences.Editor mEditor = null;
	ZillaAudio mAudio = null;
	ZillaAccelerometer mAccelerometer = null;
	boolean OverridesVolumeKeys = false;
	int UIVisibilityFlags = android.view.View.SYSTEM_UI_FLAG_LOW_PROFILE;

	@SuppressWarnings("deprecation") @Override protected void onCreate(android.os.Bundle savedInstanceState)
	{
		android.util.Log.v("ZILLALIB", "Started onCreate function");
		super.onCreate(savedInstanceState);

		android.content.pm.ApplicationInfo appInfo;
		try { appInfo = getPackageManager().getApplicationInfo(getPackageName(), 0); }
		catch (Exception e) { throw new RuntimeException("Unable to locate assets, aborting..."); }

		requestWindowFeature(android.view.Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON, WindowManager.LayoutParams.FLAG_FULLSCREEN | WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);

		mView = new android.view.SurfaceView(this);
		mView.getHolder().addCallback(this);
		mView.setFocusable(true);
		mView.setFocusableInTouchMode(true);
		mView.setContentDescription("game");

		mView.setOnKeyListener(new android.view.View.OnKeyListener() { @Override public boolean onKey(android.view.View v, int keyCode, KeyEvent event)
		{
			if ((keyCode == KeyEvent.KEYCODE_VOLUME_DOWN || keyCode == KeyEvent.KEYCODE_VOLUME_UP) && !OverridesVolumeKeys)
				return false;
			int action = event.getAction();
			//android.util.Log.v("ZILLALIB", "keyCode: " + keyCode + " - action: " + action + " - unicode: " + event.getUnicodeChar());
			if (keyCode == KeyEvent.KEYCODE_UNKNOWN && action == KeyEvent.ACTION_MULTIPLE)
				{ NativeText(event.getCharacters()); return true; }
			if (keyCode != KeyEvent.KEYCODE_UNKNOWN)
				NativeKey(keyCode, (action == KeyEvent.ACTION_DOWN ? 1 : 0));
			if (action == KeyEvent.ACTION_DOWN)
			{
				if (keyCode == KeyEvent.KEYCODE_DEL) NativeText("\b");
				else if (keyCode == KeyEvent.KEYCODE_ENTER) NativeText("\n");
				else if (keyCode == KeyEvent.KEYCODE_TAB) NativeText("\t");
				else
				{
					int u = event.getUnicodeChar();
					if (u == 8 || u >= 32) NativeText(String.valueOf((char)u));
				}
			}
			return true;
		}});

		mView.setOnTouchListener(new android.view.View.OnTouchListener() { @Override public boolean onTouch(android.view.View v, android.view.MotionEvent event)
		{
			int zvaction, action = event.getAction();
			int actionCode = (action & MotionEvent.ACTION_MASK);
			if      (actionCode == MotionEvent.ACTION_DOWN)         zvaction = 0;
			else if (actionCode == MotionEvent.ACTION_UP)           zvaction = 1;
			else if (actionCode == MotionEvent.ACTION_MOVE)         zvaction = 2;
			else if (actionCode == MotionEvent.ACTION_CANCEL)       zvaction = 1;
			else if (actionCode == MotionEvent.ACTION_OUTSIDE)      zvaction = 1;
			else if (actionCode == MotionEvent.ACTION_POINTER_DOWN) zvaction = 0;
			else if (actionCode == MotionEvent.ACTION_POINTER_UP)   zvaction = 1;
			else return false;

			int actionPointer = -1, actionPointerZVAction = 0;
			if (actionCode == MotionEvent.ACTION_POINTER_DOWN || actionCode == MotionEvent.ACTION_POINTER_UP)
			{
				actionPointer = (action & MotionEvent.ACTION_POINTER_INDEX_MASK) >> MotionEvent.ACTION_POINTER_INDEX_SHIFT;
				actionPointerZVAction = zvaction;
				zvaction = 2;
			}

			for( int i = 0; i < event.getPointerCount(); i++ )
			{
				int pid = event.getPointerId(i);
				int pidzvaction = (actionPointer == i ? actionPointerZVAction : zvaction);
				NativeTouch((int)event.getX(i), (int)event.getY(i), pidzvaction, pid, (int)(event.getPressure(i) * 1000.0), (int)(event.getSize(i) * 1000.0));
			}
			return true;
		}});

		android.widget.FrameLayout frame = new android.widget.FrameLayout(this);
		frame.addView(mView);
		if (android.os.Build.VERSION.SDK_INT >= 14)
		{
			final Runnable r = new Runnable() { public void run() { getWindow().getDecorView().setSystemUiVisibility(UIVisibilityFlags); } };
			r.run();
			frame.setOnSystemUiVisibilityChangeListener(new android.view.View.OnSystemUiVisibilityChangeListener()
			{
				private android.os.Handler h = new android.os.Handler();
				@Override public void onSystemUiVisibilityChange(int visibility)
				{
					if (visibility == UIVisibilityFlags) return;
					h.removeCallbacks(r);
					h.postDelayed(r, 2000);
				}
			});
		}
		setContentView(frame);
		NativeOnCreate(appInfo.sourceDir);
	}

	@Override protected void onPause()
	{
		NativeOnPause(isFinishing());
		if (mAudio != null) mAudio.onPause();
		super.onPause();
	}

	@Override protected void onResume()
	{
		if (android.os.Build.VERSION.SDK_INT >= 14) getWindow().getDecorView().setSystemUiVisibility(android.view.View.SYSTEM_UI_FLAG_LOW_PROFILE);
		if (mAudio != null) mAudio.onResume();
		NativeOnResume();
		super.onResume();
	}

	@Override protected void onDestroy()
	{
		if (mAudio != null) { mAudio.release(); mAudio = null; }
		NativeOnDestroy();
		super.onDestroy();
	}

	@Override public void onBackPressed()
	{
		NativeKey(4, 1);
		NativeKey(4, 0);
	}

	@Override public void surfaceChanged(SurfaceHolder holder, int format, int w, int h) { NativeSetSurface(holder.getSurface(), w, h); }
	@Override public void surfaceCreated(SurfaceHolder holder) { }
	@Override public void surfaceDestroyed(SurfaceHolder holder) { NativeSetSurface(null, 0, 0); }

	public void setFlags(boolean AllowAnyOrientation, boolean WantsLandscape, boolean OverridesVolumeKeys, boolean Immersive)
	{
		this.OverridesVolumeKeys = OverridesVolumeKeys;
		if (AllowAnyOrientation)
			setRequestedOrientation(android.content.pm.ActivityInfo.SCREEN_ORIENTATION_SENSOR);
		else if (WantsLandscape)
			//sensorLandscape 6 Would like to have the screen in landscape orientation, but can use the sensor to change which direction the screen is facing.
			setRequestedOrientation(android.os.Build.VERSION.SDK_INT >= 9 ? 6 : android.content.pm.ActivityInfo.SCREEN_ORIENTATION_LANDSCAPE);
		else
			//sensorPortait 7 Would like to have the screen in portrait orientation, but can use the sensor to change which direction the screen is facing.
			setRequestedOrientation(android.os.Build.VERSION.SDK_INT >= 9 ? 7 : android.content.pm.ActivityInfo.SCREEN_ORIENTATION_PORTRAIT);
		if (Immersive && android.os.Build.VERSION.SDK_INT >= 19)
			UIVisibilityFlags = android.view.View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY | android.view.View.SYSTEM_UI_FLAG_HIDE_NAVIGATION;
	}

	public void doFinish()
	{
		runOnUiThread(new Runnable()
		{
			@Override public void run()
			{
				finish();
				android.os.Process.killProcess(android.os.Process.myPid());
			}
		});
	}

	public Object startAudio()
	{
		if (mAudio == null) mAudio = new ZillaAudio(this);
		return mAudio;
	}

	public int getAudioFramesPerBuffer()
	{
		if (android.os.Build.VERSION.SDK_INT < 17) return 880; //seen on an old Android device
		try
		{
			java.lang.reflect.Method amGetProperty = android.media.AudioManager.class.getDeclaredMethod("getProperty", new Class[] { String.class });
			return Integer.parseInt((String)amGetProperty.invoke(getSystemService(Context.AUDIO_SERVICE), "android.media.property.OUTPUT_FRAMES_PER_BUFFER"));
		}
		catch (Exception e) { return 880; }
	}

	public String getUID()
	{
		//UID = System.getString(getContentResolver(), System.ANDROID_ID);
		//UID = android.provider.Settings.Secure.getString(getContentResolver(), android.provider.Settings.Secure.ANDROID_ID);
		return android.provider.Settings.Secure.getString(getContentResolver(), android.provider.Settings.Secure.ANDROID_ID);
	}

	public void settingsInit(String prefsName)
	{
		mSettings = getSharedPreferences(prefsName, 0);
	}
	public String settingsGet(String key)
	{
		return mSettings.getString(key, "");
	}
	public void settingsSet(String key, String value)
	{
		if (mEditor == null) mEditor = mSettings.edit();
		mEditor.putString(key, value);
	}
	public void settingsDel(String key)
	{
		if (mEditor == null) mEditor = mSettings.edit();
		mEditor.remove(key);
	}
	public boolean settingsHas(String key)
	{
		return mSettings.contains(key);
	}
	public void settingsSynchronize()
	{
		if (mEditor != null) { mEditor.commit(); mEditor = null; }
	}

	public void openExternalUrl(String url)
	{
		startActivity(new android.content.Intent(android.content.Intent.ACTION_VIEW, android.net.Uri.parse(url)));
	}

	public int softKeyboard(int action)
	{
		if      (action == 0) ((android.view.inputmethod.InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE)).toggleSoftInputFromWindow(mView.getWindowToken(), 0, 0);
		else if (action == 1) ((android.view.inputmethod.InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE)).showSoftInput(mView, 0);
		else if (action == 2) ((android.view.inputmethod.InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE)).hideSoftInputFromWindow(mView.getWindowToken(), 0);
		else if (action == 3) return (((android.view.inputmethod.InputMethodManager)getSystemService(Context.INPUT_METHOD_SERVICE)).isActive() ? 1 : 0);
		return 0;
	}

	@SuppressWarnings("deprecation") public void vibrate(int durationMs)
	{
		android.os.Vibrator v = ((android.os.Vibrator)getSystemService(VIBRATOR_SERVICE));
		if (android.os.Build.VERSION.SDK_INT < 26) v.vibrate(durationMs);
		else v.vibrate(android.os.VibrationEffect.createOneShot(durationMs,10));
	}

	public void joystickStatus(int deviceIndex, boolean open)
	{
		if (mAccelerometer == null) mAccelerometer = new ZillaAccelerometer(this);
		if      (deviceIndex == 0 &&  open) mAccelerometer.startGyroscopeSensor();
		else if (deviceIndex == 1 &&  open) mAccelerometer.startAccelerometerSensor();
		else if (deviceIndex == 0 && !open) mAccelerometer.stopGyroscopeSensor();
		else if (deviceIndex == 1 && !open) mAccelerometer.stopAccelerometerSensor();
	}

	private class ZillaAccelerometer implements android.hardware.SensorEventListener
	{
		private SensorManager mSensorManager = null;
		private ZillaActivity mContext;
		private boolean mUseGyroscope = false;
		private boolean mUseAccelerometer = false;

		public ZillaAccelerometer(ZillaActivity context)
		{
			mContext = context;
		}

		public void startGyroscopeSensor()
		{
			if (mUseGyroscope) return;
			if (mSensorManager == null) mSensorManager = (SensorManager)mContext.getSystemService(Context.SENSOR_SERVICE);
			if (mSensorManager != null)
				if (mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), SensorManager.SENSOR_DELAY_GAME))
					mUseGyroscope = true;
		}

		public void startAccelerometerSensor()
		{
			if (mUseAccelerometer) return;
			if (mSensorManager == null) mSensorManager = (SensorManager)mContext.getSystemService(Context.SENSOR_SERVICE);
			if (mSensorManager != null)
				if (mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_GAME))
					mUseAccelerometer = true;
		}

		public synchronized void stopGyroscopeSensor()
		{
			if (mSensorManager == null || !mUseGyroscope) return;
			mSensorManager.unregisterListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE));
			mUseGyroscope = false;
			if (!mUseAccelerometer) mSensorManager = null;
		}

		public synchronized void stopAccelerometerSensor()
		{
			if (mSensorManager == null || !mUseAccelerometer) return;
			mSensorManager.unregisterListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER));
			mUseAccelerometer = false;
			if (!mUseGyroscope) mSensorManager = null;
		}

		public synchronized void onPause()
		{
			if (mSensorManager == null) return;
			if (mUseGyroscope) mSensorManager.unregisterListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE));
			if (mUseAccelerometer) mSensorManager.unregisterListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER));
			mSensorManager = null;
		}

		public synchronized void onResume()
		{
			if (mSensorManager != null) return;
			mSensorManager = (SensorManager)mContext.getSystemService(Context.SENSOR_SERVICE);
			if (mSensorManager == null) { mUseGyroscope = false; mUseAccelerometer = false; return; }
			if (mUseGyroscope) mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_GYROSCOPE), SensorManager.SENSOR_DELAY_GAME);
			if (mUseAccelerometer) mSensorManager.registerListener(this, mSensorManager.getDefaultSensor(Sensor.TYPE_ACCELEROMETER), SensorManager.SENSOR_DELAY_GAME);
		}

		@Override public void onSensorChanged(android.hardware.SensorEvent event)
		{
			if (event.sensor.getType() == Sensor.TYPE_ACCELEROMETER)
				NativeAccelerometer(event.values[0], event.values[1], event.values[2]);
			else
				NativeGyroscope(event.values[0], event.values[1], event.values[2]);
		}

		@Override public void onAccuracyChanged(Sensor s, int a) { }
	}

	private class ZillaAudio
	{
		private ZillaActivity mContext;
		private class Stream { java.io.FileInputStream apkFileStream; android.media.MediaPlayer player; boolean paused; }
		java.util.ArrayList<Stream> streams = new java.util.ArrayList<Stream>();

		public ZillaAudio(ZillaActivity context) { mContext = context; }

		public void onPause()  { for (Stream s : streams) { if ((s.paused = s.player.isPlaying())) s.player.pause(); } }
		public void onResume() { for (Stream s : streams) { if (s.paused) s.player.start(); } }
		public void release()  { for (Stream s : streams) control(s, 9, 0); }

		public Object open(int offset, int size)
		{
			//android.util.Log.d("ZL_AUDIO", "open apk file Offset: " + offset + " - size: " + size);
			try
			{
				Stream s = new Stream();
				s.apkFileStream = new java.io.FileInputStream(mContext.getPackageManager().getApplicationInfo(mContext.getPackageName(), 0).sourceDir);
				s.player = new android.media.MediaPlayer();
				s.player.setDataSource(s.apkFileStream.getFD(), offset, size);
				s.player.prepare();
				streams.add(s);
				return s;
			}
			catch (Exception e) { return null; }
		}

		public void control(Object stream, int mode, int arg)
		{
			//android.util.Log.d("ZL_AUDIO", "Control streamed audio - Mode: " + mode);
			Stream s = (Stream)stream;
			if      (mode == 0) { s.player.setLooping(arg != 0); if (!s.player.isPlaying()) s.player.start(); }
			else if (mode == 1) { if (s.player.isPlaying()) s.player.stop(); }
			else if (mode == 2) { if (s.player.isPlaying()) s.player.pause(); }
			else if (mode == 9)
			{
				if (s.player.isPlaying()) s.player.stop();
				s.player.release();
				try { s.apkFileStream.close(); } catch (Exception e) { }
				streams.remove(s);
			}
		}
	}

	//private class ZillaSurfaceView extends android.view.SurfaceView
	//{
	//	public ZillaSurfaceView(Context context) { super(context); }
	//	@Override public android.view.inputmethod.InputConnection onCreateInputConnection(android.view.inputmethod.EditorInfo ei)
	//	{
	//		ei.inputType = android.text.InputType.TYPE_NULL;
	//		return new ZillaInputConnection(this, false);
	//	}
	//}
	//private class ZillaInputConnection extends android.view.inputmethod.BaseInputConnection
	//{
	//	public ZillaInputConnection(android.view.View targetView, boolean fullEditor) { super(targetView, fullEditor); }
	//	@Override public boolean sendKeyEvent(KeyEvent event)
	//	{
	//		return false;
	//	}
	//	@Override public boolean commitText(CharSequence text, int newCursorPosition)
	//	{
	//		NativeText(text.toString());
	//		return false;
	//	}
	//}
}
