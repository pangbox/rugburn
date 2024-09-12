/**
 * Copyright 2018-2024 John Chadwick <john@jchw.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose
 * with or without fee is hereby granted, provided that the above copyright notice
 * and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS
 * OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
 * THIS SOFTWARE.
 */

/*
 * This file contains many function pointer types and structures for Windows
 * APIs. Many of these structures are not actually needed for this patcher,
 * but were used in the process of developing it. These types make it easy to
 * hook various APIs to manipulate or inspect behavior.
 */

#pragma once

#include <windows.h>
#include <ws2tcpip.h>

// IWebBrowser 1 and 2
#ifdef __MINGW32__
#ifndef __IWebBrowser2_FWD_DEFINED__
#define __IWebBrowser2_FWD_DEFINED__
typedef struct IWebBrowser2 IWebBrowser2;

#endif /* __IWebBrowser2_FWD_DEFINED__ */

typedef /* [v1_enum] */
enum tagREADYSTATE {
    READYSTATE_UNINITIALIZED = 0,
    READYSTATE_LOADING = 1,
    READYSTATE_LOADED = 2,
    READYSTATE_INTERACTIVE = 3,
    READYSTATE_COMPLETE = 4
} READYSTATE;

typedef 
enum OLECMDF
{
    OLECMDF_SUPPORTED	= 0x1,
    OLECMDF_ENABLED	= 0x2,
    OLECMDF_LATCHED	= 0x4,
    OLECMDF_NINCHED	= 0x8,
    OLECMDF_INVISIBLE	= 0x10,
    OLECMDF_DEFHIDEONCTXTMENU	= 0x20
} 	OLECMDF;

typedef 
enum OLECMDID
{
    OLECMDID_OPEN	= 1,
    OLECMDID_NEW	= 2,
    OLECMDID_SAVE	= 3,
    OLECMDID_SAVEAS	= 4,
    OLECMDID_SAVECOPYAS	= 5,
    OLECMDID_PRINT	= 6,
    OLECMDID_PRINTPREVIEW	= 7,
    OLECMDID_PAGESETUP	= 8,
    OLECMDID_SPELL	= 9,
    OLECMDID_PROPERTIES	= 10,
    OLECMDID_CUT	= 11,
    OLECMDID_COPY	= 12,
    OLECMDID_PASTE	= 13,
    OLECMDID_PASTESPECIAL	= 14,
    OLECMDID_UNDO	= 15,
    OLECMDID_REDO	= 16,
    OLECMDID_SELECTALL	= 17,
    OLECMDID_CLEARSELECTION	= 18,
    OLECMDID_ZOOM	= 19,
    OLECMDID_GETZOOMRANGE	= 20,
    OLECMDID_UPDATECOMMANDS	= 21,
    OLECMDID_REFRESH	= 22,
    OLECMDID_STOP	= 23,
    OLECMDID_HIDETOOLBARS	= 24,
    OLECMDID_SETPROGRESSMAX	= 25,
    OLECMDID_SETPROGRESSPOS	= 26,
    OLECMDID_SETPROGRESSTEXT	= 27,
    OLECMDID_SETTITLE	= 28,
    OLECMDID_SETDOWNLOADSTATE	= 29,
    OLECMDID_STOPDOWNLOAD	= 30,
    OLECMDID_ONTOOLBARACTIVATED	= 31,
    OLECMDID_FIND	= 32,
    OLECMDID_DELETE	= 33,
    OLECMDID_HTTPEQUIV	= 34,
    OLECMDID_HTTPEQUIV_DONE	= 35,
    OLECMDID_ENABLE_INTERACTION	= 36,
    OLECMDID_ONUNLOAD	= 37,
    OLECMDID_PROPERTYBAG2	= 38,
    OLECMDID_PREREFRESH	= 39,
    OLECMDID_SHOWSCRIPTERROR	= 40,
    OLECMDID_SHOWMESSAGE	= 41,
    OLECMDID_SHOWFIND	= 42,
    OLECMDID_SHOWPAGESETUP	= 43,
    OLECMDID_SHOWPRINT	= 44,
    OLECMDID_CLOSE	= 45,
    OLECMDID_ALLOWUILESSSAVEAS	= 46,
    OLECMDID_DONTDOWNLOADCSS	= 47,
    OLECMDID_UPDATEPAGESTATUS	= 48,
    OLECMDID_PRINT2	= 49,
    OLECMDID_PRINTPREVIEW2	= 50,
    OLECMDID_SETPRINTTEMPLATE	= 51,
    OLECMDID_GETPRINTTEMPLATE	= 52,
    OLECMDID_PAGEACTIONBLOCKED	= 55,
    OLECMDID_PAGEACTIONUIQUERY	= 56,
    OLECMDID_FOCUSVIEWCONTROLS	= 57,
    OLECMDID_FOCUSVIEWCONTROLSQUERY	= 58,
    OLECMDID_SHOWPAGEACTIONMENU	= 59,
    OLECMDID_ADDTRAVELENTRY	= 60,
    OLECMDID_UPDATETRAVELENTRY	= 61,
    OLECMDID_UPDATEBACKFORWARDSTATE	= 62,
    OLECMDID_OPTICAL_ZOOM	= 63,
    OLECMDID_OPTICAL_GETZOOMRANGE	= 64,
    OLECMDID_WINDOWSTATECHANGED	= 65,
    OLECMDID_ACTIVEXINSTALLSCOPE	= 66,
    OLECMDID_UPDATETRAVELENTRY_DATARECOVERY	= 67,
    OLECMDID_SHOWTASKDLG	= 68,
    OLECMDID_POPSTATEEVENT	= 69,
    OLECMDID_VIEWPORT_MODE	= 70,
    OLECMDID_LAYOUT_VIEWPORT_WIDTH	= 71,
    OLECMDID_VISUAL_VIEWPORT_EXCLUDE_BOTTOM	= 72,
    OLECMDID_USER_OPTICAL_ZOOM	= 73,
    OLECMDID_PAGEAVAILABLE	= 74,
    OLECMDID_GETUSERSCALABLE	= 75,
    OLECMDID_UPDATE_CARET	= 76,
    OLECMDID_ENABLE_VISIBILITY	= 77,
    OLECMDID_MEDIA_PLAYBACK	= 78,
    OLECMDID_SETFAVICON	= 79,
    OLECMDID_SET_HOST_FULLSCREENMODE	= 80,
    OLECMDID_EXITFULLSCREEN	= 81,
    OLECMDID_SCROLLCOMPLETE	= 82,
    OLECMDID_ONBEFOREUNLOAD	= 83,
    OLECMDID_SHOWMESSAGE_BLOCKABLE	= 84,
    OLECMDID_SHOWTASKDLG_BLOCKABLE	= 85
} 	OLECMDID;

typedef 
enum OLECMDEXECOPT
{
    OLECMDEXECOPT_DODEFAULT	= 0,
    OLECMDEXECOPT_PROMPTUSER	= 1,
    OLECMDEXECOPT_DONTPROMPTUSER	= 2,
    OLECMDEXECOPT_SHOWHELP	= 3
} 	OLECMDEXECOPT;

typedef 
struct IWebBrowser2Vtbl
{

	BEGIN_INTERFACE
        
    HRESULT ( STDCALL *QueryInterface )( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ __RPC__in REFIID riid,
        /* [annotation][iid_is][out] */ 
        _COM_Outptr_  void **ppvObject);
        
    ULONG(STDCALL *AddRef)( 
        __RPC__in IWebBrowser2 * This);
        
    ULONG(STDCALL *Release)( 
        __RPC__in IWebBrowser2 * This);
        
    HRESULT(STDCALL *GetTypeInfoCount)
    ( 
        __RPC__in IWebBrowser2 * This,
        /* [out] */ __RPC__out UINT *pctinfo);
        
    HRESULT(STDCALL *GetTypeInfo)
    ( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ UINT iTInfo,
        /* [in] */ LCID lcid,
        /* [out] */ __RPC__deref_out_opt ITypeInfo **ppTInfo);
        
    HRESULT(STDCALL *GetIDsOfNames)
    ( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ __RPC__in REFIID riid,
        /* [size_is][in] */ __RPC__in_ecount_full(cNames) LPOLESTR *rgszNames,
        /* [range][in] */ __RPC__in_range(0,16384) UINT cNames,
        /* [in] */ LCID lcid,
        /* [size_is][out] */ __RPC__out_ecount_full(cNames) DISPID *rgDispId);
        
    /* [local] */ HRESULT(STDCALL *Invoke)( 
        IWebBrowser2 * This,
        /* [annotation][in] */ 
        _In_  DISPID dispIdMember,
        /* [annotation][in] */ 
        _In_  REFIID riid,
        /* [annotation][in] */ 
        _In_  LCID lcid,
        /* [annotation][in] */ 
        _In_  WORD wFlags,
        /* [annotation][out][in] */ 
        _In_  DISPPARAMS *pDispParams,
        /* [annotation][out] */ 
        _Out_opt_  VARIANT *pVarResult,
        /* [annotation][out] */ 
        _Out_opt_  EXCEPINFO *pExcepInfo,
        /* [annotation][out] */ 
        _Out_opt_  UINT *puArgErr);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *GoBack)( 
        __RPC__in IWebBrowser2 * This);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *GoForward)( 
        __RPC__in IWebBrowser2 * This);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *GoHome)( 
        __RPC__in IWebBrowser2 * This);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *GoSearch)( 
        __RPC__in IWebBrowser2 * This);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *Navigate)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ __RPC__in BSTR URL,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *Flags,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *TargetFrameName,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *PostData,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *Headers);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *Refresh)( 
        __RPC__in IWebBrowser2 * This);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *Refresh2)( 
        __RPC__in IWebBrowser2 * This,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *Level);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *Stop)( 
        __RPC__in IWebBrowser2 * This);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Application)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt IDispatch **ppDisp);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Parent)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt IDispatch **ppDisp);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Container)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt IDispatch **ppDisp);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Document)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt IDispatch **ppDisp);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_TopLevelContainer)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pBool);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Type)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt BSTR *Type);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Left)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out long *pl);
        
    /* [propput][id] */ HRESULT(STDCALL *put_Left)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ long Left);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Top)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out long *pl);
        
    /* [propput][id] */ HRESULT(STDCALL *put_Top)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ long Top);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Width)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out long *pl);
        
    /* [propput][id] */ HRESULT(STDCALL *put_Width)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ long Width);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Height)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out long *pl);
        
    /* [propput][id] */ HRESULT(STDCALL *put_Height)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ long Height);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_LocationName)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt BSTR *LocationName);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_LocationURL)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt BSTR *LocationURL);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Busy)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pBool);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *Quit)( 
        __RPC__in IWebBrowser2 * This);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *ClientToWindow)( 
        __RPC__in IWebBrowser2 * This,
        /* [out][in] */ __RPC__inout int *pcx,
        /* [out][in] */ __RPC__inout int *pcy);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *PutProperty)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ __RPC__in BSTR Property,
        /* [in] */ VARIANT vtValue);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *GetProperty)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ __RPC__in BSTR Property,
        /* [retval][out] */ __RPC__out VARIANT *pvtValue);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Name)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt BSTR *Name);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_HWND)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out SHANDLE_PTR *pHWND);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_FullName)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt BSTR *FullName);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Path)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt BSTR *Path);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Visible)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pBool);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_Visible)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL Value);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_StatusBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pBool);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_StatusBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL Value);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_StatusText)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__deref_out_opt BSTR *StatusText);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_StatusText)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ __RPC__in BSTR StatusText);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_ToolBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out int *Value);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_ToolBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ int Value);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_MenuBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *Value);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_MenuBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL Value);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_FullScreen)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pbFullScreen);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_FullScreen)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL bFullScreen);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *Navigate2)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ __RPC__in VARIANT *URL,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *Flags,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *TargetFrameName,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *PostData,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *Headers);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *QueryStatusWB)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ OLECMDID cmdID,
        /* [retval][out] */ __RPC__out OLECMDF *pcmdf);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *ExecWB)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ OLECMDID cmdID,
        /* [in] */ OLECMDEXECOPT cmdexecopt,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *pvaIn,
        /* [unique][optional][out][in] */ __RPC__inout_opt VARIANT *pvaOut);
        
    /* [helpcontext][helpstring][id] */ HRESULT(STDCALL *ShowBrowserBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ __RPC__in VARIANT *pvaClsid,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *pvarShow,
        /* [unique][optional][in] */ __RPC__in_opt VARIANT *pvarSize);
        
    /* [bindable][propget][id] */ HRESULT(STDCALL *get_ReadyState)( 
        __RPC__in IWebBrowser2 * This,
        /* [out][retval] */ __RPC__out READYSTATE *plReadyState);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Offline)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pbOffline);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_Offline)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL bOffline);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Silent)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pbSilent);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_Silent)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL bSilent);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_RegisterAsBrowser)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pbRegister);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_RegisterAsBrowser)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL bRegister);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_RegisterAsDropTarget)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pbRegister);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_RegisterAsDropTarget)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL bRegister);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_TheaterMode)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *pbRegister);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_TheaterMode)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL bRegister);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_AddressBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *Value);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_AddressBar)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL Value);
        
    /* [helpcontext][helpstring][propget][id] */ HRESULT(STDCALL *get_Resizable)( 
        __RPC__in IWebBrowser2 * This,
        /* [retval][out] */ __RPC__out VARIANT_BOOL *Value);
        
    /* [helpcontext][helpstring][propput][id] */ HRESULT(STDCALL *put_Resizable)( 
        __RPC__in IWebBrowser2 * This,
        /* [in] */ VARIANT_BOOL Value);
        
    END_INTERFACE
} IWebBrowser2Vtbl;

struct IWebBrowser2 {
    CONST_VTBL struct IWebBrowser2Vtbl *lpVtbl;
};
#else
#include <ExDisp.h>
#endif

#ifdef __MINGW32__

typedef struct _DNS_QUERY_REQUEST {
    ULONG Version;
    PCWSTR QueryName;
    WORD QueryType;
    ULONG64 QueryOptions;
    PVOID pDnsServerList;
    ULONG InterfaceIndex;
    PVOID pQueryCompletionCallback;
    PVOID pQueryContext;
} DNS_QUERY_REQUEST, *PDNS_QUERY_REQUEST;

typedef struct _DNS_QUERY_CANCEL {
    CHAR Reserved[32];
} DNS_QUERY_CANCEL, *PDNS_QUERY_CANCEL;

#endif

typedef struct tagINITCOMMONCONTROLSEX {
    DWORD dwSize; // size of this structure
    DWORD dwICC;  // flags indicating which classes to be initialized
} INITCOMMONCONTROLSEX, *LPINITCOMMONCONTROLSEX;

// comctl32.dll
typedef BOOL(STDCALL *PFNINITCOMMONCONTROLSEXPROC)(const INITCOMMONCONTROLSEX *picce);

// ole32.dll
typedef HRESULT(STDCALL *PFNCOGETCLASSOBJECTPROC)(REFCLSID rclsid, DWORD dwClsContext,
                                                  LPVOID pvReserved, REFIID riid, LPVOID *ppv);
typedef HRESULT(STDCALL *PFNCOCREATEINSTANCEPROC)(REFCLSID rclsid, LPUNKNOWN pUnkOuter,
                                                  DWORD dwClsContext, REFIID riid, LPVOID *ppv);

// oleaut32.dll
typedef BSTR (STDCALL *PFNSYSALLOCSTRINGPROC)(const OLECHAR *psz);
typedef void (STDCALL *PFNSYSFREESTRINGPROC)(BSTR bstrString);

// ieframe.dll
typedef HRESULT(STDCALL *PFNINVOKEPROC)(IDispatch *This, DISPID dispIdMember, REFIID riid,
                                        LCID lcid, WORD wFlags, DISPPARAMS *pDispParams,
                                        VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr);
typedef HRESULT(STDCALL *PFNNAVIGATEPROC)(IWebBrowser2 *This, BSTR URL, VARIANT *Flags,
                                          VARIANT *TargetFrameName, VARIANT *PostData,
                                          VARIANT *Headers);

// general
typedef BOOL(WINAPI *PFNDLLMAINPROC)(HANDLE, DWORD, LPVOID);

// ws2_32.dll
typedef int(STDCALL *PFNCONNECTPROC)(SOCKET, const struct sockaddr *, int);
typedef BOOL(STDCALL *PFNCONNECTEXPROC)(SOCKET, const struct sockaddr *, int, PVOID, DWORD, LPDWORD,
                                        LPOVERLAPPED);
typedef int(STDCALL *PFNWSACONNECTPROC)(SOCKET, const struct sockaddr *, int, LPWSABUF, LPWSABUF,
                                        LPQOS, LPQOS);
typedef WORD(WSAAPI *PFNHTONSPROC)(WORD);
typedef INT(WSAAPI *PFNGETADDRINFO)(PCSTR, PCSTR, const PADDRINFOA, PADDRINFOA *);
typedef VOID(WSAAPI *PFNFREEADDRINFO)(PADDRINFOA);

// dnsapi.dll
typedef DNS_STATUS(STDCALL *PFNDNSQUERYEXPROC)(PDNS_QUERY_REQUEST pQueryRequest,
                                               PDNS_QUERY_RESULT pQueryResults,
                                               PDNS_QUERY_CANCEL pCancelHandle);

// advapi32.dll
typedef BOOL(STDCALL *PFNCRYPTDECRYPTPROC)(HCRYPTKEY, HCRYPTHASH, BOOL, DWORD, BYTE *, DWORD *);
typedef BOOL(STDCALL *PFNCRYPTGETHASHPARAMPROC)(HCRYPTHASH, DWORD, BYTE *, DWORD *, DWORD);
typedef BOOL(STDCALL *PFNCRYPTVERIFYSIGNATUREAPROC)(HCRYPTHASH, CONST BYTE *, DWORD, HCRYPTKEY,
                                                    LPCSTR, DWORD);
typedef BOOL(STDCALL *PFNCRYPTIMPORTKEYPROC)(HCRYPTPROV, CONST BYTE *, DWORD, HCRYPTKEY, DWORD,
                                             HCRYPTKEY *);

// user32.dll
typedef int(STDCALL *PFNMESSAGEBOXAPROC)(HWND, LPCSTR, LPCSTR, UINT);
typedef HWND(STDCALL *PFNCREATEWINDOWEXAPROC)(DWORD, LPCSTR, LPCSTR, DWORD, int, int, int, int,
                                              HWND, HMENU, HINSTANCE, LPVOID);
typedef LRESULT(STDCALL *PFNSENDMESSAGEPROC)(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
typedef BOOL(STDCALL *PFNPEEKMESSAGEAPROC)(LPMSG, HWND, UINT, UINT, UINT);
typedef BOOL(STDCALL *PFNGETMESSAGEAPROC)(LPMSG, HWND, UINT, UINT);
typedef UINT_PTR(STDCALL *PFNSETTIMERPROC)(HWND, UINT_PTR, UINT, TIMERPROC);
typedef int(STDCALL *PFNWVSPRINTFAPROC)(LPSTR, LPCSTR, void *);

// kernel32.dll
typedef HMODULE(STDCALL *PFNLOADLIBRARYAPROC)(LPCSTR);
typedef VOID(STDCALL *PFNFREELIBRARYPROC)(HMODULE);
typedef FARPROC(STDCALL *PFNGETPROCADDRESSPROC)(HMODULE, LPCSTR);
typedef DWORD(STDCALL *PFNGETENVIRONMENTVARIABLEAPROC)(LPCSTR, LPSTR, DWORD);
typedef BOOL(STDCALL *PFNCREATEPROCESSAPROC)(LPCSTR, LPSTR, LPSECURITY_ATTRIBUTES,
                                             LPSECURITY_ATTRIBUTES, BOOL, DWORD, LPVOID, LPCSTR,
                                             LPSTARTUPINFOA, LPPROCESS_INFORMATION);
typedef BOOL(STDCALL *PFNISWOW64PROCESSPROC)(HANDLE, PBOOL);
typedef HANDLE(STDCALL *PFNCREATEFILEAPROC)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD,
                                            DWORD, HANDLE);
typedef HANDLE(STDCALL *PFNCREATEFILEMAPPINGAPROC)(HANDLE, LPSECURITY_ATTRIBUTES, DWORD, DWORD,
                                                   DWORD, LPCSTR);
typedef HANDLE(STDCALL *PFNOPENMUTEXAPROC)(DWORD, BOOL, LPCSTR);
typedef HANDLE(STDCALL *PFNCREATEMUTEXAPROC)(LPSECURITY_ATTRIBUTES, BOOL, LPCSTR);
typedef DWORD(STDCALL *PFNGETCURRENTTHREADIDPROC)(VOID);
typedef DWORD(STDCALL *PFNGETCURRENTPROCESSIDPROC)(VOID);
typedef VOID(STDCALL *PFNSLEEPPROC)(DWORD);
typedef DWORD(STDCALL *PFNSLEEPEXPROC)(DWORD, BOOL);
typedef DWORD(STDCALL *PFNWAITFORSINGLEOBJECTPROC)(HANDLE, DWORD);
typedef HANDLE(STDCALL *PFNOPENEVENTAPROC)(DWORD, BOOL, LPCSTR);
typedef HANDLE(STDCALL *PFNCREATEEVENTAPROC)(LPSECURITY_ATTRIBUTES lpEventAttributes,
                                             BOOL bManualReset, BOOL bInitialState, LPCSTR lpName);
typedef BOOL(STDCALL *PFNGETEXITCODEPROCESSPROC)(HANDLE hProcess, LPDWORD lpExitCode);
typedef HLOCAL(STDCALL *PFNLOCALALLOCPROC)(UINT, UINT);
typedef HLOCAL(STDCALL *PFNLOCALFREEPROC)(HLOCAL);
typedef HMODULE(STDCALL *PFNGETMODULEHANDLEAPROC)(LPCSTR);
typedef DWORD(STDCALL *PFNGETMODULEFILENAMEAPROC)(HMODULE, LPCH, DWORD);
typedef DWORD(STDCALL *PFNGETFILEATTRIBUTESAPROC)(LPCSTR);
typedef VOID(STDCALL *PFNEXITPROCESSPROC)(UINT);
typedef DWORD(STDCALL *PFNGETFILESIZEPROC)(HANDLE, LPDWORD);
typedef BOOL(STDCALL *PFNREADFILEPROC)(HANDLE, LPVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL(STDCALL *PFNWRITEFILEPROC)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED);
typedef BOOL(STDCALL *PFNCLOSEHANDLEPROC)(HANDLE);
typedef LPSTR(STDCALL *PFNLSTRCPYAPROC)(LPSTR, LPCSTR);
typedef DWORD(STDCALL *PFNGETLASTERRORPROC)(VOID);
typedef BOOL(STDCALL *PFNSETENVIRONMENTVARIABLEAPROC)(LPCSTR, LPCSTR);
typedef BOOL(STDCALL *PFNVIRTUALPROTECTEXPROC)(HANDLE, LPVOID, SIZE_T, DWORD, PDWORD);
typedef BOOL(STDCALL *PFNREADPROCESSMEMORYPROC)(HANDLE, LPCVOID, LPVOID, SIZE_T, SIZE_T *);
typedef BOOL(STDCALL *PFNWRITEPROCESSMEMORYPROC)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T *);
typedef BOOL(STDCALL *PFNFLUSHINSTRUCTIONCACHEPROC)(HANDLE, LPCVOID, SIZE_T);
typedef BOOL(STDCALL *PFNVIRTUALPROTECTPROC)(LPVOID, SIZE_T, DWORD, PDWORD);
typedef HANDLE(STDCALL *PFNGETCURRENTPROCESSPROC)(VOID);
typedef HANDLE(STDCALL *PFNCREATETHREADPROC)(LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE,
                                             LPVOID, DWORD, LPDWORD);
typedef HANDLE(STDCALL *PFNGETSTDHANDLEPROC)(DWORD);
typedef SIZE_T(STDCALL *PFNVIRTUALQUERYPROC)(LPCVOID lpAddress, PMEMORY_BASIC_INFORMATION lpBuffer,
                                             SIZE_T dwLength);
typedef VOID(STDCALL *PFNGETSTARTUPINFOAPROC)(LPSTARTUPINFOA lpStartupInfo);
typedef VOID(STDCALL *PFNGETSTARTUPINFOWPROC)(LPSTARTUPINFOW lpStartupInfo);

// wininet.dll
typedef HINTERNET(STDCALL *PFNINTERNETOPENURLAPROC)(HINTERNET, LPCSTR, LPCSTR, DWORD, DWORD,
                                                    DWORD_PTR);
typedef HINTERNET(STDCALL *PFNINTERNETCONNECTAPROC)(HINTERNET hInternet, LPCSTR lpszServerName,
                                                    INTERNET_PORT nServerPort, LPCSTR lpszUserName,
                                                    LPCSTR lpszPassword, DWORD dwService,
                                                    DWORD dwFlags, DWORD_PTR dwContext);
typedef HINTERNET(STDCALL *PFNHTTPOPENREQUESTAPROC)(HINTERNET hConnect, LPCSTR lpszVerb,
                                                    LPCSTR lpszObjectName, LPCSTR lpszVersion,
                                                    LPCSTR lpszReferrer, LPCSTR *lplpszAcceptTypes,
                                                    DWORD dwFlags, DWORD_PTR dwContext);
typedef BOOL(STDCALL *PFNHTTPSENDREQUESTAPROC)(HINTERNET hRequest, LPCSTR lpszHeaders,
                                               DWORD dwHeadersLength, LPVOID lpOptional,
                                               DWORD dwOptionalLength);
typedef BOOL(STDCALL *PFNHTTPSENDREQUESTEXAPROC)(HINTERNET hRequest,
                                                 LPINTERNET_BUFFERSA lpBuffersIn,
                                                 LPINTERNET_BUFFERSA lpBuffersOut, DWORD dwFlags,
                                                 DWORD_PTR dwContext);
typedef BOOL(STDCALL *PFNHTTPENDREQUESTAPROC)(HINTERNET hRequest, LPINTERNET_BUFFERSA lpBuffersOut,
                                              DWORD dwFlags, DWORD_PTR dwContext);
typedef BOOL(STDCALL *PFNHTTPADDREQUESTHEADERSAPROC)(HINTERNET hRequest, LPCSTR lpszHeaders,
                                                     DWORD dwHeadersLength, DWORD dwModifiers);
typedef BOOL(STDCALL *PFNHTTPQUERYINFOAPROC)(HINTERNET hRequest, DWORD dwInfoLevel, LPVOID lpBuffer,
                                             LPDWORD lpdwBufferLength, LPDWORD lpdwIndex);
typedef BOOL(STDCALL *PFNINTERNETCLOSEHANDLEPROC)(HINTERNET hInternet);
typedef BOOL(STDCALL *PFNINTERNETWRITEFILEPROC)(HINTERNET hFile, LPCVOID lpBuffer,
                                                DWORD dwNumberOfBytesToWrite,
                                                LPDWORD lpdwNumberOfBytesWritten);
typedef BOOL(STDCALL *PFNINTERNETQUERYOPTIONAPROC)(HINTERNET hInternet, DWORD dwOption,
                                                   LPVOID lpBuffer, LPDWORD lpdwBufferLength);
typedef BOOL(STDCALL *PFNINTERNETSETOPTIONAPROC)(HINTERNET hInternet, DWORD dwOption,
                                                 LPVOID lpBuffer, DWORD dwBufferLength);
typedef BOOL(STDCALL *PFNINTERNETCRACKURLAPROC)(LPCSTR lpszUrl, DWORD dwUrlLength, DWORD dwFlags,
                                                LPURL_COMPONENTSA lpUrlComponents);

// winmm.dll
typedef DWORD(STDCALL *PFNTIMEGETTIMEPROC)();

// ijl15.dll
typedef int(STDCALL *PFNIJLGETLIBVERSIONPROC)();
typedef int(STDCALL *PFNIJLINITPROC)(int);
typedef int(STDCALL *PFNIJLFREEPROC)(int);
typedef int(STDCALL *PFNIJLREADPROC)(int, int);
typedef int(STDCALL *PFNIJLWRITEPROC)(int, int);
typedef int(STDCALL *PFNIJLERRORSTRPROC)(int);
