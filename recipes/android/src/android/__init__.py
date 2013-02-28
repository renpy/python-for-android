#@PydevCodeAnalysisIgnore
import android.core
import android.apk

from android.core import *

assets = android.apk.APK()

# Web browser support.
class AndroidBrowser(object):
    
    def open(self, url, new=0, autoraise=True):
        open_url(url)
        
    def open_new(self, url):
        open_url(url) 
        
    def open_new_tab(self, url):
        open_url(url) 
        
import webbrowser
webbrowser.register("android", AndroidBrowser, None, -1)
