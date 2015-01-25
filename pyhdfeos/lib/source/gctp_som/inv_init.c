/*******************************************************************************
NAME                           INV_INIT 

PURPOSE:	Initializes inverse projection transformation parameters

PROGRAMMER              DATE		REASON
----------              ----		------
T. Mittan		3-09-93		Initial Development
S. Nelson		11-94		Added Clarke spheroid default to UTM
Raj Gejjagaraguppe(ARC)   08-30-96      Landsat Ratio is removed as hard
                                        coded value.  Now this ratio can be
                                        an input from the user through the
                                        projection parameter array element
                                        number 9.
Raj Gejjagaraguppe(ARC)   01-07-97      Added a new projection type called
                                        Integerized Sinusoidal Grid to 
                                        support MODIS level 3 datasets.
D. Wynne(ARC)             3-24-97       Added Support for Power Challenge
                                        (R10000 Processor Chip Revision: 2.5)
                                        Long is 8 bytes,on all other currently 
					supported platforms Long is 4 bytes.
Abe Taaheri               06-27-00      Added a new projection type called
                                        Behrmann Cylinderical Equal Area to
                                        support EASE grid.
Abe Taaheri               10-23-00      Updated for ISINUS projection, so 
                                        that both codes 31 and 99 (i.e.
                                        ISINUS and ISINUS1) can be used 
                                        for this projection.
Abe Taaheri                1-15-03      Modified to support both
                                        spherical and ellipsoid
                                        models of earth for 
                                        Normal Cylinderical Equal Area
                                        projection.
Abe Taaheri                8-15-03      Added CEA projection. This is used
                                        by generalized EASE grid, when the
                                        EASE grid corners are specified in
                                        meters (Note that BCEA is similar
                                        projection and used when EASE grid
                                        corners are set in packed DMS degrees.
Abe Taaheri                11-15-03     Modified the call for LA projection 
                                        to support ellipsoid earth for this
                                        projection
ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include <stdio.h>
#include "cproj.h"
#include "proj.h"

int inv_init(
int insys,		/* input system code				*/
int inzone,		/* input zone number				*/
double *inparm,	/* input array of projection parameters	*/
int indatum,	/* input datum code				    */
char *fn27,		/* NAD 1927 parameter file			*/
char *fn83,		/* NAD 1983 parameter file			*/
int *iflg,		/* status flag					*/
int (*inv_trans[])(double, double, double*, double*))	
                /* inverse function pointer			*/
{
long zone;		/* zone number					*/
double azimuth;		/* azimuth					*/
double angle;		/* rotation anlge				*/
double alf;		/* SOM angle					*/
double lon1;		/* longitude point in utm scene			*/
double lon2;		/* 2nd longitude point 				*/
double lat1;		/* 1st standard parallel			*/
double lat2;		/* 2nd standard parallel			*/
double center_long;	/* center longitude				*/
double center_lat;	/* center latitude				*/
double h;		/* height above sphere				*/
double lat_origin;	/* latitude at origin				*/
double lon_origin;	/* longitude at origin				*/
double r_major;		/* major axis in meters				*/
double r_minor;		/* minor axis in meters				*/
double scale_factor;	/* scale factor					*/
double false_easting;	/* false easting in meters			*/
double false_northing;	/* false northing in meters			*/
double radius;		/* radius of sphere				*/
double shape_m;		/* constant used for Oblated Equal Area		*/
double shape_n;		/* constant used for Oblated Equal Area		*/
/*long   start;	*/	/* start of SOM Beginning or end		*/
double time;		/* SOM time					*/
long path;		/* SOM path number				*/
long satnum;		/* SOM satellite number				*/
long mode;		/* which format is used	A or B			*/
long tmpdatum;		/* temporary datum for UTM			*/
double sat_ratio;       /* satellite ratio which specify the start point*/
double dzone;           /* number of longitudinal zones in ISG          */
double djustify;        /* justify flag in ISG projection               */

long thing;             /* used to initialize 8 byte pointer, added     */
                        /* for Power Challenge                          */
long *iflg64;		/* 8 byte status flag for Power Challenge	*/

thing = 0;                      /* These lines are to initialize the    */
iflg64 = &thing;                /* the 8-byte pointer address           */

/* Initialize inverse transformations
-----------------------------------*/
  /* find the correct major and minor axis
  --------------------------------------*/
 if(insys == CEA)
   {
   }
 else if(insys == BCEA)
   {
   }
 else
   {
     sphdz(indatum,inparm,&r_major,&r_minor,&radius);
   }
  false_easting  = inparm[6];
  false_northing = inparm[7];

  if (insys == CEA)/* Cylindrical Equal-Area, used for EASE grid wghen
                      grid corners are specified in meters */
    {
    }
  else 
  if (insys == BCEA)/* Cylindrical Equal-Area, used for EASE grid wghen
                      grid corners are specified in DMS degrees */
    {
    }
  else
  if (insys == UTM)
     {
     }
  else
  if (insys == SPCS)
     {
     }
  else
  if (insys == ALBERS)
     {
     }
  else
  if (insys == LAMCC)
     {
     }
  else
  if (insys == MERCAT)
     {
     }
  else
  if (insys == PS)
     {
     }
  else
  if (insys == POLYC)
     {
     }
  else
    if (insys == EQUIDC)
    {
    }
  else
  if (insys == TM)
     {
     }
  else
  if (insys == STEREO)
     {
     }
  else
  if (insys == LAMAZ)
     {
     }
  else
  if (insys == GNOMON)
     {
     }
  else
  if (insys == ORTHO)
     {
     }
  else
  if (insys == GVNSP)
     {
     }
  else
  if (insys == SNSOID)
     {
     }
  else
  if (insys == EQRECT)
     {
     }
  else
  if (insys == MILLER)
    {
    }
  else
  if (insys == VGRINT)
    {
    }
  else
  if (insys == HOM)
     {
     }
  else
  if (insys == SOM)
     {

     /* this is the call to initialize SOM 
     -----------------------------------*/
     path = (long) inparm[3];
     satnum = (long) inparm[2];
    if (inparm[12] == 0)
       {
       mode = 1;

       alf = paksz(inparm[3],iflg64) * 3600.0 * S2R;
       *iflg = (int)*iflg64;

       if ((int)*iflg64 != 0)
          return ERROR;

       lon1 = paksz(inparm[4],iflg64) * 3600 * S2R;
       *iflg = (int)*iflg64;

       if ((int)*iflg64 != 0)
          return ERROR;

       time = inparm[8];
       sat_ratio = inparm[9];

       /* start = (long) inparm[10];*/
       }
    else
       mode = 0;
/*
     *iflg64 = sominvint(r_major,r_minor,satnum,path,false_easting,
			false_northing);
        *iflg = (int)*iflg64;
*/

     *iflg64 = sominvint(r_major,r_minor,satnum,path,alf,lon1,false_easting,
                      false_northing,time,mode,sat_ratio);
        *iflg = (int)*iflg64;
     inv_trans[insys] = sominv;
     }
  else
  if (insys == HAMMER)
    {
    }
  else
  if (insys == ROBIN)
    {
    }
  else
  if (insys == GOODE)
     {
     }
  else
  if (insys == MOLL)
     {
     }
  else
  if (insys == IMOLL)
     {
     }
  else
  if (insys == ALASKA)
     {
     }
  else
  if (insys == WAGIV)
     {
     }
  else
  if (insys == WAGVII)
     {
     }
  else
  if (insys == OBEQA)
    {
    }
  else
    if ((insys == ISINUS) || (insys == ISINUS1))
    {
    }

return OK;
}
