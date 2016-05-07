#include "mupdf/fitz.h"


#if defined(_WIN32) || defined(_WIN64)
#include <time.h>
#include <windows.h>
#ifdef ENABLE_XDRM
#pragma comment(lib,"xdrm_client.lib")
#endif
#endif
#include <stdbool.h>
#include "xDrmClient.h"
#include "xDrmErrorCode.h"

#define READ_MAX_CACHE_SIZE				(128*1024)
#define READ_MAX_HALF_CACHE_SIZE			(READ_MAX_CACHE_SIZE>>2)

fz_stream *
fz_new_stream(fz_context *ctx, void *state, fz_stream_next_fn *next, fz_stream_close_fn *close)
{
	fz_stream *stm;

	fz_try(ctx)
	{
		stm = fz_malloc_struct(ctx, fz_stream);
	}
	fz_catch(ctx)
	{
		close(ctx, state);
		fz_rethrow(ctx);
	}

	stm->refs = 1;
	stm->error = 0;
	stm->eof = 0;
	stm->pos = 0;

	stm->bits = 0;
	stm->avail = 0;

	stm->rp = NULL;
	stm->wp = NULL;

	stm->state = state;
	stm->next = next;
	stm->close = close;
	stm->seek = NULL;

	return stm;
}

fz_stream *
fz_keep_stream(fz_context *ctx, fz_stream *stm)
{
	if (stm)
		stm->refs ++;
	return stm;
}

void
fz_drop_stream(fz_context *ctx, fz_stream *stm)
{
	if (!stm)
		return;
	stm->refs --;
	if (stm->refs == 0)
	{
		if (stm->close)
			stm->close(ctx, stm->state);
		fz_free(ctx, stm);
	}
}

/* File stream */

typedef struct fz_file_stream_s
{
	DRM_PTR 		file;
	bool			encrypted;
	unsigned int 	filelen;
	unsigned int 	fileoffset;
	unsigned char buffer[4096];

	//descript total cache buffer info
	unsigned char* cache;
	unsigned int 	cachelen;
	unsigned int 	recordfilepos;

	//descript left cache buffer info
	unsigned char* left_cache;
	unsigned int	left_cachelen;

	//descript right cache buffer info
	unsigned char* right_cache;
	unsigned int	right_cachelen;
} fz_file_stream;

static int next_file(fz_context *ctx, fz_stream *stm, int n)
{
	fz_file_stream *state = stm->state;
	int ret = -1;
	int copylen = 0;
	int distance = 0;
	unsigned int assist_copylen = 0;
	unsigned int assist_copyoffset = 0;
	unsigned int cacheoffset = 0;
	unsigned int preoffset = 0;

	/* n is only a hint, that we can safely ignore */
	if(!state->encrypted)
	{
		n = fread(state->buffer, 1, sizeof(state->buffer), (FILE*)state->file);
	}
	else
	{
#ifdef ENABLE_XDRM
#if defined(__ANDROID__)
//	struct timeval stime,etime;
//	long		 	dist = 0;
//	gettimeofday(&stime, NULL);
#endif
		if(sizeof(state->buffer)  < READ_MAX_CACHE_SIZE)
		{
			copylen = sizeof(state->buffer) > (state->filelen - state->fileoffset) ? (state->filelen - state->fileoffset) : sizeof(state->buffer);
			PDF_LOGI("fileoffset[%d], recordfilepos[%d], copylen[%d], cachelen[%d], right_cachelen[%d], left_cachelen[%d]\n", 
														state->fileoffset, state->recordfilepos, copylen, state->cachelen, state->right_cachelen, state->left_cachelen);
			//if(!((state->fileoffset >= state->recordfilepos) && (state->fileoffset + copylen <= state->recordfilepos + state->cachelen)))
			if( ( (state->fileoffset >= state->recordfilepos) && (state->fileoffset + copylen > state->recordfilepos + state->right_cachelen) ) ||
				( (state->fileoffset < state->recordfilepos) && (state->recordfilepos - state->fileoffset > state->left_cachelen))  ||
				( (state->fileoffset < state->recordfilepos) && (state->recordfilepos - state->fileoffset < state->left_cachelen) && (state->fileoffset + copylen > state->recordfilepos + state->right_cachelen)))
			{
				distance = state->fileoffset - state->recordfilepos;
				//PDF_LOGI("[%s]****L=%d*****\n", __FUNCTION__, __LINE__);
				
				state->cachelen = READ_MAX_CACHE_SIZE > (state->filelen - state->fileoffset) ? (state->filelen - state->fileoffset) : READ_MAX_CACHE_SIZE;
				//distance = state->fileoffset > state->recordfilepos ? (state->fileoffset - state->recordfilepos) : (state->recordfilepos - state->fileoffset);
				//distance = state->fileoffset - state->recordfilepos;
				//PDF_LOGI("[czb]copylen = %d, cachelen = %d, offset_distance = %d\n", copylen, state->cachelen,distance);

				if(copylen >= (state->cachelen >> 1))
				{
					preoffset = 0;
				}
				else
				{
					assist_copylen = state->cachelen >> 1;
					preoffset = (state->fileoffset <= assist_copylen) ? state->fileoffset : assist_copylen;
				}
				ret = xDrm_DecryptCommonFile((drmcontex*)state->file, state->cache, state->cachelen, state->fileoffset - preoffset, &n);
				if(ret)
				{
					n = -1;
					PDF_LOGE("[%s]Call xDrm_DecryptCommonFile failed!!\n", __FUNCTION__);
				}
				else
				{
					state->right_cache = state->cache + preoffset;
					state->right_cachelen = state->cachelen - preoffset;
					state->left_cache = state->cache;
					state->left_cachelen = state->cachelen - state->right_cachelen;

					state->recordfilepos = state->fileoffset;

					//PDF_LOGI("[czb]left_cachelen[%d], right_cachelen[%d], recordfilepos[%d]\n", state->left_cachelen, state->right_cachelen, state->recordfilepos);
					PDF_LOGI("[%s]Call xDrm_DecryptCommonFile success!!read len = %d\n", __FUNCTION__, n);
				}
			}

			if(((state->fileoffset >= state->recordfilepos) && (state->fileoffset + copylen <= state->recordfilepos + state->right_cachelen)))
			{
				//PDF_LOGI("[%s]****L=%d*****\n", __FUNCTION__, __LINE__);
				//PDF_LOGI("[czb]copy all from right cache, fileoffset = %d, copylen = %d\n", state->fileoffset, copylen);
				cacheoffset = state->fileoffset - state->recordfilepos;
				memcpy(state->buffer, state->right_cache+cacheoffset, copylen);	
				n = copylen;
			}
			else if( (state->fileoffset < state->recordfilepos) && (state->recordfilepos - state->fileoffset <= state->left_cachelen) )
			{
				assist_copylen = copylen > (state->recordfilepos - state->fileoffset) ? (state->recordfilepos - state->fileoffset) : copylen;
				if(assist_copylen)
				{
					assist_copyoffset = state->left_cachelen - (state->recordfilepos - state->fileoffset);
					//PDF_LOGI("[czb]copy part data from left cache, fileoffset = %d, copylen_left = %d\n", state->fileoffset, assist_copyoffset);
					memcpy(state->buffer, state->left_cache+assist_copyoffset, assist_copylen);
				}

				if(copylen - assist_copylen)
				{
					//PDF_LOGI("[czb]copy part data from right cache, fileoffset = %d, copylen_rgt = %d\n", state->fileoffset, copylen - assist_copylen);
					memcpy(state->buffer + assist_copylen, state->right_cache, copylen - assist_copylen);
				}

				n = copylen;
			}
			else
			{
				n = -1;
				PDF_LOGE("[%s]Get data from cache failed!!\n", __FUNCTION__);
			}
		}
		else
		{
			PDF_LOGI("[czb]No Cache case, fileoffset = %d\n", state->fileoffset);
			ret = xDrm_DecryptCommonFile((drmcontex*)state->file, state->buffer, sizeof(state->buffer), state->fileoffset, &n);		
			if(ret)
			{
				n = -1;		
			}
		}
#if defined(__ANDROID__)
//		gettimeofday(&etime, NULL);
//		dist = etime.tv_usec - stime.tv_usec;
//		dist += (etime.tv_sec - stime.tv_sec)*1000000;
//		PDF_LOGI("====>the time span is %ld\n", dist);
#endif
		//PDF_LOGI("call decrypt: wantlen = %d, readlen = %d\n", sizeof(state->buffer), n);
#endif
	}
	if (n < 0)
		fz_throw(ctx, FZ_ERROR_GENERIC, "read error: %s", strerror(errno));
	stm->rp = state->buffer;
	stm->wp = state->buffer + n;
	stm->pos += n;
	state->fileoffset += n;
	if (n == 0)
		return EOF;
	return *stm->rp++;
}

static void seek_file(fz_context *ctx, fz_stream *stm, fz_off_t offset, int whence)
{
	fz_file_stream *state = stm->state;
	if(!state->encrypted)
	{
		fz_off_t n = fz_fseek((FILE*)(state->file), offset, whence);
		if (n < 0)
			fz_throw(ctx, FZ_ERROR_GENERIC, "cannot seek: %s", strerror(errno));
		stm->pos = fz_ftell((FILE*)(state->file));;
		stm->rp = state->buffer;
		stm->wp = state->buffer;
	}
	else
	{
#ifdef ENABLE_XDRM
		int curoffset = state->fileoffset;
		int totallen = state->filelen;
		/* Convert to absolute pos */
		if (whence == 1)
		{
			offset += curoffset; /* Was relative to current pos */
		}
		else if (whence == 2)
		{
			offset += totallen; /* Was relative to end */
		}

		if (offset < 0)
			offset = 0;
		if (offset > totallen)
			offset = totallen;

		state->fileoffset = offset;

		stm->pos = offset;
		stm->rp = state->buffer;
		stm->wp = state->buffer;
#endif
	}

	PDF_LOGI("[%s]===>seek to pos:%d\n", __FUNCTION__, stm->pos);
}

static void close_file(fz_context *ctx, void *state_)
{
	fz_file_stream *state = state_;
	int n = -1;

       PDF_LOGI("--->In close file, state->encrypted = %d\n", state->encrypted);
	if(!state->encrypted)
	{
		n = fclose((FILE*)(state->file));
		PDF_LOGI("[%s]close clr file succe!!\n", __FUNCTION__);
	}
	else
	{
#ifdef ENABLE_XDRM
		if(state->file){free((drmcontex*)state->file); state->file = 0;}
		if(state->cache){free(state->cache); state->cache = NULL;state->cachelen = 0;}
		n = 1;
		PDF_LOGI("[%s]close enc file succe!!\n", __FUNCTION__);
#endif
	}
	if (n < 0)
		fz_warn(ctx, "close error: %s", strerror(errno));
	fz_free(ctx, state);
}

fz_stream *
fz_open_file_ptr(fz_context *ctx, DRM_PTR file, char* filename)
{
	fz_stream *stm;
	fz_file_stream *state = fz_malloc_struct(ctx, fz_file_stream);
	state->file = file;
#ifdef ENABLE_XDRM
	state->filelen = xDrm_GetCommonFileSize((drmcontex *)file, filename);
	state->encrypted = xDrm_IsCommonFileEncrypted(filename);
	PDF_LOGI("[%s]==>file len is %d\n", __FUNCTION__, state->filelen);
#else
	state->filelen = 0;
	state->encrypted = false;
#endif
	//state->encrypted = false;
	state->fileoffset = 0;
	if(state->encrypted)
	{
		if(state->filelen < READ_MAX_CACHE_SIZE)
		{
			
			state->cache = (unsigned char*)malloc(state->filelen);

			
			state->cachelen = 0;
			state->recordfilepos = 0;

			state->left_cache = state->cache;
			state->left_cachelen = 0;
			state->right_cache = state->cache + (unsigned int)(READ_MAX_CACHE_SIZE>>1);
			state->right_cachelen = 0;
		}
		else
		{
			state->cache = (unsigned char*)malloc(READ_MAX_CACHE_SIZE);
			state->cachelen = 0;
			state->recordfilepos = 0;
			
			state->left_cache = state->cache;
			state->left_cachelen = 0;
			state->right_cache = state->cache + (unsigned int)(READ_MAX_CACHE_SIZE>>1);
			state->right_cachelen = 0;

		}
		if(!state->cache)
		{
			PDF_LOGI("[%s] malloc cache buffer(size=%d) failed!!!\n", __FUNCTION__, READ_MAX_CACHE_SIZE);
			fz_throw(ctx, FZ_ERROR_GENERIC, "malloc cache buffer error: size %d", READ_MAX_CACHE_SIZE);
		}
	}

	fz_try(ctx)
	{
	       //PDF_LOGI("[%s]~~will register next file & close file function to ctx, addr = 0x%x\n",__FUNCTION__, (long)close_file);
		stm = fz_new_stream(ctx, state, next_file, close_file);
	}
	fz_catch(ctx)
	{
		fz_free(ctx, state);
		fz_rethrow(ctx);
	}
	stm->seek = seek_file;

	return stm;
}
#define MODE_HAVE_GET_LICENSED
#ifdef  MODE_HAVE_GET_LICENSED
fz_stream *
fz_open_file(fz_context *ctx, const char *name)
{
    	FILE*		f = NULL;
	DRM_PTR 	fd = 0;
	int 			ret = 0;
#ifdef ENABLE_XDRM
	bool encflag = xDrm_IsCommonFileEncrypted((char*)name);
	drmcontex* handle = NULL;
#else
	bool encflag = false;
#endif	
	PDF_LOGI("[%s]file path is %s, encflag=%d\n", __FUNCTION__, name, encflag);
	if(!encflag)
	{
		PDF_LOGI("------->This is clr pdf\n");
	#if defined(_WIN32) || defined(_WIN64)
		f = fopen(name, "rb");
	#else
		f = fz_fopen(name, "rb");
	#endif
		fd = (DRM_PTR)f;
		PDF_LOGI("file name is %s, fd = %ld\n",name, fd);
	}
	else
	{
	#ifdef ENABLE_XDRM
        	PDF_LOGI("------->This is encrypted pdf\n");
		handle = malloc(sizeof(drmcontex));
		ret = xDrm_Init_ex(handle, 1);
              if(ret)
		{//failed case
			fd = -1;
                     goto out;
		}
		ret = xDrm_Open(handle, NULL);
		if(ret)
		{//failed case
			fd = -1;
                     goto out;
		}
		ret = xDrm_SetPackFile(handle, name);
		if(ret)
		{//failed case
			fd = -1;
                     goto out;
		}
		ret = xDrm_InstallLicense_CurrPack(handle);
		if(ret)
		{//failed case
			fd = -1;
                     goto out;
		}
		else
		{
			fd = (DRM_PTR)handle;
		}
	#endif
	}
out:
	if (fd == NULL || fd == -1)
		fz_throw(ctx, FZ_ERROR_GENERIC, "cannot open %s", name);
	return fz_open_file_ptr(ctx, fd, name);
}
#else
fz_stream *
fz_open_file(fz_context *ctx, const char *name)
{
	FILE *f;
#if defined(_WIN32) || defined(_WIN64)
	char *s = (char*)name;
	wchar_t *wname, *d;
	int c;
	d = wname = fz_malloc(ctx, (strlen(name)+1) * sizeof(wchar_t));
	while (*s) {
		s += fz_chartorune(&c, s);
		*d++ = c;
	}
	*d = 0;
	f = _wfopen(wname, L"rb");
	fz_free(ctx, wname);
#else
	f = fz_fopen(name, "rb");
#endif
	if (f == NULL)
		fz_throw(ctx, FZ_ERROR_GENERIC, "cannot open %s", name);
	return fz_open_file_ptr(ctx, f);
}
#endif

#if defined(_WIN32) || defined(_WIN64)
fz_stream *
fz_open_file_w(fz_context *ctx, const wchar_t *name)
{
	FILE *f = _wfopen(name, L"rb");
	if (f == NULL)
		fz_throw(ctx, FZ_ERROR_GENERIC, "cannot open file %ls", name);
	return fz_open_file_ptr(ctx, f, name);
}
#endif

/* Memory stream */

static int next_buffer(fz_context *ctx, fz_stream *stm, int max)
{
	return EOF;
}

static void seek_buffer(fz_context *ctx, fz_stream *stm, fz_off_t offset, int whence)
{
	fz_off_t pos = stm->pos - (stm->wp - stm->rp);
	/* Convert to absolute pos */
	if (whence == 1)
	{
		offset += pos; /* Was relative to current pos */
	}
	else if (whence == 2)
	{
		offset += stm->pos; /* Was relative to end */
	}

	if (offset < 0)
		offset = 0;
	if (offset > stm->pos)
		offset = stm->pos;
	stm->rp += (int)(offset - pos);
}

static void close_buffer(fz_context *ctx, void *state_)
{
	fz_buffer *state = (fz_buffer *)state_;
	if (state)
		fz_drop_buffer(ctx, state);
}

fz_stream *
fz_open_buffer(fz_context *ctx, fz_buffer *buf)
{
	fz_stream *stm;

	fz_keep_buffer(ctx, buf);
	stm = fz_new_stream(ctx, buf, next_buffer, close_buffer);
	stm->seek = seek_buffer;

	stm->rp = buf->data;
	stm->wp = buf->data + buf->len;

	stm->pos = buf->len;

	return stm;
}

fz_stream *
fz_open_memory(fz_context *ctx, unsigned char *data, int len)
{
	fz_stream *stm;

	stm = fz_new_stream(ctx, NULL, next_buffer, close_buffer);
	stm->seek = seek_buffer;

	stm->rp = data;
	stm->wp = data + len;

	stm->pos = len;

	return stm;
}
