#pragma once
#include <QObject>
#include <qstring.h>
#include <Maid3.h>
#include <Maid3d1.h>
#include <NkEndian.h>
#include <Nkstdint.h>
#include <qwidget.h>
//#include <NkTypes.h>
//#pragma comment(lib);
#pragma pack(push, 2)

typedef struct tagRefObj
{
	LPNkMAIDObject	pObject;
	SLONG lMyID;
	LPVOID pRefParent;
	ULONG ulChildCount;
	LPVOID pRefChildArray;
	ULONG ulCapCount;
	LPNkMAIDCapInfo pCapArray;
} RefObj, *LPRefObj;

typedef struct tagRefCompletionProc
{
	//		BOOL bEnd;
	ULONG* pulCount;
	NKERROR nResult;
	//		LPVOID pcProgressDlg;
	LPVOID pRef;
} RefCompletionProc, *LPRefCompletionProc;

typedef struct tagRefDataProc
{
	LPVOID	pBuffer;
	ULONG	ulOffset;
	ULONG	ulTotalLines;
	SLONG	lID;
} RefDataProc, *LPRefDataProc;

typedef struct tagPSDFileHeader
{
	char	type[5];
	char	space11[1];
	char	space01[6];
	short	Planecount; 	//0004 if RGB, this is 0003
	long	rowPixels;
	long	columnPixels;
	short	bits; 			//0008 means 8bit. 16bit also supported
	short	mode; 			//0004 means CMYK, Gray -- 1, RGB -- 3
	char	space02[14];
} PSDFileHeader, *LPPSDFileHeader;

typedef struct tagRefSpecialCap
{
	ULONG ulCapID;
	ULONG ulCapValue;
	//		ULONG ulCapType;
	ULONG ulUIID;
} RefSpecialCap, *LPRefSpecialCap;

static HINSTANCE	g_hInstModule;
static LPMAIDEntryPointProc	g_pMAIDEntryPoint;
static UCHAR	g_bFileRemoved;
static BOOL		g_bFirstCall;
static ULONG	g_ulProgressValue;
static BOOL	g_bCancel;
//UCHAR	g_bFileRemoved = false;
static ULONG	g_ulCameraType = 0;	// CameraType

#pragma pack(pop)



class NKCamera : public QObject
{
	Q_OBJECT
public:
	NKCamera();
	~NKCamera();
	void SetCameraIndex(
		UINT uiCameraIndex,
		QString strSerialNum);

	void RegisterWndPtr(void * ImageControl = NULL);

	bool Init();
	BOOL Connect();								//默认打开第m_CamIndex个相机
	void DisConnect();
	BOOL SetLiveViewStatus(bool status);
	bool SetISO(int iso);
	//int GetISO();
	bool AutoFocus(int focus);
	//	BOOL GetSnapImage(LPRefObj pRefSrc, LPNkMAIDArray pstArray);
	BOOL StartSnap(UINT ImageNum = 0);				//开始采集，ImageNum参数为空或为0都为连续采集
	BOOL StopSnap();

	BOOL StartGrapViewImage();
	BOOL SaveImage(QString filename, LPNkMAIDArray pstArray);
	BOOL SetExposureTime(int64_t ExposureTime);

	BOOL SetTriggerFunction(BOOL TrigMode);
	void SetShowWnd(QWidget* pWnd);

	inline BYTE* GetImageBuffer() { ; };

	BOOL IsConnected() { return m_bIsConnect; };
	BOOL IsSnapping() { return m_bIsSnaping; };

	inline UINT Width() { return m_uiImageWidth; };
	inline UINT Height() { return m_uiImageHeight; };
	inline UINT Depth() { return m_iImageDepth; };

	BOOL SetExposureDelayTime(double ExposureDelay);
	QString GetDeviceName();


	int GetlistISO(QStringList& isolist);
	int GetImageSize(QStringList& list);


	//	bool LoadModule(void* path);

signals:
	void signalHasSaveImage(QString file);
	void signalHasSaveViewImage(QString file);


private slots:

	void slotGrapImage();

private:
	BOOL			InitConectSlots();
	BOOL            RescanDevice();
	BOOL            Open();                         /// 打开相机
	BOOL            InitConfig();                   /// 初次配置相机
	BOOL            BitmapInfo();                   /// 初始化位图并分配空间
													//BOOL			__SaveImg(const CGrabResultPtr& ptrGrabResult);
	BOOL			__ShowImg(float ElapsedTime);

public:
	int                 m_iShowMode;
	char*			    m_chBmpBuf;					    ///< BIMTAPINFO 存储缓冲区，m_pBmpInfo即指向此缓冲区
	char*               m_chBmpBuf_show;
	char*               m_chBmpBuf_Color;
	BITMAPINFO*         m_pBmpInfo;                     ///< 用来保存图像的结构指针
	BITMAPINFO*         m_pBmpInfo_Gray;                ///< 用来显示图像的结构指针
	BITMAPINFO*         m_pBmpInfo_Color;                ///< 用来显示图像的结构指针
														 //	CBitmapControl*		m_ImageControl;				    ///图片显示控件
	BOOL                m_bSnappingSave;                /// To save images.
	UINT                m_uiImageSaveTotalNum;
	//	CMyQueue*           m_pImgQ;

private:
	LPRefObj	m_pRefMod;
	LPRefObj	m_pRefSrc;

	int					m_iTriggerMode;
	UINT			    m_uiCamIndex;					///	相机编号
	QString             m_strSerialNum;                 /// 相机序列号
	UINT                m_uiImageWidth;                 /// 图像宽度
	UINT                m_uiImageHeight;                /// 图像高度
//	CWnd*               m_pShowWnd;
	BYTE*               m_pImageBuffer;
	BOOL			    m_bShowImg;					    ///< 是否显示图像，默认值为true
	int                 m_iImageDepth;
	//int                 m_iImageWidth_mem;

	NkMAIDArray			m_stArray;
	BOOL				m_bIsSnaping;
	QString				m_sAppRoot;
	BOOL				m_bIsConnect;
	int					m_icurrentValue;
	int					m_icurrentISO;
	//bool				m_bLiveViewStatus;

//	QTimer * m_timerGrapImage;
	BOOL ImportModule();

	void ShowErrorMsg(QString csMessage, int nErrorNum);

	BOOL SetUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, ULONG	newCap);
	//  BOOL SetUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID);
	//  BOOL SetUnsignedCapability(m_pRefSrc, kNkMAIDCapability_SaveMedia, kNkMAIDSaveMedia_SDRAM);
	BOOL SetIntegerCapability(LPRefObj pRefObj, ULONG ulCapID, SLONG	newCap);

	// Show the current setting of a Enum(String Integer) type capability and set a value for it.
	BOOL SetEnumStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, UWORD newCap);
	//  BOOL SetEnumStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum);
		// Show the current setting of a Enum(Packed String) type capability and set a value for it.
	BOOL SetEnumPackedStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, UWORD newCap);

	// Show the current setting of a Enum(Unsigned Integer) type capability and set a value for it.
	BOOL SetEnumUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, UWORD newCap);

	// Distribute the function according to array type.
	BOOL SetEnumCapability(LPRefObj pRefObj, ULONG ulCapID, UWORD newCap);

	BOOL GetEnumCapability(LPRefObj pRefObj, ULONG ulCapID, QStringList& cap);

	BOOL GetEnumUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, QStringList& cap);

	BOOL GetEnumPackedStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, QStringList& cap);

	BOOL GetEnumStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum, QStringList& cap);

	// Show the current setting of a Boolean type capability and set a value for it.
	BOOL SetBoolCapability(LPRefObj pRefObj, ULONG ulCapID, bool newCap);

	BOOL GetLiveViewImageCapability(LPRefObj pRefSrc, LPNkMAIDArray pstArray);

	BOOL GetCameraImage(LPRefObj pRefSrc);

	BOOL SetStringCapability(LPRefObj pRefObj, ULONG ulCapID, char newcap);
};


SLONG	CallMAIDEntryPoint(
	LPNkMAIDObject		pObject,				// module, source, item, or data object
	ULONG				ulCommand,			// Command, one of eNkMAIDCommand
	ULONG				ulParam,				// parameter for the command
	ULONG				ulDataType,			// Data type, one of eNkMAIDDataType
	NKPARAM				data,					// Pointer or long integer
	LPNKFUNC			pfnComplete,		// Completion function, may be NULL
	NKREF				refComplete);		// Value passed to pfnComplete
BOOL	Command_Async(LPNkMAIDObject pObject);
BOOL	Command_CapSet(LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete);
BOOL	Command_CapGet(LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete);
BOOL	Command_CapSetSB(LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult);
BOOL	Command_CapGetSB(LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult);
BOOL	Command_CapStart(LPNkMAIDObject pObject, ULONG ulParam, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult);
BOOL	Command_CapStartGeneric(LPNkMAIDObject pObject, ULONG ulParam, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete, SLONG* pnResult);
BOOL	Command_CapGetArray(LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete);
BOOL	Command_CapGetDefault(LPNkMAIDObject pObject, ULONG ulParam, ULONG ulDataType, NKPARAM pData, LPNKFUNC pfnComplete, NKREF refComplete);
BOOL	Command_Abort(LPNkMAIDObject pobject, LPNKFUNC pfnComplete, NKREF refComplete);
BOOL	Command_Open(LPNkMAIDObject pParentObj, NkMAIDObject* pChildObj, ULONG ulChildID);
BOOL	Command_Close(LPNkMAIDObject pObject);

void	InitRefObj(LPRefObj pRef);
BOOL	Search_Module(void* Path);
BOOL	Load_Module(void* Path);
BOOL	Close_Module(LPRefObj pRefMod);
BOOL	EnumCapabilities(LPNkMAIDObject pobject, ULONG* pulCapCount, LPNkMAIDCapInfo* ppCapArray, LPNKFUNC pfnComplete, NKREF refComplete);
BOOL	EnumChildrten(LPNkMAIDObject pobject);
BOOL	AddChild(LPRefObj pRefParent, SLONG lIDChild);
BOOL	RemoveChild(LPRefObj pRefParent, SLONG lIDChild);
BOOL	SetProc(LPRefObj pRefObj);
BOOL	ResetProc(LPRefObj pRefObj);
BOOL	IdleLoop(LPNkMAIDObject pObject, ULONG* pulCount, ULONG ulEndCount);
void WaitEvent(void);

BOOL	SourceCommandLoop(LPRefObj pRefMod, ULONG ulSrcID);
BOOL	ItemCommandLoop(LPRefObj pRefSrc, ULONG ulItemID);
BOOL	ImageCommandLoop(LPRefObj pRefItm, ULONG ulDatID);
BOOL	MovieCommandLoop(LPRefObj pRefItm, ULONG ulDatID);
BOOL	ThumbnailCommandLoop(LPRefObj pRefItm, ULONG ulDatID);
BOOL	SelectSource(LPRefObj pRefMod, ULONG *pulSrcID);
BOOL	SelectItem(LPRefObj pRefSrc, ULONG *pulItemID);
BOOL	SelectData(LPRefObj pRefItm, ULONG *pulDataType);
BOOL	CheckDataType(LPRefObj pRefItm, ULONG *pulDataType);
BOOL	SetUpCamera1(LPRefObj pRefSrc);
BOOL	SetUpCamera2(LPRefObj pRefSrc);
BOOL	SetShootingMenu(LPRefObj pRefSrc);
BOOL	SetLiveView(LPRefObj pRefSrc);
BOOL	SetCustomSettings(LPRefObj pRefSrc);
BOOL	SetSBMenu(LPRefObj pRefSrc);
BOOL	SetEnumCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	SetEnumUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum);
BOOL	SetEnumPackedStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum);
BOOL	SetEnumStringCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDEnum pstEnum);
BOOL	SetFloatCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	SetBoolCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	SetIntegerCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	SetUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	GetUnsignedCapability(LPRefObj pRefObj, ULONG ulCapID, ULONG* pulValue);
BOOL	SetStringCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	SetSizeCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	SetDateTimeCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	SetRangeCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	SetWBPresetDataCapability(LPRefObj pRefSrc);
BOOL	DeleteDramCapability(LPRefObj pRefItem, ULONG ulItmID);
BOOL	GetLiveViewImageCapability(LPRefObj pRefSrc);
BOOL	PictureControlDataCapability(LPRefObj pRefSrc, ULONG ulCapID);
BOOL	SetPictureControlDataCapability(LPRefObj pRefObj, NkMAIDPicCtrlData* pPicCtrlData, char* filename, ULONG ulCapID);
BOOL	GetPictureControlDataCapability(LPRefObj pRefObj, NkMAIDPicCtrlData* pPicCtrlData, ULONG ulCapID);
BOOL	GetPictureControlInfoCapability(LPRefObj pRefSrc);
BOOL	DeleteCustomPictureControlCapability(LPRefObj pRefSrc);
BOOL	GetSBHandlesCapability(LPRefObj pRefObj);
BOOL	GetSBAttrDescCapability(LPRefObj pRefObj);
BOOL	SBAttrValueCapability(LPRefObj pRefObj);
BOOL	SetSBAttrValueCapability(LPRefObj pRefObj, NkMAIDSBAttrValue *pstSbAttrValue);
BOOL	GetSBAttrValueCapability(LPRefObj pRefObj, NkMAIDSBAttrValue *pstSbAttrValue);
BOOL	GetSBGroupAttrDescCapability(LPRefObj pRefObj);
BOOL	SBGroupAttrValueCapability(LPRefObj pRefObj);
BOOL	SetSBGroupAttrValueCapability(LPRefObj pRefObj, NkMAIDSBGroupAttrValue *pstSbGroupAttrValue);
BOOL	GetSBGroupAttrValueCapability(LPRefObj pRefObj, NkMAIDSBGroupAttrValue *pstSbGroupAttrValue);
BOOL	TestFlashCapability(LPRefObj pRefObj);
BOOL	ShowArrayCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	GetArrayCapability(LPRefObj pRefObj, ULONG ulCapID, LPNkMAIDArray pstArray);
BOOL	LoadArrayCapability(LPRefObj pRefObj, ULONG ulCapID, char* filename);
BOOL	SetNewLut(LPRefObj pRefSrc);
char*	GetEnumString(ULONG ulCapID, ULONG ulValue, char *psString);
char*	GetUnsignedString(ULONG ulCapID, ULONG ulValue, char *psString);
BOOL	IssueProcess(LPRefObj pRefSrc, ULONG ulCapID);
BOOL	IssueProcessSync(LPRefObj pRefSrc, ULONG ulCapID);
BOOL	IssueAcquire(LPRefObj pRefDat);
BOOL	GetVideoImageCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	IssueThumbnail(LPRefObj pRefSrc);
BOOL	SetPointCapability(LPRefObj pRefObj, ULONG ulCapID);
BOOL	TerminateCaptureCapability(LPRefObj pRefSrc);
BOOL	GetRecordingInfoCapability(LPRefObj pRefObj);

LPNkMAIDCapInfo	GetCapInfo(LPRefObj pRef, ULONG ulID);
BOOL	CheckCapabilityOperation(LPRefObj pRef, ULONG ulID, ULONG ulOperations);
LPRefObj	GetRefChildPtr_Index(LPRefObj pRefParent, ULONG ulIndex);
LPRefObj	GetRefChildPtr_ID(LPRefObj pRefParent, SLONG lIDChild);
BOOL WINAPI	cancelhandler(DWORD dwCtrlType);
void	CALLPASCAL CALLBACK ProgressProc(ULONG ulCommand, ULONG ulParam, NKREF refProc, ULONG ulDone, ULONG ulTotal);
NKERROR	CALLPASCAL CALLBACK DataProc(NKREF ref, LPVOID pDataInfo, LPVOID pData);
void	CALLPASCAL CALLBACK ModEventProc(NKREF refProc, ULONG ulEvent, NKPARAM data);
void	CALLPASCAL CALLBACK SrcEventProc(NKREF refProc, ULONG ulEvent, NKPARAM data);
void	CALLPASCAL CALLBACK ItmEventProc(NKREF refProc, ULONG ulEvent, NKPARAM data);
void	CALLPASCAL CALLBACK DatEventProc(NKREF refProc, ULONG ulEvent, NKPARAM data);

ULONG	CALLPASCAL CALLBACK UIRequestProc(NKREF ref, LPNkMAIDUIRequestInfo pUIRequest);
void	CALLPASCAL CALLBACK CompletionProc(LPNkMAIDObject pObject, ULONG ulCommand, ULONG ulParam, ULONG ulDataType, NKPARAM data, NKREF refComplete, NKERROR nResult);