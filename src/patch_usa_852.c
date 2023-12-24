#include "patch_usa_852.h"

typedef struct _stMsgObject {
	void* m_pIActor;
	uint32_t m_id;
	uint32_t m_com[5];
	uint32_t m_time;
} stMsgObject;

inline stMsgObject MakeMsgObject(void* _pIActor, uint32_t _id, uint32_t _c0, uint32_t _c1, uint32_t _c2, uint32_t _c3, uint32_t _c4) {
	return (stMsgObject) { _pIActor, _id, _c0, _c1, _c2, _c3, _c4, 0u };
}

#define MAKEPROTOTYPETHISFUNCTION(_return_, _fnPrototype_, _type_this_, ...) typedef _return_ (__thiscall *_fnPrototype_)(_type_this_, uint32_t/*register EDX*/, __VA_ARGS__)
#define CALLTHISFUNCTION(_func_, _this_, ...) _func_(_this_, 0u/*register EDX*/, __VA_ARGS__)

#define MAKEVIRTUALCALL(_fnPrototype_, _addr_, _this_) (*(_fnPrototype_*)(*((uint8_t**)(_this_)) + (_addr_)))
#define CALLTHISVIRTUALCALL(_fnPrototype_, _addr_, _this_, ...) CALLTHISFUNCTION(MAKEVIRTUALCALL(_fnPrototype_, _addr_, _this_), _this_, __VA_ARGS__)

#define GETTHISPOINTERMEMBER(_type_, _addr_, _this_) (_type_*)(((uint8_t*)(_this_)) + (_addr_))

typedef void* (__cdecl *PFNMAKEINSTANCEFRLOGINRSDLG)(void);
typedef const char* (__fastcall *PFNBRASILTRANSLATESTRINGPROC)(const char* _str);
typedef void(__fastcall *PFNFRLOGINRSDLGSETSTATEPROC)(uint32_t _state);

MAKEPROTOTYPETHISFUNCTION(void*, PFNFRFORM_INITPROC, void*, void*, void*, const char*, void*);
MAKEPROTOTYPETHISFUNCTION(void*, PFNIOBJECTDYNAMICCASTPROC, void*, const void*);
MAKEPROTOTYPETHISFUNCTION(void*, PFNFRFORMLOGINRSDLGSCALARDELETINGDESTRUCTORPROC, void*, uint32_t);

MAKEPROTOTYPETHISFUNCTION(uint64_t, PFNCSHAREDDOCISCONTROLSERVERSERVICEPROC, void*, uint64_t);
MAKEPROTOTYPETHISFUNCTION(void, PFNVIRTUALIACTORHANDLEMSGPROC, void*, stMsgObject*);
MAKEPROTOTYPETHISFUNCTION(void, PFNVIRTUALIACTORCLOSEALLDIALOGSPROC, void*);
MAKEPROTOTYPETHISFUNCTION(BOOL, PFNWNETWORKSYSTEMISCONNECTEDPROC, void*, uint32_t);
MAKEPROTOTYPETHISFUNCTION(BOOL, PFNWNETWORKSYSTEMINITPROC, void*, uint32_t);
MAKEPROTOTYPETHISFUNCTION(BOOL, PFNWNETWORKSYSTEMSENDMESSAGEAPROC, void*, uint32_t, uint32_t);
MAKEPROTOTYPETHISFUNCTION(void, PFNCONSTRUCTORWSENDPACKETPROC, void*, uint32_t);
MAKEPROTOTYPETHISFUNCTION(void, PFNDESTRUCTORWSENDPACKETPROC, void*);
MAKEPROTOTYPETHISFUNCTION(void, PFNWSENDPACKETSENDPROC, void*, uint32_t);
MAKEPROTOTYPETHISFUNCTION(void, PFNCLOBBYMAINCLEARTOOLTIPPROC, void*, void*);
MAKEPROTOTYPETHISFUNCTION(BOOL, PFNFRLOGINRSDLGONLOGINRSDLGRESULTPROC, void*, int32_t, void*);
MAKEPROTOTYPETHISFUNCTION(BOOL, PFNFRFORMOPENPROC, void*, PFNFRLOGINRSDLGONLOGINRSDLGRESULTPROC, uint32_t, uint32_t, uint32_t, uint32_t);

void* CreateFormFrLoginRsDlg(void* _pFrWndManager, void* _pFrCmdTarget, const char* _name, void* _pFrWnd) {

	static void* g_pFrLoginRsDlgObject = NULL;

	PFNMAKEINSTANCEFRLOGINRSDLG pMakeInstanceFrLoginRsDlg = (PFNMAKEINSTANCEFRLOGINRSDLG)0x00AC4680u;
	PFNFRFORM_INITPROC p_Init = (PFNFRFORM_INITPROC)0x00BCDD40u;
	PFNIOBJECTDYNAMICCASTPROC pDynamicCast = (PFNIOBJECTDYNAMICCASTPROC)0x0047EFF0u;

	const void* pFrLoginRsDlg_m_RTTI = (const void*)0x00ECA148u;

	void* pFrLoginRsDlgObject = pMakeInstanceFrLoginRsDlg();

	if (pFrLoginRsDlgObject == NULL)
		return NULL;

	void* pFrLoginRsDlgObjectAfter = CALLTHISFUNCTION(p_Init, pFrLoginRsDlgObject, _pFrWndManager, _pFrCmdTarget, _name, _pFrWnd);

	if (pFrLoginRsDlgObjectAfter != NULL) {

		// Test to see if it saves the value in tls as it is in pangya
		g_pFrLoginRsDlgObject = pFrLoginRsDlgObjectAfter;

		return CALLTHISFUNCTION(pDynamicCast, pFrLoginRsDlgObjectAfter, pFrLoginRsDlg_m_RTTI);
	}

	if (pFrLoginRsDlgObject != NULL)
		CALLTHISVIRTUALCALL(PFNFRFORMLOGINRSDLGSCALARDELETINGDESTRUCTORPROC, 0x04u, pFrLoginRsDlgObject, 1u);

	return NULL;
}

BOOL __thiscall OnLoginRsDlgResult(void* _this, uint32_t _EDX, int32_t exit_code/*I think*/, void* _pFrForm) {
	(_EDX); // UNREFERENCED_PARAMETER

	if (_this != NULL)
		*GETTHISPOINTERMEMBER(void*, 0x160u, _this) = NULL;

	return TRUE;
}

void __thiscall OnUnderBar_RankingUp(void* _this) {

	PFNCSHAREDDOCISCONTROLSERVERSERVICEPROC pIsControlServerService = (PFNCSHAREDDOCISCONTROLSERVERSERVICEPROC)0x0062E4A0;
	PFNBRASILTRANSLATESTRINGPROC pBrasilTranslateString = (PFNBRASILTRANSLATESTRINGPROC)0x00B3A7E0u;
	PFNWNETWORKSYSTEMISCONNECTEDPROC pIsConnected = (PFNWNETWORKSYSTEMISCONNECTEDPROC)0x00A3AEB0u;
	PFNWNETWORKSYSTEMINITPROC pInit = (PFNWNETWORKSYSTEMINITPROC)0x00A3AD40u;
	PFNWNETWORKSYSTEMSENDMESSAGEAPROC pSendMessageA = (PFNWNETWORKSYSTEMSENDMESSAGEAPROC)0x00A3ADB0u;
	PFNCONSTRUCTORWSENDPACKETPROC pConstructorWSendPacket = (PFNCONSTRUCTORWSENDPACKETPROC)0x00A3BEE0u;
	PFNDESTRUCTORWSENDPACKETPROC pDestructorWSendPacket = (PFNDESTRUCTORWSENDPACKETPROC)0x00A3BCD0u;
	PFNWSENDPACKETSENDPROC pSend = (PFNWSENDPACKETSENDPROC)0x00A3B340u;
	PFNCLOBBYMAINCLEARTOOLTIPPROC pClearToolTip = (PFNCLOBBYMAINCLEARTOOLTIPPROC)0x00656770u;
	PFNFRLOGINRSDLGSETSTATEPROC pSetState = (PFNFRLOGINRSDLGSETSTATEPROC)0x00AC4380u;

	const char* gRankingDisabledServiceMsg = (const char*)0x00D244E8u;
	void* gCSharedDocInstance = (*(void**)0x00E10FACu);
	void* gWNetworkSystemInstance = (*(void**)0x00E3CFD0);
	void* gFresh = (*(void**)0x00EC9640);

	if (CALLTHISFUNCTION(pIsControlServerService, gCSharedDocInstance, 0x10000u) != 0u) {

		if (_this == NULL)
			return;

		stMsgObject msg = MakeMsgObject(_this, 0x23u, (uint32_t)pBrasilTranslateString(gRankingDisabledServiceMsg), 0u, 0u, 0u, 0u);

		CALLTHISVIRTUALCALL(PFNVIRTUALIACTORHANDLEMSGPROC, 0x2Cu, _this, &msg);

		return;
	}

	CALLTHISVIRTUALCALL(PFNVIRTUALIACTORCLOSEALLDIALOGSPROC, 0x3Cu, _this);

	if (CALLTHISFUNCTION(pIsConnected, gWNetworkSystemInstance, 0) == FALSE) {

		CALLTHISFUNCTION(pInit, gWNetworkSystemInstance, 3);
		CALLTHISFUNCTION(pSendMessageA, gWNetworkSystemInstance, 3, 0);
	
	}else {

		uint8_t WSendPacketObject[40u];

		CALLTHISFUNCTION(pConstructorWSendPacket, (void*)WSendPacketObject, 0x47u);
		CALLTHISFUNCTION(pSend, (void*)WSendPacketObject, 0);
		CALLTHISFUNCTION(pDestructorWSendPacket, (void*)WSendPacketObject);
	}

	void* FormWndObject = *GETTHISPOINTERMEMBER(void*, 0x160u, _this);

	if (FormWndObject == NULL) {

		FormWndObject = CreateFormFrLoginRsDlg(*GETTHISPOINTERMEMBER(void*, 0x40u, gFresh), (void*)GETTHISPOINTERMEMBER(void*, 0x18, _this), "login_gs", NULL);

		pSetState(1u);

		CALLTHISVIRTUALCALL(PFNFRFORMOPENPROC, 0x64u, FormWndObject, OnLoginRsDlgResult, 0xFFFFFFE8u, 0, 0, 0x41);
	}

	CALLTHISFUNCTION(pClearToolTip, _this, NULL);

	return;
}