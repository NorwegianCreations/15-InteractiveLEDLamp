#ifndef ONESHOT1_H
#define ONESHOT1_H

#include "lib/effect.h"
#include "lib/effect_runner.h"
#include "lib/color.h"

#include <stdio.h>
#include <stdlib.h>

#define LFO_RATE 0.1

class OneshotEffect : public Effect
{
public:
    OneshotEffect()
    {
        h = 0;
        s = 0;
        v = 0;
        dir = 1;
    }

    virtual void beginFrame(const FrameInfo &f)
    {
        if(dir)
        {
            v += LFO_RATE;
        }
        else
        {
            v -= LFO_RATE;
        }

        if(v > 0.9)
        {
            dir = 0;
        }
        else if(v < 0.1)
        {
            dir = 1;
        }

    }

    virtual void shader(Vec3& rgb, const PixelInfo &p) const
    {
        hsv2rgb(rgb, h, s, v);
    }

private:
    float h, s, v;
    bool dir;
};

#endif // ONESHOT1_H
