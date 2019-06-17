/** =============================================================================
 ** Copyright (c) 2003 WaveTop Information Corp. All rights reserved.
 **
 ** The backup system
 **
 ** =============================================================================
 */

#ifndef __WAVETOP_BYTE_ORDER_H_
#define __WAVETOP_BYTE_ORDER_H_

/* the integer byte-order */
#define XINT32(n,p) {\
    (p)[0]=(unsigned char)(((n)>>24)&0xff);\
    (p)[1]=(unsigned char)(((n)>>16)&0xff);\
    (p)[2]=(unsigned char)(((n)>>8)&0xff);\
    (p)[3]=(unsigned char)( (n)&0xff);\
}
#define VINT32(n,p) {\
    n= (((unsigned char)((p)[0])) << 24) + \
       (((unsigned char)((p)[1])) << 16) + \
       (((unsigned char)((p)[2])) << 8 ) + \
       (((unsigned char)((p)[3]))      ) ; \
}

#define XINT16(n,p) {\
    (p)[0]=(unsigned char)(((n)>>8)&0xff);\
    (p)[1]=(unsigned char)( (n)&0xff);\
}
#define VINT16(n,p) {\
    n= (((unsigned char)((p)[0])) << 8) + \
       (((unsigned char)((p)[1]))     ) ; \
}

#endif /* __WAVETOP_BYTE_ORDER_H_ 1 */
