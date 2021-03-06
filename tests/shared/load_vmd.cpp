//https://code.google.com/p/nya-engine/

#include "load_vmd.h"
#include "scene/animation.h"
#include "memory/memory_reader.h"
#include "string_encoding.h"

bool vmd_loader::load(nya_scene::shared_animation &res,nya_scene::resource_data &data,const char* name)
{
    nya_memory::memory_reader reader(data.get_data(),data.get_size());

    if(!reader.test("Vocaloid Motion Data 0002",25))
        return false;

    reader.skip(5);
    reader.skip(20);//name

    typedef unsigned int uint;

    const uint frames_count=reader.read<uint>();
    if(!reader.check_remained(frames_count*(15+sizeof(uint)+sizeof(float)*7+16*4)))
        return false;

    for(uint i=0;i<frames_count;++i)
    {
        struct
        {
            std::string name;
            uint frame;

            nya_math::vec3 pos;
            nya_math::quat rot;

            char bezier_x[16];
            char bezier_y[16];
            char bezier_z[16];
            char bezier_rot[16];

        } bone_frame;

        bone_frame.name=utf8_from_shiftjis(reader.get_data(),15);
        reader.skip(15);
        bone_frame.frame=reader.read<uint>();
        bone_frame.pos.x=reader.read<float>();
        bone_frame.pos.y=reader.read<float>();
        bone_frame.pos.z=-reader.read<float>();

        bone_frame.rot.v.x=-reader.read<float>();
        bone_frame.rot.v.y=-reader.read<float>();
        bone_frame.rot.v.z=reader.read<float>();
        bone_frame.rot.w=reader.read<float>();

        memcpy(bone_frame.bezier_x,reader.get_data(),sizeof(bone_frame.bezier_x));
        reader.skip(sizeof(bone_frame.bezier_x));
        memcpy(bone_frame.bezier_y,reader.get_data(),sizeof(bone_frame.bezier_y));
        reader.skip(sizeof(bone_frame.bezier_y));
        memcpy(bone_frame.bezier_z,reader.get_data(),sizeof(bone_frame.bezier_z));
        reader.skip(sizeof(bone_frame.bezier_z));
        memcpy(bone_frame.bezier_rot,reader.get_data(),sizeof(bone_frame.bezier_rot));
        reader.skip(sizeof(bone_frame.bezier_rot));

        const float c2f=1.0f/128.0f;

        const int bone_idx=res.anim.add_bone(bone_frame.name.c_str());
        if(bone_idx<0)
            continue;

        const uint time=bone_frame.frame*33.6;

        nya_render::animation::pos_interpolation pos_inter;

        pos_inter.x=nya_math::bezier(bone_frame.bezier_x[0]*c2f,bone_frame.bezier_x[4]*c2f,
                                     bone_frame.bezier_x[8]*c2f,bone_frame.bezier_x[12]*c2f);

        pos_inter.y=nya_math::bezier(bone_frame.bezier_y[0]*c2f,bone_frame.bezier_y[4]*c2f,
                                     bone_frame.bezier_y[8]*c2f,bone_frame.bezier_y[12]*c2f);

        pos_inter.z=nya_math::bezier(bone_frame.bezier_z[0]*c2f,bone_frame.bezier_z[4]*c2f,
                                     bone_frame.bezier_z[8]*c2f,bone_frame.bezier_z[12]*c2f);

        res.anim.add_bone_pos_frame(bone_idx,time,bone_frame.pos,pos_inter);

        const nya_math::bezier rot_inter=nya_math::bezier(bone_frame.bezier_rot[0]*c2f,bone_frame.bezier_rot[4]*c2f,
                                                          bone_frame.bezier_rot[8]*c2f,bone_frame.bezier_rot[12]*c2f);

        res.anim.add_bone_rot_frame(bone_idx,time,bone_frame.rot,rot_inter);
    }

    const uint facial_frames_count=reader.read<uint>();
    if(!reader.check_remained(facial_frames_count*(15+sizeof(uint)+sizeof(float))))
        return false;

    for(uint i=0;i<facial_frames_count;++i)
    {
        const std::string name=utf8_from_shiftjis(reader.get_data(),15);
        reader.skip(15);
        const uint frame=reader.read<uint>();
        const float value=reader.read<float>();

        const int curve_idx=res.anim.add_curve(name.c_str());
        if(curve_idx<0)
            continue;

        res.anim.add_curve_frame(curve_idx,frame*33.6,value);
    }

    return true;
}
