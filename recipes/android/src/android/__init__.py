# legacy import
from android._android import *

import os
import android.apk as apk

expansion = os.environ.get("ANDROID_EXPANSION", None)
assets = apk.APK(apk=expansion)

