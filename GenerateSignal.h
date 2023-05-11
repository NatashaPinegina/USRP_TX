//
// Created by user on 02.05.23.
//

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

#ifndef UHD_BPSK_H
#define UHD_BPSK_H

#endif //UHD_BPSK_H

enum SignalType{
    BPSK,
    MSK,
    FHSS
};

struct SignalGenerationParams
{
    double duration;
    double f0;
    double samplingFreq;
    double bitrate;
    int maxSize;
};

struct Signal{
    double duration;
    double f0;
    double samplingFreq;
    double* signal;
    double* keysTime;
};

bool RandomBit(double low_chance)
{
    return (rand() < RAND_MAX * low_chance);
}


double* GenerateBPSKSignal(struct SignalGenerationParams param, struct Signal ret_signal)
{
    ret_signal.f0 = param.f0;
    ret_signal.duration = param.duration;
    ret_signal.samplingFreq = param.samplingFreq;
    //int kol_otsh = ret_signal->duration * ret_signal->samplingFreq;
    int kol_otsh = 2 * param.maxSize;
    ret_signal.signal = (double*)malloc(kol_otsh * sizeof(double));
    ret_signal.keysTime = (double*)malloc(kol_otsh * sizeof(double));

    int samples_in_bit = (int)(param.samplingFreq/param.bitrate);
    double sampling_period = 1./param.samplingFreq;

    double dbl_ind = 0;
    int ind = 0;
    while(dbl_ind<param.duration && ind < param.maxSize)
    {
        double bit = RandomBit(0.5);
        double phase = 0.;
        for(int n_sample = 0; n_sample < samples_in_bit; n_sample++)
        {
            if(ind < param.maxSize)
            {
                if (bit == 0) phase = 0.;
                else phase = M_PI;
                ret_signal.signal[ind] = cos(2 * M_PI * ret_signal.f0 * dbl_ind + phase);
                ret_signal.signal[ind+1] = sin(2 * M_PI * ret_signal.f0 * dbl_ind + phase);
                ret_signal.keysTime[ind] = dbl_ind;
                ret_signal.keysTime[ind+1] = dbl_ind;
                ind+=2;
                dbl_ind += sampling_period;
            }
        }
    }

    return ret_signal.signal;
}

double* generateSignal(enum SignalType type, struct SignalGenerationParams param, struct Signal ret_sg)
{
    switch (type)
    {
        case BPSK: return GenerateBPSKSignal(param, ret_sg);
    }
}
