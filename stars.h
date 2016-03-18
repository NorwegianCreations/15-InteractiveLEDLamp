#ifndef STARS_H
#define STARS_H

#include "lib/effect.h"
#include "lib/effect_runner.h"
#include "lib/color.h"

#include <stdio.h>
#include <stdlib.h>
#include <list>

#define NEW_STAR_FRAME_MIN 10
#define NEW_STAR_FRAME_MAX 200
#define TOTAL_LEDS 512
#define MAX_ACTIVE_STARS 10
#define FADE_RATE 0.005

class Star
{
public:
    Star()
    {
        srand (time(NULL));
        h = (float)rand()/(RAND_MAX);
        srand (time(NULL));
        s = (float)rand()/(RAND_MAX);
        v = 1;
        index = 0;
        active = 0;
    }

    float get_h() const
    {
        return h;
    }

    float get_s() const
    {
        return s;
    }

    float get_v() const
    {
        return v;
    }

    int get_index() const
    {
        return index;
    }

    bool get_active() const
    {
        return active;
    }

    void set_h(float h_arg)
    {
        h = h_arg;
    }

    void set_s(float s_arg)
    {
        s = s_arg;
    }

    void set_v(float v_arg)
    {
        v = v_arg;
    }

    void set_index(float index_arg)
    {
        index = index_arg;
    }

    void set_active(bool active_arg)
    {
        active = active_arg;
    }

private:
    float h,s,v;
    int index;
    bool active;

};

class StarsEffect : public Effect
{
public:
    StarsEffect()
    {
        frame_counter = 0;
        frames_to_next_star = 0;
    }

    virtual void beginFrame(const FrameInfo &f)
    {
        timer += f.timeDelta;

        for(int i=0; i<MAX_ACTIVE_STARS-1; i++)
        {
            float v_tmp = star_array[i].get_v();

            if(v_tmp > 0)                               //fade all LEDs one step
            {
                v_tmp -= FADE_RATE;
                star_array[i].set_v(v_tmp);
            }
            else
            {
                star_array[i].set_active(0);            //if faded out, set star to inactive
            }
        }

        if(frame_counter == frames_to_next_star)        //start the process of new star creation
        {
            star_index_accepted = 0;
            while(!star_index_accepted)                 //keep creating star indexes until it is not an active star
            {
                srand(time(NULL));
                star_index = rand() % TOTAL_LEDS;       //choose a random LED to be a star

                star_index_accepted = 1;
                for(int i=0; i<MAX_ACTIVE_STARS-1; i++)
                {
                    if(star_array[i].get_index() == star_index)
                    {
                        star_index_accepted = 0;        //if the LED is already active, choose a new one at random
                        break;
                    }
                    else
                    {
                        star_index_accepted = 1;
                    }
                }
            }

            for(int i=0; i<MAX_ACTIVE_STARS-1; i++)
            {
                if(!star_array[i].get_active())
                {
                    star_array[i].set_index(star_index);        //set correct LED index for the new star

                    srand (time(NULL));                         //set random hue and saturation
                    float h_tmp = (float)rand()/(RAND_MAX);
                    srand (time(NULL));
                    float s_tmp = (float)rand()/(RAND_MAX);

                    star_array[i].set_h(h_tmp);
                    star_array[i].set_s(s_tmp);
                    star_array[i].set_v(1.0);

                    star_array[i].set_active(1);

                    break;
                }
            }

            srand(time(NULL));                          //choose a time until a new star appear at random
            frames_to_next_star = rand() % (NEW_STAR_FRAME_MAX-NEW_STAR_FRAME_MIN) + NEW_STAR_FRAME_MIN;

            frame_counter = 0;
        }
        frame_counter++;
    }

    virtual void shader(Vec3& rgb, const PixelInfo &p) const
    {
        for(int i=0; i<MAX_ACTIVE_STARS-1; i++)
        {
            if(star_array[i].get_index() == p.index && star_array[i].get_active())
            {
                /*
                float h_shader = star_array[i].get_h();		//possibility for random hue and saturation for stars
                float s_shader = star_array[i].get_s();
                */
                float v_shader = star_array[i].get_v();

                hsv2rgb(rgb, 0, 0, v_shader);

                break;
            }
        }
    }

private:
    int frame_counter, star_index, frames_to_next_star;
    bool star_index_accepted;
    float timer;

    Star star_array[MAX_ACTIVE_STARS];
};

#endif // STARS_H
