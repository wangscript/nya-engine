//https://code.google.com/p/nya-engine/

#pragma once

#include "render_objects.h"

#ifdef _WIN32
    #include <windows.h>
		#if _WIN32_WINNT>=_WIN32_WINNT_WIN8 && defined _M_ARM
            #define WINDOWS_PHONE8
		#endif

        //#define NYA_DIRECTX11

        #if defined WINDOWS_PHONE8 || defined NYA_DIRECTX11
            #define DIRECTX11
        #endif

		#ifdef DIRECTX11
			#include <d3d11.h>
            #include <vector>
			#define NO_EXTENSIONS_INIT
		#else
			#include <gl/gl.h>
			#include <gl/glext.h>
		#endif
#elif defined __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #import <OpenGLES/ES2/gl.h>
        #import <OpenGLES/ES2/glext.h>

        #define OPENGL_ES
    #else
        #include <OpenGL/gl.h>
        #include <OpenGL/glext.h>
    #endif

    #define NO_EXTENSIONS_INIT
#elif ANDROID
    #include <GLES2/gl2.h>
    #include <GLES2/gl2ext.h>
    #define OPENGL_ES
#else
    #include <GL/glx.h>
    #include <GL/gl.h>
    #include <GL/glext.h>
#endif

#ifdef OPENGL_ES
    #define ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    #define NO_EXTENSIONS_INIT
#endif

#ifdef DIRECTX11
    #define DIRECTX11_ONLY(...) __VA_ARGS__
    #define OPENGL_ONLY(...)
#else
    #define DIRECTX11_ONLY(...)
    #define OPENGL_ONLY(...) __VA_ARGS__
#endif

namespace nya_render
{
#ifdef ATTRIBUTES_INSTEAD_OF_CLIENTSTATES
    enum
    {
        vertex_attribute,
        normal_attribute,
        color_attribute,
        tc0_attribute,
        max_attributes = tc0_attribute+16
    };
#endif

#ifndef NO_EXTENSIONS_INIT
    bool has_extension(const char *name);
    void *get_extension(const char*ext_name);
#endif

    struct texture_obj
    {
        unsigned int size;

#ifdef DIRECTX11
        ID3D11Texture2D *tex;
        ID3D11ShaderResourceView* srv;
        ID3D11SamplerState* sampler_state;
        ID3D11RenderTargetView *color_targets[6];
        ID3D11DepthStencilView *depth_target;
        DXGI_FORMAT dx_format;
        bool has_mipmaps;

        texture_obj(): tex(0),srv(0),sampler_state(0),size(0),depth_target(0),has_mipmaps(false)
        {
            for(int i=0;i<6;++i)
                color_targets[i]=0;
        }
#else
        unsigned int tex_id;
        unsigned int gl_type;
        bool has_mipmaps;

        texture_obj(): tex_id(0),gl_type(0),size(0),has_mipmaps(false) {}
#endif
    public:
        static int add() { return get_texture_objs().add(); }
        static texture_obj &get(int idx) { return get_texture_objs().get(idx); }
        static void remove(int idx) { return get_texture_objs().remove(idx); }
        static unsigned int get_used_vmem_size();
        static void invalidate_all() { get_texture_objs().invalidate_all(); }
        static void release_all() { get_texture_objs().release_all(); }

    public:
        void release();

    private:
        typedef render_objects<texture_obj> texture_objs;
        static texture_objs &get_texture_objs()
        {
            static texture_objs objs;
            return objs;
        }
    };

#ifdef DIRECTX11
    ID3D11InputLayout *get_layout(int mesh_idx);
    ID3D11InputLayout *add_layout(int mesh_idx,
                                  const D3D11_INPUT_ELEMENT_DESC*desc,size_t desc_size);
    void remove_layout(int mesh_idx);

    struct dx_target
    {
        ID3D11RenderTargetView *color;
        ID3D11DepthStencilView *depth;

        dx_target(): color(0),depth(0) {}
    };

    dx_target get_default_target();
    dx_target get_target();

    void set_target(ID3D11RenderTargetView *color,ID3D11DepthStencilView *depth,bool default=false);
#endif
    void reset_vbo_state();

    void release_vbos();
    void release_shaders();
    void release_fbos();
    void release_states();

    inline void release_resources()
    {
        texture_obj::release_all();
        release_shaders();
        release_vbos();
        release_fbos();
        release_states();
    }

    void invalidate_shaders();
    void invalidate_vbos();
    void invalidate_fbos();

    inline void invalidate_resources() //on context loss, etc
    {
        texture_obj::invalidate_all();
        invalidate_shaders();
        invalidate_vbos();
        invalidate_fbos();
    }
}
