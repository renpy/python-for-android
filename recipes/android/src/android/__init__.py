from android._android import *

import os
import android.apk as apk

expansion = os.environ.get("ANDROID_EXPANSION", None)
assets = apk.APK(apk=expansion)

def init():
    import androidembed
    androidembed.close_window()

from jnius import autoclass

PythonSDLActivity = autoclass('org.renpy.android.PythonSDLActivity')
activity = PythonSDLActivity.mActivity

def vibrate(s):
    """
    Vibrate for `s` seconds.
    """

    activity.vibrate(s)
