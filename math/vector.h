//https://code.google.com/p/nya-engine/

#pragma once

#include <math.h>

namespace nya_math
{

struct vec3
{
    float x;
    float y;
    float z;

    vec3(): x(0),y(0),z(0) {}
    vec3(float x,float y,float z) { this->x=x; this->y=y; this->z=z; }
    vec3(const float *v) { x=v[0]; y=v[1]; z=v[2]; }

    vec3 operator + (const vec3 &v) const { return vec3(x+v.x,y+v.y,z+v.z);	}
    vec3 operator - (const vec3 &v) const { return vec3(x-v.x,y-v.y,z-v.z);	}
    float operator * (const vec3 &v) const { return x*v.x+y*v.y+z*v.z; }

    vec3 operator - () const { return vec3(-x,-y,-z); }

    vec3 operator *= (const float a) { x*=a; y*=a; z*=a; return *this; }
    vec3 operator /= (const float a) { x/=a; y/=a; z/=a; return *this; }

    vec3 operator += (const vec3 &v) { x+=v.x; y+=v.y; z+=v.z; return *this; }
    vec3 operator -= (const vec3 &v) { x-=v.x; y-=v.y; z-=v.z; return *this; }

    float length() const { return sqrtf(x*x+y*y+z*z); }

    vec3 &abs() { x=fabsf(x); y=fabsf(y); z=fabsf(z); return *this; }

    vec3 &normalize()
    {
        float len=length();
        if(len<0.00001f)
            return *this;

        *this *=(1.0f/len);
        return *this;
    }

    static vec3 cross(vec3 a,vec3 b)
    {
        return vec3(a.y*b.z - a.z*b.y,
                    a.z*b.x - a.x*b.z,
                    a.x*b.y - a.y*b.x);
    }
};

inline vec3 operator * ( float a, const vec3& v ) { return vec3(v.x*a,v.y*a,v.z*a); }
inline vec3 operator * ( const vec3& v, float a ) { return vec3(v.x*a,v.y*a,v.z*a); }
inline vec3 operator / ( const vec3& v, float a ) { return vec3(v.x/a,v.y/a,v.z/a); }

}
