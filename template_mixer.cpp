#include <sys/time.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>

#include "lib/effect_runner.h"
#include "lib/effect_mixer.h"

#include "noise.h"
#include "stars.h"
#include "oneshot1.h"

#define MAX_FPS 30
#define NIGHT_REDUCTION 0.4
#define NIGHT_MODE_HOUR_END 8
#define NIGHT_MODE_HOUR_START 19
#define FADER_SPEED 0.02

#define BUFSIZE 128
#define PORT 12345

void * receiver(void * argument);

bool event = 0;

int main(int argc, char **argv)
{
    struct timeval tim;
    int hrs_epoch;
    int hrs_day_local;
    bool day = 1;
    float fader_ambient = 1;
    float fader_oneshot = 0;
    bool trigger = 0;
    bool fade = 0;

    pthread_t t1;

    pthread_create(&t1, NULL, receiver, NULL);

    srand (time(NULL));							//random initial hue and saturation
    float h_init = (float)rand()/(RAND_MAX);
    srand (time(NULL));
    float s_init = (float)rand()/(RAND_MAX);

    NoiseEffect noise_obj(h_init, s_init);
    StarsEffect stars_obj;
    OneshotEffect oneshot_obj;

    EffectMixer mixer;
    mixer.add(&noise_obj);
    mixer.add(&stars_obj);
    mixer.add(&oneshot_obj);

    EffectRunner r;
    r.setEffect(&mixer);
    r.setLayout("grid_8x64.json");
    r.setMaxFrameRate(MAX_FPS);

    if(!r.parseArguments(argc, argv))
    {
        return 1;
    }

    int counter = 0;
    gettimeofday(&tim, NULL);

    hrs_epoch = (floor)(tim.tv_sec/3600);
    hrs_day_local = (hrs_epoch+1) % 24;

    while(true)
    {
        EffectRunner::FrameStatus frame = r.doFrame();

        if(counter >= 1000)	//night mode management
        {
            gettimeofday(&tim, NULL);

            hrs_epoch = (floor)(tim.tv_sec/3600);
            hrs_day_local = (hrs_epoch+1) % 24;

            if(hrs_day_local >= NIGHT_MODE_HOUR_END && hrs_day_local < NIGHT_MODE_HOUR_START)
            {
                day = true;
            }
            else
            {
                day = false;
            }

            counter = 0;
        }

        if(event)	//oneshot event handling and initialization 
        {
            trigger = 1;
            fade = 1;
            event = 0;
        }

        if(trigger && fade && fader_ambient < 0.01)
        {
            trigger = 0; //skip oneshot hold
        }

        if(!trigger && fade && fader_oneshot < 0.01)
        {
            trigger = 0;
            fade = 0;
        }

        if(trigger && fade)	//oneshot fade in
        {
            fader_ambient -= FADER_SPEED;
            fader_oneshot += FADER_SPEED;
        }
        else if(trigger && !fade)   //oneshot hold (skipped)
        {
            fader_ambient = 0;
            fader_oneshot = 1;
        }
        else if(!trigger && fade) //oneshot fade out
        {
            fader_ambient += FADER_SPEED;
            fader_oneshot -= FADER_SPEED;
        }
        else	//oneshot off
        {
            fader_ambient = 1;
            fader_oneshot = 0;
        }

        for(int i=0;i<mixer.numChannels();i++)	//running the mixer
        {
            switch(i){
                case 0:
                    if(day)
                    {
                        mixer.setFader(i,fader_ambient);
                    }
                    else
                    {
                        mixer.setFader(i,fader_ambient-NIGHT_REDUCTION);
                    }
                    break;
                case 1:
                    if(day)
                    {
                        mixer.setFader(i,fader_ambient);
                    }
                    else
                    {
                        mixer.setFader(i,fader_ambient-NIGHT_REDUCTION);
                    }
                    break;
                case 2:
                    if(day)
                    {
                        mixer.setFader(i,fader_oneshot);
                    }
                    break;
                default:
                    mixer.setFader(i,0);
                    break;
            }
        }

        counter++;
    }
}

void * receiver(void * argument) //thread for receiving event messages
{
    unsigned char buf[BUFSIZE];
    int recvlen = 0;

    std::cout << "Receiver thread started" << std::endl;

    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    struct sockaddr_in myaddr;
    socklen_t addrlen = sizeof(myaddr);

    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(PORT);

    bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr));

    while(1)
    {
        recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&myaddr, &addrlen);
        if(recvlen > 0)
        {
            buf[recvlen] = 0;
            std::cout << "received: " << buf << std::endl;
            event = 1;
        }
        usleep(1000);
    }
}
