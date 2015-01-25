/*---------------------------------------------------------------------------------  
 |        Copyright (C) 1999  Emergent IT Inc.  and Raytheon Systems Company      |    
 |  Permission to use, modify, and distribute this software and its documentation |
 |  for any purpose without fee is hereby granted, provided that the above        | 
 |  copyright notice appear in all copies and that both that copyright notice and |
 |  this permission notice appear in supporting documentation.                    |
 |                                                                                |
 |  BEGIN_FILE_PROLOG:                                                            |
 |  FILENAME:                                                                     |
 |  HE5_GCTPFUNC.H                                                                |
 |  DESCRIPTION:                                                                  |
 |	This file contains function prototypes that are specific to the GCTP          |
 |  AUTHOR:                                                                       |
 |	Alex Muslimov / Emergent Information Tecnologies, Inc.                        |
 |  HISTORY:                                                                      |
 |	14-May-01 AM Initial version                                                  |
 | END_FILE_PROLOG:                                                               |
 ----------------------------------------------------------------------------------
 */

#ifndef HE5_GctpFunc_h
#define HE5_GctpFunc_h

/*****************************************************************
    Function prototypes.
*****************************************************************/

int somfor(double lon, double lat, double *x, double *y);
int sominv(double x, double y, double *lon, double *lat);
int somforint(double r_major, double r_minor, long satnum, long path, 
                double alf_in, double lon, double false_east, 
                double false_north, double time, long start1, long flag, 
                double sat_ratio);
int sominvint(double r_major, double r_minor, long satnum, long path, 
                double alf_in, double lon, double false_east, 
                double false_north, double time, long start1, long flag, 
                double sat_ratio);

#if defined(SGI64) || defined(UNICOS) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(IBM6000) || defined(CYGWIN) || defined(DEC_ALPHA)
void for_init(int a, int b, double c[], int d, char *str1, char *str2, int *e, int (**func)(double, double, double *, double *)); 

void inv_init(int a,int b,double *c,int d,char *str1,char *str2,int *e,int (**func)(double, double, double*, double*));

#else
void for_init(long a, long b, double c[], long d, char *str1, char *str2, long *e, long (**func)(double, double, double *, double *)); 

void inv_init(long a,long b,double *c,long d,char *str1,char *str2,long *e,long (**func)(double, double, double*, double*));
#endif


#endif


