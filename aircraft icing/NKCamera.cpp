#include "NKCamera.h"

#include <stdlib.h>
#include <stdio.h>
#include <io.h>
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <sys/stat.h>
#include <qdebug.h>
//#include <QCoreApplication.h>
#include <QCoreApplication> 
#include <QFileInfo> 

#define VIDEO_SIZE_BLOCK  0x500000		// movie data read block size : 5MB

NKCamera::NKCamera()
{
	g_hInstModule = NULL;
	m_pRefMod = NULL;
//	pRefMod->pObject = NULL;
	m_pRefSrc = NULL;
//	pRefSrc->pObject = NULL;
	g_ulCameraType = 0;
	g_bFileRemoved = false;
	m_uiCamIndex = 0;
	m_bIsSnaping = false;
	m_bIsConnect = false;
	m_icurrentValue = -1;
	m_icurrentISO = -1;
	//m_bLiveViewStatus = false;
	m_sAppRoot = "";
	memset(&m_stArray, 0, sizeof(NkMAIDArray));
//	m_timerGrapImage = new QTimer(this);
}


NKCamera::~NKCamera()
{
	DisConnect();
//	if (pRefSrc != NULL)
//	{
//		delete pRefSrc;
//		pRefSrc = NULL;
//	}
	
}

void NKCamera::SetCameraIndex(UINT uiCameraIndex, QString strSerialNum)
{
	m_uiCamIndex = uiCameraIndex;
	m_strSerialNum = strSerialNum;
}

bool NKCamera::Init()
{
	char	ModulePath[MAX_PATH];
//	LPRefObj	pRefMod = NULL;
	char	buf[256];
	ULONG	ulModID = 0, ulSrcID = 0;
	UWORD	wSel;
	BOOL	bRet = true;
	//QString appPath = QCoreApplication::applicationDirPath();

	if (!ImportModule())
	{
		qDebug() << ("ImportModule is fault.");
		return false;
	}
	/*bRet = Search_Module(ModulePath);
	if (bRet == false) {
		qDebug()<<("\"Type0022 Module\" is not found.\n");
		return false;
	}
	bRet = Load_Module(ModulePath);
	if (bRet == false) {
		qDebug() << ("Failed in loading \"Type0022 Module\".\n");
		return false;
	}*/
	// Allocate memory for reference to Module object.
	m_pRefMod = (LPRefObj)malloc(sizeof(RefObj));
	if (m_pRefMod == NULL) {
		qDebug() << ("There is not enough memory.");
		return false;
	}
	InitRefObj(m_pRefMod);

	// Allocate memory for Module object.
	m_pRefMod->pObject = (LPNkMAIDObject)malloc(sizeof(NkMAIDObject));
	if (m_pRefMod->pObject == NULL) {
		qDebug() << ("There is not enough memory.");
		if (m_pRefMod != NULL)	free(m_pRefMod);
		return false;
	}

	//	Open Module object
	m_pRefMod->pObject->refClient = (NKREF)m_pRefMod;
	bRet = Command_Open(NULL,					// When Module_Object will be opend, "pParentObj" is "NULL".
		m_pRefMod->pObject,	// Pointer to Module_Object 
		ulModID);			// Module object ID set by Client
	if (bRet == false) {
		qDebug() << ("Module object can't be opened.\n");
		if (m_pRefMod->pObject != NULL)	
			free(m_pRefMod->pObject);
		if (m_pRefMod != NULL)	
			free(m_pRefMod);
		return false;
	}

	//	Enumerate Capabilities that the Module has.
	bRet = EnumCapabilities(m_pRefMod->pObject, &(m_pRefMod->ulCapCount), &(m_pRefMod->pCapArray), NULL, NULL);
	if (bRet == false) {
		qDebug() << ("Failed in enumeration of capabilities.");
		if (m_pRefMod->pObject != NULL)	
			free(m_pRefMod->pObject);
		if (m_pRefMod != NULL)	
			free(m_pRefMod);
		return false;
	}

	//	Set the callback functions(ProgressProc, EventProc and UIRequestProc).
	bRet = SetProc(m_pRefMod);
	if (bRet == false) {
		qDebug() << ("Failed in setting a call back function.");
		if (m_pRefMod->pObject != NULL)	
			free(m_pRefMod->pObject);
		if (m_pRefMod != NULL)	
			free(m_pRefMod);
		return false;
	}

	//	Set the kNkMAIDCapability_ModuleMode.
	if (CheckCapabilityOperation(m_pRefMod, kNkMAIDCapability_ModuleMode, kNkMAIDCapOperation_Set)) {
		bRet = Command_CapSet(m_pRefMod->pObject, kNkMAIDCapability_ModuleMode, kNkMAIDDataType_Unsigned,
			(NKPARAM)kNkMAIDModuleMode_Controller, NULL, NULL);
		if (bRet == false) {
			qDebug() << ("Failed in setting kNkMAIDCapability_ModuleMode.");
			return false;
		}
	}
	return true;
}

BOOL NKCamera::Connect()
{
	if (!Init())
	{
		return 0;
	}
	if (!Open())
	{
		return 0;
	}
	InitConfig();
	InitConectSlots();
	m_bIsConnect = true;
	return 0;
}

void NKCamera::DisConnect()
{
	if (m_pRefMod!= NULL)
	{
		Close_Module(m_pRefMod);
	//	free(pRefMod);
		m_pRefMod = NULL;
	}
	if (g_hInstModule!=NULL)
	{
		FreeLibrary(g_hInstModule);
		g_hInstModule = NULL;
	}	
	m_bIsConnect = false;
	// Free memory blocks allocated in this function.
	//if (pRefMod->pObject != NULL)	

	//free(&m_stArray);
	
}

BOOL NKCamera::SetLiveViewStatus(bool status)
{
	bool bRet;
	bRet = SetUnsignedCapability(m_pRefSrc, kNkMAIDCapability_LiveViewStatus, status);
	return bRet;
}

bool NKCamera::SetISO(int iso)
{
	bool bRet;
	bRet = SetEnumCapability(m_pRefSrc, kNkMAIDCapability_Sensitivity, (UWORD)iso);

	return bRet;
}

bool NKCamera::AutoFocus(int focus)
{
	int bRet;
	bRet = IssueProcess(m_pRefSrc, kNkMAIDCapability_AutoFocus);
	return bRet;
}

//BOOL NKCamera::GetSnapImage(LPRefObj pRefSrc, LPNkMAIDArray pstArray)
//{
//	ULONG	ulHeaderSize = 0;		//The header size of LiveView
//	int i = 0;
//	BOOL	bRet = true;
//
//	bRet = GetArrayCapability(pRefSrc, kNkMAIDCapability_GetLiveViewImage, pstArray);
//	if (bRet == false)
//		return false;
//	return true; (m_pRefSrc, &m_stArray);
//	return 0;
//}

void NKCamera::slotGrapImage()
{
	//GetCameraImage(m_pRefSrc);

	//signalHasSaveImage("filepath");
}
BOOL NKCamera::StartSnap(UINT ImageNum)
{
	if (m_bIsConnect)
	{
		SetLiveViewStatus(true);
		BOOL bRet;
		GetCameraImage(m_pRefSrc);

	//	SaveImage("E:\\vstest\\grap.jpg", );//&m_pRefSrc->pCapArray
		bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_Preview, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_CompressRAW,&m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_CaptureDustImage,&m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_FormatStorage, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_PreCapture, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_Aperture, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_AFsPriority, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_PlayBackImage, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_LimitImageDisplay, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_RGBGain, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_RawJpegImageStatus, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_PreviewImage, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_JpegCompressionPolicy, &m_stArray);
		//bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_LiveViewAF, &m_stArray);
	//	bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_LiveViewAF, &m_stArray);
	//	bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_LiveViewAF, &m_stArray);
	//	bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_LiveViewAF, &m_stArray);
	//	bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_LiveViewAF, &m_stArray);
	//	bRet = GetArrayCapability(m_pRefSrc, kNkMAIDCapability_LiveViewAF, &m_stArray);


		emit signalHasSaveImage("filepath");
		//	m_timerGrapImage->start(1000);
		return true;
	}
	return false;
}

BOOL NKCamera::StopSnap()
{
//	m_timerGrapImage->stop();
	return true;
}

BOOL NKCamera::StartGrapViewImage()
{
//	QString path = ;
	if (m_bIsConnect)
	{
		SetLiveViewStatus(true);

		GetLiveViewImageCapability(m_pRefSrc, &m_stArray);

		QString imagepath = m_sAppRoot + "/res/liveview.jpg";
		SaveImage(imagepath, &m_stArray);

		emit signalHasSaveViewImage(imagepath);
		return true;
	}
	return false;
}

BOOL NKCamera::SaveImage(QString filename, LPNkMAIDArray pstArray)
{
	char* ImageFileName;
	ULONG	ulHeaderSize = 0;		//The header size of LiveView
	unsigned char* pucData = NULL;	// LiveView data pointer
	QByteArray cdata = filename.toLocal8Bit();
	ImageFileName = cdata.data();
	FILE*	hFileImage = NULL;
	fopen_s(&hFileImage, ImageFileName, "r");
	if( hFileImage != NULL)
	{
		if (hFileImage)
		{
			fclose(hFileImage);
			hFileImage = NULL;
			return false;
		}
	}
	// Set header size of LiveView
	if (g_ulCameraType == kNkMAIDCameraType_D850)
	{
		ulHeaderSize = 384;
	}
	// Get data pointer
	pucData = (unsigned char*)pstArray->pData;
	fopen_s(&hFileImage, ImageFileName, "wb");

	fwrite(pucData + ulHeaderSize, 1, (pstArray->ulElements - ulHeaderSize), hFileImage);
	fclose(hFileImage);

	return true;
}

BOOL NKCamera::SetExposureTime(int64_t ExposureTime)
{
	return 0;
}

BOOL NKCamera::SetTriggerFunction(BOOL TrigMode)
{

	return 0;
}

void NKCamera::SetShowWnd(QWidget * pWnd)
{

}

BOOL NKCamera::SetExposureDelayTime(double ExposureDelay)
{

	return 0;
}

QString NKCamera::GetDeviceName()
{

	return QString();
}

int NKCamera::GetlistISO(QStringList& isolist)
{
	m_icurrentValue = -1;
	if (isolist.size()>0)
	{
		isolist.clear();
	}
	BOOL bRet;
	//int i = 0;
	//QStringList listISO;
	bRet = GetEnumCapability(m_pRefSrc, kNkMAIDCapability_Sensitivity, isolist);
	if (bRet == TRUE)
	{
		m_icurrentISO = m_icurrentValue;
		return m_icurrentValue;
	}
	return -1;
	//return QStringList();
}

int NKCamera::GetImageSize(QStringList & list)
{
	m_icurrentValue = -1;
	if (list.size()>0)
	{
		list.clear();
	}
	BOOL bRet;
	//int i = 0;
	//QStringList listISO;
	bRet = GetEnumCapability(m_pRefSrc, kNkMAIDCapability_ImageSize, list);
	if (bRet == TRUE)
	{
		return m_icurrentValue;
	}
	return -1;
}

BOOL NKCamera::InitConectSlots()
{
//	connect(m_timerGrapImage,SIGNAL(timeout()),this,SLOT(slotGrapImage));

	return true;
}

BOOL NKCamera::Open()
{
	// Select Device
	BOOL bRet = true;
	ULONG ulSrcID = 0;	// 0 means Device count is zero. 
	bRet &= SelectSource(m_pRefMod, &ulSrcID);
	if (bRet == false) 
		return false;
	if (ulSrcID > 0 && m_uiCamIndex <=  ulSrcID)
	{
		m_pRefSrc = GetRefChildPtr_ID(m_pRefMod, m_uiCamIndex);
		if (m_pRefSrc == NULL) {
			// Create Source object and RefSrc structure.
			if (AddChild(m_pRefMod, m_uiCamIndex) == TRUE) {
				qDebug() << ("Source object is opened.\n");
			}
			else {
				qDebug() << ("Source object can't be opened.\n");
				return false;
			}
			m_pRefSrc = GetRefChildPtr_ID(m_pRefMod, m_uiCamIndex);
		}
		// Get CameraType
		Command_CapGet(m_pRefSrc->pObject, kNkMAIDCapability_CameraType, kNkMAIDDataType_UnsignedPtr, (NKPARAM)&g_ulCameraType, NULL, NULL);
		return true;
	}	
	return false;	
}

BOOL NKCamera::InitConfig()
{
	//Set SaveMedia,0:card,1:SDRAM,2:card+SDRAM
	BOOL bRet = true;
	bRet &= SetUnsignedCapability(m_pRefSrc, kNkMAIDCapability_SaveMedia, kNkMAIDSaveMedia_SDRAM);
	//bRet &= SetStringCapability(m_pRefSrc, kNkMAIDCapability_CurrentDirectory, "d:\\");

	//Get
	return bRet;
}

//------------------------------------------------------------------------------------------------
//
SLONG CallMAIDEntryPoint(
	LPNkMAIDObject	pObject,				// module, source, item, or data object
	ULONG				ulCommand,			// Command, one of eNkMAIDCommand
	ULONG				ulParam,				// parameter for the command
	ULONG				ulDataType,			// Data type, one of eNkMAIDDataType
	NKPARAM			data,					// Pointer or long integer
	LPNKFUNC			pfnComplete,		// Completion function, may be NULL
	NKREF				refComplete)		// Value passed to pfnComplete
{
	return (*(LPMAIDEntryPointProc)g_pMAIDEntryPoint)(
		pObject, ulCommand, ulParam, ulDataType, data, pfnComplete, refComplete);
}
//------------------------------------------------------------------------------------------------
//
BOOL Search_Module(void* Path)
{
#if defined( _WIN32 )
	const char* 	TempPath;
	struct	_finddata_t c_file;
	intptr_t	hFile;

	// Search a module file in the current directory.
	QString path = QCoreApplication::applicationDirPath();
	path += "/res/Type0022.md3";

	std::string str = path.toStdString();
	TempPath = str.c_str();
	//GetCurrentDirectory(MAX_PATH , (LPWSTR)TempPath);
	//strcat(path, "\\3rdparty\\nikon\\lib\\Type0022.md3");
	if ((hFile = _findfirst(TempPath, &c_file)) == -1L) {
		return false;
	}
	strcpy((char*)Path, TempPath);
#elif defined(__APPLE__)
	uint32_t pathSize = 0;
	_NSGetExecutablePath(NULL, &pathSize);

	if (pathSize == 0) {
		return FALSE;
	}

	char*buf[pathSize];
	int noError = (_NSGetExecutablePath((char*)buf, &pathSize) == 0);
	if (!noError) {
		return FALSE;
	}

	CFStringRef runningPath = CFStringCreateWithCString(kCFAllocatorDefault, (char*)buf, kCFStringEncodingUTF8);

	CFURLRef runningPathUrl = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, runningPath, kCFURLPOSIXPathStyle, true);
	CFRelease(runningPath); runningPath = NULL;

	CFURLRef parentFolderUrl = CFURLCreateCopyDeletingLastPathComponent(kCFAllocatorDefault, runningPathUrl);
	CFRelease(runningPathUrl); runningPathUrl = NULL;

	CFURLRef moduleUrl = CFURLCreateCopyAppendingPathComponent(kCFAllocatorDefault, parentFolderUrl, CFSTR("Type0022 Module.bundle"), true);
	CFRelease(parentFolderUrl); parentFolderUrl = NULL;

	CFErrorRef error = NULL;
	if (moduleUrl != NULL && CFURLResourceIsReachable(CFURLCreateFilePathURL(kCFAllocatorDefault, moduleUrl, NULL), &error)) {
		*(CFURLRef*)Path = moduleUrl;
	}
	else {
		return FALSE;
	}
#endif
	return true;
}
//------------------------------------------------------------------------------------------------
//
BOOL Load_Module(void* Path)
{
#if defined( _WIN32 )
	g_hInstModule = LoadLibrary((LPCWSTR)Path);

	if (g_hInstModule) {
		g_pMAIDEntryPoint = (LPMAIDEntryPointProc)GetProcAddress(g_hInstModule, "MAIDEntryPoint");
		if (g_pMAIDEntryPoint == NULL)
			puts("MAIDEntryPoint cannot be found.\n");
	}
	else {
		g_pMAIDEntryPoint = NULL;
		qDebug() << ("\"%s\" cannot be opened.\n", Path);
	}
	return (g_hInstModule != NULL) && (g_pMAIDEntryPoint != NULL);
#elif defined(__APPLE__)
	// Create CFByundle object from CFURLRef.
	gBundle = CFBundleCreate(kCFAllocatorDefault, (CFURLRef)Path);
	if (gBundle == NULL) {
		return FALSE;
	}
	// Load and link dynamic CFBundle object 
	if (!CFBundleLoadExecutable(gBundle)) {
		CFRelease(gBundle);
		gBundle = NULL;
		return FALSE;
	}
	// Get entry point from BundleRef
	// Set the pointer for Maid entry point LPMAIDEntryPointProc type variabl
	g_pMAIDEntryPoint = (LPMAIDEntryPointProc)CFBundleGetFunctionPointerForName(gBundle, CFSTR("MAIDEntryPoint"));
	return (g_pMAIDEntryPoint != NULL);
#endif
}
//------------------------------------------------------------------------------------------------
//
BOOL Command_Open(NkMAIDObject* pParentObj, NkMAIDObject* pChildObj, ULONG ulChildID)
{
	SLONG lResult = CallMAIDEntryPoint(pParentObj, kNkMAIDCommand_Open, ulChildID,
		kNkMAIDDataType_ObjectPtr, (NKPARAM)pChildObj, NULL, NULL);
	return lResult == kNkMAIDResult_NoError;
}
//------------------------------------------------------------------------------------------------
//
BOOL Command_Close(LPNkMAIDObject pObject)
{
	SLONG nResult = CallMAIDEntryPoint(pObject, kNkMAIDCommand_Close, 0, 0, 0, NULL, NULL);

	return nResult == kNkMAIDResult_NoError;
}
//------------------------------------------------------------------------------------------------
//
BOOL Close_Module(LPRefObj pRefMod)
{
	BOOL bRet;
	LPRefObj pRefSrc, pRefItm, pRefDat;
	ULONG i, j, k;

	if (pRefMod->pObject != NULL)
	{
		for (i = 0; i < pRefMod->ulChildCount; i++)
		{
			pRefSrc = GetRefChildPtr_Index(pRefMod, i);
			for (j = 0; j < pRefSrc->ulChildCount; j++)
			{
				pRefItm = GetRefChildPtr_Index(pRefSrc, j);
				for (k = 0; k < pRefItm->ulChildCount; k++)
				{
					pRefDat = GetRefChildPtr_Index(pRefItm, k);
					bRet = ResetProc(pRefDat);
					if (bRet == false)	return false;
					bRet = Command_Close(pRefDat->pObject);
					if (bRet == false)	return false;
					free(pRefDat->pObject);
					free(pRefDat->pCapArray);
					free(pRefDat);//
					pRefDat = NULL;//
				}
				bRet = ResetProc(pRefItm);
				if (bRet == false)	return false;
				bRet = Command_Close(pRefItm->pObject);
				if (bRet == false)	return false;
				free(pRefItm->pObject);
				free(pRefItm->pRefChildArray);
				free(pRefItm->pCapArray);
				free(pRefItm);//
				pRefItm = NULL;//
			}
			bRet = ResetProc(pRefSrc);
			if (bRet == false)	return false;
			bRet = Command_Close(pRefSrc->pObject);
			if (bRet == false)	return false;
			free(pRefSrc->pObject);
			free(pRefSrc->pRefChildArray);
			free(pRefSrc->pCapArray);
			free(pRefSrc);//
			pRefSrc = NULL;//
		}
		bRet = ResetProc(pRefMod);
		if (bRet == false)	return false;
		bRet = Command_Close(pRefMod->pObject);
		if (bRet == false)	return false;
		free(pRefMod->pObject);
		pRefMod->pObject = NULL;

		free(pRefMod->pRefChildArray);
		pRefMod->pRefChildArray = NULL;
		pRefMod->ulChildCount = 0;

		free(pRefMod->pCapArray);
		pRefMod->pCapArray = NULL;
		pRefMod->ulCapCount = 0;
	}
	return true;
}
//------------------------------------------------------------------------------------------------
//
void InitRefObj(LPRefObj pRef)
{
	pRef->pObject = NULL;
	pRef->lMyID = 0x8000;
	pRef->pRefParent = NULL;
	pRef->ulChildCount = 0;
	pRef->pRefChildArray = NULL;
	pRef->ulCapCount = 0;
	pRef->pCapArray = NULL;
}
//------------------------------------------------------------------------------------------------------------------------------------
// issue async command while wait for the CompletionProc called.
BOOL IdleLoop(LPNkMAIDObject pObject, ULONG* pulCount, ULONG ulEndCount)
{
	BOOL bRet = true;
	while (*pulCount < ulEndCount && bRet == TRUE) {
		bRet = Command_Async(pObject);
#if defined( _WIN32 )
		Sleep(10);
#elif defined(__APPLE__)
		struct timespec t;
		t.tv_sec = 0;
		t.tv_nsec = 10 * 1000;// 10 msec == 10 * 1000 nsec
		nanosleep(&t, NULL);
#endif
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Wait for Apple event. On MacOSX, the event from camera is an Apple event. 
void WaitEvent()
{
#if defined( _WIN32 )
	// Do nothing
#elif defined(__APPLE__)
	// Do nothing
#endif
}
//------------------------------------------------------------------------------------------------------------------------------------
// enumerate capabilities belong to the object that 'pObject' points to.
BOOL EnumCapabilities(LPNkMAIDObject pObject, ULONG* pulCapCount, LPNkMAIDCapInfo* ppCapArray, LPNKFUNC pfnComplete, NKREF refComplete)
{
	SLONG nResult;

	do {
		// call the module to get the number of the capabilities.
		ULONG	ulCount = 0L;
		LPRefCompletionProc pRefCompletion;
		// This memory block is freed in the CompletionProc.
		pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
		pRefCompletion->pulCount = &ulCount;
		pRefCompletion->pRef = NULL;
		nResult = CallMAIDEntryPoint(pObject,
			kNkMAIDCommand_GetCapCount,
			0,
			kNkMAIDDataType_UnsignedPtr,
			(NKPARAM)pulCapCount,
			(LPNKFUNC)CompletionProc,
			(NKREF)pRefCompletion);
		IdleLoop(pObject, &ulCount, 1);

		if (nResult == kNkMAIDResult_NoError)
		{
			// allocate memory for the capability array
			*ppCapArray = (LPNkMAIDCapInfo)malloc(*pulCapCount * sizeof(NkMAIDCapInfo));

			if (*ppCapArray != NULL)
			{
				// call the module to get the capability array
				ulCount = 0L;
				// This memory block is freed in the CompletionProc.
				pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
				pRefCompletion->pulCount = &ulCount;
				pRefCompletion->pRef = NULL;
				nResult = CallMAIDEntryPoint(pObject,
					kNkMAIDCommand_GetCapInfo,
					*pulCapCount,
					kNkMAIDDataType_CapInfoPtr,
					(NKPARAM)*ppCapArray,
					(LPNKFUNC)CompletionProc,
					(NKREF)pRefCompletion);
				IdleLoop(pObject, &ulCount, 1);

				if (nResult == kNkMAIDResult_BufferSize)
				{
					free(*ppCapArray);
					*ppCapArray = NULL;
				}
			}
		}
	}
	// repeat the process if the number of capabilites changed between the two calls to the module
	while (nResult == kNkMAIDResult_BufferSize);

	// return TRUE if the capabilities were successfully enumerated
	return (nResult == kNkMAIDResult_NoError || nResult == kNkMAIDResult_Pending);
}
//------------------------------------------------------------------------------------------------------------------------------------
// enumerate child object
BOOL EnumChildrten(LPNkMAIDObject pobject)
{
	SLONG nResult;
	ULONG	ulCount = 0L;
	LPRefCompletionProc pRefCompletion;
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;
	nResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_EnumChildren,
		0,
		kNkMAIDDataType_Null,
		(NKPARAM)NULL,
		(LPNKFUNC)CompletionProc,
		(NKREF)pRefCompletion);
	IdleLoop(pobject, &ulCount, 1);

	return (nResult == kNkMAIDResult_NoError);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_CapGetArray(LPNkMAIDObject pobject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete)
{
	SLONG nResult;
	ULONG	ulCount = 0L;
	LPRefCompletionProc pRefCompletion;
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;
	nResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_CapGetArray,
		ulParam,
		ulDataType,
		pData,
		(LPNKFUNC)CompletionProc,
		(NKREF)pRefCompletion);
	IdleLoop(pobject, &ulCount, 1);

	return (nResult == kNkMAIDResult_NoError);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_CapGetDefault(LPNkMAIDObject pobject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete)
{
	SLONG nResult;
	ULONG	ulCount = 0L;
	LPRefCompletionProc pRefCompletion;
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;
	nResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_CapGetDefault,
		ulParam,
		ulDataType,
		pData,
		(LPNKFUNC)CompletionProc,
		(NKREF)pRefCompletion);
	IdleLoop(pobject, &ulCount, 1);

	return (nResult == kNkMAIDResult_NoError);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_CapGet(LPNkMAIDObject pobject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete)
{
	SLONG nResult;
	ULONG	ulCount = 0L;
	LPRefCompletionProc pRefCompletion;
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;
	nResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_CapGet,
		ulParam,
		ulDataType,
		pData,
		(LPNKFUNC)CompletionProc,
		(NKREF)pRefCompletion);
	IdleLoop(pobject, &ulCount, 1);

	return (nResult == kNkMAIDResult_NoError);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_CapSet(LPNkMAIDObject pobject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete)
{
	BOOL bSuccess = FALSE;
	SLONG nResult;
	ULONG	ulCount = 0L;
	LPRefCompletionProc pRefCompletion;
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;
	nResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_CapSet,
		ulParam,
		ulDataType,
		pData,
		(LPNKFUNC)CompletionProc,
		(NKREF)pRefCompletion);
	IdleLoop(pobject, &ulCount, 1);

	// MovRecInCardStatusの場合、Result Codesがゼロ(No Error)以外にも成功コードが存在する
	if (ulParam == kNkMAIDCapability_MovRecInCardStatus)
	{
		switch (nResult)
		{
		case kNkMAIDResult_NoError:
			bSuccess = TRUE;
			break;
		case kNkMAIDResult_RecInCard:
			qDebug() << ("Card Recording\n");
			bSuccess = TRUE;
			break;
		case kNkMAIDResult_RecInExternalDevice:
			qDebug() << ("ExternalDevice Recording\n");
			bSuccess = TRUE;
			break;
		case kNkMAIDResult_RecInCardAndExternalDevice:
			qDebug() << ("Card and ExternalDevice Recording\n");
			bSuccess = TRUE;
			break;
		default:
			bSuccess = FALSE;
			break;
		}
	}
	else
	{
		bSuccess = (nResult == kNkMAIDResult_NoError) ? TRUE : FALSE;
	}

	return (bSuccess);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_CapGetSB(LPNkMAIDObject pobject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult)
{
	SLONG nResult;
	ULONG	ulCount = 0L;
	LPRefCompletionProc pRefCompletion;
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;
	nResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_CapGet,
		ulParam,
		ulDataType,
		pData,
		(LPNKFUNC)CompletionProc,
		(NKREF)pRefCompletion);
	IdleLoop(pobject, &ulCount, 1);

	*pnResult = nResult;
	return (nResult == kNkMAIDResult_NoError);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_CapSetSB(LPNkMAIDObject pobject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult)
{
	SLONG nResult;
	ULONG	ulCount = 0L;
	LPRefCompletionProc pRefCompletion;
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;
	nResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_CapSet,
		ulParam,
		ulDataType,
		pData,
		(LPNKFUNC)CompletionProc,
		(NKREF)pRefCompletion);
	IdleLoop(pobject, &ulCount, 1);

	*pnResult = nResult;
	return (nResult == kNkMAIDResult_NoError);
}

//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_CapStart(LPNkMAIDObject pobject, ULONG ulParam, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult)
{
	SLONG nResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_CapStart,
		ulParam,
		kNkMAIDDataType_Null,
		(NKPARAM)NULL,
		pfnComplete,
		refComplete);
	if (pnResult != NULL) *pnResult = nResult;

	return (nResult == kNkMAIDResult_NoError || nResult == kNkMAIDResult_Pending || nResult == kNkMAIDResult_BulbReleaseBusy ||
		nResult == kNkMAIDResult_SilentReleaseBusy || nResult == kNkMAIDResult_MovieFrameReleaseBusy ||
		nResult == kNkMAIDResult_Waiting_2ndRelease);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_CapStartGeneric(LPNkMAIDObject pObject, ULONG ulParam, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult)
{
	SLONG nResult = CallMAIDEntryPoint(pObject,
		kNkMAIDCommand_CapStart,
		ulParam,
		kNkMAIDDataType_GenericPtr,
		pData,
		pfnComplete,
		refComplete);
	if (pnResult != NULL) *pnResult = nResult;

	return (nResult == kNkMAIDResult_NoError || nResult == kNkMAIDResult_Pending);
}

//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_Abort(LPNkMAIDObject pobject, LPNKFUNC pfnComplete, NKREF refComplete)
{
	SLONG lResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_Abort,
		(ULONG)NULL,
		kNkMAIDDataType_Null,
		(NKPARAM)NULL,
		pfnComplete,
		refComplete);
	return lResult == kNkMAIDResult_NoError;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL Command_Async(LPNkMAIDObject pobject)
{
	SLONG lResult = CallMAIDEntryPoint(pobject,
		kNkMAIDCommand_Async,
		0,
		kNkMAIDDataType_Null,
		(NKPARAM)NULL,
		(LPNKFUNC)NULL,
		(NKREF)NULL);
	return(lResult == kNkMAIDResult_NoError || lResult == kNkMAIDResult_Pending);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL SelectSource(LPRefObj pRefObj, ULONG *pulSrcID)
{
	BOOL	bRet;
	NkMAIDEnum	stEnum;
	char	buf[256];
	UWORD	wSel;
	ULONG	i;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_Children);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Enum) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_Children, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);
	if (bRet == false) return false;

	// check the data of the capability.
	if (stEnum.wPhysicalBytes != 4) return false;

	if (stEnum.ulElements == 0) {
		qDebug()<<("There is no Source object.\n0. Exit\n>");
		//scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	stEnum.pData = malloc(stEnum.ulElements * stEnum.wPhysicalBytes);
	if (stEnum.pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);
	if (bRet == false) {
		free(stEnum.pData);
		return false;
	}
	*pulSrcID = ((ULONG*)stEnum.pData)[stEnum.ulElements-1];
	free(stEnum.pData);	
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL SelectItem(LPRefObj pRefObj, ULONG *pulItemID)
{
	BOOL	bRet;
	NkMAIDEnum	stEnum;
	char	buf[256];
	UWORD	wSel;
	ULONG	i;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_Children);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Enum) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_Children, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);
	if (bRet == false) return false;

	// check the data of the capability.
	if (stEnum.ulElements == 0) {
		qDebug() << ("There is no item.\n");
		return true;
	}

	// check the data of the capability.
	if (stEnum.wPhysicalBytes != 4) return false;

	// allocate memory for array data
	stEnum.pData = malloc(stEnum.ulElements * stEnum.wPhysicalBytes);
	if (stEnum.pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);
	if (bRet == false) {
		free(stEnum.pData);
		return false;
	}

	// show the list of selectable Items
	for (i = 0; i < stEnum.ulElements; i++)
		qDebug() << ("%d. Internal ID = %08X\n", i + 1, ((ULONG*)stEnum.pData)[i]);

	if (stEnum.ulElements == 0)
		qDebug() << ("There is no Item object.\n0. Exit\n>");
	else if (stEnum.ulElements == 1)
		qDebug() << ("0. Exit\nSelect (1, 0)\n>");
	else
		qDebug() << ("0. Exit\nSelect (1-%d, 0)\n>", stEnum.ulElements);

	scanf("%s", buf);
	wSel = atoi(buf);

	if (wSel > 0 && wSel <= stEnum.ulElements) {
		*pulItemID = ((ULONG*)stEnum.pData)[wSel - 1];
		free(stEnum.pData);
	}
	else {
		free(stEnum.pData);
		if (wSel != 0) return false;
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL SelectData(LPRefObj pRefObj, ULONG *pulDataType)
{
	BOOL	bRet;
	char	buf[256];
	UWORD	wSel;
	ULONG	ulDataTypes, i = 0, DataTypes[8];
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_DataTypes);
	if (pCapInfo == NULL) return false;

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_DataTypes, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_DataTypes, kNkMAIDDataType_UnsignedPtr, (NKPARAM)&ulDataTypes, NULL, NULL);
	if (bRet == false) return false;

	// show the list of selectable Data type object.
	if (ulDataTypes & kNkMAIDDataObjType_Image) {

		DataTypes[i++] = kNkMAIDDataObjType_Image;
		qDebug() << ("%d. Image\n", i);
	}
	if (ulDataTypes & kNkMAIDDataObjType_Video) {

		DataTypes[i++] = kNkMAIDDataObjType_Video;
		qDebug() << ("%d. Movie\n", i);
	}
	if (ulDataTypes & kNkMAIDDataObjType_Thumbnail) {
		DataTypes[i++] = kNkMAIDDataObjType_Thumbnail;
		qDebug() << ("%d. Thumbnail\n", i);
	}

	if (i == 0)
		qDebug() << ("There is no Data object.\n0. Exit\n>");
	else if (i == 1)
		qDebug() << ("0. Exit\nSelect (1, 0)\n>");
	else
		qDebug() << ("0. Exit\nSelect (1-%d, 0)\n>", i);

	scanf("%s", buf);
	wSel = atoi(buf);

	if (wSel > 0 && wSel <= i)
		*pulDataType = DataTypes[wSel - 1];

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL CheckDataType(LPRefObj pRefObj, ULONG *pulDataType)
{
	BOOL	bRet;
	ULONG	ulDataTypes = 0;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_DataTypes);
	if (pCapInfo == NULL) return false;

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_DataTypes, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_DataTypes, kNkMAIDDataType_UnsignedPtr, (NKPARAM)&ulDataTypes, NULL, NULL);
	if (bRet == false) return false;

	// show the list of selectable Data type object.
	if (ulDataTypes & kNkMAIDDataObjType_Video)
	{
		return false;
	}

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
char* GetEnumString(ULONG ulCapID, ULONG ulValue, char *psString)
{
	switch (ulCapID) {
	case kNkMAIDCapability_FlashMode:
		switch (ulValue) {
		case kNkMAIDFlashMode_FrontCurtain:
			strcpy(psString, "Normal");
			break;
		case kNkMAIDFlashMode_RearCurtain:
			strcpy(psString, "Rear-sync");
			break;
		case kNkMAIDFlashMode_SlowSync:
			strcpy(psString, "Slow-sync");
			break;
		case kNkMAIDFlashMode_RedEyeReduction:
			strcpy(psString, "Red Eye Reduction");
			break;
		case kNkMAIDFlashMode_SlowSyncRedEyeReduction:
			strcpy(psString, "Slow-sync Red Eye Reduction");
			break;
		case kNkMAIDFlashMode_SlowSyncRearCurtain:
			strcpy(psString, "SlowRear-sync");
			break;
		case kNkMAIDFlashMode_Off:
			strcpy(psString, "flash off");
			break;
		default:
			qDebug() << (psString, "FlashMode %u", ulValue);
		}
		break;
	case kNkMAIDCapability_ExposureMode:
		switch (ulValue) {
		case kNkMAIDExposureMode_Program:
			strcpy(psString, "Program");
			break;
		case kNkMAIDExposureMode_AperturePriority:
			strcpy(psString, "Aperture");
			break;
		case kNkMAIDExposureMode_SpeedPriority:
			strcpy(psString, "Speed");
			break;
		case kNkMAIDExposureMode_Manual:
			strcpy(psString, "Manual");
			break;
		default:
			qDebug() << (psString, "ExposureMode %u", ulValue);
		}
		break;
	case kNkMAIDCapability_FocusPreferredArea:
		qDebug() << (psString, "FocusPreferredArea :%u", ulValue);
		break;
	case kNkMAIDCapability_PictureControl:
		switch (ulValue) {
		case kNkMAIDPictureControl_Undefined:
			strcpy(psString, "Undefined");
			break;
		case kNkMAIDPictureControl_Standard:
			strcpy(psString, "Standard");
			break;
		case kNkMAIDPictureControl_Neutral:
			strcpy(psString, "Neutral");
			break;
		case kNkMAIDPictureControl_Vivid:
			strcpy(psString, "Vivid");
			break;
		case kNkMAIDPictureControl_Monochrome:
			strcpy(psString, "Monochrome");
			break;
		case kNkMAIDPictureControl_Portrait:
			strcpy(psString, "Portrait");
			break;
		case kNkMAIDPictureControl_Landscape:
			strcpy(psString, "Landscape");
			break;
		case kNkMAIDPictureControl_Flat:
			strcpy(psString, "Flat");
			break;
		case kNkMAIDPictureControl_Auto:
			strcpy(psString, "Auto");
			break;
		case kNkMAIDPictureControl_Custom1:
		case kNkMAIDPictureControl_Custom2:
		case kNkMAIDPictureControl_Custom3:
		case kNkMAIDPictureControl_Custom4:
		case kNkMAIDPictureControl_Custom5:
		case kNkMAIDPictureControl_Custom6:
		case kNkMAIDPictureControl_Custom7:
		case kNkMAIDPictureControl_Custom8:
		case kNkMAIDPictureControl_Custom9:
			qDebug() << (psString, "Custom Picture Contol %d", ulValue - 200);
			break;
		default:
			qDebug() << (psString, "Picture Control %u", ulValue);
		}
		break;
	case kNkMAIDCapability_LiveViewImageZoomRate:
		switch (ulValue) {
		case kNkMAIDLiveViewImageZoomRate_All:
			strcpy(psString, "Full");
			break;
		case kNkMAIDLiveViewImageZoomRate_25:
			strcpy(psString, "25%");
			break;
		case kNkMAIDLiveViewImageZoomRate_50:
			strcpy(psString, "50%");
			break;
		case kNkMAIDLiveViewImageZoomRate_100:
			strcpy(psString, "100%");
			break;
		case kNkMAIDLiveViewImageZoomRate_200:
			strcpy(psString, "200%");
			break;
		default:
			qDebug() << (psString, "LiveViewImageZoomRate %u", ulValue);
		}
		break;
	case kNkMAIDCapability_LiveViewImageSize:
		switch (ulValue) {
		case kNkMAIDLiveViewImageSize_QVGA:
			strcpy(psString, "QVGA");
			break;
		case kNkMAIDLiveViewImageSize_VGA:
			strcpy(psString, "VGA");
			break;
		case kNkMAIDLiveViewImageSize_XGA:
			strcpy(psString, "XGA");
			break;
		default:
			qDebug() << (psString, "LiveViewImageSize %u", ulValue);
		}
		break;
	case kNkMAIDCapability_MovieFileType:
		switch (ulValue) {
		case kNkMAIDMovieFileType_MOV:
			strcpy(psString, "MOV");
			break;
		case kNkMAIDMovieFileType_MP4:
			strcpy(psString, "MP4");
			break;
		default:
			qDebug() << (psString, "MovieFileType %u", ulValue);
		}
		break;
	case kNkMAIDCapability_LiveViewTFTStatus:
		switch (ulValue) {
		case kNkMAIDLiveViewTFTStatus_Off:
			strcpy(psString, "Off");
			break;
		case kNkMAIDLiveViewTFTStatus_On:
			strcpy(psString, "On");
			break;
		default:
			qDebug() << (psString, "LiveViewTFTStatus %u", ulValue);
		}
		break;
	case kNkMAIDCapability_LiveViewButtonMode:
		switch (ulValue) {
		case kNkMAIDLiveViewButtonMode_TFTOnOff:
			strcpy(psString, "TFT On/Off");
			break;
		case kNkMAIDLiveViewButtonMode_LiveView:
			strcpy(psString, "Exit Live View");
			break;
		default:
			qDebug() << (psString, "LiveViewButtonMode %u", ulValue);
		}
		break;
	case kNkMAIDCapability_ExternalRecordingControl:
		switch (ulValue) {
		case kNkMAIDExternalRecordingControl_Off:
			strcpy(psString, "Off");
			break;
		case kNkMAIDExternalRecordingControl_On:
			strcpy(psString, "On");
			break;
		default:
			qDebug() << (psString, "ExternalRecordingControl %u", ulValue);
		}
		break;
	case kNkMAIDCapability_DetectionPeaking:
		switch (ulValue) {
		case kNkMAIDDetectionPeaking_Off:
			strcpy(psString, "Off");
			break;
		case kNkMAIDDetectionPeaking_Low:
			strcpy(psString, "Low");
			break;
		case kNkMAIDDetectionPeaking_Normal:
			strcpy(psString, "Normal");
			break;
		case kNkMAIDDetectionPeaking_High:
			strcpy(psString, "High");
			break;
		default:
			qDebug() << (psString, "DetectionPeaking %u", ulValue);
		}
		break;
	case kNkMAIDCapability_3DTrackingCaptuerArea:
		switch (ulValue) {
		case kNkMAID3DTrackingCaptuerArea_Wide:
			strcpy(psString, "Wide");
			break;
		case kNkMAID3DTrackingCaptuerArea_Normal:
			strcpy(psString, "Normal");
			break;
		default:
			qDebug() << (psString, "3DTrackingCaptuerArea %u", ulValue);
		}
		break;
	case kNkMAIDCapability_SBWirelessMode:
		switch (ulValue) {
		case kNkMAIDSBWirelessMode_Off:
			strcpy(psString, "Off");
			break;
		case kNkMAIDSBWirelessMode_Radio:
			strcpy(psString, "Radio");
			break;
		case kNkMAIDSBWirelessMode_Optical:
			strcpy(psString, "Optical");
			break;
		case kNkMAIDSBWirelessMode_OpticalandRadio:
			strcpy(psString, "OpticalandRadio");
			break;
		default:
			qDebug() << (psString, "SBWirelessMode %u", ulValue);
		}
		break;
	case kNkMAIDCapability_SBWirelessMultipleFlashMode:
		switch (ulValue) {
		case kNkMAIDSBWirelessMultipleFlashMode_Group:
			strcpy(psString, "Group");
			break;
		case kNkMAIDSBWirelessMultipleFlashMode_QuickWireless:
			strcpy(psString, "QuickWireless");
			break;
		case kNkMAIDSBWirelessMultipleFlashMode_Repeat:
			strcpy(psString, "Repeat");
			break;
		default:
			qDebug() << (psString, "SBWirelessMultipleFlashMode %u", ulValue);
		}
		break;
	case kNkMAIDCapability_WirelessCLSEntryMode:
		switch (ulValue) {
		case kNkMAIDWirelessCLSEntryMode_Peering:
			strcpy(psString, "Peering");
			break;
		case kNkMAIDWirelessCLSEntryMode_PINCode:
			strcpy(psString, "PINCode");
			break;
		default:
			qDebug() << (psString, "WirelessCLSEntryMode %u", ulValue);
		}
		break;
	case kNkMAIDCapability_OpticalMultipleFlashChannel:
		switch (ulValue) {
		case kNkMAIDOpticalMultipleFlashChannel_1ch:
			strcpy(psString, "1ch");
			break;
		case kNkMAIDOpticalMultipleFlashChannel_2ch:
			strcpy(psString, "2ch");
			break;
		case kNkMAIDOpticalMultipleFlashChannel_3ch:
			strcpy(psString, "3ch");
			break;
		case kNkMAIDOpticalMultipleFlashChannel_4ch:
			strcpy(psString, "4ch");
			break;
		default:
			qDebug() << (psString, "OpticalMultipleFlashChannel %u", ulValue);
		}
		break;
	case kNkMAIDCapability_FlashRangeDisplay:
		switch (ulValue) {
		case kNkMAIDFlashRangeDisplay_m:
			strcpy(psString, "meter");
			break;
		case kNkMAIDFlashRangeDisplay_ft:
			strcpy(psString, "feet");
			break;
		default:
			qDebug() << (psString, "FlashRangeDisplay %u", ulValue);
		}
		break;
	case kNkMAIDCapability_SBSettingMemberLock:
		switch (ulValue) {
		case kNkMAIDSBSettingMemberLock_Off:
			strcpy(psString, "Off");
			break;
		case kNkMAIDSBSettingMemberLock_On:
			strcpy(psString, "On");
			break;
		default:
			qDebug() << (psString, "SBSettingMemberLock %u", ulValue);
		}
		break;
	case kNkMAIDCapability_SBIntegrationFlashReady:
		switch (ulValue) {
		case kNkMAIDSBIntegrationFlashReady_NotReady:
			strcpy(psString, "NotReady");
			break;
		case kNkMAIDSBIntegrationFlashReady_Ready:
			strcpy(psString, "Ready");
			break;
		default:
			qDebug() << (psString, "SBIntegrationFlashReady %u", ulValue);
		}
		break;
	case kNkMAIDCapability_HighlightBrightness:
		switch (ulValue) {
		case kNkMAIDHighlightBrightness_180:
			strcpy(psString, "180");
			break;
		case kNkMAIDHighlightBrightness_191:
			strcpy(psString, "191");
			break;
		case kNkMAIDHighlightBrightness_202:
			strcpy(psString, "202");
			break;
		case kNkMAIDHighlightBrightness_213:
			strcpy(psString, "213");
			break;
		case kNkMAIDHighlightBrightness_224:
			strcpy(psString, "224");
			break;
		case kNkMAIDHighlightBrightness_235:
			strcpy(psString, "235");
			break;
		case kNkMAIDHighlightBrightness_248:
			strcpy(psString, "248");
			break;
		case kNkMAIDHighlightBrightness_255:
			strcpy(psString, "255");
			break;
		default:
			qDebug() << (psString, "HighlightBrightness %u", ulValue);
		}
		break;
	case kNkMAIDCapability_MovieAttenuator:
		switch (ulValue) {
		case kNkMAIDMovieAttenuator_disable:
			strcpy(psString, "Disable");
			break;
		case kNkMAIDMovieAttenuator_enable:
			strcpy(psString, "Enable");
			break;
		default:
			qDebug() << (psString, "MovieAttenuator %u", ulValue);
		}
		break;
	default:
		strcpy(psString, "Undefined String");
	}
	return psString;
}
//------------------------------------------------------------------------------------------------------------------------------------
char*	GetUnsignedString(ULONG ulCapID, ULONG ulValue, char *psString)
{
	char buff[256];

	switch (ulCapID)
	{
	case kNkMAIDCapability_MeteringMode:
		qDebug() << (buff, "%d : Matrix\n", kNkMAIDMeteringMode_Matrix);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : CenterWeighted\n", kNkMAIDMeteringMode_CenterWeighted);
		strcat(psString, buff);
		qDebug() << (buff, "%d : Spot\n", kNkMAIDMeteringMode_Spot);
		strcat(psString, buff);
		qDebug() << (buff, "%d : HighLight\n", kNkMAIDMeteringMode_HighLight);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_FocusMode:
		qDebug() << (buff, "%d : MF\n", kNkMAIDFocusMode_MF);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : AF-S\n", kNkMAIDFocusMode_AFs);
		strcat(psString, buff);
		qDebug() << (buff, "%d : AF-C\n", kNkMAIDFocusMode_AFc);
		strcat(psString, buff);
		qDebug() << (buff, "%d : AF-F\n", kNkMAIDFocusMode_AFf);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_RawJpegImageStatus:
		qDebug() << (buff, "%d : Single\n", eNkMAIDRawJpegImageStatus_Single);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : Raw + Jpeg\n", eNkMAIDRawJpegImageStatus_RawJpeg);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_DataTypes:
		strcpy(psString, "\0");
		if (ulValue & kNkMAIDDataObjType_Image)
		{
			strcat(psString, "Image, ");
		}
		if (ulValue & kNkMAIDDataObjType_Sound)
		{
			strcat(psString, "Sound, ");
		}
		if (ulValue & kNkMAIDDataObjType_Video)
		{
			strcat(psString, "Video, ");
		}
		if (ulValue & kNkMAIDDataObjType_Thumbnail)
		{
			strcat(psString, "Thumbnail, ");
		}
		if (ulValue & kNkMAIDDataObjType_File)
		{
			strcat(psString, "File ");
		}
		strcat(psString, "\n");
		break;
	case kNkMAIDCapability_ModuleType:
		qDebug() << (buff, "%d : Scanner\n", kNkMAIDModuleType_Scanner);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : Camera\n", kNkMAIDModuleType_Camera);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_WBFluorescentType:
		qDebug() << (buff, "%d : Sodium-vapor lamps\n", kNkWBFluorescentType_SodiumVapor);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : Warm-white fluorescent\n", kNkWBFluorescentType_WarmWhite);
		strcat(psString, buff);
		qDebug() << (buff, "%d : White fluorescent\n", kNkWBFluorescentType_White);
		strcat(psString, buff);
		qDebug() << (buff, "%d : Cool-white fluorescent\n", kNkWBFluorescentType_CoolWhite);
		strcat(psString, buff);
		qDebug() << (buff, "%d : Day white fluorescent\n", kNkWBFluorescentType_DayWhite);
		strcat(psString, buff);
		qDebug() << (buff, "%d : Daylight fluorescent\n", kNkWBFluorescentType_Daylight);
		strcat(psString, buff);
		qDebug() << (buff, "%d : High temp. mercury-vapor\n", kNkWBFluorescentType_HiTempMercuryVapor);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_LiveViewProhibit:
		strcpy(psString, "\0");
		if (ulValue & kNkMAIDLiveViewProhibit_Retractable)
		{
			strcat(psString, "Retractable Lens Warning, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_DuringMirrorup)
		{
			strcat(psString, "Mirrorup, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_BulbWarning)
		{
			strcat(psString, "Shutterspeed is time, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_TempRise)
		{
			strcat(psString, "High Temperature, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_Capture)
		{
			strcat(psString, "Executing Capture, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_NoCardLock)
		{
			strcat(psString, "No Card, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_SdramImg)
		{
			strcat(psString, "SDRAM image in camera, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_TTL)
		{
			strcat(psString, "TTL, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_Battery)
		{
			strcat(psString, "Battery, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_FEE)
		{
			strcat(psString, "FEE, ");
		}
		if (ulValue & kNkMAIDLiveViewProhibit_Sequence)
		{
			strcat(psString, "Sequence, ");
		}
		strcat(psString, "\n");
		break;
	case kNkMAIDCapability_LiveViewStatus:
		qDebug() << (buff, "%d : OFF\n", kNkMAIDLiveViewStatus_OFF);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : ON\n", kNkMAIDLiveViewStatus_ON);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_MovRecInCardStatus:
		qDebug() << (buff, "%d : OFF\n", kNkMAIDMovRecInCardStatus_Off);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : ON\n", kNkMAIDMovRecInCardStatus_On);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_MovRecInCardProhibit:
		strcpy(psString, "\0");
		if (ulValue & kNkMAIDMovRecInCardProhibit_LVPhoto)
		{
			strcat(psString, "LiveViewPhoto, ");
		}
		if (ulValue & kNkMAIDMovRecInCardProhibit_LVImageZoom)
		{
			strcat(psString, "LiveViewZoom, ");
		}
		if (ulValue & kNkMAIDMovRecInCardProhibit_CardProtect)
		{
			strcat(psString, "Card is protected, ");
		}
		if (ulValue & kNkMAIDMovRecInCardProhibit_RecMov)
		{
			strcat(psString, "Recording movie, ");
		}
		if (ulValue & kNkMAIDMovRecInCardProhibit_MovInBuf)
		{
			strcat(psString, "Movie in buffer, ");
		}
		if (ulValue & kNkMAIDMovRecInCardProhibit_CardFull)
		{
			strcat(psString, "Card full, ");
		}
		if (ulValue & kNkMAIDMovRecInCardProhibit_NoFormat)
		{
			strcat(psString, "Card unformatted, ");
		}
		if (ulValue & kNkMAIDMovRecInCardProhibit_CardErr)
		{
			strcat(psString, "Card error, ");
		}
		if (ulValue & kNkMAIDMovRecInCardProhibit_NoCard)
		{
			strcat(psString, "No card, ");
		}
		strcat(psString, "\n");
		break;
	case kNkMAIDCapability_SaveMedia:
		qDebug() << (buff, "%d : Card\n", kNkMAIDSaveMedia_Card);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : SDRAM\n", kNkMAIDSaveMedia_SDRAM);
		strcat(psString, buff);
		qDebug() << (buff, "%d : Card + SDRAM\n", kNkMAIDSaveMedia_Card_SDRAM);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_AFMode:
		qDebug() << (buff, "%d : AF-S\n", kNkMAIDAFMode_S);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : AF-C\n", kNkMAIDAFMode_C);
		strcat(psString, buff);
		qDebug() << (buff, "%d : MF(fixed)\n", kNkMAIDAFMode_M_FIX);
		strcat(psString, buff);
		qDebug() << (buff, "%d : MF(selected)\n", kNkMAIDAFMode_M_SEL);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_MirrorUpStatus:
		qDebug() << (buff, "%d : Mirror down\n", kNkMAIDMirrorUpStatus_Down);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : Mirror up\n", kNkMAIDMirrorUpStatus_Up);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_MovieMeteringMode:
		qDebug() << (buff, "%d : Matrix\n", kNkMAIDMovieMeteringMode_Matrix);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : CenterWeighted\n", kNkMAIDMovieMeteringMode_CenterWeighted);
		strcat(psString, buff);
		qDebug() << (buff, "%d : HighLight\n", kNkMAIDMovieMeteringMode_HighLight);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_AFLockOnAcross:
		qDebug() << (buff, "1(Quick) - 5(Slow)\n");
		strcpy(psString, buff);
		break;
	case kNkMAIDCapability_AFLockOnMove:
		qDebug() << (buff, "1(Steady) - 3(Erratic)\n");
		strcpy(psString, buff);
		break;
	case kNkMAIDCapability_FaceDetection:
		qDebug() << (buff, "%d : Off\n", kNkMAIDFaceDetection_Off);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : On\n", kNkMAIDFaceDetection_On);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_MatrixMetering:
		qDebug() << (buff, "%d : Off\n", kNkMAIDMatrixMetering_Off);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : On\n", kNkMAIDMatrixMetering_On);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_ExposureCompFlashUsed:
		qDebug() << (buff, "%d : Entireframe\n", kNkMAIDExposureCompFlashUsed_Entireframe);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : Backgroundonly\n", kNkMAIDExposureCompFlashUsed_Backgroundonly);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_ElectronicFrontCurtainShutter:
		qDebug() << (buff, "%d : Off\n", kNkMAIDElectronicFrontCurtainShutter_Off);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : On\n", kNkMAIDElectronicFrontCurtainShutter_On);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_AllTestFiringDisable:
		qDebug() << (buff, "%d : Flash permitt\n", kNkMAIDAllTestFiringDisable_Enable);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : Flash off\n", kNkMAIDAllTestFiringDisable_Disable);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_RadioMultipleFlashChannel:
		qDebug() << (buff, "%d : Unknown\n", kNkMAIDRadioMultipleFlashChannel_Unknown);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : 5CH\n", kNkMAIDRadioMultipleFlashChannel_5ch);
		strcat(psString, buff);
		qDebug() << (buff, "%d : 10CH\n", kNkMAIDRadioMultipleFlashChannel_10ch);
		strcat(psString, buff);
		qDebug() << (buff, "%d : 15CH\n", kNkMAIDRadioMultipleFlashChannel_15ch);
		strcat(psString, buff);
		break;

	case kNkMAIDCapability_FlickerReductionSetting:
		qDebug() << (buff, "%d : Off\n", kNkMAIDFlickerReductionSetting_Off);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : On\n", kNkMAIDFlickerReductionSetting_On);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_MovieActive_D_Lighting:
		qDebug() << (buff, "%d : Off\n", kNkMAIDMovieActive_D_Lighting_Off);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : Low\n", kNkMAIDMovieActive_D_Lighting_Low);
		strcat(psString, buff);
		qDebug() << (buff, "%d : Normal\n", kNkMAIDMovieActive_D_Lighting_Normal);
		strcat(psString, buff);
		qDebug() << (buff, "%d : High\n", kNkMAIDMovieActive_D_Lighting_High);
		strcat(psString, buff);
		qDebug() << (buff, "%d : Extra high\n", kNkMAIDMovieActive_D_Lighting_ExtraHigh);
		strcat(psString, buff);
		qDebug() << (buff, "%d : Same as photo\n", kNkMAIDMovieActive_D_Lighting_SamePhoto);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_ElectronicVR:
		qDebug() << (buff, "%d : Off\n", kMAIDElectronicVR_OFF);
		strcpy(psString, buff);
		qDebug() << (buff, "%d : On\n", kMAIDElectronicVR_ON);
		strcat(psString, buff);
		break;
	case kNkMAIDCapability_LimitAFAreaMode:
		qDebug() << (buff, " 0x%04x (   %d ). No limit\n", 0, 0);
		strcpy(psString, buff);
		qDebug() << (buff, " 0x%04x (   %d ). Dynamic 9 point\n", kNkMAIDLimitAFAreaMode2_Dynamic9, kNkMAIDLimitAFAreaMode2_Dynamic9);
		strcat(psString, buff);
		qDebug() << (buff, " 0x%04x (   %d ). Dynamic 25 point\n", kNkMAIDLimitAFAreaMode2_Dynamic25, kNkMAIDLimitAFAreaMode2_Dynamic25);
		strcat(psString, buff);
		qDebug() << (buff, " 0x%04x (   %d ). Dynamic 72 point\n", kNkMAIDLimitAFAreaMode2_Dynamic72, kNkMAIDLimitAFAreaMode2_Dynamic72);
		strcat(psString, buff);
		qDebug() << (buff, " 0x%04x (  %d ). Dynamic 153 point\n", kNkMAIDLimitAFAreaMode2_Dynamic153, kNkMAIDLimitAFAreaMode2_Dynamic153);
		strcat(psString, buff);
		qDebug() << (buff, " 0x%04x (  %d ). 3D-tracking\n", kNkMAIDLimitAFAreaMode2_3DTtracking, kNkMAIDLimitAFAreaMode2_3DTtracking);
		strcat(psString, buff);
		qDebug() << (buff, " 0x%04x (  %d ). Group\n", kNkMAIDLimitAFAreaMode2_Group, kNkMAIDLimitAFAreaMode2_Group);
		strcat(psString, buff);
		qDebug() << (buff, " 0x%04x ( %d ). Auto\n", kNkMAIDLimitAFAreaMode2_Auto, kNkMAIDLimitAFAreaMode2_Auto);
		strcat(psString, buff);
		break;
	default:
		psString[0] = '\0';
		break;
	}
	return psString;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Distribute the function according to array type.
BOOL SetEnumCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	NkMAIDEnum	stEnum;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Enum) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);
	if (bRet == false) return false;

	switch (stEnum.ulType) {
	case kNkMAIDArrayType_Unsigned:
		return SetEnumUnsignedCapability(pRefObj, ulCapID, &stEnum);
		break;
	case kNkMAIDArrayType_PackedString:
		return SetEnumPackedStringCapability(pRefObj, ulCapID, &stEnum);
		break;
	case kNkMAIDArrayType_String:
		return SetEnumStringCapability(pRefObj, ulCapID, &stEnum);
		break;
	default:
		return false;
	}
}

//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Enum(Unsigned Integer) type capability and set a value for it.
BOOL SetEnumUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum)
{
	BOOL	bRet;
	char	psString[32], buf[256];
	UWORD	wSel;
	ULONG	i;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 4) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug() << ("There is no element in this capability. Enter '0' to exit.\n>");
		scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}

	// show selectable items for this capability and current setting
	qDebug() << ("[%s]\n", pCapInfo->szDescription);

	for (i = 0; i < pstEnum->ulElements; i++)
		qDebug() << ("%2d. %s\n", i + 1, GetEnumString(ulCapID, ((ULONG*)pstEnum->pData)[i], psString));
	qDebug() << ("Current Setting: %d\n", pstEnum->ulValue + 1);

	// check if this capability supports CapSet operation.
	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input new value\n>");
		scanf("%s", buf);
		wSel = atoi(buf);
		if (wSel > 0 && wSel <= pstEnum->ulElements) {
			pstEnum->ulValue = wSel - 1;
			// send the selected number
			bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
			// This statement can be changed as follows.
			//bRet = Command_CapSet( pRefObj->pObject, ulCapID, kNkMAIDDataType_Unsigned, (NKPARAM)pstEnum->ulValue, NULL, NULL );
			if (bRet == false) {
				free(pstEnum->pData);
				return false;
			}
		}
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	free(pstEnum->pData);
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Enum(Packed String) type capability and set a value for it.
BOOL SetEnumPackedStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum)
{
	BOOL	bRet;
	char	*psStr, buf[256];
	UWORD	wSel;
	size_t  i;
	ULONG	ulCount = 0;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 1) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug() << ("There is no element in this capability. Enter '0' to exit.\n>");
		scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}

	// show selectable items for this capability and current setting
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	for (i = 0; i < pstEnum->ulElements; ) {
		psStr = (char*)((char*)pstEnum->pData + i);
		qDebug() << ("%2d. %s\n", ++ulCount, psStr);
		i += strlen(psStr) + 1;
	}
	qDebug() << ("Current Setting: %d\n", pstEnum->ulValue + 1);

	// check if this capability supports CapSet operation.
	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input new value\n>");
		scanf("%s", buf);
		wSel = atoi(buf);
		if (wSel > 0 && wSel <= pstEnum->ulElements) {
			pstEnum->ulValue = wSel - 1;
			// send the selected number
			bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
			// This statement can be changed as follows.
			//bRet = Command_CapSet( pRefObj->pObject, ulCapID, kNkMAIDDataType_Unsigned, (NKPARAM)pstEnum->ulValue, NULL, NULL );
			if (bRet == false) {
				free(pstEnum->pData);
				return false;
			}
		}
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	free(pstEnum->pData);
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Enum(String Integer) type capability and set a value for it.
BOOL SetEnumStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum)
{
	BOOL	bRet;
	char	buf[256];
	UWORD	wSel;
	ULONG	i;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 256) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug() << ("There is no element in this capability. Enter '0' to exit.\n>");
		scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}

	// show selectable items for this capability and current setting
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	for (i = 0; i < pstEnum->ulElements; i++)
		qDebug() << ("%2d. %s\n", i + 1, ((NkMAIDString*)pstEnum->pData)[i].str);
	qDebug() << ("Current Setting: %2d\n", pstEnum->ulValue + 1);

	// check if this capability supports CapSet operation.
	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input new value\n>");
		scanf("%s", buf);
		wSel = atoi(buf);
		if (wSel > 0 && wSel <= pstEnum->ulElements) {
			pstEnum->ulValue = wSel - 1;
			// send the selected number
			bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
			// This statement can be changed as follows.
			//bRet = Command_CapSet( pRefObj->pObject, ulCapID, kNkMAIDDataType_Unsigned, (NKPARAM)pstEnum->ulValue, NULL, NULL );
			if (bRet == false) {
				free(pstEnum->pData);
				return false;
			}
		}
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	free(pstEnum->pData);
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Integer type capability and set a value for it.
BOOL SetIntegerCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	SLONG	lValue;
	char	buf[256];
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Integer) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_IntegerPtr, (NKPARAM)&lValue, NULL, NULL);
	if (bRet == false) return false;

	// show current value of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	qDebug() << ("Current Value: %d\n", lValue);

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input new value\n>");
		scanf("%s", buf);
		lValue = (SLONG)atol(buf);
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_Integer, (NKPARAM)lValue, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Unsigned Integer type capability and set a value for it.
BOOL SetUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	ULONG	ulValue;
	char	buf[256];
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Unsigned) return false;
	// check if this capability suports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_UnsignedPtr, (NKPARAM)&ulValue, NULL, NULL);
	if (bRet == false) return false;
	// show current value of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	qDebug() << ("%s", GetUnsignedString(ulCapID, ulValue, buf));
	qDebug() << ("Current Value: %d\n", ulValue);

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		if (ulCapID == kNkMAIDCapability_LimitAFAreaMode)
		{
			qDebug() << ("Input new value(decimal)\n>");
		}
		else
		{
			qDebug() << ("Input new value\n>");
		}
		scanf("%s", buf);
		ulValue = (ULONG)atol(buf);
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_Unsigned, (NKPARAM)ulValue, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Get the current setting of a Unsigned Integer type capability.
BOOL GetUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, ULONG* pulValue)
{
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return FALSE;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Unsigned) return FALSE;
	// check if this capability suports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return FALSE;

	return Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_UnsignedPtr, (NKPARAM)pulValue, NULL, NULL);
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Float type capability and set a value for it.
BOOL SetFloatCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	double	lfValue;
	char	buf[256];
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Float) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_FloatPtr, (NKPARAM)&lfValue, NULL, NULL);
	if (bRet == false) return false;
	// show current value of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	qDebug() << ("Current Value: %f\n", lfValue);

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input new value\n>");
		scanf("%s", buf);
		lfValue = atof(buf);
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_FloatPtr, (NKPARAM)&lfValue, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a String type capability and set a value for it.
BOOL SetStringCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	NkMAIDString	stString;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_String) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_StringPtr, (NKPARAM)&stString, NULL, NULL);
	if (bRet == false) return false;
	// show current value of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	qDebug() << ("Current String: %s\n", stString.str);

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input new string\n>");
#if defined( _WIN32 )
		rewind(stdin);		// clear stdin
#elif defined(__APPLE__)
		gets((char*)stString.str);
#endif
		//change 2018.3.23 
		//gets( (char*)stString.str );
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_StringPtr, (NKPARAM)&stString, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		//scanf("%s", stString.str);
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Size type capability and set a value for it.
BOOL SetSizeCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	NkMAIDSize	stSize;
	char	buf[256];
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Size) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_SizePtr, (NKPARAM)&stSize, NULL, NULL);
	if (bRet == false) return false;
	// show current value of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	qDebug() << ("Current Size: Width = %d    Height = %d\n", stSize.w, stSize.h);

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input Width\n>");
		scanf("%s", buf);
		stSize.w = (ULONG)atol(buf);
		qDebug() << ("Input Height\n>");
		scanf("%s", buf);
		stSize.h = (ULONG)atol(buf);
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_SizePtr, (NKPARAM)&stSize, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a DateTime type capability and set a value for it.
BOOL SetDateTimeCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	NkMAIDDateTime	stDateTime;
	char	buf[256];
	UWORD	wValue;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_DateTime) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_DateTimePtr, (NKPARAM)&stDateTime, NULL, NULL);
	if (bRet == false) return false;
	// show current value of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	qDebug() << ("Current DateTime: %d/%02d/%4d %d:%02d:%02d\n",
		stDateTime.nMonth + 1, stDateTime.nDay, stDateTime.nYear, stDateTime.nHour, stDateTime.nMinute, stDateTime.nSecond);

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input Month(1-12) or Cancel:'c'\n>");
		scanf("%s", buf);
		if (*buf == 'c' || *buf == 'C') return true;
		wValue = atoi(buf);
		if (wValue >= 1 && wValue <= 12)
			stDateTime.nMonth = wValue - 1;

		qDebug() << ("Input Day(1-31) or Cancel:'c'\n>");
		scanf("%s", buf);
		if (*buf == 'c' || *buf == 'C') return true;
		wValue = atoi(buf);
		if (wValue >= 1 && wValue <= 31)
			stDateTime.nDay = wValue;

		qDebug() << ("Input Year(4 digits) or Cancel:'c'\n>");
		scanf("%s", buf);
		if (*buf == 'c' || *buf == 'C') return true;
		wValue = atoi(buf);
		if (wValue > 0)
			stDateTime.nYear = wValue;

		qDebug() << ("Input Hour(0-23) or Cancel:'c'\n>");
		scanf("%s", buf);
		if (*buf == 'c' || *buf == 'C') return true;
		wValue = atoi(buf);
		if (wValue >= 0 && wValue <= 23)
			stDateTime.nHour = wValue;

		qDebug() << ("Input Minute(0-59) or Cancel:'c'\n>");
		scanf("%s", buf);
		if (*buf == 'c' || *buf == 'C') return true;
		wValue = atoi(buf);
		if (wValue >= 0 && wValue <= 59)
			stDateTime.nMinute = wValue;

		qDebug() << ("Input Second(0-59) or Cancel:'c'\n>");
		scanf("%s", buf);
		if (*buf == 'c' || *buf == 'C') return true;
		wValue = atoi(buf);
		if (wValue >= 0 && wValue <= 59)
			stDateTime.nSecond = wValue;

		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_DateTimePtr, (NKPARAM)&stDateTime, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Boolean type capability and set a value for it.
BOOL SetBoolCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	BYTE	bFlag;
	char	buf[256];
	UWORD	wSel;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Boolean) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_BooleanPtr, (NKPARAM)&bFlag, NULL, NULL);
	if (bRet == false) return false;
	// show current setting of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	qDebug() << ("1. On      2. Off\n");
	qDebug() << ("Current Setting: %d\n", bFlag ? 1 : 2);

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input '1' or '2'\n>");
		scanf("%s", buf);
		wSel = atoi(buf);
		if ((wSel == 1) || (wSel == 2)) {
			bFlag = (wSel == 1) ? true : false;
			bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_Boolean, (NKPARAM)bFlag, NULL, NULL);
			if (bRet == false) return false;
		}
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Show the current setting of a Range type capability and set a value for it.
BOOL SetRangeCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	NkMAIDRange	stRange;
	double	lfValue;
	char	buf[256];
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Range) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_RangePtr, (NKPARAM)&stRange, NULL, NULL);
	if (bRet == false) return false;
	// show current value of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);

	// check if this capability supports CapSet operation.
	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		if (stRange.ulSteps == 0) {
			// the value of this capability is set to 'lfValue' directly
			qDebug() << ("Current Value: %f  (Max: %f  Min: %f)\n", stRange.lfValue, stRange.lfUpper, stRange.lfLower);
			qDebug() << ("Input new value.\n>");
			scanf("%s", buf);
			stRange.lfValue = atof(buf);
		}
		else {
			// the value of this capability is calculated from 'ulValueIndex'
			lfValue = stRange.lfLower + stRange.ulValueIndex * (stRange.lfUpper - stRange.lfLower) / (stRange.ulSteps - 1);
			qDebug() << ("Current Value: %f  (Max: %f  Min: %f)\n", lfValue, stRange.lfUpper, stRange.lfLower);
			qDebug() << ("Input new value.\n>");
			scanf("%s", buf);
			lfValue = atof(buf);
			stRange.ulValueIndex = (ULONG)((lfValue - stRange.lfLower) * (stRange.ulSteps - 1) / (stRange.lfUpper - stRange.lfLower));
		}
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_RangePtr, (NKPARAM)&stRange, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		if (stRange.ulSteps == 0) {
			// the value of this capability is set to 'lfValue' directly
			lfValue = stRange.lfValue;
		}
		else {
			// the value of this capability is calculated from 'ulValueIndex'
			lfValue = stRange.lfLower + stRange.ulValueIndex * (stRange.lfUpper - stRange.lfLower) / (stRange.ulSteps - 1);
		}
		qDebug() << ("Current Value: %f  (Max: %f  Min: %f)\n", lfValue, stRange.lfUpper, stRange.lfLower);
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Distribute the function according to Point type.
BOOL SetPointCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet;
	NkMAIDPoint	stPoint;
	char	buf[256];
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Point) return false;

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		qDebug() << ("Input x\n>");
		scanf("%s", buf);
		stPoint.x = atoi(buf);
		qDebug() << ("Input y\n>");
		scanf("%s", buf);
		stPoint.y = atoi(buf);
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_PointPtr, (NKPARAM)&stPoint, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		scanf("%s", buf);
	}
	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Set White balance preset data.
BOOL SetWBPresetDataCapability(LPRefObj pRefSrc)
{
	char	buf[256], filename[256];
	NkMAIDWBPresetData	stPresetData;
	LPNkMAIDCapInfo		pCapInfo = NULL;
	FILE	*stream;
	ULONG	count = 0;
	ULONG   ulTotal = 0;
	char	*ptr = NULL;
	BOOL	bRet;

	strcpy(filename, "PresetData.jpg");

	while (1)
	{
		// Preset Number
		qDebug() << ("\nSelect Preset Number(1-6, 0)\n");
		qDebug() << (" 1. d-1\n");
		qDebug() << (" 2. d-2\n");
		qDebug() << (" 3. d-3\n");
		qDebug() << (" 4. d-4\n");
		qDebug() << (" 5. d-5\n");
		qDebug() << (" 6. d-6\n");
		qDebug() << (" 0. Exit\n>");
		scanf("%s", buf);
		stPresetData.ulPresetNumber = atoi(buf);
		if (stPresetData.ulPresetNumber == 0) return true; //Exit
		if (1 > stPresetData.ulPresetNumber || stPresetData.ulPresetNumber > 6)
		{
			qDebug() << ("Invalid Preset Number.\n");
			continue;
		}
		break;
	}

	// Preset gain
	qDebug() << ("\nSet preset gain value by decimal, or Exit(0).\n>");
	scanf("%s", buf);
	stPresetData.ulPresetGain = atoi(buf);
	if (stPresetData.ulPresetGain == 0) return true; //Exit


													 // Check operations
	pCapInfo = GetCapInfo(pRefSrc, kNkMAIDCapability_WBPresetData);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic) return false;
	// check if this capability supports CapSet operation.
	if (!CheckCapabilityOperation(pRefSrc, kNkMAIDCapability_WBPresetData, kNkMAIDCapOperation_Set)) return false;

	// Read preset data from file.
	if ((stream = fopen(filename, "rb")) == NULL)
	{
		qDebug() << ("\nfile open error.\n");
		return false;
	}

	// Max preset data size id 13312
	// Allocate memory for preset data.
	ptr = (char*)malloc(14000);
	stPresetData.pThumbnailData = ptr;
	while (!feof(stream))
	{
		// Read file until eof.
		count = (ULONG)fread(ptr, sizeof(char), 100, stream);
		if (ferror(stream))
		{
			qDebug() << ("\nfile read error.\n");
			fclose(stream);
			free(stPresetData.pThumbnailData);
			return false;
		}
		/* Total up actual bytes read */
		ulTotal += count;
		ptr += count;
		if (13312 < ulTotal)
		{

			qDebug() << ("\nThe size of \"PresetData.jpg\" is over 13312 byte.\n");
			fclose(stream);
			free(stPresetData.pThumbnailData);
			return false;
		}
	}
	stPresetData.ulThumbnailSize = ulTotal;

	// Set preset data.
	bRet = Command_CapSet(pRefSrc->pObject, kNkMAIDCapability_WBPresetData, kNkMAIDDataType_GenericPtr, (NKPARAM)&(stPresetData), NULL, NULL);

	fclose(stream);
	free(stPresetData.pThumbnailData);

	return bRet;
}

//------------------------------------------------------------------------------------------------------------------------------------
//Delete Dram Image
BOOL DeleteDramCapability(LPRefObj pRefItem, ULONG ulItmID)
{
	LPRefObj	pRefSrc = (LPRefObj)pRefItem->pRefParent;
	LPRefObj	pRefDat = NULL;
	BOOL	bRet = true;
	NkMAIDCallback	stProc;
	LPRefDataProc	pRefDeliver;
	LPRefCompletionProc	pRefCompletion;
	ULONG	ulCount = 0L;
	SLONG nResult;


	// 1. Open ImageObject
	pRefDat = GetRefChildPtr_ID(pRefItem, kNkMAIDDataObjType_Image);
	if (pRefDat == NULL)
	{
		// Create Image object and RefSrc structure.
		if (AddChild(pRefItem, kNkMAIDDataObjType_Image) == false)
		{
			qDebug() << ("Image object can't be opened.\n");
			return false;
		}
		pRefDat = GetRefChildPtr_ID(pRefItem, kNkMAIDDataObjType_Image);
	}

	// 2. Set DataProc function
	// 2-1. set reference from DataProc
	pRefDeliver = (LPRefDataProc)malloc(sizeof(RefDataProc));// this block will be freed in CompletionProc.
	pRefDeliver->pBuffer = NULL;
	pRefDeliver->ulOffset = 0L;
	pRefDeliver->ulTotalLines = 0L;
	pRefDeliver->lID = pRefItem->lMyID;
	// 2-2. set reference from CompletionProc
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));// this block will be freed in CompletionProc.
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = pRefDeliver;
	// 2-3. set reference from DataProc
	stProc.pProc = (LPNKFUNC)DataProc;
	stProc.refProc = (NKREF)pRefDeliver;
	// 2-4. set DataProc as data delivery callback function
	if (CheckCapabilityOperation(pRefDat, kNkMAIDCapability_DataProc, kNkMAIDCapOperation_Set))
	{
		bRet = Command_CapSet(pRefDat->pObject, kNkMAIDCapability_DataProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
		if (bRet == false) return false;
	}
	else
	{
		return false;
	}

	// 3. Acquire image
	bRet = Command_CapStart(pRefDat->pObject, kNkMAIDCapability_Acquire, (LPNKFUNC)CompletionProc, (NKREF)pRefCompletion, &nResult);
	if (bRet == false) return false;

	if (nResult == kNkMAIDResult_NoError)
	{
		// image had read before issuing delete command.
		qDebug() << ("\nInternal ID [0x%08X] had read before issuing delete command.\n", ulItmID);
	}
	else
		if (nResult == kNkMAIDResult_Pending)
		{
			// 4. Async command
			bRet = Command_Async(pRefDat->pObject);
			if (bRet == false) return false;

			// 5. Abort
			bRet = Command_Abort(pRefDat->pObject, NULL, NULL);
			if (bRet == false) return false;

			// 6. Set Item ID
			bRet = Command_CapSet(pRefSrc->pObject, kNkMAIDCapability_CurrentItemID, kNkMAIDDataType_Unsigned, (NKPARAM)ulItmID, NULL, NULL);
			if (bRet == false) return false;

			// 7. Delete DRAM (Delete timing No.2)
			bRet = Command_CapStart(pRefSrc->pObject, kNkMAIDCapability_DeleteDramImage, NULL, NULL, NULL);
			if (bRet == false) return false;

			// 8. Reset DataProc
			bRet = Command_CapSet(pRefDat->pObject, kNkMAIDCapability_DataProc, kNkMAIDDataType_Null, (NKPARAM)NULL, NULL, NULL);
			if (bRet == false) return false;

			qDebug() << ("\nInternal ID [0x%08X] was deleted.\n", ulItmID);
		}

	// Upper function to close ItemObject. 
	g_bFileRemoved = true;
	// progress proc flag reset 
	g_bFirstCall = true;

	// 9. Close ImageObject
	bRet = RemoveChild(pRefItem, kNkMAIDDataObjType_Image);


	return bRet;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Get Live view image
BOOL GetLiveViewImageCapability(LPRefObj pRefSrc)
{
	char	HeaderFileName[256], ImageFileName[256];
	FILE*	hFileHeader = NULL;		// LiveView Image file name
	FILE*	hFileImage = NULL;		// LiveView header file name
	ULONG	ulHeaderSize = 0;		//The header size of LiveView
	NkMAIDArray	stArray;
	int i = 0;
	unsigned char* pucData = NULL;	// LiveView data pointer
	BOOL	bRet = true;


	// Set header size of LiveView
	if (g_ulCameraType == kNkMAIDCameraType_D850)
	{
		ulHeaderSize = 384;
	}

	memset(&stArray, 0, sizeof(NkMAIDArray));

	bRet = GetArrayCapability(pRefSrc, kNkMAIDCapability_GetLiveViewImage, &stArray);
	if (bRet == false) return false;

	// create file name
	while (true)
	{
		qDebug() << (HeaderFileName, "LiveView%03d_H.%s", ++i, "dat");
		qDebug() << (ImageFileName, "LiveView%03d.%s", i, "jpg");
		if ((hFileHeader = fopen(HeaderFileName, "r")) != NULL ||
			(hFileImage = fopen(ImageFileName, "r")) != NULL)
		{
			// this file name is already used.
			if (hFileHeader)
			{
				fclose(hFileHeader);
				hFileHeader = NULL;
			}
			if (hFileImage)
			{
				fclose(hFileImage);
				hFileImage = NULL;
			}
		}
		else
		{
			break;
		}
	}

	// open file
	hFileHeader = fopen(HeaderFileName, "wb");
	if (hFileHeader == NULL)
	{
		qDebug() << ("file open error.\n");
		return false;
	}
	hFileImage = fopen(ImageFileName, "wb");
	if (hFileImage == NULL)
	{
		fclose(hFileHeader);
		qDebug() << ("file open error.\n");
		return false;
	}

	// Get data pointer
	pucData = (unsigned char*)stArray.pData;

	// write file
	fwrite(pucData, 1, ulHeaderSize, hFileHeader);
	fwrite(pucData + ulHeaderSize, 1, (stArray.ulElements - ulHeaderSize), hFileImage);
	qDebug() << ("\n%s was saved.\n", HeaderFileName);
	qDebug() << ("%s was saved.\n", ImageFileName);

	// close file
	fclose(hFileHeader);
	fclose(hFileImage);
	free(stArray.pData);

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Set/Get PictureControlDataCapability
BOOL PictureControlDataCapability(LPRefObj pRefSrc, ULONG ulCapID)
{
	char buf[256], filename[256];
	NkMAIDPicCtrlData stPicCtrlData;
	ULONG	ulSel, ulSubSel;
	BOOL	bRet = true;

	strcpy(filename, "PicCtrlData.dat");
	// sub command loop
	do {
		memset(&stPicCtrlData, 0, sizeof(NkMAIDPicCtrlData));

		qDebug() << ("\nSelect (1-2, 0)\n");
		qDebug() << (" 1. Set Picture Control data the file named \"PicCtrlData.dat\"\n");
		qDebug() << (" 2. Get Picture Control data\n");
		qDebug() << (" 0. Exit\n>");
		scanf("%s", buf);
		ulSel = (ULONG)atol(buf);
		switch (ulSel)
		{
		case 1://Set Picture Control data 
		{
			qDebug() << ("\nSelect Picture Control(1-17, 0)\n");
			qDebug() << (" 1. Standard                    2. Neutral\n");
			qDebug() << (" 3. Vivid                       4. Monochrome\n");
			qDebug() << (" 5. Portrait                    6. Landscape\n");
			qDebug() << (" 7. Flat                        8. Auto\n");
			qDebug() << (" 9. Custom Picture Contol 1    10. Custom Picture Contol 2 \n");
			qDebug() << ("11. Custom Picture Contol 3    12. Custom Picture Contol 4 \n");
			qDebug() << ("13. Custom Picture Contol 5    14. Custom Picture Contol 6 \n");
			qDebug() << ("15. Custom Picture Contol 7    16. Custom Picture Contol 8 \n");
			qDebug() << ("17. Custom Picture Contol 9\n");
			qDebug() << (" 0. Exit\n>");
			scanf("%s", buf);
			ulSubSel = atoi(buf);
			if (ulSubSel == 0) break; //Exit
			if (ulSubSel < 1 || ulSubSel > 17)
			{
				qDebug() << ("Invalid Picture Control\n");
				break;
			}
			if (ulSubSel >= 9)
			{
				ulSubSel += 192; // Custom 201 - 209
			}
			// set target Picture Control
			stPicCtrlData.ulPicCtrlItem = ulSubSel;

			// initial registration is not supported about 1-8
			if ((stPicCtrlData.ulPicCtrlItem >= 1 && stPicCtrlData.ulPicCtrlItem <= 8))
			{
				qDebug() << ("\nSelect ModifiedFlag (1, 0)\n");
				qDebug() << (" 1. edit\n");
				qDebug() << (" 0. Exit\n>");
				scanf("%s", buf);
				ulSubSel = atoi(buf);
				if (ulSubSel == 0) break; // Exit
				if (ulSubSel < 1 || ulSubSel > 1)
				{
					qDebug() << ("Invalid ModifiedFlag\n");
					break;
				}
				// set Modification flas
				stPicCtrlData.bModifiedFlag = true;
			}
			else
			{
				qDebug() << ("\nSelect ModifiedFlag (1-2, 0)\n");
				qDebug() << (" 1. initial registration          2. edit\n");
				qDebug() << (" 0. Exit\n>");
				scanf("%s", buf);
				ulSubSel = atoi(buf);
				if (ulSubSel == 0) break; // Exit
				if (ulSubSel < 1 || ulSubSel > 2)
				{
					qDebug() << ("Invalid ModifiedFlag\n");
					break;
				}
				// set Modification flas
				stPicCtrlData.bModifiedFlag = (ulSubSel == 1) ? false : true;
			}

			bRet = SetPictureControlDataCapability(pRefSrc, &stPicCtrlData, filename, ulCapID);
			break;

		case 2://Get Picture Control data
			qDebug() << ("\nSelect Picture Control(1-17, 0)\n");
			qDebug() << (" 1. Standard                    2. Neutral\n");
			qDebug() << (" 3. Vivid                       4. Monochrome\n");
			qDebug() << (" 5. Portrait                    6. Landscape\n");
			qDebug() << (" 7. Flat                        8. Auto\n");
			qDebug() << (" 9. Custom Picture Contol 1    10. Custom Picture Contol 2 \n");
			qDebug() << ("11. Custom Picture Contol 3    12. Custom Picture Contol 4 \n");
			qDebug() << ("13. Custom Picture Contol 5    14. Custom Picture Contol 6 \n");
			qDebug() << ("15. Custom Picture Contol 7    16. Custom Picture Contol 8 \n");
			qDebug() << ("17. Custom Picture Contol 9\n");
			qDebug() << (" 0. Exit\n>");
			scanf("%s", buf);
			ulSubSel = atoi(buf);
			if (ulSubSel == 0) break; //Exit
			if (ulSubSel < 1 || ulSubSel > 17)
			{
				qDebug() << ("Invalid Picture Control\n");
				break;
			}

			if (ulSubSel >= 9)
			{
				ulSubSel += 192; // Custom 201 - 209
			}
			// set target Picture Control
			stPicCtrlData.ulPicCtrlItem = ulSubSel;

			bRet = GetPictureControlDataCapability(pRefSrc, &stPicCtrlData, ulCapID);
			break;
		default:
			ulSel = 0;
		}
		}
		if (bRet == false)
		{
			qDebug() << ("An Error occured. \n");
		}
	} while (ulSel > 0);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Set PictureControlDataCapability
BOOL SetPictureControlDataCapability(LPRefObj pRefObj, NkMAIDPicCtrlData* pPicCtrlData, char* filename, ULONG ulCapID)
{
	BOOL	bRet = TRUE;
	FILE	*stream;
	ULONG	count = 0;
	ULONG   ulTotal = 0;
	char	*ptr = NULL;

	if (ulCapID == kNkMAIDCapability_PictureControlData)
	{
		LPNkMAIDCapInfo	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_PictureControlData);
		if (pCapInfo == NULL) return false;

		// check data type of the capability
		if (pCapInfo->ulType != kNkMAIDCapType_Generic) return false;
		// check if this capability supports CapSet operation.
		if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_PictureControlData, kNkMAIDCapOperation_Set)) return false;
	}
	else
	{
		LPNkMAIDCapInfo	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_PictureControlDataEx);
		if (pCapInfo == NULL) return false;

		// check data type of the capability
		if (pCapInfo->ulType != kNkMAIDCapType_Generic) return false;
		// check if this capability supports CapSet operation.
		if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_PictureControlDataEx, kNkMAIDCapOperation_Set)) return false;
	}

	// Read preset data from file.
	if ((stream = fopen(filename, "rb")) == NULL)
	{
		qDebug() << ("\nfile open error.\n");
		return false;
	}

	if (ulCapID == kNkMAIDCapability_PictureControlData)
	{
		// Max Picture Control data size is 609
		// Allocate memory for Picture Control data.
		pPicCtrlData->pData = (char*)malloc(609);
		ptr = (char*)pPicCtrlData->pData;
		while (!feof(stream))
		{
			// Read file until eof.
			count = (ULONG)fread(ptr, sizeof(char), 100, stream);
			if (ferror(stream))
			{
				qDebug() << ("\nfile read error.\n");
				fclose(stream);
				free(pPicCtrlData->pData);
				return false;
			}
			/* Total count up actual bytes read */
			ulTotal += count;
			ptr += count;
			if (609 < ulTotal)
			{

				qDebug() << ("\nThe size of \"PicCtrlData.dat\" is over 609 byte.\n");
				fclose(stream);
				free(pPicCtrlData->pData);
				return false;
			}
		}
	}
	else
	{
		// Max Picture Control data size is 610
		// Allocate memory for Picture Control dataEX.
		pPicCtrlData->pData = (char*)malloc(610);
		ptr = (char*)pPicCtrlData->pData;
		while (!feof(stream))
		{
			// Read file until eof.
			count = (ULONG)fread(ptr, sizeof(char), 100, stream);
			if (ferror(stream))
			{
				qDebug() << ("\nfile read error.\n");
				fclose(stream);
				free(pPicCtrlData->pData);
				return false;
			}
			/* Total count up actual bytes read */
			ulTotal += count;
			ptr += count;
			if (610 < ulTotal)
			{

				qDebug() << ("\nThe size of \"PicCtrlData.dat\" is over 610 byte.\n");
				fclose(stream);
				free(pPicCtrlData->pData);
				return false;
			}
		}
	}
	pPicCtrlData->ulSize = ulTotal;

	if (ulCapID == kNkMAIDCapability_PictureControlData)
	{
		// Set Picture Control data.
		bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_PictureControlData, kNkMAIDDataType_GenericPtr, (NKPARAM)pPicCtrlData, NULL, NULL);
		if (bRet == false)
		{
			qDebug() << ("\nFailed in setting Picture Contol Data.\n");
		}
	}
	else
	{
		// Set Picture Control data.
		bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_PictureControlDataEx, kNkMAIDDataType_GenericPtr, (NKPARAM)pPicCtrlData, NULL, NULL);
		if (bRet == false)
		{
			qDebug() << ("\nFailed in setting Picture Contol DataEx.\n");
		}
	}

	fclose(stream);
	free(pPicCtrlData->pData);

	return bRet;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Get PictureControlDataCapability
BOOL GetPictureControlDataCapability(LPRefObj pRefObj, NkMAIDPicCtrlData* pPicCtrlData, ULONG ulCapID)
{
	BOOL	bRet = TRUE;
	FILE	*stream = NULL;
	unsigned char* pucData = NULL;	// Picture Control Data pointer

	if (ulCapID == kNkMAIDCapability_PictureControlData)
	{
		LPNkMAIDCapInfo	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_PictureControlData);
		if (pCapInfo == NULL) return false;

		// check data type of the capability
		if (pCapInfo->ulType != kNkMAIDCapType_Generic) return false;
		// check if this capability supports CapGet operation.
		if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_PictureControlData, kNkMAIDCapOperation_Get)) return false;
	}
	else
	{
		LPNkMAIDCapInfo	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_PictureControlDataEx);
		if (pCapInfo == NULL) return false;

		// check data type of the capability
		if (pCapInfo->ulType != kNkMAIDCapType_Generic) return false;
		// check if this capability supports CapGet operation.
		if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_PictureControlDataEx, kNkMAIDCapOperation_Get)) return false;
	}

	// Max Picture Control data size is 610
	// Allocate memory for Picture Control data.
	pPicCtrlData->ulSize = 610;
	pPicCtrlData->pData = (char*)malloc(610);

	if (ulCapID == kNkMAIDCapability_PictureControlData)
	{
		// Get Picture Control data.
		bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_PictureControlData, kNkMAIDDataType_GenericPtr, (NKPARAM)pPicCtrlData, NULL, NULL);
		if (bRet == false)
		{
			qDebug() << ("\nFailed in getting Picture Control Data.\n");
			free(pPicCtrlData->pData);
			return false;
		}
	}
	else
	{
		// Get Picture Control dataEx.
		bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_PictureControlDataEx, kNkMAIDDataType_GenericPtr, (NKPARAM)pPicCtrlData, NULL, NULL);
		if (bRet == false)
		{
			qDebug() << ("\nFailed in getting Picture Control DataEx.\n");
			free(pPicCtrlData->pData);
			return false;
		}
	}

	// Save to file
	// open file
	stream = fopen("PicCtrlData.dat", "wb");
	if (stream == NULL)
	{
		qDebug() << ("\nfile open error.\n");
		free(pPicCtrlData->pData);
		return false;
	}

	// Get data pointer
	pucData = (unsigned char*)pPicCtrlData->pData;

	// write file
	fwrite(pucData, 1, pPicCtrlData->ulSize, stream);
	qDebug() << ("\nPicCtrlData.dat was saved.\n");

	// close file
	fclose(stream);
	free(pPicCtrlData->pData);

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Get PictureControlInfoCapability
BOOL GetPictureControlInfoCapability(LPRefObj pRefSrc)
{
	char buf[256], filename[256];
	NkMAIDGetPicCtrlInfo stPicCtrlInfo;
	ULONG	ulSel;
	LPNkMAIDCapInfo	pCapInfo = NULL;
	FILE* stream = NULL;
	unsigned char* pucData = NULL;	// Picture Control Info pointer
	BOOL	bRet = true;


	strcpy(filename, "PicCtrlInfo.dat");

	memset(&stPicCtrlInfo, 0, sizeof(NkMAIDGetPicCtrlInfo));

	qDebug() << ("\nSelect Picture Control(1-17, 0)\n");
	qDebug() << (" 1. Standard                    2. Neutral\n");
	qDebug() << (" 3. Vivid                       4. Monochrome\n");
	qDebug() << (" 5. Portrait                    6. Landscape\n");
	qDebug() << (" 7. Flat                        8. Auto\n");
	qDebug() << (" 9. Custom Picture Contol 1    10. Custom Picture Contol 2 \n");
	qDebug() << ("11. Custom Picture Contol 3    12. Custom Picture Contol 4 \n");
	qDebug() << ("13. Custom Picture Contol 5    14. Custom Picture Contol 6 \n");
	qDebug() << ("15. Custom Picture Contol 7    16. Custom Picture Contol 8 \n");
	qDebug() << ("17. Custom Picture Contol 9\n");
	qDebug() << (" 0. Exit\n>");
	scanf("%s", buf);
	ulSel = atoi(buf);
	if (ulSel == 0) return true; // Exit
	if (ulSel < 1 || ulSel > 17)
	{
		qDebug() << ("Invalid Picture Control\n");
		return false;
	}

	if (ulSel >= 9)
	{
		ulSel += 192; // Custom 201 - 209
	}
	// set target Picture Control
	stPicCtrlInfo.ulPicCtrlItem = ulSel;


	pCapInfo = GetCapInfo(pRefSrc, kNkMAIDCapability_GetPicCtrlInfo);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefSrc, kNkMAIDCapability_GetPicCtrlInfo, kNkMAIDCapOperation_Get)) return false;

	// Max Picture Control info size is 39
	// Allocate memory for Picture Control info.
	stPicCtrlInfo.ulSize = 39;
	stPicCtrlInfo.pData = (char*)malloc(39);

	// Get Picture Control info.
	bRet = Command_CapGet(pRefSrc->pObject, kNkMAIDCapability_GetPicCtrlInfo, kNkMAIDDataType_GenericPtr, (NKPARAM)&(stPicCtrlInfo), NULL, NULL);
	if (bRet == false)
	{
		free(stPicCtrlInfo.pData);
		return false;
	}

	// Save to file
	// open file
	stream = fopen("PicCtrlInfo.dat", "wb");
	if (stream == NULL)
	{
		qDebug() << ("\nfile open error.\n");
		free(stPicCtrlInfo.pData);
		return false;
	}

	// Get data pointer
	pucData = (unsigned char*)stPicCtrlInfo.pData;

	// write file
	fwrite(pucData, 1, stPicCtrlInfo.ulSize, stream);
	qDebug() << ("\nPicCtrlInfo.dat was saved.\n");

	// close file
	fclose(stream);
	free(stPicCtrlInfo.pData);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Delete Custom Picture Control
BOOL DeleteCustomPictureControlCapability(LPRefObj pRefSrc)
{
	ULONG	ulSel = 0;
	BOOL	bRet;
	ULONG	ulValue;
	char	buf[256];


	qDebug() << ("\nSelect Custom Picture Control to delete.(1-9, 0)\n");

	qDebug() << ("1. Custom Picture Contol 1\n");
	qDebug() << ("2. Custom Picture Contol 2\n");
	qDebug() << ("3. Custom Picture Contol 3\n");
	qDebug() << ("4. Custom Picture Contol 4\n");
	qDebug() << ("5. Custom Picture Contol 5\n");
	qDebug() << ("6. Custom Picture Contol 6\n");
	qDebug() << ("7. Custom Picture Contol 7\n");
	qDebug() << ("8. Custom Picture Contol 8\n");
	qDebug() << ("9. Custom Picture Contol 9\n");
	qDebug() << ("0. Exit\n>");
	scanf("%s", buf);

	ulSel = atoi(buf);
	if (ulSel == 0) return true; // Exit
	if (ulSel < 1 || ulSel > 9)
	{
		qDebug() << ("Invalid Custom Picture Control\n");
		return false;
	}
	ulSel += 200;		// Custom 201 - 209
	ulValue = ulSel;	// Set Custom Picture Control to delete

	bRet = Command_CapSet(pRefSrc->pObject, kNkMAIDCapability_DeleteCustomPictureControl, kNkMAIDDataType_Unsigned, (NKPARAM)ulValue, NULL, NULL);
	return bRet;
}

//------------------------------------------------------------------------------------------------------------------------------------
// Get SBHandlesCapability
BOOL GetSBHandlesCapability(LPRefObj pRefObj)
{
	BOOL					bRet = true;
	char					buf[256] = { 0 };
	ULONG					ulSel;
	NkMAIDGetSBHandles		stSbHandles;
	LPNkMAIDCapInfo			pCapInfo = NULL;
	FILE					*fileStream;

	// To clear the structure.
	memset(&stSbHandles, 0, sizeof(NkMAIDGetSBHandles));

	// To display the menu.
	qDebug() << ("\nSelect SBGroupID(1-8, 0)\n");
	qDebug() << (" 1. SBGroupID ALL\n");
	qDebug() << (" 2. SBGroupID Master\n");
	qDebug() << (" 3. SBGroupID A\n");
	qDebug() << (" 4. SBGroupID B\n");
	qDebug() << (" 5. SBGroupID C\n");
	qDebug() << (" 6. SBGroupID D\n");
	qDebug() << (" 7. SBGroupID E\n");
	qDebug() << (" 8. SBGroupID F\n");
	qDebug() << (" 0. Exit\n>");
	scanf("%s", buf);
	ulSel = (ULONG)atoi(buf);

	// Check the input value.
	if ((ulSel == 0) || (ulSel > 8))
	{
		return true;
	}

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_GetSBHandles);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_GetSBHandles, kNkMAIDCapOperation_Get))
	{
		return false;
	}

	// 1.To get the SB handle the total number.
	{
		// ulSBGroupID：To specify the SBGroupID to be acquisition target.
		switch (ulSel)
		{
			// SBGroupID ALL
		case 1:
			stSbHandles.ulSBGroupID = kNkMAIDSBGroupID_ALL;
			break;
			// SBGroupID Master
		case 2:
			stSbHandles.ulSBGroupID = kNkMAIDSBGroupID_Master;
			break;
			// SBGroupID A
		case 3:
			stSbHandles.ulSBGroupID = kNkMAIDSBGroupID_A;
			break;
			// SBGroupID B
		case 4:
			stSbHandles.ulSBGroupID = kNkMAIDSBGroupID_B;
			break;
			// SBGroupID C
		case 5:
			stSbHandles.ulSBGroupID = kNkMAIDSBGroupID_C;
			break;
			// SBGroupID D
		case 6:
			stSbHandles.ulSBGroupID = kNkMAIDSBGroupID_D;
			break;
			// SBGroupID E
		case 7:
			stSbHandles.ulSBGroupID = kNkMAIDSBGroupID_E;
			break;
			// SBGroupID F
		case 8:
			stSbHandles.ulSBGroupID = kNkMAIDSBGroupID_F;
			break;
		default:
			stSbHandles.ulSBGroupID = 0;
			break;
		}

		// ulNumber：0 is specified.
		stSbHandles.ulNumber = 0;

		// To get the SB handle the total number.
		bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_GetSBHandles, kNkMAIDDataType_GenericPtr, (NKPARAM)&stSbHandles, NULL, NULL);
		if (bRet == false)
		{
			// abnomal.
			qDebug() << ("\nFailed in getting SBHandles.\n");

			return false;
		}
	}

	// 2.To get the SB handle list .
	{
		// ulSBGroupID：To specify the SBGroupID to be acquisition target. ( Already set )
		// ulNumber：The total number of SBHandle to be an acquisition target. ( Already set )
		// ulSize：Buffer Isaizu for the acquisition to SBHandle list set to pData. ( Already set )
		// pData：Data pointer for the acquisition to SBHandle list.
		// To ensure the memory area.
		stSbHandles.pData = (char *)malloc(stSbHandles.ulSize);
		if (stSbHandles.pData == NULL)
		{
			// abnomal.
			qDebug() << ("\nMemory allocation error.\n");

			return false;
		}

		// To get the SB handle list .
		bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_GetSBHandles, kNkMAIDDataType_GenericPtr, (NKPARAM)&stSbHandles, NULL, NULL);
		if (bRet == false)
		{
			// abnomal.
			qDebug() << ("\nFailed in getting SBHandles.\n");
			free(stSbHandles.pData);

			return false;
		}
	}

	// Save to file
	// open file
	fileStream = fopen("SBHandles.dat", "wb");
	if (fileStream == NULL)
	{
		// abnomal.
		qDebug() << ("\nfile open error.\n");
		free(stSbHandles.pData);

		return false;
	}

	// Get data pointer
	unsigned char* pucData = NULL;
	pucData = (unsigned char*)stSbHandles.pData;

	// write file
	fwrite(pucData, 1, stSbHandles.ulSize, fileStream);
	qDebug() << ("\nSBHandles.dat was saved.\n");

	// close file
	fclose(fileStream);
	free(stSbHandles.pData);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Get SBAttrDesc
BOOL GetSBAttrDescCapability(LPRefObj pRefObj)
{
	BOOL					bRet = true;
	char					buf[256] = { 0 };
	ULONG					ulSBHandleMenu = 0;
	ULONG					ulSBHandle = 0;
	ULONG					ulSBAttrID = 0;
	FILE					*fileStream;
	NkMAIDGetSBAttrDesc		stSbAttrDesc;
	LPNkMAIDCapInfo			pCapInfo = NULL;
	SLONG nResult = kNkMAIDResult_NoError;

	// To clear the structure.
	memset(&stSbAttrDesc, 0, sizeof(NkMAIDGetSBAttrDesc));

	// To display the menu.
	qDebug() << ("\nSelect SBHandle Menu(0-1, 2)\n");
	qDebug() << (" 0. All\n");
	qDebug() << (" 1. Any value\n");
	qDebug() << (" 2. Exit\n>");
	scanf("%s", buf);
	ulSBHandleMenu = (ULONG)atoi(buf);

	// Check the input value.
	if (ulSBHandleMenu >= 2)
	{
		return true;
	}
	else if (ulSBHandleMenu == 1)
	{
		// To display the menu.
		qDebug() << ("\nEnter SBHandle Value(Hex)\n");
		qDebug() << ("0x");
		scanf("%s", buf);

		char *pcErrbuf;
		ulSBHandle = (ULONG)strtol(buf, &pcErrbuf, 16);
		if (*pcErrbuf != '\0')
		{
			return true;
		}
	}
	else
	{
		ulSBHandle = kNkMAIDSBHandle_ALL;
	}

	// To display the menu.
	qDebug() << ("\nSelect SB Attribute(1-5, 0)\n");
	qDebug() << (" 1. SB Attribute All\n");
	qDebug() << (" 2. SB Attribute Name\n");
	qDebug() << (" 3. SB Attribute GroupID\n");
	qDebug() << (" 4. SB Attribute Status\n");
	qDebug() << (" 5. SB Attribute TestFlashDisable\n");
	qDebug() << (" 0. Exit\n>");
	scanf("%s", buf);
	ulSBAttrID = (ULONG)atoi(buf);

	// Check the input value.
	if ((ulSBAttrID == 0) || (ulSBAttrID > 5))
	{
		return true;
	}

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_GetSBAttrDesc);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_GetSBAttrDesc, kNkMAIDCapOperation_Get))
	{
		return false;
	}

	// 1.To get the size of the order of the SB attribute descriptor acquisition.
	{
		// ulSBHandle：To specify the SBHandle to be acquisition target.
		stSbAttrDesc.ulSBHandle = ulSBHandle;

		// ulSBAttrID：To specify the SBAttrID to be acquisition target.
		switch (ulSBAttrID)
		{
		case 1:
			// SBAttribute_ALL
			stSbAttrDesc.ulSBAttrID = kNkMAIDSBAttribute_ALL;
			break;
		case 2:
			// SBAttribute_Name
			stSbAttrDesc.ulSBAttrID = kNkMAIDSBAttribute_Name;
			break;
		case 3:
			// SBAttribute_GroupID
			stSbAttrDesc.ulSBAttrID = kNkMAIDSBAttribute_GroupID;
			break;
		case 4:
			// SBAttribute_Status
			stSbAttrDesc.ulSBAttrID = kNkMAIDSBAttribute_Status;
			break;
		case 5:
			// SBAttribute_TestFlashDisable
			stSbAttrDesc.ulSBAttrID = kNkMAIDSBAttribute_TestFlashDisable;
			break;
		default:
			stSbAttrDesc.ulSBAttrID = 0;
			break;
		}

		// ulSize：0 is specified.
		stSbAttrDesc.ulSize = 0;

		// To get the size of the order of the SB attribute descriptor acquisition.
		bRet = Command_CapGetSB(pRefObj->pObject, kNkMAIDCapability_GetSBAttrDesc, kNkMAIDDataType_GenericPtr, (NKPARAM)&stSbAttrDesc, NULL, NULL, &nResult);
		if (bRet == false)
		{
			if (nResult == kNkMAIDResult_UnexpectedError)
			{
				qDebug() << ("\nNot supported SBAttrDesc.\n");
				return true;
			}
			else
			{
				// abnomal.
				qDebug() << ("\nFailed in getting SBAttrDesc.\n");

				return false;
			}
		}
	}

	// 2.To get the SB attribute descriptor.
	{
		// ulSBHandle：To specify the SBHandle to be acquisition target. ( Already set )
		// ulSBAttrID：To specify the SBAttrID to be acquisition target. ( Already set )
		// ulSize：Data size for SBHandleAttributeDescList acquired set to pData. ( Already set )
		// pData：Data pointer for SBHandleAttributeDescList acquisition.
		stSbAttrDesc.pData = (char *)malloc(stSbAttrDesc.ulSize);
		if (stSbAttrDesc.pData == NULL)
		{
			// abnomal.
			qDebug() << ("\nMemory allocation error.\n");

			return false;
		}

		// To get the SB attribute descriptor.
		nResult = kNkMAIDResult_NoError;
		bRet = Command_CapGetSB(pRefObj->pObject, kNkMAIDCapability_GetSBAttrDesc, kNkMAIDDataType_GenericPtr, (NKPARAM)&stSbAttrDesc, NULL, NULL, &nResult);
		if (bRet == false)
		{
			free(stSbAttrDesc.pData);
			if (nResult == kNkMAIDResult_UnexpectedError)
			{
				qDebug() << ("\nNot supported SBAttrDesc.\n");
				return true;
			}
			else
			{
				// abnomal.
				qDebug() << ("\nFailed in getting SBAttrDesc.\n");
				return false;
			}
		}
	}

	// Save to file
	// open file
	fileStream = fopen("SBAttrDesc.dat", "wb");
	if (fileStream == NULL)
	{
		// abnomal.
		qDebug() << ("\nFile open error.\n");
		free(stSbAttrDesc.pData);

		return false;
	}

	// Get data pointer
	unsigned char* pucData = NULL;
	pucData = (unsigned char*)stSbAttrDesc.pData;

	// write file
	fwrite(pucData, 1, stSbAttrDesc.ulSize, fileStream);
	qDebug() << ("\nSBAttrDesc.dat was saved.\n");

	// close file
	fclose(fileStream);
	free(stSbAttrDesc.pData);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Set/Get SBAttrValueCapability
BOOL SBAttrValueCapability(LPRefObj pRefObj)
{
	BOOL					bRet = true;
	char					buf[256] = { 0 };
	ULONG					ulSel;
	NkMAIDSBAttrValue		stSbAttrValue;

	do
	{
		// To clear the structure.
		memset(&stSbAttrValue, 0, sizeof(NkMAIDSBAttrValue));

		// To display the menu.
		qDebug() << ("\nSelect (1-2, 0)\n");
		qDebug() << (" 1. Set SB Attribute\n");
		qDebug() << (" 2. Get SB Attribute\n");
		qDebug() << (" 0. Exit\n>");
		scanf("%s", buf);
		ulSel = (ULONG)atol(buf);
		switch (ulSel)
		{
		case 0:
			// Exit.
			break;
		case 1:
			// Set SBAttrValue.
			bRet = SetSBAttrValueCapability(pRefObj, &stSbAttrValue);
			break;
		case 2:
			// GetSBAttrValue.
			bRet = GetSBAttrValueCapability(pRefObj, &stSbAttrValue);
			break;
		default:
			break;
		}

		if (bRet == false)
		{
			// abnomal.
			qDebug() << ("An Error occured. \n");
		}
	} while (ulSel != 0);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Set SBAttrValueCapability
BOOL SetSBAttrValueCapability(LPRefObj pRefObj, NkMAIDSBAttrValue *pstSbAttrValue)
{
	BOOL				bRet = false;
	char				cFileName[256] = { 0 };
	char				*pData = NULL;
	ULONG				ulCount = 0;
	ULONG				ulTotal = 0;
	LPNkMAIDCapInfo		pCapInfo = NULL;
	FILE				*fileStream;

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_SBAttrValue);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapSet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_SBAttrValue, kNkMAIDCapOperation_Set))
	{
		return false;
	}

	// ulSBHandle：Not use the Set.
	// ulSBAttrID：Not use the Set.

	// Open the file
	strcpy(cFileName, "SBAttrValue.dat");
	fileStream = fopen(cFileName, "rb");
	if (fileStream == NULL)
	{
		// abnomal.
		qDebug() << ("\nFile open error.\n");

		return false;
	}

	// To get the file size.
	struct stat stStatBuf;
#if defined( _WIN32 )
	if (fstat(_fileno(fileStream), &stStatBuf) == -1)
#elif defined(__APPLE__)
	if (fstat(fileno(fileStream), &stStatBuf) == -1)
#endif
	{
		// abnomal.
		qDebug() << ("\nFile status error.\n");
		fclose(fileStream);

		return false;
	}
	// Data size of SBAttributeList set to pData.
	pstSbAttrValue->ulSize = (ULONG)stStatBuf.st_size;

	// pData：A pointer to the SBAttributeList.
	pstSbAttrValue->pData = (char*)malloc(pstSbAttrValue->ulSize);
	if (pstSbAttrValue->pData == NULL)
	{
		// abnomal.
		qDebug() << ("\nMemory allocation error.\n");
		fclose(fileStream);

		return false;
	}
	pData = (char *)pstSbAttrValue->pData;

	// Read the file.
	while (!feof(fileStream))
	{
		// Read file until eof.
		ulCount = (ULONG)fread(pData, sizeof(char), 100, fileStream);
		if (ferror(fileStream))
		{
			// abnomal.
			qDebug() << ("\nFile read error.\n");
			fclose(fileStream);
			free(pstSbAttrValue->pData);

			return false;
		}

		// Total count up actual bytes read.
		ulTotal += ulCount;
		pData += ulCount;
		if (pstSbAttrValue->ulSize < ulTotal)
		{
			qDebug() << ("\nThe size of \"SBAttrValue.dat\" is size over.\n");
			fclose(fileStream);
			free(pstSbAttrValue->pData);
			return false;
		}
	}

	// Set SBAttrValue.
	SLONG nResult = kNkMAIDResult_NoError;
	bRet = Command_CapSetSB(pRefObj->pObject, kNkMAIDCapability_SBAttrValue, kNkMAIDDataType_GenericPtr, (NKPARAM)pstSbAttrValue, NULL, NULL, &nResult);
	if (bRet == false)
	{
		fclose(fileStream);
		free(pstSbAttrValue->pData);
		if (nResult == kNkMAIDResult_UnexpectedError)
		{
			qDebug() << ("\nNot supported SB Attribute Value.\n");
			return true;
		}
		else
		{
			// abnomal.
			qDebug() << ("\nFailed in setting SB Attribute Value.\n");
			return false;
		}
	}

	qDebug() << ("\nSucceed in setting SB Attribute Value.\n");

	fclose(fileStream);
	free(pstSbAttrValue->pData);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Get SBAttrValueCapability
BOOL GetSBAttrValueCapability(LPRefObj pRefObj, NkMAIDSBAttrValue *pstSbAttrValue)
{
	BOOL				bRet = true;
	char				buf[256] = { 0 };
	ULONG				ulSBHandleMenu = 0;
	ULONG				ulSBHandle = 0;
	ULONG				ulSBAttrID = 0;
	FILE				*fileStream;
	LPNkMAIDCapInfo		pCapInfo = NULL;
	SLONG nResult = kNkMAIDResult_NoError;

	// To display the menu.
	qDebug() << ("\nSelect SBHandle Menu(0-1, 2)\n");
	qDebug() << (" 0. All\n");
	qDebug() << (" 1. Any value\n");
	qDebug() << (" 2. Exit\n>");
	scanf("%s", buf);
	ulSBHandleMenu = (ULONG)atoi(buf);

	// Check the input value.
	if (ulSBHandleMenu >= 2)
	{
		return true;
	}
	else if (ulSBHandleMenu == 1)
	{
		// To display the menu.
		qDebug() << ("\nEnter SBHandle Value(Hex)\n");
		qDebug() << ("0x");
		scanf("%s", buf);

		char *pcErrbuf;
		ulSBHandle = (ULONG)strtol(buf, &pcErrbuf, 16);
		if (*pcErrbuf != '\0')
		{
			return true;
		}
	}
	else
	{
		ulSBHandle = kNkMAIDSBHandle_ALL;
	}

	// To display the menu.
	qDebug() << ("\nSelect SBAttribute(1-5, 0)\n");
	qDebug() << (" 1. SBAttribute ALL\n");
	qDebug() << (" 2. SBAttribute Name\n");
	qDebug() << (" 3. SBAttribute GroupID\n");
	qDebug() << (" 4. SBAttribute Status\n");
	qDebug() << (" 5. SBAttribute TestFlashDisable\n");
	qDebug() << (" 0. Exit\n>");
	scanf("%s", buf);
	ulSBAttrID = (ULONG)atoi(buf);

	// Check the input value.
	if ((ulSBAttrID == 0) || (ulSBAttrID > 5))
	{
		return true;
	}

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_SBAttrValue);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_SBAttrValue, kNkMAIDCapOperation_Get))
	{
		return false;
	}

	// 1.To get the size in order to obtain the current value of the SB attribute.
	{
		// ulSBHandle：To specify the SBHandle to be acquisition target.
		pstSbAttrValue->ulSBHandle = ulSBHandle;

		// ulSBAttrID：To specify the SBAttrID to be acquisition target.
		switch (ulSBAttrID)
		{
		case 1:
			// SBAttribute_ALL
			pstSbAttrValue->ulSBAttrID = kNkMAIDSBAttribute_ALL;
			break;
		case 2:
			// SBAttribute_Name
			pstSbAttrValue->ulSBAttrID = kNkMAIDSBAttribute_Name;
			break;
		case 3:
			// SBAttribute_GroupID
			pstSbAttrValue->ulSBAttrID = kNkMAIDSBAttribute_GroupID;
			break;
		case 4:
			// SBAttribute_Status
			pstSbAttrValue->ulSBAttrID = kNkMAIDSBAttribute_Status;
			break;
		case 5:
			// SBAttribute_TestFlashDisable
			pstSbAttrValue->ulSBAttrID = kNkMAIDSBAttribute_TestFlashDisable;
			break;
		default:
			pstSbAttrValue->ulSBAttrID = 0;
			break;
		}

		// ulSize：0 is specified.
		pstSbAttrValue->ulSize = 0;

		// To get the size in order to obtain the current value of the SB attribute.
		bRet = Command_CapGetSB(pRefObj->pObject, kNkMAIDCapability_SBAttrValue, kNkMAIDDataType_GenericPtr, (NKPARAM)pstSbAttrValue, NULL, NULL, &nResult);
		if (bRet == false)
		{
			if (nResult == kNkMAIDResult_UnexpectedError)
			{
				qDebug() << ("\nNot supported SB Attribute Value.\n");
				return true;
			}
			else
			{
				// abnomal.
				qDebug() << ("\nFailed in getting SB Attribute Value.\n");

				return false;
			}
		}
	}

	// 2.To get the current value of the SB attribute.
	{
		// ulSBHandle：To specify the SBHandle to be acquisition target. ( Already set )
		// ulSBAttrID：To specify the SBAttr to be acquisition target. ( Already set )
		// ulSize：Size of SBHandleAttributeDescList set to pData. ( Already set )
		// pData：Data pointer for SBHandleAttributeDescList acquisition.
		pstSbAttrValue->pData = (char*)malloc(pstSbAttrValue->ulSize);
		if (pstSbAttrValue->pData == NULL)
		{
			// abnomal.
			qDebug() << ("\nMemory allocation error.\n");

			return false;
		}

		// To get the current value of the SB attribute.
		nResult = kNkMAIDResult_NoError;
		bRet = Command_CapGetSB(pRefObj->pObject, kNkMAIDCapability_SBAttrValue, kNkMAIDDataType_GenericPtr, (NKPARAM)pstSbAttrValue, NULL, NULL, &nResult);
		if (bRet == false)
		{
			free(pstSbAttrValue->pData);
			if (nResult == kNkMAIDResult_UnexpectedError)
			{
				qDebug() << ("\nNot supported SB Attribute Value.\n");
				return true;
			}
			else
			{
				// abnomal.
				qDebug() << ("\nFailed in getting SB Attribute Value.\n");
				return false;
			}
		}
	}

	// Save to file
	// open file
	fileStream = fopen("SBAttrValue.dat", "wb");
	if (fileStream == NULL)
	{
		// abnomal.
		qDebug() << ("\nFile open error.\n");
		free(pstSbAttrValue->pData);

		return false;
	}

	// Get data pointer
	unsigned char* pucData = NULL;
	pucData = (unsigned char*)pstSbAttrValue->pData;

	// write file
	fwrite(pucData, 1, pstSbAttrValue->ulSize, fileStream);
	qDebug() << ("\nSBAttrValue.dat was saved.\n");

	// close file
	fclose(fileStream);
	free(pstSbAttrValue->pData);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Get SBGroupAttrDescCapability
BOOL GetSBGroupAttrDescCapability(LPRefObj pRefObj)
{
	BOOL						bRet = true;
	char						buf[256] = { 0 };
	ULONG						ulSBGroupID;
	ULONG						ulSBGroupAttrID;
	FILE						*fileStream;
	NkMAIDGetSBGroupAttrDesc	stSbGroupAttrDesc;
	LPNkMAIDCapInfo				pCapInfo = NULL;

	// To clear the structure.
	memset(&stSbGroupAttrDesc, 0, sizeof(NkMAIDGetSBGroupAttrDesc));

	// To display the menu.
	qDebug() << ("\nSelect SBGroupID(1-8, 0)\n");
	qDebug() << (" 1. All\n");
	qDebug() << (" 2. Master\n");
	qDebug() << (" 3. A\n");
	qDebug() << (" 4. B\n");
	qDebug() << (" 5. C\n");
	qDebug() << (" 6. D\n");
	qDebug() << (" 7. E\n");
	qDebug() << (" 8. F\n");
	qDebug() << (" 0. Exit\n>");
	scanf("%s", buf);
	ulSBGroupID = (ULONG)atoi(buf);

	// Check the input value.
	if ((ulSBGroupID == 0) || (ulSBGroupID > 8))
	{
		return true;
	}

	// To display the menu.
	qDebug() << ("\nSelect SBGroupAttrID(1-10, 0)\n");
	qDebug() << (" 1. All\n");
	qDebug() << (" 2. FlashMode\n");
	qDebug() << (" 3. FlashCompensation\n");
	qDebug() << (" 4. FlashRatio\n");
	qDebug() << (" 5. FlashLevel\n");
	qDebug() << (" 6. FlashRange\n");
	qDebug() << (" 7. Repeat\n");
	qDebug() << (" 8. RepeatCount\n");
	qDebug() << (" 9. RepeatInterval\n");
	qDebug() << ("10. Invalid\n");
	qDebug() << (" 0. Exit\n>");
	scanf("%s", buf);
	ulSBGroupAttrID = (ULONG)atoi(buf);

	// Check the input value.
	if ((ulSBGroupAttrID == 0) || (ulSBGroupAttrID > 10))
	{
		return true;
	}

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_GetSBGroupAttrDesc);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_GetSBGroupAttrDesc, kNkMAIDCapOperation_Get))
	{
		return false;
	}

	// 1.To get the size of the order of the SB group attribute descriptor acquisition.
	{
		// ulSBGroupID：To specify the SBGroupID to be acquisition target.
		switch (ulSBGroupID)
		{
		case 1:
			// SBGroupID_All
			stSbGroupAttrDesc.ulSBGroupID = kNkMAIDSBGroupID_ALL;
			break;
		case 2:
			// SBGroupID_Master
			stSbGroupAttrDesc.ulSBGroupID = kNkMAIDSBGroupID_Master;
			break;
		case 3:
			// SBGroupID_A
			stSbGroupAttrDesc.ulSBGroupID = kNkMAIDSBGroupID_A;
			break;
		case 4:
			// SBGroupID_B
			stSbGroupAttrDesc.ulSBGroupID = kNkMAIDSBGroupID_B;
			break;
		case 5:
			// SBGroupID_C
			stSbGroupAttrDesc.ulSBGroupID = kNkMAIDSBGroupID_C;
			break;
		case 6:
			// SBGroupID_D
			stSbGroupAttrDesc.ulSBGroupID = kNkMAIDSBGroupID_D;
			break;
		case 7:
			// SBGroupID_E
			stSbGroupAttrDesc.ulSBGroupID = kNkMAIDSBGroupID_E;
			break;
		case 8:
			// SBGroupID_F
			stSbGroupAttrDesc.ulSBGroupID = kNkMAIDSBGroupID_F;
			break;
		default:
			break;
		}

		// ulSBGroupAttrID：To specify the SBGroupAttrID to be acquisition target.
		switch (ulSBGroupAttrID)
		{
		case 1:
			// SBGroupAttribute_All
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_ALL;
			break;
		case 2:
			// SBGroupAttribute_FlashMode
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashMode;
			break;
		case 3:
			// SBGroupAttribute_FlashCompensation
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashCompensation;
			break;
		case 4:
			// SBGroupAttribute_FlashRatio
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashRatio;
			break;
		case 5:
			// SBGroupAttribute_FlashLevel
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashLevel;
			break;
		case 6:
			// SBGroupAttribute_FlashRange
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashRange;
			break;
		case 7:
			// SBGroupAttribute_Repeat
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_Repeat;
			break;
		case 8:
			// SBGroupAttribute_RepeatCount
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_RepeatCount;
			break;
		case 9:
			// SBGroupAttribute_RepeatInterval
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_RepeatInterval;
			break;
		case 10:
			// SBGroupAttribute_Invalid
			stSbGroupAttrDesc.ulSBGroupAttrID = kNkMAIDSBGroupAttribute_Invalid;
			break;
		default:
			break;
		}

		// ulSize：0 is specified.
		stSbGroupAttrDesc.ulSize = 0;

		// To get the size of the order of the SB group attribute descriptor acquisition.
		bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_GetSBGroupAttrDesc, kNkMAIDDataType_GenericPtr, (NKPARAM)&stSbGroupAttrDesc, NULL, NULL);
		if (bRet == false)
		{
			// abnomal.
			qDebug() << ("\nFailed in getting SBGroupAttrDesc.\n");

			return false;
		}
	}

	// 2.To get the SB group attribute descriptor.
	{
		// ulSBGroupID：To specify the SBGroupID to be acquisition target. ( Already set )
		// ulSBGroupAttrID：To specify the SBGroupAttrID to be acquisition target. ( Already set )
		// ulSize：Size of SBGroupAttributeDescList set to pData. ( Already set )
		// pData：Data pointer for SBGroupAttributeDescList acquisition.
		stSbGroupAttrDesc.pData = (char*)malloc(stSbGroupAttrDesc.ulSize);
		if (stSbGroupAttrDesc.pData == NULL)
		{
			// abnomal.
			qDebug() << ("\nmemory allocation error.\n");

			return false;
		}

		// To get the SB group attribute descriptor.
		bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_GetSBGroupAttrDesc, kNkMAIDDataType_GenericPtr, (NKPARAM)&stSbGroupAttrDesc, NULL, NULL);
		if (bRet == false)
		{
			// abnomal.
			qDebug() << ("\nFailed in getting SBGroupAttrDesc.\n");
			free(stSbGroupAttrDesc.pData);

			return false;
		}
	}

	// Save to file
	// open file
	fileStream = fopen("SBGroupAttrDesc.dat", "wb");
	if (fileStream == NULL)
	{
		// abnomal.
		qDebug() << ("\nfile open error.\n");
		free(stSbGroupAttrDesc.pData);

		return false;
	}

	// Get data pointer
	unsigned char* pucData = NULL;
	pucData = (unsigned char*)stSbGroupAttrDesc.pData;

	// write file
	fwrite(pucData, 1, stSbGroupAttrDesc.ulSize, fileStream);
	qDebug() << ("\nSBGroupAttrDesc.dat was saved.\n");

	// close file
	fclose(fileStream);
	free(stSbGroupAttrDesc.pData);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Set/Get SBGroupAttrValueCapability
BOOL SBGroupAttrValueCapability(LPRefObj pRefObj)
{
	BOOL					bRet = true;
	char					buf[256] = { 0 };
	ULONG					ulSel;
	NkMAIDSBGroupAttrValue	stSbGroupAttrValue;

	do
	{
		// To clear the structure.
		memset(&stSbGroupAttrValue, 0, sizeof(NkMAIDSBGroupAttrValue));

		// To display the menu.
		qDebug() << ("\nSelect (1-2, 0)\n");
		qDebug() << (" 1. Set SB Group Attribute Value\n");
		qDebug() << (" 2. Get SB Group Attribute Value\n");
		qDebug() << (" 0. Exit\n>");
		scanf("%s", buf);
		ulSel = (ULONG)atol(buf);
		switch (ulSel)
		{
		case 0:
			// Exit
			break;
		case 1:
			// Set
			bRet = SetSBGroupAttrValueCapability(pRefObj, &stSbGroupAttrValue);
			break;
		case 2:
			// Get
			bRet = GetSBGroupAttrValueCapability(pRefObj, &stSbGroupAttrValue);
			break;
		default:
			break;
		}

		if (bRet == false)
		{
			// abnomal.
			qDebug() << ("An Error occured. \n");
		}
	} while (ulSel != 0);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Set SBGroupAttrValueCapability
BOOL SetSBGroupAttrValueCapability(LPRefObj pRefObj, NkMAIDSBGroupAttrValue *pstSbGroupAttrValue)
{
	BOOL				bRet = false;
	char				cFileName[256] = { 0 };
	char				*pData = NULL;
	ULONG				ulCount = 0;
	ULONG				ulTotal = 0;
	FILE				*fileStream;
	LPNkMAIDCapInfo		pCapInfo = NULL;

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_SBGroupAttrValue);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_SBGroupAttrValue, kNkMAIDCapOperation_Set))
	{
		return false;
	}

	// ulSBGroupID：Not use the Set.
	// ulSBGroupAttrID：Not use the Set.

	// Open the file
	strcpy(cFileName, "SBGroupAttrValue.dat");
	fileStream = fopen(cFileName, "rb");
	if (fileStream == NULL)
	{
		// abnomal.
		qDebug() << ("\nFile open error.\n");

		return false;
	}

	// To get the size.
	struct stat stStatBuf;
#if defined( _WIN32 )
	if (fstat(_fileno(fileStream), &stStatBuf) == -1)
#elif defined(__APPLE__)
	if (fstat(fileno(fileStream), &stStatBuf) == -1)
#endif
	{
		// abnomal.
		qDebug() << ("\nFile status error.\n");
		fclose(fileStream);

		return false;
	}

	// ulSize：Data size of SBGroupAttributeList set to pData.
	pstSbGroupAttrValue->ulSize = (ULONG)stStatBuf.st_size;

	// pData：A pointer to the SBGroupAttributeList.
	pstSbGroupAttrValue->pData = (char*)malloc(pstSbGroupAttrValue->ulSize);
	if (pstSbGroupAttrValue->pData == NULL)
	{
		// abnomal.
		qDebug() << ("\nMemory allocation error.\n");
		fclose(fileStream);

		return false;
	}

	// To get the binary data .
	pData = (char *)pstSbGroupAttrValue->pData;
	while (!feof(fileStream))
	{
		// Read file until eof.
		ulCount = (ULONG)fread(pData, sizeof(char), 100, fileStream);
		if (ferror(fileStream))
		{
			// abnomal.
			qDebug() << ("\nfile read error.\n");
			fclose(fileStream);
			free(pstSbGroupAttrValue->pData);

			return false;
		}

		// Total count up actual bytes read.
		ulTotal += ulCount;
		pData += ulCount;
		if (pstSbGroupAttrValue->ulSize < ulTotal)
		{
			qDebug() << ("\nThe size of \"SBGroupAttrValue.dat\" is size over.\n");
			fclose(fileStream);
			free(pstSbGroupAttrValue->pData);
			return false;
		}
	}

	// Set SB Group Attr Value.
	bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_SBGroupAttrValue, kNkMAIDDataType_GenericPtr, (NKPARAM)pstSbGroupAttrValue, NULL, NULL);
	if (bRet == false)
	{
		// abnomal.
		qDebug() << ("\nFailed in setting SB Group Attribute Value.\n");
		fclose(fileStream);
		free(pstSbGroupAttrValue->pData);

		return false;
	}

	qDebug() << ("\nSucceed in setting SB Group Attribute Value.\n");

	fclose(fileStream);
	free(pstSbGroupAttrValue->pData);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Get SBGroupAttrValueCapability
BOOL GetSBGroupAttrValueCapability(LPRefObj pRefObj, NkMAIDSBGroupAttrValue *pstSbGroupAttrValue)
{
	BOOL				bRet = true;
	char				buf[256] = { 0 };
	ULONG				ulSBGroupID;
	ULONG				ulSBGroupAttrID;
	FILE						*fileStream;
	LPNkMAIDCapInfo		pCapInfo = NULL;

	// To display the menu.
	qDebug() << ("\nSelect SBGroupID(1-8, 0)\n");
	qDebug() << (" 1. SBGroupID ALL\n");
	qDebug() << (" 2. SBGroupID Master\n");
	qDebug() << (" 3. SBGroupID A\n");
	qDebug() << (" 4. SBGroupID B\n");
	qDebug() << (" 5. SBGroupID C\n");
	qDebug() << (" 6. SBGroupID D\n");
	qDebug() << (" 7. SBGroupID E\n");
	qDebug() << (" 8. SBGroupID F\n");
	qDebug() << (" 0. Exit\n>");
	scanf("%s", buf);
	ulSBGroupID = (ULONG)atoi(buf);

	// Check the input value.
	if ((ulSBGroupID == 0) || (ulSBGroupID > 8))
	{
		return true;
	}

	// To display the menu.
	qDebug() << ("\nSelect SBGroupAttrID(1-10, 0)\n");
	qDebug() << (" 1. All\n");
	qDebug() << (" 2. FlashMode\n");
	qDebug() << (" 3. FlashCompensation\n");
	qDebug() << (" 4. FlashRatio\n");
	qDebug() << (" 5. FlashLevel\n");
	qDebug() << (" 6. FlashRange\n");
	qDebug() << (" 7. Repeat\n");
	qDebug() << (" 8. RepeatCount\n");
	qDebug() << (" 9. RepeatInterval\n");
	qDebug() << ("10. Invalid\n");
	qDebug() << (" 0. Exit\n>");
	scanf("%s", buf);
	ulSBGroupAttrID = (ULONG)atoi(buf);

	// Check the input value.
	if ((ulSBGroupAttrID == 0) || (ulSBGroupAttrID > 10))
	{
		return true;
	}

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_SBGroupAttrValue);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_SBGroupAttrValue, kNkMAIDCapOperation_Get))
	{
		return false;
	}

	// 1.To get the size in order to obtain the current value of SB group attributes.
	{
		// ulSBGroupID：To specify the SBGroupID to be acquisition target.
		switch (ulSBGroupID)
		{
		case 1:
			// SBGroupID_All
			pstSbGroupAttrValue->ulSBGroupID = kNkMAIDSBGroupID_ALL;
			break;
		case 2:
			// SBGroupID_Master
			pstSbGroupAttrValue->ulSBGroupID = kNkMAIDSBGroupID_Master;
			break;
		case 3:
			// SBGroupID_A
			pstSbGroupAttrValue->ulSBGroupID = kNkMAIDSBGroupID_A;
			break;
		case 4:
			// SBGroupID_B
			pstSbGroupAttrValue->ulSBGroupID = kNkMAIDSBGroupID_B;
			break;
		case 5:
			// SBGroupID_C
			pstSbGroupAttrValue->ulSBGroupID = kNkMAIDSBGroupID_C;
			break;
		case 6:
			// SBGroupID_D
			pstSbGroupAttrValue->ulSBGroupID = kNkMAIDSBGroupID_D;
			break;
		case 7:
			// SBGroupID_E
			pstSbGroupAttrValue->ulSBGroupID = kNkMAIDSBGroupID_E;
			break;
		case 8:
			// SBGroupID_F
			pstSbGroupAttrValue->ulSBGroupID = kNkMAIDSBGroupID_F;
			break;
		default:
			break;
		}

		// ulSBGroupAttrID：To specify the SBGroupAttrID to be acquisition target.
		switch (ulSBGroupAttrID)
		{
		case 0:
			break;
		case 1:
			// SBGroupAttribute_All
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_ALL;
			break;
		case 2:
			// SBGroupAttribute_FlashMode
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashMode;
			break;
		case 3:
			// SBGroupAttribute_FlashCompensation
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashCompensation;
			break;
		case 4:
			// SBGroupAttribute_FlashRatio
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashRatio;
			break;
		case 5:
			// SBGroupAttribute_FlashLevel
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashLevel;
			break;
		case 6:
			// SBGroupAttribute_FlashRange
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_FlashRange;
			break;
		case 7:
			// SBGroupAttribute_Repeat
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_Repeat;
			break;
		case 8:
			// SBGroupAttribute_RepeatCount
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_RepeatCount;
			break;
		case 9:
			// SBGroupAttribute_RepeatInterval
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_RepeatInterval;
			break;
		case 10:
			// SBGroupAttribute_Invalid
			pstSbGroupAttrValue->ulSBGroupAttrID = kNkMAIDSBGroupAttribute_Invalid;
			break;
		default:
			break;
		}

		// ulSize：0 is specified.
		pstSbGroupAttrValue->ulSize = 0;

		// To get the size in order to obtain the current value of SB group attributes.
		bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_SBGroupAttrValue, kNkMAIDDataType_GenericPtr, (NKPARAM)pstSbGroupAttrValue, NULL, NULL);
		if (bRet == false)
		{
			// abnomal.
			qDebug() << ("\nFailed in getting SB Group Attribute Value.\n");

			return false;
		}
	}

	// 2.To get the current value of the SB group attribute.
	{
		// ulSBGroupID：To specify the SBGroupID to be acquisition target. ( Already set )
		// ulSBGroupAttrID：To specify the SBGroupAttrID to be acquisition target. ( Already set )
		// ulSize：Size of SBGroupIDAttributeDescList set to pData. ( Already set )
		// pData：Data pointer for SBGroupIDAttributeDescList acquisition.
		pstSbGroupAttrValue->pData = (char*)malloc(pstSbGroupAttrValue->ulSize);
		if (pstSbGroupAttrValue->pData == NULL)
		{
			// abnomal.
			qDebug() << ("\nMemory allocation error.\n");

			return false;
		}

		// To get the current value of the SB group attribute.
		bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_SBGroupAttrValue, kNkMAIDDataType_GenericPtr, (NKPARAM)pstSbGroupAttrValue, NULL, NULL);
		if (bRet == false)
		{
			// abnomal.
			qDebug() << ("\nFailed in getting SB Group Attribute Value.\n");
			free(pstSbGroupAttrValue->pData);

			return false;
		}
	}

	// Save to file
	// open file
	fileStream = fopen("SBGroupAttrValue.dat", "wb");
	if (fileStream == NULL)
	{
		// abnomal.
		qDebug() << ("\nFile open error.\n");
		free(pstSbGroupAttrValue->pData);

		return false;
	}

	// Get data pointer
	unsigned char* pucData = NULL;
	pucData = (unsigned char*)pstSbGroupAttrValue->pData;

	// write file
	fwrite(pucData, 1, pstSbGroupAttrValue->ulSize, fileStream);
	qDebug() << ("\nSBGroupAttrValue.dat was saved.\n");

	// close file
	fclose(fileStream);
	free(pstSbGroupAttrValue->pData);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Set TestFlashCapability
BOOL TestFlashCapability(LPRefObj pRefObj)
{
	BOOL				bRet = true;
	char				buf[256] = { 0 };
	ULONG				ulSBHandleMenu = 0;
	ULONG				ulSBHandle = 0;
	NkMAIDTestFlash		stTestFlash;
	LPNkMAIDCapInfo		pCapInfo = NULL;

	// To clear the structure.
	memset(&stTestFlash, 0, sizeof(NkMAIDTestFlash));

	// To display the menu.
	qDebug() << ("\nSelect SBHandle(0-1, 2)\n");
	qDebug() << (" 0. SBHandle ALL\n");
	qDebug() << (" 1. Any value\n");
	qDebug() << (" 2. Exit\n>");
	scanf("%s", buf);
	ulSBHandleMenu = (ULONG)atoi(buf);

	// Check the input value.
	if (ulSBHandleMenu >= 2)
	{
		return true;
	}
	else if (ulSBHandleMenu == 1)
	{
		// To display the menu.
		qDebug() << ("\nEnter SBHandle Value(Hex)\n");
		qDebug() << ("0x");
		scanf("%s", buf);

		char *pcErrbuf;
		ulSBHandle = (ULONG)strtol(buf, &pcErrbuf, 16);
		if (*pcErrbuf != '\0')
		{
			return true;
		}
	}
	else
	{
		ulSBHandle = 0;
	}

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_TestFlash);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_TestFlash, kNkMAIDCapOperation_Start))
	{
		return false;
	}

	// ulType：kNkMAIDTestFlash_Test is specified.
	stTestFlash.ulType = kNkMAIDTestFlash_Test;

	// ulParam：To specify the SBHandle to be acquisition target.
	stTestFlash.ulParam = ulSBHandle;
	bRet = Command_CapStartGeneric(pRefObj->pObject, kNkMAIDCapability_TestFlash, (NKPARAM)&stTestFlash, NULL, NULL, NULL);

	if (bRet == false)
	{
		// abnomal.
		qDebug() << ("An Error occured. \n");
		return false;
	}

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// read the array data from the camera and display it on the screen
BOOL ShowArrayCapability(LPRefObj pRefObj, ULONG ulCapID)
{
	BOOL	bRet = true;
	NkMAIDArray	stArray;
	ULONG	ulSize, i, j;
	LPNkMAIDCapInfo	pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Array) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;
	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_ArrayPtr, (NKPARAM)&stArray, NULL, NULL);
	if (bRet == false) return false;

	ulSize = stArray.ulElements * stArray.wPhysicalBytes;
	// allocate memory for array data
	stArray.pData = malloc(ulSize);
	if (stArray.pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_ArrayPtr, (NKPARAM)&stArray, NULL, NULL);
	if (bRet == false) {
		free(stArray.pData);
		return false;
	}

	// show selectable items for this capability and current setting
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	for (i = 0, j = 0; i * 16 + j < ulSize; i++) {
		for (; j < 16 && i * 16 + j < ulSize; j++) {
			qDebug() << (" %02X", ((UCHAR*)stArray.pData)[i * 16 + j]);
		}
		j = 0;
		qDebug() << ("\n");
	}

	if (stArray.pData != NULL)
		free(stArray.pData);
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// read the array data from the camera and save it on a storage(hard drive)
//  for kNkMAIDCapability_GetLiveViewImage
BOOL GetArrayCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDArray pstArray)
{
	BOOL	bRet = true;
	LPNkMAIDCapInfo	pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Array) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;
	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_ArrayPtr, (NKPARAM)pstArray, NULL, NULL);
	if (bRet == false) return false;

	// check if this capability supports CapGetArray operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_GetArray)) return false;
	// allocate memory for array data
	pstArray->pData = malloc(pstArray->ulElements * pstArray->wPhysicalBytes);
	if (pstArray->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_ArrayPtr, (NKPARAM)pstArray, NULL, NULL);
	if (bRet == false) {
		//free(pstArray->pData);
		//pstArray->pData = NULL;
		return false;
	}

	// show selectable items for this capability and current setting
	qDebug() << ("[%s]\n", pCapInfo->szDescription);

	// Do not free( pstArray->pData )
	// Upper class use pstArray->pData to save file.  
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// load the array data from a storage and send it to the camera
BOOL LoadArrayCapability(LPRefObj pRefObj, ULONG ulCapID, char* filename)
{
	BOOL	bRet = true;
	NkMAIDArray	stArray;
	FILE *stream;
	LPNkMAIDCapInfo	pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Array) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;
	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_ArrayPtr, (NKPARAM)&stArray, NULL, NULL);
	if (bRet == false) return false;

	// allocate memory for array data
	stArray.pData = malloc(stArray.ulElements * stArray.wPhysicalBytes);
	if (stArray.pData == NULL) return false;

	// show selectable items for this capability and current setting
	qDebug() << ("[%s]\n", pCapInfo->szDescription);

	if ((stream = fopen(filename, "rb")) == NULL) {
		qDebug() << ("file not found\n");
		if (stArray.pData != NULL)
			free(stArray.pData);
		return false;
	}
	fread(stArray.pData, 1, stArray.ulElements * stArray.wPhysicalBytes, stream);
	fclose(stream);

	// check if this capability supports CapSet operation.
	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// set array data
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_ArrayPtr, (NKPARAM)&stArray, NULL, NULL);
		if (bRet == false) {
			free(stArray.pData);
			return false;
		}
	}
	if (stArray.pData != NULL)
		free(stArray.pData);
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// make look up table and send it to module.
BOOL SetNewLut(LPRefObj pRefSrc)
{
	BOOL	bRet;
	NkMAIDArray stArray;
	double	lfGamma, dfMaxvalue;
	ULONG	i, ulLUTDimSize, ulPlaneCount;
	char	buf[256];

	qDebug() << ("Gamma >");
	scanf("%s", buf);
	lfGamma = atof(buf);

	bRet = Command_CapGet(pRefSrc->pObject, kNkMAIDCapability_Lut, kNkMAIDDataType_ArrayPtr, (NKPARAM)&stArray, NULL, NULL);
	if (bRet == false) return false;
	stArray.pData = malloc(stArray.ulElements * stArray.wPhysicalBytes);
	if (stArray.pData == NULL) return false;

	ulLUTDimSize = stArray.ulDimSize1;
	ulPlaneCount = stArray.ulDimSize2;
	// If the array is one dimension, ulDimSize2 is 0. So the ulPlaneCount should be set 1.
	if (ulPlaneCount == 0) ulPlaneCount = 1;

	dfMaxvalue = (double)(pow(2.0, stArray.wLogicalBits) - 1.0);

	// Make first plane of LookUp Table
	if (stArray.wPhysicalBytes == 1) {
		for (i = 0; i < ulLUTDimSize; i++)
			((UCHAR*)stArray.pData)[i] = (UCHAR)(pow(((double)i / ulLUTDimSize), 1.0 / lfGamma) * dfMaxvalue + 0.5);
	}
	else if (stArray.wPhysicalBytes == 2) {
		for (i = 0; i < ulLUTDimSize; i++)
			((UWORD*)stArray.pData)[i] = (UWORD)(pow(((double)i / ulLUTDimSize), 1.0 / lfGamma) * dfMaxvalue + 0.5);
	}
	else {
		free(stArray.pData);
		return false;
	}
	// Copy from first plane to second and third... plane.
	for (i = 1; i < ulPlaneCount; i++)
		memcpy((LPVOID)((char*)stArray.pData + i * ulLUTDimSize * stArray.wPhysicalBytes), stArray.pData, ulLUTDimSize * stArray.wPhysicalBytes);

	// check if this capability supports CapSet operation.
	if (CheckCapabilityOperation(pRefSrc, kNkMAIDCapability_Lut, kNkMAIDCapOperation_Set)) {
		// Send look up table
		bRet = Command_CapSet(pRefSrc->pObject, kNkMAIDCapability_Lut, kNkMAIDDataType_ArrayPtr, (NKPARAM)&stArray, NULL, NULL);
		if (bRet == false) {
			free(stArray.pData);
			return false;
		}
	}
	free(stArray.pData);
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL IssueProcess(LPRefObj pRefSrc, ULONG ulCapID)
{
	LPNkMAIDObject pSourceObject = pRefSrc->pObject;
	LPNkMAIDCapInfo pCapInfo;
	ULONG	ulCount = 0L;
	BOOL bRet;
	LPRefCompletionProc pRefCompletion;
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;

	// Confirm whether this capability is supported or not.
	pCapInfo = GetCapInfo(pRefSrc, ulCapID);
	// check if the CapInfo is available.
	if (pCapInfo == NULL) return false;

	qDebug() << ("[%s]\n", pCapInfo->szDescription);

	// Start the process
//	bRet = Command_CapStart(pSourceObject, ulCapID, (LPNKFUNC)CompletionProc, (NKREF)pRefCompletion, &m_stArray);
	bRet = Command_CapStart(pSourceObject, ulCapID, (LPNKFUNC)CompletionProc, (NKREF)pRefCompletion, NULL);
	if (bRet == false) return false;
	// Wait for end of the process and issue Command_Async.
	IdleLoop(pSourceObject, &ulCount, 1);

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL TerminateCaptureCapability(LPRefObj pRefSrc)
{
	LPNkMAIDObject pSourceObject = pRefSrc->pObject;
	LPNkMAIDCapInfo pCapInfo;
	ULONG	ulCount = 0L;
	BOOL bRet;
	NkMAIDTerminateCapture Param;
	LPRefCompletionProc pRefCompletion;

	Param.ulParameter1 = 0;
	Param.ulParameter2 = 0;

	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = NULL;

	// Confirm whether this capability is supported or not.
	pCapInfo = GetCapInfo(pRefSrc, kNkMAIDCapability_TerminateCapture);
	// check if the CapInfo is available.
	if (pCapInfo == NULL) return false;

	qDebug() << ("[%s]\n", pCapInfo->szDescription);

	// Start the process
	bRet = Command_CapStartGeneric(pSourceObject, kNkMAIDCapability_TerminateCapture, (NKPARAM)&Param, (LPNKFUNC)CompletionProc, (NKREF)pRefCompletion, NULL);
	if (bRet == false) return false;

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
BOOL GetRecordingInfoCapability(LPRefObj pRefObj)
{
	BOOL bRet = true;
	NkMAIDGetRecordingInfo stGetRecordingInfo;
	LPNkMAIDCapInfo pCapInfo = NULL;

	pCapInfo = GetCapInfo(pRefObj, kNkMAIDCapability_GetRecordingInfo);
	if (pCapInfo == NULL)
	{
		return false;
	}

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Generic)
	{
		return false;
	}

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, kNkMAIDCapability_GetRecordingInfo, kNkMAIDCapOperation_Get))
	{
		return false;
	}

	bRet = Command_CapGet(pRefObj->pObject, kNkMAIDCapability_GetRecordingInfo, kNkMAIDDataType_GenericPtr, (NKPARAM)&stGetRecordingInfo, NULL, NULL);
	if (bRet == false)
	{
		// abnomal.
		qDebug() << ("\nFailed in getting RecordingInfo.\n");

		return false;
	}

	qDebug() << ("File Index                  %d\n", stGetRecordingInfo.ulIndexOfMov);
	qDebug() << ("The number of divided files %d\n", stGetRecordingInfo.ulTotalMovCount);
	qDebug() << ("Total Size                  %llu[byte]\n", stGetRecordingInfo.ullTotalMovSize);

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL IssueProcessSync(LPRefObj pRefSrc, ULONG ulCapID)
{
	LPNkMAIDObject pSourceObject = pRefSrc->pObject;
	LPNkMAIDCapInfo pCapInfo;
	BOOL bRet;
	// Confirm whether this capability is supported or not.
	pCapInfo = GetCapInfo(pRefSrc, ulCapID);
	// check if the CapInfo is available.
	if (pCapInfo == NULL) return false;

	qDebug() << ("[%s]\n", pCapInfo->szDescription);

	// Start the process
	bRet = Command_CapStart(pSourceObject, ulCapID, NULL, NULL, NULL);
	if (bRet == false) return false;

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL IssueAcquire(LPRefObj pRefDat)
{
	BOOL	bRet;
	LPRefObj	pRefItm = (LPRefObj)pRefDat->pRefParent;
	NkMAIDCallback	stProc;
	LPRefDataProc	pRefDeliver;
	LPRefCompletionProc	pRefCompletion;
	ULONG	ulCount = 0L;

	// set reference from DataProc
	pRefDeliver = (LPRefDataProc)malloc(sizeof(RefDataProc));// this block will be freed in CompletionProc.
	pRefDeliver->pBuffer = NULL;
	pRefDeliver->ulOffset = 0L;
	pRefDeliver->ulTotalLines = 0L;
	pRefDeliver->lID = pRefItm->lMyID;
	// set reference from CompletionProc
	pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));// this block will be freed in CompletionProc.
	pRefCompletion->pulCount = &ulCount;
	pRefCompletion->pRef = pRefDeliver;
	// set reference from DataProc
	stProc.pProc = (LPNKFUNC)DataProc;
	stProc.refProc = (NKREF)pRefDeliver;

	// set DataProc as data delivery callback function
	if (CheckCapabilityOperation(pRefDat, kNkMAIDCapability_DataProc, kNkMAIDCapOperation_Set)) {
		bRet = Command_CapSet(pRefDat->pObject, kNkMAIDCapability_DataProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
		if (bRet == false) return false;
	}
	else
		return false;

	// start getting an image
	bRet = Command_CapStart(pRefDat->pObject, kNkMAIDCapability_Acquire, (LPNKFUNC)CompletionProc, (NKREF)pRefCompletion, NULL);
	if (bRet == false) return false;
	IdleLoop(pRefDat->pObject, &ulCount, 1);

	// reset DataProc
	bRet = Command_CapSet(pRefDat->pObject, kNkMAIDCapability_DataProc, kNkMAIDDataType_Null, (NKPARAM)NULL, NULL, NULL);
	if (bRet == false) return false;

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Get Video image
BOOL GetVideoImageCapability(LPRefObj pRefDat, ULONG ulCapID)
{
	BOOL	bRet = true;
	char	MovieFileName[256];
	FILE*	hFileMovie = NULL;		// Movie file name
	unsigned char* pucData = NULL;	// Movie data pointer
	ULONG	ulTotalSize = 0;
	int i = 0;
	NkMAIDGetVideoImage	stVideoImage;

#if defined( _WIN32 )
	SetConsoleCtrlHandler(cancelhandler, TRUE);
#elif defined(__APPLE__)
	struct sigaction action, oldaction;
	memset(&action, 0, sizeof(action));
	action.sa_handler = cancelhandler;
	action.sa_flags = SA_RESETHAND;
	sigaction(SIGINT, &action, &oldaction);
#endif

	memset(&stVideoImage, 0, sizeof(NkMAIDGetVideoImage));

	// get total size
	stVideoImage.ulDataSize = 0;
	bRet = Command_CapGet(pRefDat->pObject, ulCapID, kNkMAIDDataType_GenericPtr, (NKPARAM)&stVideoImage, NULL, NULL);
	if (bRet == false) return false;

	ulTotalSize = stVideoImage.ulDataSize;
	if (ulTotalSize == 0) return false;

	// get movie data
	stVideoImage.ulType = kNkMAIDArrayType_Unsigned;
	stVideoImage.ulDataSize = VIDEO_SIZE_BLOCK;		// read block size : 5MB
	stVideoImage.ulReadSize = 0;
	stVideoImage.ulOffset = 0;

	// allocate memory for array data
	stVideoImage.pData = malloc(VIDEO_SIZE_BLOCK);
	if (stVideoImage.pData == NULL) return false;

	// create file name
	while (true)
	{
		qDebug() << (MovieFileName, "MovieData%03d.%s", ++i, "mov");
		if ((hFileMovie = fopen(MovieFileName, "r")) != NULL)
		{
			// this file name is already used.
			fclose(hFileMovie);
			hFileMovie = NULL;
		}
		else
		{
			break;
		}
	}

	// open file
	hFileMovie = fopen(MovieFileName, "wb");
	if (hFileMovie == NULL)
	{
		fclose(hFileMovie);
		qDebug() << ("file open error.\n");
		return false;
	}

	// Get data pointer
	pucData = (unsigned char*)stVideoImage.pData;

	qDebug() << ("Please press the Ctrl+C to cancel.\n");

	// write file
	while ((stVideoImage.ulOffset < ulTotalSize) && (bRet == TRUE))
	{
		if (TRUE == g_bCancel)
		{
			stVideoImage.ulDataSize = 0;
			bRet = Command_CapGetArray(pRefDat->pObject, ulCapID, kNkMAIDDataType_GenericPtr, (NKPARAM)&stVideoImage, NULL, NULL);
			break;
		}

		bRet = Command_CapGetArray(pRefDat->pObject, ulCapID, kNkMAIDDataType_GenericPtr, (NKPARAM)&stVideoImage, NULL, NULL);

		stVideoImage.ulOffset += (stVideoImage.ulReadSize);

		fwrite(pucData, stVideoImage.ulReadSize, 1, hFileMovie);

		if (bRet == false) {
			free(stVideoImage.pData);
			return false;
		}

	}
#if defined( _WIN32 )
	SetConsoleCtrlHandler(cancelhandler, FALSE);
#elif defined(__APPLE__)
	sigaction(SIGINT, &oldaction, NULL);
#endif

	if (stVideoImage.ulOffset < ulTotalSize && TRUE == g_bCancel)
	{
		qDebug() << ("Get Video image was canceled.\n");
	}
	else {
		qDebug() << ("%s was saved.\n", MovieFileName);
	}
	g_bCancel = false;

	// close file
	fclose(hFileMovie);
	free(stVideoImage.pData);

	return true;
}

//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL IssueThumbnail(LPRefObj pRefSrc)
{
	BOOL	bRet;
	LPRefObj	pRefItm, pRefDat;
	NkMAIDCallback	stProc;
	LPRefDataProc	pRefDeliver;
	LPRefCompletionProc	pRefCompletion;
	ULONG	ulItemID, ulFinishCount = 0L;
	ULONG	i, j;
	NkMAIDEnum	stEnum;
	LPNkMAIDCapInfo	pCapInfo;

	pCapInfo = GetCapInfo(pRefSrc, kNkMAIDCapability_Children);
	// check if the CapInfo is available.
	if (pCapInfo == NULL)	return false;

	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefSrc, kNkMAIDCapability_Children, kNkMAIDCapOperation_Get)) return false;
	bRet = Command_CapGet(pRefSrc->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);
	if (bRet == false) return false;

	// If the source object has no item, it does nothing and returns soon.
	if (stEnum.ulElements == 0)	return true;

	// allocate memory for array data
	stEnum.pData = malloc(stEnum.ulElements * stEnum.wPhysicalBytes);
	if (stEnum.pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefSrc->pObject, kNkMAIDCapability_Children, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);
	if (bRet == false) {
		free(stEnum.pData);
		return false;
	}

	// Open all thumbnail objects in the current directory.
	for (i = 0; i < stEnum.ulElements; i++) {
		ulItemID = ((ULONG*)stEnum.pData)[i];
		pRefItm = GetRefChildPtr_ID(pRefSrc, ulItemID);
		if (pRefItm == NULL) {
			// open the item object
			bRet = AddChild(pRefSrc, ulItemID);
			if (bRet == false) {
				free(stEnum.pData);
				return false;
			}
			pRefItm = GetRefChildPtr_ID(pRefSrc, ulItemID);
		}
		if (pRefItm != NULL) {
			pRefDat = GetRefChildPtr_ID(pRefItm, kNkMAIDDataObjType_Thumbnail);
			if (pRefDat == NULL) {
				// open the thumbnail object
				bRet = AddChild(pRefItm, kNkMAIDDataObjType_Thumbnail);
				if (bRet == false) {
					free(stEnum.pData);
					return false;
				}
				pRefDat = GetRefChildPtr_ID(pRefItm, kNkMAIDDataObjType_Thumbnail);
			}
		}
	}
	free(stEnum.pData);

	// set NkMAIDCallback structure for DataProc
	stProc.pProc = (LPNKFUNC)DataProc;

	// acquire all thumbnail images.
	for (i = 0; i < pRefSrc->ulChildCount; i++) {
		pRefItm = GetRefChildPtr_Index(pRefSrc, i);
		pRefDat = GetRefChildPtr_ID(pRefItm, kNkMAIDDataObjType_Thumbnail);

		if (pRefDat != NULL) {
			// set RefDeliver structure refered in DataProc
			pRefDeliver = (LPRefDataProc)malloc(sizeof(RefDataProc));// this block will be freed in CompletionProc.
			pRefDeliver->pBuffer = NULL;
			pRefDeliver->ulOffset = 0L;
			pRefDeliver->ulTotalLines = 0L;
			pRefDeliver->lID = pRefItm->lMyID;

			// set DataProc as data delivery callback function
			stProc.refProc = (NKREF)pRefDeliver;
			if (CheckCapabilityOperation(pRefDat, kNkMAIDCapability_DataProc, kNkMAIDCapOperation_Set)) {
				bRet = Command_CapSet(pRefDat->pObject, kNkMAIDCapability_DataProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
				if (bRet == false) return false;
			}
			else
				return false;

			pRefCompletion = (LPRefCompletionProc)malloc(sizeof(RefCompletionProc));// this block will be freed in CompletionProc.

																					// Set RefCompletion structure refered from CompletionProc.
			pRefCompletion->pulCount = &ulFinishCount;
			pRefCompletion->pRef = pRefDeliver;

			// Starting Acquire Thumbnail
			bRet = Command_CapStart(pRefDat->pObject, kNkMAIDCapability_Acquire, (LPNKFUNC)CompletionProc, (NKREF)pRefCompletion, NULL);
			if (bRet == false) return false;
		}
		else {
			// This item doesn't have a thumbnail, so we count up ulFinishCount.
			ulFinishCount++;
		}

		// Send Async command to all DataObjects that have started acquire command.
		for (j = 0; j <= i; j++) {
			bRet = Command_Async(GetRefChildPtr_ID(GetRefChildPtr_Index(pRefSrc, j), kNkMAIDDataObjType_Thumbnail)->pObject);
			if (bRet == false) return false;
		}
	}

	// Send Async command to all DataObjects, untill all scanning complete.
	while (ulFinishCount < pRefSrc->ulChildCount) {
		for (j = 0; j < pRefSrc->ulChildCount; j++) {
			bRet = Command_Async(GetRefChildPtr_ID(GetRefChildPtr_Index(pRefSrc, j), kNkMAIDDataObjType_Thumbnail)->pObject);
			if (bRet == false) return false;
		}
	}

	// Close all item objects(include image and thumbnail object).
	while (pRefSrc->ulChildCount > 0) {
		pRefItm = GetRefChildPtr_Index(pRefSrc, 0);
		ulItemID = pRefItm->lMyID;
		// reset DataProc
		bRet = Command_CapSet(pRefDat->pObject, kNkMAIDCapability_DataProc, kNkMAIDDataType_Null, (NKPARAM)NULL, NULL, NULL);
		if (bRet == false) return false;
		bRet = RemoveChild(pRefSrc, ulItemID);
		if (bRet == false) return false;
	}

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// get pointer to CapInfo, the capability ID of that is 'ulID'
LPNkMAIDCapInfo GetCapInfo(LPRefObj pRef, ULONG ulID)
{
	ULONG i;
	LPNkMAIDCapInfo pCapInfo;

	if (pRef == NULL)
		return NULL;
	for (i = 0; i < pRef->ulCapCount; i++) {
		pCapInfo = (LPNkMAIDCapInfo)((char*)pRef->pCapArray + i * sizeof(NkMAIDCapInfo));
		if (pCapInfo->ulID == ulID)
			break;
	}
	if (i < pRef->ulCapCount)
		return pCapInfo;
	else
		return NULL;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL CheckCapabilityOperation(LPRefObj pRef, ULONG ulID, ULONG ulOperations)
{
	SLONG nResult;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRef, ulID);

	if (pCapInfo != NULL) {
		if (pCapInfo->ulOperations & ulOperations) {
			nResult = kNkMAIDResult_NoError;
		}
		else {
			nResult = kNkMAIDResult_NotSupported;
		}
	}
	else {
		nResult = kNkMAIDResult_NotSupported;
	}

	return (nResult == kNkMAIDResult_NoError);
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL AddChild(LPRefObj pRefParent, SLONG lIDChild)
{
	SLONG lResult;
	ULONG ulCount = pRefParent->ulChildCount;
	LPVOID pNewMemblock = realloc(pRefParent->pRefChildArray, (ulCount + 1) * sizeof(LPRefObj));
	LPRefObj pRefChild = (LPRefObj)malloc(sizeof(RefObj));

	if (pNewMemblock == NULL || pRefChild == NULL) {
		puts("There is not enough memory");
		return false;
	}
	pRefParent->pRefChildArray = pNewMemblock;
	((LPRefObj*)pRefParent->pRefChildArray)[ulCount] = pRefChild;
	InitRefObj(pRefChild);
	pRefChild->lMyID = lIDChild;
	pRefChild->pRefParent = pRefParent;
	pRefChild->pObject = (LPNkMAIDObject)malloc(sizeof(NkMAIDObject));
	if (pRefChild->pObject == NULL) {
		puts("There is not enough memory");
		pRefParent->pRefChildArray = realloc(pRefParent->pRefChildArray, ulCount * sizeof(LPRefObj));
		return false;
	}

	pRefChild->pObject->refClient = (NKREF)pRefChild;
	lResult = Command_Open(pRefParent->pObject, pRefChild->pObject, lIDChild);
	if (lResult == TRUE)
		pRefParent->ulChildCount++;
	else {
		puts("Failed in Opening an object.");
		pRefParent->pRefChildArray = realloc(pRefParent->pRefChildArray, ulCount * sizeof(LPRefObj));
		free(pRefChild->pObject);
		free(pRefChild);
		return false;
	}

	lResult = EnumCapabilities(pRefChild->pObject, &(pRefChild->ulCapCount), &(pRefChild->pCapArray), NULL, NULL);

	// set callback functions to child object.
	SetProc(pRefChild);

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL RemoveChild(LPRefObj pRefParent, SLONG lIDChild)
{
	LPRefObj pRefChild = NULL, *pOldRefChildArray, *pNewRefChildArray;
	ULONG i, n;
	pRefChild = GetRefChildPtr_ID(pRefParent, lIDChild);
	if (pRefChild == NULL) return false;

	while (pRefChild->ulChildCount > 0)
		RemoveChild(pRefChild, ((LPRefObj*)pRefChild->pRefChildArray)[0]->lMyID);

	if (ResetProc(pRefChild) == false) return false;
	if (Command_Close(pRefChild->pObject) == false) return false;
	pOldRefChildArray = (LPRefObj*)pRefParent->pRefChildArray;
	pNewRefChildArray = NULL;
	if (pRefParent->ulChildCount > 1) {
		pNewRefChildArray = (LPRefObj*)malloc((pRefParent->ulChildCount - 1) * sizeof(LPRefObj));
		for (n = 0, i = 0; i < pRefParent->ulChildCount; i++) {
			if (((LPRefObj)pOldRefChildArray[i])->lMyID != lIDChild)
				memmove(&pNewRefChildArray[n++], &pOldRefChildArray[i], sizeof(LPRefObj));
		}
	}
	pRefParent->pRefChildArray = pNewRefChildArray;
	pRefParent->ulChildCount--;
	if (pRefChild->pObject != NULL)
		free(pRefChild->pObject);
	if (pRefChild->pCapArray != NULL)
		free(pRefChild->pCapArray);
	if (pRefChild->pRefChildArray != NULL)
		free(pRefChild->pRefChildArray);
	if (pRefChild != NULL)
		free(pRefChild);
	if (pOldRefChildArray != NULL)
		free(pOldRefChildArray);
	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL SetProc(LPRefObj pRefObj)
{
	BOOL bRet;
	NkMAIDCallback	stProc;
	stProc.refProc = (NKREF)pRefObj;

	if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_ProgressProc, kNkMAIDCapOperation_Set)) {
		stProc.pProc = (LPNKFUNC)ProgressProc;
		bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_ProgressProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
		if (bRet == false) return false;
	}

	switch (pRefObj->pObject->ulType) {
	case kNkMAIDObjectType_Module:
		// If Module object supports Cap_EventProc, set ModEventProc. 
		if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_EventProc, kNkMAIDCapOperation_Set)) {
			stProc.pProc = (LPNKFUNC)ModEventProc;
			bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_EventProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
			if (bRet == false) return false;
		}
		// UIRequestProc is supported by Module object only.
		if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_UIRequestProc, kNkMAIDCapOperation_Set)) {
			stProc.pProc = (LPNKFUNC)UIRequestProc;
			bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_UIRequestProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
			if (bRet == false) return false;
		}
		break;
	case kNkMAIDObjectType_Source:
		// If Source object supports Cap_EventProc, set SrcEventProc. 
		if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_EventProc, kNkMAIDCapOperation_Set)) {
			stProc.pProc = (LPNKFUNC)SrcEventProc;
			bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_EventProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
			if (bRet == false) return false;
		}
		break;
	case kNkMAIDObjectType_Item:
		// If Item object supports Cap_EventProc, set ItmEventProc. 
		if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_EventProc, kNkMAIDCapOperation_Set)) {
			stProc.pProc = (LPNKFUNC)ItmEventProc;
			bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_EventProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
			if (bRet == false) return false;
		}
		break;
	case kNkMAIDObjectType_DataObj:
		// if Data object supports Cap_EventProc, set DatEventProc. 
		if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_EventProc, kNkMAIDCapOperation_Set)) {
			stProc.pProc = (LPNKFUNC)DatEventProc;
			bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_EventProc, kNkMAIDDataType_CallbackPtr, (NKPARAM)&stProc, NULL, NULL);
			if (bRet == false) return false;
		}
		break;
	}

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
//
BOOL ResetProc(LPRefObj pRefObj)
{
	BOOL bRet;

	if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_ProgressProc, kNkMAIDCapOperation_Set)) {
		bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_ProgressProc, kNkMAIDDataType_Null, (NKPARAM)NULL, NULL, NULL);
		if (bRet == false) return false;
	}
	if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_EventProc, kNkMAIDCapOperation_Set)) {
		bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_EventProc, kNkMAIDDataType_Null, (NKPARAM)NULL, NULL, NULL);
		if (bRet == false) return false;
	}

	if (pRefObj->pObject->ulType == kNkMAIDObjectType_Module) {
		// UIRequestProc is supported by Module object only.
		if (CheckCapabilityOperation(pRefObj, kNkMAIDCapability_UIRequestProc, kNkMAIDCapOperation_Set)) {
			bRet = Command_CapSet(pRefObj->pObject, kNkMAIDCapability_UIRequestProc, kNkMAIDDataType_Null, (NKPARAM)NULL, NULL, NULL);
			if (bRet == false) return false;
		}
	}

	return true;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Get pointer to reference of child object by child's ID
LPRefObj GetRefChildPtr_ID(LPRefObj pRefParent, SLONG lIDChild)
{
	LPRefObj pRefChild;
	ULONG ulCount;

	if (pRefParent == NULL)
		return NULL;

	for (ulCount = 0; ulCount < pRefParent->ulChildCount; ulCount++) {
		if ((pRefChild = GetRefChildPtr_Index(pRefParent, ulCount)) != NULL) {
			if (pRefChild->lMyID == lIDChild)
				return pRefChild;
		}
	}

	return NULL;
}
//------------------------------------------------------------------------------------------------------------------------------------
// Get pointer to reference of child object by index
LPRefObj GetRefChildPtr_Index(LPRefObj pRefParent, ULONG ulIndex)
{
	if (pRefParent == NULL)
		return NULL;

	if ((pRefParent->pRefChildArray != NULL) && (ulIndex < pRefParent->ulChildCount))
		return (LPRefObj)((LPRefObj*)pRefParent->pRefChildArray)[ulIndex];
	else
		return NULL;
}
//------------------------------------------------------------------------------------------------------------------------------------
//  
#if defined( _WIN32 )
BOOL WINAPI	cancelhandler(DWORD dwCtrlType)
{
	if (dwCtrlType == CTRL_C_EVENT) {
		g_bCancel = true;
		return TRUE;
	}
	return FALSE;
}
#elif defined(__APPLE__)
void	cancelhandler(int sig)
{
	g_bCancel = true;
}
#endif
//------------------------------------------------------------------------------------------------------------------------------------

void CALLPASCAL CALLBACK SrcEventProc(NKREF refProc, ULONG ulEvent, NKPARAM data)
{
	BOOL bRet;
	LPRefObj pRefParent = (LPRefObj)refProc, pRefChild = NULL;
	NkMAIDEventParam* pParam = NULL;

	switch (ulEvent) {
	case kNkMAIDEvent_AddChild:
		bRet = AddChild(pRefParent, (SLONG)data);
		if (bRet == false) return;
		pRefChild = GetRefChildPtr_ID(pRefParent, (SLONG)data);
		// Enumerate children(Data Objects) and open them.
		bRet = EnumChildrten(pRefChild->pObject);
		if (bRet == false) return;
		break;
	case kNkMAIDEvent_RemoveChild:
		bRet = RemoveChild(pRefParent, (SLONG)data);
		if (bRet == false) return;
		break;
	case kNkMAIDEvent_WarmingUp:
		// The Type0022 Module does not use this event.
		puts("Event_WarmingUp to Source object is not supported.\n");
		break;
	case kNkMAIDEvent_WarmedUp:
		// The Type0022 Module does not use this event.
		puts("Event_WarmedUp to Source object is not supported.\n");
		break;
	case kNkMAIDEvent_CapChange:
	case kNkMAIDEvent_CapChangeOperationOnly:
		// re-enumerate the capabilities
		if (pRefParent->pCapArray != NULL) {
			free(pRefParent->pCapArray);
			pRefParent->ulCapCount = 0;
			pRefParent->pCapArray = NULL;
		}
		bRet = EnumCapabilities(pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL);
		if (bRet == false) return;
		// ToDo: Execute a process when the property of a capability was changed.
		break;
	case kNkMAIDEvent_CapChangeValueOnly:
		// ToDo: Execute a process when the value of a capability was changed.
		qDebug() << ("The value of Capability(CapID=0x%llX) was changed.\n", data);
		break;
	case kNkMAIDEvent_OrphanedChildren:
		// ToDo: Close children(Item Objects).
		break;
	case kNkMAIDEvent_AddPreviewImage:
		// The Type0022 Module does not use this event.
		puts("Event_AddPreviewImage to Item object is not supported.\n");
		break;
	case kNkMAIDEvent_CaptureComplete:
		// ToDo: Show the image transfer finished.
		break;
	case kNkMAIDEvent_AddChildInCard:
		qDebug() << ("a Video object(ID=0x%llX) added in card.\n", data);
		break;
	case kNkMAIDEvent_RecordingInterrupted:
	{
		NkMAIDEventParam* pData = reinterpret_cast<NkMAIDEventParam*>(data);
		ULONG param1 = pData->ulParam[0];
		ULONG param2 = pData->ulParam[1];
		char strParam1[64] = "";
		char strParam2[64] = "";
		switch (param1)
		{
		case 1:
			strcpy(strParam1, "Something");
			break;
		case 0:
			strcpy(strParam1, "Low-speedCard");
			break;
		default:
			strcpy(strParam1, "Unknown parameter");
			break;
		}
		switch (param2)
		{
		case 0:
			strcpy(strParam2, "Card Recording");
			break;
		case 1:
			strcpy(strParam2, "ExternalDevice Recording");
			break;
		case 2:
			strcpy(strParam2, "Card and ExternalDevice Recording");
			break;
		default:
			strcpy(strParam2, "Unknown parameter");
			break;
		}
		qDebug() << ("RecordingInterrupted Event(%s, %s)\r\n", strParam1, strParam2);
		break;
	}
	case kNkMAIDEvent_SBAdded:
		pParam = (NkMAIDEventParam*)data;
		qDebug() << ("SB Added (0x%X).\n", pParam->ulParam[0]);
		break;
	case kNkMAIDEvent_SBRemoved:
		pParam = (NkMAIDEventParam*)data;
		qDebug() << ("SB Removed (0x%X).\n", pParam->ulParam[0]);
		break;
	case kNkMAIDEvent_SBAttrChanged:
		pParam = (NkMAIDEventParam*)data;
		qDebug() << ("SB Attr Changed (SBHandle=0x%X, SBAttrID=0x%X).\n", pParam->ulParam[0], pParam->ulParam[1]);
		break;
	case kNkMAIDEvent_SBGroupAttrChanged:
		pParam = (NkMAIDEventParam*)data;
		qDebug() << ("SB Group Attr Changed (SBGroupID=0x%X, SBGroupAttrID=0x%X).\n", pParam->ulParam[0], pParam->ulParam[1]);
		break;
	case kNkMAIDEvent_1stCaptureComplete:
		qDebug() << ("1st Capture Complete.\n");
		break;
	case kNkMAIDEvent_MirrorUpCancelComplete:
		qDebug() << ("MirrorUp Cancel Complete.\n");
		break;
	case kNkMAIDEvent_MovieRecordComplete:
	{
		char strParam[64] = "";
		switch ((ULONG)data)
		{
		case 0:
			strcpy(strParam, "Card Recording");
			break;
		case 1:
			strcpy(strParam, "ExternalDevice Recording");
			break;
		case 2:
			strcpy(strParam, "Card and ExternalDevice Recording");
			break;
		default:
			strcpy(strParam, "Unknown parameter");
			break;
		}
		qDebug() << ("MovieRecordComplete Event(%s)\r\n", strParam);
		break;
	}
	case kNkMAIDEvent_RequestLiveViewStart:
		qDebug() << ("Request Live View Start.\n");
		break;
	case kNkMAIDEvent_StartMovieRecord:
	{
		char strParam[64] = "";
		switch ((ULONG)data)
		{
		case 0:
			strcpy(strParam, "Card Recording");
			break;
		case 1:
			strcpy(strParam, "ExternalDevice Recording");
			break;
		case 2:
			strcpy(strParam, "Card and ExternalDevice Recording");
			break;
		default:
			strcpy(strParam, "Unknown parameter");
			break;
		}
		qDebug() << ("Start Movie Record (%s)\r\n", strParam);
		break;
	}
	default:
		puts("Detected unknown Event to the Source object.\n");
	}
}
//------------------------------------------------------------------------------------------------------------------------------------

void CALLPASCAL CALLBACK ItmEventProc(NKREF refProc, ULONG ulEvent, NKPARAM data)
{
	BOOL bRet;
	LPRefObj pRefParent = (LPRefObj)refProc;

	switch (ulEvent) {
	case kNkMAIDEvent_AddChild:
		bRet = AddChild(pRefParent, (SLONG)data);
		if (bRet == false) return;
		break;
	case kNkMAIDEvent_RemoveChild:
		bRet = RemoveChild(pRefParent, (SLONG)data);
		if (bRet == false) return;
		break;
	case kNkMAIDEvent_WarmingUp:
		// The Type0022 Module does not use this event.
		puts("Event_WarmingUp to Item object is not supported.\n");
		break;
	case kNkMAIDEvent_WarmedUp:
		// The Type0022 Module does not use this event.
		puts("Event_WarmedUp to Item object is not supported.\n");
		break;
	case kNkMAIDEvent_CapChange:
	case kNkMAIDEvent_CapChangeOperationOnly:
		// re-enumerate the capabilities
		if (pRefParent->pCapArray != NULL) {
			free(pRefParent->pCapArray);
			pRefParent->ulCapCount = 0;
			pRefParent->pCapArray = NULL;
		}
		bRet = EnumCapabilities(pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL);
		if (bRet == false) return;
		// ToDo: Execute a process when the property of a capability was changed.
		break;
	case kNkMAIDEvent_CapChangeValueOnly:
		// ToDo: Execute a process when the value of a capability was changed.
		qDebug() << ("The value of Capability(CapID=0x%llX) was changed.\n", data);
		break;
	case kNkMAIDEvent_OrphanedChildren:
		// ToDo: Close children(Data Objects).
		break;
	default:
		puts("Detected unknown Event to the Item object.\n");
	}
}
//------------------------------------------------------------------------------------------------------------------------------------

void CALLPASCAL CALLBACK DatEventProc(NKREF refProc, ULONG ulEvent, NKPARAM data)
{
	BOOL bRet;
	LPRefObj	pRefParent = (LPRefObj)refProc;

	switch (ulEvent) {
	case kNkMAIDEvent_AddChild:
		// The Type0022 Module does not use this event.
		puts("Event_AddChild to Data object is not supported.\n");
		break;
	case kNkMAIDEvent_RemoveChild:
		// The Type0022 Module does not use this event.
		puts("Event_RemoveChild to Data object is not supported.\n");
		break;
	case kNkMAIDEvent_WarmingUp:
		// The Type0022 Module does not use this event.
		puts("Event_WarmingUp to Data object is not supported.\n");
		break;
	case kNkMAIDEvent_WarmedUp:
		// The Type0022 Module does not use this event.
		puts("Event_WarmedUp to Data object is not supported.\n");
		break;
	case kNkMAIDEvent_CapChange:// module notify that a capability is changed.
	case kNkMAIDEvent_CapChangeOperationOnly:
		// re-enumerate the capabilities
		if (pRefParent->pCapArray != NULL) {
			free(pRefParent->pCapArray);
			pRefParent->ulCapCount = 0;
			pRefParent->pCapArray = NULL;
		}
		bRet = EnumCapabilities(pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL);
		if (bRet == false) return;
		// ToDo: Execute a process when the property of a capability was changed.
	case kNkMAIDEvent_CapChangeValueOnly:
		// ToDo: Execute a process when the value of a capability was changed.
		qDebug() << ("The value of Capability(CapID=0x%llX) was changed.\n", data);
		break;
	case kNkMAIDEvent_OrphanedChildren:
		// The Type0022 Module does not use this event.
		puts("Event_OrphanedChildren to Data object is not supported.\n");
		break;
	default:
		puts("Detected unknown Event to the Data object.\n");
	}
}
//------------------------------------------------------------------------------------------------------------------------------------
// copy the delivered data
NKERROR CALLPASCAL CALLBACK DataProc(NKREF ref, LPVOID pInfo, LPVOID pData)
{
	LPNkMAIDDataInfo pDataInfo = (LPNkMAIDDataInfo)pInfo;
	LPNkMAIDImageInfo pImageInfo = (LPNkMAIDImageInfo)pInfo;
	LPNkMAIDFileInfo pFileInfo = (LPNkMAIDFileInfo)pInfo;
	ULONG ulTotalSize, ulOffset;
	LPVOID pCurrentBuffer;
	ULONG ulByte;

	if (pDataInfo->ulType & kNkMAIDDataObjType_File) {
		if (((LPRefDataProc)ref)->ulOffset == 0 && ((LPRefDataProc)ref)->pBuffer == NULL)
			((LPRefDataProc)ref)->pBuffer = malloc(pFileInfo->ulTotalLength);
		if (((LPRefDataProc)ref)->pBuffer == NULL) {
			puts("There is not enough memory.");
			return kNkMAIDResult_OutOfMemory;
		}
		ulOffset = ((LPRefDataProc)ref)->ulOffset;
		pCurrentBuffer = (LPVOID)((char*)((LPRefDataProc)ref)->pBuffer + ((LPRefDataProc)ref)->ulOffset);
		memmove(pCurrentBuffer, pData, pFileInfo->ulLength);
		ulOffset += pFileInfo->ulLength;

		if (ulOffset < pFileInfo->ulTotalLength) {
			// We have not finished the delivery.
			((LPRefDataProc)ref)->ulOffset = ulOffset;
		}
		else {
			// We have finished the delivery. We will save this file.
			FILE *stream;
			char filename[256], Prefix[16], Ext[16];
			UWORD i = 0;
			if (pDataInfo->ulType & kNkMAIDDataObjType_Image)
				strcpy(Prefix, "Image");
			else if (pDataInfo->ulType & kNkMAIDDataObjType_Thumbnail)
				strcpy(Prefix, "Thumb");
			else
				strcpy(Prefix, "Unknown");
			switch (pFileInfo->ulFileDataType) {
			case kNkMAIDFileDataType_JPEG:
				strcpy(Ext, ".jpg");
				break;
			case kNkMAIDFileDataType_TIFF:
				strcpy(Ext, ".tif");
				break;
			case kNkMAIDFileDataType_NIF:
				strcpy(Ext, ".nef");
				break;
			case kNkMAIDFileDataType_NDF:
				strcpy(Ext, ".ndf");
				break;
			default:
				strcpy(Ext, ".dat");
			}
			while (true) {
				qDebug() << (filename, "%s%03d%s", Prefix, ++i, Ext);
				if ((stream = fopen(filename, "r")) != NULL)
					fclose(stream);
				else
					break;
			}
			if ((stream = fopen(filename, "wb")) == NULL)
				return kNkMAIDResult_UnexpectedError;
			fwrite(((LPRefDataProc)ref)->pBuffer, 1, pFileInfo->ulTotalLength, stream);
			fclose(stream);
			free(((LPRefDataProc)ref)->pBuffer);
			((LPRefDataProc)ref)->pBuffer = NULL;
			((LPRefDataProc)ref)->ulOffset = 0;
			// If the flag of fRemoveObject in NkMAIDFileInfo structure is true, we should remove this item.
			if (pFileInfo->fRemoveObject && (pDataInfo->ulType & kNkMAIDDataObjType_Image))
				g_bFileRemoved = true;
		}
	}
	else {
		ulTotalSize = pImageInfo->ulRowBytes * pImageInfo->szTotalPixels.h;
		if (((LPRefDataProc)ref)->ulOffset == 0 && ((LPRefDataProc)ref)->pBuffer == NULL)
			((LPRefDataProc)ref)->pBuffer = malloc(ulTotalSize);
		if (((LPRefDataProc)ref)->pBuffer == NULL) {
			puts("There is not enough memory.");
			return kNkMAIDResult_OutOfMemory;
		}
		ulOffset = ((LPRefDataProc)ref)->ulOffset;
		pCurrentBuffer = (LPVOID)((char*)((LPRefDataProc)ref)->pBuffer + ulOffset);
		ulByte = pImageInfo->ulRowBytes * pImageInfo->rData.h;
		memmove(pCurrentBuffer, pData, ulByte);
		ulOffset += ulByte;

		if (ulOffset < ulTotalSize) {
			// We have not finished the delivery.
			((LPRefDataProc)ref)->ulOffset = ulOffset;
		}
		else {
			// We have finished the delivery. We will save this file.
			FILE *stream;
			char filename[256], Prefix[16];
			UWORD i = 0;
			if (pDataInfo->ulType & kNkMAIDDataObjType_Image)
				strcpy(Prefix, "Image");
			else if (pDataInfo->ulType & kNkMAIDDataObjType_Thumbnail)
				strcpy(Prefix, "Thumb");
			else
				strcpy(Prefix, "Unknown");
			while (true) {
				qDebug() << (filename, "%s%03d.raw", Prefix, ++i);
				if ((stream = fopen(filename, "r")) != NULL)
					fclose(stream);
				else
					break;
			}
			if ((stream = fopen(filename, "wb")) == NULL)
				return kNkMAIDResult_UnexpectedError;
			fwrite(((LPRefDataProc)ref)->pBuffer, 1, ulTotalSize, stream);
			fclose(stream);
			free(((LPRefDataProc)ref)->pBuffer);
			((LPRefDataProc)ref)->pBuffer = NULL;
			((LPRefDataProc)ref)->ulOffset = 0;
			// If the flag of fRemoveObject in NkMAIDFileInfo structure is true, we should remove this item.
			if (pImageInfo->fRemoveObject && (pDataInfo->ulType & kNkMAIDDataObjType_Image))
				g_bFileRemoved = true;
		}
	}
	return kNkMAIDResult_NoError;
}
void CALLPASCAL ModEventProc(NKREF refProc, ULONG ulEvent, NKPARAM data)
{
	BOOL bRet;
	LPRefObj pRefParent = (LPRefObj)refProc, pRefChild = NULL;

	switch (ulEvent) {
	case kNkMAIDEvent_AddChild:
		bRet = AddChild(pRefParent, (SLONG)data);
		if (bRet == false) return;
		pRefChild = GetRefChildPtr_ID(pRefParent, (SLONG)data);
		// Enumerate children(Item and Data Objects) and open them.
		bRet = EnumChildrten(pRefChild->pObject);
		if (bRet == false) return;
		break;
	case kNkMAIDEvent_RemoveChild:
		bRet = RemoveChild(pRefParent, (SLONG)data);
		if (bRet == false) return;
		break;
	case kNkMAIDEvent_WarmingUp:
		// The Type0022 Module does not use this event.
		puts("Event_WarmingUp to Module object is not supported.\n");
		break;
	case kNkMAIDEvent_WarmedUp:
		// The Type0022 Module does not use this event.
		puts("Event_WarmedUp to Module object is not supported.\n");
		break;
	case kNkMAIDEvent_CapChange:
	case kNkMAIDEvent_CapChangeOperationOnly:
		// re-enumerate the capabilities
		if (pRefParent->pCapArray != NULL) {
			free(pRefParent->pCapArray);
			pRefParent->ulCapCount = 0;
			pRefParent->pCapArray = NULL;
		}
		bRet = EnumCapabilities(pRefParent->pObject, &(pRefParent->ulCapCount), &(pRefParent->pCapArray), NULL, NULL);
		if (bRet == false) return;
		// ToDo: Execute a process when the property of a capability was changed.
		break;
	case kNkMAIDEvent_CapChangeValueOnly:
		// ToDo: Execute a process when the value of a capability was changed.
		qDebug() << ("The value of Capability(CapID=0x%llX) was changed.\n", data);
		break;
	case kNkMAIDEvent_OrphanedChildren:
		// ToDo: Close children(Source Objects).
		break;
	default:
		puts("Detected unknown Event to the Module object.\n");
	}
}
//------------------------------------------------------------------------------------------------------------------------------------

void CALLPASCAL CALLBACK CompletionProc(
	LPNkMAIDObject	pObject,			// module, source, item, or data object
	ULONG				ulCommand,		// Command, one of eNkMAIDCommand
	ULONG				ulParam,			// parameter for the command
	ULONG				ulDataType,		// Data type, one of eNkMAIDDataType
	NKPARAM			data,				// Pointer or long integer
	NKREF				refComplete,	// Reference set by client
	NKERROR			nResult)		// One of eNkMAIDResult)
{
	((LPRefCompletionProc)refComplete)->nResult = nResult;
	(*((LPRefCompletionProc)refComplete)->pulCount)++;

	// if the Command is CapStart acquire, we terminate RefDeliver.
	if (ulCommand == kNkMAIDCommand_CapStart && ulParam == kNkMAIDCapability_Acquire) {
		LPRefDataProc pRefDeliver = (LPRefDataProc)((LPRefCompletionProc)refComplete)->pRef;
		if (pRefDeliver != NULL) {
			if (pRefDeliver->pBuffer != NULL)
				free(pRefDeliver->pBuffer);
			free(pRefDeliver);
		}
	}
	// terminate refComplete.
	if (refComplete != NULL)
		free(refComplete);

}
//------------------------------------------------------------------------------------------------------------------------------------

void CALLPASCAL CALLBACK ProgressProc(
	ULONG				ulCommand,			// Command, one of eNkMAIDCommand
	ULONG				ulParam,				// parameter for the command
	NKREF				refProc,				// Reference set by client
	ULONG				ulDone,				// Numerator
	ULONG				ulTotal)			// Denominator
{
#if defined( _WIN32 )
	ULONG ulNewProgressValue, ulCount;
#elif defined(__APPLE__)
	unsigned long ulNewProgressValue, ulCount;
#endif
	if (ulTotal == 0) {
		// when we don't know how long this process is, we show such as barber's pole.
		if (ulDone == 1) {
#if defined( _WIN32 )
			ulNewProgressValue = timeGetTime();
			if ((ulNewProgressValue < g_ulProgressValue) || (ulNewProgressValue > g_ulProgressValue + 500)) {
				qDebug() << ("c");
				g_ulProgressValue = ulNewProgressValue;
			}
#elif defined(__APPLE__)
			struct tms tm;
			ulNewProgressValue = times(&tm);
			if ((ulNewProgressValue < g_ulProgressValue) || (ulNewProgressValue > g_ulProgressValue + 30)) {
				qDebug() << ("c");
				g_ulProgressValue = ulNewProgressValue;
			}
#endif
		}
		else if (ulDone == 0) {
			qDebug() << ("o");
		}
	}
	else {
		// when we know how long this process is, we show progress bar.
		if (ulDone == 0) {
			if (g_bFirstCall == TRUE) {
				g_ulProgressValue = 0;
				g_bFirstCall = false;
				qDebug() << ("\n0       20        40        60        80        100");
				qDebug() << ("\n---------+---------+---------+---------+---------+\n");
			}
		}
		else {
			// show progress bar
			ulNewProgressValue = (50 * ulDone + ulTotal - 1) / ulTotal;
			ulCount = ulNewProgressValue - g_ulProgressValue;
			while (ulCount--)
				qDebug() << ("]");
			g_ulProgressValue = ulNewProgressValue;
			if (ulDone == ulTotal) {
				qDebug() << ("\n");
				g_bFirstCall = true;
			}
		}
	}
}
//------------------------------------------------------------------------------------------------------------------------------------

ULONG CALLPASCAL CALLBACK UIRequestProc(NKREF ref, LPNkMAIDUIRequestInfo pUIRequest)
{
	short	 nRet = kNkMAIDUIRequestResult_None;
	char	sAns[256];

	// display message
	if (pUIRequest->lpPrompt)
		qDebug() << ("\n%s\n", pUIRequest->lpPrompt);
	if (pUIRequest->lpDetail)
		qDebug() << ("\n%s\n", pUIRequest->lpDetail);

	// get an answer
	switch (pUIRequest->ulType) {
	case kNkMAIDUIRequestType_Ok:
		do {
			qDebug() << ("\nPress 'O' key. ('O': OK)\n>");
			scanf("%s", sAns);
		} while (*sAns != 'o' && *sAns != 'O');
		nRet = kNkMAIDUIRequestResult_Ok;
		break;
	case kNkMAIDUIRequestType_OkCancel:
		do {
			qDebug() << ("\nPress 'O' or 'C' key. ('O': OK   'C': Cancel)\n>");
			scanf("%s", sAns);
		} while (*sAns != 'o' && *sAns != 'O' && *sAns != 'c' && *sAns != 'C');
		if (*sAns == 'o' || *sAns == 'O')
			nRet = kNkMAIDUIRequestResult_Ok;
		else if (*sAns == 'c' || *sAns == 'C')
			nRet = kNkMAIDUIRequestResult_Cancel;
		break;
	case kNkMAIDUIRequestType_YesNo:
		do {
			qDebug() << ("\nPress 'Y' or 'N' key. ('Y': Yes   'N': No)\n>");
			scanf("%s", sAns);
		} while (*sAns != 'y' && *sAns != 'Y' && *sAns != 'n' && *sAns != 'N');
		if (*sAns == 'y' || *sAns == 'Y')
			nRet = kNkMAIDUIRequestResult_Yes;
		else if (*sAns == 'n' || *sAns == 'N')
			nRet = kNkMAIDUIRequestResult_No;
		break;
	case kNkMAIDUIRequestType_YesNoCancel:
		do {
			qDebug() << ("\nPress 'Y' or 'N' or 'C' key. ('Y': Yes   'N': No   'C': Cancel)\n>");
			scanf("%s", sAns);
		} while (*sAns != 'y' && *sAns != 'Y' && *sAns != 'n' && *sAns != 'N' && *sAns != 'c' && *sAns != 'C');
		if (*sAns == 'y' || *sAns == 'Y')
			nRet = kNkMAIDUIRequestResult_Yes;
		else if (*sAns == 'n' || *sAns == 'N')
			nRet = kNkMAIDUIRequestResult_No;
		else if (*sAns == 'c' || *sAns == 'C')
			nRet = kNkMAIDUIRequestResult_Cancel;
		break;
	default:
		nRet = kNkMAIDUIRequestResult_None;
	}

	return nRet;
}
//------------------------------------------------------------------------------------------------------------------------------------

BOOL NKCamera::ImportModule()
{
	m_sAppRoot = QCoreApplication::applicationDirPath();
	QString path = m_sAppRoot+"/res/Type0022.md3";
	QFileInfo file(path);
	
	if (!file.isFile())
	{
		qDebug()<<("\"Type0022 Module\" is not found.\n");
		return false;
	}

	QByteArray cdata = path.toLocal8Bit();
	char * Path = cdata.data();

	g_hInstModule = LoadLibraryA((LPCSTR)Path);

	if (g_hInstModule) {
		g_pMAIDEntryPoint = (LPMAIDEntryPointProc)GetProcAddress(g_hInstModule, "MAIDEntryPoint");
		if (g_pMAIDEntryPoint == NULL)
			qDebug() << ("MAIDEntryPoint cannot be found.\n");
	}
	else {
		g_pMAIDEntryPoint = NULL;
		qDebug() << ("\"%s\" cannot be opened.\n", Path);
	}
	return (g_hInstModule != NULL) && (g_pMAIDEntryPoint != NULL);

}

void NKCamera::ShowErrorMsg(QString csMessage, int nErrorNum)
{
}

BOOL NKCamera::SetUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, ULONG newCap)
{
	BOOL	bRet;
	ULONG	ulValue;
	char	buf[256];
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Unsigned) return false;
	// check if this capability suports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_UnsignedPtr, (NKPARAM)&ulValue, NULL, NULL);
	if (bRet == false) return false;
	
	if (ulValue == newCap)
	{
		return true;
	}
	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {

		ulValue = newCap;
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_Unsigned, (NKPARAM)ulValue, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug()<<("This value cannot be changed. \n>");
		return false;
	}
	return true;
}

BOOL NKCamera::SetIntegerCapability(LPRefObj pRefObj, ULONG ulCapID, SLONG newCap)
{
	return 0;
}

BOOL NKCamera::SetEnumStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, UWORD newCap)
{
	BOOL	bRet;
	char	buf[256];
//	UWORD	wSel;
	ULONG	i;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 256) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug() << ("There is no element in this capability. Enter '0' to exit.\n>");
		//scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}

	// check if this capability supports CapSet operation.
	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {

		if (newCap >= 0 && newCap <= pstEnum->ulElements) {
			pstEnum->ulValue = newCap;
			// send the selected number
			bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
			// This statement can be changed as follows.
			//bRet = Command_CapSet( pRefObj->pObject, ulCapID, kNkMAIDDataType_Unsigned, (NKPARAM)pstEnum->ulValue, NULL, NULL );
			if (bRet == false) {
				free(pstEnum->pData);
				return false;
			}
		}
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed.\n>");
		//scanf("%s", buf);
	}
	free(pstEnum->pData);
	return true;
}

BOOL NKCamera::SetEnumPackedStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, UWORD newCap)
{
	BOOL	bRet;
	char	*psStr, buf[256];
//	UWORD	wSel;
	size_t  i;
	ULONG	ulCount = 0;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 1) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug()<<("There is no element in this capability. Enter '0' to exit.\n>");
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		if (newCap >= 0 && newCap <= pstEnum->ulElements) {
			pstEnum->ulValue = newCap ;
			// send the selected number
			bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
			// This statement can be changed as follows.
			//bRet = Command_CapSet( pRefObj->pObject, ulCapID, kNkMAIDDataType_Unsigned, (NKPARAM)pstEnum->ulValue, NULL, NULL );
			if (bRet == false) {
				free(pstEnum->pData);
				return false;
			}
		}
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
	}
	free(pstEnum->pData);
	return true;
	return 0;
}

BOOL NKCamera::SetEnumUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, UWORD newCap)
{
	BOOL	bRet;
	char	psString[32], buf[256];
//	UWORD	wSel;
	ULONG	i;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 4) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug() << ("There is no element in this capability. Enter '0' to exit.\n>");
		//scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		if (newCap >= 0 && newCap <= pstEnum->ulElements) {
			pstEnum->ulValue = newCap;
			// send the selected number
			bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
			// This statement can be changed as follows.
			//bRet = Command_CapSet( pRefObj->pObject, ulCapID, kNkMAIDDataType_Unsigned, (NKPARAM)pstEnum->ulValue, NULL, NULL );
			if (bRet == false) {
				free(pstEnum->pData);
				return false;
			}
		}
	}
	else {
		// This capablity is read-only.
		qDebug()<<("This value cannot be changed. Enter '0' to exit.\n>");
	}
	free(pstEnum->pData);
	return true;
}

BOOL NKCamera::SetEnumCapability(LPRefObj pRefObj, ULONG ulCapID, UWORD newCap)
{
	BOOL	bRet;
	NkMAIDEnum	stEnum;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Enum) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);
	if (bRet == false) return false;
	//int cap = 0;
	switch (stEnum.ulType) {
	case kNkMAIDArrayType_Unsigned:
		//cap = atoi(newCap);
		return SetEnumUnsignedCapability(pRefObj, ulCapID, &stEnum, newCap);
		break;
	case kNkMAIDArrayType_PackedString:
		//cap = atoi(newCap);
		return SetEnumPackedStringCapability(pRefObj, ulCapID, &stEnum, newCap);
		break;
	case kNkMAIDArrayType_String:
		return SetEnumStringCapability(pRefObj, ulCapID, &stEnum, newCap);
		break;
	default:
		return false;
	}
}

BOOL NKCamera::GetEnumCapability(LPRefObj pRefObj, ULONG ulCapID, QStringList& cap)
{
	BOOL	bRet;
	NkMAIDEnum	stEnum;
	ULONG	ulDataTypes = 0;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;
	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Enum) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;
	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)&stEnum, NULL, NULL);

	switch (stEnum.ulType) {
	case kNkMAIDArrayType_Unsigned:
		bRet = GetEnumUnsignedCapability(pRefObj, ulCapID, &stEnum, cap);
		break;
	case kNkMAIDArrayType_PackedString:
		bRet = GetEnumPackedStringCapability(pRefObj, ulCapID, &stEnum, cap);
		break;
	case kNkMAIDArrayType_String:
		bRet = GetEnumStringCapability(pRefObj, ulCapID, &stEnum, cap);
		break;
	default:
		return false;
	}
	
	return true;

}

BOOL NKCamera::GetEnumPackedStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, QStringList& cap)
{
	BOOL	bRet;
	char	*psStr, buf[256];
	UWORD	wSel;
	ULONG	i;
	ULONG	ulCount = 0;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 1) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug() << ("There is no element in this capability. Enter '0' to exit.\n>");
		//scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}
	if (bRet == TRUE)
	{
		if (cap.size()>0)
		{
			cap.clear();
		}
		SCHAR* ch;
		for (i = 0; i < pstEnum->ulElements; )
		{
			psStr = (char*)((char*)pstEnum->pData + i);
			cap.push_back(QString("%1").arg(psStr));
			i += strlen(psStr) + 1;
		}
	}
	m_icurrentValue = pstEnum->ulValue;
	free(pstEnum->pData);
	return true;
}

BOOL NKCamera::GetEnumStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, QStringList& cap)
{
	BOOL	bRet;
	char	buf[256];
	UWORD	wSel;
	ULONG	i;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 256) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug()<<("There is no element in this capability. Enter '0' to exit.\n>");
		//scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}
	if (bRet == TRUE)
	{
		if (cap.size()>0)
		{
			cap.clear();
		}
		SCHAR* ch;
		for (int i = 0; i < pstEnum->ulElements; i++)
		{
			//ch = ((NkMAIDString*)stEnum.pData)[i].str;
			cap.push_back(QString("%1").arg(*(((NkMAIDString*)pstEnum->pData)[i].str)));
		}
	}
	m_icurrentValue = pstEnum->ulValue;
	free(pstEnum->pData);
	return true;
}

BOOL NKCamera::GetEnumUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, QStringList& cap)
{
	BOOL	bRet;
	char	psString[32], buf[256];
	UWORD	wSel;
//	ULONG	i;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check the data of the capability.
	if (pstEnum->wPhysicalBytes != 4) return false;

	// check if this capability has elements.
	if (pstEnum->ulElements == 0)
	{
		// This capablity has no element and is not available.
		qDebug() << ("There is no element in this capability. Enter '0' to exit.\n>");
		//scanf("%s", buf);
		return true;
	}

	// allocate memory for array data
	pstEnum->pData = malloc(pstEnum->ulElements * pstEnum->wPhysicalBytes);
	if (pstEnum->pData == NULL) return false;
	// get array data
	bRet = Command_CapGetArray(pRefObj->pObject, ulCapID, kNkMAIDDataType_EnumPtr, (NKPARAM)pstEnum, NULL, NULL);
	if (bRet == false) {
		free(pstEnum->pData);
		return false;
	}
	if (cap.size()>0)
	{
		cap.clear();
	}
	SCHAR* ch;
	for (int i = 0; i < pstEnum->ulElements; i++)
	{
		//ch = ((NkMAIDString*)stEnum.pData)[i].str;
		cap.push_back(QString("%1").arg(GetEnumString(ulCapID, ((ULONG*)pstEnum->pData)[i], psString)));
	}
	m_icurrentValue = pstEnum->ulValue;
	free(pstEnum->pData);
	return true;
}

BOOL NKCamera::SetBoolCapability(LPRefObj pRefObj, ULONG ulCapID, bool newCap)
{
	BOOL	bRet;
	BYTE	bFlag;
	UWORD	wSel;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_Boolean) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_BooleanPtr, (NKPARAM)&bFlag, NULL, NULL);
	if (bRet == false) return false;

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		bFlag = newCap;
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_Boolean, (NKPARAM)bFlag, NULL, NULL);
		if (bRet == false) 
			return false;
	}
	else {
		// This capablity is read-only.
		qDebug()<<("This value cannot be changed. Enter '0' to exit.\n>");
		return false;
	}
	return true;
}

BOOL NKCamera::GetLiveViewImageCapability(LPRefObj pRefSrc, LPNkMAIDArray pstArray)
{
	ULONG	ulHeaderSize = 0;		//The header size of LiveView
	int i = 0;	
	BOOL	bRet = true;
	
	bRet = GetArrayCapability(pRefSrc, kNkMAIDCapability_GetLiveViewImage, pstArray);
	if (bRet == false) 
		return false;
	return true;
}

BOOL NKCamera::GetCameraImage(LPRefObj pRefSrc)
{

	BOOL bRet = IssueProcess(pRefSrc, kNkMAIDCapability_CaptureAsync);
	Command_Async(pRefSrc->pObject);
	return bRet;
}

BOOL NKCamera::SetStringCapability(LPRefObj pRefObj, ULONG ulCapID, char  newcap)
{
	BOOL	bRet;
	NkMAIDString	stString;
	LPNkMAIDCapInfo pCapInfo = GetCapInfo(pRefObj, ulCapID);
	if (pCapInfo == NULL) return false;

	// check data type of the capability
	if (pCapInfo->ulType != kNkMAIDCapType_String) return false;
	// check if this capability supports CapGet operation.
	if (!CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Get)) return false;

	bRet = Command_CapGet(pRefObj->pObject, ulCapID, kNkMAIDDataType_StringPtr, (NKPARAM)&stString, NULL, NULL);
	if (bRet == false) return false;
	// show current value of this capability
	qDebug() << ("[%s]\n", pCapInfo->szDescription);
	qDebug() << ("Current String: %s\n", stString.str);

	if (CheckCapabilityOperation(pRefObj, ulCapID, kNkMAIDCapOperation_Set)) {
		// This capablity can be set.
		
		//stString.str  ,newcap;
		bRet = Command_CapSet(pRefObj->pObject, ulCapID, kNkMAIDDataType_StringPtr, (NKPARAM)&newcap, NULL, NULL);
		if (bRet == false) return false;
	}
	else {
		// This capablity is read-only.
		qDebug() << ("This value cannot be changed. Enter '0' to exit.\n>");
		//scanf("%s", stString.str);
	}
	return true;
}
