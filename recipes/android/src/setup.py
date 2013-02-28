from distutils.core import setup, Extension
import os

setup(name='android',
      version='1.0',
      
      packages = [ 'android' ],
      
      ext_modules=[

        Extension(
            'android.core', ['core.c', 'android_jni.c'],
            libraries=[ 'sdl', 'log' ],
            library_dirs=[ 'libs/'+os.environ['ARCH'] ],
            ),
        Extension(
            'android.sound', ['sound.c', 'android_sound_jni.c',],
            libraries=[ 'sdl', 'log' ],
            library_dirs=[ 'libs/'+os.environ['ARCH'] ],
            ),

        ]
      )
