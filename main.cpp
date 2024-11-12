#include <stdio.h>
#include <stdint.h>
#include <stddef.h>  // for size_t
#include "wav.h"
#include "filterCoef.h"

// char inputWavName[128] = "out.wav";
char inputWavName[128] = "alireza_doa_res.wav";
// char inputWavName[128] = "01001919_filtered.wav";
// char inputWavName[128] = "01002036_filtered.wav";

char outputWavName[128] = "01002036_myFiltered.wav";

int16_t wavBuf[96000 * 2][8] = { 0 };
float  wavBuf_float[96000 * 2][8] = { 0 };
// int16_t readFileBuffer[96000 * 2 * 8] = { 0 };
// wav_header_t wavHeaderMM;
wav_handle_t  hWav;

// FIR filter function that processes the input signal using the given coefficients
int16_t fir_filter(const int16_t* coeffs, size_t coeffs_len, const int16_t* input_signal, size_t signal_len, int16_t* output_signal) {
    // Ensure that output_signal is large enough to hold the filtered result
    for (size_t i = 0; i < signal_len; i++) {
        int32_t filtered_value = 0;
        
        // Apply the FIR filter (multiply and accumulate)
        for (size_t j = 0; j < coeffs_len; j++) {
            if (i >= j) {
                filtered_value += (int32_t)coeffs[j] * (int32_t)input_signal[i - j];
            }
        }
        
        // Scale and store the result
        // Here we use a simple right shift to scale the result back to int16_t range
        filtered_value >>= 15; // Adjust the scaling as necessary based on the coefficients
        
        // Ensure the result is within the valid range for int16_t
        if (filtered_value > 32767) {
            filtered_value = 32767;
        } else if (filtered_value < -32768) {
            filtered_value = -32768;
        }
        
        output_signal[i] = (int16_t)filtered_value;
    }

    return 0;  // Return 0 to indicate success
}

int16_t input_signal[1024] = {0};
int16_t output_signal[1024] = {0};
int main(){
    // Example usage

    printf("Starting...\n");

	wav_openReadFile(&hWav, inputWavName, true);
	uint32_t sampleRate   = wav_getDetails(&(hWav.header), WAV_SAMPLE_RATE);
	uint32_t numOfChannel = wav_getDetails(&(hWav.header), WAV_CHANNELS);
	uint32_t numOfSamplePerChannel = wav_getDetails(&(hWav.header), WAV_SAMPLE_PER_CHANNEL);
	// uint8_t neededCh[8];//numOfChannel
	// for(uint8_t ind = 0; ind < numOfChannel; ind++)
	// 	neededCh[ind] = ind;
	uint32_t numOfSamplePerChannelToRead = numOfSamplePerChannel; //numOfSamplePerChannel
	uint32_t startIndexToRead			 = 0;//0
	wav_readSample(&hWav, startIndexToRead, numOfSamplePerChannelToRead, (void**)&wavBuf[0]);
	// ready wavBufMM
	// for (uint32_t sample = 0; sample < numOfSamplePerChannelToRead; sample++) {
	// 	for (uint32_t ch = 0; ch < numOfChannel; ch++) {
	// 		wavBuf_float[sample][ch] = wav_normalizeInt16ToFloat(wavBuf[sample][ch]);
	// 	}
	// }
	wav_close(&hWav);

    for (uint32_t sample = 0; sample < numOfSamplePerChannel; sample++)
    {
        input_signal[sample] = wavBuf[sample][0];
    }
    


    uint32_t signal_len = numOfSamplePerChannel;
    // Call the FIR filter function
    fir_filter(filterCoef, filterOrder, input_signal, signal_len, output_signal);

    // Print the filtered signal (just for testing, you can replace this with your actual output mechanism)
    for (size_t i = 0; i < signal_len; i++) {
        printf("%d ", output_signal[i]);
    }

    //Write 
	wav_openWriteFile(&hWav, outputWavName, sampleRate, 1, WAV_PCM_DATA, true);
	wav_WriteSample(&hWav, WAV_WRITE_APPEND, numOfSamplePerChannel, (void**)output_signal);
	wav_close(&hWav);
    return 0;
}
