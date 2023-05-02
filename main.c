#include <stdio.h>
#include <uhd.h>
#include <signal.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "GenerateSignal.h"

#define EXECUTE_OR_GOTO(label, ...) \
    if(__VA_ARGS__){ \
        return_code = EXIT_FAILURE; \
        goto label; \
    }

bool stop_signal_called = false;

void sigint_handler(int code){
    (void)code;
    stop_signal_called = true;
}

int main(int argc, char* argv[]){
    int option = 0;
    double dur = 0.001;
    double bitrate = 4800;
    double freq = 1e9;
    double f0 = 3e5;
    double rate = 1e6;
    double gain = 0;
    char* device_args = NULL;
    size_t channel = 0;
    uint64_t total_num_samps = 0;
    bool verbose = false;
    int return_code = EXIT_SUCCESS;
    char error_string[512];

    if (!device_args)
        device_args = strdup("");

    /** Создать дескриптор USRP*/
    uhd_usrp_handle usrp; /** Интерфейс C-уровня для работы с устройством USRP.*/
    EXECUTE_OR_GOTO(free_option_strings,
                    uhd_usrp_make(&usrp, device_args)
    )

    /** Создать дескриптор стримера TX.*/
    uhd_tx_streamer_handle tx_streamer; /** Интерфейс C-уровня для работы с TX-стримером.*/
    EXECUTE_OR_GOTO(free_usrp,
                    uhd_tx_streamer_make(&tx_streamer)
    )

    /** Создать новый дескриптор метаданных TX.*/
    uhd_tx_metadata_handle md; /** Интерфейс метаданных TX для описания полученных данных IF.*/
    EXECUTE_OR_GOTO(free_tx_streamer,
                    uhd_tx_metadata_make(&md, false, 0, 0.1, true, false)
    )

    uhd_tune_request_t tune_request = {
            .target_freq = freq,
            .rf_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO,
            .dsp_freq_policy = UHD_TUNE_REQUEST_POLICY_AUTO
    };
    uhd_tune_result_t tune_result;

    uhd_stream_args_t stream_args = {
            .cpu_format = "fc32",
            .otw_format = "sc16",
            .args = "",
            .channel_list = &channel,
            .n_channels = 1
    };

    size_t samps_per_buff;
    float* buff = NULL;
    const void** buffs_ptr = NULL;

    /** Установить частоту дискретизации данного канала TX (в Sps)*/
    fprintf(stderr, "Setting TX Rate: %f...\n", rate);
    EXECUTE_OR_GOTO(free_tx_metadata,
                    uhd_usrp_set_tx_rate(usrp, rate, channel)
    )

    /** Получить частоту дискретизации данного канала TX (в Sps)*/
    EXECUTE_OR_GOTO(free_tx_metadata,
                    uhd_usrp_get_tx_rate(usrp, channel, &rate)
    )
    fprintf(stderr, "Actual TX Rate: %f...\n\n", rate);

    /** Установите усиление TX для данного канала и имени.*/
    fprintf(stderr, "Setting TX Gain: %f db...\n", gain);
    EXECUTE_OR_GOTO(free_tx_metadata,
                    uhd_usrp_set_tx_gain(usrp, gain, 0, "")
    )

    /** Получите усиление TX данного канала.*/
    EXECUTE_OR_GOTO(free_tx_metadata,
                    uhd_usrp_get_tx_gain(usrp, channel, "", &gain)
    )
    fprintf(stderr, "Actual TX Gain: %f...\n", gain);

    /** Установить центральную частоту передачи данного канала.*/
    fprintf(stderr, "Setting TX frequency: %f MHz...\n", freq / 1e6);
    EXECUTE_OR_GOTO(free_tx_metadata,
                    uhd_usrp_set_tx_freq(usrp, &tune_request, channel, &tune_result)
    )

    /** Получить центральную частоту TX данного канала.*/
    EXECUTE_OR_GOTO(free_tx_metadata,
                    uhd_usrp_get_tx_freq(usrp, channel, &freq)
    )
    fprintf(stderr, "Actual TX frequency: %f MHz...\n", freq / 1e6);

    /** Создать стример TX из дескриптора USRP и заданных аргументов потока.*/
    stream_args.channel_list = &channel;
    EXECUTE_OR_GOTO(free_tx_streamer,
                    uhd_usrp_get_tx_stream(usrp, &stream_args, tx_streamer)
    )

    /** Получите максимальное количество выборок в буфере на пакет.*/
    EXECUTE_OR_GOTO(free_tx_streamer,
                    uhd_tx_streamer_max_num_samps(tx_streamer, &samps_per_buff)
    )
    fprintf(stderr, "Buffer size in samples: %zu\n", samps_per_buff);

    struct SignalGenerationParams param = {dur, f0, rate, bitrate, samps_per_buff};
    enum SignalType type = BPSK;
    struct Signal ret_signal;
    buff = generateSignal(type, param, ret_signal);
    buffs_ptr = (const void**)&buff;
    //buff = calloc(sizeof(float), samps_per_buff * 2);
    //buffs_ptr = (const void**)&buff;
    //size_t i = 0;
    //double samp_per = 1./rate;
   // double time = 0;
    //for(i = 0; i < (samps_per_buff*2); i+=2){
     //   buff[i] = cos(2*M_PI*f0*time);
     //   buff[i+1] = sin(2*M_PI*f0*time);
     //   time+=samp_per;
    //}

    /** Обработка завершения.*/
    signal(SIGINT, &sigint_handler);
    fprintf(stderr, "Press Ctrl+C to stop streaming...\n");

    uint64_t num_acc_samps = 0;
    size_t num_samps_sent = 0;

    while(1) {
        if (stop_signal_called) break;
        if (total_num_samps > 0 && num_acc_samps >= total_num_samps) break;

        EXECUTE_OR_GOTO(free_buff,
                        uhd_tx_streamer_send(tx_streamer, buffs_ptr, samps_per_buff, &md, 0.1, &num_samps_sent)
        )

        num_acc_samps += num_samps_sent;

        if(verbose){
            fprintf(stderr, "Sent %zu samples\n", num_samps_sent);
        }
    }

    free_buff:
    free(buff);

    free_tx_streamer:
    if(verbose){
        fprintf(stderr, "Cleaning up TX streamer.\n");
    }
    uhd_tx_streamer_free(&tx_streamer);

    free_tx_metadata:
    if(verbose){
        fprintf(stderr, "Cleaning up TX metadata.\n");
    }
    uhd_tx_metadata_free(&md);

    free_usrp:
    if(verbose){
        fprintf(stderr, "Cleaning up USRP.\n");
    }
    if(return_code != EXIT_SUCCESS && usrp != NULL){
        uhd_usrp_last_error(usrp, error_string, 512);
        fprintf(stderr, "USRP reported the following error: %s\n", error_string);
    }
    uhd_usrp_free(&usrp);

    free_option_strings:
    if(device_args)
        free(device_args);

    fprintf(stderr, (return_code ? "Failure\n" : "Success\n"));

    return return_code;
}
