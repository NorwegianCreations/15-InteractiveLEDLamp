#ifndef NOISE_H
#define NOISE_H

#include "lib/effect.h"
#include "lib/effect_runner.h"
#include "lib/noise.h"
#include "lib/color.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#define RAD_PER_SECOND M_PI/4
#define MAX_Z_HEIGHT 140.71
#define H_RES 100000
#define S_RES 100000
#define H_RATE 0.00001
#define S_RATE 0.00001

class NoiseEffect : public Effect
{
public:
    NoiseEffect(float h_arg, float s_arg);

    float timer;
    float h, s, h_tgt, s_tgt;
    float outer, inner;
    int h_dir;

    virtual void beginFrame(const FrameInfo &f)
    {
        timer += f.timeDelta;

        if(timer > 1000000)
        {
            timer = 0;
        }

        outer = std::min(h,h_tgt) + 1-std::max(h,h_tgt); 	//distance from current hue to target hue via 0 and 255
        inner = abs(h-h_tgt);								//distance from current hue to target hue within the scale


        if((roundf(h*H_RES)/H_RES) == (roundf(h_tgt*H_RES)/H_RES)) //when target hue is reached, select new hue target at random
        {
            srand (time(NULL));
            h_tgt = (float)rand()/(RAND_MAX); 

            if((inner <= outer && h_tgt-h >= 0) || (inner > outer && h_tgt - h < 0)) //select direction (shortest way)
                h_dir = 1;
            else
                h_dir = 0;

        }
        else if(h_dir == 1)	//continue moving if hue not at target
        {
            h += H_RATE;
            if(h==1)
                h = 0;
        }
        else
        {
            h -= H_RATE;
            if(h==0)
                h = 1;
        }


        if((roundf(s*S_RES)/S_RES) == (roundf(s_tgt*S_RES)/S_RES))	//when target saturation is reached, select new saturation target at random
        {
            srand (time(NULL));
            s_tgt = (float)rand()/(RAND_MAX);
        }
        else if(s_tgt-s > 0)
        {
            s+= S_RATE;
        }
        else
        {
            s -= S_RATE;
        }

    }

    virtual void shader(Vec3& rgb, const PixelInfo &p) const
    {
        float z_factor = p.point[2]/MAX_Z_HEIGHT;	//z factor is compensation for hexagonal columns height (defined in the .json)

        float v = 0.25 * (noise2(0.004*p.point[0],0.004*p.point[1] + 0.1*timer)) + 0.35 + 0.4*z_factor;	//set value with slowly panning noise, compensating for tall hexagonal columns

        float h_tmp = h + 0.1*(noise2(0.001*p.point[0], 0.001*p.point[1] + 0.1*timer));	//set hue with slowly panning noise

        hsv2rgb(rgb, h_tmp, s, v);

        if(rgb[0] > 1)
            std::cout << rgb[0] << std::endl;
    }
};

NoiseEffect::NoiseEffect(float h_arg, float s_arg)
{
    h = h_arg;
    s = s_arg;
    timer = 0;
    h_tgt = 0;
    s_tgt = 0;
    h_dir = 0;

}

#endif // NOISE_H
