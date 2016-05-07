#ifndef __XDRM_ERROR_CODE_H__
#define __XDRM_ERROR_CODE_H__

#ifdef __cplusplus
extern "C"{
#endif

/** @brief Error code enum define*/
typedef enum{
	Result_SUCCESS=0,						/**< sucess*/
	Result_Error_Unkown=-500,					/**<Unkown error*/
	Result_Error_NULL_Point,					/**<Null data pointer,*/
	Result_Error_InvalidParam,				/**< InvalidParam*/
	Result_Error_InvalidContext,				/**< InvalidContext*/
	Result_Error_OpenFileFailed,				/**< OpenFileFailed*/
	Result_Error_OpenFailed,					/**< OpenFailed*/
	Result_Error_CloseFailed,					/**< CloseFailed*/
	Result_Error_GenChanllegeFailed,			/**< GenChanllege*/
	Result_Error_InstallLicByLicFileFailed,		/**< InstallLic ByLicFile Failed*/
	Result_Error_InstallLicByRespDataFailed,	/**< InstallLic ByRespData Failed*/
	Result_Error_InstallLicByCurrPackFailed,	/**< InstallLic ByCurrPack Failed*/
	Result_Error_GetDrmHeaderFailed,		/**< GetDrmHeader*/
	Result_Error_GetRightsInfoFailed,			/**< GetRightsInfo*/
	Result_Error_GetLicenseUrlFailed,			/**< GetLicenseUrl*/
	Result_Error_GetLicenseXmlFailed,		/**< GetLicenseXml*/
	Result_Error_GetLic_ByCurPackFileFailed,	/**< GetLicense ByCurPackFile*/
	Result_Error_DeleteLicFailed,				/**< DeleteLicFailed*/
	Result_Error_CleanAllLicFailed,			/**< CleanAllLicFailed*/
	Result_Error_CleanInvalidLicFailed,		/**< CleanInvalidLicFailed*/
	Result_Error_DecryptFailed,				/**< DecryptFailed*/
	Result_Error_JoinDomainFailed,			/**< JoinDomainFailed*/
	Result_Error_LeaveDomainFailed,			/**< LeaveDomainFailed*/
	Result_Error_DecryptSubContentFailed,	/**< DecryptSubContentFailed*/
	Result_Error_RepacketContentfileFailed,	/**< RepacketContentfileFailed*/
	Result_Error_ShortBuffer,					/**< Short Buffer error*/
	Result_Error_SetPacket,					/**< SetPacket error*/
	Result_Error_LicenceExpire,				/**< LicenceExpire error*/
	Result_Error_LicenseNotFind,
	Result_Error_NewMemFailed,
	Result_Error_GetDeviceUUID,
	Result_Error_GenRsaKeyPair,
	Result_Error_GetIP4Addr,
	Result_Error_GenDKMSReqFailed,
	Result_Error_ParseDKMSReqFailed,
	Result_Error_GetSrvInfoFailed,
	Result_Error_AEDecryptFailed,
	Result_Error_SEDecryptFailed,
	Result_Error_SSWriteFailed,
	Result_Error_SSReadFailed,
	Result_Error_ParseJson_ServerInfoFailed,
	Result_Error_Curl_Internet_Failed,
	Result_Error_Get_ContentID_Failed,
	Result_Error_Get_FileData_ByName,
	Result_Error_Gen_CertReq,
	Result_Error_Parse_Algorithm,
	Result_Error_Parse_Customdata,
	Result_Error_Get_FileSizeByName,
	Result_Error_CEK_Error,
	Result_Error_UnCompress_Failed,
	Result_Error_LicStore_Failed,
	Result_Error_GenLicReq_Failed,
	Result_Error_TimeSystem_Tampered,
	Result_Error_Com_ReadDrmInfoFailed,		/*read drm info from common type file failed*/
	Result_Error_Get_ClrFile_ByNameFailed,	/*read clear file from epub failed*/
	Result_Error_Parse_NewJsonFailed,
	Result_Error_Parse_OldJsonFailed,
	Result_Error_OverrideUserDataFailed,
	Result_Error_Upload_UserDataFailed,
	Result_Error_Renew_LicenseFailed,
	Result_Error_Parse_RenewDataFailed,
	Result_Error_Store_AuthcodeFailed,
	Result_Error_Get_AuthcodeFailed,
	Result_Error_Get_LibID,
	Result_Error_Parse_DomainRspFailed,
	Result_Error_Curl_WriteBack_Failed,
	Result_Error_UnSupportType,
	Result_Error_CreateRenewData_Failed,
}ResultErrorCode;

/** @brief Error code enum define*/
typedef enum{
	Result_Warn_Internet_Unusable = 200,		/*internet can not use now**/
}ResultWarnCode;

/** @brief Error code enum define*/
typedef enum{
	Value_SUCCESS=0,						/**< sucess*/
	Value_Unkown=-500,							/**<Unkown error*/
	Value_NULL_Point,						/**<Null data pointer,*/
	Value_InvalidParam,						/**< InvalidParam*/
	Value_InvalidContext,					/**< InvalidContext*/
	Value_OpenFileFailed,					/**< OpenFileFailed*/
	Value_OpenFailed,						/**< OpenFailed*/
	Value_CloseFailed,						/**< CloseFailed*/
	Value_GenChanllegeFailed,				/**< GenChanllege*/
	Value_InstallLicByLicFileFailed,			/**< InstallLic ByLicFile Failed*/
	Value_InstallLicByRespDataFailed,			/**< InstallLic ByRespData Failed*/
	Value_InstallLicByCurrPackFailed,			/**< InstallLic ByCurrPack Failed*/
	Value_GetDrmHeaderFailed,				/**< GetDrmHeader*/
	Value_GetRightsInfoFailed,				/**< GetRightsInfo*/
	Value_GetLicenseUrlFailed,				/**< GetLicenseUrl*/
	Value_GetLicenseXmlFailed,				/**< GetLicenseXml*/
	Value_GetLic_ByCurPackFileFailed,		/**< GetLicense ByCurPackFile*/
	Value_DeleteLicFailed,					/**< DeleteLicFailed*/
	Value_CleanAllLicFailed,					/**< CleanAllLicFailed*/
	Value_CleanInvalidLicFailed,				/**< CleanInvalidLicFailed*/
	Value_DecryptFailed,						/**< DecryptFailed*/
	Value_JoinDomainFailed,					/**< JoinDomainFailed*/
	Value_LeaveDomainFailed,				/**< LeaveDomainFailed*/
	Value_DecryptSubContentFailed,			/**< DecryptSubContentFailed*/
	Value_RepacketContentfileFailed,			/**< RepacketContentfileFailed*/
	Value_ShortBuffer,						/**< Short Buffer error*/
	Value_SetPacket,						/**< SetPacket error*/
	Value_LicenceExpire,
	Value_LicenseNotFind,
	Value_NewMemFailed,
	Value_GetDeviceUUID,
	Value_GenRsaKeyPair,
	Value_GetIP4Addr,
	Value_GenDKMSReqFailed,
	Value_ParseDKMSReqFailed,
	Value_GetSrvInfoFailed,
	Value_AEDecryptFailed,
	Value_SEDecryptFailed,
	Value_SSWriteFailed,
	Value_SSReadFailed,
	Value_ParseJson_ServerInfoFailed,
	Value_Curl_Internet_Failed,
	Value_Get_ContentID_Failed,
	Value_Get_FileData_ByName,
	Value_Gen_CertReq,
	Value_Parse_Algorithm,
	Value_Parse_Customdata,
	Value_Get_FileSizeByName,
	Value_CEK_Error,
	Value_UnCompress_Failed,
	Value_LicStore_Failed,
	Value_GenLicReq_Failed,
	Value_TimeSystem_Tampered,
	Value_Com_ReadDrmInfoFailed,			/*read drm info from common type file failed*/
	Value_Get_ClrFile_ByNameFailed,			/*read clear file from epub failed*/
	Value_Parse_NewJsonFailed,
	Value_Parse_OldJsonFailed,
	Value_OverrideUserDataFailed,
	Value_Upload_UserDataFailed,
	Value_Renew_LicenseFailed,
	Value_Parse_RenewDataFailed,
	Value_Store_AuthcodeFailed,
	Value_Get_AuthcodeFailed,
	Value_Get_LibID,
	Value_Parse_DomainRspFailed,
	Value_Curl_WriteBack_Failed,
	Value_UnSupportType,
	Value_CreateRenewData_Failed,
	Value_Invalid_JsonData,
}ErrorCode;

typedef enum{
	XML_Pas_RightsXML=-1000,				/*parse rights xml failed*/
	XML_Pas_EncryptionXML,
	XML_Create_LicReqXML,
	XML_Create_BasicCode_LicReqXML,
	XML_Create_ServiceCode_LicReqXML,
	XML_Create_DeviceID_LicReqXML,
	XML_Create_UserID_LicReqXML,
	XML_Create_DomainID_LicReqXML,
	XML_Create_ContentID_LicReqXML,
	XML_Create_Customdata_LicReqXML,
	XML_Create_LibID_LicReqXML,
	
	XML_Pas_ResultCode_LicRspXML,
	XML_Pas_BasicCode_LicRspXML,
	XML_Pas_ServiceCode_LicRspXML,
	XML_Pas_LicServerUUID_LicRspXML,
	XML_Pas_LicServerPublikey_LicRspXML,
	XML_Pas_UserID_LicRspXML,
	XML_Pas_DomainID_LicRspXML,
	XML_Pas_ContentID_LicRspXML,
	XML_Pas_CEK_LicRspXML,
	XML_Pas_Authcode_LicRspXML,
	XML_Pas_Issuedate_LicRspXML,
	XML_Pas_Expireddate_LicRspXML,
	XML_Pas_Readingtime_LicRspXML,
	XML_Pas_Usecount_LicRspXML,
	XML_Pas_AuthcodeType_LicRspXML,
	XML_Pas_RerentInter_LicRspXML,
	
	XML_ReCreate_Other_LicXML,
	XML_ReCreate_BasicCode_LicXML,
	XML_ReCreate_ServiceCode_LicXML,
	XML_ReCreate_ResultCode_LicXML,
	XML_ReCreate_LicServerUUID_LicXML,
	XML_ReCreate_LicServerPublikey_LicXML,
	XML_ReCreate_UserID_LicXML,
	XML_ReCreate_DomainID_LicXML,
	XML_ReCreate_ContentID_LicXML,
	XML_ReCreate_CEK_LicXML,
	XML_ReCreate_Authcode_LicXML,
	XML_ReCreate_Issuedate_LicXML,
	XML_ReCreate_Expireddate_LicXML,
	XML_ReCreate_Readingtime_LicXML,
	XML_ReCreate_Usecount_LicXML,
	XML_ReCreate_AuthcodeType_LicXML,
	XML_ReCreate_RerentInter_LicXML,
	XML_ReCreate_NextRerentTime_LicXML,


	XML_Create_Other_DMKReqXML,
	XML_Create_BasicCode_DMKReqXML,
	XML_Create_ServiceCode_DMKReqXML,
	XML_Create_DeviceUUID_DMKReqXML,
	XML_Create_DeviceType_DMKReqXML,
	XML_Create_PublicKey_DMKReqXML,
	XML_Create_IP_DMKReqXML,
	XML_Create_OSInfo_DMKReqXML,
	XML_Create_UserID_DMKReqXML,
	XML_Create_LibID_DMKReqXML,
	
	XML_Pas_ResultCode_DKMRspXML,
	XML_Pas_BasicCode_DKMRspXML,
	XML_Pas_ServiceCode_DKMRspXML,
	XML_Pas_DKMServerUUID_DKMRspXML,
	XML_Pas_DKMPublikey_DKMRspXML,
	XML_Pas_DKMClientUUID_DKMRspXML,
	XML_Pas_DKMClientPublikey_DKMRspXML,

	XML_Pas_AlgorithmXML,

	XML_Pas_ContentId_CustomDataXML,
	XML_Pas_LibId_CustomDataXML,
	XML_Pas_UserName_CustomDataXML,

	XML_Pas_Rerent_TimeXML,
	XML_Pas_Enc_DomKeyXML,

	XML_Pas_ResultCode_DOMRspXML,
	XML_Pas_BasicCode_DOMRspXML,
	XML_Pas_ServiceCode_DOMRspXML,
	XML_Pas_DeviceUUID_DOMRspXML,
	XML_Pas_UserID_DOMRspXML,
	XML_Pas_DomainID_DOMRspXML,
	XML_Pas_EncryptedDomKey_DOMRspXML,

	XML_Pas_OpfPath_ContainerXML,
	XML_Pas_MetaData_OpfXML,
	XML_Pas_CoverPath_OpfXML,
	XML_Pas_CoverHrefPath_OpfXML,

	XML_Create_RentMode_LicReqXML,
	XML_Pas_Token_LicRspXML,
	
}ParseXMLErrorCode;
#ifdef __cplusplus
};
#endif

#endif //!__XDRM_ERROR_CODE_H__
