/*
 *Copyright(c) 2014 xdrm All rights reserved
 *
 */

 /**
  *@file XdrmClient.h
  *@brief Provide API for xDRM Client SDK
  *@author zhengbo cai
  *@version 1.0
  *@date 2014.08.06
  *
  */

/**
 *@defgroup drm DRM
 *@brief DRM Client APIs description
 *section overview
 *DRM will offers APIs for xdrm
 *
 */

/**
 *@addtogroup xdrm
 *@{
 */
#ifndef __XDRM_CLIENT_H__
#define  __XDRM_CLIENT_H__

#include "xDrmErrorCode.h"

#ifdef __cplusplus
extern "C"{
#endif

#define UTC_STRING 22 //!yyyy/mm/dd [hh:mm:ss]
/** typedef new data class*/
typedef unsigned char  	uint8_t;
typedef unsigned int	  	uint32_t;

/** define the drm contex*/
typedef struct
{
	void* pBuf;
}innerImp;

typedef struct contex
{
#ifdef __LP64__
	long hdrm;
#else
	int hdrm;
#endif
	innerImp imp;
}drmcontex;

typedef enum
{
	CUSTOM_DATA_TYPE_FOR_DOMAIN_JOIN = 0,
	CUSTOM_DATA_TYPE_FOR_LIC_ACQUIRE,
	CUSTOM_DATA_TYPE_END,
}custome_data_type;

typedef struct 
{
	uint32_t size;
}myss_info;

 /**
  *@brief 	Init a context and open a client drm instance 
  *@param	eCtx			[in]the xdrm drm context
  *@param	ctrl_info		[in]contained a control info
  *@return 
  *\n		Result_SUCCESS				if success return 
  *\n		Result_Error_OpenFailed			if failed return 
  */
int
xDrm_Init(drmcontex* eCtx, char* ctrl_info); 

int
xDrm_Init_ex(drmcontex* eCtx, int mode);

int 
xDrm_InitWithMode(drmcontex* eCtx, char* ctrl_info, int mode, char* msg);

 /**
  *@brief 	open a client drm instance
  *@param	eCtx			[in]the xdrm drm context
  *@param	mac			[in]device mac address, if NULL will gen it in inner
  *@return 
  *\n		Result_SUCCESS				if success return 
  *\n		Result_Error_OpenFailed			if failed return 
  */
int xDrm_Open(drmcontex* eCtx, char* mac);

/**
 *@brief 		close the opened drm instance
 *@return 	Result_SUCCESS			if success 
 *\n			Result_Error_CloseFailed	if failed     
 */
int xDrm_Close(drmcontex* eCtx);


/**
*@brief		 Set Pack File which will be used in the current DRM Context.
*@param	 eCtx			 [in]the xdrm drm context
*@param	 packfile			 [in]file name path of pack file being used
*@return	 Result_SUCCESS 				 		if success 
*\n			 Result_Error_GetDrmHeaderFailed	if failed	   
*/
int xDrm_SetPackFile(drmcontex* eCtx, const char* packfile);

/*
 * For test case
*/
int
xDrm_SetStorePath(drmcontex* eCtx, const char* spath);
/**
*@brief      	get epub size. 
			if subItemURL != NULL, it will return subItemURL file size. 
			if subItemURL == NULL, it will return whole epub size.
*@param  	eCtx            [in]the xdrm drm context
*@param  	inFLname      [in]file name path of epub.
*@param     	subItemURL   [in] the subItem name
*@return     	Result_SUCCESS                         if success 
*\n          	Result_Error_GetEpubSizeFailed if failed      
*/
int xDrm_GetEpubSize(drmcontex* eCtx, const char* inFLname, const char* subItemURL);

/**
*@brief		 Set Custom Data from xDRM UserApp to xDRM Client Module. (the orignal data is from Business Server)
                    xDRM Client will forward this data to XDRM Server for further communicator with Business Server I/F
                    Refer 4.BizServer_Interface.doc
*@param	 eCtx			[in]the xdrm drm context
*@param	 eDataType		[in]custom data type (2222 - License server auth; 1111 - Domain server auth; etc)
*@param	 pDataBuffer		[in]custom data (Protocol Data Unit FORMAT was mainly decided by business server)
*@param  nDataLen		[in]custom data length
*@return	 Result_SUCCESS 				 		if success 
*\n			 Result_Error_GetDrmHeaderFailed	if failed	   
*/
int xDrm_SetCustomData(drmcontex* eCtx, int eDataType, uint8_t* pDataBuffer, uint32_t nDataLen);

/**
*@brief		 Get drm header information of Pack File of current drm context.
                    DRM Header here is used for GenLic_Chanllenge.
*@param	 eCtx			 [in]the xdrm drm context
*@param	 ppHeader		 [out]Point to drm header info buffer
*@param	 nLenHeader		 [out]the drm header info buffer length
*@return	 Result_SUCCESS 				 		if success 
*\n			 Result_Error_GetDrmHeaderFailed	if failed	   
*/
int xDrm_GetDrmHeader(drmcontex* eCtx,  void** ppHeader, uint32_t* nLenHeader);

 /**
  *@brief 	Generate license chanllenge and get response from server
  *@param	eCtx			[in]the xdrm drm context
  *@param 	pdrmHeader	[in] point to drm header info
  *@param	headerLen	[in] pdrmHeader info length
  *@param	ppLicResponse[out] server chanllege response
  *@param	rspLen		[out] server response length
  *@return 
  *\n		Result_SUCCESS				if success return 
  *\n		Result_Error_GenChanllegeFailed	if failed return 
  */
int xDrm_GenLic_Chanllenge(drmcontex* eCtx, uint8_t* pdrmHeader, uint32_t headerLen, void** ppLicResponse, uint32_t* rspLen);

 
 /**
  *@brief 	install license from output data generate by xDrm_GenLic_Chanllenge
  *@param	eCtx			[in]the xdrm drm context
  *@param 	pLicData		[in] point to buffer of license response data
  *@param	nLicDataLen	[in] the length of lecense response data
  *@return 
  *\n		Result_SUCCESS				if success return 
  *\n		Result_Error_InstallLicFailed		if failed return 
  */
int xDrm_InstallLicense_ByRespData(drmcontex* eCtx, void* pLicData, uint32_t nLicDataLen);

/**
  *@brief 	install license that generate by xDrm_RepacketContentfile
  *@param	eCtx			[in]the xdrm drm context
  *@param 	licdata		[in] license data passed by upper layer
  *@param	len			[in] license data length 
  *@return 
  *\n		Result_SUCCESS				if success return 
  *\n		Result_Error_InstallLicFailed		if failed return 
  */
int xDrm_InstallLicense_ByLicFile(drmcontex* eCtx, const char* licdata, uint32_t len);

/**
  *@brief 	Install license directly from current ZIP pack file which are set by xDrm_SetPackFile.
  *               normally the packed ZIP file license was re-packed by xDrm_RepacketContentfile.
  *@param	eCtx			[in]the xdrm drm context
  *@return 
  *\n		Result_SUCCESS				if success return 
  *\n		Result_Error_InstallLicFailed		if failed return 
  */
int xDrm_InstallLicense_CurrPack(drmcontex* eCtx);


/**
 *@brief 		Get and install a license from URL
 *@param	eCtx					[in]the xdrm drm context
 *@param	pLicWebinitiatorUrl		[in]point to license url
 *@return 	Result_SUCCESS				if success 
 *\n			Result_Error_GetLicenseUrlFailed	if failed     
 */
int xDrm_GetLic_WebinitiatorUrl(drmcontex* eCtx, const char* pLicWebinitiatorUrl);

/**
 *@brief 		Get and install a license from XML
 *@param	eCtx						[in]the xdrm drm context
 *@param	pLicWebinitiatorXml		[in]point to license xml
 *@param	uLen						[in]the length of pLicXml
 *@return 	Result_SUCCESS					if success 
 *\n			Result_Error_GetLicenseXmllFailed 	if failed     
 */
int xDrm_GetLic_WebinitiatorXml(drmcontex* eCtx, const char* pLicWebinitiatorXml, uint32_t uLen);

#if 0
//!有啥用? 如何删除buffer
/**
 *@brief 		delete a  specify license data
 *@param	eCtx			[in]the xdrm drm context
 *@param	pmsg		[in]include the necessary message data to be delete
 *@param	umsglen		[in]pmsg data length
 *@return 	Result_SUCCESS				if success 
 *\n			Result_Error_DeleteLicFailed		if failed     
 */
int xDrm_DeleteLicense(drmcontex* eCtx, void* pmsg, uint32_t umsglen);
#endif

/**
 *@brief 		Clean all license which have installed
 *@param	eCtx			[in]the xdrm drm context
 *@return 	Result_SUCCESS				if success 
 *\n			Result_Error_CleanAllLicFailed	if failed     
 */
int xDrm_CleanAllLicense(drmcontex* eCtx);

/**
 *@brief 		Clean the invalid license which have installed
 *@param	eCtx			[in]the xdrm drm context
 *@return 	Result_SUCCESS					if success 
 *\n			Result_Error_CleanInvalidLicFailed		if failed     
 */
int xDrm_CleanInvalidLicense(drmcontex* eCtx);

/**
 *@brief 		Get the current context  drm rights information
 *@param	eCtx					[in]the xdrm drm context
 *@param	rightsInfo				[out]Contain drm rights info
 *@return 	Result_SUCCESS					if success 
 *\n			Result_Error_GetRightsInfoFailed		if failed     
 */
int xDrm_GetRightsInfo(drmcontex* eCtx,  char* rightsInfo, uint32_t* len);

/**
 *@brief 		Get license from the packfile which include license
 *@param	eCtx					[in]the xdrm drm context
 *@return 	Result_SUCCESS						if success 
 *\n			Result_Error_GetLic_ByCurPackFileFailed	if failed     
 */
int xDrm_GetLic_ByCurrentPackFile(drmcontex* eCtx);

/**
 *@brief 		A instance will join a specify domain
 *@param	eCtx				[in]the xdrm drm context
 *@param	pcertserver		[in]certification server info
 *@param	pserviceId		[in]service Id info
 *@param	paccountId		[in]account Id info
 *@return 	Result_SUCCESS					if success 
 *\n			Result_Error_JoinDomainFailed		if failed     
 */
int xDrm_JoinDomain(drmcontex* eCtx, const char* pcertserver, const char* pserviceId, const char* paccountId);

/**
 *@brief 		A instance will leave a specify domain
 *@param	eCtx				[in]the xdrm drm context
 *@param	pcertserver		[in]certification server info
 *@param	pserviceId		[in]service Id info
 *@param	paccountId		[in]account Id info
 *@return 	Result_SUCCESS					if success 
 *\n			Result_Error_LeaveDomainFailed		if failed     
 */
int xDrm_LeaveDomain(drmcontex* eCtx, const char* pcertserver, const char* pserviceId, const char* paccountId);

/**
  *@brief 	Decrypt full content of one sub content file
  *@param	eCtx			[in]the xdrm drm context
  *@param 	subItemURL	[in] the URL(file name path) sub content item resource inside the epub file
  *@param	outBuff		[out] decrpted raw data of related encrypted content item.
  *@param	outLen		[in/out] decrypted raw data length. 
  *                                              (while call in, the variable keeps the length of MAX outLen)
  *@return 
  *\n		Result_SUCCESS						if success return 
  *\n		Result_Error_DecryptSubContentFailed		if failed return 
  */
int xDrm_DecryptFullObj(drmcontex* eCtx, const char* subItemURL, uint8_t* outBuff, uint32_t* outLen);

/**
  *@brief 	Decrypt partial content of sub content file
  *@param	eCtx			[in]the xdrm drm context
  *@param 	subItemURL	[in] the URL(file name path) sub content item resource inside the epub file
  *@param	nOffset		[in] content offset for locating start point inside the URL file.
  *@param	nSize		[in] content length expect to be processed
  *@param	outBuff		[out] decrpted raw data of related encrypted content item.
  *@param	outLen		[in/out] decrypted raw data length. 
  *                                              (while call in, the variable keeps the length of MAX outLen)
  *@return 
  *\n		Result_SUCCESS						if success return 
  *\n		Result_Error_DecryptSubContentFailed		if failed return 
  */
int xDrm_DecryptPartialObj(drmcontex* eCtx, const char* subItemURL, uint32_t nOffset, uint32_t nSize, uint8_t* outBuff, uint32_t* outLen);

/**
  *@brief		 Decrypt the encryptd data of global area.
  *                (This API is only for decrypt data which are no in normally sub content of ePub
                     Currently maybe no obvious usage, reserved for further extention.)
  *@param	 eCtx		[in]the xdrm drm context
  *@param	 input		[in]Point to input encrypted data
  *@param	 inLen		[in]input data lenth
  *@param	 output 	 	[out]Point to out decrypt buffer
  *@param	 outLen 		[out]output data length
  *@return	 Result_SUCCESS 				 if success 
  *\n			 Result_Error_DecryptFailed 		 if failed	   
  */
 int xDrm_DecryptGlobal(drmcontex* eCtx, const char* input, uint32_t inLen, char* output, uint32_t* outLen);

/**
  *@brief		 Decrypt the whole epub content.
  *                
  *@param	 eCtx		[in]the xdrm drm context
  *@param	 outpath		[in]Point to Securestorage path
  *@param	 outLen		[inout]Securestorage path length
  *@param	 output 	 	[out]Point to out decrypt buffer
  *@param	 outLen 		[out]output data length
  *@return	 Result_SUCCESS 				 if success 
  *\n			 Result_Error_DecryptFailed 		 if failed	   
  */
int xDrm_DecryptWholeEpub(drmcontex* eCtx, uint8_t* outpath, uint32_t* outLen);

 /**
  *@brief 	Repacket content file with license info
  *@param	eCtx			[in]the xdrm drm context
  *@param 	inFLname		[in] content file name
  *@param	infLen		[in] content file name len
  *@param	licensefile	[in] license file name
  *@param	licLen		[in] license file name len
  *@param 	oFLname		[in] out zip file name
  *@param	ofLen		[in] out zip file name len
  *@return 
  *\n		Result_SUCCESS						if success return 
  *\n		Result_Error_RepacketContentfileFailed	if failed return 
  */
int xDrm_RepacketContentfile(drmcontex* eCtx, const char* inFLname, uint32_t infLen, uint8_t* licensefile, uint32_t licLen);

 /**
  *@brief 	delete the memory alloced by drm_client API
  *@param 	pMem		[in] memory address
  *@return 
  *\n		Result_SUCCESS						if success return 
  */
int xDrm_DelMem(char* pMem);

 /**
  *@brief 	Judge the specified epub book is encrypted or not
  *@param	eCtx			[in]the xdrm drm context
  *@param 	name		[in] epub book name
  *@return 
  *\n		true						if encrypted return 
  */
bool xDrm_IsEpubEncrypted(drmcontex* eCtx, const char* name);

bool xDrm_IsItemEncrypted(drmcontex* eCtx, const char* epubname, const char* subItmes);



int xDrm_SS_GetInfo(drmcontex* eCtx, char* filepath, myss_info* ssinfo, int ssflag, char* group);

int xDrm_SS_Read(drmcontex* eCtx, char* filepath, char* obuf, uint32_t offset, uint32_t wantsize, uint32_t* rdsize, int ssflag, char* group);

int xDrm_SS_Del(drmcontex* eCtx, char* filepath, int ssflag, char* group);

/**
*@brief      	get client error message explain. 
*@param     	retcode   		[in] retcode value
*@return     	const string value
*/
char* xDrm_GetErrorMessage(int retcode);


int xDrm_DecryptCommonFile(drmcontex* eCtx, uint8_t* outBuff, uint32_t wantLen, uint32_t offset, uint32_t* outlen);

int xDrm_GetCommonFileSize(drmcontex* eCtx, const char* inFLname);

int xDrm_GetFileType(drmcontex* eCtx, char* name);

int xDrm_RepacketCommonFile(drmcontex * eCtx, char * licdata, uint32_t len);

bool xDrm_IsCommonFileEncrypted(char* filename);
    
bool xDrm_IsBookEncrypted(char* filename);

int xDrm_GetEpubContentID(drmcontex * eCtx, char *filepath, char * outbuffer, uint32_t *olen);

int xDrm_GetBookContentID(drmcontex * eCtx, char *filepath, char * outbuffer, uint32_t *olen);

int xDrm_GetCurrent_BookInfo(drmcontex * eCtx, char* buffer, unsigned int* len);

int xDrm_UploadUserData(drmcontex * eCtx, char* userdata);

int xDrm_InstallLicense_RenewData(drmcontex * eCtx, char* renewData, uint32_t inlen);

int xDrm_OnTimeEvent(drmcontex * eCtx);

int xDrm_DelLicFileTotally(drmcontex * eCtx, int type, char* bookpath, char* account);

int xDrm_DelLicFileALL(drmcontex *eCtx);

bool xDrm_IsBookNeedRerent(int type, char* bookpath, char* account);

int xDrm_Offline_GetKEK(drmcontex * eCtx, char* kek, uint32_t* keklen);
int xDrm_Offline_SetCEK(drmcontex * eCtx, char* cid, char* enc_cek, uint32_t ceklen);

int xDrm_GetSaveBookDetails(char* bookpath, int type);

int xDrm_ReturnCurLicense(drmcontex * eCtx);

int xDrm_RenewCurLicense(drmcontex * eCtx);

int xDrm_RerentCurLicense(drmcontex * eCtx);

#ifdef __cplusplus
};
#endif

 /**
  *@}
  */
#endif //!__XDRM_CLIENT_H__

