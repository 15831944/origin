/** =========================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The Mirror system
 **
 ** =========================================================================
 */

#ifndef _LIBMIRROR_TYPES_H_
#define _LIBMIRROR_TYPES_H_ 1

#if defined(__cplusplus)
#define	WTMI_BEGIN_DECLS	extern "C" {
#define	WTMI_END_DECLS	    }
#else
#define	WTMI_BEGIN_DECLS
#define	WTMI_END_DECLS
#endif /* defined(__cplusplus) */

#if defined(_LIB)
#define WTMI_LIBMI_EXPORT
#define WTMI_LIBMI_EXPORT_(_type)       _type
#else
#if defined(WIN32)
#define WTMI_LIBMI_EXPORT                __declspec(dllexport)
#define WTMI_LIBMI_EXPORT_(_type)        __declspec(dllexport) _type
#else
#define WTMI_LIBMI_EXPORT                extern 
#define WTMI_LIBMI_EXPORT_(_type)        extern _type
#endif /* defined(WIN32) */
#endif /* defined(_LIB) */

#endif /* !defined(_LIBMIRROR_TYPES_H_) */

