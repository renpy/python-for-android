/*
    SDL - Simple DirectMedia Layer
    Copyright (C) 1997-2010 Sam Lantinga

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

    Sam Lantinga
    slouken@libsdl.org
*/
#include "SDL_config.h"
#include "SDL_version.h"

#include <android/log.h>

#if SDL_VIDEO_RENDER_OGL_ES

#if SDL_VERSION_ATLEAST(1,3,0)
#include "SDL_video.h"
#include "SDL_sysvideo.h"
#else
#include "SDL_video-1.3.h"
#include "SDL_sysvideo-1.3.h"
#endif
#include "SDL_pixels_c.h"
#include "SDL_rect_c.h"
#if SDL_VERSION_ATLEAST(1,3,0)
#include "SDL_yuv_sw_c.h"
#endif
#ifdef ANDROID
#include <android/log.h>
#endif

#include <GLES2/gl2.h>

#define ATTRIB_VERTEX 0
#define ATTRIB_TEXCOORD 1

#if defined(__QNXNTO__)
/* Include QNX system header to check QNX version later */
#include <sys/neutrino.h>
#endif /* __QNXNTO__ */

#if defined(SDL_VIDEO_DRIVER_QNXGF)  ||  \
    defined(SDL_VIDEO_DRIVER_PHOTON) ||  \
    defined(SDL_VIDEO_DRIVER_PANDORA)

/* Empty function stub to get OpenGL ES 1.x support without  */
/* OpenGL ES extension GL_OES_draw_texture supported         */
GL_API void GL_APIENTRY
glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height)
{
    return;
}

#endif /* QNXGF || PHOTON || PANDORA */

/* OpenGL ES 1.1 renderer implementation, based on the OpenGL renderer */

static const float inv255f = 1.0f / 255.0f;

static SDL_Renderer *GLES_CreateRenderer(SDL_Window * window, Uint32 flags);
static int GLES_ActivateRenderer(SDL_Renderer * renderer);
static int GLES_DisplayModeChanged(SDL_Renderer * renderer);
static int GLES_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture);
static int GLES_QueryTexturePixels(SDL_Renderer * renderer,
                                   SDL_Texture * texture, void **pixels,
                                   int *pitch);
static int GLES_SetTexturePalette(SDL_Renderer * renderer,
                                  SDL_Texture * texture,
                                  const SDL_Color * colors, int firstcolor,
                                  int ncolors);
static int GLES_GetTexturePalette(SDL_Renderer * renderer,
                                  SDL_Texture * texture, SDL_Color * colors,
                                  int firstcolor, int ncolors);
static int GLES_SetTextureColorMod(SDL_Renderer * renderer,
                                   SDL_Texture * texture);
static int GLES_SetTextureAlphaMod(SDL_Renderer * renderer,
                                   SDL_Texture * texture);
static int GLES_SetTextureBlendMode(SDL_Renderer * renderer,
                                    SDL_Texture * texture);
static int GLES_SetTextureScaleMode(SDL_Renderer * renderer,
                                    SDL_Texture * texture);
static int GLES_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                              const SDL_Rect * rect, const void *pixels,
                              int pitch);
static int GLES_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                            const SDL_Rect * rect, int markDirty,
                            void **pixels, int *pitch);
static void GLES_UnlockTexture(SDL_Renderer * renderer,
                               SDL_Texture * texture);
static void GLES_DirtyTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                              int numrects, const SDL_Rect * rects);
static int GLES_RenderDrawPoints(SDL_Renderer * renderer,
                                 const SDL_Point * points, int count);
static int GLES_RenderDrawLines(SDL_Renderer * renderer,
                                const SDL_Point * points, int count);
static int GLES_RenderDrawRects(SDL_Renderer * renderer,
                                const SDL_Rect ** rects, int count);
static int GLES_RenderFillRects(SDL_Renderer * renderer,
                                const SDL_Rect ** rects, int count);
static int GLES_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                           const SDL_Rect * srcrect,
                           const SDL_Rect * dstrect);
static void GLES_RenderPresent(SDL_Renderer * renderer);
static void GLES_DestroyTexture(SDL_Renderer * renderer,
                                SDL_Texture * texture);
static void GLES_DestroyRenderer(SDL_Renderer * renderer);


SDL_RenderDriver GL_ES_RenderDriver = {
    GLES_CreateRenderer,
    {
     "opengl_es",
     (SDL_RENDERER_SINGLEBUFFER | SDL_RENDERER_PRESENTDISCARD |
      SDL_RENDERER_PRESENTVSYNC | SDL_RENDERER_ACCELERATED),
     (SDL_TEXTUREMODULATE_NONE | SDL_TEXTUREMODULATE_COLOR |
      SDL_TEXTUREMODULATE_ALPHA),
     (SDL_BLENDMODE_NONE | SDL_BLENDMODE_MASK |
      SDL_BLENDMODE_BLEND | SDL_BLENDMODE_ADD | SDL_BLENDMODE_MOD),
     (SDL_TEXTURESCALEMODE_NONE | SDL_TEXTURESCALEMODE_FAST |
      SDL_TEXTURESCALEMODE_SLOW), 5,
     {
      /* OpenGL ES 1.x supported formats list */
      SDL_PIXELFORMAT_ABGR4444,
      SDL_PIXELFORMAT_ABGR1555,
      SDL_PIXELFORMAT_BGR565,
#ifdef ANDROID
      SDL_PIXELFORMAT_RGB565, // Android is special, GL pixelformat has R and B channels not swapped
      SDL_PIXELFORMAT_RGBA5551,
      SDL_PIXELFORMAT_RGBA4444,
#endif
      SDL_PIXELFORMAT_BGR24,
      SDL_PIXELFORMAT_ABGR8888},
     0,
     0}
};

typedef struct
{
    SDL_GLContext context;
    SDL_bool updateSize;
    int blendMode;

#ifndef APIENTRY
#define APIENTRY
#endif

    SDL_bool useDrawTexture;
    SDL_bool GL_OES_draw_texture_supported;

} GLES_RenderData;

typedef struct
{
    GLuint texture;
    GLenum type;
    GLfloat texw;
    GLfloat texh;
    GLenum format;
    GLenum formattype;
    void *pixels;
    int pitch;
    SDL_DirtyRectList dirty;
} GLES_TextureData;

static void
GLES_SetError(const char *prefix, GLenum result)
{
    const char *error;

    switch (result) {
    case GL_NO_ERROR:
        error = "GL_NO_ERROR";
        break;
    case GL_INVALID_ENUM:
        error = "GL_INVALID_ENUM";
        break;
    case GL_INVALID_VALUE:
        error = "GL_INVALID_VALUE";
        break;
    case GL_INVALID_OPERATION:
        error = "GL_INVALID_OPERATION";
        break;
    case GL_OUT_OF_MEMORY:
        error = "GL_OUT_OF_MEMORY";
        break;
    default:
        error = "UNKNOWN";
        break;
    }
    SDL_SetError("%s: %s", prefix, error);
}

static int
GLES_LoadFunctions(GLES_RenderData * data)
{
    return 0;
}

SDL_Renderer *
GLES_CreateRenderer(SDL_Window * window, Uint32 flags)
{

    SDL_Renderer *renderer;
    GLES_RenderData *data;
    GLint value;
    int doublebuffer;

#if SDL_VERSION_ATLEAST(1,3,0)
    if (!(window->flags & SDL_WINDOW_OPENGL)) {
        if (SDL_RecreateWindow(window, window->flags | SDL_WINDOW_OPENGL) < 0) {
            return NULL;
        }
    }
#endif

    renderer = (SDL_Renderer *) SDL_calloc(1, sizeof(*renderer));
    if (!renderer) {
        SDL_OutOfMemory();
        return NULL;
    }

    data = (GLES_RenderData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        GLES_DestroyRenderer(renderer);
        SDL_OutOfMemory();
        return NULL;
    }

    renderer->ActivateRenderer = GLES_ActivateRenderer;
    renderer->DisplayModeChanged = GLES_DisplayModeChanged;
    renderer->CreateTexture = GLES_CreateTexture;
    renderer->QueryTexturePixels = GLES_QueryTexturePixels;
    renderer->SetTexturePalette = GLES_SetTexturePalette;
    renderer->GetTexturePalette = GLES_GetTexturePalette;
    renderer->SetTextureColorMod = GLES_SetTextureColorMod;
    renderer->SetTextureAlphaMod = GLES_SetTextureAlphaMod;
    renderer->SetTextureBlendMode = GLES_SetTextureBlendMode;
    renderer->SetTextureScaleMode = GLES_SetTextureScaleMode;
    renderer->UpdateTexture = GLES_UpdateTexture;
    renderer->LockTexture = GLES_LockTexture;
    renderer->UnlockTexture = GLES_UnlockTexture;
    renderer->DirtyTexture = GLES_DirtyTexture;
    renderer->RenderDrawPoints = GLES_RenderDrawPoints;
    renderer->RenderDrawLines = GLES_RenderDrawLines;
    renderer->RenderDrawRects = GLES_RenderDrawRects;
    renderer->RenderFillRects = GLES_RenderFillRects;
    renderer->RenderCopy = GLES_RenderCopy;
    renderer->RenderPresent = GLES_RenderPresent;
    renderer->DestroyTexture = GLES_DestroyTexture;
    renderer->DestroyRenderer = GLES_DestroyRenderer;
    renderer->info = GL_ES_RenderDriver.info;
    renderer->window = window;
    renderer->driverdata = data;

    renderer->info.flags =
        (SDL_RENDERER_PRESENTDISCARD | SDL_RENDERER_ACCELERATED);

#if defined(__QNXNTO__)
#if _NTO_VERSION<=641
    /* QNX's OpenGL ES implementation is broken regarding             */
    /* packed textures support, affected versions 6.3.2, 6.4.0, 6.4.1 */
    renderer->info.num_texture_formats = 2;
    renderer->info.texture_formats[0] = SDL_PIXELFORMAT_ABGR8888;
    renderer->info.texture_formats[1] = SDL_PIXELFORMAT_BGR24;
#endif /* _NTO_VERSION */
#endif /* __QNXNTO__ */

    if (GLES_LoadFunctions(data) < 0) {
        GLES_DestroyRenderer(renderer);
        return NULL;
    }

    data->context = SDL_GL_CreateContext(window);
    if (!data->context) {
        GLES_DestroyRenderer(renderer);
        return NULL;
    }
    if (SDL_GL_MakeCurrent(window, data->context) < 0) {
        GLES_DestroyRenderer(renderer);
        return NULL;
    }

    if (flags & SDL_RENDERER_PRESENTVSYNC) {
        SDL_GL_SetSwapInterval(1);
    } else {
        SDL_GL_SetSwapInterval(0);
    }
    if (SDL_GL_GetSwapInterval() > 0) {
        renderer->info.flags |= SDL_RENDERER_PRESENTVSYNC;
    }

#ifdef ANDROID
    // Always double-buffered
#else
    if (SDL_GL_GetAttribute(SDL_GL_DOUBLEBUFFER, &doublebuffer) == 0) {
        if (!doublebuffer) {
            renderer->info.flags |= SDL_RENDERER_SINGLEBUFFER;
        }
    }
#endif
#if SDL_VIDEO_DRIVER_PANDORA
    data->GL_OES_draw_texture_supported = SDL_FALSE;
    data->useDrawTexture = SDL_FALSE;
#else
    if (SDL_GL_ExtensionSupported("GL_OES_draw_texture")) {
        data->GL_OES_draw_texture_supported = SDL_TRUE;
        data->useDrawTexture = SDL_TRUE;
    } else {
        data->GL_OES_draw_texture_supported = SDL_FALSE;
        data->useDrawTexture = SDL_FALSE;
    }
#ifdef ANDROID
    data->GL_OES_draw_texture_supported = SDL_TRUE;
    data->useDrawTexture = SDL_TRUE;
#endif
#endif

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    renderer->info.max_texture_width = value;
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &value);
    renderer->info.max_texture_height = value;

    /* Set up parameters for rendering */
    data->blendMode = -1;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    data->updateSize = SDL_TRUE;

    return renderer;
}


float GLES_vbox_left = 0;
float GLES_vbox_right = 0;
float GLES_vbox_top = 0;
float GLES_vbox_bottom = 0;
float GLES_pwidth = 0.0;
float GLES_pheight = 0.0 ;


static int
GLES_ActivateRenderer(SDL_Renderer * renderer)
{

    GLES_RenderData *data = (GLES_RenderData *) renderer->driverdata;
    SDL_Window *window = renderer->window;

    if (SDL_GL_MakeCurrent(window, data->context) < 0) {
        return -1;
    }

    float pwidth = window->display->desktop_mode.w;
    float pheight = window->display->desktop_mode.h;
    float vwidth = window->w;
    float vheight = window->h;

    float physical_ar = pwidth / pheight;
    float virtual_ar = vwidth / vheight;

    float x_padding, y_padding;    

    if (physical_ar > virtual_ar) {
        x_padding = physical_ar * vheight - vwidth;
        y_padding = 0;
    } else {
        x_padding = 0;
        y_padding = ( 1.0 / physical_ar ) * vwidth - vheight;
    }

    GLES_vbox_left = -x_padding / 2.0;
    GLES_vbox_top = -y_padding / 2.0;
    GLES_vbox_right = vwidth + x_padding / 2.0;
    GLES_vbox_bottom = vheight + y_padding / 2.0;
    GLES_pwidth = pwidth;
    GLES_pheight = pheight;


    
    if (data->updateSize) {

    	GLfloat ortho_matrix[] = {
    			2 / (GLES_vbox_right - GLES_vbox_left), 0, 0, -(GLES_vbox_right + GLES_vbox_left)/(GLES_vbox_right - GLES_vbox_left),
    			0, 2 / (GLES_vbox_top - GLES_vbox_bottom), 0, -(GLES_vbox_top + GLES_vbox_bottom)/(GLES_vbox_top - GLES_vbox_bottom),
    			0, 0, -2 / 1, -1,
    			0, 0, 0, 1
    	};

    	GLchar *vertex_code =
    			"precision highp float;"
    			"uniform mat4 xform;"
    			"attribute vec2 vertex;"
    			"attribute vec2 texcoord;"
    			"varying vec2 f_texcoord;"
    			"void main(void) {"
    			"   gl_Position = vec4(vertex, 0, 1.0) * xform;"
    			"   f_texcoord = texcoord;"
    			"}";

    	GLchar *fragment_code =
    			"precision highp float;"
    			"varying vec2 pos;"
    			"varying vec2 f_texcoord;"
    			"uniform sampler2D tex;"
    			"void main(void) {"
    			"    gl_FragColor = texture2D(tex, f_texcoord);"
    			"}";

    	GLuint vertex_shader, fragment_shader, program;

    	vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    	glShaderSource(vertex_shader, 1, &vertex_code, NULL);
    	glCompileShader(vertex_shader);

    	fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    	glShaderSource(fragment_shader, 1, &fragment_code, NULL);
    	glCompileShader(fragment_shader);

    	program = glCreateProgram();

    	glBindAttribLocation(program, ATTRIB_VERTEX, "vertex");
    	glBindAttribLocation(program, ATTRIB_TEXCOORD, "texcoord");

    	glAttachShader(program, vertex_shader);
    	glAttachShader(program, fragment_shader);
    	glLinkProgram(program);

    	glUseProgram(program);

    	glUniformMatrix4fv(
    			glGetUniformLocation(program, "xform"),
    			1,
    			GL_FALSE,
    			ortho_matrix);

    	glUniform1i(
    			glGetUniformLocation(program, "tex"),
    			0);

    	glViewport(0, 0, pwidth, pheight);

    	__android_log_print(ANDROID_LOG_INFO, "libSDL", "pwidth %d pheight %d", pwidth, pheight);

        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);
                 
/* #if SDL_VIDEO_RENDER_RESIZE */
        /*         __android_log_print(ANDROID_LOG_INFO, "libSDL", "GLES_ActivateRenderer(): %dx%d", (int)window->display->desktop_mode.w, (int)window->display->desktop_mode.h); */
/*         glViewport(0, 0, window->display->desktop_mode.w, window->display->desktop_mode.h); */
/*         glOrthof(0.0, (GLfloat) window->display->desktop_mode.w, (GLfloat) window->display->desktop_mode.h, */
/*                        0.0, 0.0, 1.0); */
/* #else */
/*         glViewport(0, 0, window->w, window->h); */
/*         glOrthof(0.0, (GLfloat) window->w, (GLfloat) window->h,  */
/*                        0.0, 0.0, 1.0); */
/* #endif */

        data->updateSize = SDL_FALSE;
    }

    return 0;
}

static int
GLES_DisplayModeChanged(SDL_Renderer * renderer)
{
    GLES_RenderData *data = (GLES_RenderData *) renderer->driverdata;

    data->updateSize = SDL_TRUE;
    return 0;
}

static __inline__ int
power_of_2(int input)
{
    int value = 1;

    while (value < input) {
        value <<= 1;
    }
    return value;
}

static int
GLES_CreateTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    GLES_RenderData *renderdata = (GLES_RenderData *) renderer->driverdata;
    GLES_TextureData *data;
    GLint internalFormat;
    GLenum format, type;
    int texture_w, texture_h;
    GLenum result;

    switch (texture->format) {
    case SDL_PIXELFORMAT_BGR24:
        internalFormat = GL_RGB;
        format = GL_RGB;
        type = GL_UNSIGNED_BYTE;
        break;
    case SDL_PIXELFORMAT_ABGR8888:
        internalFormat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_BYTE;
        break;
    case SDL_PIXELFORMAT_BGR565:
        internalFormat = GL_RGB;
        format = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
        break;
    case SDL_PIXELFORMAT_ABGR1555:
        internalFormat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT_5_5_5_1;
        break;
    case SDL_PIXELFORMAT_ABGR4444:
        internalFormat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT_4_4_4_4;
        break;
#ifdef ANDROID
    case SDL_PIXELFORMAT_RGB565:
        internalFormat = GL_RGB;
        format = GL_RGB;
        type = GL_UNSIGNED_SHORT_5_6_5;
        break;
    case SDL_PIXELFORMAT_RGBA5551:
        internalFormat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT_5_5_5_1;
        break;
    case SDL_PIXELFORMAT_RGBA4444:
        internalFormat = GL_RGBA;
        format = GL_RGBA;
        type = GL_UNSIGNED_SHORT_4_4_4_4;
        break;
#endif
    default:
        SDL_SetError("Unsupported by OpenGL ES texture format");
        return -1;
    }

    data = (GLES_TextureData *) SDL_calloc(1, sizeof(*data));
    if (!data) {
        SDL_OutOfMemory();
        return -1;
    }

    if (texture->access == SDL_TEXTUREACCESS_STREAMING) {
        data->pitch = texture->w * SDL_BYTESPERPIXEL(texture->format);
        data->pixels = SDL_malloc(texture->h * data->pitch);
        if (!data->pixels) {
            SDL_OutOfMemory();
            SDL_free(data);
            return -1;
        }
    }

    texture->driverdata = data;

    glGetError();
    glGenTextures(1, &data->texture);

    data->type = GL_TEXTURE_2D;
    /* no NPOV textures allowed in OpenGL ES (yet) */
    texture_w = power_of_2(texture->w);
    texture_h = power_of_2(texture->h);
    data->texw = (GLfloat) texture->w / texture_w;
    data->texh = (GLfloat) texture->h / texture_h;

    data->format = format;
    data->formattype = type;
    glBindTexture(data->type, data->texture);
    glTexParameteri(data->type, GL_TEXTURE_MIN_FILTER,
                                GL_NEAREST);
    glTexParameteri(data->type, GL_TEXTURE_MAG_FILTER,
                                GL_NEAREST);
    glTexParameteri(data->type, GL_TEXTURE_WRAP_S,
                                GL_CLAMP_TO_EDGE);
    glTexParameteri(data->type, GL_TEXTURE_WRAP_T,
                                GL_CLAMP_TO_EDGE);

    glTexImage2D(data->type, 0, internalFormat, texture_w,
                             texture_h, 0, format, type, NULL);

    result = glGetError();
    if (result != GL_NO_ERROR) {
        GLES_SetError("glTexImage2D()", result);
        return -1;
    }
    return 0;
}

static int
GLES_QueryTexturePixels(SDL_Renderer * renderer, SDL_Texture * texture,
                        void **pixels, int *pitch)
{
    GLES_TextureData *data = (GLES_TextureData *) texture->driverdata;

    *pixels = data->pixels;
    *pitch = data->pitch;
    return 0;
}

static int
GLES_SetTexturePalette(SDL_Renderer * renderer, SDL_Texture * texture,
                       const SDL_Color * colors, int firstcolor, int ncolors)
{
    SDL_SetError("OpenGL ES does not support paletted textures");
    return -1;
}

static int
GLES_GetTexturePalette(SDL_Renderer * renderer, SDL_Texture * texture,
                       SDL_Color * colors, int firstcolor, int ncolors)
{
    SDL_SetError("OpenGL ES does not support paletted textures");
    return -1;
}

static void
SetupTextureUpdate(GLES_RenderData * renderdata, SDL_Texture * texture,
                   int pitch)
{
    GLES_TextureData *data = (GLES_TextureData *) texture->driverdata;
    glBindTexture(data->type, data->texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}

static int
GLES_SetTextureColorMod(SDL_Renderer * renderer, SDL_Texture * texture)
{
    return 0;
}

static int
GLES_SetTextureAlphaMod(SDL_Renderer * renderer, SDL_Texture * texture)
{
    return 0;
}

static int
GLES_SetTextureBlendMode(SDL_Renderer * renderer, SDL_Texture * texture)
{
    switch (texture->blendMode) {
    case SDL_BLENDMODE_NONE:
    case SDL_BLENDMODE_MASK:
    case SDL_BLENDMODE_BLEND:
    case SDL_BLENDMODE_ADD:
    case SDL_BLENDMODE_MOD:
        return 0;
    default:
        SDL_Unsupported();
        texture->blendMode = SDL_BLENDMODE_NONE;
        return -1;
    }
}

static int
GLES_SetTextureScaleMode(SDL_Renderer * renderer, SDL_Texture * texture)
{
    switch (texture->scaleMode) {
    case SDL_TEXTURESCALEMODE_NONE:
    case SDL_TEXTURESCALEMODE_FAST:
    case SDL_TEXTURESCALEMODE_SLOW:
        return 0;
    case SDL_TEXTURESCALEMODE_BEST:
        SDL_Unsupported();
        texture->scaleMode = SDL_TEXTURESCALEMODE_SLOW;
        return -1;
    default:
        SDL_Unsupported();
        texture->scaleMode = SDL_TEXTURESCALEMODE_NONE;
        return -1;
    }
}

static int
GLES_UpdateTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                   const SDL_Rect * rect, const void *pixels, int pitch)
{
    GLES_RenderData *renderdata = (GLES_RenderData *) renderer->driverdata;
    GLES_TextureData *data = (GLES_TextureData *) texture->driverdata;
    GLenum result;
    int bpp = SDL_BYTESPERPIXEL(texture->format);
    void * temp_buffer;
    void * temp_ptr;
    int i;

    glGetError();
    SetupTextureUpdate(renderdata, texture, pitch);

    if( rect->w * bpp == pitch ) {
         temp_buffer = (void *)pixels; /* No need to reformat */
    } else {
         /* Reformatting of mem area required */
         temp_buffer = SDL_malloc(rect->w * rect->h * bpp);
         temp_ptr = temp_buffer;
         for (i = 0; i < rect->h; i++) {
             SDL_memcpy(temp_ptr, pixels, rect->w * bpp);
             temp_ptr += rect->w * bpp;
             pixels += pitch;
         }
    }

    glTexSubImage2D(data->type, 0, rect->x, rect->y, rect->w,
                                rect->h, data->format, data->formattype,
                                temp_buffer);

    if( temp_buffer != pixels ) {
        SDL_free(temp_buffer);
    }

    result = glGetError();
    if (result != GL_NO_ERROR) {
        GLES_SetError("glTexSubImage2D()", result);
        return -1;
    }
    return 0;
}

static int
GLES_LockTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                 const SDL_Rect * rect, int markDirty, void **pixels,
                 int *pitch)
{
    GLES_TextureData *data = (GLES_TextureData *) texture->driverdata;

    if (markDirty) {
        SDL_AddDirtyRect(&data->dirty, rect);
    }

    *pixels =
        (void *) ((Uint8 *) data->pixels + rect->y * data->pitch +
                  rect->x * SDL_BYTESPERPIXEL(texture->format));
    *pitch = data->pitch;
    return 0;
}

static void
GLES_UnlockTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
}

static void
GLES_DirtyTexture(SDL_Renderer * renderer, SDL_Texture * texture,
                  int numrects, const SDL_Rect * rects)
{
    GLES_TextureData *data = (GLES_TextureData *) texture->driverdata;
    int i;

    for (i = 0; i < numrects; ++i) {
        SDL_AddDirtyRect(&data->dirty, &rects[i]);
    }
}

static void
GLES_SetBlendMode(GLES_RenderData * data, int blendMode, int isprimitive)
{
}

static int
GLES_RenderDrawPoints(SDL_Renderer * renderer, const SDL_Point * points,
                      int count)
{
	return 0;
}

static int
GLES_RenderDrawLines(SDL_Renderer * renderer, const SDL_Point * points,
                     int count)
{

    return 0;
}

static int
GLES_RenderDrawRects(SDL_Renderer * renderer, const SDL_Rect ** rects,
                     int count)
{
    return 0;
}

static int
GLES_RenderFillRects(SDL_Renderer * renderer, const SDL_Rect ** rects,
                     int count)
{
	return 0;
}

static int
GLES_RenderCopy(SDL_Renderer * renderer, SDL_Texture * texture,
                const SDL_Rect * srcrect, const SDL_Rect * dstrect)
{

    GLES_RenderData *data = (GLES_RenderData *) renderer->driverdata;
    GLES_TextureData *texturedata = (GLES_TextureData *) texture->driverdata;
    int minx, miny, maxx, maxy;
    GLfloat minu, maxu, minv, maxv;
    int i;
    void *temp_buffer;          /* used for reformatting dirty rect pixels */
    void *temp_ptr;

    if (texturedata->dirty.list) {
        SDL_DirtyRect *dirty;
        void *pixels;
        int bpp = SDL_BYTESPERPIXEL(texture->format);
        int pitch = texturedata->pitch;

        SetupTextureUpdate(data, texture, pitch);

        glBindTexture(texturedata->type, texturedata->texture);
        for (dirty = texturedata->dirty.list; dirty; dirty = dirty->next) {
            SDL_Rect *rect = &dirty->rect;
            pixels =
                (void *) ((Uint8 *) texturedata->pixels + rect->y * pitch +
                          rect->x * bpp);
            /*      There is no GL_UNPACK_ROW_LENGTH in OpenGLES 
               we must do this reformatting ourselves(!)

               maybe it'd be a good idea to keep a temp buffer around
               for this purpose rather than allocating it each time
             */
            if( rect->x == 0 && rect->w * bpp == pitch ) {
                temp_buffer = pixels; /* Updating whole texture, no need to reformat */
            } else {
                temp_buffer = SDL_malloc(rect->w * rect->h * bpp);
                temp_ptr = temp_buffer;
                for (i = 0; i < rect->h; i++) {
                    SDL_memcpy(temp_ptr, pixels, rect->w * bpp);
                    temp_ptr += rect->w * bpp;
                    pixels += pitch;
                }
            }

            glTexSubImage2D(texturedata->type, 0, rect->x, rect->y,
                                  rect->w, rect->h, texturedata->format,
                                  texturedata->formattype, temp_buffer);

            if( temp_buffer != pixels ) {
                SDL_free(temp_buffer);
            }
        }
        SDL_ClearDirtyRects(&texturedata->dirty);
    }

    glBindTexture(texturedata->type, texturedata->texture);

//    if (texture->modMode) {
//        glColor4f((GLfloat) texture->r * inv255f,
//                        (GLfloat) texture->g * inv255f,
//                        (GLfloat) texture->b * inv255f,
//                        (GLfloat) texture->a * inv255f);
//    } else {
//        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
//    }
//
//    GLES_SetBlendMode(data, texture->blendMode, 0);

    switch (texture->scaleMode) {
    case SDL_TEXTURESCALEMODE_NONE:
    case SDL_TEXTURESCALEMODE_FAST:
        /* glTexParameteri(texturedata->type, GL_TEXTURE_MIN_FILTER, */
        /*                       GL_NEAREST); */
        /* glTexParameteri(texturedata->type, GL_TEXTURE_MAG_FILTER, */
        /*                       GL_NEAREST); */
        /* break; */
    case SDL_TEXTURESCALEMODE_SLOW:
    case SDL_TEXTURESCALEMODE_BEST:
        glTexParameteri(texturedata->type, GL_TEXTURE_MIN_FILTER,
                              GL_LINEAR);
        glTexParameteri(texturedata->type, GL_TEXTURE_MAG_FILTER,
                              GL_LINEAR);
        break;
    }


	minx = dstrect->x;
	miny = dstrect->y;
	maxx = dstrect->x + dstrect->w;
	maxy = dstrect->y + dstrect->h;

	minu = (GLfloat) srcrect->x / texture->w;
	minu *= texturedata->texw;
	maxu = (GLfloat) (srcrect->x + srcrect->w) / texture->w;
	maxu *= texturedata->texw;
	minv = (GLfloat) srcrect->y / texture->h;
	minv *= texturedata->texh;
	maxv = (GLfloat) (srcrect->y + srcrect->h) / texture->h;
	maxv *= texturedata->texh;

	GLshort vertices[8];
	GLfloat texCoords[8];

	vertices[0] = minx;
	vertices[1] = miny;
	vertices[2] = maxx;
	vertices[3] = miny;
	vertices[4] = minx;
	vertices[5] = maxy;
	vertices[6] = maxx;
	vertices[7] = maxy;

	texCoords[0] = minu;
	texCoords[1] = minv;
	texCoords[2] = maxu;
	texCoords[3] = minv;
	texCoords[4] = minu;
	texCoords[5] = maxv;
	texCoords[6] = maxu;
	texCoords[7] = maxv;

	glEnableVertexAttribArray(ATTRIB_VERTEX);
	glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_SHORT, GL_FALSE, 0, vertices);

	glEnableVertexAttribArray(ATTRIB_TEXCOORD);
	glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

//        glVertexPointer(2, GL_SHORT, 0, vertices);
//        glEnableClientState(GL_VERTEX_ARRAY);
//        glTexCoordPointer(2, GL_FLOAT, 0, texCoords);
//        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
//        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
//        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
//        glDisableClientState(GL_VERTEX_ARRAY);

    return 0;
}

static void
GLES_RenderPresent(SDL_Renderer * renderer)
{
    SDL_GL_SwapWindow(renderer->window);
}

static void
GLES_DestroyTexture(SDL_Renderer * renderer, SDL_Texture * texture)
{
    GLES_TextureData *data = (GLES_TextureData *) texture->driverdata;

    if (!data) {
        return;
    }
    if (data->texture) {
        glDeleteTextures(1, &data->texture);
    }
    if (data->pixels) {
        SDL_free(data->pixels);
    }
    SDL_FreeDirtyRects(&data->dirty);
    SDL_free(data);
    texture->driverdata = NULL;
}

static void
GLES_DestroyRenderer(SDL_Renderer * renderer)
{
    GLES_RenderData *data = (GLES_RenderData *) renderer->driverdata;

    if (data) {
        if (data->context) {
            SDL_GL_DeleteContext(data->context);
        }
        SDL_free(data);
    }
    SDL_free(renderer);
}

#endif /* SDL_VIDEO_RENDER_OGL_ES */

/* vi: set ts=4 sw=4 expandtab: */
