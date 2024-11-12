#ifndef WAV_H
#define WAV_H


enum {
	WAV_DEVICE_PC_CPP     = 0, //0 //WAV_DEVICE_PC_CPP
	WAV_DEVICE_SIPEED_CPP = 1, //1 //WAV_DEVICE_SIPEED_CPP
	WAV_DEVICE_STM32      = 2, //2 //WAV_DEVICE_STM32
};
#define WAV_DEVICE 0 //WAV_DEVICE_PC_CPP


#include <stdio.h>
#include <stdlib.h>
#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
#include <Arduino.h>
// #include "main.h"
#include <SD.h>
#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
#include <string.h>
#include <stdint.h>
#include "main.h"
#include "dma.h"
#include "fatfs.h"
#include "i2c.h"
#include "i2s.h"
#include "sdio.h"
#include "usart.h"
#include "gpio.h"
#include "myTxQueue.h"
#endif //WAV_DEVICE



#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
#define FILENAME_MAX_LEN FILENAME_MAX + 4 /* 4 = strlen(".wav")*/
#define wav_printf printf
#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
#define FILENAME_MAX_LEN 12 //8 + 4  // 4 = strlen(".wav")
#define wav_printf Serial.printf
#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
#define FILENAME_MAX_LEN _MAX_LFN
// extern myTxQueueHandle_t hQ;
#define wav_printf printf
#endif //WAV_DEVICE
#define WAV_WRITE_APPEND UINT32_MAX

#pragma pack(push, 1)
typedef enum {
	WAV_READER = 0,
	WAV_WRITER
}wav_handle_mode_t;
typedef enum {
	WAV_UNKNOWN_DATA = 0,
	WAV_PCM_DATA,
	WAV_NON_PCM_DATA_FACT,
	WAV_NON_PCM_DATA_NOFACT,
}wav_header_standard_t;
typedef enum {
	WAV_SAMPLE_PER_CHANNEL,
	WAV_SAMPLE_ALIGNMENT,
	WAV_SAMPLE_RATE,
	WAV_DATA_BYTES,
	WAV_BIT_DEPTH,
	WAV_BYTE_RATE,
	WAV_CHANNELS,
}wav_header_item_t;
typedef struct _wav_header_pcm {
  // RIFF Header
  char riff_header[4];      // Contains "RIFF"
  uint32_t wav_size;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
  char wave_header[4];      // Contains "WAVE"

  // Format Header
  char fmt_header[4];       // Contains "fmt " (includes trailing space)
  uint32_t fmt_chunk_size;  // Should be 16 for PCM //18 for non-PCM data
  uint16_t audio_format;    // Should be 1 for PCM. 3 for IEEE Float
  uint16_t num_channels;    // num of channel
  uint32_t sample_rate;     // sample rate
  uint32_t byte_rate;       // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
  uint16_t sample_alignment;// num_channels * Bytes Per Sample
  uint16_t bit_depth;       // Number of bits per sample

  // Data
  char data_header[4];      // Contains "data"
  uint32_t data_bytes;      // Number of bytes in data. Number of samples * num_channels * sample byte size
  // uint32_t bytes[]; // Remainder of wave file is bytes
}wav_header_pcm_t;
typedef struct _wav_header_non_pcm_fact {
	// RIFF Header
	char riff_header[4];      // Contains "RIFF"
	uint32_t wav_size;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
	char wave_header[4];      // Contains "WAVE"

	// Format Header
	char fmt_header[4];       // Contains "fmt " (includes trailing space)
	uint32_t fmt_chunk_size;  // Should be 16 for PCM //18 for non-PCM data
	uint16_t audio_format;    // Should be 1 for PCM. 3 for IEEE Float
	uint16_t num_channels;    // num of channel
	uint32_t sample_rate;     // sample rate
	uint32_t byte_rate;       // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
	uint16_t sample_alignment;// num_channels * Bytes Per Sample
	uint16_t bit_depth;       // Number of bits per sample
	uint16_t sizeof_extention;// 0
	// Fact
	char fact_header[4];      // Contains "fact"
	uint32_t fact_chunk_size; // 4
	uint32_t fact_bytes;      // Number of bytes in fact. Number of samples * num_channels
	// Data
	char data_header[4];      // Contains "data"
	uint32_t data_bytes;      // Number of bytes in data. Number of samples * num_channels * sample byte size
	// uint32_t bytes[]; // Remainder of wave file is bytes
}wav_header_non_pcm_fact_t;
typedef struct _wav_header_non_pcm_nofact {
	// RIFF Header
	char riff_header[4];      // Contains "RIFF"
	uint32_t wav_size;        // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
	char wave_header[4];      // Contains "WAVE"

	// Format Header
	char fmt_header[4];       // Contains "fmt " (includes trailing space)
	uint32_t fmt_chunk_size;  // Should be 16 for PCM //18 for non-PCM data
	uint16_t audio_format;    // Should be 1 for PCM. 3 for IEEE Float
	uint16_t num_channels;    // num of channel
	uint32_t sample_rate;     // sample rate
	uint32_t byte_rate;       // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
	uint16_t sample_alignment;// num_channels * Bytes Per Sample
	uint16_t bit_depth;       // Number of bits per sample
	uint16_t sizeof_extention;// 0

	// Data
	char data_header[4];      // Contains "data"
	uint32_t data_bytes;      // Number of bytes in data. Number of samples * num_channels * sample byte size
	// uint32_t bytes[]; // Remainder of wave file is bytes
}wav_header_non_pcm_nofact_t;
typedef union _wav_header_union_tag {
	uint8_t buffer[sizeof(wav_header_non_pcm_fact_t)];
	wav_header_non_pcm_fact_t   non_pcm_fact;
	wav_header_non_pcm_nofact_t non_pcm_nofact;
	wav_header_pcm_t pcm;
}wav_header_union_t;
typedef struct _wav_header_tag {
	wav_header_union_t      head;
	wav_header_standard_t   standard;
	uint32_t                headerSize;
	uint16_t                reserved;
}wav_header_t;
typedef struct _wav_handle_tag {
	char fileName[FILENAME_MAX_LEN];
	wav_header_t header;
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	FILE* file;
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	SDLib::File file;
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	FIL SDFile;
	#endif //WAV_DEVICE
	wav_handle_mode_t RorW;
}wav_handle_t;

typedef int16_t data_1_channel [1];
typedef int16_t data_2_channel [2];
typedef int16_t data_4_channel [4];
typedef int16_t data_8_channel [8];
typedef int16_t data_16_channel[16];

typedef union _data_multi_channel_tag {
	data_1_channel*  chunk1ch;
	data_2_channel*  chunk2ch;
	data_4_channel*  chunk4ch;
	data_8_channel*  chunk8ch;
	data_16_channel* chunk16ch;
}data_multi_channel_t;

/**
 * @brief use for open wav file for read mode
 * 
 * @param hWav 
 * @param wavName 
 * @param needPrintDetails 
 */
void wav_openReadFile(wav_handle_t *hWav, char *wavName, bool needPrintDetails);

/**
 * @brief use for open and creat wav for write mode
 * 
 * @param hWav 
 * @param wavName 
 * @param sampleRate 
 * @param numOfChannel 
 * @param standard from 'wav_header_standard_t'
 * @param needPrintDetails 
 */
void wav_openWriteFile(wav_handle_t *hWav, const char *wavName, uint32_t sampleRate, uint32_t numOfChannel, wav_header_standard_t standard, bool needPrintDetails);

/**
 * @brief 
 * 
 * @param hWav 
 * @param StartInd 
 * @param len 
 * @param wav[len][chNum]
 */
void wav_readSample(wav_handle_t *hWav, uint32_t StartInd, uint32_t len, void **wav);

/**
 * @brief 
 * 
 * @param hWav 
 * @param StartInd use WAV_WRITE_APPEND if need (startInd from file writed index not @param wav [index][ch] !!!)
 * @param len 
 * @param wav[len][chNum]
 */
void wav_WriteSample(wav_handle_t *hWav, uint32_t StartInd, uint32_t len, void **wav);
float wav_normalizeInt16ToFloat(int16_t x);
uint32_t wav_getDetails(wav_header_t* header, wav_header_item_t item);
void wav_printHeader(wav_header_t* header);
void wav_refillHeader(wav_header_t* header);

/**
 * @brief close after wav file works
 * 
 * @param handler 
 */
void wav_close(wav_handle_t* handler);

#pragma pack(pop)

#endif //WAV_H
