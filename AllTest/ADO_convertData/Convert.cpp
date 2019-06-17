// Convert.cpp: implementation of the CConvert class.
//
//////////////////////////////////////////////////////////////////////
//#include "stdafx.h"
#include <Windows.h>
#include <afx.h>
#include "Convert.h"
#include <comutil.h>
#include <comdef.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
/* 
* Copyright (c) 2010,ｚｙｑ５９４５@126.com
* All rights reserved. 
*  
* 文件名称：Convert.cpp 
* 文件标识：数据类型转换
* 摘    要：主要简化ADO中数据类型的转换 
*  
* 当前版本：1.0 
* 作    者：ｚｙｑ５９４５@126.com（注意把中文字符换成英文字符）
* 完成日期：2010年5月11日 
* 发布Blog：http://blog.csdn.net/zyq5945/
* 
*/ 
//////////////////////////////////////////////////////////////////////
CConvert Convert;
CString CConvert::ToString(BYTE btValue)
{
    CString strValue;
    strValue.Format(_T("%d"), btValue);
    return strValue;
}
CString CConvert::ToString(int iValue)
{
    CString strValue;
    strValue.Format(_T("%d"), iValue);
    return strValue;
}
CString CConvert::ToString(unsigned int iValue)
{
    CString strValue;
    strValue.Format(_T("%u"), iValue);
    return strValue;
}
CString CConvert::ToString(long lValue)
{
    CString strValue;
    strValue.Format(_T("%d"), lValue);
    return strValue;
}
CString CConvert::ToString(unsigned long lValue)
{
    CString strValue;
    strValue.Format(_T("%u"), lValue);
    return strValue;
}
CString CConvert::ToString(__int64 i64Value)
{
    CString strValue;
    strValue.Format(_T("%I64d"), i64Value);
    return strValue;
}
CString CConvert::ToString(unsigned __int64 i64Value)
{
    CString strValue;
    strValue.Format(_T("%I64u"), i64Value);
    return strValue;
}
CString CConvert::ToString(float fltValue)
{
    CString strValue;
    strValue.Format(_T("%f"), fltValue);
    return strValue;
}
CString CConvert::ToString(double dblValue)
{
    CString strValue;
    strValue.Format(_T("%f"), dblValue);
    return strValue;
}
// 时间输出格式：2010-05-06 22:07:08
CString CConvert::ToString(const COleDateTime& time)
{
    CString strValue;
    strValue = time.Format(_T("%Y-%m-%d %H:%M:%S"));
    return strValue;
}
CString CConvert::ToString(const _variant_t& var)
{
    CString strValue;
    switch (var.vt)
    {
    case VT_BSTR: //字符串
    case VT_LPSTR:
    case VT_LPWSTR:
        strValue = (LPCTSTR)(_bstr_t)var;
        break;
    case VT_I1:   //无符号字符
    case VT_UI1:
        strValue.Format(_T("%d"), var.bVal);
        break;
    case VT_I2:   //短整型
        strValue.Format(_T("%d"), var.iVal);
        break;
    case VT_UI2:   //无符号短整型
        strValue.Format(_T("%u"), var.uiVal);
        break;
    case VT_INT: //整型
        strValue.Format(_T("%d"), var.intVal);
        break;
    case VT_I4:   //整型
    case VT_I8:   //长整型
        strValue.Format(_T("%d"), var.lVal);
        break;
    case VT_UINT:   //无符号整型
        strValue.Format(_T("%d"), var.uintVal);
        break;
    case VT_UI4:    //无符号整型
    case VT_UI8:    //无符号长整型
        strValue.Format(_T("%d"), var.ulVal);
        break;
    case VT_VOID:
        strValue.Format(_T("%08x"), var.byref);
        break;
    case VT_R4:   //浮点型
        strValue.Format(_T("%f"), var.fltVal);
        break;
    case VT_R8:   //双精度型
        strValue.Format(_T("%f"), var.dblVal);
        break;
    case VT_DECIMAL: //小数
        strValue.Format(_T("%f"), (double)var);
        break;
    case VT_CY:
        {
            COleCurrency cy = var.cyVal;
            strValue = cy.Format();
        }
        break;
    case VT_BLOB:
    case VT_BLOB_OBJECT:
    case 0x2011:
        strValue = _T("[BLOB]");
        break;
    case VT_BOOL:   //布尔型  
        strValue = var.boolVal ? _T("TRUE") : _T("FALSE");
        break;
    case VT_DATE: //日期型
        {
            DATE dt = var.date;
            COleDateTime da = COleDateTime(dt);
            strValue = da.Format(_T("%Y-%m-%d %H:%M:%S"));
        }
        break;
    case VT_NULL://NULL值
    case VT_EMPTY:   //空
        strValue = _T("");
        break;
    case VT_UNKNOWN:   //未知类型
    default:
        strValue = _T("VT_UNKNOW");
        break;
    }

    return strValue;
}
BYTE CConvert::ToByte(LPCTSTR lpszValue)
{
    BYTE btValue;
    btValue = (BYTE)_ttoi(lpszValue);
    return btValue;
}
int CConvert::ToInt(LPCTSTR lpszValue)
{
    int iValue;
    iValue = _ttoi(lpszValue);
    return iValue;
}
unsigned int CConvert::ToUInt(LPCTSTR lpszValue)
{
    unsigned int iValue;
    iValue = _ttoi(lpszValue);
    return iValue;
}
long CConvert::ToLong(LPCTSTR lpszValue)
{
    long lValue;
    lValue = _ttol(lpszValue);
    return lValue;
}
unsigned long CConvert::ToULong(LPCTSTR lpszValue)
{
    unsigned long lValue;
    lValue = _ttol(lpszValue);
    return lValue;
}
__int64 CConvert::ToInt64(LPCTSTR lpszValue)
{
    __int64 i64Value;
    i64Value = _ttoi64(lpszValue);
    return i64Value;
}
unsigned __int64 CConvert::ToUInt64(LPCTSTR lpszValue)
{
    unsigned __int64 i64Value;
    i64Value = _ttoi64(lpszValue);
    return i64Value;
}
float CConvert::ToFloat(LPCTSTR lpszValue)
{
    float fltValue;
#ifdef _MBCS
    fltValue = (float)atof(lpszValue);
#else
    fltValue = (float)wtof(lpszValue);
#endif
    return fltValue;
}
double CConvert::ToDouble(LPCTSTR lpszValue)
{
    double dblValue;
#ifdef _MBCS
    dblValue = atof(lpszValue);
#else
    dblValue = wtof(lpszValue);
#endif
    return dblValue;
}
// 时间格式例子：2010-05-06 22:07:08
//				 2010 05 06 22 07 08
//				 2010:05:06 22:07:08
//				 2010-05-06-22-07-08
// 只要是“- :”分割的时间格式都符合
COleDateTime CConvert::ToTime(LPCTSTR lpszValue)
{
    unsigned int iArray[6] = {2010, 5, 11, 12, 00, 00};
    int nIndex = 0;
#if _MSC_VER >= 1310  //VC6.0不支持CStringAlt::Tokenize
    CString strTmp;
    int curPos = 0;
    CString strValue(lpszValue);

    strTmp = strValue.Tokenize(_T("- :"),curPos);
    while (strTmp != _T("") && nIndex <6)
    {
        iArray[nIndex++] = ToUInt(strTmp);
        strTmp = strValue.Tokenize(_T("- :"), curPos);
    };
#else
    TCHAR tChar[MAX_PATH] = {0};
    TCHAR tCharTmp[MAX_PATH] = {0};
    TCHAR seps[]   = "- :";
    TCHAR *next_token = NULL;
    ASSERT(_tcslen(lpszValue) < MAX_PATH);
    RtlCopyMemory(tChar, lpszValue, _tcslen(lpszValue) * sizeof(TCHAR));
    next_token = _tcstok(tChar, seps);
    while ( next_token != NULL && nIndex <6)
    {
        iArray[nIndex++] = _ttoi(next_token);
        next_token = _tcstok( NULL, seps);
    }

#endif
    COleDateTime time(iArray[0], iArray[1], iArray[2],
        iArray[3], iArray[4], iArray[5]);
    return time;
}
_variant_t CConvert::ToVariant(const COleDateTime& time)
{
    _variant_t vtVal;

    vtVal.vt = VT_DATE;
    vtVal.date = time;
    return vtVal;
}
