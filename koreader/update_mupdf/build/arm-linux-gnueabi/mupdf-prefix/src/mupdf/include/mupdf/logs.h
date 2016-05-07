#ifndef __LOG_H_
#define __LOG_H_

#define DEBUG_ON

#define  	PDF_LOG_TAG    "muPdf"
#ifdef DEBUG_ON
#if defined(_WIN32) || defined(_WIN64)	
	int app_print_open(char* logpath, int level);
	void app_print_close();
	void app_print_log(char* logbuffer);

	extern char	mupdf_one_time_print_buffer[10240];
	extern int	       mupdf_g_client_dbglvl;

	#define 	DEBUG_LEVEL_NON	0
	#define	DEBUG_LEVEL_ERR	1
	#define	DEBUG_LEVEL_LOG	2

	#define	CLIENT_ERR(title, fmt, ...) 	do{ sprintf(mupdf_one_time_print_buffer, "[%s][ERR]" fmt, title, __VA_ARGS__); app_print_log(mupdf_one_time_print_buffer);}while(0);
	#define	CLIENT_LOG(title, fmt, ...) 	do{ sprintf(mupdf_one_time_print_buffer, "[%s][INFO]" fmt, title, __VA_ARGS__); app_print_log(mupdf_one_time_print_buffer);}while(0);

	#define  PDF_LOGI(fmt, ...)		do{	if(mupdf_g_client_dbglvl >= DEBUG_LEVEL_ERR ){CLIENT_LOG(PDF_LOG_TAG, fmt, __VA_ARGS__)};}while(0);
	#define  PDF_LOGE(fmt, ...)		do{	if(mupdf_g_client_dbglvl >= DEBUG_LEVEL_LOG ){CLIENT_ERR(PDF_LOG_TAG, fmt, __VA_ARGS__)};}while(0);
#elif defined(__ANDROID__)
	#include <android/log.h>
	//#define LOG_TAG "libmupdf"
	#define PDF_LOGI(...) __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
	#define PDF_LOGE(...) __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else //iOS
	#define  PDF_LOGI                         printf
	#define  PDF_LOGE               		printf
#endif

#else
	#define  PDF_LOGI(x) while(0);
	#define  PDF_LOGE(x) while(0);
#endif //if DEBUG_ON

#endif
