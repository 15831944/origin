// Convert.h: interface for the CConvert class.
//
//////////////////////////////////////////////////////////////////////
#if !defined(AFX_CONVERT_H__EC38F865_4607_4659_BAC8_AA6096C50EC7__INCLUDED_)
#define AFX_CONVERT_H__EC38F865_4607_4659_BAC8_AA6096C50EC7__INCLUDED_
#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
/* 
* Copyright (c) 2010,�����񣵣�����@126.com
* All rights reserved. 
*  
* �ļ����ƣ�Convert.h 
* �ļ���ʶ����������ת��
* ժ    Ҫ����Ҫ��ADO���������͵�ת�� 
*  
* ��ǰ�汾��1.0 
* ��    �ߣ������񣵣�����@126.com��ע��������ַ�����Ӣ���ַ���
* ������ڣ�2010��5��11�� 
* ����Blog��http://blog.csdn.net/zyq5945/ 
* 
*/ 
//////////////////////////////////////////////////////////////////////
#define TS(value) CConvert::ToString(value)
class CConvert  
{

public:
    static CString ToString(BYTE btValue);
    static CString ToString(int iValue);
    static CString ToString(unsigned int iValue);
    static CString ToString(long lValue);
    static CString ToString(unsigned long lValue);
    static CString ToString(__int64 i64Value);
    static CString ToString(unsigned __int64 i64Value);
    static CString ToString(float fltValue);
    static CString ToString(double dblValue);
    static CString ToString(const COleDateTime& time);
    static CString ToString(const _variant_t& var);
public:
    static BYTE ToByte(LPCTSTR lpszValue);
    static int ToInt(LPCTSTR lpszValue);
    static unsigned int ToUInt(LPCTSTR lpszValue);
    static long ToLong(LPCTSTR lpszValue);
    static unsigned long ToULong(LPCTSTR lpszValue);
    static __int64 ToInt64(LPCTSTR lpszValue);
    static unsigned __int64 ToUInt64(LPCTSTR lpszValue);
    static float ToFloat(LPCTSTR lpszValue);
    static double ToDouble(LPCTSTR lpszValue);
    static COleDateTime ToTime(LPCTSTR lpszValue);
    static _variant_t ToVariant(const COleDateTime& time);
};
extern CConvert Convert;
#endif // !defined(AFX_CONVERT_H__EC38F865_4607_4659_BAC8_AA6096C50EC7__INCLUDED_)
