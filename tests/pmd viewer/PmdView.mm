//https://code.google.com/p/nya-engine/

#import "PmdView.h"
#import "PmdDocument.h"

#include "scene/camera.h"
#include "memory/memory_reader.h"

void viewer_camera::add_rot(float dx,float dy)
{
    m_rot_x+=dx;
    m_rot_y+=dy;
    
    const float max_angle=360.0f;
    
    if ( m_rot_x > max_angle )
        m_rot_x -= max_angle;
    
    if ( m_rot_x < -max_angle )
        m_rot_x += max_angle;
    
    if ( m_rot_y > max_angle )
        m_rot_y -= max_angle;
    
    if ( m_rot_y < -max_angle )
        m_rot_y += max_angle;
    
    update();
}

void viewer_camera::add_pos(float dx,float dy,float dz)
{
    m_pos.x-=dx;
    m_pos.y-=dy;
    m_pos.z-=dz;
    if(m_pos.z < 5.0f)
        m_pos.z = 5.0f;

    if(m_pos.z > 100.0f)
        m_pos.z = 100.0f;

    update();
}

void viewer_camera::set_aspect(float aspect)
{
    nya_scene::get_camera().set_proj(25.0,aspect,1.0,150.0);
    update();
}

void viewer_camera::update()
{
    nya_scene::get_camera().set_rot(m_rot_x,m_rot_y,0.0);
    
    nya_math::quat rot(nya_math::vec3(-m_rot_y*3.14f/180.0f,-m_rot_x*3.14f/180.0f,0.0f));
    nya_math::vec3 pos=rot.rotate(m_pos);
    
    nya_scene::get_camera().set_pos(pos.x,pos.y+10.0f,pos.z);
}

void flip_vertical(unsigned char *data,int width,int height,int bpp)
{
    const int line_size=width*bpp;
    const int top=line_size*(height-1);
    const int half=line_size*height/2;
    
    unsigned char tmp[4];
    
    for(int offset=0;offset<half;offset+=line_size)
    {
        unsigned char *ha=data+offset;
        unsigned char *hb=data+top-offset;
        
        for(int w=0;w<line_size;w+=bpp)
        {
            unsigned char *a=ha+w;
            unsigned char *b=hb+w;
            memcpy(tmp,a,bpp);
            memcpy(a,b,bpp);
            memcpy(b,tmp,bpp);
        }
    }
}

bool load_texture(nya_scene::shared_texture &res,nya_scene::resource_data &texture_data,const char* name)
{
    if(!texture_data.get_size())
        return false;

    NSData * data  = [NSData dataWithBytesNoCopy:texture_data.get_data() 
                                          length: texture_data.get_size() freeWhenDone:NO];
    if (data == nil)
    {
        nya_log::get_log()<<"unable to load texture: NSData error\n";
        return false;
    }
    
    NSBitmapImageRep *image=[NSBitmapImageRep imageRepWithData:data];
    if (image == nil)
    {
        nya_log::get_log()<<"unable to load texture: invalid file\n";
        return false;
    }
    
    unsigned int bpp=(unsigned int)[image bitsPerPixel];
    
    nya_render::texture::color_format format;
    
    if(bpp==24)
        format=nya_render::texture::color_rgb;
    else if(bpp== 32 )
        format=nya_render::texture::color_rgba;
    else
    {
        nya_log::get_log()<<"unable to load texture: unsupported format\n";
        return false;
    }

    unsigned int width =(unsigned int)[image pixelsWide];
    unsigned int height=(unsigned int)[image pixelsHigh];
    unsigned char *image_data=[image bitmapData];

    flip_vertical(image_data,width,height,bpp/8);

    res.tex.build_texture(image_data,width,height,format);

    return true;
}

bool load_pmx(nya_scene::shared_mesh &res,nya_scene::resource_data &data,const char* name)
{
    if(!data.get_size())
        return false;
    
    nya_memory::memory_reader reader(data.get_data(),data.get_size());
    if(!reader.test("PMX ",4))
        return false;
    
    if(reader.read<float>()!=2.0f)
        return false;

    struct pmx_header
    {
        char text_encoding;
        char extended_uv;
        char vertex_index_size;
        char texture_index_size;
        char material_index_size;
        char bone_index_size;
        char morph_index_size;
        char rigidbody_index_size;
    };

    const char header_size=reader.read<char>();
    if(header_size!=sizeof(pmx_header))
        return false;

    const pmx_header header=reader.read<pmx_header>();
    if(header.extended_uv>0)
        return false; //ToDo
    
    for(int i=0;i<4;++i)
    {
        const int size=reader.read<int>();
        reader.skip(size);
    }
    
    const int vert_count=reader.read<int>();
    
    return true;
}

@implementation PmdView

- (void) mouseDown: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    
    m_mouse_old = [self convertPoint: pt fromView: nil];
}

- (void) rightMouseDown: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    
    m_mouse_old = [self convertPoint: pt fromView: nil];
}

- (void) mouseDragged: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];
    
    pt = [self convertPoint: pt fromView: nil];

    m_camera.add_rot(pt.x-m_mouse_old.x,-(pt.y-m_mouse_old.y));

    m_mouse_old = pt;

    [self setNeedsDisplay: YES];
}

- (void) rightMouseDragged: (NSEvent *) theEvent
{
    NSPoint pt = [theEvent locationInWindow];

    pt = [self convertPoint: pt fromView: nil];
    
    m_camera.add_pos((pt.x-m_mouse_old.x)/20.0f,(pt.y-m_mouse_old.y)/20.0f,0.0f);

    m_mouse_old = pt;

    [self setNeedsDisplay: YES];
}

- (void) scrollWheel: (NSEvent*) event
{
    m_camera.add_pos(0.0f,0.0f,[event deltaY]);
    
    [self setNeedsDisplay: YES];
}

-(void)reshape
{
    glViewport( 0,0,[self frame].size.width,[self frame].size.height );     
    
    m_camera.set_aspect([self frame].size.width / [self frame].size.height);

    [self setNeedsDisplay: YES];
}

- (void)draw 
{
    PmdDocument *doc=[[[self window] windowController] document];
    if(!doc->m_model_name.empty())
    {
        glClearColor(0.2,0.4,0.5,0);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        nya_scene::texture::register_load_function(load_texture);
        m_mesh.load(doc->m_model_name.c_str());

        doc->m_model_name.clear();
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_mesh.draw();
}

- (void)drawRect:(NSRect)rect 
{
	[self draw];

    [ [ self openGLContext ] flushBuffer ];    
}

-(void) dealloc
{
    m_mesh.unload();

    [super dealloc];
}

@end
