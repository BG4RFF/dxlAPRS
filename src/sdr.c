/*
 * dxlAPRS toolchain
 *
 * Copyright (C) Christian Rabler <oe5dxl@oevsv.at>
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */


#define X2C_int32
#define X2C_index32
#ifndef sdr_H_
#include "sdr.h"
#endif
#define sdr_C_
#ifndef osi_H_
#include "osi.h"
#endif
#include <osic.h>
#ifndef tcp_H_
#include "tcp.h"
#endif
#ifndef tcpb_H_
#include "tcpb.h"
#endif
#ifndef aprsstr_H_
#include "aprsstr.h"
#endif
#ifndef Select_H_
#include "Select.h"
#endif





/* rtl_tcp iq fm demodulator by OE5DXL */
#define sdr_IQBUF 65536

#define sdr_DDSMAXLEN 2048

#define sdr_AFCSPEED 400000000
/* slower afc reaction */

#define sdr_AFCRECENTER 1024
/* pull afc to middle */

struct Complex;


struct Complex {
   float Re;
   float Im;
};

static long fd;

static unsigned long audiohz;

static unsigned long rtlhz;

static char reconnect;

static unsigned char iqbuf[65536];

static short DDS[2048];

static float DDSR[2048];

static char url[1001];

static char port[11];

static unsigned long sampsum;

static unsigned long sampsize;

static unsigned long reduce;

static unsigned long ddslen;

static unsigned long ddslen4;


static void initdds(unsigned long size)
{
   unsigned long i;
   float d;
   unsigned long tmp;
   if (size>2047UL) size = 2048UL;
   d = X2C_DIVR(6.2831853071796f,(float)size);
   tmp = size-1UL;
   i = 0UL;
   if (i<=tmp) for (;; i++) {
      DDSR[i] = 32767.5f*osic_sin((float)i*d);
      DDS[i] = (short)(long)X2C_TRUNCI(DDSR[i],X2C_min_longint,
                X2C_max_longint);
      if (i==tmp) break;
   } /* end for */
   ddslen = (unsigned long)(size-1UL);
   ddslen4 = size/4UL;
} /* end initdds() */

#define sdr_FG (-8)


static void iir256(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>8; /* mixer + lowpass i */
      r2 += r3-r2>>8;
      r3 += r1-r2>>7;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>8; /* mixer + lowpass q */
      i2 += i3-i2>>8;
      i3 += i1-i2>>7;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir256() */

#define sdr_FG0 (-7)


static void iir128(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>7; /* mixer + lowpass i */
      r2 += r3-r2>>7;
      r3 += r1-r2>>6;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>7; /* mixer + lowpass q */
      i2 += i3-i2>>7;
      i3 += i1-i2>>6;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir128() */

#define sdr_FG1 (-6)


static void iir64(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>6; /* mixer + lowpass i */
      r2 += r3-r2>>6;
      r3 += r1-r2>>5;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>6; /* mixer + lowpass q */
      i2 += i3-i2>>6;
      i3 += i1-i2>>5;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir64() */

#define sdr_FG2 (-5)


static void iir32(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>5; /* mixer + lowpass i */
      r2 += r3-r2>>5;
      r3 += r1-r2>>4;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>5; /* mixer + lowpass q */
      i2 += i3-i2>>5;
      i3 += i1-i2>>4;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir32() */

#define sdr_FG3 (-4)


static void iir16(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>4; /* mixer + lowpass i */
      r2 += r3-r2>>4;
      r3 += r1-r2>>3;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>4; /* mixer + lowpass q */
      i2 += i3-i2>>4;
      i3 += i1-i2>>3;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir16() */

#define sdr_FG4 (-3)


static void iir8(sdr_pRX rx, unsigned long a, unsigned long b)
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   long i3;
   long i2;
   long i1;
   long r3;
   long r2;
   long r1;
   long ddsi;
   long ddsr;
   long xi;
   long xr;
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->uc1;
      r2 = anonym->uc2;
      r3 = anonym->il;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->uc1;
      i2 = anonym0->uc2;
      i3 = anonym0->il;
   }
   ph = rx->phase;
   dfc = (unsigned long)((unsigned long)(rx->df+(unsigned long)rx->afckhz)
                &ddslen);
   i = a;
   do {
      xr = (long)iqbuf[i]-127L;
      ++i;
      xi = (long)iqbuf[i]-127L;
      ++i;
      ddsr = (long)DDS[ph]; /* mix osz sin */
      ddsi = (long)DDS[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 += ((xr*ddsr-xi*ddsi)-r1)-r3>>3; /* mixer + lowpass i */
      r2 += r3-r2>>3;
      r3 += r1-r2>>2;
      i1 += ((xr*ddsi+xi*ddsr)-i1)-i3>>3; /* mixer + lowpass q */
      i2 += i3-i2>>3;
      i3 += i1-i2>>2;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->uc1 = r1;
      anonym1->uc2 = r2;
      anonym1->il = r3;
      anonym1->ucr2 = (float)r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->uc1 = i1;
      anonym2->uc2 = i2;
      anonym2->il = i3;
      anonym2->ucr2 = (float)i2;
   }
   rx->phase = ph;
} /* end iir8() */

#define sdr_FG5 128


static void iirvar(sdr_pRX rx, unsigned long a, unsigned long b, float bw)
/* variable bandwidth */
{
   unsigned long dfc;
   unsigned long ph;
   unsigned long i;
   float ddsi;
   float ddsr;
   float bw2;
   float i3;
   float i2;
   float i1;
   float r3;
   float r2;
   float r1;
   float xi;
   float xr;
   /*WrFixed(bw, 5,1);WrStrLn(""); */
   struct sdr_TAP * anonym;
   struct sdr_TAP * anonym0;
   struct sdr_TAP * anonym1;
   struct sdr_TAP * anonym2;
   { /* with */
      struct sdr_TAP * anonym = &rx->tapre;
      r1 = anonym->ucr1;
      r2 = anonym->ucr2;
      r3 = anonym->ilr;
   }
   { /* with */
      struct sdr_TAP * anonym0 = &rx->tapim;
      i1 = anonym0->ucr1;
      i2 = anonym0->ucr2;
      i3 = anonym0->ilr;
   }
   ph = rx->phase;
   dfc = rx->df+(unsigned long)rx->afckhz;
   i = a;
   bw2 = bw*2.0f;
   do {
      xr = (float)iqbuf[i]-127.5f;
      ++i;
      xi = (float)iqbuf[i]-127.5f;
      ++i;
      ddsr = DDSR[ph]; /* mix osz sin */
      ddsi = DDSR[(unsigned long)((unsigned long)(ph+ddslen4)&ddslen)];
                /* mix osz cos */
      ph = (unsigned long)((unsigned long)(ph+dfc)&ddslen);
                /* drive mix osz */
      r1 = r1+(((xr*ddsr-xi*ddsi)-r1)-r3)*bw; /* mixer + lowpass i */
      r2 = r2+(r3-r2)*bw;
      r3 = r3+(r1-r2)*bw2;
      i1 = i1+(((xr*ddsi+xi*ddsr)-i1)-i3)*bw; /* mixer + lowpass q */
      i2 = i2+(i3-i2)*bw;
      i3 = i3+(i1-i2)*bw2;
   } while (i<=b);
   { /* with */
      struct sdr_TAP * anonym1 = &rx->tapre;
      anonym1->ucr1 = r1;
      anonym1->ucr2 = r2;
      anonym1->ilr = r3;
      anonym1->ucr2 = r2;
   }
   { /* with */
      struct sdr_TAP * anonym2 = &rx->tapim;
      anonym2->ucr1 = i1;
      anonym2->ucr2 = i2;
      anonym2->ilr = i3;
      anonym2->ucr2 = i2;
   }
   rx->phase = ph;
} /* end iirvar() */


static short getsamp(sdr_pRX rx)
{
   struct Complex abs0;
   struct Complex u;
   float af;
   float w;
   float l;
   float lev;
   u.Re = rx->tapre.ucr2;
   u.Im = rx->tapim.ucr2;
   /* complex to phase */
   abs0.Re = (float)fabs(u.Re);
   abs0.Im = (float)fabs(u.Im);
   if (abs0.Im>abs0.Re) {
      if (abs0.Im>0.0f) w = X2C_DIVR(abs0.Re,abs0.Im);
      else w = 0.0f;
      w = 1.5707963267949f-(w*1.055f-w*w*0.267f); /* arctan */
   }
   else {
      if (abs0.Re>0.0f) w = X2C_DIVR(abs0.Im,abs0.Re);
      else w = 0.0f;
      w = w*1.055f-w*w*0.267f;
   }
   if (u.Re<0.0f) w = 3.1415926535898f-w;
   if (u.Im<0.0f) w = -w;
   /* complex to phase */
   /* phase highpass make FM */
   af = w-rx->w1;
   rx->w1 = w;
   if (af>3.1415926535898f) af = af-6.2831853071796f;
   if (af<(-3.1415926535898f)) af = af+6.2831853071796f;
   /* phase highpass make FM */
   /* rssi */
   lev = 1.0f+abs0.Re*abs0.Re+abs0.Im*abs0.Im;
   rx->rssi = rx->rssi+(lev-rx->rssi)*0.001f;
   /* rssi */
   if (rx->am) {
      /* am squelch */
      if (rx->squelch) {
         l = (float)fabs(rx->a1-af); /* frequency variation */
         rx->a1 = af;
         rx->sqmed = rx->sqmed+(l-rx->sqmed)*0.002f;
      }
      /* am squelch */
      /* am demod */
      lev = osic_sqrt(lev); /* amplitude */
      af = lev-rx->lastlev; /* - median aplitude */
      rx->lastlev = rx->lastlev+af*0.001f;
      af = (X2C_DIVR(af,rx->lastlev))*32000.0f; /* agc */
   }
   else {
      /* am demod */
      /* fm squelch */
      if (rx->squelch) {
         l = X2C_DIVR((float)fabs(rx->lastlev-lev),rx->lastlev+lev);
         rx->sqmed = rx->sqmed+(l-rx->sqmed)*0.002f;
         rx->lastlev = lev;
      }
      /* fm squelch */
      af = af*7.9577471545948E+3f;
   }
   if (af>32000.0f) af = 32000.0f;
   else if (af<(-3.2E+4f)) af = (-3.2E+4f);
   return (short)X2C_TRUNCI(af,-32768,32767);
} /* end getsamp() */

#define sdr_FINESTEP 1024


extern long sdr_getsdr(unsigned long samps, sdr_pRX rx[],
                unsigned long rx_len)
{
   unsigned long bs;
   unsigned long as;
   unsigned long b;
   unsigned long a;
   unsigned long r;
   unsigned long s;
   long u;
   struct sdr_RX * anonym;
   unsigned long tmp;
   if (reconnect && fd<0L) {
      Usleep(1000000UL);
      fd = connecttob(url, port);
   }
   if (fd>=0L) {
      sampsum = sampsum&1023UL; /* partial sample reminder of last block */
      if (samps*(sampsize+1UL)>32768UL) samps = 32768UL/(sampsize+1UL);
      if (readsockb(fd, (char *)iqbuf,
                (long)(((samps*reduce+sampsum)/1024UL)*2UL))<0L) {
         /* connect lost */
         osic_Close(fd);
         fd = -1L;
         return -1L;
      }
      a = sampsum; /* continue from last partial step */
      tmp = samps-1UL;
      s = 0UL;
      if (s<=tmp) for (;; s++) {
         r = 0UL;
         b = a+reduce;
         as = (a/1024UL)*2UL;
         bs = (b/1024UL)*2UL-2UL;
         while (rx[r]) {
            { /* with */
               struct sdr_RX * anonym = rx[r];
               switch (anonym->width) {
               case 256UL:
                  iir256(rx[r], as, bs);
                  break;
               case 128UL:
                  iir128(rx[r], as, bs);
                  break;
               case 64UL:
                  iir64(rx[r], as, bs);
                  break;
               case 32UL:
                  iir32(rx[r], as, bs);
                  break;
               case 16UL:
                  iir16(rx[r], as, bs);
                  break;
               case 8UL:
                  iir8(rx[r], as, bs);
                  break;
               default:;
                  iirvar(rx[r], as, bs,
                (float)anonym->width*6.7934782608696E-7f);
                  break;
               } /* end switch */
               u = (long)getsamp(rx[r]);
               anonym->samples[s] = (short)u;
               /* AFC */
               if (!anonym->am && anonym->maxafc>0UL) {
                  anonym->median = (anonym->median+u)-(anonym->afckhz*1024L)
                /(long)anonym->maxafc;
                  if (anonym->median>400000000L) {
                     if (anonym->afckhz<(long)anonym->maxafc) {
                        ++anonym->afckhz;
                     }
                     anonym->median = 0L;
                  }
                  else if (anonym->median<-400000000L) {
                     if (anonym->afckhz>-(long)anonym->maxafc) {
                        --anonym->afckhz;
                     }
                     anonym->median = 0L;
                  }
               }
               else anonym->afckhz = 0L;
            }
            /* AFC */
            ++r;
         }
         a = b;
         if (s==tmp) break;
      } /* end for */
      sampsum = a;
      return (long)samps;
   }
   else return -1L;
   return 0;
} /* end getsdr() */


extern void sdr_setparm(unsigned long num, unsigned long value)
{
   long res;
   char tbuf[5];
   tbuf[0U] = (char)num;
   tbuf[1U] = (char)(value/16777216UL);
   tbuf[2U] = (char)(value/65536UL&255UL);
   tbuf[3U] = (char)(value/256UL&255UL);
   tbuf[4U] = (char)(value&255UL);
   res = sendsock(fd, tbuf, 5L);
} /* end setparm() */


extern char sdr_startsdr(char ip[], unsigned long ip_len, char tport[],
                unsigned long tport_len, unsigned long inhz,
                unsigned long outhz, char reconn)
{
   aprsstr_Assign(url, 1001ul, ip, ip_len);
   aprsstr_Assign(port, 11ul, tport, tport_len);
   reconnect = reconn;
   if (inhz>0UL) rtlhz = inhz;
   if (rtlhz!=1024000UL && rtlhz!=2048000UL) return 0;
   if (outhz>0UL) audiohz = outhz;
   reduce = (1024UL*rtlhz+audiohz/2UL)/audiohz; /* sample reduction * 1024 */
   sampsize = reduce/1024UL; /* input samples per output sample, trunc */
   if (fd<0L) fd = connecttob(url, port);
   if (fd>=0L) {
      sdr_setparm(2UL, rtlhz);
      initdds(inhz/1000UL);
   }
   return fd>=0L;
} /* end startsdr() */


extern void sdr_BEGIN(void)
{
   static int sdr_init = 0;
   if (sdr_init) return;
   sdr_init = 1;
   if (sizeof(sdr_AUDIOSAMPLE)!=131072) X2C_ASSERT(0);
   aprsstr_BEGIN();
   osi_BEGIN();
   fd = -1L;
   reconnect = 0;
   rtlhz = 2048000UL;
   audiohz = 16000UL;
}

