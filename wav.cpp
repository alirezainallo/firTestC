#include "wav.h"

// #pragma warning(disable  : 4996)


const char WAV_HEADER_STR_RIFF[5] = "RIFF";
const char WAV_HEADER_STR_WAVE[5] = "WAVE";
const char WAV_HEADER_STR_FMT [5] = "fmt ";
const char WAV_HEADER_STR_DATA[5] = "data";
const char WAV_HEADER_STR_FACT[5] = "fact";

// #if  WAV_DEVICE == 2 //WAV_DEVICE_STM32
// void wav_printf(const char *format, ...) {
//     va_list args;
//     va_start(args, format);
//     // Forward all arguments to myTxQueue_printf
//     myTxQueue_printf(&hQ, format, args);
//     va_end(args);
// }
// #endif //WAV_DEVICE

float wav_normalizeInt16ToFloat(int16_t x) {
	return x < 0 ? x / (float)(32768) : x / (float)(32767);
}
void wav_refillHeader(wav_header_t* header) {
	if (header->head.pcm.fmt_chunk_size == 16) {
		header->standard   = WAV_PCM_DATA;
		header->headerSize = sizeof(wav_header_pcm_t);
	}
	else if (header->head.pcm.fmt_chunk_size == 18) {
		if (memcmp(header->head.non_pcm_fact.fact_header, "fact", 3) == 0) {
			header->standard   = WAV_NON_PCM_DATA_FACT;
			header->headerSize = sizeof(wav_header_non_pcm_fact_t);
		}
		else
		{
			header->standard   = WAV_NON_PCM_DATA_NOFACT;
			header->headerSize = sizeof(wav_header_non_pcm_nofact_t);
		}
	}
	else {
		header->standard = WAV_UNKNOWN_DATA;
		header->headerSize = 0;
	}
}
void wav_fillHeader(wav_header_t* header, uint32_t sampleRate, uint32_t numOfChannel, wav_header_standard_t standard) {
	switch (standard)
	{
	case WAV_PCM_DATA:
		header->standard   = WAV_PCM_DATA;
		header->headerSize = sizeof(wav_header_pcm_t);

		// VALUE
		header->head.pcm.num_channels = (uint16_t)numOfChannel;
		header->head.pcm.sample_rate  = sampleRate;
		
		// CONST
		header->head.pcm.audio_format     = 1;  			  // Should be 1 for PCM. 3 for IEEE Float
		header->head.pcm.bit_depth		  = 16; 			  // Number of bits per sample
		header->head.pcm.fmt_chunk_size	  = 16; 			  // Should be 16 for PCM //18 for non-PCM data
		header->head.pcm.sample_alignment = (uint16_t)(numOfChannel * 2); // num_channels * Bytes Per Sample

		// HEADER
		memcpy(header->head.pcm.data_header, WAV_HEADER_STR_DATA, 4);
		memcpy(header->head.pcm.fmt_header,  WAV_HEADER_STR_FMT , 4);
		memcpy(header->head.pcm.riff_header, WAV_HEADER_STR_RIFF, 4);
		memcpy(header->head.pcm.wave_header, WAV_HEADER_STR_WAVE, 4);

		// SIZING
		header->head.pcm.wav_size   = 0; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
		header->head.pcm.byte_rate  = 0; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
		header->head.pcm.data_bytes = 0; // Number of bytes in data. Number of samples * num_channels * sample byte size

		break;
	case WAV_NON_PCM_DATA_FACT:
		header->standard   = WAV_NON_PCM_DATA_FACT;
		header->headerSize = sizeof(wav_header_non_pcm_fact_t);
		
		// VALUE
		header->head.non_pcm_fact.num_channels = (uint16_t)numOfChannel;
		header->head.non_pcm_fact.sample_rate  = sampleRate;
		
		// CONST
		header->head.non_pcm_fact.audio_format     = 1;  			   // Should be 1 for PCM. 3 for IEEE Float
		header->head.non_pcm_fact.bit_depth		   = 16; 			   // Number of bits per sample
		header->head.non_pcm_fact.fmt_chunk_size   = 18; 			   // Should be 16 for PCM //18 for non-PCM data
		header->head.non_pcm_fact.sample_alignment = (uint16_t)(numOfChannel * 2); // num_channels * Bytes Per Sample
		header->head.non_pcm_fact.sizeof_extention = 0;                // 0
		header->head.non_pcm_fact.fact_chunk_size  = 4;                // 4

		// HEADER
		memcpy(header->head.non_pcm_fact.data_header, WAV_HEADER_STR_DATA, 4);
		memcpy(header->head.non_pcm_fact.fmt_header,  WAV_HEADER_STR_FMT , 4);
		memcpy(header->head.non_pcm_fact.riff_header, WAV_HEADER_STR_RIFF, 4);
		memcpy(header->head.non_pcm_fact.wave_header, WAV_HEADER_STR_WAVE, 4);
		memcpy(header->head.non_pcm_fact.fact_header, WAV_HEADER_STR_FACT, 4);

		// SIZING
		header->head.non_pcm_fact.wav_size   = 0; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
		header->head.non_pcm_fact.byte_rate  = 0; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
		header->head.non_pcm_fact.data_bytes = 0; // Number of bytes in data. Number of samples * num_channels * sample byte size
		header->head.non_pcm_fact.fact_bytes = 0; // Number of bytes in fact. Number of samples * num_channels

		break;
	case WAV_NON_PCM_DATA_NOFACT:
		header->standard   = WAV_NON_PCM_DATA_NOFACT;
		header->headerSize = sizeof(wav_header_non_pcm_nofact_t);
		
		// VALUE
		header->head.non_pcm_nofact.num_channels = (uint16_t)numOfChannel;
		header->head.non_pcm_nofact.sample_rate  = sampleRate;
		
		// CONST
		header->head.non_pcm_nofact.audio_format     = 1;  			     // Should be 1 for PCM. 3 for IEEE Float
		header->head.non_pcm_nofact.bit_depth		 = 16;     			 // Number of bits per sample
		header->head.non_pcm_nofact.fmt_chunk_size   = 18; 			     // Should be 16 for PCM //18 for non-PCM data
		header->head.non_pcm_nofact.sample_alignment = (uint16_t)(numOfChannel * 2); // num_channels * Bytes Per Sample
		header->head.non_pcm_nofact.sizeof_extention = 0;                // 0

		// HEADER
		memcpy(header->head.non_pcm_nofact.data_header, WAV_HEADER_STR_DATA, 4);
		memcpy(header->head.non_pcm_nofact.fmt_header,  WAV_HEADER_STR_FMT , 4);
		memcpy(header->head.non_pcm_nofact.riff_header, WAV_HEADER_STR_RIFF, 4);
		memcpy(header->head.non_pcm_nofact.wave_header, WAV_HEADER_STR_WAVE, 4);

		// SIZING
		header->head.non_pcm_nofact.wav_size   = 0; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
		header->head.non_pcm_nofact.byte_rate  = 0; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
		header->head.non_pcm_nofact.data_bytes = 0; // Number of bytes in data. Number of samples * num_channels * sample byte size

		break;
	
	default:
		break;
	}
	
}
uint32_t wav_getDetails(wav_header_t* header, wav_header_item_t item) {
	uint32_t retVal = 0;

	switch (item)
	{
	case WAV_SAMPLE_PER_CHANNEL:
		switch (header->standard)
		{
		case WAV_UNKNOWN_DATA:

			break;
		case WAV_PCM_DATA:                                                             //num of byte per sample
			retVal = header->head.pcm.data_bytes / (uint32_t)(header->head.pcm.num_channels * (header->head.pcm.bit_depth / 8));
			break;
		case WAV_NON_PCM_DATA_FACT:                                                             //num of byte per sample
			retVal = header->head.non_pcm_fact.data_bytes / (uint32_t)(header->head.pcm.num_channels * (header->head.pcm.bit_depth / 8));
			break;
		case WAV_NON_PCM_DATA_NOFACT:                                                             //num of byte per sample
			retVal = header->head.non_pcm_nofact.data_bytes / (uint32_t)(header->head.pcm.num_channels * (header->head.pcm.bit_depth / 8));
			break;
		default:
			break;
		}
		break;
	case WAV_SAMPLE_ALIGNMENT:
		retVal = header->head.pcm.sample_alignment;
		break;
	case WAV_SAMPLE_RATE:
		retVal = header->head.pcm.sample_rate;
		break;
	case WAV_DATA_BYTES:
		switch (header->standard)
		{
		case WAV_UNKNOWN_DATA:

			break;
		case WAV_PCM_DATA:
			retVal = header->head.pcm.data_bytes;
			break;
		case WAV_NON_PCM_DATA_FACT:
			retVal = header->head.non_pcm_fact.data_bytes;
			break;
		case WAV_NON_PCM_DATA_NOFACT:
			retVal = header->head.non_pcm_nofact.data_bytes;
			break;
		default:
			break;
		}
		break;
	case WAV_BIT_DEPTH:
		retVal = header->head.pcm.bit_depth;
		break;
	case WAV_BYTE_RATE:
		retVal = header->head.pcm.byte_rate;
		break;
	case WAV_CHANNELS:
		retVal = header->head.pcm.num_channels;
		break;
	default:
		break;
	}
	
	return retVal;
}
void wav_printHeader(wav_header_t* header) {
	char printBufTmp[128] = { 0 };

	switch (header->standard)
	{
	case WAV_UNKNOWN_DATA:
		wav_printf("[WAV] wrong header format :(\n");
		break;
	case WAV_PCM_DATA:
		wav_printf("[WAV] header format : WAV_PCM_DATA\n");
		memcpy(printBufTmp, header->head.pcm.riff_header, 4);
		printBufTmp[4] = 0;
		wav_printf("riff_header         : %s\n", printBufTmp);
		wav_printf("wav_size            : %d\n", header->head.pcm.wav_size);
		memcpy(printBufTmp, header->head.pcm.wave_header, 4);
		printBufTmp[4] = 0;
		wav_printf("wave_header         : %s\n", printBufTmp);
		memcpy(printBufTmp, header->head.pcm.fmt_header, 4);
		printBufTmp[4] = 0;
		wav_printf("fmt_header          : %s\n", printBufTmp);
		wav_printf("fmt_chunk_size      : %d\n", header->head.pcm.fmt_chunk_size);
		wav_printf("audio_format        : %d\n", header->head.pcm.audio_format);
		wav_printf("num_channels        : %d\n", header->head.pcm.num_channels);
		wav_printf("sample_rate         : %d\n", header->head.pcm.sample_rate);
		wav_printf("byte_rate           : %d\n", header->head.pcm.byte_rate);
		wav_printf("sample_alignment    : %d\n", header->head.pcm.sample_alignment);
		wav_printf("bit_depth           : %d\n", header->head.pcm.bit_depth);
		memcpy(printBufTmp, header->head.pcm.data_header, 4);
		printBufTmp[4] = 0;
		wav_printf("data_header         : %s\n", printBufTmp);
		wav_printf("data_bytes          : %d\n", header->head.pcm.data_bytes);
		wav_printf("numOfSamplePerCh    : %d\n", wav_getDetails(header, WAV_SAMPLE_PER_CHANNEL));
		break;
	case WAV_NON_PCM_DATA_FACT:
		wav_printf("[WAV] header format : WAV_NON_PCM_DATA_FACT\n");
		memcpy(printBufTmp, header->head.non_pcm_fact.riff_header, 4);
		printBufTmp[4] = 0;
		wav_printf("riff_header         : %s\n", printBufTmp);
		wav_printf("wav_size            : %d\n", header->head.non_pcm_fact.wav_size);
		memcpy(printBufTmp, header->head.non_pcm_fact.wave_header, 4);
		printBufTmp[4] = 0;
		wav_printf("wave_header         : %s\n", printBufTmp);
		memcpy(printBufTmp, header->head.non_pcm_fact.fmt_header, 4);
		printBufTmp[4] = 0;
		wav_printf("fmt_header          : %s\n", printBufTmp);
		wav_printf("fmt_chunk_size      : %d\n", header->head.non_pcm_fact.fmt_chunk_size);
		wav_printf("audio_format        : %d\n", header->head.non_pcm_fact.audio_format);
		wav_printf("num_channels        : %d\n", header->head.non_pcm_fact.num_channels);
		wav_printf("sample_rate         : %d\n", header->head.non_pcm_fact.sample_rate);
		wav_printf("byte_rate           : %d\n", header->head.non_pcm_fact.byte_rate);
		wav_printf("sample_alignment    : %d\n", header->head.non_pcm_fact.sample_alignment);
		wav_printf("bit_depth           : %d\n", header->head.non_pcm_fact.bit_depth);
		wav_printf("sizeof_extention    : %d\n", header->head.non_pcm_fact.sizeof_extention);
		memcpy(printBufTmp, header->head.non_pcm_fact.fact_header, 4);
		printBufTmp[4] = 0;
		wav_printf("fact_header         : %s\n", printBufTmp);
		wav_printf("fact_chunk_size     : %d\n", header->head.non_pcm_fact.fact_chunk_size);
		wav_printf("fact_bytes          : %d\n", header->head.non_pcm_fact.fact_bytes);
		memcpy(printBufTmp, header->head.non_pcm_fact.data_header, 4);
		printBufTmp[4] = 0;
		wav_printf("data_header         : %s\n", printBufTmp);
		wav_printf("data_bytes          : %d\n", header->head.non_pcm_fact.data_bytes);
		wav_printf("numOfSamplePerCh    : %d\n", wav_getDetails(header, WAV_SAMPLE_PER_CHANNEL));
		break;
	case WAV_NON_PCM_DATA_NOFACT:
		wav_printf("[WAV] header format : WAV_NON_PCM_DATA_NOFACT\n");
		memcpy(printBufTmp, header->head.non_pcm_nofact.riff_header, 4);
		printBufTmp[4] = 0;
		wav_printf("riff_header         : %s\n", printBufTmp);
		wav_printf("wav_size            : %d\n", header->head.non_pcm_nofact.wav_size);
		memcpy(printBufTmp, header->head.non_pcm_nofact.wave_header, 4);
		printBufTmp[4] = 0;
		wav_printf("wave_header         : %s\n", printBufTmp);
		memcpy(printBufTmp, header->head.non_pcm_nofact.fmt_header, 4);
		printBufTmp[4] = 0;
		wav_printf("fmt_header          : %s\n", printBufTmp);
		wav_printf("fmt_chunk_size      : %d\n", header->head.non_pcm_nofact.fmt_chunk_size);
		wav_printf("audio_format        : %d\n", header->head.non_pcm_nofact.audio_format);
		wav_printf("num_channels        : %d\n", header->head.non_pcm_nofact.num_channels);
		wav_printf("sample_rate         : %d\n", header->head.non_pcm_nofact.sample_rate);
		wav_printf("byte_rate           : %d\n", header->head.non_pcm_nofact.byte_rate);
		wav_printf("sample_alignment    : %d\n", header->head.non_pcm_nofact.sample_alignment);
		wav_printf("bit_depth           : %d\n", header->head.non_pcm_nofact.bit_depth);
		wav_printf("sizeof_extention    : %d\n", header->head.non_pcm_nofact.sizeof_extention);
		memcpy(printBufTmp, header->head.non_pcm_nofact.data_header, 4);
		printBufTmp[4] = 0;
		wav_printf("data_header         : %s\n", printBufTmp);
		wav_printf("data_bytes          : %d\n", header->head.non_pcm_nofact.data_bytes);
		wav_printf("numOfSamplePerCh    : %d\n", wav_getDetails(header, WAV_SAMPLE_PER_CHANNEL));
		break;
	default:
		break;
	}
}
void wav_openReadFile(wav_handle_t *hWav, char *wavName, bool needPrintDetails) {
	
	hWav->RorW = WAV_READER;

	// copy file name
	strncpy(hWav->fileName, wavName, FILENAME_MAX_LEN);

	// open file
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	hWav->file = 0;
	hWav->file = fopen(hWav->fileName, "rb+");
	if (!hWav->file) {
		wav_printf("cannot open hWav->wavFile : %s\n", hWav->fileName);
	}
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file = SD.open(wavName, O_READ);
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	f_open(&hWav->SDFile, wavName, FA_READ);
	#endif //WAV_DEVICE

	// read header
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	fread(&(hWav->header.head.buffer), 1, sizeof(wav_header_non_pcm_fact_t), hWav->file);
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file.read((void*)(hWav->header.head.buffer), (uint32_t)sizeof(wav_header_non_pcm_fact_t));
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	size_tbr;
	f_read(&hWav->SDFile, (void*)(hWav->header.head.buffer), (UINT)sizeof(wav_header_non_pcm_fact_t), &br);(void)br;
	#endif //WAV_DEVICE
	wav_refillHeader(&(hWav->header));
	if(needPrintDetails){
		wav_printf("[wav] ----------------------------------------------\n");
		wav_printHeader(&(hWav->header));
	}

	//calculate size
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	fseek(hWav->file, 0, SEEK_END);
	uint32_t file_size = (uint32_t)ftell(hWav->file);
	fseek(hWav->file, 0, SEEK_SET);
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file.seek(0);
	uint32_t file_size = hWav->file.size();
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	f_lseek(&hWav->SDFile, 0);
	uint32_t file_size = (uint32_t)f_size(&(hWav->SDFile));
	#endif //WAV_DEVICE
	
	if(needPrintDetails){
		wav_printf("file_size           : %d\n", file_size);
		wav_printf("----------------------------------------------------\n");
	}
}
void wav_openWriteFile(wav_handle_t *hWav, const char *wavName, uint32_t sampleRate, uint32_t numOfChannel, wav_header_standard_t standard, bool needPrintDetails) {
	
	hWav->RorW = WAV_WRITER;

	// copy file name
	//strncpy(hWav->fileName, wavName, FILENAME_MAX_LEN);
	strcpy(hWav->fileName, wavName);

	// open file
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	hWav->file = 0;
	hWav->file = fopen(hWav->fileName, "wb+"); //destroy data if file was exist :(
	if (!hWav->file) {
		wav_printf("cannot open hWav->wavFile : %s\n", hWav->fileName);
	}
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file = SD.open(wavName, O_WRITE | O_CREAT);
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	FRESULT res = f_open(&hWav->SDFile, wavName, FA_WRITE | FA_CREATE_ALWAYS);
	if (res != FR_OK){
		//HAL_UART_Transmit(&huart1, wavName, strlen(wavName), 10);
		printf("cannot open hWav->wavFile : %s, %d\n", wavName, (uint32_t)res);
	}
	#endif //WAV_DEVICE

	// fill header
	wav_fillHeader(&(hWav->header), sampleRate, numOfChannel, standard);
	if(needPrintDetails){
		wav_printHeader(&(hWav->header));
	}

	uint32_t writtenByte = 0;
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	writtenByte = (uint32_t)fwrite(hWav->header.head.buffer, 1, hWav->header.headerSize, hWav->file);
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	writtenByte = (uint32_t)hWav->file.write(hWav->header.head.buffer, hWav->header.headerSize);
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	size_tbytLeft = 0;
	f_write(&hWav->SDFile, hWav->header.head.buffer, (UINT)hWav->header.headerSize, &bytLeft);
	HAL_Delay(5);
	writtenByte = (uint32_t)bytLeft;
	#endif //WAV_DEVICE
	if(writtenByte != hWav->header.headerSize){
		wav_printf("must be wrote %d bytes for header but just written %d bytes!\n", writtenByte, hWav->header.headerSize);
	}

	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	fseek(hWav->file, (long)(hWav->header.headerSize), SEEK_SET);
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file.seek(hWav->header.headerSize);
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	f_lseek(&hWav->SDFile, (FSIZE_t)hWav->header.headerSize);
	#endif //WAV_DEVICE

}
//                                                                              wav[len][chNum]
void wav_readSample(wav_handle_t *hWav, uint32_t StartInd, uint32_t len, void **wav) {

	//read from file
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	fseek(hWav->file, (long)(hWav->header.headerSize), SEEK_SET);
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file.seek(hWav->header.headerSize);
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	f_lseek(&hWav->SDFile, (FSIZE_t)hWav->header.headerSize);
	#endif //WAV_DEVICE
	uint32_t numOfChannel = wav_getDetails(&hWav->header, WAV_CHANNELS); // 8;

	uint32_t numOfSamplePerChannel = wav_getDetails(&hWav->header, WAV_SAMPLE_PER_CHANNEL); // 192000;
	if ((numOfSamplePerChannel - StartInd) <= len){
		// len is more than file sample
		len = (numOfSamplePerChannel - StartInd);
	}
	else {
		// len is Ok
	}

	// set to start index
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	fseek(hWav->file, (long)(hWav->header.headerSize + (uint32_t)(StartInd * numOfChannel * (uint32_t)sizeof(int16_t))), SEEK_SET);
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file.seek(hWav->header.headerSize + (uint32_t)(StartInd * numOfChannel * (uint32_t)sizeof(int16_t)));
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	f_lseek(&hWav->SDFile, (FSIZE_t)(hWav->header.headerSize + (uint32_t)(StartInd * numOfChannel * (uint32_t)sizeof(int16_t))));
	#endif //WAV_DEVICE

	//ready wav
	// sizeof(data_multi_channel_t);
	// data_multi_channel_t tmp;
	// sizeof(data_2_channel);
	// sizeof(tmp.chunk2ch);
	// sizeof(data_4_channel);
	// sizeof(tmp.chunk4ch);
	// sizeof(data_8_channel);
	// sizeof(tmp.chunk8ch);
	// sizeof(data_16_channel);
	// sizeof(tmp.chunk16ch);

	data_multi_channel_t wavMulti = {0};
	uint32_t readSampleChunks = 0;
	size_tbr;
	switch (numOfChannel)
	{
	case 1:
		wavMulti.chunk1ch = (data_1_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		readSampleChunks = (uint32_t)fread(wavMulti.chunk1ch, sizeof(data_1_channel), len, hWav->file);
		if (readSampleChunks != (len)) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		readSampleChunks = (uint32_t)hWav->file.read((void*)(wavMulti.chunk1ch), len * (uint32_t)sizeof(data_1_channel));
		if (readSampleChunks != (len * sizeof(data_1_channel))) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_read(&hWav->SDFile, (void*)(wavMulti.chunk1ch), (UINT)(len * sizeof(data_1_channel)), &br);
		HAL_Delay(5);
		readSampleChunks = (uint32_t)br;
		if (readSampleChunks != (len * sizeof(data_1_channel))) {
			wav_printf("readSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	case 2:
		wavMulti.chunk2ch = (data_2_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		readSampleChunks = (uint32_t)fread(wavMulti.chunk2ch, sizeof(data_2_channel), len, hWav->file);
		if (readSampleChunks != (len)) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		readSampleChunks = (uint32_t)hWav->file.read((void*)(wavMulti.chunk2ch), len * (uint32_t)sizeof(data_2_channel));
		if (readSampleChunks != (len * sizeof(data_2_channel))) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_read(&hWav->SDFile, (void*)(wavMulti.chunk2ch), (UINT)(len * sizeof(data_2_channel)), &br);
		HAL_Delay(5);
		readSampleChunks = (uint32_t)br;
		if (readSampleChunks != (len * sizeof(data_2_channel))) {
			wav_printf("readSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	case 4:
		wavMulti.chunk4ch = (data_4_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		readSampleChunks = (uint32_t)fread(wavMulti.chunk4ch, sizeof(data_4_channel), len, hWav->file);
		if (readSampleChunks != (len)) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		readSampleChunks = (uint32_t)hWav->file.read((void*)(wavMulti.chunk4ch), len * (uint32_t)sizeof(data_4_channel));
		if (readSampleChunks != (len * sizeof(data_4_channel))) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_read(&hWav->SDFile, (void*)(wavMulti.chunk4ch), (UINT)(len * sizeof(data_4_channel)), &br);
		HAL_Delay(5);
		readSampleChunks = (uint32_t)br;
		if (readSampleChunks != (len * sizeof(data_4_channel))) {
			wav_printf("readSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	case 8:
		wavMulti.chunk8ch = (data_8_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		readSampleChunks = (uint32_t)fread(wavMulti.chunk8ch, sizeof(data_8_channel), len, hWav->file);
		if (readSampleChunks != (len)) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		readSampleChunks = (uint32_t)hWav->file.read((void*)(wavMulti.chunk8ch), len * (uint32_t)sizeof(data_8_channel));
		if (readSampleChunks != (len * sizeof(data_8_channel))) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_read(&hWav->SDFile, (void*)(wavMulti.chunk8ch), (UINT)(len * sizeof(data_8_channel)), &br);
		HAL_Delay(5);
		readSampleChunks = (uint32_t)br;
		if (readSampleChunks != (len * sizeof(data_8_channel))) {
			wav_printf("readSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	case 16:
		wavMulti.chunk16ch = (data_16_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		readSampleChunks = (uint32_t)fread(wavMulti.chunk16ch, sizeof(data_16_channel), len, hWav->file);
		if (readSampleChunks != (len)) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		readSampleChunks = (uint32_t)hWav->file.read((void*)(wavMulti.chunk16ch), len * (uint32_t)sizeof(data_16_channel));
		if (readSampleChunks != (len * sizeof(data_16_channel))) {
			wav_printf("readSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_read(&hWav->SDFile, (void*)(wavMulti.chunk16ch), (UINT)(len * sizeof(data_16_channel)), &br);
		HAL_Delay(5);
		readSampleChunks = (uint32_t)br;
		if (readSampleChunks != (len * sizeof(data_16_channel))) {
			wav_printf("readSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	
	default:
		break;
	}

}
//                                       use WAV_WRITE_APPEND for add audio segment to end of file
void wav_WriteSample(wav_handle_t *hWav, uint32_t StartInd, uint32_t len, void **wav) {
	
	if(hWav->RorW != WAV_WRITER)
		return ;

	// //write to file
	// #if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	// fseek(hWav->file, (long)(hWav->header.headerSize), SEEK_SET);
	// #elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	// hWav->file.seek(hWav->header.headerSize);
	// #elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	// f_lseek(&hWav->SDFile, (FSIZE_t)hWav->header.headerSize);
	// #endif //WAV_DEVICE
	uint32_t numOfChannel = wav_getDetails(&hWav->header, WAV_CHANNELS); // 8;
	// uint32_t numOfSamplePerChannel = wav_getDetails(&hWav->header, WAV_SAMPLE_PER_CHANNEL); // 192000;

	if(StartInd != WAV_WRITE_APPEND){
		// set to start index
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		fseek(hWav->file, (long)(hWav->header.headerSize + (uint32_t)(StartInd * numOfChannel * (uint32_t)sizeof(int16_t))), SEEK_SET);
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		hWav->file.seek(hWav->header.headerSize + (uint32_t)(StartInd * numOfChannel * (uint32_t)sizeof(int16_t)));
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_lseek(&hWav->SDFile, (FSIZE_t)(hWav->header.headerSize + (uint32_t)(StartInd * numOfChannel * (uint32_t)sizeof(int16_t))));
		#endif //WAV_DEVICE
	}

	//ready wav
	// sizeof(data_multi_channel_t);
	// data_multi_channel_t tmp;
	// sizeof(data_2_channel);
	// sizeof(tmp.chunk2ch);
	// sizeof(data_4_channel);
	// sizeof(tmp.chunk4ch);
	// sizeof(data_8_channel);
	// sizeof(tmp.chunk8ch);
	// sizeof(data_16_channel);
	// sizeof(tmp.chunk16ch);

	data_multi_channel_t wavMulti = {0};
	uint32_t writeSampleChunks = 0;
	size_tbytLeft = 0;
	switch (numOfChannel)
	{
	case 1:
		wavMulti.chunk1ch = (data_1_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		writeSampleChunks = (uint32_t)fwrite(wavMulti.chunk1ch, sizeof(data_1_channel), len, hWav->file);
		if (writeSampleChunks != (len)) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		writeSampleChunks = (uint32_t)hWav->file.write((const char*)(wavMulti.chunk1ch), (size_t)len * (size_t)sizeof(data_1_channel));
		if (writeSampleChunks != (len * sizeof(data_1_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_write(&hWav->SDFile, wavMulti.chunk1ch, (UINT)((size_t)len * (size_t)sizeof(data_1_channel)), &bytLeft);
		HAL_Delay(10);
		writeSampleChunks = (uint32_t)bytLeft;
		if (writeSampleChunks != (len * sizeof(data_1_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	case 2:
		wavMulti.chunk2ch = (data_2_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		writeSampleChunks = (uint32_t)fwrite(wavMulti.chunk2ch, sizeof(data_2_channel), len, hWav->file);
		if (writeSampleChunks != (len)) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		writeSampleChunks = (uint32_t)hWav->file.write((const char*)(wavMulti.chunk2ch), (size_t)len * (size_t)sizeof(data_2_channel));
		if (writeSampleChunks != (len * sizeof(data_2_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_write(&hWav->SDFile, wavMulti.chunk2ch, (UINT)((size_t)len * (size_t)sizeof(data_2_channel)), &bytLeft);
		HAL_Delay(10);
		writeSampleChunks = (uint32_t)bytLeft;
		if (writeSampleChunks != (len * sizeof(data_2_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	case 4:
		wavMulti.chunk4ch = (data_4_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		writeSampleChunks = (uint32_t)fwrite(wavMulti.chunk4ch, sizeof(data_4_channel), len, hWav->file);
		if (writeSampleChunks != (len)) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		writeSampleChunks = (uint32_t)hWav->file.write((const char*)(wavMulti.chunk4ch), (size_t)len * (size_t)sizeof(data_4_channel));
		if (writeSampleChunks != (len * sizeof(data_4_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_write(&hWav->SDFile, wavMulti.chunk4ch, (UINT)((size_t)len * (size_t)sizeof(data_4_channel)), &bytLeft);
		HAL_Delay(10);
		writeSampleChunks = (uint32_t)bytLeft;
		if (writeSampleChunks != (len * sizeof(data_4_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	case 8:
		wavMulti.chunk8ch = (data_8_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		writeSampleChunks = (uint32_t)fwrite(wavMulti.chunk8ch, sizeof(data_8_channel), len, hWav->file);
		if (writeSampleChunks != (len)) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		writeSampleChunks = (uint32_t)hWav->file.write((const char*)(wavMulti.chunk8ch), (size_t)len * (size_t)sizeof(data_8_channel));
		if (writeSampleChunks != (len * sizeof(data_8_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_write(&hWav->SDFile, wavMulti.chunk8ch, (UINT)((size_t)len * (size_t)sizeof(data_8_channel)), &bytLeft);
		HAL_Delay(10);
		writeSampleChunks = (uint32_t)bytLeft;
		if (writeSampleChunks != (len * sizeof(data_8_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	case 16:
		wavMulti.chunk16ch = (data_16_channel *)wav;
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		writeSampleChunks = (uint32_t)fwrite(wavMulti.chunk16ch, sizeof(data_16_channel), len, hWav->file);
		if (writeSampleChunks != (len)) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		writeSampleChunks = (uint32_t)hWav->file.write((const char*)(wavMulti.chunk16ch), (size_t)len * (size_t)sizeof(data_16_channel));
		if (writeSampleChunks != (len * sizeof(data_16_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_write(&hWav->SDFile, wavMulti.chunk16ch, (UINT)((size_t)len * (size_t)sizeof(data_16_channel)), &bytLeft);
		HAL_Delay(10);
		writeSampleChunks = (uint32_t)bytLeft;
		if (writeSampleChunks != (len * sizeof(data_16_channel))) {
			wav_printf("writeSamples failed\n");
		}
		#endif //WAV_DEVICE
		break;
	
	default:
		break;
	}

	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	fflush(hWav->file);
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file.flush();
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	// f_flu(&hWav->SDFile);
	#endif //WAV_DEVICE

}
void wav_close(wav_handle_t* hWav) {
	uint32_t writtenByte = 0;
	
	//calculate size
	#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
	fseek(hWav->file, 0, SEEK_END);
	uint32_t file_size = (uint32_t)ftell(hWav->file);
	fseek(hWav->file, 0, SEEK_SET);
	#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
	hWav->file.seek(0);
	uint32_t file_size = hWav->file.size();
	#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
	f_lseek(&hWav->SDFile, 0);
	uint32_t file_size = (uint32_t)f_size(&(hWav->SDFile));
	#endif //WAV_DEVICE

	switch (hWav->RorW)
	{
	case WAV_READER:
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		fclose(hWav->file);
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		hWav->file.close();
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_close(&hWav->SDFile);
		#endif //WAV_DEVICE
		memset(hWav->fileName, 0, FILENAME_MAX_LEN * sizeof(char));
		memset(&(hWav->header), 0, sizeof(wav_header_t));
		hWav->RorW = (wav_handle_mode_t)0;
		break;
	case WAV_WRITER:
		switch (hWav->header.standard)
		{
		case WAV_PCM_DATA:
			// SIZING
			hWav->header.head.pcm.wav_size   = file_size - 8; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
			hWav->header.head.pcm.byte_rate  = hWav->header.head.pcm.sample_rate * hWav->header.head.pcm.num_channels * 2; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
			hWav->header.head.pcm.data_bytes = (file_size - hWav->header.headerSize); // Number of bytes in data. Number of samples * num_channels * sample byte size
			break;
		case WAV_NON_PCM_DATA_FACT:
			// SIZING
			hWav->header.head.non_pcm_fact.wav_size   = file_size - 8; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
			hWav->header.head.non_pcm_fact.byte_rate  = hWav->header.head.non_pcm_fact.sample_rate * hWav->header.head.non_pcm_fact.num_channels * 2; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
			hWav->header.head.non_pcm_fact.data_bytes = (file_size - hWav->header.headerSize); // Number of bytes in data. Number of samples * num_channels * sample byte size
			hWav->header.head.non_pcm_fact.fact_bytes = hWav->header.head.non_pcm_fact.data_bytes / 2; // Number of bytes in fact. Number of samples * num_channels
			break;
		case WAV_NON_PCM_DATA_NOFACT:
			// SIZING
			hWav->header.head.non_pcm_nofact.wav_size   = file_size - 8; // Size of the wav portion of the file, which follows the first 8 bytes. File size - 8
			hWav->header.head.non_pcm_nofact.byte_rate  = hWav->header.head.non_pcm_nofact.sample_rate * hWav->header.head.non_pcm_nofact.num_channels * 2; // Number of bytes per second. sample_rate * num_channels * Bytes Per Sample
			hWav->header.head.non_pcm_nofact.data_bytes = (file_size - hWav->header.headerSize); // Number of bytes in data. Number of samples * num_channels * sample byte size
			break;
		default:
			break;
		}

		// update header
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		writtenByte = (uint32_t)fwrite(hWav->header.head.buffer, 1, hWav->header.headerSize, hWav->file);
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		writtenByte = (uint32_t)hWav->file.write(hWav->header.head.buffer, hWav->header.headerSize);
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		size_tbytLeft = 0;
		f_write(&hWav->SDFile, hWav->header.head.buffer, (UINT)hWav->header.headerSize, &bytLeft);
		HAL_Delay(10);
		writtenByte = (uint32_t)bytLeft;
		#endif //WAV_DEVICE
		if(writtenByte != hWav->header.headerSize){
			wav_printf("must be wrote %d bytes for header but just written %d bytes!\n", writtenByte, hWav->header.headerSize);
		}

		// close file
		#if    WAV_DEVICE == 0 //WAV_DEVICE_PC_CPP
		fclose(hWav->file);
		#elif  WAV_DEVICE == 1 //WAV_DEVICE_SIPEED_CPP
		hWav->file.close();
		#elif  WAV_DEVICE == 2 //WAV_DEVICE_STM32
		f_close(&hWav->SDFile);
		#endif //WAV_DEVICE

		// empty buffer
		memset(hWav->fileName, 0, FILENAME_MAX_LEN * sizeof(char));
		memset(&(hWav->header), 0, sizeof(wav_header_t));
		hWav->RorW = (wav_handle_mode_t)0;
		break;
	default:
		break;
	}
	
}
