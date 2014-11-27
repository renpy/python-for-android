def init():
    pass

# Vibration support.
cdef extern void android_vibrate(double)

def vibrate(s):
    android_vibrate(s)

# DisplayMetrics information.
cdef extern int android_get_dpi()

def get_dpi():
    return android_get_dpi()

# Build info.
cdef extern char* BUILD_MANUFACTURER
cdef extern char* BUILD_MODEL
cdef extern char* BUILD_PRODUCT
cdef extern char* BUILD_VERSION_RELEASE

cdef extern void android_get_buildinfo()

class BuildInfo:
    MANUFACTURER = None
    MODEL = None
    PRODUCT = None
    VERSION_RELEASE = None

def get_buildinfo():
    android_get_buildinfo()
    binfo = BuildInfo()
    binfo.MANUFACTURER = BUILD_MANUFACTURER
    binfo.MODEL = BUILD_MODEL
    binfo.PRODUCT = BUILD_PRODUCT
    binfo.VERSION_RELEASE = BUILD_VERSION_RELEASE
    return binfo

# Action send
cdef extern void android_action_send(char*, char*, char*, char*, char*)
def action_send(mimetype, filename=None, subject=None, text=None,
        chooser_title=None):
    cdef char *j_mimetype = <bytes>mimetype
    cdef char *j_filename = NULL
    cdef char *j_subject = NULL
    cdef char *j_text = NULL
    cdef char *j_chooser_title = NULL
    if filename is not None:
        j_filename = <bytes>filename
    if subject is not None:
        j_subject = <bytes>subject
    if text is not None:
        j_text = <bytes>text
    if chooser_title is not None:
        j_chooser_title = <bytes>chooser_title
    android_action_send(j_mimetype, j_filename, j_subject, j_text,
            j_chooser_title)

# -------------------------------------------------------------------
# URL Opening.
cdef extern void android_open_url(char *url)
def open_url(url):
    android_open_url(url)

# Web browser support.
class AndroidBrowser(object):
    def open(self, url, new=0, autoraise=True):
        open_url(url)
    def open_new(self, url):
        open_url(url)
    def open_new_tab(self, url):
        open_url(url)

import webbrowser
webbrowser.register('android', AndroidBrowser, None, -1)

