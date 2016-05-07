#include <stdio.h>

char		mupdf_one_time_print_buffer[10240];
int		mupdf_g_client_dbglvl = 0;
FILE*	mupdf_fp_log = NULL;

int app_print_open(char* logpath, int level)
{
	if(mupdf_fp_log)
	{
		return 0;
	}
       mupdf_fp_log = fopen(logpath, "a+");
	if(!mupdf_fp_log)
	{
		return -1;
	}
	
	mupdf_g_client_dbglvl = level;
	return 0;
}

void app_print_close()
{
	if(!mupdf_fp_log)
		return;

	fclose(mupdf_fp_log);
	mupdf_fp_log = NULL;
}
void app_print_log(char* logbuffer)
{
	fprintf(mupdf_fp_log, "%s", logbuffer);
	fflush(mupdf_fp_log);
}
