#include "ranking.h"
#include "../../../patch.h"

typedef struct _stMsgObject {
    LPVOID m_pIActor;
    DWORD m_id;
    DWORD m_com[5];
    DWORD m_time;
} stMsgObject;

inline static stMsgObject MakeMsgObject(LPVOID _pIActor, DWORD _id, DWORD _c0, DWORD _c1, DWORD _c2, DWORD _c3, DWORD _c4) {
    stMsgObject obj;
    obj.m_pIActor = _pIActor;
    obj.m_id = _id;
    obj.m_com[0] = _c0;
    obj.m_com[1] = _c1;
    obj.m_com[2] = _c2;
    obj.m_com[3] = _c3;
    obj.m_com[4] = _c4;
    obj.m_time = 0u;
    return obj;
}

#define GETTHISPOINTERMEMBER(_type_, _addr_, _this_) (_type_ *)(((BYTE *)(_this_)) + (_addr_))

typedef LPVOID (__cdecl *PFNMAKEINSTANCEFRLOGINRSDLG)(void);
typedef LPCSTR (STDCALL *PFNBRASILTRANSLATESTRINGPROC)(LPCSTR _str);
typedef VOID (STDCALL *PFNFRLOGINRSDLGSETSTATEPROC)(DWORD _state);

typedef LPVOID (STDCALL *PFNFRFORM_INITPROC)(LPVOID, LPVOID, LPVOID, LPCSTR, LPVOID);
typedef LPVOID (STDCALL *PFNIOBJECTDYNAMICCASTPROC)(LPVOID, LPCVOID);
typedef LPVOID (STDCALL *PFNFRFORMLOGINRSDLGSCALARDELETINGDESTRUCTORPROC)(LPVOID, DWORD);

typedef DWORD64 (STDCALL *PFNCSHAREDDOCISCONTROLSERVERSERVICEPROC)(LPVOID, DWORD64);
typedef VOID (STDCALL *PFNVIRTUALIACTORHANDLEMSGPROC)(LPVOID, stMsgObject *);
typedef VOID (STDCALL *PFNVIRTUALIACTORCLOSEALLDIALOGSPROC)(LPVOID);
typedef BOOL (STDCALL *PFNWNETWORKSYSTEMISCONNECTEDPROC)(LPVOID, DWORD);
typedef BOOL (STDCALL *PFNWNETWORKSYSTEMINITPROC)(LPVOID, DWORD);
typedef BOOL (STDCALL *PFNWNETWORKSYSTEMSENDMESSAGEAPROC)(LPVOID, DWORD, DWORD);
typedef VOID (STDCALL *PFNCONSTRUCTORWSENDPACKETPROC)(LPVOID, DWORD);
typedef VOID (STDCALL *PFNDESTRUCTORWSENDPACKETPROC)(LPVOID);
typedef VOID (STDCALL *PFNWSENDPACKETSENDPROC)(LPVOID, DWORD);
typedef VOID (STDCALL *PFNCLOBBYMAINCLEARTOOLTIPPROC)(LPVOID, LPVOID);
typedef BOOL (STDCALL *PFNFRLOGINRSDLGONLOGINRSDLGRESULTPROC)(LPVOID, DWORD, LPVOID);
typedef BOOL (STDCALL *PFNFRFORMOPENPROC)(LPVOID, LPVOID, DWORD, DWORD, DWORD, DWORD);

static PFNMAKEINSTANCEFRLOGINRSDLG pMakeInstanceFrLoginRsDlg = NULL;
static PFNFRFORM_INITPROC p_Init = NULL;
static PFNIOBJECTDYNAMICCASTPROC pDynamicCast = NULL;

static PFNCSHAREDDOCISCONTROLSERVERSERVICEPROC pIsControlServerService = NULL;
static PFNBRASILTRANSLATESTRINGPROC pBrasilTranslateString = NULL;
static PFNWNETWORKSYSTEMISCONNECTEDPROC pIsConnected = NULL;
static PFNWNETWORKSYSTEMINITPROC pInit = NULL;
static PFNWNETWORKSYSTEMSENDMESSAGEAPROC pSendMessageA = NULL;
static PFNCONSTRUCTORWSENDPACKETPROC pConstructorWSendPacket = NULL;
static PFNDESTRUCTORWSENDPACKETPROC pDestructorWSendPacket = NULL;
static PFNWSENDPACKETSENDPROC pSend = NULL;
static PFNCLOBBYMAINCLEARTOOLTIPPROC pClearToolTip = NULL;
static PFNFRLOGINRSDLGSETSTATEPROC pSetState = NULL;
static LPVOID pOnUnderBar_RankingUpThunk = NULL;
static LPVOID pOnLoginRsDlgResultThunk = NULL;

static PFNFRFORMLOGINRSDLGSCALARDELETINGDESTRUCTORPROC pPerformLoginRsDlgScalarDeletingDestructor =
    NULL;
static PFNVIRTUALIACTORHANDLEMSGPROC pIActorHandleMsg = NULL;
static PFNVIRTUALIACTORCLOSEALLDIALOGSPROC pIActorCloseAllDialogs = NULL;
static PFNFRFORMOPENPROC pFrFormOpen = NULL;

static void STDCALL OnUnderBar_RankingUp(LPVOID _this);
static BOOL STDCALL OnLoginRsDlgResult(LPVOID _this, DWORD exit_code, LPVOID _pFrForm);

VOID InitUS852RankingHook() {
    pMakeInstanceFrLoginRsDlg = (PFNMAKEINSTANCEFRLOGINRSDLG)0x00AC4680u;
    p_Init = (PFNFRFORM_INITPROC)BuildStdcallToThiscallThunk((LPVOID)0x00BCDD40u);
    pDynamicCast = (PFNIOBJECTDYNAMICCASTPROC)BuildStdcallToThiscallThunk((LPVOID)0x0047EFF0u);

    pIsControlServerService =
        (PFNCSHAREDDOCISCONTROLSERVERSERVICEPROC)BuildStdcallToThiscallThunk((LPVOID)0x0062E4A0);
    pBrasilTranslateString =
        (PFNBRASILTRANSLATESTRINGPROC)BuildStdcallToThiscallThunk((LPVOID)0x00B3A7E0u);
    pIsConnected =
        (PFNWNETWORKSYSTEMISCONNECTEDPROC)BuildStdcallToThiscallThunk((LPVOID)0x00A3AEB0u);
    pInit = (PFNWNETWORKSYSTEMINITPROC)BuildStdcallToThiscallThunk((LPVOID)0x00A3AD40u);
    pSendMessageA =
        (PFNWNETWORKSYSTEMSENDMESSAGEAPROC)BuildStdcallToThiscallThunk((LPVOID)0x00A3ADB0u);
    pConstructorWSendPacket =
        (PFNCONSTRUCTORWSENDPACKETPROC)BuildStdcallToThiscallThunk((LPVOID)0x00A3BEE0u);
    pDestructorWSendPacket =
        (PFNDESTRUCTORWSENDPACKETPROC)BuildStdcallToThiscallThunk((LPVOID)0x00A3BCD0u);
    pSend = (PFNWSENDPACKETSENDPROC)BuildStdcallToThiscallThunk((LPVOID)0x00A3B340u);
    pClearToolTip = (PFNCLOBBYMAINCLEARTOOLTIPPROC)BuildStdcallToThiscallThunk((LPVOID)0x00656770u);
    pSetState = (PFNFRLOGINRSDLGSETSTATEPROC)BuildStdcallToThiscallThunk((LPVOID)0x00AC4380u);
    pOnUnderBar_RankingUpThunk = BuildThiscallToStdcallThunk((LPVOID)OnUnderBar_RankingUp);
    pOnLoginRsDlgResultThunk = BuildThiscallToStdcallThunk((LPVOID)OnLoginRsDlgResult);

    pPerformLoginRsDlgScalarDeletingDestructor =
        (PFNFRFORMLOGINRSDLGSCALARDELETINGDESTRUCTORPROC)BuildStdcallToVirtualThiscallThunk(0x04u);
    pIActorHandleMsg =
        (PFNVIRTUALIACTORHANDLEMSGPROC)BuildStdcallToVirtualThiscallThunk(0x2Cu);
    pIActorCloseAllDialogs =
        (PFNVIRTUALIACTORCLOSEALLDIALOGSPROC)BuildStdcallToVirtualThiscallThunk(0x3Cu);
    pFrFormOpen =
        (PFNFRFORMOPENPROC)BuildStdcallToVirtualThiscallThunk(0x64u);

    InstallHook((LPVOID)0x00655630, (LPVOID)pOnUnderBar_RankingUpThunk);
}

static LPVOID CreateFormFrLoginRsDlg(LPVOID _pFrWndManager, LPVOID _pFrCmdTarget, LPCSTR _name,
                              LPVOID _pFrWnd) {
    static LPVOID g_pFrLoginRsDlgObject = NULL;

    LPCVOID pFrLoginRsDlg_m_RTTI = (LPCVOID)0x00ECA148u;

    LPVOID pFrLoginRsDlgObject = pMakeInstanceFrLoginRsDlg();

    LPVOID pFrLoginRsDlgObjectAfter;

    if (pFrLoginRsDlgObject == NULL) {
        return NULL;
    }

    pFrLoginRsDlgObjectAfter =
        p_Init(pFrLoginRsDlgObject, _pFrWndManager, _pFrCmdTarget, _name, _pFrWnd);

    if (pFrLoginRsDlgObjectAfter != NULL) {
        // Test to see if it saves the value in tls as it is in pangya
        g_pFrLoginRsDlgObject = pFrLoginRsDlgObjectAfter;

        return pDynamicCast(pFrLoginRsDlgObjectAfter, pFrLoginRsDlg_m_RTTI);
    }

    if (pFrLoginRsDlgObject != NULL) {
        pPerformLoginRsDlgScalarDeletingDestructor(pFrLoginRsDlgObject, 1u);
    }

    return NULL;
}

static BOOL STDCALL OnLoginRsDlgResult(LPVOID _this, DWORD exit_code /*I think*/, LPVOID _pFrForm) {
    if (_this != NULL) {
        *GETTHISPOINTERMEMBER(LPVOID, 0x160u, _this) = NULL;
    }

    return TRUE;
}

static void STDCALL OnUnderBar_RankingUp(LPVOID _this) {
    LPCSTR gRankingDisabledServiceMsg = (LPCSTR)0x00D244E8u;
    LPVOID gCSharedDocInstance = (*(LPVOID *)0x00E10FACu);
    LPVOID gWNetworkSystemInstance = (*(LPVOID *)0x00E3CFD0);
    LPVOID gFresh = (*(LPVOID *)0x00EC9640);
    LPVOID *FormWndObject = NULL;

    if (pIsControlServerService(gCSharedDocInstance, 0x10000u) != 0u) {
        LPCSTR translatedMsgStr;
        stMsgObject msg;

        if (_this == NULL) {
            return;
        }

        translatedMsgStr = pBrasilTranslateString(gRankingDisabledServiceMsg);
        msg = MakeMsgObject(_this, 0x23u, (DWORD)translatedMsgStr, 0u, 0u, 0u, 0u);

        pIActorHandleMsg(_this, &msg);
    }

    pIActorCloseAllDialogs(_this);

    if (pIsConnected(gWNetworkSystemInstance, 0) == FALSE) {
        pInit(gWNetworkSystemInstance, 3);
        pSendMessageA(gWNetworkSystemInstance, 3, 0);
    } else {
        BYTE WSendPacketObject[40u];
        pConstructorWSendPacket((LPVOID)WSendPacketObject, 0x47u);
        pSend((LPVOID)WSendPacketObject, 0);
        pDestructorWSendPacket((LPVOID)WSendPacketObject);
    }

    FormWndObject = GETTHISPOINTERMEMBER(void *, 0x160u, _this);

    if (*FormWndObject == NULL) {
        *FormWndObject = CreateFormFrLoginRsDlg(*GETTHISPOINTERMEMBER(void *, 0x40u, gFresh),
                                                (void *)GETTHISPOINTERMEMBER(void *, 0x18, _this),
                                                "login_gs", NULL);

        pSetState(1u);

        pFrFormOpen(*FormWndObject, pOnLoginRsDlgResultThunk, 0xFFFFFFE8u, 0, 0, 0x41);
    }

    pClearToolTip(_this, NULL);

    return;
}
