/*
 ----------------------------------------------------------------------------
 |    Copyright (C) 1999   Emergent IT Inc. and Raytheon Systems Company    |
 |                                                                          |
 |  Permission to use, modify, and distribute this software and its         |
 |  documentation for any purpose without fee is hereby granted, provided   |
 |  that the above copyright notice appear in all copies and that both that |
 |  copyright notice and this permission notice appear in supporting        |
 |                          documentation.                                  |
 ----------------------------------------------------------------------------
 */

#include <HE5_HdfEosDef.h>

/*#define	HE5_PTIDOFFSET 1342177282*/   /*  (2**27)*10+2  */
/*modified HE5_PTIDOFFSET to the same value as HE5_GRIDOFFSET,
  HE5_SWIDOFFSET,  HE5_ZAIDOFFSET, i.e. 671088642, for HDF-EOS5 version 2.11 */

#define	HE5_PTIDOFFSET 671088642    /* (2**26)*10 + 2  */
#define HE5_NPOINT             64
#define HE5_NPOINTREGN        256
#define HE5_NRECS               1

struct HE5_pointStructure
{
  int          active;		  	  /* Flag: if file is active or not */

  hid_t        fid;     	    	  /* HDF-EOS file ID                */
  hid_t        pt_id;			  /* Specified Point group ID       */
  hid_t        data_id;			  /* "Data" group ID                */
  hid_t        link_id;			  /* "Linkage" group ID             */

  HE5_DTSinfo  level[HE5_DTSETRANKMAX];   /* Level ID and name              */

  char         ptname[HE5_OBJNAMELENMAX]; /* Point name                     */
};

struct HE5_pointStructure HE5_PTXPoint[HE5_NPOINT];

typedef struct 
{
  HE5_DTSinfo  level[HE5_DTSETRANKMAX];
  int      nlevels;
} pack;


typedef struct
{
  LONGLONG    back;
} bckptr;


typedef struct
{
  LONGLONG    begin;
  LONGLONG    extent;
} fwdptr;


struct HE5_pointRegion 
{
  hid_t     fid;
  hid_t     pointID;

  hsize_t   nrec[HE5_DTSETRANKMAX];
  hssize_t  *recPtr[HE5_DTSETRANKMAX];
};

struct HE5_pointRegion *HE5_PTXRegion[HE5_NPOINTREGN];


/* Point Prototypes (internal routines) */

static herr_t       
HE5_PTchkptid(hid_t pointID, const char *routname, hid_t *fid, hid_t *gid,  long *idx);
static herr_t      
HE5_PTfexist(hid_t  dataID, const char *fldname, size_t *size,  int *rank, size_t dims[], hid_t *dtype );
herr_t       
HE5_PTlinkinfo(hid_t, hid_t, int level, char *mode, char *linkfield);
static herr_t       
HE5_PTwrbckptr(hid_t pointID, int level);
static herr_t      
HE5_PTrdbckptr(hid_t pointID, int level, void *buffer) ;
static herr_t       
HE5_PTwrfwdptr(hid_t pointID, int level);
static herr_t       
HE5_PTrdfwdptr(hid_t pointID, int level, void *buffer);
static herr_t       
HE5_PTwritesetup(hid_t pointID,  int level, hssize_t *nrec);
static hid_t     
HE5_PTrecnum(hid_t pointID, int level, int minlevel, int maxlevel, hsize_t nrec, hssize_t recs[]);
hid_t        
HE5_PTopenlevel(hid_t loc_id, const char *name, void *opdata);
int 
HE5_PTgetdtypesize(hid_t levelID, int i);
herr_t  
HE5_PTgetlevelname_fromSM(hid_t pointID, int level, char *LevelName); 

/* FORTRAN wrappers */
/* ================ */

/* File/Point access routine */

int
HE5_PTopenF(char *filename, uintn Flags);
int 
HE5_PTcreateF(int FileID, char *pointname);
int
HE5_PTattachF(int FileID, char *pointname);
int
HE5_PTdetachF(int pointID);
int 
HE5_PTcloseF(int FileID);

/* Definition routines */

int
HE5_PTdeflevelF(int  pointID, const char *levelname, int rank[], char *fieldlist, long *dim_sizes, int dtype[], int array[]);
int  
HE5_PTdeflinkageF(int pointID, char *parent, char *child, char *linkfield);

/* I/O routines */

int 
HE5_PTreadlevelF(int pointID, int level, char *fieldname, int ntype, void *datbuf);
int
HE5_PTupdatelevelF(int pointID, int level, char *fieldlist, long nrec,  long recs[], int ntype, void *data);
int
HE5_PTwritelevelF(int pointID, int level, long count[], char *fieldname, int ntype, void *data);
int
HE5_PTwriteattrF(int pointID, const char *attrname, int ntype, long fortcount[], void *datbuf);
int
HE5_PTwritegrpattrF(int pointID, const char *attrname, int ntype, long fortcount[], void * datbuf);
int 
HE5_PTwritelocattrF(int pointID, const char *levelname, const char *attrname, int ntype, long count[],void * datbuf);
int 
HE5_PTreadattrF(int pointID, const char *attrname, void *datbuf);
int  
HE5_PTreadgrpattrF(int pointID, const char *attrname, void *datbuf);
int  
HE5_PTreadlocattrF(int pointID, const char *levelname, const char *attrname, void *datbuf);
int       
HE5_PTwrbckptrF(int pointID, int level);
int       
HE5_PTwrfwdptrF(int pointID, int level);

/* Inquiry routines */

int 
HE5_PTnrecsF(int pointID, int level);
int
HE5_PTnlevelsF(hid_t pointID);
int 
HE5_PTnfieldsF(int pointID, int level, char *fieldlist, long *strbufsize);
int  
HE5_PTlevelindxF(int pointID, const char *levelname);
int 
HE5_PTgetlevelnameF(int pointID, int  level, char *levelname, long *strbufsize);
int 
HE5_PTbcklinkinfoF(int pointID, int level, char *linkfield);
int   
HE5_PTfwdlinkinfoF(int pointID, int level, char *linkfield);
int  
HE5_PTlevelinfoF(int pointID, int level, char *levelname, int rank[], char *fieldlist, long *dim_sizes, long *datasize, long offset[], int dtype[]);
int   
HE5_PTinqdatatypeF(int pointID, char *fieldname, char *attrname, int fieldgroup, int *Type, int *Class, int *Order, long *size);
int  
HE5_PTinqpointF(const char *filename, char *pointlist, long *strbufsize);
int 
HE5_PTattrinfoF(int pointID, const char *attrname, int *numbertype, long *fortcount);
int
HE5_PTgrpattrinfoF(int pointID, const char *attrname, int *numbertype, long *fortcount);
int   
HE5_PTlocattrinfoF(int pointID, const char *levelname, const char *attrname, int *numbertype, long *fortcount);
long 
HE5_PTinqattrsF(int pointID, char *attrnames, long *strbufsize);
long 
HE5_PTinqgrpattrsF(int pointID, char *attrnames, long *strbufsize);
long 
HE5_PTinqlocattrsF(int pointID, const char *levelname, char *attrnames, long *strbufsize);
int
HE5_PTnumtypeinfoF(int pointID, int level, int numtype[]);


/* Fortran Wrapper Utility Routines  */

static int
HE5_PTfort2c(long dims[], int rank, int datatype, void *inputfortbuf, void *outputfortbuf);
static int
HE5_PTc2fort(long dims[], int rank, int datatype, void *inputfortbuf, void *outputfortbuf);

/* Fortran Wrappers (internal routines) */

static int 
HE5_PTgetfort_order(long *dims, int dim_index, long *c_index, int rank, long *P, long *coeffs, long *n);
static herr_t   
HE5_PTreadlevel_f(hid_t pointID, int level, char *fieldname, hid_t dtype, void *datbuf);  
static herr_t 
HE5_PTwritelevel_f(hid_t pointID, int level, hsize_t count[], char *fieldname, int ntype, hid_t dtype, void *data);
static herr_t  
HE5_PTupdatelevel_f(hid_t pointID, int level, char *fieldlist, hsize_t nrec,  hssize_t recs[], int ntype, hid_t dtype, void *data);

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTopen                                                       |
|                                                                             |
|  DESCRIPTION: Opens an HDF file and returns file ID.                        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  fid            hid_t               HDF-EOS file ID                         |
|                                                                             |
|  INPUTS:                                                                    |
|  filename       char                Point Filename                          |
|  flags          uintn               Access Code                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.20.99  A. Muslimov                                                       |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t 
HE5_PTopen(const char *filename, uintn flags)
{
  hid_t       fid    = FAIL;/* HDF-EOS file ID */

  /* Call HE5_EHopen to open file */
  /* ---------------------------- */
  fid = HE5_EHopen(filename,flags, H5P_DEFAULT );

  return (fid);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTcreate                                                     |
|                                                                             |
|  DESCRIPTION: Creates a new point structure and returns point ID.           |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  pointID        hid_t               Point structure ID                      |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS File ID                         |
|  pointname      char                Point structure name                    |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date         Programmer     Description                                   |
|  ===========   ============  ============================================   |
|  7.18.99       A.Muslimov    Unmasked hdf5 data types.                      |
|  12/13/99      A.Muslimov    Added proper error handlings after the function|
|                              calls. Replace a call EHattrcat() by           |
|                              EHdatasetcat().                                |
|  09/13/01      A.Muslimov    Added mutex "lock"/"unlock" calls.             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t 
HE5_PTcreate(hid_t fid, const char *pointname)
{
  hid_t           pointID    = FAIL;/* HDF-EOS point ID (return value) */

  herr_t          status     = FAIL;/* routine return status variable  */

  int             i, j;			    /* Loop indices                    */
  int             npointopen = 0;   /* # of point structures open      */
  int             rank       = 1;   /* rank of a data space            */

  uintn           access     = 99;  /* Read/Write file access code     */

  unsigned int    zerobuf[HE5_DTSETRANKMAX];/* "Level Written" buffer  */

  hid_t           HDFfid    = FAIL;/* HDF file ID                      */
  hid_t           gid       = FAIL;/* HDFEOS group ID                  */
  hid_t           pt_id     = FAIL;/* HDF5 object point ID             */      
  hid_t           data_id   = FAIL;/* Data group ID                    */
  hid_t           link_id   = FAIL;/* Linkage group ID                 */
  hid_t           lvwr_id   = FAIL;/* "LevelWritten" ID                */
  hid_t           sid       = FAIL;/* Data space ID                    */
  hid_t           POINTS_ID = FAIL;/* "POINTS" group ID                */
 
  long            nPoint    =  0;  /* Point counter                    */

  hsize_t         count     = HE5_DTSETRANKMAX;
    
  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* error message buffer */
  char            utlbuf[HE5_HDFE_UTLBUFSIZE]; /* Utility buffer       */
  char            utlbuf2[HE5_HDFE_UTLBUFSIZE];/* Utility buffer 2     */


  HE5_LOCK;
  CHECKNAME(pointname);

  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    zerobuf[ i ] = 0;
    
  /* Check HDF-EOS file ID  */
  /* ---------------------  */
  status = HE5_EHchkfid(fid, pointname, &HDFfid, &gid, &access);
  if (status == FAIL)
    {
      sprintf(errbuf, "Checking file ID failed.\n");
      H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
    
  /* Check pointname for length */
  /* -------------------------- */
  if ( strlen(pointname) > HE5_OBJNAMELENMAX)
    {
      sprintf(errbuf, "Pointname \"%s\" must be less than %d characters.\n",  pointname, HE5_OBJNAMELENMAX);
      H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
   
  /* Determine number of points currently opened */
  /* ------------------------------------------- */
  for (i = 0; i < HE5_NPOINT; i++)
    npointopen += HE5_PTXPoint[i].active;

  if(npointopen <  HE5_NPOINT)
    {
      /* Probe , check if "POINTS" group exist*/
      H5E_BEGIN_TRY {
	POINTS_ID = H5Gopen(gid, "POINTS");
      }
      H5E_END_TRY;

      /* Group "POINTS" not exist*/
      if(POINTS_ID == FAIL)
	{
	  nPoint = 0;

	  /* create the POINTS group*/
	  POINTS_ID = H5Gcreate(gid,"POINTS",0);
	  if (POINTS_ID == FAIL)
	    {
              sprintf(errbuf, "Cannot create \"POINTS\" group.\n");
              H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
              HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	     
	}
      /* Group "POINTS" exist */
      else
	{
	  /* Probe , check if pointname group exist*/
	  H5E_BEGIN_TRY{
	    pt_id = H5Gopen(POINTS_ID, pointname);
	  }
	  H5E_END_TRY;

	  if( pt_id == FAIL)
	    {
	      /* Get the number of points in "POINTS" group*/
	      nPoint= HE5_EHdatasetcat(fid, "/HDFEOS/POINTS", NULL,NULL);
	      if (nPoint == FAIL)
		{
		  sprintf(errbuf,"Cannot retrieve the number of points.");
		  H5Epush(__FILE__, "HE5_PTcreate",__LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  status = H5Gclose(POINTS_ID);
		  return(FAIL);
		}
	    }
	  else
	    {
	      sprintf(errbuf, "Point \"%s\" already exists.", pointname);
	      H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_BTREE, H5E_EXISTS, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      status = H5Gclose(POINTS_ID);
	      return(FAIL);
	    }
	}

      /* pointname not exist, will create it! */
      /* Create pointname group under "POINTS"*/
      pt_id = H5Gcreate( POINTS_ID, pointname , 0);
      if (pt_id == FAIL)
	{
	  sprintf(errbuf, "Cannot create \"%s\" group.\n", pointname);
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Create LevelWritten attribute */
      sid = H5Screate_simple(rank, &count, 0);
      if (sid == FAIL)
	{
	  sprintf(errbuf, "Cannot create data space ID.\n");
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      lvwr_id = H5Acreate(pt_id, "_LevelWritten", H5T_NATIVE_UINT, sid, H5P_DEFAULT);
      if (lvwr_id == FAIL)
	{
	  sprintf(errbuf, "Cannot create \"_LevelWritten\" attribute.\n");
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Write "LevelWriten" attribute*/
      status = H5Awrite(lvwr_id, H5T_NATIVE_UINT, zerobuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot wtrite data to the  \"_LevelWritten\" attribute.\n");
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      status = H5Sclose(sid);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      status = H5Aclose(lvwr_id);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the \"_LevelWritten\" attribute ID.\n");
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ATTR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Create Data Records group */
      data_id = H5Gcreate( pt_id, "Data", 0);
      if (data_id == FAIL)
	{
	  sprintf(errbuf, "Cannot create \"Data\" group.\n");
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Create Linkage Records group */
      link_id = H5Gcreate( pt_id, "Linkage", 0);
      if (link_id == FAIL)
	{
	  sprintf(errbuf, "Cannot create \"Linkage\" group.\n");
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Establish Point in Structural MetaData Block */
      sprintf(utlbuf, "%s%li%s%s%s","\tGROUP=POINT_", nPoint + 1,"\n\t\tPointName=\"", pointname, "\"\n");

      strcat(utlbuf, "\t\tGROUP=Level\n");
      strcat(utlbuf, "\t\tEND_GROUP=Level\n");
      strcat(utlbuf, "\t\tGROUP=LevelLink\n");
      strcat(utlbuf, "\t\tEND_GROUP=LevelLink\n");
      sprintf(utlbuf2, "%s%li%s","\tEND_GROUP=POINT_", nPoint + 1, "\n");
      strcat(utlbuf, utlbuf2);

      status = HE5_EHinsertmeta(fid, pointname, "p", 1003L, utlbuf, NULL);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot insert metadata for the \"%s\" point.\n", pointname);
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      status = H5Gclose(POINTS_ID);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the \"POINTS\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}


      /* Assign pointID # & Load point and PTXPoint table entries */
      for (i = 0; i < HE5_NPOINT; i++)
	{
	  if (HE5_PTXPoint[ i ].active == 0)
	    {
	      pointID                      = i + HE5_PTIDOFFSET;
	      HE5_PTXPoint[i].active       = 1;
	      HE5_PTXPoint[i].fid          = fid;
	      HE5_PTXPoint[i].pt_id        = pt_id;
	      HE5_PTXPoint[i].link_id      = link_id;
	      HE5_PTXPoint[i].data_id      = data_id;
	      strcpy(HE5_PTXPoint[ i ].ptname, pointname);
	      for(j = 0; j < HE5_DTSETRANKMAX; j++)
		{
		  HE5_PTXPoint[ i ].level[j].ID   = FAIL;
		  HE5_PTXPoint[ i ].level[j].name = NULL;
		}

	      break;
	    }
	}
 
    }
  else
    {
      /* Too many files opened */
      pointID = FAIL;
      sprintf(errbuf, "\nNo more than %d points may be open simutaneously", HE5_NPOINT);
      H5Epush(__FILE__, "HE5_PTcreate", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

 COMPLETION:
  HE5_UNLOCK;
  return (pointID);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTchkptid                                                    |
|                                                                             |
|  DESCRIPTION: Checks for valid pointID and returns file ID, group ID,       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  routname       char                Name of routine calling PTchkptid       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fid            hid_t               File ID                                 |
|  gid            hid_t               group ID                                |
|  idx            long                Point index                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t  
HE5_PTchkptid(hid_t pointID, const char *routname, hid_t *fid, hid_t *gid, long *idx)
{
  herr_t          status   = SUCCEED;/* routine return status variable */

  hid_t           HDFfid   = FAIL;   /* HDF-EOS file ID                */
  hid_t           idOffset = HE5_PTIDOFFSET;/* Point ID offset         */

  uintn           access   = 77;     /* Read/Write access code         */

  char            message1[] = "Invalid point ID: %d in routine \"%s\".  ID must be >= %li and < %li.\n";
  char            message2[] = "Point ID %d in routine \"%s\" not active.\n";
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer   */


  /* Check for valid point ID */
  /* ------------------------ */
  if (pointID < idOffset || pointID >= (HE5_NPOINT + idOffset) )
    {
      status = FAIL;
      sprintf(errbuf, message1, pointID, routname, idOffset, (long)(HE5_NPOINT + idOffset) );
      H5Epush(__FILE__, "HE5_PTchkptid", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  else
    {
      /* Check for active point ID */
      /* ------------------------- */
      if (HE5_PTXPoint[pointID % idOffset].active == 0)
	{
	  status = FAIL;
	  sprintf(errbuf,  message2, pointID, routname);
	  H5Epush(__FILE__, "HE5_PTchkptid", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
      else
	{
	  /* Get file and Point key  */
	  /* ----------------------- */
	  *idx = (long)(pointID % idOffset);

	  /* Get HDF-EOS file &  "HDFEOS" group IDs */
	  /* -------------------------------------- */
	  *fid   = HE5_PTXPoint[*idx].fid;
	  status = HE5_EHchkfid(*fid,"", &HDFfid, gid, &access);
        }
    }
  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTattach                                                     |
|                                                                             |
|  DESCRIPTION: Attaches to an existing point data set.                       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  pointID        hid_t               point structure ID                      |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS file ID                         |
|  pointname      char                point structure name                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  2/24/00  A.Muslimov    Added error checkings and made cleanings.           |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|  02/01/06 Abe Taaheri   Added getting level names from Structure Metadata   |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t
HE5_PTattach(hid_t fid, const char *pointname)
{
  hid_t           pointID    = FAIL;      /* Point ID (return value)   */
  hid_t           idOffset   = HE5_PTIDOFFSET;/* Point ID offset       */
  hid_t           HDFfid     = FAIL;	  /* HDF file ID               */
  hid_t           gid        = FAIL;	  /* "POINTS" group ID         */
  hid_t           data_id    = FAIL;      /* "Data" group ID           */
  hid_t           link_id    = FAIL;	  /* "Linkage" group ID        */
  hid_t           pt_id      = FAIL;	  /* Point group ID            */
  hid_t           i,j;			  /* Loop index                */
  hid_t           emptyslot  = FAIL;	  /* Spare Point index         */

  int             npointopen =  0;  /* # of point structures opened    */
  int             nlevels    =  0;  /* Number of levels in point       */
  int             idx        = FAIL;/* Return iteration index          */

  herr_t          status     = FAIL;/* routine return status variable  */

  uintn           acs        = 77;  /* Access code                     */

  pack            tmpdata;	    /* Temporary pack structure        */

  char            *grpname = (char *)NULL;/* group name                */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */
  char            LevelName[HE5_HDFE_UTLBUFSIZE];
  char            tempname[HE5_HDFE_UTLBUFSIZE];
  hid_t           tempid;

  HE5_LOCK;
  CHECKPOINTER(pointname);

  /* Check HDF-EOS file ID, get back HDF file ID and access code */
  /* ----------------------------------------------------------- */
  status = HE5_EHchkfid(fid, pointname, &HDFfid, &gid, &acs);
  if (status == SUCCEED)
    {
      /* Determine number of points currently opened */
      /* ------------------------------------------- */
      for (i = 0; i < HE5_NPOINT; i++)
	npointopen += HE5_PTXPoint[i].active;

    
      if (npointopen < HE5_NPOINT)
        {
	  grpname = (char *)calloc( (strlen(pointname)+40), sizeof(char));
	  if( grpname == NULL )
	    {
	      sprintf(errbuf, "Cannot allocate memory.\n");
	      H5Epush(__FILE__, "HE5_PTattach", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }


	  strcpy( grpname, "/HDFEOS/POINTS/");
	  strcat( grpname, pointname);

	  /* Try to open the Point group */
	  /* =========================== */
	  H5E_BEGIN_TRY{
	    pt_id = H5Gopen( HDFfid, grpname );
	  }
	  H5E_END_TRY;

	  if( pt_id == FAIL )
	    {
	      sprintf(errbuf, "Point:\"%s\" does not exist within HDF-EOS file.\n",pointname) ;
	      H5Epush(__FILE__, "HE5_PTattach", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(grpname);
	      return(FAIL);
	    }

	  /* Attach to "Data" group */
	  /* ---------------------- */
	  data_id = H5Gopen( pt_id, "Data");
	  if( data_id == FAIL )
	    {
	      sprintf(errbuf, "Cannot open the \"Data\" group in \"%s\" Point", pointname) ;
	      H5Epush(__FILE__, "HE5_PTattach", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(grpname);
	      return(FAIL);
	    }

	  /* Attach to "Linkage" group */
	  /* ------------------------- */
	  link_id = H5Gopen( pt_id, "Linkage");
	  if( link_id == FAIL )
	    {
	      sprintf(errbuf, "Cannot open the \"Linkage\" group in \"%s\" Point", pointname) ;
	      H5Epush(__FILE__, "HE5_PTattach", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(grpname);
	      return(FAIL);
	    }


	  /* Setup External Arrays  */
	  /* ---------------------- */
	  for (i = 0; i < HE5_NPOINT; i++)
	    {
	      /* Find empty entry in array */
	      /* ------------------------- */
	      if (HE5_PTXPoint[i].active == 0)
		{
		  pointID                    = i + idOffset;
		  HE5_PTXPoint[ i ].active   = 1;
		  HE5_PTXPoint[ i ].fid      = fid;
		  HE5_PTXPoint[ i ].pt_id    = pt_id;
		  HE5_PTXPoint[ i ].data_id  = data_id;
		  HE5_PTXPoint[ i ].link_id  = link_id;
		  strcpy(HE5_PTXPoint[ i ].ptname, pointname);

		  break;
		}
	    }

	  emptyslot = i;

	  /* Initialize the members of external arrays */
	  /* ========================================= */ 
	  for( i = 0; i < HE5_DTSETRANKMAX; i++ )
	    {
	      HE5_PTXPoint[ emptyslot ].level[ i ].ID   = FAIL;
	      HE5_PTXPoint[ emptyslot ].level[ i ].name = NULL;

	      tmpdata.level[ i ].name = NULL;
	      tmpdata.level[ i ].ID   = FAIL;
	    }
	  tmpdata.nlevels = 0;

	  /* ---------------------------------------------------- */
	  /* Loop throuhgh the group "/HDFEOS/POINTS/../Data" to  */
	  /*   get the number of levels and level name list       */
	  /* ---------------------------------------------------- */

	  strcat(grpname,"/Data");

	  idx = H5Giterate(data_id, grpname , NULL, HE5_PTopenlevel, &tmpdata);
	  if ( idx == FAIL )
	    {
	      sprintf(errbuf, "Iteration of \"HE5_EHobj_info()\" operation failed.") ;
	      H5Epush(__FILE__, "HE5_PTattach", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(grpname);
 
	      return(FAIL);
	    }

	  free( grpname );

	  nlevels = tmpdata.nlevels;

	  /* sort levels using structure metadata to 
	     set from 0 to nlevels */
	  /* ==================================== */
	  for(i=0; i < nlevels; i++ )
	    {
	      status = HE5_PTgetlevelname_fromSM(pointID, i, LevelName);
	      if ( status == FAIL )
		{
		  sprintf(errbuf, "Failed to get LevelName for the Level_%d.",i) ;
		  H5Epush(__FILE__, "HE5_PTattach", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
			  
		  return(FAIL);
		}

	      /* Remove double quotes */
	      /* -------------------- */
	      memmove(LevelName, LevelName + 1, strlen(LevelName) - 2);
	      LevelName[strlen(LevelName) - 2] = 0;

	      /* replace if needed for sorting */
	      /* ------------------------------*/
	      for(j=i; j < nlevels; j++)
		{
		  if(strcmp(tmpdata.level[j].name,LevelName) == 0)
		    { 
		      strcpy(tempname, tmpdata.level[i].name);
		      tempid = tmpdata.level[ i ].ID;
		      /*strcpy(tmpdata.level[i].name, tmpdata.level[j].name);*/
		      memmove(tmpdata.level[i].name, tmpdata.level[j].name,
			      strlen(tmpdata.level[j].name) + 1);

		      tmpdata.level[ i ].ID = tmpdata.level[ j ].ID;
		      strcpy(tmpdata.level[j].name, tempname);
		      tmpdata.level[ j ].ID = tempid;
		      break;
		    }
		}
	    }

	  /* Set the data  for the PTXPoint array */
	  /* ==================================== */
	  for( i = 0; i < nlevels; i++ )
	    {
	      HE5_PTXPoint[ emptyslot ].level[ i ].name =  
		(char *)calloc( (strlen(tmpdata.level[i].name)+1), sizeof(char));
	      if ( HE5_PTXPoint[ emptyslot ].level[ i ].name == NULL )
		{
		  sprintf(errbuf, "Cannot allocate memory for the name.") ;
		  H5Epush(__FILE__, "HE5_PTattach", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  if ( tmpdata.level[ i ].name != NULL ) 
		    free(tmpdata.level[ i ].name);
 
		  return(FAIL);                  
		}
		  
	      strcpy(HE5_PTXPoint[ emptyslot ].level[ i ].name, tmpdata.level[ i ].name);
	      if ( tmpdata.level[ i ].name != NULL ) 
		free(tmpdata.level[ i ].name);

	      HE5_PTXPoint[ emptyslot ].level[ i ].ID = tmpdata.level[ i ].ID;

	    }

        }
      else
        {
	  /* Too many Points opened */
	  /* --------------------- */
	  pointID = FAIL;
	  sprintf(errbuf,"No more than %d points may be open simutaneously", HE5_NPOINT);
	  H5Epush(__FILE__, "HE5_PTattach", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
        }
    }

 COMPLETION:
  HE5_UNLOCK;
  return(pointID);

}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTdetach                                                     |
|                                                                             |
|  DESCRIPTION: Releases a point data set and frees memory.                   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t   None        point structure ID                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  8/26/99  A. Muslimov   Changed the type of fid from int32_t to hid_t.      |
|  Feb,2000 A.Muslimov    Fixed bug-prone release of IDs.                     |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTdetach(hid_t  pointID)
{
  herr_t          status  = FAIL;/* routine return status variable */

  int             j, k;			 /* Loop indices                   */
  int             nlevels = 0;	 /* Number of levels in point      */

  hid_t           fid     = FAIL;/* HDF-EOS file ID                */
  hid_t           gid     = FAIL;/* HDFEOS group ID                */

  long            idx     = FAIL;/* Point index                    */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer  */


  HE5_LOCK;

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTdetach", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTdetach", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if(nlevels == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the number of levels. \n");
      H5Epush(__FILE__, "HE5_PTdetach", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* close Point level datasets */
  /* -------------------------- */
  for (j = 0; j < nlevels; j++)
    {
      if ( HE5_PTXPoint[ idx ].level[ j ].ID > 0 )
	{
	  status = H5Dclose(HE5_PTXPoint[ idx ].level[ j ].ID);
	  if(status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the level dataset ID\n");
	      H5Epush(__FILE__, "HE5_PTdetach", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }

	}
	 
    }
    

  /* close Point "Data" group   */
  /* -------------------------- */
  if ( HE5_PTXPoint[ idx ].data_id > 0 )
    {
      status = H5Gclose(HE5_PTXPoint[ idx ].data_id);
      if(status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the group \"Data\" group ID \n");
	  H5Epush(__FILE__, "HE5_PTdetach", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }
    
  /* close Point "Linkage" group */
  /* --------------------------- */ 
  if ( HE5_PTXPoint[idx].link_id > 0 )
    {
      status = H5Gclose(HE5_PTXPoint[ idx ].link_id);
      if(status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the group \"Linkage\" group ID\n");
	  H5Epush(__FILE__, "HE5_PTdetach", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

    }
    
  /* close the Point group */
  /* --------------------- */ 
  if ( HE5_PTXPoint[ idx ].pt_id > 0 )
    {
      status = H5Gclose(HE5_PTXPoint[ idx ].pt_id);
      if(status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the Point group ID\n");
	  H5Epush(__FILE__, "HE5_PTdetach", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
	    
    }
    

  /* Clear entries from external arrays */
  /* ---------------------------------- */
  HE5_PTXPoint[ idx ].active       =  0;
  HE5_PTXPoint[ idx ].fid          =  FAIL;
  HE5_PTXPoint[ idx ].data_id      =  FAIL;
  HE5_PTXPoint[ idx ].link_id      =  FAIL;
  HE5_PTXPoint[ idx ].pt_id        =  FAIL;

  for (j = 0; j < nlevels; j++)
    {
      HE5_PTXPoint[ idx ].level[ j ].ID   = FAIL;
      if ( HE5_PTXPoint[ idx ].level[ j ].name != NULL )
	free(HE5_PTXPoint[ idx ].level[ j ].name);
    }


  /* Free Region Pointers */
  /* -------------------- */
  for (k = 0; k < HE5_NPOINTREGN; k++)
    {
      if (HE5_PTXRegion[ k ] != NULL && HE5_PTXRegion[ k ]->pointID == pointID)
	{
	  for (j = 0; j < HE5_DTSETRANKMAX; j++)
	    {
	      if (HE5_PTXRegion[ k ]->recPtr[ j ] != NULL)
		{
		  free(HE5_PTXRegion[ k ]->recPtr[ j ]);
		}
	    }

	  free(HE5_PTXRegion[ k ]);
	  HE5_PTXRegion[ k ] = NULL;
	}
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTclose                                                      |
|                                                                             |
|  DESCRIPTION: Closes HDF-EOS file                                           |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t   None        File ID                                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7/20/99  A.Muslimov                                                        |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_PTclose(hid_t fid)
{
  herr_t  status = FAIL;/* routine return status variable */

  /* Call HE5_EHclose to perform file close */
  /* -------------------------------------- */
  status = HE5_EHclose(fid);

  return (status);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTnrecs                                                      |
|                                                                             |
|  DESCRIPTION: Returns the number of records in a level.                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nrec           hsize_t             Number of records in level              |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/13/99      A.Muslimov    Added proper error handlings after the function|
|                              calls.                                         |
|  7.18.99       A.Muslimov    Unmasked hdf5 data types.                      |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hsize_t 
HE5_PTnrecs(hid_t pointID, int level)
{
  hsize_t         nrec = 1;		/* Number of records in level (return value) */

  herr_t          status = FAIL;/* routine return status variable            */   

  int             i;			/* loop index                                */
  int             nlevels = 0;  /* Number of levels in point                 */

  hid_t           fid   = FAIL;	/* HDF-EOS file ID                           */
  hid_t           gid   = FAIL;	/* "POINTS" group ID                         */
  hid_t	          space = FAIL; /* data space ID                             */
  hid_t           ID    = FAIL;	/* Level dataset ID                          */

  long            idx   = FAIL; /* Point Index                               */

  unsigned int    zerobuf[HE5_DTSETRANKMAX];

  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* error message buffer       */


  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    zerobuf[ i ] = 0;
    
  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTnrecs", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTnrecs", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(0);
    }

  /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if (nlevels == 0)
    {
      sprintf(errbuf, "No Levels Defined for point ID: %d\n", pointID);
      H5Epush(__FILE__, "HE5_PTnrecs", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(0);
    }
  else if (nlevels <= level)
    {
      /* Report error if level # to large */
      /* -------------------------------- */
      sprintf(errbuf, "Only %d levels Defined for point ID: %d\n", nlevels, pointID);
      H5Epush(__FILE__, "HE5_PTnrecs", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(0);
    }

  /* Get level dataset ID and number of records */
  /* ------------------------------------------ */
  ID = HE5_PTXPoint[idx].level[level].ID;

  space = H5Dget_space(ID);
  if (space == FAIL)
    {
      sprintf(errbuf, "Cannot get the data space ID.\n");
      H5Epush(__FILE__, "HE5_PTnrecs", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(0);
    }


  nrec = H5Sget_simple_extent_npoints(space);
  if (nrec == 0)
    {
      sprintf(errbuf, "Cannot get the number of records.\n");
      H5Epush(__FILE__, "HE5_PTnrecs", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(0);
    }

  status = H5Sclose(space);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the data space ID.\n");
      H5Epush(__FILE__, "HE5_PTnrecs", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(0);
    }


  /* If nrec = 1 check whether actual data has been written */
  /* ------------------------------------------------------ */
  if (nrec == 1)
    {
      /* read  "_LevelWritten" Attribute */
      /* ------------------------------- */
      status = HE5_PTreadattr(pointID,"_LevelWritten", zerobuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot read the \"_LevelWritten\" attribute.\n");
	  H5Epush(__FILE__, "HE5_PTnrecs", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(0);
	}

      /* If level not yet written then reset nrec to 0 */
      /* --------------------------------------------- */
      if ( zerobuf[level] == 0)
	nrec = 0;
		
    }
 
  return(nrec);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTnlevels                                                    |
|                                                                             |
|  DESCRIPTION: Returns the number of levels in a point data set.             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nlevels        int                 Number of levels in point structure     |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t      	      point structure ID                      |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTnlevels(hid_t pointID)
{
  int             nlevels = FAIL;	/* Number of levels in point (return value) */

  herr_t          status  = SUCCEED;/* routine return status variable           */

  hid_t           fid     = FAIL;	/* HDF-EOS file ID                          */
  hid_t           gid     = FAIL;	/* "POINTS" group ID                        */

  long            idx     = FAIL;   /* Point index                              */

  char            *grpname = (char *)NULL;/* Group name string buffer           */

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTnlevels", &fid, &gid, &idx);

  if(status == SUCCEED )
    {
      grpname = (char *) calloc( (strlen(HE5_PTXPoint[idx].ptname)+40),sizeof(char));
      if( grpname == NULL)
    	{
	  H5Epush(__FILE__, "HE5_PTnlevels", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Can not allocate memory");
	  HE5_EHprint("Error: Can not allocate memory, occured", __FILE__, __LINE__);
	  return(FAIL);
    	}

      /* Build the  "grpname" string */
      /* --------------------------- */
      strcpy(grpname,"/HDFEOS/POINTS/");
      strcat(grpname, HE5_PTXPoint[idx].ptname);
      strcat(grpname,"/Data");   

      /* Get number of levels (Number of entries) in Data group */
      /* ------------------------------------------------------ */
      nlevels = (int)HE5_EHdatasetcat(HE5_PTXPoint[idx].fid, grpname, NULL, NULL);

      free(grpname);
    }

  return(nlevels);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTfexist                                                     |
|                                                                             |
|  DESCRIPTION:  Verify if field exist in a compound datatype and return      |
  |		 the member data information                                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS:                                                                    |
|   dataID        hid_t               Level dataset ID                        |
|   fldname       char                Field name                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|   size          size_t              Field data size (bytes)                 |
|   rank          int                 Field rank                              |
|   dims[]        size_t              Array of field dimension sizes          |
|   dtype         hid_t               Field data type ID                      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  Oct 00   A.Muslimov    Changed the interface.                              |
|  Mar 01   A.Muslimov    Updated to reflect introduction of ARRAY class.     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static int
HE5_PTfexist(hid_t  dataID, const char *fldname, size_t *size, int *rank, size_t dims[], hid_t *dtype )
{
  int            exist  = FAIL;	 /* return flag value                */  
  int            i, j;			 /* loop indices                     */

  herr_t         status = FAIL;  /* Return status variable           */

  hid_t	         typeID = FAIL;  /* data type ID                     */

  hsize_t        dim_size[HE5_DTSETRANKMAX];/* array of dime. sizes  */

  HE5_CmpDTSinfo *info;          /* Compound dataset info structure  */

  char           errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer */

  CHECKPOINTER(fldname);

  /* Allocate memory for info structure */
  /* ---------------------------------- */
  info = (HE5_CmpDTSinfo *)calloc(1, sizeof(HE5_CmpDTSinfo));
  if (info == (HE5_CmpDTSinfo *)NULL)
    {
      sprintf(errbuf, "Cannot allocate memory for info structure.\n");
      H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get level dataset type  ID */
  /* -------------------------- */
  typeID = H5Dget_type(dataID);
  if (typeID == FAIL)
    {
      sprintf(errbuf, "Cannot get data type.\n");
      H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(info);
      return(FAIL);
    }

  /* Initialize dim_size[] array */
  /* --------------------------- */
  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    dim_size[i] = 0;
  
  /* Reset the flag */
  /* -------------- */ 
  exist = 0;

  /* get the number of members in compound datatype */
  /* ---------------------------------------------- */
  info->nfields = H5Tget_nmembers( typeID );	
  for( i = 0; i < info->nfields; i++ )
    {
      /* Get member name */
      /* --------------- */
      info->fieldname[i] = H5Tget_member_name(typeID, i);
      if (info->fieldname[i] == NULL)
	{
	  sprintf(errbuf, "Cannot get the field name.\n");
	  H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (info != NULL) free(info);
	  return(FAIL);
	}
	  
      /* Loop through all fields */
      /* ----------------------- */
      if( strncmp(info->fieldname[i], fldname, strlen(fldname)) == 0 )
	{
	  exist = 1;

	  /* Get field disk data type */
	  /* ------------------------ */
	  info->dtype[i] = H5Tget_member_type(typeID, i);
	  if (info->dtype[i] == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the field data type.\n");
	      H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (info->fieldname[i] != NULL) 
		free(info->fieldname[i]); 
	      if (info != NULL) free(info);
	      return(FAIL);
	    }
			 

	  /* Get field data type */
	  /* ------------------- */
	  if (dtype != NULL)
	    *dtype = info->dtype[i];

		  
	  /* Get field data size */
	  /* ------------------- */
	  if (size != NULL)
	    {
	      /* Get the field disk data size (in bytes) */
	      /* --------------------------------------- */
	      *size = H5Tget_size(info->dtype[i]);
	      if (*size == 0)
		{
		  sprintf(errbuf, "Cannot get the field size.\n");
		  H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  if (info->fieldname[i] != NULL) 
		    free(info->fieldname[i]);
		  if (info != NULL) free(info);
		  return(FAIL);
		}
	    }
		  
	  /* Get the field rank and dimension sizes */
	  /* -------------------------------------- */
	  if (rank != NULL && dims != NULL)
	    {
	      /* Get field data class */
	      /* -------------------- */
	      info->dclass[i] = H5Tget_class(info->dtype[i]);
	      if (info->dclass[i] == H5T_NO_CLASS)
		{
		  sprintf(errbuf, "Cannot get the field class.\n");
		  H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  if (info->fieldname[i] != NULL) 
		    free(info->fieldname[i]); 
		  if (info != NULL) free(info);
		  return(FAIL);
		}	  

	      /* Get the rank and "array" flag */
	      /* ----------------------------- */
	      if (info->dclass[i] == H5T_ARRAY)
		{
		  /* Get the field rank */
		  /* ------------------ */
		  *rank  = H5Tget_array_ndims(info->dtype[i]);
		  if (*rank == FAIL)
		    {
		      sprintf(errbuf, "Cannot get the field rank.\n");
		      H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      if (info->fieldname[i] != NULL) 
			free(info->fieldname[i]); 
		      if (info != NULL) free(info);
		      return(FAIL);
		    }	  

		  /* Get the field dimension sizes */
		  /* ----------------------------- */
		  status = H5Tget_array_dims(info->dtype[i],dim_size, NULL);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot get the field dimension sizes.\n");
		      H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      if (info->fieldname[i] != NULL) 
			free(info->fieldname[i]); 
		      if (info != NULL) free(info);
		      return(FAIL);
		    }	  

		  for (j = 0; j < *rank; j++)
		    dims[j] = dim_size[j];
		}
	      else /* scalar */
		{
		  *rank   = 1;
		  dims[0] = 1;
		}

	    }

	  /* Release field type ID */
	  /* --------------------- */
	  status = H5Tclose(info->dtype[i]);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release data type ID.\n");
	      H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (info->fieldname[i] != NULL) 
		free(info->fieldname[i]); 
	      if (info != NULL) free(info);
	      return(FAIL);
	    }
		  
	}

      /* Release pointers to the name buffers */
      /* ------------------------------------ */
      if (info->fieldname[i] != NULL) 
	free(info->fieldname[i]);
    }

  if (info != NULL) free(info);
 
  /* Release ID */
  /* ---------- */
  status = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release data type ID.\n");
      H5Epush(__FILE__, "HE5_PTfexist", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
 COMPLETION:
  return(exist);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTnfields                                                    |
|                                                                             |
|  DESCRIPTION: Returns number of fields defined in a level.                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nflds          int                 Number of fields in a level             |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|  OUTPUTS:                                                                   |
|  strbufsize     long                String length of fieldlist              |
|  fieldlist      char                Coma separated list of fields           |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date       Programmer   Description                                       |
|  ========   ============  ================================================= |
|  12/16/99   A.Muslimov    Added appropriate error handlings after the       |
|                           function calls.                                   |
|  7.18.99    A.Muslimov    Unmasked hdf5 data types.                         |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTnfields(hid_t pointID, int level, char *fieldlist, long *strbufsize)
{
  int             nflds   = FAIL;/* Number of fields in a list (return) */
  int             nlevels = 0;   /* Number of levels in point           */
  int             i;             /* Loop index                          */

  herr_t          status  = FAIL;/* routine return status variable      */

  hid_t           fid     = FAIL;/* HDF-EOS file ID                     */
  hid_t           gid     = FAIL;/* POINTS group ID                     */
  hid_t           levelID = FAIL;/* Level dataset ID                    */        
  hid_t           tid     = FAIL;/* Data type ID                        */

  long            idx     = FAIL;/* Point index                         */
 
  size_t          strsize = 0;   /* String length                       */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer   */
  char            *fldname = (char *)NULL;    /* pointer to field name  */
  char            *fldlist = (char *)NULL;    /* pointer to field list  */


  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTnfields", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if (nlevels == 0)
    {
      status = FAIL;
      sprintf(errbuf, "No Levels Defined for point ID: %d\n", pointID);
      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  else if (nlevels <= level)
    {
      /* Report error if level # too large */
      /* --------------------------------- */
      status = FAIL;
      sprintf(errbuf, "Only %d levels Defined for point ID: %d\n", nlevels, pointID);
      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get level dataset  ID */
  /* --------------------- */
  levelID = HE5_PTXPoint[idx].level[level].ID;

  /* Get Compound dataset datatype ID */
  /* -------------------------------- */
  tid = H5Dget_type(levelID);
  if (tid == FAIL)
    {
      sprintf(errbuf, "Cannot get the compound data type ID. \n");
      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get number of fields( or number of members of compound type) */
  /* ------------------------------------------------------------ */
  nflds = H5Tget_nmembers(tid);
  if (nflds == FAIL)
    {
      sprintf(errbuf, "Cannot get the number of fields in compound data type. \n");
      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate memory */
  /* --------------- */
  fldname = (char *)calloc(HE5_HDFE_NAMBUFSIZE, sizeof(char));
  if( fldname == NULL)
    {
      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory.");
      HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
      return(FAIL);
    }
    

  /* Get field list, field list size */
  /* ------------------------------- */
  for( i = 0; i < nflds; i++ )
    {
      fldname = H5Tget_member_name(tid, i);
      if (fldname == NULL )
	{
	  sprintf(errbuf, "Cannot get the compound data field name. \n");
	  H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      strsize = strsize + strlen(fldname) + 1 ;

      if( i == 0  )
	{
	  fldlist = (char *) calloc( strsize, sizeof(char));
	  if( fldlist == NULL)
	    {
	      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
	      HE5_EHprint("Error: Can not allocate memory, occured", __FILE__, __LINE__);
	      nflds = FAIL; 
	      free(fldname);
	      break;
   
	    }  
	  strcpy(fldlist,fldname);
        
	}
      else
	{ 
	  fldlist= (char *)realloc(fldlist, sizeof(char)* strsize);
	  if( fldlist==NULL)
	    {
	      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
	      HE5_EHprint("Error: Can not allocate memory, occured", __FILE__, __LINE__);
	      free(fldname);
	      nflds = FAIL;  
	      break;
	    }

	  strcat(fldlist,",");
	  strcat(fldlist,fldname);
	}

      if (fldname != NULL) free(fldname);
    }

  /* Release data type ID */
  /* -------------------- */
  status = H5Tclose(tid);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release the compound data type ID. \n");
      H5Epush(__FILE__, "HE5_PTnfields", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(fldlist);
		
      return(status);
    }

  /* Return fieldlist string size */
  /* ---------------------------- */
  if ( strbufsize != NULL )
    {
      *strbufsize = (long)strlen(fldlist);
    }
    
  /* Also return a coma separated list of fields */
  /* ------------------------------------------- */
  if(fieldlist != NULL)
    strcpy(fieldlist, fldlist);

  free(fldlist);

  return(nflds);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTgetlevelname                                               |
|                                                                             |
|  DESCRIPTION: Returns level name                                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  levelname      char                Level name                              |
|  strbufsize     long                String length of fieldlist              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTgetlevelname(hid_t pointID, int  level, char *levelname, long *strbufsize)
{
  herr_t          status  = FAIL;/* routine return status variable  */

  int             nlevels =  0;	 /* Number of levels in point       */

  hid_t           fid     = FAIL;/* HDF-EOS file id                 */
  hid_t           gid     = FAIL;/* "POINTS" group ID               */

  long            idx     = FAIL;/* Point index                     */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */


  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTgetlevelname", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get number of levels */
      /* -------------------- */
      nlevels = HE5_PTnlevels(pointID);
      if (nlevels == 0)
	{
	  status = FAIL;
	  sprintf(errbuf, "No Levels Defined for point ID: %d\n", pointID);
	  H5Epush(__FILE__, "HE5_PTgetlevelname", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
            
	}
      else if (nlevels <= level)
	{
	  /* Report error if level # to large */
	  /* -------------------------------- */
	  status = FAIL;
	  sprintf(errbuf, "Only %d levels Defined for point ID: %d\n", nlevels, pointID);
	  H5Epush(__FILE__, "HE5_PTgetlevelname", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}

      if (status == SUCCEED)
	{
	  /* Get level name */
	  /* -------------- */
	  *strbufsize = (long)strlen(HE5_PTXPoint[ idx ].level[ level ].name);

	  /* Return levelname if requested */
	  /* ----------------------------- */
	  if (levelname != NULL)
	    strcpy(levelname,HE5_PTXPoint[ idx ].level[ level ].name)  ;
	}
    }

  return (status);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTopenlevel                                                  |
|                                                                             |
|  DESCRIPTION: Returns level ID and name                                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|   =======  ==========   ==============                                      |
|   7.18.99  A.Muslimov    Unmasked hdf5 data types.                          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_PTopenlevel(hid_t loc_id, const char *name, void *opdata)
{
  int      i = 0;

  pack     *ptr;

  ptr = (pack *)opdata;
    
  if( name != NULL )
    {
      i = ptr->nlevels;
      if( ptr->level[i].name == NULL && ptr->level[i].ID == FAIL )
	{
	  ptr->level[i].name = (char *)calloc((strlen(name) + 1), sizeof(char)); 
	  strcpy( ptr->level[i].name, name );
	  ptr->level[i].ID = H5Dopen( loc_id, name );
	}
      ptr->nlevels ++ ;
    }

  return 0;

}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTdeflevel                                                   |
|                                                                             |
|  DESCRIPTION: Defines a level within the point data set.                    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t               return status (0) SUCCEED, (-1) FAIL   |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  levelname      char                name of level                           |
|  levelinfo      HE5_CmpDTSinfo      Pointer to the structure containing     |
|                                      information about level dataset        |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:     Metadata block needs to be developed                            |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Oct 00   A.Muslimov    Revised and recoded.                                |
|  Mar 00   A.Muslimov    Modified to reflect changes in HDF5 library:        |
|                         H5Tinsert_array should be replaced by two calls     |
|                         H5Tarray_create() and H5Tinsert().                  |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|  May 05   S.Zhao        Added HE5_EHdtype2numtype() and HE5_EHhid2hsize()   |
|                         functions calls.                                    |
|  Feb 06   Abe Taaheri   Added support for H5T_STRING                        |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_PTdeflevel(hid_t  pointID, const char *levelname, HE5_CmpDTSinfo *levelinfo)
{
  herr_t          status  = FAIL;/* routine return status variable     */

  int             i, j;		 /* Loop indices                       */
  int             nlevels = FAIL;/* Number of levels in point          */
  int             rank    = 1;   /* Dataset rank                       */

  hid_t           fid     = FAIL;/* HDF-EOS file ID                    */
  hid_t           gid     = FAIL;/* HDFEOS group ID                    */
  hid_t           dtsID   = FAIL;/* Dataset ID                         */
  hid_t           space   = FAIL;/* dataspace ID                       */
  hid_t           tid     = FAIL;/* compound datatype ID               */
  hid_t           fwdtid  = FAIL;/* compound datatype ID               */
  hid_t           bcktid  = FAIL;/* compound datatype ID               */
  hid_t           plist   = FAIL;/* property list ID                   */
  hid_t           tempid  = FAIL;/* temporary ID                       */
  hid_t           xfrlist = FAIL;/* Xfer property list ID              */
  hid_t           arr_id  = FAIL;/* Array datatype ID                  */

  long            idx     = FAIL;/* Point index                        */

  hsize_t         dim         = 1;/* Dimension size                    */    
  hsize_t         dims[HE5_DTSETRANKMAX];/* Array of dimension sizes   */
  hsize_t         chunkdim[1] = {HE5_NRECS};/* Record chunk size       */ 
  hsize_t         maxdim      = H5S_UNLIMITED;/* Max. dimension size   */
  hsize_t         dum         = 0;/* dummy variable                    */
  hsize_t         metadata[2]={0,0};/* Metadata input array            */
  hid_t           ntype   = FAIL;   /* number type ID                  */

  fwdptr          ForwardData;/* Structure for forward pointer dataset */

  char            utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer        */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */
  int             order;

  HE5_LOCK;
  CHECKNAME(levelname);
  CHECKPOINTER(levelinfo);

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTdeflevel", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Check levelname for length */
  /* -------------------------- */
  if ( strlen(levelname) > HE5_OBJNAMELENMAX)
    {
      sprintf(errbuf, "Levelname \"%s\" more than %d characters.\n", levelname, HE5_OBJNAMELENMAX);
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  
  /* Initialize Forward Pointer buffer */
  /* --------------------------------- */
  ForwardData.begin  = FAIL;
  ForwardData.extent = FAIL;

  /* Get number of levels in points */
  /* ------------------------------ */
  nlevels = HE5_PTnlevels(pointID);
  if(nlevels == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the number of levels\n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Create dataspace with rank=1, dim=1(1 record), unlimited */
  /* -------------------------------------------------------- */
  space = H5Screate_simple(rank, &dim, &maxdim);
  if (space == FAIL)
    {
      sprintf(errbuf, "Cannot create the data space. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Create and modify dataset property, enable chunking */
  /* --------------------------------------------------- */
  plist = H5Pcreate(H5P_DATASET_CREATE);
  if (plist == FAIL)
    {
      sprintf(errbuf, "Cannot create a property list ID. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Enable chunking */
  /* --------------- */
  status = H5Pset_chunk(plist, rank, chunkdim);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot set chunking. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  
  
  /* Create the memory compound data type */
  /* ------------------------------------ */
  tid = H5Tcreate(H5T_COMPOUND, levelinfo->datasize);
  if (tid == FAIL)
    {
      sprintf(errbuf, "Cannot create the data type ID. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Add members to the compound data type */
  /* ------------------------------------- */
  for (i = 0; i < levelinfo->nfields; i++)
    {  
      /* Check if the field is an array */
      /* ------------------------------ */
      if (levelinfo->array[i] == TRUE)
	{
	  for (j = 0; j < levelinfo->rank[i]; j++)
	    dims[j] = (hsize_t)levelinfo->dims[i][j];
		  
	  /* Create array datatypes */
	  /* ---------------------- */
	  arr_id = H5Tarray_create(levelinfo->dtype[i], levelinfo->rank[i], (const hsize_t *)dims, NULL);
	  if (arr_id == FAIL)
	    {
	      sprintf(errbuf, "Cannot create \"array\" datatype. \n");
	      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Add array field */
	  /* --------------- */
	  status = H5Tinsert(tid,levelinfo->fieldname[i], levelinfo->offset[i], arr_id);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot insert \"array\" element. \n");
	      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Release the data type ID */
	  /* ------------------------ */
	  status = H5Tclose(arr_id);
	  if (status == FAIL )
	    {
	      sprintf(errbuf, "Cannot release data type ID. \n");
	      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    } 

	}
      else 
	{
	  /* Add atomic field */
	  /* ---------------- */
	  status = H5Tinsert(tid,levelinfo->fieldname[i], levelinfo->offset[i],levelinfo->dtype[i]);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot insert \"array\" element. \n");
	      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}
	  

    }
   

  /* Create (appendible) dataset for the level */
  /* ----------------------------------------- */
  dtsID = H5Dcreate(HE5_PTXPoint[idx].data_id, levelname, tid, space, plist);
  if (dtsID == FAIL)
    {
      sprintf(errbuf, "Cannot create the dataset. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Extend the dataset */
  /* ------------------ */
  status = H5Dextend(dtsID, &dim);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot extend the dataset. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  

  /* Release the data type ID */
  /* ------------------------ */
  status = H5Tclose(tid);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release data type ID. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Release the data space ID */
  /* ------------------------- */
  status = H5Sclose(space);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release data space ID. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
	
  /* Release the property list ID */
  /* ---------------------------- */
  status = H5Pclose(plist);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release a property list ID. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  

  /* Store dataset ID and name in external array */
  /* ------------------------------------------- */
  HE5_PTXPoint[idx].level[nlevels].ID  = dtsID;
  HE5_PTXPoint[idx].level[nlevels].name = (char *)calloc((strlen(levelname)+1), sizeof(char));
  strcpy(HE5_PTXPoint[idx].level[nlevels].name, levelname);

  /* If previous levels exist ... */
  /* ---------------------------- */
  if (nlevels > 0)
    {
      /* Create dataspace with rank=1, dim=1(1 record), unlimited */
      space = H5Screate_simple(rank, &dim, &maxdim);
      if (space == FAIL)
	{
	  sprintf(errbuf, "Cannot create the  data space. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Create property list */
      /* -------------------- */
      plist = H5Pcreate(H5P_DATASET_CREATE);
      if (plist == FAIL)
	{
	  sprintf(errbuf, "Cannot create a property list ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Enable chunking */
      /* --------------- */
      status = H5Pset_chunk(plist,rank, chunkdim);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot set chunking. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Create compound data type */
      /* ------------------------- */
      bcktid = H5Tcreate(H5T_COMPOUND, sizeof(bckptr));
      if (bcktid == FAIL)
	{
	  sprintf(errbuf, "Cannot create a compound datatype. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
	  
      /* Add back pointer data member */
      /* ---------------------------- */
      status = H5Tinsert(bcktid,"BCKPOINTER", HOFFSET(bckptr, back), H5T_NATIVE_LLONG);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot add a new member named \"BCKPOINTER\"to a compound datatype. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  

      /* Set name of BCKPOINTER dataset */
      /* ------------------------------ */
      sprintf(utlbuf, "%s%d%s%d", "BCKPOINTER:", nlevels, "->", nlevels - 1);


      /* Create (appendible) compound dataset */
      /* ------------------------------------ */
      tempid = H5Dcreate(HE5_PTXPoint[idx].link_id, utlbuf, bcktid, space, plist);
      if (tempid == FAIL)
	{
	  sprintf(errbuf, "Cannot create the  dataset. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
  
      /* Extend the dataset */
      /* ------------------ */
      status = H5Dextend(tempid, &dim);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot extend the  dataset. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}

      /* Release the data type ID */
      /* ------------------------ */
      status = H5Tclose(bcktid);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the  data type ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}

      /* Release the data space ID */
      /* ------------------------- */
      status = H5Sclose(space);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot release the  data space ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}

      /* Release the dataset ID */
      /* ---------------------- */
      status = H5Dclose(tempid);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot release the  dataset ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}


      /* Setup Forward Pointer dataset */
      /* ----------------------------- */
	  
      /* Set name of FWDPOINTER dataset */
      /* ------------------------------ */
      sprintf(utlbuf, "%s%d%s%d", "FWDPOINTER:", nlevels - 1,"->", nlevels);
	  
      /* Create dataspace with rank=1, dim=1(1 record), unlimited */
      /* -------------------------------------------------------- */
      space = H5Screate_simple(rank, &dim, &maxdim);
      if (space == FAIL)
	{
	  sprintf(errbuf, "Cannot create the data space ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Create the memory data type */
      /* --------------------------- */
      fwdtid = H5Tcreate(H5T_COMPOUND, sizeof(fwdptr));
      if (fwdtid == FAIL)
	{
	  sprintf(errbuf, "Cannot create  a compound datatype. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Insert "BEGIN" data member to the dataset */
      /* ----------------------------------------- */
      status = H5Tinsert(fwdtid,"BEGIN", HOFFSET(fwdptr, begin), H5T_NATIVE_LLONG);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot add a new member named \"BEGIN\"to a compound datatype. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Insert "EXTENT" data member to the dataset */
      /* ------------------------------------------ */
      status = H5Tinsert(fwdtid,"EXTENT", HOFFSET(fwdptr, extent), H5T_NATIVE_LLONG);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot add a new member named \"EXTENT\"to a compound datatype. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
	  
      /* Create (appendible) dataset */
      /* --------------------------- */
      tempid = H5Dcreate(HE5_PTXPoint[idx].link_id, utlbuf, fwdtid, space, plist);
      if (tempid == FAIL)
	{
	  sprintf(errbuf, "Cannot create the dataset. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Extend the dataset */
      /* ------------------ */
      status = H5Dextend(tempid, &dim);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot extend the dataset. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}
	  
  
      /* Create X-fer property list */
      /* -------------------------- */
      xfrlist = H5Pcreate(H5P_DATASET_XFER);
      if (xfrlist == FAIL)
	{
	  sprintf(errbuf, "Cannot create x-fer property list ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Set type conversion and background buffer */
      /* ----------------------------------------- */
      status = H5Pset_buffer(xfrlist,sizeof(fwdptr), NULL, NULL);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot set the buffer. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Write out buffer (with -1 fill value) to establish dataset */
      /* --------------------------------------------------------- */
      status = H5Dwrite(tempid, fwdtid, H5S_ALL, H5S_ALL, xfrlist, &ForwardData);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write data to the dataset. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}
	  
	  

      /* Release the data type ID */
      /* ------------------------ */
      status = H5Tclose(fwdtid);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data type ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;		
	}

	
      /* Release the data space ID */
      /* ------------------------- */  
      status = H5Sclose(space);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data space ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}


      /* Release the property list ID */
      /* ---------------------------- */
      status = H5Pclose(plist);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot release the property list ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}

      /* Release the x-ferproperty list ID */
      /* --------------------------------- */
      status = H5Pclose(xfrlist);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot release the property list ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}
	  

      /* Release the dataset ID */
      /* ---------------------- */
      status = H5Dclose(tempid);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the dataset ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}
	  
    }
  

  /*
**************************************************************
*             M E T A D A T A   S E C T I O N                *
**************************************************************
*/  
  
  /* Insert Point Level metadata */
  /* --------------------------- */
  status = HE5_EHinsertmeta(fid , HE5_PTXPoint[idx].ptname, "p", 10L, (char *)levelname, &dum);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot insert Point Level metadata \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return status;
    }
	    
  for (i = 0; i < levelinfo->nfields; i++)
    {
      /* Concatenate fieldname with level name */
      /* ------------------------------------- */
      memmove(utlbuf, levelinfo->fieldname[i], strlen(levelinfo->fieldname[i]));
      utlbuf[strlen(levelinfo->fieldname[i])] = 0;
      strcat(utlbuf, ":");
      strcat(utlbuf, levelname);
	  
	  
      /* Load ntype and field order into metadata input array */
      /* ---------------------------------------------------- */
      levelinfo->dclass[i] = H5Tget_class(levelinfo->dtype[i]);
      /* If datatype class is H5T_STRING, then the field is
	 string type */
      if (levelinfo->dclass[i] == H5T_STRING)
	{
	  ntype = HE5T_CHARSTRING;
	}
      else
	{ 
	  ntype = HE5_EHdtype2numtype(levelinfo->dtype[ i ]);
	}

      if (ntype == FAIL)
	{
	  sprintf(errbuf, "Cannot get the number type ID. \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      else if (ntype == 0)
	{
	  metadata[0] = 0;
	}
      else
	{
	  metadata[0] = HE5_EHhid2hsize(ntype);
	  if (metadata[0] == 0)
	    {
	      sprintf(errbuf, "Cannot convert \"hid_t\" to \"hsize_t\" data type. \n");
	      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}

      if (levelinfo->array[i] == TRUE)
	{
	  order = levelinfo->rank[i];
	  metadata[1] = order;
	}
      else
	{
	  metadata[1] = 1;
	}

      /* Set field order (change order = 0 to order = 1) for scalars*/
      /* ---------------------------------------------------------- */
	  
      if (metadata[1] == 0)
	{
	  metadata[1] = 1;
	}
	  
      /* Insert point field metadata */
      /* --------------------------- */
      status = HE5_EHinsertmeta(fid, HE5_PTXPoint[idx].ptname, "p", 11L, utlbuf, metadata);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot insert Point Field metadata \n");
	  H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return status;
	}
    }
  
 COMPLETION:
  HE5_UNLOCK;
  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTdeflinkage                                                 |
|                                                                             |
|  DESCRIPTION: Defines link field between two (adjacent) levels.             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  parent         char                parent level name                       |
|  child          char                child level name                        |
|  linkfield      char                linkage field name                      |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date        Programmer   Description                                      |
|  =========   ============  ==============================================   |
|  12/13/99    A.Muslimov    Added proper error handlings after the function  |
|                            calls. In the call EHinsertmeta() replaced gid   |
|                            by fid.                                          |
|  1/06/00     A.Muslimov    Changed datatype of "dum" from int32_t to hsize_t|
|                            to make a call to EHinsertmeta() successfull.    |
|  7.18.99     A.Muslimov    Unmasked hdf5 data types.                        |
|  Oct 00      A.Muslimov    Modified to reflect changes in HE5_PTfexist().   |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_PTdeflinkage(hid_t pointID, char *parent, char *child, char *linkfield)
{
  herr_t          status      = FAIL;/* routine return status variable */

  int             i;			     /* Loop index                     */
  int             nlevels     = FAIL;/* Number of levels in point      */
  int             foundParent = FAIL;/* Found parent level flag        */
  int             foundChild  = FAIL;/* Found child level flag         */

  hid_t           fid         = FAIL;/* HDF-EOS file ID                */
  hid_t           gid         = FAIL;/* POINTS group ID                */
  hid_t           dtsid       = FAIL;/* Dataset ID                     */

  long            idx         = FAIL;/* Point index                    */

  hsize_t         dum         =  0;/* dammy variable */

  char            utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer        */
  char            *mess  = "Linkage Field \"%s\" not found in Parent Level: \"%s\".\n";
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer  */

  HE5_LOCK;
  CHECKPOINTER(parent);
  CHECKPOINTER(child);
  CHECKPOINTER(linkfield);

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTdeflinkage", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTdeflinkage", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Get number of levels in point */
  /* ----------------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if(nlevels == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the number of levels\n");
      H5Epush(__FILE__, "HE5_PTdeflinkage", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* For all levels ... */
  /* ------------------ */
  for (i = 0; i < nlevels; i++)
    {
      /* Get level data ID and name */
      /* -------------------------- */
      dtsid = HE5_PTXPoint[idx].level[i].ID;
      strcpy(utlbuf, HE5_PTXPoint[idx].level[i].name);

      /* If equal to parent level name ... */
      /* --------------------------------- */
      if (strcmp(utlbuf, parent) == 0)
	{
	  /* Set found parent flag */
	  /* --------------------- */
	  foundParent = i;

	  /* Check if linkfield exists in parent level */
	  if (HE5_PTfexist(dtsid,linkfield,NULL,NULL,NULL,NULL) == 1 )
	    {
	      break;
	    }
	  else
	    {
	      /* ... else report error */
	      /* --------------------- */
	      status = FAIL;
	      sprintf(errbuf, mess, linkfield, parent);
	      H5Epush(__FILE__, "HE5_PTdeflinkage", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      break;

	    }

	}
	    
    }
    
  /* For all levels ... */
  /* ------------------ */
  for (i = 0; i < nlevels; i++)
    {
      /* Get level  ID and name */
      /* -------------------------- */
      dtsid = HE5_PTXPoint[idx].level[i].ID;
      strcpy(utlbuf, HE5_PTXPoint[idx].level[i].name);

      /* If equal to child level name ... */
      /* -------------------------------- */
      if (strcmp(utlbuf, child) == 0)
	{
	  /* Set found child flag */
	  /* -------------------- */
	  foundChild = i;

	  /* Check if linkfield exists in patent level */
	  if (HE5_PTfexist(dtsid,linkfield,NULL,NULL,NULL,NULL) == 1 )
	    {
	      break;
	    }
	  else
	    {
	      /* ... else report error */
	      /* --------------------- */
	      status = FAIL;
	      sprintf(errbuf, mess, linkfield, parent);
	      H5Epush(__FILE__, "HE5_PTdeflinkage", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      break;

	    }


	}
    }


  /* Report parent level not found if relevant */
  /* ----------------------------------------- */
  if (foundParent == FAIL)
    {
      status = FAIL;
      sprintf(errbuf, "Parent Level: \"%s\" not found.\n", parent);
      H5Epush(__FILE__, "HE5_PTdeflinkage", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Report child level not found if relevant */
  /* ---------------------------------------- */
  if (foundChild == FAIL)
    {
      status = FAIL;
      sprintf(errbuf, "Child Level: \"%s\" not found.\n", child);
      H5Epush(__FILE__, "HE5_PTdeflinkage", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Check that parent and child levels are adjacent */
  /* ----------------------------------------------- */
  if (foundParent != FAIL && foundChild != FAIL && foundChild - foundParent != 1)
    {
      status = FAIL;
      sprintf(errbuf, "Parent/Child Levels not adjacent \"%s/%s\".\n",  parent, child);
      H5Epush(__FILE__, "HE5_PTdeflinkage", __LINE__, H5E_BTREE, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Insert linkage info in structural metadata */
  /* ------------------------------------------ */
  sprintf(utlbuf, "%s%s%s%s%s", parent, "/", child, ":", linkfield);
  status = HE5_EHinsertmeta(fid, HE5_PTXPoint[idx].ptname, "p", 12L, utlbuf, &dum);
  if(status == FAIL)
    {
      sprintf(errbuf, "Cannot insert metadata for the \"%s\" point\n", HE5_PTXPoint[idx].ptname);
      H5Epush(__FILE__, "HE5_PTdeflinkage", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwritesetup                                                 |
|                                                                             |
|  DESCRIPTION:                                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               HDFEOS point ID                         |
|  level          int                 level number (0 - based)                |
|                                                                             |
|  OUTPUTS:                                                                   |
|  nrec           hssize_t            Number of current records               |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  8/26/99  A.Muslimov    Added a new variable count to pass as a 4th argument|
|                         to PTwriteattr().                                   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  Oct 00   A.Muslimov    Modified to reflect chages in other interfaces.     |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t  
HE5_PTwritesetup(hid_t pointID,  int level, hssize_t *nrec)
{
  herr_t          status  = FAIL;/* routine return status variable   */

  int             i;			 /* loop index                       */

  hid_t           fid     = FAIL;/* HDF-EOS file ID                  */
  hid_t           gid     = FAIL;/* POINTS group ID                  */

  long            idx     = FAIL;/* Point index                      */

  unsigned int    zerobuf[HE5_DTSETRANKMAX];     

  hsize_t         count[HE5_DTSETRANKMAX];/* # of attribute elements */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* error messge buffer */

  HE5_LOCK;

  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    {
      zerobuf[ i ] = 0;
      count[ i ]   = 0;
    }
        
  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTwritesetup", &fid, &gid, &idx);
  if (status == FAIL)
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTwritesetup", __LINE__, H5E_FUNC, H5E_CANTINIT,  errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get current number of records */
  /* ----------------------------- */
  *nrec = (hssize_t)HE5_PTnrecs(pointID, level);
  if (*nrec == 0)
    {
      /* Read  "_LevelWritten" Attribute */
      /* ------------------------------- */
      status = HE5_PTreadattr(pointID,"_LevelWritten", zerobuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot read the attribute \"_LevelWritten\". \n");
	  H5Epush(__FILE__, "HE5_PTwritesetup", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      /* If level not yet written then write it */
      /* -------------------------------------- */
      if ( zerobuf[level] == 0)
        {
	  zerobuf[ level ] = 1;
	  count[ level ]   = 1;

	  /* Write "1" to "_LevelWritten" record for this level */
	  /* -------------------------------------------------- */		  
	  status = HE5_PTwriteattr(pointID, "_LevelWritten", H5T_NATIVE_UINT, count, zerobuf);
	  if (status == FAIL)
	    {
              sprintf(errbuf, "Cannot write record to the attribute \"_LevelWritten\". \n");
              H5Epush(__FILE__, "HE5_PTwritesetup", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
              HE5_EHprint(errbuf, __FILE__, __LINE__);
              return(status);
	    }
           
        }
    }
	  
 COMPLETION:
  HE5_UNLOCK;
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION:  HE5_PTwritelevel                                                |
|                                                                             |
|  DESCRIPTION: Writes full records to a level.                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  count[]        hsize_t             Number of records to write              |
|  size           size_t              Data size (bytes) to write              |
|  data           void                data buffer                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|      None                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date        Programmer    Description                                     |
|  ==========   ============  =============================================   |
|  12/13/99     A.Muslimov    Added proper error handlings after the function |
|                              calls.                                         |
|  7.18.99      A.Muslimov    Unmasked hdf5 data types.                       |
|  Oct 00       A.Muslimov    Changed the interface to make it consistent with|
|                             writing data to compound dataset.               |
|  09/13/01     A.Muslimov    Added mutex "lock"/"unlock" calls.              |
|  02/01/06     Abe Taaheri   use the same pointID for both  HE5_PTwrbckptr & |
|                             HE5_PTwrfwdptr calls                            |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTwritelevel(hid_t pointID, int level, hsize_t count[], size_t *size, void *data)
{
  herr_t         status    = FAIL;/* routine return status variable   */

  int            nlevels   = FAIL;/* Number of levels in point        */

  hid_t          fid       = FAIL;/* HDF-EOS file ID                  */
  hid_t          gid       = FAIL;/* POINTS group ID                  */
  hid_t          dataID    = FAIL;/* dataset ID                       */
  hid_t          typeID    = FAIL;/* data type ID                     */
  hid_t          xfer_list = FAIL;/* Property list  ID                */ 

  long           idx       = FAIL;/* Point index                      */

  hssize_t       nrec      = FAIL;/* Current number of records        */

  char           errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */
  hid_t          savePointID;

  size_t         datasize = 0;       /* datatype size for data in level records */
  HE5_CmpDTSinfo inStruct;
  herr_t         readstatus;
  unsigned char *newdatabuff = NULL; /* buffer for old + new records in level   */
  unsigned char *val = NULL;         /* buffer for reading old records in level */
  hsize_t        tcount[1];

  HE5_LOCK;
  CHECKPOINTER(count);
  CHECKPOINTER(size);
  CHECKPOINTER(data);

  tcount[0] = count[0];

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTwritelevel", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for file ID failed. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Get the current number of records */
  /* --------------------------------- */
  status = HE5_PTwritesetup(pointID, level, &nrec);
  if(status == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the current number of records\n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

   /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if(nlevels == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the number of levels\n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Report error if no levels (dataset) defined */
  /* ------------------------------------------- */
  if (nlevels == 0)
    {
      status = FAIL;
      sprintf(errbuf, "No Levels Defined for point ID: %d\n", pointID);
      H5Epush(__FILE__, "HE5_PTwritelevel",   __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  else if (nlevels < level)
    {
      /* Report error if level # to large */
      /* -------------------------------- */
      status = FAIL;
      sprintf(errbuf, "Only %d levels Defined for point ID: %d\n", nlevels, pointID);
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* If level already has records, append to it */
  if(nrec > 0)
    {
      /* read nrec records and append data to it */
      readstatus = HE5_PTreadlevel(pointID, level, &inStruct, &datasize, NULL);
      if (readstatus == FAIL )
	{
	  sprintf(errbuf, "Checking for datasize of records in level failed. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(readstatus);
	}

      readstatus = HE5_PTlevelinfo(pointID, level, &inStruct);
      if (readstatus == FAIL )
	{
	  sprintf(errbuf, "Checking for info on level failed. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(readstatus);
	}

      val = calloc(nrec, datasize);
      if(val == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory for the buffer for reading existing records.\n");
	  H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      readstatus = HE5_PTreadlevel(pointID, level, &inStruct, &datasize, (void *)val);
      if (readstatus == FAIL )
	{
	  sprintf(errbuf, "Reading existing records in level failed. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(readstatus);
	}

      /* allocate memory for the buffer to hold all existing and new records */
      newdatabuff = calloc((nrec+tcount[0]), datasize);

      /* copy existing records to newdatabuff */
      memcpy(newdatabuff, val, nrec * datasize);
      if( val != NULL)
	{
	  free(val);
	  val = NULL;
	}
      /* add new records to newdatabuff */
      memcpy(newdatabuff+(nrec * datasize), data, tcount[0]*datasize);
      tcount[0] = tcount[0]+nrec;
    }
  /* Get the level dataset ID  */
  /* ------------------------- */
  dataID = HE5_PTXPoint[idx].level[level].ID;

  /* Extend the dataset size */
  /* ----------------------- */
  status = H5Dextend(dataID, tcount);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot extend the  dataset. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get the datatype ID */
  /* ------------------- */
  H5Eset_auto(NULL, NULL);
  typeID = H5Dget_type(dataID);
  if (typeID == FAIL)
    {
      sprintf(errbuf, "Cannot get data type ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Create the property list */
  /* ------------------------ */
  xfer_list = H5Pcreate(H5P_DATASET_XFER);
  if (xfer_list == FAIL)
    {
      sprintf(errbuf, "Cannot create the property list ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_PLIST, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Set type conversion and background buffer */
  /* ----------------------------------------- */
  status = H5Pset_buffer(xfer_list, *size, NULL, NULL);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot set type conversion and background buffer. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Write data to the dataset */
  /* ------------------------- */
  if(nrec > 0)
    {
      status = H5Dwrite(dataID, typeID, H5S_ALL, H5S_ALL, xfer_list, (void *)newdatabuff);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write data to the dataset. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      
      if(newdatabuff != NULL)
	{
	  free(newdatabuff);
	  newdatabuff = NULL;
	}
    }
  else
    {
      status = H5Dwrite(dataID, typeID, H5S_ALL, H5S_ALL, xfer_list, (void *)data);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write data to the dataset. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }

  /* Release data type ID */
  /* -------------------- */
  status = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the data type ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Release property list ID */
  /* ------------------------ */
  status = H5Pclose(xfer_list);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the property list ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  if (level > 0)
    { 
      savePointID = pointID;

      /* Write BackPointers */
      /* ------------------ */
      status = HE5_PTwrbckptr(pointID, level);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write back pointer records. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      /* Write FwdPointers */
      /* ----------------- */
      status = HE5_PTwrfwdptr(savePointID, level - 1); 
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write forward pointer records. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwriteattr                                                  |
|                                                                             |
|  DESCRIPTION:                                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  attrname       char                attribute name                          |
|  numtype        hid_t               attribute HDF numbertype                |
|  count          hsize_t             Number of attribute elements            |
|  wrcode         char                Read/Write Code "w/r"                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf                                                                     |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTwriteattr(hid_t pointID, const char *attrname, hid_t numtype, hsize_t count[],void * datbuf)
{
  herr_t     status     = FAIL;/* routine return status variable */

  hid_t      fid        = FAIL;/* HDF-EOS file ID                */
  hid_t      gid        = FAIL;/* HDFEOS group ID                */

  long       idx        = FAIL;/* Point index                    */
  hid_t      ntype;

  HE5_LOCK;
  CHECKNAME(attrname);
  CHECKPOINTER(count);
  CHECKPOINTER(datbuf);

  /* 
     if numtype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use numtype itself
  */
  ntype = HE5_EHconvdatatype(numtype);
  if(ntype == FAIL)
    {
      ntype = numtype;
    }

  /* Check Point ID */
  /* -------------- */
  status = HE5_PTchkptid(pointID, "HE5_PTwriteattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Perform Attribute I/O */
      /* --------------------- */
      status = HE5_EHattr(HE5_PTXPoint[idx].pt_id, attrname, ntype, count,"w", datbuf);
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadattr                                                   |
|                                                                             |
|  DESCRIPTION: Reads attribute from a point.                                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTreadattr(hid_t pointID, const char *attrname, void *datbuf)
{
  herr_t      status  = FAIL;/* routine return status variable */

  hid_t       fid     = FAIL;/* HDF-EOS file ID                */
  hid_t       tid     = FAIL;/* Data type ID                   */
  hid_t       gid     = FAIL;/* POINTS group ID                */

  hsize_t     count[] = {0}; /* Number of elements             */

  long        idx     = FAIL;/* Point index                    */

  HE5_LOCK;
  CHECKPOINTER(attrname);

  /* Check Point ID */
  /* -------------- */
  status = HE5_PTchkptid(pointID, "HE5_PTreadattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get attribute group ID and call EHattr to perform I/O  */
      /* ------------------------------------------------------ */
      status = HE5_EHattr(HE5_PTXPoint[idx].pt_id, attrname, tid, count, "r", datbuf);
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTlevelinfo                                                  |
|                                                                             |
|  DESCRIPTION: Returns information about a given level.                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  info           HE5_CmpDTSinfo      Structure containing information about  |
|                                      specified level.                       |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  Oct  00  A.Muslimov    Changed the interface.                              |
|  Aug  04  S.Zhao        Added the number type for a field in a point level. |
|  Feb  06  Abe Taaheri   Added support for STRING fields and size of         ||                          structure                                          |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_PTlevelinfo(hid_t pointID, int level, HE5_CmpDTSinfo *info)
{
  herr_t          status     = SUCCEED;	/* routine return status variable */

  int             nlevels    =  0;	/* Number of levels in point      */
  int             i, j;			/* Loop indices                   */

  hid_t           fid        = FAIL;    /* HDF-EOS file ID                */
  hid_t           levelID    = FAIL;    /* data ID                        */
  hid_t      	  gid        = FAIL;    /* "POINTS" group ID              */
  hid_t           typeID     = FAIL;    /* Data type ID                   */
  hid_t           btypeID    = FAIL;    /* Base Data type ID              */
  hid_t           mdt[HE5_FLDNUMBERMAX]       = {FAIL};    /* Member data type ID            */

  hsize_t         dims[HE5_DTSETRANKMAX];/* Array of dimension sizes      */

  long            idx        = FAIL;	/* Point index                    */

  size_t          *size      = (size_t *)NULL;/* data size (bytes)        */
  size_t          *order     = (size_t *)NULL;/* data byte order          */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTlevelinfo", &fid, &gid, &idx);
  if (status == FAIL)
    {
      sprintf(errbuf, "Checking for Point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Initialize the members of a structure */
  /* ------------------------------------- */
  info->nfields  = FAIL;
  info->datasize = 0;
  for (i = 0; i < HE5_FLDNUMBERMAX; i++)
    {
      info->rank[i]    = FAIL;
      info->array[i]   = FALSE;
      info->offset[i]  = FAIL;
      info->dtype[i]   = FAIL;
      info->numtype[i] = FAIL;
      info->dclass[i]  = H5T_NO_CLASS;
      for (j = 0; j < HE5_DTSETRANKMAX; j++)
	info->dims[i][j] = 0;

    }

  /* Initialize dims[] array */
  /* ----------------------- */
  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    dims[ i ] = 0;
  

  /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if (nlevels == 0)
    {
      sprintf(errbuf, "No Levels Defined for point ID: %d\n", pointID);
      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  else if (nlevels < level)
    {
      /* Report error if level # to large */
      /* -------------------------------- */
      sprintf(errbuf, "Only %d levels Defined for point ID: %d\n", nlevels, pointID);
      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Get data ID of point level */
  /* -------------------------- */
  levelID = HE5_PTXPoint[idx].level[level].ID;

  /* Get compound datatype id */
  /* ------------------------ */
  typeID = H5Dget_type(levelID);
  if (typeID == FAIL)
    {
      sprintf(errbuf, "Cannot get the datatype ID for a level.\n");
      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Get number of fields */
  /* -------------------- */
  info->nfields = H5Tget_nmembers(typeID);
  if (info->nfields == FAIL)
    {
      sprintf(errbuf, "Cannot get the number of fields for a level.\n");
      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate memory for "size" variable */
  /* ----------------------------------- */
  size = (size_t *)calloc(info->nfields, sizeof(size_t));
  if (size == (size_t *)NULL)
    {
      sprintf(errbuf, "Cannot allocate memory for the size variable.\n");
      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate memory for "order" variable */
  /* ----------------------------------- */
  order = (size_t *)calloc(info->nfields, sizeof(size_t));
  if (order == (size_t *)NULL)
    {
      sprintf(errbuf, "Cannot allocate memory for the order variable.\n");
      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(size);
      return(FAIL);
    }

  /* Loop through all data members */
  /* ----------------------------- */
  for( i = 0; i < info->nfields; i++)
    {	  
      size[i] = 0;

      /* Get member name */
      /* --------------- */
      info->fieldname[i] = H5Tget_member_name(typeID, i);
      if (info->fieldname[i] == NULL)
	{
	  sprintf(errbuf, "Cannot get the field name.\n");
	  H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (size != NULL) free(size);
	  if (order != NULL) free(order);
	  return(FAIL);
	}
	  
      /* Get member type */
      /*---------------- */
      mdt[i] = H5Tget_member_type(typeID, i);
      if (mdt[i] == FAIL)
	{
	  sprintf(errbuf, "Cannot get the field type.\n");
	  H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (size != NULL) free(size);
	  if (order != NULL) free(order);
	  return(FAIL);
	}
	  
      /* Get field data class */
      /* -------------------- */
      info->dclass[i] = H5Tget_class(mdt[i]);
      if (info->dclass[i] == H5T_NO_CLASS)
	{
	  sprintf(errbuf, "Cannot get the field class.\n");
	  H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (size != NULL) free(size);
	  if (order != NULL) free(order);
	  return(FAIL);
	}	  
	  	  
      /* Get the rank and "array" flag */
      /* ----------------------------- */
      if (info->dclass[i] == H5T_ARRAY)
	{
	  /* Get the field rank */
	  /* ------------------ */
	  info->rank[i]  = H5Tget_array_ndims(mdt[i]);
	  if (info->rank[i] == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the field rank.\n");
	      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (size != NULL) free(size);
	      if (order != NULL) free(order);
	      return(FAIL);
	    }	  

	  /* The field is an array */
	  /* --------------------- */
	  info->array[i] = TRUE;

	  /* Get the field dimension sizes */
	  /* ----------------------------- */
	  status = H5Tget_array_dims(mdt[i], dims, NULL);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the field dimension sizes.\n");
	      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (size != NULL) free(size);
	      if (order != NULL) free(order);
	      return(FAIL);
	    }	  

	  /* Set the field dimension sizes */
	  /* ----------------------------- */
	  for (j = 0; j < info->rank[i]; j++)
	    info->dims[i][j] = dims[j];

	  /* Get the field base data type ID */
	  /* ------------------------------- */
	  btypeID = H5Tget_super(mdt[i]);
	  if (btypeID == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the field base data type ID.\n");
	      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (size != NULL) free(size);
	      if (order != NULL) free(order);
	      return(FAIL);
	    }	  


	  /* Set the field data type */
	  /* ----------------------- */
	  info->dtype[i] = btypeID;

	  /* Set the field number type */
	  /* ------------------------- */
	  info->numtype[i] = HE5_EHdtype2numtype(info->dtype[i]);

	  if (info->numtype[i] == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the number type ID.\n");
	      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (size != NULL) free(size);
	      if (order != NULL) free(order);
	      return(FAIL);
	    }

	  /* Get the field data size */
	  /* ----------------------- */
	  size[i] = H5Tget_size(btypeID);
	  if (size[i] == 0)
	    {
	      sprintf(errbuf, "Cannot get the field data size.\n");
	      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (size != NULL) free(size);
	      if (order != NULL) free(order);
	      return(FAIL);
	    }	  

	  /* Release the data type ID */
	  /* ------------------------ */
	  status = H5Tclose(btypeID);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the field data type ID.\n");
	      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (size != NULL) free(size);
	      if (order != NULL) free(order);
	      return(FAIL);
	    }	  
	}
      else
	{
	  /* Set the scalar field parameters */
	  /* ------------------------------- */
	  info->rank[i]    = 1;
	  info->array[i]   = FALSE;
	  info->dims[i][0] = 1;
	  info->dtype[i]   = mdt[i];

	  if (info->dclass[i] == H5T_STRING)
	    {
	      info->numtype[i] = HE5T_CHARSTRING;
	    }
	  else
	    {
	      info->numtype[i] = HE5_EHdtype2numtype(info->dtype[i]);
	    }

	  if (info->numtype[i] == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the number type ID.\n");
	      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (size != NULL) free(size);
	      if (order != NULL) free(order);
	      return(FAIL);
	    }

	  /* Get the scalar field data size */
	  /* ------------------------------ */
	  size[i] = H5Tget_size(mdt[i]);
	  if (size[i] == 0)
	    {
	      sprintf(errbuf, "Cannot release the field data size.\n");
	      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (size != NULL) free(size);
	      if (order != NULL) free(order);
	      return(FAIL);
	    }
	}

      /* Calculate the field order */
      /* ------------------------- */
      order[i] = 1;
      for (j = 0; j < info->rank[i]; j++)
	order[i] *= info->dims[i][j];

      /* Calculate the data size in memory */
      /* --------------------------------- */
      /* Note: info->datasize += size[i]*order[i]; calculates 
	 structure size exactly, but it causes problem with 
	 PTreadlevel, where padded size of structure may be 
	 needed. For example the following structure for
	 Sensor level

	 typedef struct {
	 char     name[32];
	 float    lat;
	 float    lon;
	 float    alt;
	 unsigned short int      id;
	 } Sensor;
	     
	 has exact size of 46 bytes, but padded size is 48, 
	 which is needed for 
	 " datasize = (hsize_t)(dtsinfo.datasize);" 
	 in the PTreadlevel routine. So instead of 
	 info->datasize += size[i]*order[i] we will 
	 use H5Tget_size(typeID) which gets padded 
	 size of level structue */

      /*info->datasize += size[i]*order[i];*/
      info->datasize = H5Tget_size(typeID);
    }

  /* Calculate field unalligned offsets */
  /* ---------------------------------- */
  info->offset[0] = 0;
  for (i = 1; i < info->nfields; i++)
    info->offset[i] = info->offset[i-1]+(off_t)(size[i-1]*order[i-1]);

  if (size != NULL) free(size);
  if (order != NULL) free(order);


  /* Release the field data type ID */
  /* ------------------------------ */
  for( i = 0; i < info->nfields; i++)
    {
      status = H5Tclose(mdt[i]);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the field data type ID.\n");
	  H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (size != NULL) free(size);
	  if (order != NULL) free(order);
	  return(FAIL);
	}
    }

  /* Release the datatype ID */
  /* ----------------------- */
  status = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the datatype ID.\n");
      H5Epush(__FILE__, "HE5_PTlevelinfo", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }	  

  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadlevel                                                  |
|                                                                             |
|  DESCRIPTION: Reads data from the specified fields and records of a level.  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  inStruct       HE5_CmpDTSinfo      Input data structure containing infor-  |
|                                      mation about compound dataset to read. |
|  size           size_t              Memory size (bytes) of data structure to|
|                                      read data into.                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf                             Data buffer                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Apr 00   A.Muslimov    Changed type of 'slen' from long to size_t.         |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  Oct 00   A.Muslimov    Changed the interface.                              |
|  Mar 00   A.Muslimov    Modified to reflect changes in HDF5 library:        |
|                         H5Tinsert_array should be replaced by two calls     |
|                         H5Tarray_create() and H5Tinsert().                  |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|  Feb 06   Abe Taaheri   Modified for getting datasize only,                 |
|                         Correction for setting datasize in xfer_list,       |
|                         Support for STRING fields,                          |
|                         and using correct datatypes                         |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_PTreadlevel(hid_t pointID, int level, HE5_CmpDTSinfo *inStruct, size_t *size, void *datbuf)
{
  herr_t          status    = FAIL;/* routine return status variable  */

  int             nlevels   =  0;  /* Number of levels in point       */
  int             i, j;            /* Loop indices                    */

  hid_t           fid       = FAIL;/* HDF-EOS file ID                 */
  hid_t           gid       = FAIL;/* POINTS group ID                 */
  hid_t           dataID    = FAIL;/* dataset ID                      */
  hid_t           xfer_list = FAIL;/* Property list ID                */
  hid_t           typeID    = FAIL;/* compound datatype ID            */
  hid_t           arr_id    = FAIL;/* Array datatype ID               */

  hsize_t         dims[HE5_DTSETRANKMAX];/* Array of dimension sizes  */

  hsize_t         datasize  = 0;/* Size (bytes) of a dataset to read  */

  long            idx       = FAIL;/* Point index                     */

  HE5_CmpDTSinfo dtsinfo;/* Structure to read information about level */    

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */
  hid_t           cdatatype = FAIL;           /* return data type ID  */
  int             dtypesize = FAIL;
  hid_t           mytypeID  = FAIL;           /* compound datatype ID */
  hid_t           mdt       = FAIL;           /* Member data type ID  */
  int             jjj       = 0;
  int             jj        = 0;
  int             result    = 99;


  HE5_LOCK;
  CHECKPOINTER(inStruct);
  CHECKPOINTER(size);

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTreadlevel", &fid, &gid, &idx);
  if (status == FAIL)
    {
      sprintf(errbuf, "Checking for the point  ID failed.\n");
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Initialize dims[] array */
  /* ----------------------- */
  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    dims[ i ] = 0;

  /* Get number of levels in point */
  /* ----------------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if (nlevels == 0)
    {
      status = FAIL;
      sprintf(errbuf, "No Levels Defined for point ID: %d\n", pointID);
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  else if (nlevels < level)
    {
      /* Report error if level # to large */
      /* -------------------------------- */
      status = FAIL;
      sprintf(errbuf, "Only %d levels Defined for point ID: %d\n", nlevels, pointID);
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_FILE, H5E_SEEKERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get level dataset ID */
  /* ------------------ */
  dataID = HE5_PTXPoint[idx].level[level].ID;

  /* Create property list */
  /* -------------------- */
  xfer_list = H5Pcreate(H5P_DATASET_XFER);
  if (xfer_list == FAIL)
    {
      sprintf(errbuf, "Cannot create the property list. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Call HE5_PTlevelinfo() */
  /* ---------------------- */
  status = HE5_PTlevelinfo(pointID, level, &dtsinfo);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot get the level information. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Get the dataset size */
  /* -------------------- */
  datasize = (hsize_t)(dtsinfo.datasize);

  /* If user only wants to get the size of datbuf
     so that he/she can allocate enough memory for 
     datbuf, user will pass datbuf as NULL. In this 
     case return here with the required size. */ 
  if(datbuf == NULL)
    {
      *size = datasize;
      return(SUCCEED);
    }

  /* Set the buffer */
  /* -------------- */
  status = H5Pset_buffer(xfer_list, dtsinfo.datasize, NULL, NULL);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot set the read buffer. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Create Compound datatype */
  /* ------------------------ */
  typeID = H5Tcreate(H5T_COMPOUND, *size);
  if (typeID == FAIL)
    {
      status = FAIL;
      sprintf(errbuf, "Cannot create the compound data type.\n");
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

   
  /* Add members of a compound datatype to read */
  /* ------------------------------------------ */
  for ( i = 0; i < inStruct->nfields; i++)
    {
      for( jj = 0; jj <dtsinfo.nfields ; jj++ )
	{
	  result  = strcmp(inStruct->fieldname[i],dtsinfo.fieldname[jj]);
	  if( result == 0 ) 
	    {
	      jjj = jj;
	      result = 99;
	    }    
	} 
      jj = jjj;
      inStruct->numtype[i]=dtsinfo.numtype[jj];
      /* Check if the member is an array */
      /* ------------------------------- */
      if ( inStruct->array[i] == TRUE)
	{
		  
	  for (j = 0; j < inStruct->rank[i]; j++)
	    dims[j] = (hsize_t)(inStruct->dims[i][j]);

	  /* Create array datatype */
	  /* --------------------- */
	  /*arr_id = H5Tarray_create(inStruct->dtype[i], inStruct->rank[i], (const hsize_t *)dims, NULL);*/
	  cdatatype = HE5_EHconvdatatype(inStruct->numtype[i]);
	  /* 
	    if numtype is HE5 numbertype, rather than H5 numbertype, 
	    then convert it, otherwise use numtype itself
	  */
	  if(cdatatype == FAIL)
	    {
	      cdatatype = inStruct->numtype[i];
	    }

	  arr_id = H5Tarray_create(cdatatype, inStruct->rank[i], (const hsize_t *)dims, NULL);
		  
	  if (arr_id == FAIL)
	    {
	      sprintf(errbuf, "Cannot create \"array\" datatype. \n");
	      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Add array field */
	  /* --------------- */
	  status = H5Tinsert(typeID, inStruct->fieldname[i], inStruct->offset[i], arr_id);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot insert \"array\" element. \n");
	      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Release the data type ID */
	  /* ------------------------ */
	  status = H5Tclose(arr_id);
	  if (status == FAIL )
	    {
	      sprintf(errbuf, "Cannot release data type ID. \n");
	      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
	}
      else 
	{
	  /* Add atomic field */
	  /* ---------------- */

	  if(inStruct->dclass[i] == H5T_STRING)
	    {
	      /* Get compound datatype id */
	      /* ------------------------ */
	      mytypeID = H5Dget_type(dataID);
	      mdt = H5Tget_member_type(mytypeID, i);
	      dtypesize = H5Tget_size(mdt);
		      
	      status = H5Tclose(mdt);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot release the field data type ID.\n");
		  H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
		      
	      arr_id = H5Tcopy(H5T_C_S1);
	      H5Tset_size(arr_id,dtypesize);
	      H5Tset_strpad(arr_id, H5T_STR_NULLPAD);
	      status = H5Tinsert(typeID, inStruct->fieldname[i], 
				 inStruct->offset[i],arr_id);
		    
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot insert atomic data element. \n");
		  H5Epush(__FILE__, "HE5_PTreadlevel", 
			  __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}

	      status = H5Tclose(arr_id);
	      if (status == FAIL )
		{
		  sprintf(errbuf, "Cannot release data type ID. \n");
		  H5Epush(__FILE__, "HE5_PTreadlevel", 
			  __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
	    }
	  else
	    {
	      cdatatype = HE5_EHconvdatatype(inStruct->numtype[i]);
	      /* 
		 if numtype is HE5 numbertype, rather than H5 numbertype, 
		 then convert it, otherwise use numtype itself
	      */
	      if(cdatatype == FAIL)
		{
		  cdatatype = inStruct->numtype[i];
		}
	      if(inStruct->nfields == 1)
		{
		  status = H5Tinsert(typeID, inStruct->fieldname[i], 
				     0,cdatatype );
		}
	      else
		{
		  status = H5Tinsert(typeID, inStruct->fieldname[i],
				     inStruct->offset[i],cdatatype );
		}
		      
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot insert atomic data element.\n");
		  H5Epush(__FILE__, "HE5_PTreadlevel", 
			  __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	    }
	}
    }

  /* Release pointers to the name buffers */
  /* ------------------------------------ */
  for (i = 0; i < dtsinfo.nfields; i++)
    {
      if (dtsinfo.fieldname[i] != NULL) 
	free(dtsinfo.fieldname[i]);
    }

  /* Read out the level fields */
  /* ------------------------- */
  status = H5Dread(dataID, typeID, H5S_ALL, H5S_ALL, xfer_list, datbuf);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot read the level dataset. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  

  /* Release the property list ID */
  /* ---------------------------- */
  status = H5Pclose(xfer_list);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the property list ID. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Release the datatype ID */
  /* ----------------------- */
  status = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the data type ID.\n");
      H5Epush(__FILE__, "HE5_PTreadlevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqdatatype                                                |
|                                                                             |
|  DESCRIPTION: Inquires about datasets in Point                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t               return status (0) SUCCEED, (-1) FAIL   |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID         hid_t                                                      |
|  fieldname       char *                                                     |
|  attrname        char *                                                     |
|  fieldgroup      int                                                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  dtype           hid_t                                                      |
|  classID         H5T_class_t                                                |
|  Order           H5T_order_t                                                |
|  size            size_t                                                     |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Oct 99   D.Wynne       Original Programmer                                 |
|  Dec 99   A.Muslimov    Modified for Point interface.                       |
|  May 00   A.Muslimov    Changed 'H5T_class_t' type to 'int' data type.      |
|  Jun 00   A.Muslimov    Updated to include Group/Local Attributes options.  |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  Oct  00  A.Muslimov    Added missing H5Tclose() calls and error handlings. |    
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_PTinqdatatype(hid_t pointID, const char *fieldname, const char *attrname, int fieldgroup, hid_t *dtype, H5T_class_t *classID, H5T_order_t *Order, size_t *size)
{
 
  herr_t      status    = FAIL;       /* return status variable */

  hid_t       datasetid = FAIL;       /* dataset ID             */
  hid_t       fid       = FAIL;       /* HDF-EOS file ID        */
  hid_t       gid       = FAIL;       /* "POINTS" group ID      */
  hid_t       attr      = FAIL;       /* attribute ID           */
  hid_t       fldgroup  = FAIL;       /* "field group" flag     */

  long        idx       = FAIL;       /* Point index            */

  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */


  /* Check for valid point id */
  /* ======================== */
  status = HE5_PTchkptid(pointID, "HE5_PTinqdatatype", &fid, &gid, &idx);
  if(status == SUCCEED)
    {
      if(fieldgroup == HE5_HDFE_DATAGROUP)
	fldgroup = HE5_PTXPoint[idx].data_id;
      else if(fieldgroup == HE5_HDFE_ATTRGROUP)
	fldgroup = HE5_PTXPoint[idx].pt_id;
      else if(fieldgroup == HE5_HDFE_GRPATTRGROUP)
	fldgroup = HE5_PTXPoint[idx].data_id;
      else if(fieldgroup == HE5_HDFE_LOCATTRGROUP)
	fldgroup = HE5_PTXPoint[idx].data_id;
      else
	{
	  sprintf(errbuf, "Group \"%d\" unknown.\n", fieldgroup);
	  H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_FUNC, H5E_BADVALUE , errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Call HDF 5 routines */
      /* =================== */
      if(fieldgroup == HE5_HDFE_DATAGROUP)
	{
	  datasetid = H5Dopen(fldgroup, fieldname);
	  if(datasetid == FAIL)
	    {
	      sprintf(errbuf, "Cannot open the dataset \"%s\".\n", fieldname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	  *dtype    = H5Dget_type(datasetid);
	  if(*dtype == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the datatype ID for \"%s\".\n", fieldname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  *classID  = H5Tget_class(*dtype);
	  if(*classID == H5T_NO_CLASS)
	    {
	      sprintf(errbuf, "Cannot get the data type class ID for \"%s\".\n", fieldname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	  if (*classID != H5T_COMPOUND)
	    {
	      *Order    = H5Tget_order(*dtype);
	      if(*Order == H5T_ORDER_ERROR)
		{
		  sprintf(errbuf, "Cannot get the datatype byte order for \"%s\".\n", fieldname);
		  H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	    }
		  
	  if (*classID == H5T_STRING)
	    {
	      *size = 1;
	    }
	  else
	    {
	      *size     = H5Tget_size(*dtype);
	      if(*size == 0)
		{
		  sprintf(errbuf, "Cannot get the datatype size for \"%s\".\n", fieldname);
		  H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	    }

	  status    = H5Tclose(*dtype);
	  if(status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the datatype ID.\n");
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  status    = H5Dclose(datasetid);
	  if(status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataset ID.\n");
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	}
      else if(fieldgroup == HE5_HDFE_ATTRGROUP || fieldgroup == HE5_HDFE_GRPATTRGROUP)
	{
	  attr      = H5Aopen_name(fldgroup, attrname);
	  if(attr == FAIL)
	    {
	      sprintf(errbuf, "Cannot open the attribute \"%s\".\n", attrname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  *dtype    = H5Aget_type(attr);
	  if(*dtype == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the datatype ID for \"%s\".\n", attrname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	  *classID  = H5Tget_class(*dtype);
	  if(*classID == H5T_NO_CLASS)
	    {
	      sprintf(errbuf, "Cannot get the data type class ID for \"%s\".\n", attrname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	  if (*classID != H5T_COMPOUND)
	    {
	      *Order    = H5Tget_order(*dtype);
	      if(*Order == H5T_ORDER_ERROR)
		{
		  sprintf(errbuf, "Cannot get the datatype byte order for \"%s\".\n", attrname);
		  H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	    }

	  if (*classID == H5T_STRING)
	    {
	      *size = 1;
	    }
	  else
	    { 
	      *size     = H5Tget_size(*dtype);
	      if(*size == 0)
		{
		  sprintf(errbuf, "Cannot get the datatype size for \"%s\".\n", attrname);
		  H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	    }

	  status    = H5Tclose(*dtype);
	  if(status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the datatype ID.\n");
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  status    = H5Aclose(attr);
	  if(status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the attribute dataset ID.\n");
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	}
      else if(fieldgroup == HE5_HDFE_LOCATTRGROUP)
	{
	  datasetid = H5Dopen(fldgroup, fieldname);
	  if(datasetid == FAIL)
	    {
	      sprintf(errbuf, "Cannot open the dataset \"%s\".\n", fieldname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	  attr      = H5Aopen_name(datasetid, attrname);
	  if(attr == FAIL)
	    {
	      sprintf(errbuf, "Cannot open the attribute \"%s\".\n", attrname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  *dtype    = H5Aget_type(attr);
	  if(*dtype == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the datatype ID for \"%s\".\n", attrname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  *classID  = H5Tget_class(*dtype);
	  if(*classID == H5T_NO_CLASS)
	    {
	      sprintf(errbuf, "Cannot get the data type class ID for \"%s\".\n", attrname);
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  if (*classID != H5T_COMPOUND)
	    {
	      *Order    = H5Tget_order(*dtype);
	      if(*Order == H5T_ORDER_ERROR)
		{
		  sprintf(errbuf, "Cannot get the datatype byte order for \"%s\".\n", attrname);
		  H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	    }
		  
	  if (*classID == H5T_STRING)
	    {
	      *size = 1;
	    }
	  else
	    {
	      *size     = H5Tget_size(*dtype);
	      if(*size == 0)
		{
		  sprintf(errbuf, "Cannot get the datatype size for \"%s\".\n", attrname);
		  H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	    }

	  status    = H5Tclose(*dtype);
	  if(status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the datatype ID.\n");
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  status    = H5Aclose(attr);
	  if(status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the attribute dataset ID.\n");
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  status    = H5Dclose(datasetid);
	  if(status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataset ID.\n");
	      H5Epush(__FILE__, "HE5_PTinqdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}
    }

  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqpoint                                                   |
|                                                                             |
|  DESCRIPTION: Returns number and names of point structures in file          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nPoint         int                 Number of point structures in file      |
|                                                                             |
|  INPUTS:                                                                    |
|  filename       char                HDF-EOS filename                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  pointlist      char                List of point names (comma-separated)   |
|  strbufsize     long                Length of pointlist                     |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTinqpoint(const char *filename, char *pointlist, long *strbufsize)
{
  int           nPoint = FAIL;/* Number of point structures in file */

  herr_t        status = FAIL;/* status variable                    */

  CHECKPOINTER(filename);

  /* Call EHinquire */
  /* -------------- */
  nPoint = (int)HE5_EHinquire(filename, "/HDFEOS/POINTS", pointlist, strbufsize);

 COMPLETION:  
  return (nPoint);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTattrinfo                                                   |
|                                                                             |
|  DESCRIPTION:                                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          hid_t               attribute data type ID                  |
|  count          hsize_t             Number of attribute elements            |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer    Description                                        |
|  =======   ============  =================================================  |
|  5/11/00   A.Muslimov    Changed 'H5T_class_t' type to 'int' data type.     |
|  7.18.99   A.Muslimov    Unmasked hdf5 data types.                          |
|  Feb 03    S.Zhao        Changed the type of 'ntype' from an H5T_class_t to |
|                          an hid_t.                                          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTattrinfo(hid_t pointID, const char *attrname, hid_t *ntype, hsize_t *count)
{
  herr_t        status     = FAIL;/* routine return status variable */

  hid_t         fid        = FAIL;/* HDF-EOS file ID                */
  hid_t         gid        = FAIL;/* POINTS group ID                */

  long          idx        = FAIL;/* Point index                    */

  CHECKPOINTER(attrname);

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTattrinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get attribute group ID and call EHattrinfo */
      status = HE5_EHattrinfo( HE5_PTXPoint[idx].pt_id, attrname, ntype, count);
    }

 COMPLETION:
  return (status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqattrs                                                   |
|                                                                             |
|  DESCRIPTION:                                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in point struct    |
|                                                                             |
|  INPUTS:                                                                    |
|  point ID       hid_t               point structure ID                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in point struct         |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_PTinqattrs(hid_t pointID, char *attrnames, long *strbufsize)
{
  long            nattr    = FAIL;/* Number of attributes (return)  */
  long            idx      = FAIL;/* Point index                    */

  herr_t          status   = FAIL;/* routine return status variable */

  hid_t           gid      = FAIL;/* "POINTS" group ID              */
  hid_t           fid      = FAIL;/* HDF-EOS file ID                */

  char            *grpname = NULL;/* Group name string buffer       */

  status = HE5_PTchkptid(pointID, "HE5_PTinqattrs", &fid, &gid, &idx);
  if(status == SUCCEED )
    {
      grpname = (char *) calloc( (strlen(HE5_PTXPoint[idx].ptname)+40), sizeof(char));
      if( grpname == NULL)
        {
          H5Epush(__FILE__, "HE5_PTinqattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
          HE5_EHprint("Error: Can not allocate memory, occured", __FILE__, __LINE__);
          return(FAIL);
        }

      strcpy(grpname,"/HDFEOS/POINTS/");
      strcat(grpname, HE5_PTXPoint[idx].ptname);

      /* search group with grpname for the number of attrs */
      nattr = HE5_EHattrcat(fid,  grpname, attrnames, strbufsize);

      free(grpname);
    }

  return (nattr);
   
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwritegrpattr                                               |
|                                                                             |
|  DESCRIPTION:   Writes/updates attribute associated with the "Data"  group  |
|                     in a point.                                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  attrname       char                attribute name                          |
|  numtype        hid_t               attribute data type ID                  |
|  count          hsize_t             Number of attribute elements            |
|  wrcode         char                Read/Write Code "w/r"                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf                                                                     |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 00   A.Muslimov                                                        |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTwritegrpattr(hid_t pointID, const char *attrname, hid_t numtype, hsize_t count[],void *datbuf)
{
  herr_t     status     = FAIL;/* routine return status variable */

  hid_t      fid        = FAIL;/* HDF-EOS file ID                */
  hid_t      gid        = FAIL;/* HDFEOS group ID                */
  hid_t      DataGrpID  = FAIL;/* "Data" group ID                */

  long       idx        = FAIL;/* Point index                    */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */
  hid_t      ntype;

  HE5_LOCK;
  CHECKNAME(attrname);
  CHECKPOINTER(count);
  CHECKPOINTER(datbuf);

  /* 
     if numtype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use numtype itself
  */
  ntype = HE5_EHconvdatatype(numtype);
  if(ntype == FAIL)
    {
      ntype = numtype;
    }

  /* Check Point ID */
  /* -------------- */
  status = HE5_PTchkptid(pointID, "HE5_PTwritegrpattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      DataGrpID = H5Gopen(HE5_PTXPoint[idx].pt_id, "Data");
      if(DataGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTwritegrpattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
        }

      /* Perform Attribute I/O */
      /* --------------------- */
      status = HE5_EHattr(DataGrpID, attrname, ntype, count,"w", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write Attribute \"%s\"to the \"Data\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_PTwritegrpattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }

      status = H5Gclose(DataGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTwritegrpattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }	

    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}





/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadgrpattr                                                |
|                                                                             |
|  DESCRIPTION: Reads attribute associated with the "Data" group from a point.|
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 00   A.Muslimov                                                        |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_PTreadgrpattr(hid_t pointID, const char *attrname, void *datbuf)
{
  herr_t      status    = FAIL;/* routine return status variable */

  hid_t       fid       = FAIL;/* HDF-EOS file ID                */
  hid_t       ntype     = FAIL;/* Data type ID                   */
  hid_t       gid       = FAIL;/* POINTS group ID                */
  hid_t       DataGrpID = FAIL;/* "Data" group ID                */

  hsize_t     count[]   = {0}; /* Number of attribute elements   */

  long        idx       = FAIL;/* Point index                    */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */

  HE5_LOCK;
  CHECKPOINTER(attrname);

  /* Check Point ID */
  /* -------------- */
  status = HE5_PTchkptid(pointID, "HE5_PTreadgrpattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get "Data"  group ID */
      /* -------------------- */
      DataGrpID = H5Gopen(HE5_PTXPoint[idx].pt_id, "Data");
      if(DataGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTreadgrpattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
        }

      /* Call EHattr to perform I/O  */
      /* --------------------------- */
      status = HE5_EHattr(DataGrpID, attrname, ntype, count, "r", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot read Attribute \"%s\" from the \"Data\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_PTreadgrpattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }

      status = H5Gclose(DataGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTreadgrpattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTgrpattrinfo                                                |
|                                                                             |
|  DESCRIPTION:  Retrieves information about attributes in "Data" group.      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          hid_t               attribute data type ID                  |
|  count          hsize_t             Number of attribute elements            |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 00   A.Muslimov                                                        |
|  May 00   A.Muslimov    Changed 'H5T_class_t' type to 'int' data type.      |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  Feb 03   S.Zhao        Changed the type of 'ntype' from an H5T_class_t to  |
|                         an hid_t.                                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTgrpattrinfo(hid_t pointID, const char *attrname, hid_t *ntype, hsize_t *count)
{
  herr_t     status     = FAIL;/* routine return status variable */

  hid_t      fid        = FAIL;/* HDF-EOS file ID                */
  hid_t      gid        = FAIL;/* HDFEOS group ID                */
  hid_t      DataGrpID  = FAIL;/* "Data" group ID                */

  long       idx        = FAIL;/* Point index                    */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */

  CHECKPOINTER(attrname);

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTgrpattrinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get "Data"  group ID  */
      /* --------------------- */
      DataGrpID = H5Gopen(HE5_PTXPoint[idx].pt_id, "Data");
      if(DataGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTgrpattrinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Call HE5_EHattrinfo */
      /* ------------------- */
      status = HE5_EHattrinfo(DataGrpID, attrname, ntype, count);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot retrieve information about Attribute \"%s\" in the \"Data\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_PTgrpattrinfo", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Gclose(DataGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTgrpattrinfo", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}	


    }

 COMPLETION:
  return (status);

}






/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqgrpattrs                                                |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list in "Data" group                               |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in point struct    |
|                                                                             |
|  INPUTS:                                                                    |
|  point ID       hid_t               point structure ID                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in point struct         |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 00   A.Muslimov                                                        |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_PTinqgrpattrs(hid_t pointID, char *attrnames, long *strbufsize)
{
  long       nattr      = FAIL;/* Number of attributes (return)  */
  long       idx        = FAIL;/* Point index                    */

  herr_t     status     = FAIL;/* routine return status variable */

  hid_t      fid        = FAIL;/* HDF-EOS file ID                */
  hid_t      gid        = FAIL;/* HDFEOS group ID                */

  char       *grpname   = NULL;/* Buffer for a group name        */

  status = HE5_PTchkptid(pointID, "HE5_PTinqgrpattrs", &fid, &gid, &idx);
  if(status == SUCCEED )
    {
      grpname = (char *) calloc(HE5_HDFE_NAMBUFSIZE, sizeof(char));
      if( grpname == NULL)
        {
          H5Epush(__FILE__, "HE5_PTinqgrpattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
          HE5_EHprint("Error: Can not allocate memory, occured", __FILE__, __LINE__);
          return(FAIL);
        }

      strcpy(grpname,"/HDFEOS/POINTS/");
      strcat(grpname, HE5_PTXPoint[idx].ptname);
      strcat(grpname,"/Data");
	

      /* search group with grpname for the number of attrs */
      /* ------------------------------------------------- */
      nattr = HE5_EHattrcat(fid,  grpname, attrnames, strbufsize);

      free(grpname);
    }

  return (nattr);
   
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwritelocattr                                               |
|                                                                             |
|  DESCRIPTION:   Writes/updates attribute associated with the a specified    |
|                   level in a point.                                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  levelname      char                level name                              |
|  attrname       char                attribute name                          |
|  numtype        hid_t               attribute data type ID                  |
|  count          hsize_t             Number of attribute elements            |
|  wrcode         char                Read/Write Code "w/r"                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf                                                                     |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 00   A.Muslimov                                                        |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTwritelocattr(hid_t pointID, const char *levelname, const char *attrname, hid_t numtype, hsize_t count[],void *datbuf)
{
  herr_t     status     = FAIL;/* routine return status variable */

  hid_t      fid        = FAIL;/* HDF-EOS file ID                */
  hid_t      gid        = FAIL;/* HDFEOS group ID                */
  hid_t      DataGrpID  = FAIL;/* "Data" group ID                */
  hid_t      levelID    = FAIL;/* Level-related dataset ID       */

  long       idx        = FAIL;/* Point index                    */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */
  hid_t      ntype;

  HE5_LOCK;
  CHECKPOINTER(levelname);
  CHECKNAME(attrname);
  CHECKPOINTER(count);
  CHECKPOINTER(datbuf);

  /* 
     if numtype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use ntype itself
  */
  ntype = HE5_EHconvdatatype(numtype);
  if(ntype == FAIL)
    {
      ntype = numtype;
    }

  /* Check Point ID */
  /* -------------- */
  status = HE5_PTchkptid(pointID, "HE5_PTwritelocattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      DataGrpID = H5Gopen(HE5_PTXPoint[idx].pt_id, "Data");
      if(DataGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTwritelocattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
        }

      /* Get level ID  */
      /* ------------- */
      levelID = H5Dopen(DataGrpID, levelname);
      if(levelID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" level dataset ID.\n", levelname);
	  H5Epush(__FILE__, "HE5_PTwritelocattr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
        }
	

      /* Perform Attribute I/O */
      /* --------------------- */
      status = HE5_EHattr(levelID, attrname, ntype, count,"w", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write Attribute \"%s\"to the \"%s\" level.\n", attrname,levelname);
	  H5Epush(__FILE__, "HE5_PTwritelocattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }

      status = H5Dclose(levelID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" level dataset ID.\n", levelname);
	  H5Epush(__FILE__, "HE5_PTwritelocattr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }

      status = H5Gclose(DataGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTwritelocattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }	

    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}





/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadlocattr                                                |
|                                                                             |
|  DESCRIPTION: Reads attribute associated with a specified level in a point. |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  levelname      char                level name                              |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 00   A.Muslimov                                                        |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_PTreadlocattr(hid_t pointID, const char *levelname, const char *attrname, void *datbuf)
{
  herr_t     status     = FAIL;/* routine return status variable */

  hid_t      fid        = FAIL;/* HDF-EOS file ID                */
  hid_t      ntype      = FAIL;/* Data type ID                   */
  hid_t      gid        = FAIL;/* POINTS group ID                */
  hid_t      DataGrpID  = FAIL;/* "Data" group ID                */
  hid_t      levelID    = FAIL;/* Level-related dataset ID       */

  hsize_t     count[]   = {0}; /* Number of attribute elements   */

  long        idx       = FAIL;/* Point index                    */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */

  HE5_LOCK;
  CHECKPOINTER(levelname);
  CHECKPOINTER(attrname);

  /* Check Point ID */
  /* -------------- */
  status = HE5_PTchkptid(pointID, "HE5_PTreadlocattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get "Data"  group ID */
      /* -------------------- */
      DataGrpID = H5Gopen(HE5_PTXPoint[idx].pt_id, "Data");
      if(DataGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTreadlocattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
        }

      /* Get level ID  */
      /* ------------- */
      levelID = H5Dopen(DataGrpID, levelname);
      if(levelID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" level dataset ID.\n", levelname);
	  H5Epush(__FILE__, "HE5_PTreadlocattr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Call EHattr to perform I/O  */
      /* --------------------------- */
      status = HE5_EHattr(levelID, attrname, ntype, count, "r", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot read Attribute \"%s\" from the \"%s\" level.\n", attrname, levelname);
	  H5Epush(__FILE__, "HE5_PTreadlocattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }

      status = H5Dclose(levelID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" level dataset ID.\n", levelname);
	  H5Epush(__FILE__, "HE5_PTreadlocattr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }	

      status = H5Gclose(DataGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTreadlocattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTlocattrinfo                                                |
|                                                                             |
|  DESCRIPTION:  Retrieves information about attributes in associated with a  |
|                 specified level in a point.                                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  levelname      char                level name                              |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          hid_t               attribute data type ID                  |
|  count          hsize_t             Number of attribute elements            |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 00   A.Muslimov                                                        |
|  May 00   A.Muslimov    Changed 'H5T_class_t' type to 'int' data type.      |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  Feb 03   S.Zhao        Changed the type of 'ntype' from an H5T_class_t to  |
|                         an hid_t.                                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_PTlocattrinfo(hid_t pointID, const char *levelname, const char *attrname, hid_t *ntype, hsize_t *count)
{
  herr_t     status     = FAIL;/* routine return status variable */

  hid_t      fid        = FAIL;/* HDF-EOS file ID                */
  hid_t      gid        = FAIL;/* HDFEOS group ID                */
  hid_t      DataGrpID  = FAIL;/* "Data" group ID                */
  hid_t      levelID    = FAIL;/* Level-related dataset ID       */

  long       idx        = FAIL;/* Point index                    */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */

  CHECKPOINTER(levelname);
  CHECKPOINTER(attrname);

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTlocattrinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get "Data"  group ID  */
      /* --------------------- */
      DataGrpID = H5Gopen(HE5_PTXPoint[idx].pt_id, "Data");
      if(DataGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTlocattrinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      /* Get level ID  */
      /* ------------- */
      levelID = H5Dopen(DataGrpID, levelname);
      if(levelID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" level dataset ID.\n", levelname);
	  H5Epush(__FILE__, "HE5_PTlocattrinfo", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Call EHattrinfo */
      /* --------------- */
      status = HE5_EHattrinfo(levelID, attrname, ntype, count);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot retrieve information about Attribute \"%s\" in the \"%s\" level.\n", attrname,levelname);
	  H5Epush(__FILE__, "HE5_PTlocattrinfo", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Dclose(levelID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" level dataset ID.\n", levelname);
	  H5Epush(__FILE__, "HE5_PTlocattrinfo", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }	

      status = H5Gclose(DataGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data\" group ID.\n");
	  H5Epush(__FILE__, "HE5_PTlocattrinfo", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}	
    }

 COMPLETION:
  return (status); 
}






/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqlocattrs                                                |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list from a specified level in a point.            |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in a level         |
|                                                                             |
|  INPUTS:                                                                    |
|  point ID       hid_t               point structure ID                      |
|  levelname      char                level name                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in a level              |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 00   A.Muslimov                                                        |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_PTinqlocattrs(hid_t pointID, const char *levelname, char *attrnames, long *strbufsize)
{
  long       nattr      = FAIL;/* Number of attributes (return)  */
  long       idx        = FAIL;/* Point index                    */

  herr_t     status     = FAIL;/* routine return status variable */

  hid_t      fid        = FAIL;/* HDF-EOS file ID                */
  hid_t      gid        = FAIL;/* HDFEOS group ID                */

  char       *dtsname   = NULL;/* Buffer for a dataset name      */

  CHECKPOINTER(levelname);

  status = HE5_PTchkptid(pointID, "HE5_PTinqlocattrs", &fid, &gid, &idx);
  if(status == SUCCEED )
    {
      dtsname = (char *) calloc(HE5_HDFE_NAMBUFSIZE, sizeof(char));
      if( dtsname == NULL)
        {
          H5Epush(__FILE__, "HE5_PTinqlocattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
          HE5_EHprint("Error: Can not allocate memory, occured", __FILE__, __LINE__);
          return(FAIL);
        }

      strcpy(dtsname,"/HDFEOS/POINTS/");
      strcat(dtsname, HE5_PTXPoint[idx].ptname);
      strcat(dtsname,"/Data/");
      strcat(dtsname,levelname);
	

      /* search group with grpname for the number of attrs */
      /* ------------------------------------------------- */
      nattr = HE5_EHattrcat(fid, dtsname, attrnames, strbufsize);

      free(dtsname);
    }

 COMPLETION:
  return (nattr);
   
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTlinkinfo                                                   |
|                                                                             |
|  DESCRIPTION: Returns ("bck/fwd") linkage info                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  mode           char                mode ("+/-")                            |
|  linkfield      char                linkage field                           |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  8/26/99  A.Muslimov    Changed the return type from intn to int   .        |
|  12/13/99 A.Muslimov    Added proper error handlings after the function     |
|                         calls. Added a call to PTchkptid(), and replaced    |
|                         the parameter "gid" by "fid" in the call to         |
|                         EHmetagroup().                                      |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_PTlinkinfo(hid_t pointID, hid_t gid,  int level, char *mode, char *linkfield)
{
  herr_t          status   = FAIL;            /* Return status variable   */

  hid_t           fid      = FAIL;            /* HDF-EOS file ID          */
  hid_t           pID      = FAIL;            /* Point ID - offset        */
  hid_t           idOffset = HE5_PTIDOFFSET;  /* Point ID offset          */

  long 	          idx      = FAIL;            /* Point index              */
 
  char            *metabuf = NULL;	      /* Pointer to SM            */
  char            *metaptrs[2]={NULL,NULL};   /* Ptrs to begin/end of SM  */
  char            name1[HE5_HDFE_NAMBUFSIZE]; /* Name string 1            */
  char            name2[HE5_HDFE_NAMBUFSIZE]; /* Name string 2            */
  char            utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer           */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error messg buffer       */

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTfwdlinkinfo", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTfwdlinkinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Compute "reduced" point ID */
  /* -------------------------- */
  pID = pointID % idOffset;

  /* Get level link structural metadata */
  /* ---------------------------------- */
  metabuf = (char *)HE5_EHmetagroup(fid, HE5_PTXPoint[pID].ptname, "p","LevelLink", metaptrs);
  if (metabuf == NULL )
    {
      status = FAIL;
      sprintf(errbuf, "Cannot get pointer to metadata buffer for the \"%s\" point. \n", HE5_PTXPoint[pID].ptname);
      H5Epush(__FILE__, "HE5_PTfwdlinkinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  if (strcmp(mode, "-") == 0)
    {
      /* If back link get names of previous & current levels */
      /* --------------------------------------------------- */
      strcpy(name1, HE5_PTXPoint[pID].level[level - 1].name);
      strcpy(name2, HE5_PTXPoint[pID].level[level].name);
    }
  else
    {
      /* If fwd link get names of current & following levels */
      /* --------------------------------------------------- */
      strcpy(name1, HE5_PTXPoint[pID].level[level].name);
      strcpy(name2, HE5_PTXPoint[pID].level[level+1].name);
    }


  /* Search for parent name entry */
  /* ---------------------------- */
  sprintf(utlbuf, "%s%s", "\t\t\t\tParent=\"", name1);
  metaptrs[0] = strstr(metaptrs[0], utlbuf);


  /* If name found within linkage metadata ... */
  /* ----------------------------------------- */
  if (metaptrs[0] < metaptrs[1] || metaptrs[0] == NULL)
    {
      /* Get Linkage Field */
      /* ----------------- */
      status = HE5_EHgetmetavalue(metaptrs, "LinkField", linkfield);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot get \"LinkField\" metavalue for the \"%s\" point. \n", HE5_PTXPoint[pID].ptname);
	  H5Epush(__FILE__, "HE5_PTfwdlinkinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(metabuf);
	  return(status);
	}

      /* Remove double quotes */
      /* -------------------- */
      memmove(linkfield, linkfield + 1, strlen(linkfield) - 2);
      linkfield[strlen(linkfield) - 2] = 0;
    }
  else
    {
      /* If not found return error status */
      /* -------------------------------- */
      status = FAIL;
    }


  free(metabuf);

  return(status);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTbcklinkinfo                                                |
|                                                                             |
|  DESCRIPTION: Returns link field to previous level.                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  linkfield      char                linkage field                           |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_PTbcklinkinfo(hid_t pointID, int level, char *linkfield)
{
  herr_t        status  = SUCCEED;/* routine return status variable */

  hid_t         fid     = FAIL;   /* HDF-EOS file ID                */
  hid_t   	    gid     = FAIL;   /* "POINTS" group ID              */

  long    	    idx     = FAIL;   /* Point index                    */

  char          *mess = "No Back Linkage Defined between levels: %d and %d.\n";
  char          errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer */


  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTfwdlinkinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Check that level is within bounds for backlink info */
      if (level > 0 && level < HE5_PTnlevels(pointID))
	{
	  status = HE5_PTlinkinfo(pointID, gid, level, "-", linkfield);
	  if (status == FAIL)
	    {
	      sprintf( errbuf, mess, level, level - 1);
	      H5Epush(__FILE__, "HE5_PTbcklinkinfo", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	    }
	}
      else
	{
	  status = FAIL;
	  sprintf( errbuf, mess, level, level - 1);
	  H5Epush(__FILE__, "HE5_PTbcklinkinfo", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
        
    }

  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTfwdlinkinfo                                                |
|                                                                             |
|  DESCRIPTION: Returns link field to following level.                        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  linkfield      char                linkage field                           |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_PTfwdlinkinfo(hid_t pointID, int level, char *linkfield)
{
  herr_t         status  = SUCCEED;/* routine return status variable  */
  
  hid_t          fid     = FAIL;   /* HDF-EOS file id                 */
  hid_t          gid     = FAIL;   /* "POINTS" group ID               */
  
  long           idx     = FAIL;   /* Point index                     */
  
  char           *mess = "No Forward Linkage Defined between levels: %d and %d.\n";
  char           errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer  */
  
  
  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTfwdlinkinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Check that level is within bounds for fwdlink info */
      if (level >= 0 && level < HE5_PTnlevels(pointID) - 1)
	{
	  status = HE5_PTlinkinfo(pointID, gid, level, "+", linkfield);
	  if (status == FAIL)
	    {
	      sprintf( errbuf, mess, level, level + 1);
	      H5Epush(__FILE__, "HE5_PTfwdlinkinfo", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);			  
	    }
	}
      else
	{
	  status = FAIL;
	  sprintf( errbuf, mess, level, level + 1);
	  H5Epush(__FILE__, "HE5_PTfwdlinkinfo", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
    }
  
  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTlevelindx                                                  |
|                                                                             |
|  DESCRIPTION: Returns index number for a named level.                       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  n              int                 Level number (0 - based)                |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  levelname      char                point level name                        |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTlevelindx(hid_t pointID, const char *levelname)
{
  int             level   = FAIL;/* Level corresponding to levelname (RETURN) */
  int             nlevels = FAIL;/* Number of levels in point                 */

  herr_t          status  = FAIL;/* routine return status variable            */

  hid_t           fid     = FAIL;/* HDF-EOS file ID                           */
  hid_t    	      gid     = FAIL;/* "POINTS" group ID                         */

  long     	      idx     = FAIL;/* Point index                               */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer         */ 

  CHECKPOINTER(levelname);

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTlevelindx", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get number of levels */
      /* -------------------- */
      nlevels = HE5_PTnlevels(pointID);
      if(nlevels == FAIL)
	{
	  sprintf(errbuf, "Cannot get the number of levels in point.\n");
	  H5Epush(__FILE__, "HE5_PTlevelindx", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      else if(nlevels == 0)
	{
	  sprintf(errbuf, "There are no levels in point.\n");
	  H5Epush(__FILE__, "HE5_PTlevelindx", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      else 
	{

	  /* Loop through all levels in point */
	  /* -------------------------------- */
	  for (level = 0; level < nlevels; level++)
	    {
	      /* If it matches input levelname then exit loop */
	      /* -------------------------------------------- */
	      if (strcmp(HE5_PTXPoint[ idx ].level[ level ].name, levelname) == 0)
		break;
	    }
	}
	  
    }
  
  /* Levelname not found so set error status */
  /* --------------------------------------- */
  if (level == nlevels)
    level = FAIL;

 COMPLETION:
  return(level);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwrbckptr                                                   |
|                                                                             |
|  DESCRIPTION: Writes back pointer records                                   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level index (0 - based)                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Nov 00   A.Muslimov    Changed the interface.                              |
|  Mar 00   A.Muslimov    Modified to reflect changes in HDF5 library:        |
|                         H5Tinsert_array should be replaced by two calls     |
|                         H5Tarray_create() and H5Tinsert().                  |
|  Sep 04   S.Zhao        Changed "long long" to "LONGLONG" for Windows.      |
|  Feb 06   Abe Taaheri   Support for STRING fields,                          |
|                                                                             |
|  END_PROLOG                                                                 |
  ----------------------------------------------------------------------------*/
static herr_t   
HE5_PTwrbckptr(hid_t pointID, int level)
{
  herr_t          status   = FAIL;/* routine return status variable           */

  int             TypeFlag = FAIL;/* Data type Flag                           */
  int             k;              /* Loop index                               */

  hid_t           fid      = FAIL;/* HDF-EOS file ID                          */
  hid_t           gid      = FAIL;/* "POINTS" group ID                        */
  hid_t           link_id  = FAIL;/* "Linkage" group ID                       */
  hid_t           bckid    = FAIL;/* "BACKPOINTER" dataset ID                 */
  hid_t           typeID   = FAIL;/* Data type ID                             */
  hid_t           tid      = FAIL;/* Data type ID                             */
  hid_t           fldtype  = FAIL;/* Field data type ID                       */
  hid_t           xfer_list= FAIL;/* Property list  ID                        */
  hid_t           dataID_curr = FAIL;/* current level dataset ID              */
  hid_t           dataID_prev = FAIL;/* previous level dataset ID             */
  hid_t           arr_id      = FAIL;/* Array datatype ID                     */ 

  size_t          datasize = 0;   /* Input data size parameter                */

  long            idx      = FAIL;/* Point index                              */
  LONGLONG        j;		  /* Loop indices                             */

  hsize_t         count[1] = {0}; /* Number of elements to write              */
  hsize_t         nrecPrev = 0;   /* Number of records in previous level      */
  hsize_t         nrecCurr = 0;	  /* Number of records in current level       */
  hsize_t         dims[HE5_DTSETRANKMAX];/* Data member dimension             */

  char            utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer               */
  char            *mess = "No Linkage Defined between levels: %d and %d.\n";
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* buffer for error message     */

  bckptr          *bck = (bckptr *)NULL;/* struct to write bckpointer data to */

  HE5_CmpDTSinfo  currLevel;      /* structure for current level information  */

  /*         Data buffers      */
  /* ------------------------- */

  int             *intFldCurr;
  int             *intFldPrev;

  float           *fltFldCurr;
  float           *fltFldPrev;

  double          *dblFldCurr;
  double          *dblFldPrev;

  long            *lngFldCurr;
  long            *lngFldPrev;

  char            *chrFldCurr;
  char            *chrFldPrev;
  int              string_size;
  hid_t            cdatatype = FAIL;   
  hid_t            fldnumtype;

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTwrbckptr", &fid, &gid, &idx);
  if (status == FAIL)
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Check the level number */
  /* ---------------------- */
  if (level <= 0)
    {
      sprintf(errbuf, "Passed wrong level number. Level should be > 0. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Get Back-Linkage Field (copy into utlbuf) */
  /* ----------------------------------------- */
  status = HE5_PTbcklinkinfo(pointID, level, utlbuf);
  if (status == FAIL)
    {
      sprintf(errbuf, mess, level, level - 1);
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Get number of fields in Current level */
  /* ------------------------------------- */
  status = HE5_PTlevelinfo(pointID, level, &currLevel);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot get information about current level. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Initialize dims[] array */
  /* ----------------------- */
  for (k = 0; k < HE5_DTSETRANKMAX; k++)
    dims[ k ] = 0;


  /* Get current level dataset ID */
  /* ---------------------------- */
  dataID_curr = HE5_PTXPoint[idx].level[level].ID;

  /* Get the field data type and number of elements (for char ) */
  /* ---------------------------------------------------------- */
  for (k = 0; k < currLevel.nfields; k++)
    {
      if (memcmp(currLevel.fieldname[k], utlbuf, strlen(utlbuf)) == 0)
	{
	  fldtype = currLevel.dtype[k];
	  fldnumtype = currLevel.numtype[k];
	  dims[0] = (hsize_t)(currLevel.dims[k][0]);

	  if(currLevel.dclass[k] == H5T_STRING)
	    {
	      TypeFlag = 5;
	      string_size = HE5_PTgetdtypesize(dataID_curr, k);
	      if (string_size == FAIL)
		{
		  sprintf(errbuf,"Cannot get member datatype size.\n");
		  H5Epush(__FILE__, "HE5_PTwrbckptr", 
			  __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	    }
	  else
	    {
	      cdatatype = HE5_EHconvdatatype(currLevel.numtype[k]);
	      /* 
		 if numtype is HE5 numbertype, rather than H5 numbertype, 
		 then convert it, otherwise use numtype itself
	      */
	      if(cdatatype == FAIL)
		{
		  cdatatype = currLevel.numtype[k];
		}
	      if(H5Tequal(cdatatype, H5T_NATIVE_INT))
		TypeFlag = 1;
	      else if(H5Tequal(cdatatype, H5T_NATIVE_LONG))
		TypeFlag = 2;
	      else if(H5Tequal(cdatatype, H5T_NATIVE_FLOAT))
		TypeFlag = 3;
	      else if(H5Tequal(cdatatype, H5T_NATIVE_DOUBLE))
		TypeFlag = 4;
	      else if(H5Tequal(cdatatype, H5T_NATIVE_CHAR))
		TypeFlag = 0;
	    }
	}

      /* Deallocate memory */
      /* ----------------- */
      if (currLevel.fieldname[k] != NULL) 
	free(currLevel.fieldname[k]);
    }

  
  /* Get number of records in current level */
  /* -------------------------------------- */
  nrecCurr = HE5_PTnrecs(pointID,level);
  if (nrecCurr == 0)
    {
      sprintf(errbuf, "No records in current level. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get number of records in previous level */
  /* --------------------------------------- */
  nrecPrev = HE5_PTnrecs(pointID,level-1);
  if (nrecPrev == 0)
    {
      sprintf(errbuf, "No records in previous level. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate memory for bckpointer dataset */
  /* -------------------------------------- */
  bck = (bckptr *)calloc(nrecCurr, sizeof(bckptr));
  if (bck == NULL)
    {
      sprintf(errbuf, "Cannot allocate memory. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Get previous level dataset ID */
  /* ----------------------------- */
  dataID_prev = HE5_PTXPoint[idx].level[level-1].ID;


  /* Get ID of Linkage group  */
  /* ------------------------ */
  link_id = HE5_PTXPoint[idx].link_id;

  /* Get data buffer containing link field records in current level */
  /* -------------------------------------------------------------- */  
  switch (TypeFlag)
    {

    case 5: /* string type field */
      {
	datasize = (size_t)(dims[0] * sizeof(char));
	chrFldCurr = (char *)calloc(nrecCurr, string_size);
	if (chrFldCurr == NULL)
	  {
	    sprintf(errbuf, "Cannot allocate memory. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    return(FAIL);
	  }

	chrFldPrev = (char *)calloc(nrecPrev, string_size);
	if (chrFldPrev == NULL)
	  {
	    sprintf(errbuf, "Cannot allocate memory. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(chrFldCurr);
	    free(bck);
	    return(FAIL);
	  }

	/* Create Compound datatype */
	/* ------------------------ */
	tid    = H5Tcreate(H5T_COMPOUND, string_size);
	if (tid == FAIL)
	  {
	    sprintf(errbuf, "Cannot create Compound datatype. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    free(bck);
	    return(FAIL);
	  }

	/* Create array datatype */
	/* --------------------- */
	arr_id = H5Tcopy(H5T_C_S1);
	if (arr_id == FAIL)
	  {
	    sprintf(errbuf, "Cannot create array data type. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }

	H5Tset_size(arr_id,string_size);
	H5Tset_strpad(arr_id, H5T_STR_NULLPAD);

	/* Add array field */
	/* --------------- */
	status = H5Tinsert(tid, utlbuf, 0, arr_id);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot add array data field. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }
		  
	/* Read the link field data from Current level */
	/* ------------------------------------------- */
	status = H5Dread(dataID_curr, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, chrFldCurr);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from current level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }
		
	/* Read the link field data from Previous level */
	/* -------------------------------------------- */
	status = H5Dread(dataID_prev, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, chrFldPrev);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from previous level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }

	/* Release datatype ID  */
	/* -------------------- */
	status = H5Tclose(arr_id);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot release the array datatype ID. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }
		
		
	/* Loop through input records */
	/* -------------------------- */
	for (k = 0; k < nrecCurr; k++){
	  for (j = 0; j < nrecPrev; j++){
	    if (strcmp((chrFldCurr + (k*string_size)), 
		       (chrFldPrev + (j*string_size))) == 0)
	      {
		bck[k].back = j;
		break;
	      }
	  }
	}

	free(chrFldCurr);
	free(chrFldPrev);
      }

      break;

    case 0:
      {
	datasize = (size_t)(dims[0] * sizeof(char));

	chrFldCurr = (char *)calloc(nrecCurr, datasize);
	if (chrFldCurr == NULL)
	  {
	    sprintf(errbuf, "Cannot allocate memory. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    return(FAIL);
	  }

	chrFldPrev = (char *)calloc(nrecPrev, datasize);
	if (chrFldPrev == NULL)
	  {
	    sprintf(errbuf, "Cannot allocate memory. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(chrFldCurr);
	    free(bck);
	    return(FAIL);
	  }

	/* Create Compound datatype */
	/* ------------------------ */
	tid    = H5Tcreate(H5T_COMPOUND, datasize);
	if (tid == FAIL)
	  {
	    sprintf(errbuf, "Cannot create Compound datatype. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    free(bck);
	    return(FAIL);
	  }

	/* Create array datatype */
	/* --------------------- */
	/*arr_id = H5Tarray_create(HE5_EHdtype2mtype(fldtype), 1, dims, NULL);*/
	cdatatype = HE5_EHconvdatatype(fldnumtype);
	/* 
	   if fldnumtype is HE5 numbertype, rather than H5 numbertype, 
	   then convert it, otherwise use fldnumtype itself
	*/
	if(cdatatype == FAIL)
	  {
	    cdatatype = fldnumtype;
	  }
	arr_id = H5Tarray_create(cdatatype, 1, dims, NULL);
		
	if (arr_id == FAIL)
	  {
	    sprintf(errbuf, "Cannot create array data type. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }

	/* Add array field */
	/* --------------- */
	status = H5Tinsert(tid, utlbuf, 0, arr_id);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot add array data field. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }
		  
	/* Read the link field data from Current level */
	/* ------------------------------------------- */
	status = H5Dread(dataID_curr, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, chrFldCurr);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from current level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }
		
	/* Read the link field data from Previous level */
	/* -------------------------------------------- */
	status = H5Dread(dataID_prev, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, chrFldPrev);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from previous level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }

	/* Release datatype ID  */
	/* -------------------- */
	status = H5Tclose(arr_id);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot release the array datatype ID. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(chrFldCurr);
	    free(chrFldPrev);
	    return(FAIL);
	  }
		
		
	/* Loop through input records */
	/* -------------------------- */
	for (k = 0; k < nrecCurr; k++){
	  for (j = 0; j < nrecPrev; j++){
	    if (memcmp(chrFldCurr+k*datasize, chrFldPrev+j*datasize, dims[0]) == 0)
	      {
		bck[k].back = j;
		break;
	      }
	  }
	}
		
	free(chrFldCurr);
	free(chrFldPrev);
      }

      break;

    case 1:

      {
	intFldCurr = (int *)calloc(nrecCurr, sizeof(int));
	intFldPrev = (int *)calloc(nrecPrev, sizeof(int));
	  
	/* Create Compound datatype */
	/* ------------------------ */
	tid    = H5Tcreate(H5T_COMPOUND, sizeof(int));
	if (tid == FAIL)
	  {
	    sprintf(errbuf, "Cannot create Compound datatype. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(intFldCurr);
	    free(intFldPrev);
	    free(bck);
	    return(FAIL);
	  }

	/* Insert data member */
	/* ------------------ */
	status = H5Tinsert(tid, utlbuf, 0, HE5_EHdtype2mtype(fldtype));
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot insert data member. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(intFldCurr);
	    free(intFldPrev);
	    return(FAIL);
	  }
		  
	/* Read the link field data from Current level */
	/* ------------------------------------------- */
	status = H5Dread(dataID_curr, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, intFldCurr);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from current level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(intFldCurr);
	    free(intFldPrev);
	    return(FAIL);
	  }
		
	/* Read the link field data from Previous level */
	/* -------------------------------------------- */
	status = H5Dread(dataID_prev, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, intFldPrev);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from previous level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(intFldCurr);
	    free(intFldPrev);
	    return(FAIL);
	  }

		
	/* Loop through input records */
	/* -------------------------- */
	for (k = 0; k < nrecCurr; k++){
	  for (j = 0; j < nrecPrev; j++){
	    if (intFldCurr[k] == intFldPrev[j])
	      {
		bck[k].back = j;
		break;
	      }
	  }
	}  
	free(intFldCurr);
	free(intFldPrev);	  
      }  

      break;

    case 2:

      {
	lngFldCurr = (long *)calloc(nrecCurr, sizeof(long));
	lngFldPrev = (long *)calloc(nrecPrev, sizeof(long));

	/* Create Compound datatype */
	/* ------------------------ */
	tid    = H5Tcreate(H5T_COMPOUND, sizeof(long));
	if (tid == FAIL)
	  {
	    sprintf(errbuf, "Cannot create Compound datatype. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(lngFldCurr);
	    free(lngFldPrev);
	    free(bck);
	    return(FAIL);
	  }

	/* Insert data member */
	/* ------------------ */
	status = H5Tinsert(tid, utlbuf, 0, HE5_EHdtype2mtype(fldtype));
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot insert data member. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(lngFldCurr);
	    free(lngFldPrev);
	    return(FAIL);
	  }
		  
	/* Read the link field data from Current level */
	/* ------------------------------------------- */
	status = H5Dread(dataID_curr, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, lngFldCurr);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from current level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(lngFldCurr);
	    free(lngFldPrev);
	    return(FAIL);
	  }
		
	/* Read the link field data from Previous level */
	/* -------------------------------------------- */
	status = H5Dread(dataID_prev, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, lngFldPrev);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from previous level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(lngFldCurr);
	    free(lngFldPrev);
	    return(FAIL);
	  }
	  
	  
	/* Loop through input records */
	/* -------------------------- */
	for (k = 0; k < nrecCurr; k++){
	  for (j = 0; j < nrecPrev; j++){
	    if (lngFldCurr[k] == lngFldPrev[j])
	      {
		bck[k].back = j;
		break;
	      }
	  }
	}  
	free(lngFldCurr);
	free(lngFldPrev);	  
      }  

      break;

    case 3:
	  
      {
	fltFldCurr = (float *)calloc(nrecCurr, sizeof(float));
	fltFldPrev = (float *)calloc(nrecPrev, sizeof(float));

	/* Create Compound datatype */
	/* ------------------------ */
	tid    = H5Tcreate(H5T_COMPOUND, sizeof(float));
	if (tid == FAIL)
	  {
	    sprintf(errbuf, "Cannot create Compound datatype. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(fltFldCurr);
	    free(fltFldPrev);
	    free(bck);
	    return(FAIL);
	  }

	/* Insert data member */
	/* ------------------ */
	status = H5Tinsert(tid, utlbuf, 0, HE5_EHdtype2mtype(fldtype));
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot insert data member. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(fltFldCurr);
	    free(fltFldPrev);
	    return(FAIL);
	  }
		  
	/* Read the link field data from Current level */
	/* ------------------------------------------- */
	status = H5Dread(dataID_curr, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, fltFldCurr);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from current level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(fltFldCurr);
	    free(fltFldPrev);
	    return(FAIL);
	  }
		
	/* Read the link field data from Previous level */
	/* -------------------------------------------- */
	status = H5Dread(dataID_prev, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, fltFldPrev);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from previous level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(fltFldCurr);
	    free(fltFldPrev);
	    return(FAIL);
	  }	  
	  
	/* Loop through input records */
	/* -------------------------- */
	for (k = 0; k < nrecCurr; k++){
	  for (j = 0; j < nrecPrev; j++){
	    if (fltFldCurr[k] == fltFldPrev[j])
	      {
		bck[k].back = j;
		break;
	      }
	  }
	}  
	free(fltFldCurr);
	free(fltFldPrev);	  
      }  

      break;

    case 4:
	  
      {
	dblFldCurr = (double *)calloc(nrecCurr, sizeof(double));
	dblFldPrev = (double *)calloc(nrecPrev, sizeof(double));

	/* Create Compound datatype */
	/* ------------------------ */
	tid    = H5Tcreate(H5T_COMPOUND, sizeof(double));
	if (tid == FAIL)
	  {
	    sprintf(errbuf, "Cannot create Compound datatype. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(dblFldCurr);
	    free(dblFldPrev);
	    free(bck);
	    return(FAIL);
	  }

	/* Insert data member */
	/* ------------------ */
	status = H5Tinsert(tid, utlbuf, 0, HE5_EHdtype2mtype(fldtype));
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot insert data member. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(dblFldCurr);
	    free(dblFldPrev);
	    return(FAIL);
	  }
		  
	/* Read the link field data from Current level */
	/* ------------------------------------------- */
	status = H5Dread(dataID_curr, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, dblFldCurr);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from current level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(dblFldCurr);
	    free(dblFldPrev);
	    return(FAIL);
	  }
		
	/* Read the link field data from Previous level */
	/* -------------------------------------------- */
	status = H5Dread(dataID_prev, tid, H5S_ALL, H5S_ALL, H5P_DEFAULT, dblFldPrev);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read link field from previous level. \n");
	    H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(bck);
	    free(dblFldCurr);
	    free(dblFldPrev);
	    return(FAIL);
	  }
	  
	/* Loop through input records */
	/* -------------------------- */
	for (k = 0; k < nrecCurr; k++){
	  for (j = 0; j < nrecPrev; j++){
	    if (dblFldCurr[k] == dblFldPrev[j])
	      {
		bck[k].back = j;
		break;
	      }
	  }
	}  
	free(dblFldCurr);
	free(dblFldPrev);	  
      }  

      break;

    default:

      {
	sprintf(errbuf, "Link record datatype is not supported. \n");
	H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	HE5_EHprint(errbuf, __FILE__, __LINE__);
	free(bck);
	return(FAIL);
      }
	  
    }

  /* Release datatype ID  */
  /* -------------------- */
  status = H5Tclose(tid);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the datatype ID. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }


  /* Get ID of BCKPOINTER data  */
  /* -------------------------- */
  sprintf(utlbuf, "%s%d%s%d", "BCKPOINTER:", level, "->", level - 1);
  
  /* Open "BCKPOINTER" dataset */
  /* ------------------------- */
  bckid = H5Dopen(link_id, utlbuf);
  if (bckid == FAIL)
    {
      sprintf(errbuf, "Cannot open the \"BCKPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }
  
  datasize = (size_t)sizeof(bckptr);
  count[0] = (hsize_t)nrecCurr;

  /* Extend BCKPOINTER dataset  */
  /* -------------------------- */
  status = H5Dextend(bckid, count);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot extend the \"BCKPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }
 
  /* Get datatype ID of BCKPOINTER data  */
  /* ----------------------------------- */
  typeID = H5Dget_type(bckid);
  if (typeID == FAIL)
    {
      sprintf(errbuf, "Cannot get the datatype of \"BCKPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }

  /* Create the property list   */
  /* -------------------------- */
  xfer_list = H5Pcreate(H5P_DATASET_XFER);
  if (xfer_list == FAIL)
    {
      sprintf(errbuf, "Cannot create the property list. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_PLIST, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }
  
  /* Set buffer  */
  /* ----------- */
  status = H5Pset_buffer(xfer_list, datasize, NULL, NULL);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot set the buffer. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }
  
	  
  /* Write back pointer values to "BCKPOINTER" dataset */
  /* ------------------------------------------------- */
  status = H5Dwrite(bckid, typeID, H5S_ALL,H5S_ALL, xfer_list, bck);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot write data to the \"BCKPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }
  
  /* Release dataset ID  */
  /* ------------------- */
  status = H5Dclose(bckid);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the dataset ID. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }
  
  /* Release datatype ID  */
  /* -------------------- */
  status = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the datatype ID. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }

  /* Release property list ID  */
  /* ------------------------- */
  status = H5Pclose(xfer_list);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the property list ID. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptr", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(bck);
      return(FAIL);
    }


  free(bck);

  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTrdbckptr                                                   |
|                                                                             |
|  DESCRIPTION: Reads back pointers                                           |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level index (0 - based)                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|  buffer         void                Data buffer                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|  Date      Programmer    Description                                        |
|  ========  ============  =================================================  |
|  12/20/99  A. Muslimov   Added proper error handlings after the function    |
|                           calls. Made some minor (mostly cosmetic) changes. |
|  7.18.99   A.Muslimov    Unmasked hdf5 data types.                          |
|  Nov  00   A.Muslimov    Changed the interface.                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t  
HE5_PTrdbckptr(hid_t pointID, int level, void *buffer)
{
  herr_t         status  = FAIL;/* routine return status variable */

  hid_t          fid     = FAIL;/* HDF-EOS file ID                */
  hid_t          gid     = FAIL;/* POINTS group ID                */
  hid_t          bckid   = FAIL;/* "BACKPOINTER" dataset ID       */
  hid_t          typeID  = FAIL;/* Data type ID                   */
  hid_t          link_id = FAIL;/* "Linkage" group ID             */

  long           idx     = FAIL;/* Point index                    */

  char           utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer    */
  char           *mess = "No Linkage Defined between levels: %d and %d.\n";
  char           errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer */


  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTrdbckptr", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Check the level number */
  /* ---------------------- */
  if (level <= 0)
    {
      sprintf(errbuf, "Passed wrong level number. Level should be > 0. \n");
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get Back-Linkage Field */
  /* ---------------------- */
  status = HE5_PTbcklinkinfo(pointID, level, utlbuf);
  if (status == FAIL)
    {
      sprintf(errbuf, mess, level, level - 1);
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  
  /* Get ID of "Linkage" group */
  /* ------------------------ */
  link_id = HE5_PTXPoint[idx].link_id;
  
  /* Get ID of BCKPOINTER data */
  /* -------------------------- */
  sprintf(utlbuf, "%s%d%s%d","BCKPOINTER:", level, "->", level - 1);

  
  /* READ THE "BCKPOINTER" DATASET */
  /* ----------------------------- */


  /* Open the "BCKPOINTER" dataset */
  /* ----------------------------- */
  bckid = H5Dopen(link_id, utlbuf); 
  if (bckid == FAIL)
    {
      sprintf(errbuf,"Cannot open the \"BCKPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

    
  /* Create Compound datatype */
  /* ------------------------ */
  typeID = H5Tcreate(H5T_COMPOUND, sizeof(bckptr));
  if (typeID == FAIL)
    {
      sprintf(errbuf,"Cannot create the Compound datatype. \n");
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Insert data member */
  /* ------------------ */
  status = H5Tinsert(typeID,"BCKPOINTER",HOFFSET(bckptr, back),H5T_NATIVE_LLONG); 
  if (status == FAIL)
    {
      sprintf(errbuf,"Cannot insert data member to  \"BCKPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Read the data from the dataset */
  /* ------------------------------ */
  status = H5Dread(bckid, typeID, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
  if (status == FAIL)
    {
      sprintf(errbuf,"Cannot read data from  the \"BCKPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* release dataset ID */
  /* ------------------ */
  status = H5Dclose(bckid);
  if (status == FAIL)
    {
      sprintf(errbuf,"Cannot release the dataset ID. \n");
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* release datatype ID */
  /* ------------------- */
  status = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf,"Cannot release the datatype ID. \n");
      H5Epush(__FILE__, "HE5_PTrdbckptr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  return (status);
}





/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwrfwdptr                                                   |
|                                                                             |
|  DESCRIPTION: Write forward pointer records                                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level index (0 - based)                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/20/99 A. Muslimov   Added proper error handlings after the function     |
|                           calls. Corrected memory management stuff. Made    |
|                           some minor (mostly cosmetic) changes.             |
|  7.18.99  A.Muslimov    Unmasked hdf5 data types.                           |
|  Nov  00  A.Muslimov    Changed the interface.                              |
|  Sept 04  S.Zhao        Changed "long long" to "LONGLONG" for Windows.      |
|  Feb  06  Abe Taaheri   Support for STRING fields and setting pointers to -1|
|                         values when the records in child level is not       |
|                         monotonic                                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t   
HE5_PTwrfwdptr(hid_t pointID, int level)
{
  herr_t          status  = FAIL; /* routine return status variable         */

  int             FORWARD = TRUE; /* Forward pointer flag                   */
  int             nlevels = 0;	  /* Number of levels in point              */

  hid_t           fid     = FAIL; /* HDF-EOS file ID                        */
  hid_t           gid     = FAIL; /* POINTS group ID                        */
  hid_t           fwdID   = FAIL; /* "FORWARD" pointer data set ID          */
  hid_t           typeID  = FAIL; /* Data type ID                           */
  hid_t           link_id = FAIL; /* "Linkage" group ID                     */

  long            idx     = FAIL; /* Point index                            */

  LONGLONG        max     = 0;	  /* Maximun back pointer value             */
  LONGLONG        i, j, k;        /* Loop indices                           */
  LONGLONG        start   = 0;    /* Loop index                             */
  LONGLONG        nrec    = 0;	  /* # of records in FWDPOINTER dataset     */
  /* hsize_t         nrec    = 0;*/ /* # of records in FWDPOINTER dataset   */

  hsize_t         count   = 0;    /* Size of the fwdpointer dataset         */

  bckptr          *bck = (bckptr *)NULL;/* Struct for bckpointer dataset    */
  fwdptr          *fwd = (fwdptr *)NULL;/* Struct for fwdpointer dataset    */

  char            utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer             */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer       */


  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTwrfwdptr", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get number of levels in point */
  /* ----------------------------- */
  nlevels = HE5_PTnlevels(pointID);

  /* Get number of records in following level */
  /* ---------------------------------------- */
  nrec = (level < nlevels - 1) ? (LONGLONG)HE5_PTnrecs(pointID, level + 1) : 0;

  /* If records exist in current and following level... */
  /* -------------------------------------------------- */
  if ((int)HE5_PTnrecs(pointID, level) > 0 && (int)nrec > 0)
	
    {
      /* Allocate memory */
      /* --------------- */
      bck = (bckptr *)calloc((int)nrec, sizeof(bckptr));
	 
      if (bck == (bckptr *)NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory. \n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      status = HE5_PTrdbckptr(pointID, level + 1, bck);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot read the back pointer. \n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(bck);
	  return(FAIL);
	}


      /* Find Max BackPointer value */
      /* -------------------------- */
      max = bck[0].back;
      for (i = 1; i < nrec; i++)
	{
	  if (bck[i].back > max)
	    {
	      max = bck[i].back;
	    }
	}
	  

      /* Allocate memory for fwdpointer struct */
      /* ------------------------------------- */
      fwd = (fwdptr *)calloc((int)max + 1, sizeof(fwdptr));
	 
      if(fwd == (fwdptr *)NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory. \n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(bck);

	  return(FAIL);
	}


      /* Get ID of "Linkage" group */
      /* ------------------------- */
      link_id = HE5_PTXPoint[idx].link_id;

      /* Get ID of FWDPOINTER dataset */
      /* ---------------------------- */
      sprintf(utlbuf, "%s%d%s%d", "FWDPOINTER:", level, "->", level+1);

      /* Open FWDPOINTER dataset */
      /* ----------------------- */
      fwdID = H5Dopen(link_id, utlbuf);
      if (fwdID == FAIL)
	{
	  sprintf(errbuf, "Cannot open the dataset. \n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(fwd);
	  free(bck);
	  return(FAIL);
	}

      count = (hsize_t)max + 1;

      /* Extend the dataset size */
      /* ----------------------- */
      status = H5Dextend(fwdID, &count);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot extend the dataset.\n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(fwd);
	  free(bck);
	  return(FAIL);
	}


      /* Get dataset FWDPOINT:?->? datatype */
      /* ---------------------------------- */ 
      typeID = H5Dget_type(fwdID);
      if (typeID == FAIL)
	{
	  sprintf(errbuf, "Cannot get the data type ID. \n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(fwd);
	  free(bck);
	  return(FAIL);
	}
	  
      /* Set the values for the forward pointers */
      /* --------------------------------------- */


      /* Fill Fwd Ptr buffers with -1 */
      /* ---------------------------- */
      for ( k = 0; k <= max; k++)
	{
	  fwd[k].begin  = (LONGLONG)FAIL;
	  fwd[k].extent = (LONGLONG)FAIL;
	}
	  
      /* Set forward pointer flag to 1 (TRUE) */
      /* ----------------------------- */
      FORWARD = TRUE;
	  
      /* Loop through all records */
      /* ------------------------ */
      for (j = 0; j < nrec; j++)
	{
	  /* If fwd extent entry not yet written for rec[j] ... */
	  /* ----------------------------------------------- */
	  if (fwd[bck[j].back].extent == (LONGLONG)FAIL)
	    {
	      /* Set fwd begin to (possible) beginning of sequence */
	      /* -------------------------------------------- */
	      fwd[bck[j].back].begin  =  (LONGLONG)j;
		  
	      /* Set fwd extent to initial value of sequence */
	      /* ------------------------------------- */
	      fwd[bck[j].back].extent = (LONGLONG)j;
	    }
	  else
	    {
	      /* If numbers in sequence ... */
	      /* -------------------------- */
	      if ((LONGLONG)j - fwd[bck[j].back].extent == 1)
		{
		  /* Set Buf1 to current value of sequence */
		  /* ------------------------------------- */
		  fwd[bck[j].back].extent = (LONGLONG)j;
		}
	      else
		{
		  /* Back pointers in following level 
		     not monotonic */
		  /* ---------------------------------*/
		      
		      
		  /* Set begin begin and extent for first record 
		     in Forward pointer to Fail
		  */
		  /* -------------------------------------- */
		  fwd[0].begin = (LONGLONG)FAIL;
		  fwd[0].extent = (LONGLONG)FAIL;

		  /* also set to fail other elemets */
		  fwd[bck[j].back].begin  = (LONGLONG)FAIL;
		  fwd[bck[j].back].extent = (LONGLONG)FAIL;
		      
		  /* Set forward pointer flag to 0 */
		  /* ----------------------------- */
		  FORWARD = FALSE;
		  break;
		}
	    }
	}

      /* Although setting to (-1,-1) of first forward pointer 
	 record for non-monotonic backward pointers is good enough
	 for knowing this fact, let's put -1 in rest of the records
	 for making output in hdf file less confusing */

      if((fwd[0].begin == (LONGLONG)FAIL) &&
	 (fwd[0].extent == (LONGLONG)FAIL))
	{
	  for ( k = 1; k <= max; k++)
	    {
	      fwd[k].begin  = (LONGLONG)FAIL;
	      fwd[k].extent = (LONGLONG)FAIL;
	    }
	}

      /* Back pointers in following level are monotonic */
      /* ---------------------------------------------- */
	  
      if (FORWARD == TRUE)
	{
	  for (k = 0; k <= max; k++)
	    {
	      fwd[k].begin = fwd[k].begin;
	      fwd[k].extent = fwd[k].extent - fwd[k].begin	+ 1;
	    }
	}
	  
      /* Write data to the "FWDPOINTER" */
      /* ------------------------------ */
      status = H5Dwrite(fwdID, typeID, H5S_ALL, H5S_ALL, H5P_DEFAULT, fwd);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot write data to the dataset. \n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fwd != NULL) free(fwd);
	  if (bck != NULL) free(bck);
	  return(FAIL);
	}
	  

      if (bck != NULL) free(bck);
      if (fwd != NULL) free(fwd);
	  
      /* Release the dataset ID */
      /* ---------------------- */
      status = H5Dclose(fwdID);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot release the dataset ID. \n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Release the datatype ID */
      /* ----------------------- */
      status = H5Tclose(typeID);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot release the data type ID. \n");
	  H5Epush(__FILE__, "HE5_PTwrfwdptr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }

  return(status);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTrdfwdptr                                                   |
|                                                                             |
|  DESCRIPTION: Read forward pointer records                                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t                 point structure ID                    |
|  level          int                   level index (0 - based)               |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  buffer         void                  Data buffer                           |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer    Description                                        |
|  ========  ============  =================================================  |
|  12/20/99  A. Muslimov   Added proper error handlings after the function    |
|                          calls. Made some minor (mostly cosmetic) changes.  |
|  7.18.99   A.Muslimov    Unmasked hdf5 data types.                          |
|  Nov 00    A.Muslimov    Changed the interface.                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t   
HE5_PTrdfwdptr(hid_t pointID, int level, void *buffer)
{
  herr_t          status    = FAIL;/* routine return status variable  */

  hid_t           fid       = FAIL;/* HDF-EOS file ID                 */
  hid_t           gid       = FAIL;/* POINTS group ID                 */
  hid_t           link_id   = FAIL;/* Linkage group ID                */
  hid_t           fwdID     = FAIL;/* FWDPOINTER dataset ID           */
  hid_t           typeID    = FAIL;/* FWDPOINTER dataset datatype ID  */

  long            idx       = FAIL;/* Point index                     */

  char            utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer       */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* error message buffer */

  fwdptr          fwd;             /* Tmp FWDPOINTER struct dataset   */  

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTrdfwdptr", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get ID of Linkage  group */
  /* ------------------------ */
  link_id = HE5_PTXPoint[idx].link_id;

  /* Get name string of FWDPOINTER dataset */
  /* ------------------------------------- */
  sprintf(utlbuf, "%s%d%s%d", "FWDPOINTER:", level, "->", level + 1);


  /* Open the "FWDPOINTER" dataset */
  /* ----------------------------- */
  fwdID = H5Dopen(link_id, utlbuf);
  if (fwdID == FAIL)
    {
      sprintf(errbuf, "Cannot open the dataset \"%s\". \n", utlbuf);
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get the (Compound) dataset type ID */
  /* ---------------------------------- */
  typeID = H5Dget_type(fwdID);
  if (typeID == FAIL)
    {
      sprintf(errbuf, "Cannot get the data type ID. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Create (Compound) datatype */
  /* -------------------------- */
  typeID = H5Tcreate(H5T_COMPOUND, sizeof(fwdptr));
  if (typeID == FAIL)
    {
      sprintf(errbuf, "Cannot create compound datatype. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Insert "BEGIN" data member */
  /* -------------------------- */
  status = H5Tinsert(typeID, "BEGIN", HOFFSET(fwdptr, begin), H5T_NATIVE_LLONG);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot insert \"BEGIN\" data member. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Insert "EXTENT" data member */
  /* --------------------------- */
  status = H5Tinsert(typeID, "EXTENT", HOFFSET(fwdptr, extent), H5T_NATIVE_LLONG);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot insert \"EXTENT\" data member. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Read BEGIN & EXTENT fields in first record */
  /* ------------------------------------------ */
  status = H5Dread(fwdID, typeID, H5S_ALL, H5S_ALL, H5P_DEFAULT, &fwd);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot read \"FWDPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Check if first record is not a garbage */
  /* -------------------------------------- */
  if (fwd.begin == FAIL || fwd.extent == FAIL)
    {
      /* Release the dataset ID */
      /* ---------------------- */
      status = H5Dclose(fwdID);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release dataset ID. \n");
	  H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
      /* Release the data type ID */
      /* ------------------------ */
      status = H5Tclose(typeID);  
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release datatype ID. \n");
	  H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
      return(FAIL);
    }


  /* Read BEGIN & EXTENT fields for all records */
  /* ------------------------------------------ */
  status = H5Dread(fwdID, typeID, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot read \"FWDPOINTER\" dataset. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Release the dataset ID */
  /* ---------------------- */
  status = H5Dclose(fwdID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release dataset ID. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  /* Release the data type ID */
  /* ------------------------ */
  status = H5Tclose(typeID);  
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release datatype ID. \n");
      H5Epush(__FILE__, "HE5_PTrdfwdptr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTupdatelevel                                                |
|                                                                             |
|  DESCRIPTION: Updates the specified fields and records of a level.          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  fieldlist      char                fieldlist to read (comma-separated)     |
|  nrec           hsize_t             Number of records to read               |
|  recs           hssize_t            array of record numbers to read         |
|  data           void                data buffer                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|  Date     Programmer    Description                                         |
|  ======   ============  =================================================   |
|  Nov 00   A.Muslimov    Changed the interface.                              |
|  Mar 01   A.Muslimov    Updated to reflect changes in the HDF5 library.     |
|  09/13/01 A.Muslimov    Added mutex "lock"/"unlock" calls.                  |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_PTupdatelevel(hid_t pointID, int level, char *fieldlist, hsize_t nrec,  hssize_t recs[], void *data)
{
  herr_t            status   = FAIL;/* routine return status variable    */

  int               nlevels  =  0;  /* Number of levels in point         */
  int               rank     =  1;  /* Rank of compound dataset          */
  int               j;              /* Loop index                        */

  long              idx      = FAIL;/* Point index                       */
  long              nfields  =  0;  /* Number of fields in fieldlist     */
  long              i;  		    /* Loop inde  x                      */
  long              dum      = FAIL;/* Dummy variable                    */

  hid_t             fid      = FAIL;/* HDF-EOS file ID                   */
  hid_t             gid      = FAIL;/* "POINTS" group ID                 */
  hid_t             dataID   = FAIL;/* Dataset ID                        */
  hid_t             dspace   = FAIL;/* Disk data space ID                */
  hid_t             mspace   = FAIL;/* Memory data space ID              */
  hid_t             typeID   = FAIL;/* Data type ID                      */

  hsize_t           nallrec  = 0;   /* Number of records in  Point       */
  hsize_t           dims[1]  = {0}; /* member dimensions in  dataset     */
  hsize_t           *coord   = NULL;/* Coordinates of elemnts to update  */
  
  size_t            datasize = 0;   /* Data size (bytes) of each record  */
  size_t            slen[HE5_HDFE_DIMBUFSIZE];  /* String length array   */

  char              utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer        */
  char              *pntr[HE5_HDFE_DIMBUFSIZE];	/* String pointer array  */
  char              errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */

  HE5_CmpDTSinfo    info;      /* Structure with the dataset information */

  HE5_LOCK;
  CHECKPOINTER(data);

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTupdatelevel", &fid, &gid, &idx);
  if (status == FAIL)
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if (nlevels == 0)
    {
      status = FAIL;
      sprintf(errbuf, "No Levels Defined for point.\n");
      H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  else if (nlevels < level)
    {
      status = FAIL;
      sprintf(errbuf, "Only %d levels defined for the point. \n", nlevels);
      H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
	
  /* Get dataset ID */
  /* -------------- */
  dataID = HE5_PTXPoint[idx].level[level].ID;


  /* Initialize slen[] array */
  /* ----------------------- */
  for (j = 0; j < HE5_HDFE_DIMBUFSIZE; j++)
    slen[ j ] = 0;

  /* Parse field list */
  /* ---------------- */
  nfields = HE5_EHparsestr(fieldlist, ',', pntr, slen);
	
  /* Check that all fields in list exist in level */
  /* -------------------------------------------- */
  for (i = 0; i < nfields; i++)
    {
      memmove(utlbuf, pntr[i], slen[i]);
      utlbuf[slen[i]] = 0;
      if (HE5_PTfexist(dataID, utlbuf, NULL, NULL, NULL, NULL) != 1)
	{
	  status = FAIL;
	  sprintf(errbuf, "Field: \"%s\" does not exist.\n",  utlbuf);
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf );
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }
  
  /* Check input records to update */
  /* ----------------------------- */
  if( nrec == 0 || recs == NULL )
    {
      /* Get neumber of records in level */
      /* ------------------------------- */
      nallrec = HE5_PTnrecs(pointID, level);
      if (nallrec == 0)
	{
	  sprintf(errbuf, "Cannot retrieve number of records in Point. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Call HE5_PTlevelinfo() */
      /* ---------------------- */
      status = HE5_PTlevelinfo(pointID, level, &info);
      if (status == FAIL)
        {
          sprintf(errbuf, "Cannot get the level information. \n");
          H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(FAIL);
        }
      /* Get the dataset size */
      /* -------------------- */
      datasize = info.datasize;
	  
      /* Release pointers to the name buffers */
      /* ------------------------------------ */
      for (j = 0; j < info.nfields; j++)
	{
	  if (info.fieldname[j] != NULL) 
	    free(info.fieldname[j]);
	}
	  

      /* Update all records in level */
      /* --------------------------- */
      status = HE5_PTwritelevel(pointID, level, &nallrec, &datasize, data);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write updated data to the level. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

    }
  else if( nrec > 0 && recs != NULL )
    {
      /* Get dataset space and datatype */
      /* ------------------------------ */
      dspace = H5Dget_space(dataID);
      if (dspace == FAIL)
	{
	  sprintf(errbuf, "Cannot get the data space ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  

      /* Get data type ID */
      /* ---------------- */
      typeID = H5Dget_type(dataID);
      if (typeID == FAIL)
	{
	  sprintf(errbuf, "Cannot get the data type ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);  
	  return(FAIL);
	}
	  
      dims[0] = nrec;
	  
      /* Create the memory data space */
      /* ---------------------------- */
      mspace = H5Screate_simple(rank, dims, NULL);
      if (mspace == FAIL)
	{
	  sprintf(errbuf, "Cannot create the data space.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  
	  return(FAIL);
	}
	  
      /* Allocate memory */
      /* --------------- */
      coord = (hsize_t *)calloc(nrec, sizeof(hsize_t));
      if (coord == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	   
      /* Loop through all records to update */
      /* ---------------------------------- */
      for (i = 0; i < nrec; i++)
	coord[i] = (hsize_t)recs[i];

      /* Select elements to update */
      /* ------------------------- */
      status = H5Sselect_elements(dspace, H5S_SELECT_SET, nrec, (const hsize_t *)coord);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot select elements.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(coord);
	  return(FAIL);
	}

      /* Update selected records */
      /* ----------------------- */
      status = H5Dwrite(dataID, typeID, mspace, dspace, H5P_DEFAULT, data);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write data to the dataset.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(coord);
	  return(FAIL);
	}

      /* Release memory */
      /* -------------- */
      free(coord);

      /* Release data space ID */
      /* --------------------- */
      status = H5Sclose(mspace);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Release data  type ID */
      /* --------------------- */
      status = H5Tclose(typeID);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data type ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Release data space ID */
      /* --------------------- */
      status = H5Sclose(dspace);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }


  /* Update Pointers to Previous Level */
  /* -------------------------------- */
  if (level > 0)
    {
      /* Store back linkage field in utlbuf */
      /* ---------------------------------- */
      status = HE5_PTbcklinkinfo(pointID, level, utlbuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot get backward linkage information.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Check whether back linkage field is in fieldlist */
      /* ------------------------------------------------ */
      dum = HE5_EHstrwithin(utlbuf, fieldlist, ',');
      if (dum != FAIL)
	{
	  /* Back pointers to previous level */
	  /* ------------------------------- */
	  status = HE5_PTwrbckptr(pointID, level);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write backward pointer.\n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Forward pointers from previous level */
	  /* ------------------------------------ */
	  status = HE5_PTwrfwdptr(pointID, level - 1);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write forward pointer.\n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	}
	  
    }
  
  
  /* Update Pointers to Next Level */
  /* ---------------------------- */
  if (level < HE5_PTnlevels(pointID) - 1 && dum != FAIL)
    {
      /* Store forward linkage field in utlbuf */
      /* ------------------------------------- */
      status = HE5_PTfwdlinkinfo(pointID, level, utlbuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot get forward linkage information.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Check whether forward linkage field is in fieldlist */
      /* --------------------------------------------------- */
      dum = HE5_EHstrwithin(utlbuf, fieldlist, ',');
      if (dum != FAIL)
	{
	  /* Back pointers from next level */
	  /* ----------------------------- */
	  status = HE5_PTwrbckptr(pointID, level + 1);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write backward pointer.\n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
		  
	  /* Forward pointers to next level */
	  /* ------------------------------ */
	  status = HE5_PTwrfwdptr(pointID, level);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write forward pointer.\n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}
	  
    }
	
 COMPLETION:
  HE5_UNLOCK;
  return(status);
}

/* 
*****************************************************************************
|                                                                           |
|          F  O  R  T  R  A  N  7 7     W  R  A  P  P  E  R  S              |
|                                                                           |
*****************************************************************************
*/



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTopenF   (FORTRAN wrapper)                                  |
|                                                                             |
|  DESCRIPTION: Opens or creates HDF file in order to create, read, or write  |
|                a grid.                                                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  FileID         int     None        HDF-EOS file ID                         |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|  filename       char*   None        Filename                                |
|  Flags          uintn               Access Code                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|  HDF5 Library Functions Called:                                             |
|                                                                             |
|             None                                                            |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|  3/12/02    A.Muslimov   Added more "Flags" options for core metadata.      |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_PTopenF(char *filename, uintn Flags)
{
  hid_t     fid     = FAIL;/* hdf5 type file ID     */ 

  int       FileID  = FAIL;/* Return file ID        */

  uintn     flags   = 9999;/* HDF5 file access code */
  
  char      *errbuf = NULL;/* Error message buffer  */
  

  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTopenF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }  
  

  /* Set up the file access flag */
  /* --------------------------- */
  if (Flags == HE5F_ACC_RDWR || Flags == HDF5_ACC_RDWR) flags = H5F_ACC_RDWR;
  else if (Flags == HE5F_ACC_RDONLY || Flags == HDF5_ACC_RDONLY) flags = H5F_ACC_RDONLY;
  else if (Flags == HE5F_ACC_TRUNC || Flags == HDF5_ACC_CREATE) flags = H5F_ACC_TRUNC;
  else
    {
      sprintf(errbuf, "File access flag is not supported. \n");
      H5Epush(__FILE__, "HE5_PTopenF", __LINE__, H5E_ARGS, H5E_UNSUPPORTED, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }  

  /* Call HE5_PTopen to perform file access */
  /* -------------------------------------- */
  fid = HE5_PTopen(filename,flags);
  if(fid == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_PTopen() from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_PTopenF", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }  


  FileID = (int)fid;

  free(errbuf);

  return(FileID);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTcreateF    (FORTRAN wrapper)                               |
|                                                                             |
|  DESCRIPTION: Creates a new point structure and returns point ID.           |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  pointID        int                 Point structure ID                      |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int     None        HDF-EOS file id                         |
|  pointname      char                Point structure name                    |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTcreateF(int FileID, char *pointname)
{
  int             PointID   = FAIL;/* point ID (return value)             */

  hid_t           pointID   = FAIL;/* HDF5 type point ID                  */
  hid_t           fid       = FAIL;/* HDF5 type file ID                   */

  char            *errbuf   = (char *)NULL; /* Buffer for error message   */



  errbuf  = (char *)calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char) );
  if (errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTcreateF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  fid = (hid_t)FileID;

  pointID = HE5_PTcreate(fid, pointname);
  if (pointID == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_PTcreate() from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_PTcreateF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  free(errbuf);

  PointID = (int)pointID;
  return(PointID);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTattachF    (FORTRAN wrapper)                               |
|                                                                             |
|  DESCRIPTION: Attaches to an existing grid within the file.                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  pointID        int                 point structure ID                      |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 HDF-EOS file ID                         |
|  pointname      char                point structure name                    |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTattachF(int FileID, char *pointname)
{
  int             PointID  = FAIL;	/* Return value of the Point ID    */

  hid_t           pointID  = FAIL;	/* HDF5 type Point  ID             */
  hid_t           fid      = FAIL;	/* HDF5 type file ID               */

  char            *errbuf = (char *)NULL;/* Buffer for error message  */


  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTattachF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  
  fid = (hid_t)FileID;

  pointID = HE5_PTattach(fid, pointname);
  if(pointID == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_PTattach() from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_PTattachF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  free(errbuf);
  
  PointID = (int)pointID;
  return(PointID);

}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTdetachF    (FORTRAN wrapper)                               |
|                                                                             |
|  DESCRIPTION: Detaches from grid interface and performs file housekeeping.  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int     None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  PointID        int     None        point structure ID                      |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTdetachF(int  PointID)   
{
  int             ret    = FAIL;/* (int) Return status variable   */

  herr_t          status = FAIL;/* routine return status variable */    
   
  hid_t           pointID = FAIL;/* HDF5 type point ID            */

  char            *errbuf = (char *)NULL;/*Error message buffer   */



  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTdetachF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  pointID = (hid_t)PointID;

  status = HE5_PTdetach(pointID);
  if(status == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_PTdetach() from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_PTdetachF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
	
  free(errbuf);

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTcloseF     (FORTRAN wrapper)                               |
|                                                                             |
|  DESCRIPTION: Closes file.                                                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int     None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|  FileID         int                 HDF-EOS type file ID                    |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTcloseF(int FileID)
{
  int               ret    = FAIL;/* (int) return status variable   */

  hid_t             fid    = FAIL;/* HDF5 type file ID              */

  herr_t            status = FAIL;/* routine return status variable */

  char              *errbuf = (char *)NULL;/*Error message buffer   */


  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTcloseF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  fid = (hid_t)FileID;

  /* Call HE5_PTclose to perform file close */
  /* -------------------------------------- */
  status = HE5_PTclose(fid);
  if(status == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_PTclose() from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_PTcloseF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

	
  free(errbuf);

  ret = (int)status;
  return(ret);
}

/* Definition routines */

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTdeflevelF           (FORTRAN wrapper)                      |
|                                                                             |
|  DESCRIPTION: Defines a level within the point data set.                    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  levelname      char                name of level                           |
|  rank           int                 Rank of Fields                          |
|  fieldlist      char                Fieldlist to read (comma-separated)     |
|  dim_sizes      void                Sizes of field dimensions               |
|  datasize       long                Datasize of compound structure          |
|  offset         long                Offsets of each of the fields           |
|  dtype          int                 Datatypes of each of the fields         |
|  array          int                 array flag identifier                   |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:     Metadata block needs to be developed                            |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTdeflevelF(int  pointID, const char *levelname, int rank[], char *fieldlist, long *dim_sizes, int dtype[], int array[])
{
  herr_t            status    = FAIL;/* routine return status variable */    

  int               ret       = FAIL;/* (int) Return status variable   */
  int               i, j, k   = 0;
 
  hid_t             PointID   = FAIL;

  HE5_CmpDTSinfo    dtsinfo;

  long              nfields         = FAIL;
  long              max_rank        = 0;
  long              nelements_field = 0;
  long              *datatype_size  = (long *)NULL;
  long              *datasize_field = (long *)NULL;
  long              *offset         = (long *)NULL;
  long              datasize        = 0;

  size_t            slen[HE5_HDFE_DIMBUFSIZE];

  char              *errbuf = (char *)NULL;     /* Error message buffer   */
  char              utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer         */
  char              *pntr[HE5_HDFE_DIMBUFSIZE];
 
     
  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTdeflevelF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Parse field list */
  /* ---------------- */
  nfields = HE5_EHparsestr(fieldlist, ',', pntr, slen);

  /* Set up level data structure */
  /* --------------------------- */
  dtsinfo.nfields = (int)nfields;

  /* Calculate maximum rank of all the fields */
  /* ---------------------------------------- */
  for( i = 0; i < nfields; i++ )
    {
      if( rank[i] > max_rank )
	max_rank = rank[i];
    }


  /*  Load dimension array (2-d fortran dimension array is in fortran order) */
  /* ----------------------------------------------------------------------- */
  for( i = 0; i < max_rank; i++ )
    {
      for( j = 0; j < nfields; j++ )
	{
	  k  =  i * (int)nfields + j;
	  dtsinfo.dims[j][i] = (size_t)dim_sizes[k];
	}
    }
  

  /* Store fieldname in level data structure */
  /* --------------------------------------- */
  for (i = 0; i < nfields; i++)
    {
      dtsinfo.fieldname[i] = (char *)calloc(HE5_HDFE_NAMBUFSIZE,sizeof(char));
      memmove(utlbuf, pntr[i], slen[i]);
      utlbuf[slen[i]]       = '\0';             
      memmove(dtsinfo.fieldname[i], utlbuf, slen[i]+1);  
    }

  datatype_size  = (long *)calloc(nfields,sizeof(long));
  datasize_field = (long *)calloc(nfields,sizeof(long));
  offset         = (long *)calloc(nfields,sizeof(long));

  for( i = 0; i < nfields; i++) 
    {
      switch( dtype[i] )
	{
	case HE5T_NATIVE_INT:
	  {	datatype_size[i] = sizeof(int); }
	  break;  
	case HE5T_NATIVE_FLOAT:
	  {	datatype_size[i] = sizeof(float); }
	  break;  
	case HE5T_NATIVE_DOUBLE:
	  {	datatype_size[i] = sizeof(double); }
	  break;  
	case HE5T_NATIVE_CHAR:
	  {	datatype_size[i] = sizeof(char); }
	  break;  
	case HE5T_NATIVE_LONG:
	  {	datatype_size[i] = sizeof(long); }
	  break;  
	default:
	  break;  
	}

      nelements_field = 0;
      for( j = 0; j < rank[i]; j++ )
	{
	  nelements_field = nelements_field + (long)dtsinfo.dims[i][j];
	}

      datasize_field[i] = nelements_field*datatype_size[i];

    }


  offset[0] = 0;
  datasize  = datasize_field[0];
  for( i = 1; i < nfields; i++) 
    {
      offset[i] = offset[i-1] + datasize_field[i-1];
      datasize  = datasize + datasize_field[i];
    }

  for( i = 0; i < nfields; i++ )
    {
      dtsinfo.array[i]  = array[i];
      dtsinfo.rank[i]   = rank[i];
      dtsinfo.offset[i] = (off_t)offset[i];    
      dtsinfo.dtype[i]  = HE5_EHconvdatatype(dtype[i]);
      /* 
	 if dtype[i] is HE5 numbertype, rather than H5 numbertype, then convert
	 it, otherwise use dtype[i] itself
      */
      if(dtsinfo.dtype[i] == FAIL)
	{
	  dtsinfo.dtype[i] = dtype[i];
	}
    }
  
  dtsinfo.datasize = (size_t)datasize;

  PointID = (hid_t)pointID;
  
  /* Call HE5_PTdeflevel() */
  /* --------------------- */ 
  status = HE5_PTdeflevel(PointID, levelname, &dtsinfo);
  if (status == FAIL )
    {
      sprintf(errbuf, "Call to HE5_PTdeflevel() failed. \n");
      H5Epush(__FILE__, "HE5_PTdeflevel", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  /* Free up space */
  /* ------------- */ 
  for (i = 0; i < nfields; i++ )
    free(dtsinfo.fieldname[i]);
  
  free(errbuf);
  free(datatype_size);
  free(datasize_field);
  free(offset);
  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTdeflinkageF   (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION: Defines link field between two (adjacent) levels.             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  parent         char                parent level name                       |
|  child          char                child level name                        |
|  linkfield      char                linkage field name                      |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date        Programmer   Description                                      |
|  =========   ============  ==============================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTdeflinkageF(int pointID, char *parent, char *child, char *linkfield)
{
  int       ret     = FAIL;/* (int) Return status variable   */

  herr_t    status  = FAIL;/* routine return status variable */    

  hid_t     PointID = FAIL;/* hdf5 type file ID              */ 
  
  char      *errbuf = NULL;/* Error message buffer           */


  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTdeflinkageF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }  
   
  PointID = (hid_t)pointID;

  /* Call HE5_PTdeflinkage                  */
  /* -------------------------------------- */
  status = HE5_PTdeflinkage(PointID, parent, child, linkfield);
  if(status == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_PTdeflinkageF() from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_PTdeflinkageF", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }  

  ret = (int)status;

  free(errbuf);

  return(ret);
}


/* I/O routines */

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadlevelF   (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION: Reads data from the specified fields and records of a level.  |
|               Reads only one field at a time                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|  fieldname      char                Name of field                           |
|  ntype          int                 HDF numbertype of field datatype        |
|  size           int                 Memory size (bytes) of field  being     |
|                                     read.                                   |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                Data buffer                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTreadlevelF(int pointID, int level, char *fieldname, int ntype, void *datbuf)
{
  int            ret     = FAIL;/* (int) return status variable   */

  herr_t         status  = FAIL;/* routine return status variable */

  hid_t          ptID    = FAIL;
  hid_t          dtype   = FAIL;/* HDF5 type  data type ID        */            

  char           *errbuf = (char *)NULL;/*Error message buffer    */


  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTcloseF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  dtype = HE5_EHconvdatatype(ntype);
  /* 
     if ntype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use ntype itself
  */
  if(dtype == FAIL)
    {
      dtype = ntype;
    }

  ptID           = (hid_t)pointID;

  /* Call HE5_PTreadlevel_f to read data */
  /* ----------------------------------- */
  status = HE5_PTreadlevel_f(ptID, level, fieldname, dtype, datbuf);  
  if (status == FAIL)
    {
      sprintf(errbuf, "Reading data failed.\n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  free(errbuf);

  ret = (int)status;
  return(ret);

}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadlevel_f                                                |
|                                                                             |
|  DESCRIPTION: Reads data from the specified fields and records of a level.  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  fieldname      char                Name of field                           |
|  ntype          int                 HDF numbertype of field datatype        |
|  dtype          hid_t               datatype ID of field                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf                             Data buffer                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t   
HE5_PTreadlevel_f(hid_t pointID, int level, char *fieldname, hid_t dtype, void *datbuf)  
{
  herr_t          status    = FAIL;/* routine return status variable  */

  int             nlevels   =  0;  /* Number of levels in point       */
  int             i, j;            /* Loop index                      */
  int             ii        = 0;
  int             nflds     = FAIL;
  int             result    = 99;
  int             field_size    = 0;
  int             rank          = 0;

  size_t          datatype_size = 0;

  hid_t           fid       = FAIL;/* HDF-EOS file ID                 */
  hid_t           gid       = FAIL;/* POINTS group ID                 */
  hid_t           dataID    = FAIL;/* dataset ID                      */
  hid_t           xfer_list = FAIL;/* Property list ID                */
  hid_t           typeID    = FAIL;/* compound datatype ID            */
  hid_t           arr_id    = FAIL;/* Array datatype ID               */
  hid_t           mtype     = FAIL;/* Memory data type ID                 */

  hsize_t         dims[HE5_DTSETRANKMAX];/* Array of dimension sizes  */
  hsize_t         datasize  = 0;/* Size (bytes) of a dataset to read  */

  long            idx       = FAIL;/* Point index                     */

  HE5_CmpDTSinfo  dtsinfo;/* Structure to read information about level*/    

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */


  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTreadlevel_f", &fid, &gid, &idx);
  if (status == FAIL)
    {
      sprintf(errbuf, "Checking for the point  ID failed.\n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Initialize dims[] array */
  /* ----------------------- */
  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    dims[ i ] = 0;

  /* Get number of levels in point */
  /* ----------------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if (nlevels == 0)
    {
      status = FAIL;
      sprintf(errbuf, "No Levels Defined for point ID: %d\n", pointID);
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  else if (nlevels < level)
    {
      /* Report error if level # to large */
      /* -------------------------------- */
      status = FAIL;
      sprintf(errbuf, "Only %d levels Defined for point ID: %d\n", nlevels, pointID);
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_FILE, H5E_SEEKERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get level dataset ID */
  /* ------------------ */
  dataID = HE5_PTXPoint[idx].level[level].ID;

  /* Create property list */
  /* -------------------- */
  xfer_list = H5Pcreate(H5P_DATASET_XFER);
  if (xfer_list == FAIL)
    {
      sprintf(errbuf, "Cannot create the property list. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Call HE5_PTlevelinfo() */
  /* ---------------------- */
  status = HE5_PTlevelinfo(pointID, level, &dtsinfo);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot get the level information. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Get the dataset size */
  /* -------------------- */
  datasize = (hsize_t)(dtsinfo.datasize);   
  
  /* Set the buffer */
  /* -------------- */
  status = H5Pset_buffer(xfer_list, datasize, NULL, NULL);  
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot set the read buffer. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
   
  nflds = dtsinfo.nfields;

  /* Check that field does exist */
  /* --------------------------- */
  if (HE5_PTfexist(dataID, fieldname, NULL, NULL, NULL, NULL) != 1)
    {
      status = FAIL;
      sprintf(errbuf, "Field: \"%s\" does not exist.\n",  fieldname);
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Find index of fieldname in the compound datatype */
  /* ------------------------------------------------ */
  for( i = 0; i < nflds; i++ )
    {
      result  = strcmp(fieldname,dtsinfo.fieldname[i]);
      if( result == 0 ) 
	{
	  ii = i;
	  result = 99;
	}    
    } 
  i = ii;

  /* Calculate the data size of the field */
  /* ------------------------------------ */

  rank = dtsinfo.rank[i];

  field_size = 1;
  for( j=0; j < rank; j++ )
    field_size = (int)(field_size*dtsinfo.dims[i][j]);


  mtype = HE5_EHdtype2mtype(dtype);

  datatype_size     = H5Tget_size(mtype);

  field_size = field_size*((int)datatype_size);

  /* Create Compound datatype */
  /* ------------------------ */
  typeID = H5Tcreate(H5T_COMPOUND, field_size);   /* field_size is size of the entire field  */
  if (typeID == FAIL)
    {
      status = FAIL;
      sprintf(errbuf, "Cannot create the compound data type.\n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Add members of a compound datatype to read */
  /* ------------------------------------------ */

  /* Check if the member is an array */
  /* ------------------------------- */
  if ( dtsinfo.array[i] == TRUE)
    {
		  
      for (j = 0; j < dtsinfo.rank[i]; j++)
	dims[j] = (hsize_t)(dtsinfo.dims[i][j]);


      /* Create array datatype */
      /* --------------------- */
      arr_id = H5Tarray_create(dtype, dtsinfo.rank[i], (const hsize_t *)dims, NULL);  
      if (arr_id == FAIL)
	{
	  sprintf(errbuf, "Cannot create \"array\" datatype. \n");
	  H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Add array field */
      /* --------------- */
      status = H5Tinsert(typeID, dtsinfo.fieldname[i], 0, arr_id);                  
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot insert \"array\" element. \n");
	  H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      /* Release the data type ID */
      /* ------------------------ */
      status = H5Tclose(arr_id);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot release data type ID. \n");
	  H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }
  else 
    {
      status = H5Tinsert(typeID, dtsinfo.fieldname[i], 0,dtype);
      /* Add atomic field */
      /* ---------------- */
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot insert \"array\" element. \n");
	  H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

    }
 
  /* Release pointers to the name buffers */
  /* ------------------------------------ */

  for (i = 0; i < dtsinfo.nfields; i++)
    {
      if (dtsinfo.fieldname[i] != NULL) 
	free(dtsinfo.fieldname[i]);
    }

  /* Read out the level fields */
  /* ------------------------- */
  status = H5Dread(dataID, typeID, H5S_ALL, H5S_ALL, xfer_list, datbuf);      
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot read the level dataset. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Release the property list ID */
  /* ---------------------------- */
  status = H5Pclose(xfer_list);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the property list ID. \n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Release the datatype ID */
  /* ----------------------- */
  status = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the data type ID.\n");
      H5Epush(__FILE__, "HE5_PTreadlevel_f", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  return (status);
}

 
/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwritelevelF   (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION: Writes full records to a level.                               |
|               Writes only one field at a time.                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|  fieldname      char                Name of field                           |
|  count[]        long                Number of records to write              |
|  ntype          int                 HDF numbertype of field datatype        |
|  data           void                data buffer                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|      None                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date        Programmer    Description                                     |
|  ==========   ============  =============================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTwritelevelF(int pointID, int level, long count[], char *fieldname, int ntype, void *data)
{
  herr_t            status = FAIL;/* routine return status variable */

  int               ret    = FAIL;/* (int) return status variable   */

  hid_t             ptID   = FAIL;
  hid_t             dtype  = FAIL;/* HDF5 type  data type ID        */            

  hsize_t           count_t1[1] = {0};

  char              *errbuf = (char *)NULL;/*Error message buffer   */


  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTcloseF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  dtype = HE5_EHconvdatatype(ntype);
  /* 
     if ntype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use ntype itself
  */
  if(dtype == FAIL)
    {
      dtype = ntype;
    }

  ptID           = (hid_t)pointID;
  count_t1[0]    = (hsize_t)count[0]; 

  /*   WRITE data   */
  /*   ----------   */
  status = HE5_PTwritelevel_f(ptID, level, count_t1, fieldname, ntype, dtype, data);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot write updated data to the level. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
 
  free(errbuf);

  ret = (int)status;
  return(ret);

}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwritelevel_f                                               |
|                                                                             |
|  DESCRIPTION: Writes full records to a level.                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  count[]        hsize_t             Number of records to write              |
|  fieldname      char                Name of field                           |
|  ntype          int                 HDF numbertype of field datatype        |
|  dtype          hid_t               datatype ID of field                    |
|  data           void                data buffer                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|      None                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date        Programmer    Description                                     |
|  ==========   ============  =============================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t 
HE5_PTwritelevel_f(hid_t pointID, int level, hsize_t count[], char *fieldname, int ntype, hid_t dtype, void *data)
{
  herr_t         status    = FAIL;/* routine return status variable   */

  int            nlevels   = FAIL;/* Number of levels in point        */
  int            i,j;             /* Loop index                       */
  int            ii        = 0;
  int            nflds     = 0;
  int            result    = 99;
  int            datatype_size = 0;
  int            field_size    = 0;
  int            rank          = 0;
  int            err_xfer      = FALSE;

  hid_t          fid       = FAIL;/* HDF-EOS file ID                  */
  hid_t          gid       = FAIL;/* POINTS group ID                  */
  hid_t          dataID    = FAIL;/* dataset ID                       */
  hid_t          typeID    = FAIL;/* data type ID                     */
  hid_t          xfer_list = FAIL;/* Property list  ID                */ 

  hid_t          arr_id    = FAIL;/* Array datatype ID                  */

  long           idx       = FAIL;/* Point index                      */

  size_t         orig_size = 0;

  hssize_t       nrec      = FAIL;/* Current number of records        */
  hsize_t        dims[HE5_DTSETRANKMAX];/* Array of dimension sizes   */
  hsize_t        orig_size_t = 0;

  char           errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */

  HE5_CmpDTSinfo infolevel;   /*  Level information data structure  */


  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTwritelevel_f", &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for file ID failed. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get the current number of records */
  /* --------------------------------- */
  status = HE5_PTwritesetup(pointID, level, &nrec);
  if(status == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the current number of records\n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if(nlevels == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the number of levels\n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Report error if no levels (dataset) defined */
  /* ------------------------------------------- */
  if (nlevels == 0)
    {
      status = FAIL;
      sprintf(errbuf, "No Levels Defined for point ID: %d\n", pointID);
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  else if (nlevels < level)
    {
      /* Report error if level # to large */
      /* -------------------------------- */
      status = FAIL;
      sprintf(errbuf, "Only %d levels Defined for point ID: %d\n", nlevels, pointID);
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get the level dataset ID  */
  /* ------------------------- */
  dataID = HE5_PTXPoint[idx].level[level].ID;

  /* Extend the dataset size */
  /* ----------------------- */
  status = H5Dextend(dataID, count);   
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot extend the  dataset. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get the datatype ID */
  /* ------------------- */
  typeID = H5Dget_type(dataID);   /*  need to get the size of the orig compound datatype  */

  orig_size = H5Tget_size(typeID);

  status = H5Tclose(typeID);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release data type ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    } 

  status = HE5_PTlevelinfo(pointID, level, &infolevel);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot get the level information. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Check that field does exist */
  /* --------------------------- */

  if (HE5_PTfexist(dataID, fieldname, NULL, NULL, NULL, NULL) != 1)
    {
      status = FAIL;
      sprintf(errbuf, "Field: \"%s\" does not exist.\n",  fieldname);
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Find index of fieldname in the compound datatype */
  /* ------------------------------------------------ */
  nflds = infolevel.nfields;
  for( i = 0; i < nflds; i++ )
    {
      result  = strcmp(fieldname,infolevel.fieldname[i]);
      if( result == 0 ) 
	{
	  ii = i;
	  result = 99;
	}    
    }
 
  i = ii;


  /* Calculate the data size of the field */
  /* ------------------------------------ */
  rank = infolevel.rank[i];

  field_size = 1;
  for( j=0; j < rank; j++ )
    field_size = (int)(field_size*infolevel.dims[i][j]);

  switch( ntype )
    {
    case HE5T_NATIVE_INT:
      {	datatype_size = sizeof(int); }
      break;  
    case HE5T_NATIVE_FLOAT:
      {	datatype_size = sizeof(float); }
      break;  
    case HE5T_NATIVE_DOUBLE:
      {	datatype_size = sizeof(double); }
      break;  
    case HE5T_NATIVE_CHAR:
      {	datatype_size = sizeof(char); }
      break;  
    case HE5T_NATIVE_LONG:
      {	datatype_size = sizeof(long); }
      break;  
    default:
      break;  
    }

  field_size = field_size*datatype_size;

  /* Create Compound datatype */
  /* ------------------------ */
  typeID = H5Tcreate(H5T_COMPOUND, field_size);     /*  field_size is size of the entire field  */
  if (typeID == FAIL)
    {
      sprintf(errbuf, "Cannot get data type ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Add members to the compound data type */
  /* ------------------------------------- */

  /* Check if the field is an array */
  /* ------------------------------ */
  if (infolevel.array[i] == TRUE)
    {
      for (j = 0; j < infolevel.rank[i]; j++)
	dims[j] = (hsize_t)infolevel.dims[i][j];
		  
      /* Create array datatypes */
      /* ---------------------- */
      arr_id = H5Tarray_create(dtype, infolevel.rank[i], (const hsize_t *)dims, NULL);
      if (arr_id == FAIL)
	{
	  sprintf(errbuf, "Cannot create \"array\" datatype. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Add array field */
      /* --------------- */
      status = H5Tinsert(typeID,infolevel.fieldname[i], 0, arr_id);    
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot insert \"array\" element. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      /* Release the data type ID */
      /* ------------------------ */
      status = H5Tclose(arr_id);
      if (status == FAIL )
	{
	  sprintf(errbuf, "Cannot release data type ID. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	} 
    }
  else 
    {
      /* Add atomic field */
      /* ---------------- */
      status = H5Tinsert(typeID,infolevel.fieldname[i], 0,dtype);      
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot insert \"array\" element. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }


  /* Release pointers to the name buffers */
  /* ------------------------------------ */
  for (j = 0; j < infolevel.nfields; j++)
    {
      if (infolevel.fieldname[j] != NULL) 
	free(infolevel.fieldname[j]);
    }
	  
  /* Create the property list */
  /* ------------------------ */
  xfer_list = H5Pcreate(H5P_DATASET_XFER);
  if (xfer_list == FAIL)
    {
      sprintf(errbuf, "Cannot create the property list ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_PLIST, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Set type conversion and background buffer */
  /* ----------------------------------------- */
  orig_size_t = (hsize_t)orig_size;
  status = H5Pset_buffer(xfer_list, orig_size_t, NULL, NULL);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot set type conversion and background buffer. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Preserve partially initialized fields to the compound datatypes */
  /* --------------------------------------------------------------- */
  err_xfer = H5Pget_preserve(xfer_list);
  if (err_xfer == FAIL)
    {
      sprintf(errbuf, "Cannot get X-fer property list ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  err_xfer = H5Pset_preserve(xfer_list, 1);
  if (err_xfer == FAIL)
    {
      sprintf(errbuf, "Cannot set X-fer property list ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  err_xfer = H5Pget_preserve(xfer_list);
  if (err_xfer == FAIL)
    {
      sprintf(errbuf, "Cannot get X-fer property list ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Write data to the dataset */
  /* ------------------------- */
  status = H5Dwrite(dataID, typeID, H5S_ALL, H5S_ALL, xfer_list, data);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot write data to the dataset. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  
  /* Release data type ID */
  /* -------------------- */
  status = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the data type ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Release property list ID */
  /* ------------------------ */
  status = H5Pclose(xfer_list);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the property list ID. \n");
      H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  return (status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwrbckptrF   (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION: Writes back pointer records                                   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level index (0 - based)                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_PTwrbckptrF(int pointID, int level)
{
  int             ret    = FAIL;/* (int) Return status variable   */

  herr_t          status = FAIL;/* routine return status variable */    
   
  hid_t           PointID = FAIL;/* HDF5 type point ID            */

  char            *errbuf = (char *)NULL;/*Error message buffer   */



  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTwrbckptrF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  PointID = (hid_t)pointID;

  status = HE5_PTwrbckptr(PointID, level);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot write back pointer records. \n");
      H5Epush(__FILE__, "HE5_PTwrbckptrF", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  free(errbuf);

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwrfwdptrF   (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION: Write foward pointer records                                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level index (0 - based)                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_PTwrfwdptrF(int pointID, int level)
{
  int             ret    = FAIL;/* (int) Return status variable   */

  herr_t          status = FAIL;/* routine return status variable */    
   
  hid_t           PointID = FAIL;/* HDF5 type point ID            */

  char            *errbuf = (char *)NULL;/*Error message buffer   */



  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTwrfwdptrF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  PointID = (hid_t)pointID;

  status = HE5_PTwrfwdptr(PointID, level - 1);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot write back pointer records. \n");
      H5Epush(__FILE__, "HE5_PTwrfwdptrF", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  free(errbuf);

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTupdatelevelF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION: Updates the specified field and records of a level.           |
|               Only updates one field at a time.                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|  fieldlist      char                fieldlist (of one field)                |
|  nrec           long                number of records to update             |
|  recs[]         long                array of record numbers to update       |
|  ntype          int                 HDF numbertype of field datatype        |
|  data           void                data buffer                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|  Date     Programmer    Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTupdatelevelF(int pointID, int level, char *fieldlist, long nrec,  long recs[], int ntype, void *data)
{
  herr_t            status  = FAIL;

  hid_t             PointID = FAIL;
  hid_t             dtype   = FAIL;     /* HDF5 type  data type ID        */            

  hsize_t           nrec_t  = 0;

  hssize_t          *recs_t;

  int               i;
  int               ret     = FAIL;

  char              *errbuf = (char *)NULL;/*Error message buffer   */



  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTcloseF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(ret);
    }

  dtype   = HE5_EHconvdatatype(ntype);
  /* 
     if ntype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use ntype itself
  */
  if(dtype == FAIL)
    {
      dtype = ntype;
    }

  PointID = (hid_t)pointID;
  nrec_t  = (hsize_t)nrec;

  recs_t = (hssize_t *)calloc(nrec, sizeof(hssize_t));

  for( i = 0; i < nrec; i++ )
    recs_t[i] = (hssize_t)recs[i];
  

  /* Call HE5_PTupdatelevel_f() */
  /* -------------------------- */ 
  status = HE5_PTupdatelevel_f(PointID, level, fieldlist, nrec_t, recs_t, ntype, dtype, data);
  if (status == FAIL)
    {
      sprintf(errbuf, "Trouble calling HE5_PTupdatelevel_f(). \n");
      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      if (recs_t != NULL) free(recs_t);
    }
  
  free(errbuf);
  if (recs_t != NULL) free(recs_t);

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTupdatelevel_f                                              |
|                                                                             |
|  DESCRIPTION: Updates the specified field and records of a level.           |
|               Only updates one field at a time.                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|  fieldlist      char                fieldlist (of one field)                |
|  nrec           hsize_t             Number of records to update             |
|  recs[]         hssize_t            array of record numbers to update       |
|  ntype          int                 HDF numbertype of field datatype        |
|  dtype          hid_t               datatype ID of field                    |
|  data           void                data buffer                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|  Date     Programmer    Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t  
HE5_PTupdatelevel_f(hid_t pointID, int level, char *fieldlist, hsize_t nrec,  hssize_t recs[], int ntype, hid_t dtype, void *data)
{
  herr_t            status    = FAIL;/* routine return status variable    */

  int               j;               /* Loop index                        */
  int               nlevels   =  0;  /* Number of levels in point         */
  int               rank      =  1;  /* Rank of compound dataset          */
  int               ii        =  0;
  int               nflds     =  0;
  int               result    = 99;
  int               datatype_size = 0;
  int               field_size    = 0;
  int               field_rank    = 0;
  int               err_xfer  = FALSE;

  long              i;  	     /* Loop index                        */
  long              idx       = FAIL;/* Point index                       */
  long              nfields   =  0;  /* Number of fields in fieldlist     */
  long              dum       = FAIL;/* Dummy variable                    */

  hid_t             fid       = FAIL;/* HDF-EOS file ID                   */
  hid_t             gid       = FAIL;/* "POINTS" group ID                 */
  hid_t             dataID    = FAIL;/* Dataset ID                        */
  hid_t             dspace    = FAIL;/* Disk data space ID                */
  hid_t             mspace    = FAIL;/* Memory data space ID              */
  hid_t             typeID    = FAIL;/* Data type ID                      */
  hid_t             arr_id    = FAIL;/* Array datatype ID                 */
  hid_t             xfer_list = FAIL;/* Property list  ID                 */ 
  
  hsize_t           nallrec   = 0;   /* Number of records in  Point       */
  hsize_t           dims_1[1] = {0}; /* member dimensions in  dataset     */
  hsize_t           *coord    = NULL;/* Coordinates of elemnts to update  */
  hsize_t           dims[HE5_DTSETRANKMAX];/* Array of dimension sizes    */
  hsize_t           orig_size_t = 0;
  
  size_t            orig_size = 0;
  size_t            slen[HE5_HDFE_DIMBUFSIZE];  /* String length array   */

  char              *fieldname = (char *)NULL;  /* Field name buffer     */       
  char              utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer        */
  char              *pntr[HE5_HDFE_DIMBUFSIZE];	/* String pointer array  */
  char              errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */

  HE5_CmpDTSinfo    info;      /* Structure with the dataset information */
  HE5_CmpDTSinfo    infolevel; /*  Level information data structure  */


  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTupdatelevel_f", &fid, &gid, &idx);
  if (status == FAIL)
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(pointID);
  if (nlevels == 0)
    {
      status = FAIL;
      sprintf(errbuf, "No Levels Defined for point.\n");
      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  else if (nlevels < level)
    {
      status = FAIL;
      sprintf(errbuf, "Only %d levels defined for the point. \n", nlevels);
      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
	
  /* Get dataset ID */
  /* -------------- */
  dataID = HE5_PTXPoint[idx].level[level].ID;

  /* Initialize slen[] array */
  /* ----------------------- */
  for (j = 0; j < HE5_HDFE_DIMBUFSIZE; j++)
    slen[ j ] = 0;
  
  /* Parse field list */
  /* ---------------- */
  nfields = HE5_EHparsestr(fieldlist, ',', pntr, slen);
  if( nfields != 1 ) 
    { 
      status = FAIL;
      sprintf(errbuf, "Should be only one field specified in fieldlist...nfields is not equal to 1 \n");
      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  else
    {
      fieldname = (char *)calloc(HE5_HDFE_NAMBUFSIZE,sizeof(char));
      memmove(utlbuf, pntr[0], slen[0]);
      utlbuf[slen[0]]       = '\0';             
      memmove(fieldname, utlbuf, slen[0]+1);  
    }

  /* Check that all fields in list exist in level */
  /* -------------------------------------------- */
  for (i = 0; i < nfields; i++)
    {
      memmove(utlbuf, pntr[i], slen[i]);
      utlbuf[slen[i]] = 0;
      if (HE5_PTfexist(dataID, utlbuf, NULL, NULL, NULL, NULL) != 1)
	{
	  status = FAIL;
	  sprintf(errbuf, "Field: \"%s\" does not exist.\n",  utlbuf);
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf );
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
	  return(status);
	}
    }
  
  /* Check input records to update */
  /* ----------------------------- */
  if( nrec == 0 || recs == NULL )
    {
      /* Get neumber of records in level */
      /* ------------------------------- */
      nallrec = HE5_PTnrecs(pointID, level);
      if (nallrec == 0)
	{
	  sprintf(errbuf, "Cannot retrieve number of records in Point. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
	  return(FAIL);
	}

      /* Call HE5_PTlevelinfo() */
      /* ---------------------- */
      status = HE5_PTlevelinfo(pointID, level, &info);
      if (status == FAIL)
        {
          sprintf(errbuf, "Cannot get the level information. \n");
          H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
          return(FAIL);
        }

      /* Release pointers to the name buffers */
      /* ------------------------------------ */
      for (j = 0; j < info.nfields; j++)
	{
	  if (info.fieldname[j] != NULL) 
	    free(info.fieldname[j]);
	}
	  
      /* Update all records in level */
      /* --------------------------- */

      /* USE "writelevel_f" routine which works on ONE FIELD at a time */
      /* ------------------------------------------------------------- */
      status = HE5_PTwritelevel_f(pointID, level, &nallrec, fieldname, ntype, dtype, data);   
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write updated data to the level. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
	  return(FAIL);
	}
	  
      if (fieldname != NULL) free(fieldname);

    }
  else if( nrec > 0 && recs != NULL )
    {
      /* Get dataset space and datatype */
      /* ------------------------------ */
      dspace = H5Dget_space(dataID);
      if (dspace == FAIL)
	{
	  sprintf(errbuf, "Cannot get the data space ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
	  return(FAIL);
	}
	  

      /* Get data type ID */
      /* ---------------- */
      typeID = H5Dget_type(dataID);
      if (typeID == FAIL)
	{
	  sprintf(errbuf, "Cannot get the data type ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
	  return(FAIL);
	}

      /* Get the size of the original compound datatype */
      /* ---------------------------------------------- */
      orig_size = H5Tget_size(typeID);      
      status    = H5Tclose(typeID);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data type ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
	  return(FAIL);
	}

      status = HE5_PTlevelinfo(pointID, level, &infolevel);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot get the level information. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
	  return(FAIL);
	}

      /* Check that field does exist */
      /* --------------------------- */
      if (HE5_PTfexist(dataID, fieldname, NULL, NULL, NULL, NULL) != 1)
	{
	  status = FAIL;
	  sprintf(errbuf, "Field: \"%s\" does not exist.\n",  fieldname);
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf );
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (fieldname != NULL) free(fieldname);
	  return(status);
	}

      /* Find index of fieldname in the compound datatype */
      /* ------------------------------------------------ */
      nflds = infolevel.nfields;
      for( i = 0; i < nflds; i++ )
	{
	  result  = strcmp(fieldname,infolevel.fieldname[i]);
	  if( result == 0 ) 
	    {
	      ii = (int)i;
	      result = 99;
	    }    
	} 
      i = ii;

      /* Calculate the data size of the field */
      /* ------------------------------------ */
      field_rank = infolevel.rank[i];

      field_size = 1;
      for( j=0; j < field_rank; j++ )
	field_size = (int)(field_size*infolevel.dims[i][j]);

      switch( ntype )
	{
	case HE5T_NATIVE_INT:
	  {	datatype_size = sizeof(int); }
	  break;  
	case HE5T_NATIVE_FLOAT:
	  {	datatype_size = sizeof(float); }
	  break;  
	case HE5T_NATIVE_DOUBLE:
	  {	datatype_size = sizeof(double); }
	  break;  
	case HE5T_NATIVE_CHAR:
	  {	datatype_size = sizeof(char); }
	  break;  
	case HE5T_NATIVE_LONG:
	  {	datatype_size = sizeof(long); }
	  break;  
	default:
	  break;  
	}

      field_size = field_size*datatype_size;

      /* Create Compound datatype */
      /* ------------------------ */
      typeID = H5Tcreate(H5T_COMPOUND, field_size);     /*  field_size is size of the entire field  */
      if (typeID == FAIL)
	{
	  sprintf(errbuf, "Cannot get data type ID. \n");
	  H5Epush(__FILE__, "HE5_PTwritelevel_f", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Free previously allocated space */
      /* ------------------------------- */
      if (fieldname != NULL) free(fieldname);
	  
      /* Add members to the compound data type */
      /* ------------------------------------- */
	  
      /* Check if the field is an array */
      /* ------------------------------ */
      if (infolevel.array[i] == TRUE)
	{
	  for (j = 0; j < infolevel.rank[i]; j++)
	    dims[j] = (hsize_t)infolevel.dims[i][j];

	  /* Create array datatypes */
	  /* ---------------------- */
	  arr_id = H5Tarray_create(dtype, infolevel.rank[i], (const hsize_t *)dims, NULL);   
	  if (arr_id == FAIL)
	    {
	      sprintf(errbuf, "Cannot create \"array\" datatype. \n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Add array field */
	  /* --------------- */
	  status = H5Tinsert(typeID,infolevel.fieldname[i], 0, arr_id);  
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot insert \"array\" element. \n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Release the data type ID */
	  /* ------------------------ */
	  status = H5Tclose(arr_id);
	  if (status == FAIL )
	    {
	      sprintf(errbuf, "Cannot release data type ID. \n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    } 

	}
      else 
	{
	  /* Add atomic field */
	  /* ---------------- */
	  status = H5Tinsert(typeID,infolevel.fieldname[i], 0,dtype);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot insert \"array\" element. \n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}
	  
      /* Release pointers to the name buffers */
      /* ------------------------------------ */
      for (j = 0; j < infolevel.nfields; j++)
	{
	  if (infolevel.fieldname[j] != NULL) 
	    free(infolevel.fieldname[j]);
	}

      /* Preserve partially initialized fields of the compound datatypes */
      /* --------------------------------------------------------------- */

      /* Create the property list */
      /* ------------------------ */

      xfer_list = H5Pcreate(H5P_DATASET_XFER);
      if (xfer_list == FAIL)
	{
	  sprintf(errbuf, "Cannot create the property list ID. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_PLIST, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Set type conversion and background buffer */
      /* ----------------------------------------- */
      orig_size_t  = (hsize_t)orig_size;
      status = H5Pset_buffer(xfer_list, orig_size_t, NULL, NULL);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot set type conversion and background buffer. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      err_xfer = H5Pget_preserve(xfer_list);
      if (err_xfer == FAIL)
	{
	  sprintf(errbuf, "Cannot get X-fer property list ID. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      err_xfer = H5Pset_preserve(xfer_list, 1);
      if (err_xfer == FAIL)
	{
	  sprintf(errbuf, "Cannot set X-fer property list ID. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      err_xfer = H5Pget_preserve(xfer_list);
      if (err_xfer == FAIL)
	{
	  sprintf(errbuf, "Cannot get X-fer property list ID. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      dims_1[0] = nrec;
	  
      /* Create the memory data space */
      /* ---------------------------- */
      mspace = H5Screate_simple(rank, dims_1, NULL);
      if (mspace == FAIL)
	{
	  sprintf(errbuf, "Cannot create the data space.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  
	  return(FAIL);
	}

	  
      /* Allocate memory */
      /* --------------- */
      coord = (hsize_t *)calloc(nrec, sizeof(hsize_t));
      if (coord == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory. \n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	   
      /* Loop through all records to update */
      /* ---------------------------------- */
      for (i = 0; i < nrec; i++)
	coord[i] = (hsize_t)recs[i];

      /* Select elements to update */
      /* ------------------------- */
      status = H5Sselect_elements(dspace, H5S_SELECT_SET, nrec, (const hsize_t *)coord);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot select elements.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(coord);
	  return(FAIL);
	}

      /* Update selected records */
      /* ----------------------- */
      status = H5Dwrite(dataID, typeID, mspace, dspace, xfer_list, data);     
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write data to the dataset.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(coord);
	  return(FAIL);
	}

      /* Release memory */
      /* -------------- */
      free(coord);

      /* Release data space ID */
      /* --------------------- */
      status = H5Sclose(mspace);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Release data  type ID */
      /* --------------------- */
      status = H5Tclose(typeID);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data type ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Release data space ID */
      /* --------------------- */
      status = H5Sclose(dspace);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }


  /* Update Pointers to Previous Level */
  /* -------------------------------- */
  if (level > 0)
    {
      /* Store back linkage field in utlbuf */
      /* ---------------------------------- */
      status = HE5_PTbcklinkinfo(pointID, level, utlbuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot get backward linkage information.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Check whether back linkage field is in fieldlist */
      /* ------------------------------------------------ */
      dum = HE5_EHstrwithin(utlbuf, fieldlist, ',');
      if (dum != FAIL)
	{
	  /* Back pointers to previous level */
	  /* ------------------------------- */
	  status = HE5_PTwrbckptr(pointID, level);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write backward pointer.\n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Forward pointers from previous level */
	  /* ------------------------------------ */
	  status = HE5_PTwrfwdptr(pointID, level - 1);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write forward pointer.\n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	}
	  
    }
  
  
  /* Update Pointers to Next Level */
  /* ---------------------------- */
  if (level < HE5_PTnlevels(pointID) - 1 && dum != FAIL)
    {
      /* Store forward linkage field in utlbuf */
      /* ------------------------------------- */
      status = HE5_PTfwdlinkinfo(pointID, level, utlbuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot get forward linkage information.\n");
	  H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Check whether forward linkage field is in fieldlist */
      /* --------------------------------------------------- */
      dum = HE5_EHstrwithin(utlbuf, fieldlist, ',');
      if (dum != FAIL)
	{
	  /* Back pointers from next level */
	  /* ----------------------------- */
	  status = HE5_PTwrbckptr(pointID, level + 1);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write backward pointer.\n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
		  
	  /* Forward pointers to next level */
	  /* ------------------------------ */
	  status = HE5_PTwrfwdptr(pointID, level);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write forward pointer.\n");
	      H5Epush(__FILE__, "HE5_PTupdatelevel_f", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}
	  
    }
	
  return(status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwriteattrF   (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION:                                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  attrname       char                attribute name                          |
|  ntype          int                 attribute HDF numbertype                |
|  fortcount      long                Number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTwriteattrF(int pointID, const char *attrname, int ntype, long fortcount[], void *datbuf)
{

  int       ret     = FAIL;/* int return status variable                 */
  int       rank    = 1;   /* Note: It is assumed that fortcout has just */
  /* one element to agree with EHattr function  */
  int	    i;             /* Loop index                                 */

  hid_t     PointID    = FAIL;/* HDF-EOS Swath ID                        */
  hid_t     numbertype = FAIL;/* HDF5 data type ID                       */ 

  herr_t    status     = FAIL;/* routine return status variable          */
  
  hsize_t   *count = (hsize_t *)NULL; /* Pointer to count array (C order)*/
    
  char      *errbuf = (char *)NULL;   /* error message buffer            */
  char      *tempbuf = (char *)NULL;  /* temp buffer                     */
 

  numbertype = HE5_EHconvdatatype(ntype);
  if(numbertype == FAIL)
    {
      H5Epush(__FILE__, "HE5_PTwriteattrF", __LINE__, H5E_DATATYPE, H5E_BADVALUE, "Error calling HE5_EHconvdatatype() from FORTRAN wrapper.");
      HE5_EHprint("Error: Error calling HE5_EHconvdatatype() from FORTRAN wrapper, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Allocate memory for error message buffers */
  errbuf  = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTwriteattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Allocate memory for "count" array */  
  count = (hsize_t *)calloc(rank, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_PTwriteattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }


  /* Reverse order of dimensions (FORTRAN -> C) */
  /* ------------------------------------------ */
  for (i = 0; i < rank; i++)
    count[i] = (hsize_t)fortcount[rank - 1 - i];
  
  PointID = (hid_t)pointID;

  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR) || (numbertype == H5T_C_S1))
    {
      if (strlen(datbuf) < count[0])
	{
	  sprintf(errbuf,"Size of databuf is less than the number of attribute elements.\n");
	  H5Epush(__FILE__, "HE5_PTwriteattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      /* Allocate memory for temp buffer */
      /* ------------------------------- */
      tempbuf = (char * )calloc((count[0]+1), sizeof(char));
      if(tempbuf == NULL)
	{
	  sprintf(errbuf,"Cannot allocate memory for temp buffer.\n");
	  H5Epush(__FILE__, "HE5_PTwriteattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      strncpy(tempbuf, datbuf, count[0]);
      tempbuf[count[0]] = '\0';
 
      /* Call PTwriteattr() */
      /* ------------------ */
      status = HE5_PTwriteattr(PointID, attrname, numbertype, count, tempbuf);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot write data to the attribute \"%s\".\n", attrname);
	  H5Epush(__FILE__, "HE5_PTwriteattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  free(tempbuf);
	  return(FAIL);
	}
      free(count);
      free(errbuf);
      free(tempbuf);
    }
  else
    { 
      /* Call PTwriteattr() */
      /* ------------------ */
      status = HE5_PTwriteattr(PointID, attrname, numbertype, count, datbuf);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot write data to the attribute \"%s\".\n", attrname);
	  H5Epush(__FILE__, "HE5_PTwriteattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
  
      free(count);
      free(errbuf);
    }

  ret = (int)status;
  return(ret);

}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwritegrpattrF   (FORTRAN wrapper)                          |
|                                                                             |
|  DESCRIPTION:   Writes/updates attribute associated with the "Data"  group  |
|                     in a point.                                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  attrname       char                attribute name                          |
|  ntype          int                 attribute HDF numbertype                |
|  fortcount      long                Number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTwritegrpattrF(int pointID, const char *attrname, int ntype, long fortcount[], void * datbuf)
{
  int          ret        = FAIL;        /* (int) return status variable      */
  int	       rank       = 1;           /* Rank variable                     */
  int	       i;                        /* Loop index                        */

  herr_t       status     = FAIL;        /* routine return status variable    */
  
  hid_t        PointID    = FAIL;        /* HDF5 type Swath ID                */
  hid_t        numbertype = FAIL;        /* HDF5 data type ID                 */
             
  hsize_t      *count  = (hsize_t *)NULL;/* Pointer to count array (C order)  */
  
  char         *errbuf = (char *)NULL;   /* error message buffer              */
  char         *tempbuf = (char *)NULL;  /* temp buffer                       */
  
    
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTwritegrpattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get HDF5 data type ID */
  /* --------------------- */
  numbertype = HE5_EHconvdatatype(ntype);
  if(numbertype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 type data type ID.\n");
      H5Epush(__FILE__, "HE5_PTwritegrpattrF", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }


  count = (hsize_t *)calloc(rank, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_PTwritegrpattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Reverse order of dimensions (FORTRAN -> C) */
  /* ------------------------------------------ */
  for (i = 0; i < rank; i++)
    count[i] = (hsize_t)fortcount[rank - 1 - i];  
  
  PointID = (hid_t)pointID;

  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR) || (numbertype == H5T_C_S1))
    {
      if (strlen(datbuf) < count[0])
	{
	  sprintf(errbuf,"Size of databuf is less than the number of group attribute elements.\n");
	  H5Epush(__FILE__, "HE5_PTwritegrpattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      /* Allocate memory for temp buffer */
      /* ------------------------------- */
      tempbuf = (char * )calloc((count[0]+1), sizeof(char));
      if(tempbuf == NULL)
	{
	  sprintf(errbuf,"Cannot allocate memory for temp buffer.\n");
	  H5Epush(__FILE__, "HE5_PTwritegrpattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      strncpy(tempbuf, datbuf, count[0]);
      tempbuf[count[0]] = '\0';
 
      status = HE5_PTwritegrpattr(PointID, attrname, numbertype, count, tempbuf);
      if(status == FAIL)
	{
          sprintf(errbuf,"Cannot write group attribute value.\n");
          H5Epush(__FILE__, "HE5_PTwritegrpattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          free(errbuf);
          free(count);
          free(tempbuf);
          return(FAIL);
	}
      free(count);
      free(errbuf);
      free(tempbuf);
    }
  else
    {
      status = HE5_PTwritegrpattr(PointID, attrname, numbertype, count, datbuf);
      if(status == FAIL)
	{
          sprintf(errbuf,"Cannot write group attribute value.\n");
	  H5Epush(__FILE__, "HE5_PTwritegrpattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
          free(errbuf);
          free(count);
          return(FAIL);
	}
      free(count);
      free(errbuf);
    }

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTwritelocattrF   (FORTRAN wrapper)                          |
|                                                                             |
|  DESCRIPTION:   Writes/updates attribute associated with the a specified    |
|                   level in a point.                                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  levelname      char                level name                              |
|  attrname       char                attribute name                          |
|  ntype          int                 attribute HDF numbertype                |
|  fortcount      long                Number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTwritelocattrF(int pointID, const char *levelname, const char *attrname, int ntype, long fortcount[],void * datbuf)
{
  int	       ret  = FAIL;             /* routine return status variable             */
  int 	       rank = 1;                /* Note: It is assumed that fortcout has just */
  /* one element to agree with EHattr function  */
  int	       i      = 0;

  herr_t       status = FAIL;           /* routine return status variable             */
  
  hsize_t      *count = (hsize_t *)NULL;/* Pointer to count array (C order)           */
 
  hid_t        PointID    = FAIL;       /* Swath HDF5 type ID                         */   
  hid_t        numbertype = FAIL;       /* HDF5 dta type ID                           */
  
  char         *errbuf = (char *)NULL;  /* error message buffer                       */
  char         *tempbuf = (char *)NULL; /* temp buffer                                */
  

  /* Get HDF5 type data type ID */
  /* -------------------------- */
  numbertype = HE5_EHconvdatatype(ntype);
  if(numbertype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 data type ID.\n");
      H5Epush(__FILE__, "HE5_PTwritelocattrF", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Allocate memory for error message buffers */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTwritelocattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *)calloc(rank, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_PTwritelocattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  
  /* Reverse order of dimensions (FORTRAN -> C) */
  /* ------------------------------------------ */
  for (i = 0; i < rank; i++)
    count[i] = (hsize_t)fortcount[rank - 1 - i];
  
  PointID = (hid_t)pointID;

  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR) || (numbertype == H5T_C_S1))
    {
      if (strlen(datbuf) < count[0])
	{
	  sprintf(errbuf,"Size of databuf is less than the number of local attribute elements.\n");
	  H5Epush(__FILE__, "HE5_PTwritelocattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      /* Allocate memory for temp buffer */
      /* ------------------------------- */
      tempbuf = (char * )calloc((count[0]+1), sizeof(char));
      if(tempbuf == NULL)
	{
	  sprintf(errbuf,"Cannot allocate memory for temp buffer.\n");
	  H5Epush(__FILE__, "HE5_PTwritelocattrF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      strncpy(tempbuf, datbuf, count[0]);
      tempbuf[count[0]] = '\0';
 
      /* Write local attribute buffer */
      /* ---------------------------- */
      status = HE5_PTwritelocattr(PointID, levelname, attrname, numbertype, count, tempbuf);   
      if(status == FAIL)
	{
          sprintf(errbuf,"Cannot write local attribute value.\n");
          H5Epush(__FILE__, "HE5_PTwritelocattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          free(count);
          free(errbuf);
          free(tempbuf);
          return(FAIL);
	}
      free(count);
      free(errbuf);
      free(tempbuf);
    }
  else
    {
      /* Write local attribute buffer */
      /* ---------------------------- */
      status = HE5_PTwritelocattr(PointID, levelname, attrname, numbertype, count, datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write local attribute value.\n");
	  H5Epush(__FILE__, "HE5_PTwritelocattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
          free(count);
          free(errbuf);
          return(FAIL);
	}
      free(count);
      free(errbuf);
    }

  ret = (int)status;
  return(ret);

}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadattrF   (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION: Reads attribute from a point.                                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTreadattrF(int pointID, const char *attrname, void *datbuf)
{
  int         ret     = FAIL;/* routine return status variable  */

  herr_t      status  = FAIL;/* routine return status variable  */

  hid_t       PointID = FAIL;/* HDF-EOS point ID                */

  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
  
  PointID = (hid_t)pointID;

  /* Read the attribute buffer */
  /* ------------------------- */
  status = HE5_PTreadattr(PointID, attrname, datbuf);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot read the attribute value. \n");
      H5Epush(__FILE__, "HE5_PTreadattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadgrpattrF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION: Reads attribute associated with the "Data" group from a point.|
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTreadgrpattrF(int pointID, const char *attrname, void *datbuf)
{
  int         ret             = FAIL;   /* (int) return status      */

  herr_t      status          = FAIL;   /* return status variable   */
  
  hid_t       PointID         = FAIL;   /* HDF5 type Point ID       */
  
  char        errbuf[HE5_HDFE_ERRBUFSIZE];  /* Error message buffer */
 
  PointID = (hid_t)pointID;

  status = HE5_PTreadgrpattr(PointID,attrname,datbuf);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot read Group Attribute \"%s\".\n", attrname);
      H5Epush(__FILE__, "HE5_PTreadgrpattr", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTreadlocattrF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION: Reads attribute associated with a specified level in a point. |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  levelname      char                level name                              |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTreadlocattrF(int pointID, const char *levelname, const char *attrname, void *datbuf)
{
  int         ret             =  FAIL;/* return status variable   */

  herr_t      status          =  FAIL;/* return status variable   */

  hid_t       PointID         =  FAIL;/* HDF5 type Point  ID      */
  
  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */


  PointID = (hid_t)pointID;

  status = HE5_PTreadlocattr( PointID, levelname, attrname, datbuf);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot read Attribute \"%s\" associated wth the \"%s\" level.\n", attrname,levelname);
      H5Epush(__FILE__, "HE5_PTreadlocattr", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret  = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION:  HE5_PTc2fort                                                    |
|                                                                             |
|  DESCRIPTION:   Translates a buffer in Fortran order into a buffer in       |
|                 'C' order.                                                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  dims[]         long                Dimensions of Fortran array             |
|  rank           int                 Rank of Fortran array                   |
|  datatype       int                 Datatype of Fortran array               |
|  inputfortbuf   void                input data buffer (Fortran array)       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  outputfortbuf  void                output data buffer (Fortran array)      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date        Programmer    Description                                     |
|  ==========   ============  =============================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static int
HE5_PTc2fort(long dims[], int rank, int datatype, void *inputfortbuf, void *outputfortbuf)
{
  herr_t            status    = FAIL;/* routine return status variable */

  int               ret       = FAIL;/* (int) return status variable   */
  int               i         = 0;
  int               dim_index = 0;

  long              *c_index         = (long *)NULL;
  long              *coeffs          = (long *)NULL;
  long              *n1              = (long *)NULL;
  long              *fort_order_buf  = (long *)NULL;
  long              num_elements1    = 0;
  long              num_elements2    = 0;

  int               *databuf1_int       = (int *)NULL;     
  int               *databuf2_int       = (int *)NULL;
  float             *databuf1_float     = (float *)NULL;
  float             *databuf2_float     = (float *)NULL;
  double            *databuf1_double    = (double *)NULL;
  double            *databuf2_double    = (double *)NULL;
  char              *databuf1_char      = (char *)NULL;
  char              *databuf2_char      = (char *)NULL;
  long              *databuf1_long      = (long *)NULL;
  long              *databuf2_long      = (long *)NULL;

  char              *errbuf = (char *)NULL;/*Error message buffer   */



  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTc2fort", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Calculate the number of elements in the array */
  /* --------------------------------------------- */
  num_elements1 = 1;
  for( i=0; i < rank; i++ )
    num_elements1 = dims[i]*num_elements1;

  /* Set up arguments for call to HE5_PTgetfort_order() */
  /* -------------------------------------------------- */
  
  /* Initialize c_index array */
  /* ------------------------ */
  c_index    = (long *) calloc(rank, sizeof(long));
  for( i=0; i< rank; i++ )
    c_index[i] = 0;

  /* Initialize coeffs array */
  /* ----------------------- */
  coeffs    = (long *) calloc(rank, sizeof(long));
  for( i=0; i< rank; i++ )
    coeffs[i] = 0;
  coeffs[0] = 1;

  /* Initialize n1 variable */
  /* ---------------------- */
  n1 = (long *) calloc(1,sizeof(long));
  *n1        = 0;

  dim_index = 1;

  /* Initialize fort_order_buf */
  /* ------------------------- */
  fort_order_buf  =  (long *) calloc(num_elements1,sizeof(long));


  status = HE5_PTgetfort_order(dims, dim_index, c_index, rank, fort_order_buf, coeffs, n1);
  if (status == FAIL)
    {
      sprintf(errbuf, "Call to HE5_PTgetfort_order() failed.\n");
      H5Epush(__FILE__, "HE5_PTc2fort", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(fort_order_buf);
      return(status);
    }

  num_elements2 = *n1;

  /* Check num_elements2 against num_elements1 */
  /* ----------------------------------------- */
  if( num_elements1 != num_elements2 )
    {
      sprintf(errbuf, "Error with number of elements processed by HE5_PTgetfort_order or HE5_ptc2fort.\n");
      H5Epush(__FILE__, "HE5_PTc2fort", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(fort_order_buf);
      return(FAIL);
    }

  switch( datatype )
    {
    case HE5T_NATIVE_INT:
      {	
	databuf1_int = (int *) calloc(num_elements1,sizeof(int));
	memmove(databuf1_int,inputfortbuf,num_elements1*sizeof(int));  
   
	databuf2_int = (int *) calloc(num_elements1,sizeof(int));

	/* Create the Fortran ordered buffer from the hash list */
	/* and the C buffer                                     */
	/* ---------------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_int[fort_order_buf[i]] = databuf1_int[i];  
	  }

	memmove(outputfortbuf,databuf2_int,num_elements1*sizeof(int)); 

	free(databuf1_int);
	free(databuf2_int);
      }	
      break;  

    case HE5T_NATIVE_FLOAT:
      {	
	databuf1_float = (float *) calloc(num_elements1,sizeof(float));
	memmove(databuf1_float,inputfortbuf,num_elements1*sizeof(float));  
   
	databuf2_float = (float *) calloc(num_elements1,sizeof(float));

	/* Create the Fortran ordered buffer from the hash list */
	/* and the C buffer                                     */
	/* ---------------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_float[fort_order_buf[i]] = databuf1_float[i];  
	  }

	memmove(outputfortbuf,databuf2_float,num_elements1*sizeof(float)); 

	free(databuf1_float);
	free(databuf2_float);
      }	
      break;  

    case HE5T_NATIVE_DOUBLE:
      {
	databuf1_double = (double *) calloc(num_elements1,sizeof(double));
	memmove(databuf1_double,inputfortbuf,num_elements1*sizeof(double));  
   
	databuf2_double = (double *) calloc(num_elements1,sizeof(double));

	/* Create the Fortran ordered buffer from the hash list */
	/* and the C buffer                                     */
	/* ---------------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_double[fort_order_buf[i]] = databuf1_double[i];  
	  }

	memmove(outputfortbuf,databuf2_double,num_elements1*sizeof(double)); 

	free(databuf1_double);
	free(databuf2_double);
      }	
      break;  

    case HE5T_NATIVE_CHAR:
      {	
	databuf1_char = (char *) calloc(num_elements1,sizeof(char));
	memmove(databuf1_char,inputfortbuf,num_elements1*sizeof(char));  
   
	databuf2_char = (char *) calloc(num_elements1,sizeof(char));

	/* Create the Fortran ordered buffer from the hash list */
	/* and the C buffer                                     */
	/* ---------------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_char[fort_order_buf[i]] = databuf1_char[i];  
	  }
		
	memmove(outputfortbuf,databuf2_char,num_elements1*sizeof(char)); 
		
	free(databuf1_char);
	free(databuf2_char);
      }	
      break;  
	  
    case HE5T_NATIVE_LONG:
      {	
	databuf1_long = (long *) calloc(num_elements1,sizeof(long));
	memmove(databuf1_long,inputfortbuf,num_elements1*sizeof(long));  
		
	databuf2_long = (long *) calloc(num_elements1,sizeof(long));

	/* Create the Fortran ordered buffer from the hash list */
	/* and the C buffer                                     */
	/* ---------------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_long[fort_order_buf[i]] = databuf1_long[i];  
	  }
		
	memmove(outputfortbuf,databuf2_long,num_elements1*sizeof(long)); 
		
	free(databuf1_long);
	free(databuf2_long);
      }	
      break;  

    default:
      {
	sprintf(errbuf, "Bad datatype value in call to HE5_PTc2fort. \n");
	H5Epush(__FILE__, "HE5_PTc2fort", __LINE__,  H5E_ARGS, H5E_BADVALUE,errbuf);
	HE5_EHprint(errbuf, __FILE__, __LINE__);
	status = FAIL;
      }
      break;  
    }

  /* Free up space */
  /* ------------- */ 
  free(c_index);
  free(coeffs);
  free(n1);
  free(fort_order_buf);
  free(errbuf);
  
  status = 0;
  ret = (int)status;   
  
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION:  HE5_PTfort2c                                                    |
|                                                                             |
|  DESCRIPTION:   Translates a buffer in Fortran order into a buffer in       |
|                 'C' order.                                                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  dims[]         long                Dimensions of Fortran array             |
|  rank           int                 Rank of Fortran array                   |
|  datatype       int                 Datatype of Fortran array               |
|  inputfortbuf   void                input data buffer (Fortran array)       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  outputfortbuf  void                output data buffer (Fortran array)      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date        Programmer    Description                                     |
|  ==========   ============  =============================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static int
HE5_PTfort2c(long dims[], int rank, int datatype, void *inputfortbuf, void *outputfortbuf)
{
  herr_t            status    = FAIL;/* routine return status variable */

  int               ret       = FAIL;/* (int) return status variable   */
  int               i         = 0;
  int               dim_index = 0;

  long              *c_index         = (long *)NULL;
  long              *coeffs          = (long *)NULL;
  long              *n1              = (long *)NULL;
  long              *fort_order_buf  = (long *)NULL;
  long              num_elements1    = 0;
  long              num_elements2    = 0;

  int               *databuf1_int       = (int *)NULL;     
  int               *databuf2_int       = (int *)NULL;
  float             *databuf1_float     = (float *)NULL;
  float             *databuf2_float     = (float *)NULL;
  double            *databuf1_double    = (double *)NULL;
  double            *databuf2_double    = (double *)NULL;
  char              *databuf1_char      = (char *)NULL;
  char              *databuf2_char      = (char *)NULL;
  long              *databuf1_long      = (long *)NULL;
  long              *databuf2_long      = (long *)NULL;

  char              *errbuf = (char *)NULL;/*Error message buffer   */


  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTfort2c", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }


  /* Calculate the number of elements in the array */
  /* --------------------------------------------- */
  num_elements1 = 1;
  for( i=0; i < rank; i++ )
    {
      num_elements1 = dims[i]*num_elements1;      
    }
  

  /* Set up arguments for call to HE5_PTgetfort_order() */
  /* -------------------------------------------------- */
  
  /* Initialize c_index array */
  /* ------------------------ */
  c_index    = (long *) calloc(rank, sizeof(long));
  for( i=0; i< rank; i++ )
    c_index[i] = 0;
  
  /* Initialize coeffs array */
  /* ----------------------- */
  coeffs    = (long *) calloc(rank, sizeof(long));
  for( i=0; i< rank; i++ )
    coeffs[i] = 0;
  coeffs[0] = 1;
  
  /* Initialize n1 variable */
  /* ---------------------- */
  n1 = (long *) calloc(1,sizeof(long));
  *n1        = 0;
  
  dim_index = 1;

  /* Initialize fort_order_buf */
  /* ------------------------- */
  fort_order_buf  =  (long *) calloc(num_elements1,sizeof(long));
  
  status = HE5_PTgetfort_order(dims, dim_index, c_index, rank, fort_order_buf, coeffs, n1);
  if (status == FAIL)
    {
      sprintf(errbuf, "Call to HE5_PTgetfort_order() failed.\n");
      H5Epush(__FILE__, "HE5_PTfort2c", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(fort_order_buf);
      return(status);
    }
  
  num_elements2 = *n1;
  
  /* Check num_elements2 against num_elements1 */
  /* ----------------------------------------- */
  if( num_elements1 != num_elements2 )
    {
      sprintf(errbuf, "Error with number of elements processed by HE5_PTgetfort_order or HE5_ptfort2c.\n");
      H5Epush(__FILE__, "HE5_PTfort2c", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(fort_order_buf);
      return(FAIL);
    }
  
  switch( datatype )
    {
    case HE5T_NATIVE_INT:
      {	
	databuf1_int = (int *) calloc(num_elements1,sizeof(int));
	memmove(databuf1_int,inputfortbuf,num_elements1*sizeof(int));  
   
	databuf2_int = (int *) calloc(num_elements1,sizeof(int));
		
	/* Create the c ordered buffer from the hash list */
	/* ---------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_int[i] = databuf1_int[fort_order_buf[i]];
	  }

	memmove(outputfortbuf,databuf2_int,num_elements1*sizeof(int)); 
		
	free(databuf1_int);
	free(databuf2_int);
      }	
      break;  
	  
    case HE5T_NATIVE_FLOAT:
      {	
	databuf1_float = (float *) calloc(num_elements1,sizeof(float));
	memmove(databuf1_float,inputfortbuf,num_elements1*sizeof(float));  
		
	databuf2_float = (float *) calloc(num_elements1,sizeof(float));
		
	/* Create the c ordered buffer from the hash list */
	/* ---------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_float[i] = databuf1_float[fort_order_buf[i]];
	  }
		
	memmove(outputfortbuf,databuf2_float,num_elements1*sizeof(float)); 
		
	free(databuf1_float);
	free(databuf2_float);
      }	
      break;  
	  
    case HE5T_NATIVE_DOUBLE:
      {	
	databuf1_double = (double *) calloc(num_elements1,sizeof(double));
	memmove(databuf1_double,inputfortbuf,num_elements1*sizeof(double));  
		
	databuf2_double = (double *) calloc(num_elements1,sizeof(double));

	/* Create the c ordered buffer from the hash list */
	/* ---------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_double[i] = databuf1_double[fort_order_buf[i]];
	  }
		
	memmove(outputfortbuf,databuf2_double,num_elements1*sizeof(double)); 
		
	free(databuf1_double);
	free(databuf2_double);
      }	
      break;  
	  
    case HE5T_NATIVE_CHAR:
      {	
	databuf1_char = (char *) calloc(num_elements1,sizeof(char));
	memmove(databuf1_char,inputfortbuf,num_elements1*sizeof(char));  
		
	databuf2_char = (char *) calloc(num_elements1,sizeof(char));

	/* Create the c ordered buffer from the hash list */
	/* ---------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_char[i] = databuf1_char[fort_order_buf[i]];
	  }
		
	memmove(outputfortbuf,databuf2_char,num_elements1*sizeof(char)); 
		
	free(databuf1_char);
	free(databuf2_char);
      }	
      break;  
	  
    case HE5T_NATIVE_LONG:
      {	
	databuf1_long = (long *) calloc(num_elements1,sizeof(long));
	memmove(databuf1_long,inputfortbuf,num_elements1*sizeof(long));  
		
	databuf2_long = (long *) calloc(num_elements1,sizeof(long));
		
	/* Create the c ordered buffer from the hash list */
	/* ---------------------------------------------- */
	for( i = 0; i < num_elements1; i++ )
	  {
	    databuf2_long[i] = databuf1_long[fort_order_buf[i]];
	  }
		
	memmove(outputfortbuf,databuf2_long,num_elements1*sizeof(long)); 
		
	free(databuf1_long);
	free(databuf2_long);
      }	
      break;  
	  
	  
    default:
      {
	sprintf(errbuf, "Bad datatype value in call to HE5_PTfort2c. \n");
	H5Epush(__FILE__, "HE5_PTfort2c", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	HE5_EHprint(errbuf, __FILE__, __LINE__);
	status = FAIL;
      }
      break;  
    }
  
  /* Free up space */
  /* ------------- */ 
  free(c_index);
  free(coeffs);
  free(n1);
  free(fort_order_buf);
  free(errbuf);
  
  ret = (int)status;   
  
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION:  HE5_PTgetfort_order                                             |
|                                                                             |
|  DESCRIPTION:   Translates a buffer in Fortran order into a buffer in       |
|                 'C' order.                                                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  dims           long                Dimensions of Fortran array             |
|  dim_index      int                 Current dimension in recursive iteration|
|  c_index                            Changing coeffiencients in equation to  |
|                                     calculate fortran order                 |
|  rank           int                 Rank of Fortran array                   |
|  coeffs         long                Constant coefficients in equation to    |
|                                     calculate fortran order                 |
|                                                                             |
|  n              long                number of elements processed which      |
|                                     will equal num_elemnts in array         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  P              long                buffer containing Fortran ordering      |
|                                                                             |
|  NOTES:    This program is called recursively, once for each of the         |
|            dimensions in the fortran array. Values in c_index and coeffs    |
|            are calculated during each of the recursive calls.   Values in   |
|            P are calculated in the last recursive call.                     |
|                                                                             |
|                                                                             |
|                                                                             |
|   Date        Programmer    Description                                     |
|  ==========   ============  =============================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static int
HE5_PTgetfort_order(long *dims, int dim_index, long *c_index, int rank, long *P, long *coeffs, long *n)
{
  herr_t              status       = FAIL;/* routine return status variable */

  int                 ret          = FAIL;/* (int) return status variable   */
  int                 i            = 0;
  int                 j            = 0;
  int                 k            = 0;
  int                 l            = 0;
  int                 dim_index_tt = 0;

  long                K1           = 0;
  long                P1           = 0;

  char                *errbuf = (char *)NULL;/*Error message buffer   */
   

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTgetfort_order", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
   
  K1  = 1;
   
  for( i = 0; i < dim_index; i++ )
    {
      K1 = dims[i]*K1;
    }

  if( dim_index < rank ) 
    {
      coeffs[dim_index] = K1;
	   
      dim_index_tt      =  dim_index;
      dim_index         =  dim_index + 1;
	   
      for( k = 0; k < dims[dim_index_tt-1]; k++ )
	{
	  c_index[dim_index_tt-1] = k;
	  status = HE5_PTgetfort_order( dims, dim_index, c_index, rank, P, coeffs, n);         
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Call to HE5_PTgetfort_order() failed.\n");
	      H5Epush(__FILE__, "HE5_PTgetfort_order", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(status);
	    }
	}
    }
  else
    {
      for( l = 0; l < dims[dim_index-1]; l++ )
	{
	  P1 = 0;
	  c_index[dim_index-1] = l;
		   
	  for( j = 0; j < rank ; j++ )
	    {
	      P1  =  (coeffs[j])*(c_index[j]) + P1;
	    }         
		   
	  P[*n] = P1;
	  *n = *n+1;
	}
    }
   
  free(errbuf);
   
  status = 0;
  ret = (int)status;
  return(ret);   
}

/* Inquiry routines */

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTnrecsF   (FORTRAN wrapper)                                 |
|                                                                             |
|  DESCRIPTION: Returns the number of records in a level.                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nallrec        int                 Number of records in level              |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTnrecsF(int pointID, int level)
{ 
  hid_t           PointID = FAIL;/* HDF5 type point ID            */

  hsize_t         nallrec  = 0;   /* Number of records in  Point       */

  char            *errbuf = (char *)NULL;/*Error message buffer   */

  PointID = (hid_t)pointID;

  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTnrecsF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }  
  
  nallrec = HE5_PTnrecs(PointID, level);
  if(nallrec == 0)
    {
      sprintf(errbuf, "No records in current level. \n");
      H5Epush(__FILE__, "HE5_PTnrecsF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }	

  free(errbuf);
  return((int)nallrec);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTnlevelsF   (FORTRAN wrapper)                               |
|                                                                             |
|  DESCRIPTION: Returns the number of levels in a point data set.             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nlevels        int                 Number of levels in point structure     |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int        	      point structure ID                      |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTnlevelsF(hid_t pointID)
{
  hid_t           PointID = FAIL;/* HDF5 type point ID            */

  int             nlevels  = 0;  /* Number of levels in point     */

  char            *errbuf = (char *)NULL;/*Error message buffer   */

  PointID = (hid_t)pointID;

  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTnlevelsF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }  
 
  /* Get number of levels */
  /* -------------------- */
  nlevels = HE5_PTnlevels(PointID);
  if(nlevels == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the number of levels. \n");
      H5Epush(__FILE__, "HE5_PTnlevelsF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  free(errbuf);
  return((int)nlevels);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTnfieldsF   (FORTRAN wrapper)                               |
|                                                                             |
|  DESCRIPTION: Returns number of fields defined in a level.                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nfields        int                 Number of fields in a level             |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|  OUTPUTS:                                                                   |
|  strbufsize     long                String length of fieldlist              |
|  fieldlist      char                Coma separated list of fields           |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date       Programmer   Description                                       |
|  ========   ============  ================================================= |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTnfieldsF(int pointID, int level, char *fieldlist, long *strbufsize)
{
  hid_t           PointID  = FAIL;/* HDF5 type point ID            */

  int             nfields  = 0;   /* Number of fields in the level of the point*/

  char            *errbuf = (char *)NULL;/*Error message buffer   */

  PointID = (hid_t)pointID;

  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTnfieldsF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }  
 
  /* Get number of fields and fieldlist */
  /* ---------------------------------- */
  nfields = HE5_PTnfields(PointID, level, fieldlist, strbufsize);
  if(nfields == 0)
    {
      sprintf(errbuf, "Cannot retrieve the number of fields. \n");
      H5Epush(__FILE__, "HE5_PTnfieldsF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  free(errbuf);

  return((int)nfields);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTlevelindxF   (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION: Returns index number for a named level.                       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  n              int                 Level number (0 - based)                |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        in                  point structure ID                      |
|  levelname      char                point level name                        |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTlevelindxF(int pointID, const char *levelname)
{
  hid_t           PointID = FAIL;/* HDF5 type point ID            */

  int             index   = FAIL;          

  char            *errbuf = (char *)NULL;/*Error message buffer   */

  PointID = (hid_t)pointID;

  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTlevelindxF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }  

  PointID = (hid_t)pointID;

  /* Get index of named Level           */
  /* ---------------------------------- */
  index = HE5_PTlevelindx(PointID, levelname);
  if(index == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the Level Index. \n");
      H5Epush(__FILE__, "HE5_PTlevelindxF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  free(errbuf);
  return(index);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTgetlevelnameF   (FORTRAN wrapper)                          |
|                                                                             |
|  DESCRIPTION: Returns level name                                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  levelname      char                Level name                              |
|  strbufsize     long                String length of fieldlist              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTgetlevelnameF(int pointID, int  level, char *levelname, long *strbufsize)
{
  int             ret     = FAIL;

  herr_t          status  = FAIL;/* routine return status variable */

  hid_t           PointID = FAIL;

  char            *errbuf = (char *)NULL;/*Error message buffer   */


  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTgetlevelnameF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(ret);
    }  

  PointID = (hid_t)pointID;

  /* Get index of named Level           */
  /* ---------------------------------- */

  status = HE5_PTgetlevelname( PointID, level, levelname, strbufsize);
  if(status == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve the Level Name. \n");
      H5Epush(__FILE__, "HE5_PTgetlevelnameF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  free(errbuf);
  ret = (int)status;
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTbcklinkinfoF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION: Returns link field to previous level.                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|  linkfield      char                linkage field                           |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTbcklinkinfoF(int pointID, int level, char *linkfield)
{
  int             ret     = FAIL;

  herr_t          status  = FAIL;/* routine return status variable */

  hid_t           PointID = FAIL;/* HDF5 type point ID            */

  char            *errbuf = (char *)NULL;/*Error message buffer   */


  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTbcklinkinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(ret);
    }  

  PointID = (hid_t)pointID;

  status = HE5_PTbcklinkinfo( PointID, level, linkfield);
  if(status == FAIL)
    {
      sprintf(errbuf, "Error in Fortran Wrapper HE5_PTbcklinkinfoF(). \n");
      H5Epush(__FILE__, "HE5_PTbcklinkinfoF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  free(errbuf);

  ret = (int)status;
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTfwdlinkinfoF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION: Returns link field to following level.                        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|  linkfield      char                linkage field                           |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_PTfwdlinkinfoF(int pointID, int level, char *linkfield)
{
  int             ret     = FAIL;

  herr_t          status  = FAIL;/* routine return status variable */

  hid_t           PointID = FAIL;/* HDF5 type point ID            */

  char            *errbuf = (char *)NULL;/*Error message buffer   */


  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTfwdlinkinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(ret);
    }  

  PointID = (hid_t)pointID;

  status = HE5_PTfwdlinkinfo( PointID, level, linkfield);
  if(status == FAIL)
    {
      sprintf(errbuf, "Error in Fortran Wrapper HE5_PTfwdlinkinfoF(). \n");
      H5Epush(__FILE__, "HE5_PTfwdlinkinfoF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  free(errbuf);
  ret = (int)status;
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTlevelinfoF   (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION: Returns information about a given level.                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  levelname      char                Name of Level                           |
|  rank           int                 Rank of Fields                          |
|  fieldlist      char                Fieldlist to read (comma-separated)     |
|  dim_sizes                          Sizes of field dimensions               |
|  datasize       long                Datasize of compound structure          |
|  offset         long                Offsets of each of the fields           |
|  dtype          int                 Datatypes of each of the fields         |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTlevelinfoF(int pointID, int level, char *levelname, int rank[], char *fieldlist, long *dim_sizes, long *datasize, long offset[], int dtype[])
{
  herr_t            status = FAIL;/* routine return status variable */    
   
  hid_t             PointID = FAIL;

  HE5_CmpDTSinfo    dtsinfo;

  int               ret           = FAIL;/* (int) Return status variable   */
  int               i, j, k       = 0;
  long              nfields       = FAIL;
  long              max_rank      = 0;
  long              strbufsize    = 0;
 
  char              errbuf[HE5_HDFE_ERRBUFSIZE]; /* error message buffer */


  PointID  =  (hid_t)pointID;


  /* Call HE5_PTlevelinfo to get information structure */
  /* ------------------------------------------------- */ 
  status = HE5_PTlevelinfo(PointID, level, &dtsinfo);
  if (status == FAIL)
    {
      sprintf(errbuf, "Call to HE5_PTlevelinfo failed.\n");
      H5Epush(__FILE__, "HE5_PTlevelinfoF", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  nfields   = (long)dtsinfo.nfields;
  *datasize = (long)dtsinfo.datasize;

  /* Release pointers to the name buffers */
  /* ------------------------------------ */
  for (j = 0; j < dtsinfo.nfields; j++)
    {
      if (dtsinfo.fieldname[j] != NULL) 
	free(dtsinfo.fieldname[j]);
    }

  /* Get the levelname */
  /* ----------------- */
  status = HE5_PTgetlevelname(PointID, level, levelname, &strbufsize);
  if (status == FAIL)
    {
      sprintf(errbuf, "Call to HE5_PTgetlevelname failed.\n");
      H5Epush(__FILE__, "HE5_PTlevelinfoF", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
   
  for( i = 0; i < nfields; i++ )
    {
      rank[i]   = dtsinfo.rank[i];
      offset[i] = (long)dtsinfo.offset[i];
      dtype[i]  = (int)dtsinfo.dtype[i]; 
    }
   
  /* Get fieldlist (output from HE5_PTnfields) */
  /* ----------------------------------------- */
  HE5_PTnfields(pointID, level, fieldlist, &strbufsize);  

  /* Do work to store dimensions in level data structure */
  /* --------------------------------------------------- */
   
  /* Calculate maximum rank */
  /* ---------------------- */
  for( i = 0; i < nfields; i++ )
    {
      if( rank[i] > max_rank )
	max_rank = (long)rank[i]; 
    }
   
  /* Load dimension arrays (2-d fortran dimension array is in fortran order) */
  /* ----------------------------------------------------------------------- */
  for( i = 0; i < max_rank; i++ )
    {
      for( j = 0; j < nfields; j++ )
	{
	  k  =  j * (int)max_rank + i;                   
	  dim_sizes[k] = (long)dtsinfo.dims[j][i] ;   
		   
	}
    }
   
  ret = (int)status;  
  return(ret);  
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqdatatype   (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION: Inquires about datasets in Point                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status          int                return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID         int                point structure ID                      |
|  fieldname       char *             Name of field                           |
|  attrname        char *             Name of attribute                       |
|  fieldgroup      int                Field group flag                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  dtype           int                Datatype flag                           |
|  class           int                Class ID                                |
|  Order           int                                                        |
|  size            long                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_PTinqdatatypeF(int pointID, char *fieldname, char *attrname, int fieldgroup, int *Type, int *Class, int *Order, long *size)
{
  int	       ret      = FAIL;               /* (int) status variable  */

  herr_t       status   = FAIL;               /* return status variable */
  
  size_t       *tsize   = (size_t *)NULL;     /* data size variable     */ 

  hid_t        *typeID  = (hid_t *)NULL;      /* HDF5 Data type ID      */
  hid_t        PointID  = FAIL;               /* HDF5 type Point ID     */
 
  H5T_class_t  *classID = (H5T_class_t *)NULL;/*  Data type class ID    */
  H5T_order_t  *order   = (H5T_order_t *)NULL;/* Byte order of datatype */

  char         *errbuf  = (char *)NULL;       /* error message buffer   */


  /* Change Fortran character '0' to NULL */
  /* (still works with '0'                */
  /* ------------------------------------ */
  if( *fieldname == '0' ) { *fieldname = (char)(intptr_t)NULL; }
  if( *attrname  == '0' ) { *attrname  = (char)(intptr_t)NULL; }


  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTinqdatatypeF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  typeID = (hid_t *)calloc(1, sizeof(hid_t));
  if(typeID == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for typeID.\n");
      H5Epush(__FILE__, "HE5_PTinqdatatypeF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  tsize = (size_t *)calloc(1, sizeof(size_t));
  if(tsize == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for tsize.\n");
      H5Epush(__FILE__, "HE5_PTinqdatatypeF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(typeID);
      return(FAIL);
    }

  classID = (H5T_class_t *)calloc(1, sizeof(H5T_class_t));
  if(classID == (H5T_class_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for classID.\n");
      H5Epush(__FILE__, "HE5_PTinqdatatypeF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(typeID);
      free(tsize);
      return(FAIL);
    }

  order = (H5T_order_t *)calloc(1, sizeof(H5T_order_t));
  if(order == (H5T_order_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for order.\n");
      H5Epush(__FILE__, "HE5_PTinqdatatypeF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(typeID);
      free(tsize);
      free(classID);
      return(FAIL);
    }

  PointID = (hid_t)pointID;

  /* Get data type information */
  /* ------------------------- */
  status = HE5_PTinqdatatype(PointID, fieldname, attrname, fieldgroup, typeID, classID, order, tsize);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about data type.\n");
      H5Epush(__FILE__, "HE5_PTinqdatatypeF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  *Type   = (int)(*typeID);
  *Class  = (int)(*classID);
  *Order  = (int)(*order);
  *size   = (long)(*tsize);
  
  free(errbuf);
  free(typeID);
  free(classID);
  free(order);
  free(tsize);

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqpointF   (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION: Returns number and names of point structures in file          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nPoint         int                 Number of point structures in file      |
|                                                                             |
|  INPUTS:                                                                    |
|  filename       char                HDF-EOS filename                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  pointlist      char                List of point names (comma-separated)   |
|  strbufsize     long                Length of pointlist                     |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_PTinqpointF(const char *filename, char *pointlist, long *strbufsize)
{
  int             npoint    = FAIL;

  long            nPoint    = FAIL;/* Number of point structures in file (return)  */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */

  nPoint = HE5_PTinqpoint( filename, pointlist, strbufsize);
  if ( nPoint < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the list of Point names. \n");
      H5Epush(__FILE__, "HE5_PTinqpointF", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  npoint = (int)nPoint;
  return(npoint);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTattrinfoF   (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION:                                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          int                 Data type of attribute                  |
|  count          int                 Number of attribute elements            |
|                                                                             |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer    Description                                        |
|  =======   ============  =================================================  |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTattrinfoF(int pointID, const char *attrname, int *numbertype, long *fortcount)
{
  int          ret      = FAIL;               /* (int) return status variable  */ 

  herr_t       status   = FAIL;	              /* return status variable        */

  hid_t        *ntype   = (hid_t *)NULL;      /* Data type class ID            */

  hid_t        PointID  = FAIL;               /* HDF5 type Point ID            */

  hsize_t      *count   = (hsize_t *)NULL;    /* Pointer to count (C order)    */
  
  char         *errbuf  = (char *)NULL;       /* error message buffer          */

  
  /* Allocate memory for error message buffers */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *) calloc(1, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_PTattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
 
  ntype = (hid_t *) calloc(1, sizeof(hid_t));
  if(ntype == (hid_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for ntype.\n");
      H5Epush(__FILE__, "HE5_PTattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(count);
      return(FAIL);
    }

  PointID = (hid_t)pointID;

  status = HE5_PTattrinfo(PointID, attrname, ntype, count);    
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about attribute \"%s\".\n", attrname);
      H5Epush(__FILE__, "HE5_PTattrinfoF", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }  
  
  *fortcount  = (long)(*count);
  *numbertype = (int)(*ntype);

  free(errbuf);
  free(count);
  free(ntype);

  ret = (int)status;
  return(ret); 
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTgrpattrinfoF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION:  Retrieves information about attributes in "Data" group.      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          int                 Data type of attribute                  |
|  fortcount      long                Number of attribute elements            |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_PTgrpattrinfoF(int pointID, const char *attrname, int *numbertype, long *fortcount)
{
  int	      ret     = FAIL;	        /* (int) return status variable   */

  herr_t      status  = FAIL;           /* routine return status variable */

  hid_t       *ntype  = (hid_t *)NULL;  /* Data type class ID         */

  hid_t       PointID = FAIL;           /* HDF5 type Point ID             */

  hsize_t     *count  = (hsize_t *)NULL;/* Pointer to count  (C order)    */
  
  char        *errbuf = (char *)NULL;   /* error message buffer           */
  
  
  /* Allocate memory for error message buffers */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTgrpattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *)calloc(1, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_PTgrpattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  ntype = (hid_t *)calloc(1, sizeof(hid_t));
  if( ntype == (hid_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for ntype.\n");
      H5Epush(__FILE__, "HE5_PTgrpattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(count);
      return(FAIL);
    }

  PointID = (hid_t)pointID;

  status = HE5_PTgrpattrinfo(PointID, attrname, ntype, count);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about group attribute.\n");
      H5Epush(__FILE__, "HE5_PTgrpattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  *fortcount  = (long)(*count);
  *numbertype = (int)(*ntype);

  free(errbuf);
  free(count);
  free(ntype);
  
  ret = (int)status;
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTlocattrinfoF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION:  Retrieves information about attributes in associated with a  |
|                 specified level in a point.                                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  levelname      char                level name                              |
|  attrname       char                attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          int                 Data type of attribute                  |
|  count          long                Number of attribute elements            |
|                                                                             |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_PTlocattrinfoF(int pointID, const char *levelname, const char *attrname, int *numbertype, long *fortcount)
{
  int	      ret     = FAIL;	        /* (int) return status variable   */

  herr_t      status  = FAIL;           /* routine return status variable */

  hid_t       *ntype  = (hid_t *)NULL;  /* Data type class ID         */

  hid_t       PointID = FAIL;           /* HDF5 type Point ID             */

  hsize_t     *count  = (hsize_t *)NULL;/* Pointer to count  (C order)    */
  
  char        *errbuf = (char *)NULL;   /* error message buffer           */

  
  /* Allocate memory for error message buffers */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_PTlocattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *)calloc(1, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_PTlocattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  ntype = (hid_t *)calloc(1, sizeof(hid_t));
  if( ntype == (hid_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for ntype.\n");
      H5Epush(__FILE__, "HE5_PTlocattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(count);
      return(FAIL);
    }

  PointID = (hid_t)pointID;
  
  status = HE5_PTlocattrinfo(PointID, levelname, attrname, ntype, count);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about local attribute.\n");
      H5Epush(__FILE__, "HE5_PTlocattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  *fortcount  = (long)(*count);
  *numbertype = (int)(*ntype);
  
  free(errbuf);
  free(count);
  free(ntype);
  

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqattrsF   (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION:                                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in point struct    |
|                                                                             |
|  INPUTS:                                                                    |
|  point ID       int                 point structure ID                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in point struct         |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_PTinqattrsF(int pointID, char *attrnames, long *strbufsize)
{
  long            nattr    = FAIL;/* Number of attributes (return)  */

  hid_t           PointID  = FAIL;/* HDF5 type Point ID             */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */

  char            *attrnames_t = (char *)NULL;   
  char            buffer[1]    = {'0'};  
  char            buffer1[1]   = {'1'};  

  int             result       = 1;

  PointID = (hid_t)pointID;

  /* Change Fortran character '0' to NULL if needed */
  /* ---------------------------------------------- */
  buffer1[0] = attrnames[0];

  result  = memcmp(buffer1,buffer,sizeof(buffer));

  if( memcmp(buffer1,buffer,sizeof(buffer)) == 0 ) { result = 0; }   

  if( result == 0 )  nattr = HE5_PTinqattrs(PointID, attrnames_t, strbufsize);
  if( result != 0 )  nattr = HE5_PTinqattrs(PointID, attrnames, strbufsize);

  if ( nattr < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the attributes. \n");
      H5Epush(__FILE__, "HE5_PTinqattrsF", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  return(nattr);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqgrpattrsF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list in "Data" group                               |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in point struct    |
|                                                                             |
|  INPUTS:                                                                    |
|  point ID       int                 point structure ID                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in point struct         |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_PTinqgrpattrsF(int pointID, char *attrnames, long *strbufsize)
{
  long            nattr   = FAIL;         /* Number of attributes (return)  */

  hid_t           PointID = FAIL;         /* HDF5 type swath ID             */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer           */

  char            *attrnames_t = (char *)NULL;   
  char            buffer[1]    = {'0'};  
  char            buffer1[1]   = {'1'};  

  int             result       = 1;

  PointID = (hid_t)pointID;

  /* Change Fortran character '0' to NULL if needed */
  /* ---------------------------------------------- */
  buffer1[0] = attrnames[0];

  result  = memcmp(buffer1,buffer,sizeof(buffer));

  if( memcmp(buffer1,buffer,sizeof(buffer)) == 0 ) { result = 0; }   
  
  if( result == 0 )  nattr = HE5_PTinqgrpattrs(PointID, attrnames_t, strbufsize);
  if( result != 0 )  nattr = HE5_PTinqgrpattrs(PointID, attrnames, strbufsize);
  if ( nattr < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the attributes. \n");
      H5Epush(__FILE__, "HE5_PTinqgrpattrsF", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  return(nattr);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTinqlocattrsF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list from a specified level in a point.            |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in a level         |
|                                                                             |
|  INPUTS:                                                                    |
|  point ID       int                 point structure ID                      |
|  levelname      char                level name                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in a level              |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/01/01   C.Praderas   Initial coding                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_PTinqlocattrsF(int pointID, const char *levelname, char *attrnames, long *strbufsize)
{
  long            nattr       = FAIL;     /* Number of attributes   */

  hid_t           PointID     = FAIL;     /* HDF5 type swath ID     */
    
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */  
  char            *attrnames_t = (char *)NULL;   
  char            buffer[1]    = {'0'};  
  char            buffer1[1]   = {'1'};  

  int             result       = 1;

  PointID = (hid_t)pointID;

  /* Change Fortran character '0' to NULL if needed */
  /* ---------------------------------------------- */
  buffer1[0] = attrnames[0];

  result  = memcmp(buffer1,buffer,sizeof(buffer));

  if( memcmp(buffer1,buffer,sizeof(buffer)) == 0 ) { result = 0; }   

  if( result == 0 )  nattr = HE5_PTinqlocattrs(PointID, levelname, attrnames_t, strbufsize);
  if( result != 0 )  nattr = HE5_PTinqlocattrs(PointID, levelname, attrnames, strbufsize);
  if ( nattr < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the attributes. \n");
      H5Epush(__FILE__, "HE5_PTinqlocattrsF", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
 
  return (nattr); 
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTnumtypeinfoF   (FORTRAN wrapper)                           |
|                                                                             |
|  DESCRIPTION: Returns the number type about a given level.                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        int                 point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  numtype        int                 Number types of each of the fields      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Aug 04    S.Zhao       Original development                                |
|  Dec 05    T.Roegner    NCR 44092 - Added CYGWIN capability                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTnumtypeinfoF(int pointID, int level, int numtype[])
{
  herr_t            status = FAIL;/* routine return status variable */
  
  hid_t             PointID = FAIL;/* HDF-EOS Point ID              */

  HE5_CmpDTSinfo    dtsinfo;

  int               ret           = FAIL;/* (int) Return status variable */
  int               i             = 0;
  long              nfields       = FAIL;/* Number of fields in the level*/

  char              errbuf[HE5_HDFE_ERRBUFSIZE]; /* error message buffer */


  PointID  =  (hid_t)pointID;


  /* Call HE5_PTlevelinfo to get information structure */
  /* ------------------------------------------------- */
  status = HE5_PTlevelinfo(PointID, level, &dtsinfo);
  if (status == FAIL)
    {
      sprintf(errbuf, "Call to HE5_PTlevelinfo failed.\n");
      H5Epush(__FILE__, "HE5_PTnumtypeinfoF", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  nfields   = (long)dtsinfo.nfields;

  for( i = 0; i < nfields; i++ )
    {
      numtype[i]  = (int)dtsinfo.numtype[i];
    }
  
  ret = (int)status;
  return(ret); 
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION:   HE5_PTgetdtypesize                                             |
|                                                                             |
|  DESCRIPTION: Returns size of datatype for a member in a given level.       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return size if SUCCEED, (-1) FAIL       |
|                                                                             |
|  INPUTS:                                                                    |
|  levelID        hid_t               level dataset ID                        |
|  fieldnum       int                 member number in the level              |
|                                     structure (0 - based)                   |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|   None                                                                      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Dec 05    Abe Taaheri  Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_PTgetdtypesize(hid_t levelID, int i)
{
  hid_t           typeID    = FAIL;    /* compound datatype ID           */
  hid_t           mdt       = FAIL;    /* Member data type ID            */
  int             ret       = FAIL;
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer    */
  int             status;

  typeID = H5Dget_type(levelID);
  mdt = H5Tget_member_type(typeID, i);
  ret = H5Tget_size(mdt);
  if (ret == FAIL)
    {
      sprintf(errbuf, "Cannot get the size of member daat type ID.\n");
      H5Epush(__FILE__, "HE5_PTgetdtypesize", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  status = H5Tclose(mdt);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the field data type ID.\n");
      H5Epush(__FILE__, "HE5_PTgetdtypesize", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  status  = H5Tclose(typeID);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot release the data type ID.\n");
      H5Epush(__FILE__, "HE5_PTgetdtypesize", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_PTgetlevelname_fromSM                                        |
|                                                                             |
|  DESCRIPTION: Returns name of a given level from Structure Metadata         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  pointID        hid_t               point structure ID                      |
|  level          int                 level number (0 - based)                |
|                                                                             |
|  OUTPUTS:                                                                   |
|  LevelName      char string         Name for rquested level obtained from   |
|                                     Structure Metadata                      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  12/28/05  Abe Taaheri   created                                    .       |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/

herr_t  
HE5_PTgetlevelname_fromSM(hid_t pointID, int level, char *LevelName)
{
  herr_t          status   = FAIL;            /* Return status variable   */

  hid_t           fid      = FAIL;            /* HDF-EOS file ID          */
  hid_t           pID      = FAIL;            /* Point ID - offset        */
  hid_t           gid      = FAIL;
  hid_t           idOffset = HE5_PTIDOFFSET;  /* Point ID offset          */

  long 	          idx      = FAIL;            /* Point index              */
 
  char            *metabuf = NULL;	      /* Pointer to SM            */
  char            *metaptrs[2]={NULL,NULL};   /* Ptrs to begin/end of SM  */
  char            name1[HE5_HDFE_NAMBUFSIZE]; /* Name string 1            */
  char            level_subgroupname[HE5_HDFE_NAMBUFSIZE]; 
  char            utlbuf[HE5_HDFE_UTLBUFSIZE];/* Utility buffer           */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error messg buffer       */


  strcpy(LevelName,"");

  /* Check for valid point ID */
  /* ------------------------ */
  status = HE5_PTchkptid(pointID, "HE5_PTgetlevelname_fromSM", 
			 &fid, &gid, &idx);
  if (status == FAIL )
    {
      sprintf(errbuf, "Checking for point ID failed. \n");
      H5Epush(__FILE__, "HE5_PTgetlevelname_fromSM", __LINE__, 
	      H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Compute "reduced" point ID */
  /* -------------------------- */
  pID = pointID % idOffset;

  /* construct level's group name */
  /* -------------------------- */

  sprintf(level_subgroupname, "Level_%d", level);

  /* Get specific level's  structural metadata */
  /* ---------------------------------- */
  metabuf = (char *)HE5_EHmetagroup(fid, HE5_PTXPoint[pID].ptname, "p",
				    level_subgroupname, metaptrs);
  if (metabuf == NULL )
    {
      status = FAIL;
      sprintf(errbuf, "Cannot get pointer to metadata buffer for the \"%s\" point. \n", HE5_PTXPoint[pID].ptname);
      H5Epush(__FILE__, "HE5_PTgetlevelname_fromSM", __LINE__, 
	      H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Search for LevelName=  entry */
  /* ---------------------------- */
  
  status = HE5_EHgetmetavalue(metaptrs, "LevelName", LevelName);

  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot get \"LevelName\" metavalue for the \"%s\" point. \n", HE5_PTXPoint[pID].ptname);
      H5Epush(__FILE__, "HE5_PTgetlevelname_fromSM", __LINE__, 
	      H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(metabuf);
      return(status);
    }
  free(metabuf);
  return(status);
}

#ifndef __cplusplus

/* HDF types used in FORTRAN bindings */
 
#if defined(DEC_ALPHA) || defined(IRIX) || defined(UNICOS) || defined(LINUX64) || defined(IA64) || defined(MACINTOSH) || defined(MACINTEL) || defined(IBM6000) || defined(CYGWIN)
 
#define INT32  INT
#define INT32V INTV
#define PINT32 PINT
 
#else
 
#define INT32  LONG
#define INT32V LONGV
#define PINT32 PLONG
 
#endif

/* File/Point access routines */
FCALLSCFUN2(INT, HE5_PTopenF, HE5_PTOPEN, he5_ptopen, STRING, INT)
FCALLSCFUN2(INT, HE5_PTcreateF, HE5_PTCREATE, he5_ptcreate, INT, STRING)
FCALLSCFUN2(INT, HE5_PTattachF, HE5_PTATTACH, he5_ptattach, INT, STRING)
FCALLSCFUN1(INT, HE5_PTdetachF, HE5_PTDETACH, he5_ptdetach, INT)
FCALLSCFUN1(INT, HE5_PTcloseF, HE5_PTCLOSE, he5_ptclose, INT)

/* Definition routines */
FCALLSCFUN7(INT, HE5_PTdeflevelF,HE5_PTDEFLEVEL, he5_ptdeflevel, INT, STRING, INTV, STRING, PLONG, INTV, INTV)
FCALLSCFUN4(INT, HE5_PTdeflinkageF,HE5_PTDEFLINKAGE, he5_ptdeflinkage, INT, STRING, STRING, STRING)

/* I/O routines */
FCALLSCFUN5(INT, HE5_PTreadlevelF,HE5_PTREADLEVEL, he5_ptreadlevel, INT, INT, STRING, INT, PVOID)   
FCALLSCFUN7(INT, HE5_PTupdatelevelF, HE5_PTUPDATELEVEL, he5_ptupdatelevel, INT, INT, STRING, LONG, LONGV, INT, PVOID)
FCALLSCFUN6(INT, HE5_PTwritelevelF, HE5_PTWRITELEVEL, he5_ptwritelevel, INT, INT, LONGV, STRING, INT, PVOID)
FCALLSCFUN5(INT, HE5_PTwriteattrF, HE5_PTWRITEATTR, he5_ptwriteattr, INT, STRING, INT, LONGV, PVOID)
FCALLSCFUN5(INT, HE5_PTwritegrpattrF, HE5_PTWRITEGRPATTR, he5_ptwritegrpattr, INT, STRING, INT, LONGV, PVOID)
FCALLSCFUN6(INT, HE5_PTwritelocattrF, HE5_PTWRITELOCATTR, he5_ptwritelocattr, INT, STRING, STRING, INT, LONGV, PVOID)
FCALLSCFUN3(INT, HE5_PTreadattrF, HE5_PTREADATTR, he5_ptreadattr, INT, STRING, PVOID)
FCALLSCFUN3(INT, HE5_PTreadgrpattrF, HE5_PTREADGRPATTR, he5_ptreadgrpattr, INT, STRING, PVOID)
FCALLSCFUN4(INT, HE5_PTreadlocattrF, HE5_PTREADLOCATTR, he5_ptreadlocattr, INT, STRING, STRING, PVOID)
FCALLSCFUN2(INT, HE5_PTwrbckptrF, HE5_PTWRBCKPTR, he5_ptwrbckptr, INT, INT)
FCALLSCFUN2(INT, HE5_PTwrfwdptrF, HE5_PTWRFWDPTR, he5_ptwrfwdptr, INT, INT)

/* Point inquiry routines */
FCALLSCFUN2(INT, HE5_PTnrecsF,HE5_PTNRECS, he5_ptnrecs, INT, INT)
FCALLSCFUN1(INT, HE5_PTnlevelsF,HE5_PTNLEVELS, he5_ptnlevels, INT)
FCALLSCFUN4(INT, HE5_PTnfieldsF,HE5_PTNFIELDS, he5_ptnfields, INT, INT, PSTRING, PLONG)
FCALLSCFUN2(INT, HE5_PTlevelindxF,HE5_PTLEVELINDX, he5_ptlevelindx, INT, PSTRING)
FCALLSCFUN4(INT, HE5_PTgetlevelnameF,HE5_PTGETLEVELNAME, he5_ptgetlevelname, INT, INT, PSTRING, PLONG)
FCALLSCFUN3(INT, HE5_PTbcklinkinfoF,HE5_PTBCKLINKINFO, he5_bcklinkinfo, INT, INT, PSTRING)
FCALLSCFUN3(INT, HE5_PTfwdlinkinfoF,HE5_PTFWDLINKINFO, he5_fwdlinkinfo, INT, INT, PSTRING)
FCALLSCFUN9(INT, HE5_PTlevelinfoF, HE5_PTLEVELINFO, he5_ptlevelinfo, INT, INT, PSTRING, INTV, PSTRING, PLONG, PLONG, LONGV, INTV)
FCALLSCFUN8(INT, HE5_PTinqdatatypeF, HE5_PTINQDATATYPE, he5_ptinqdatatype, INT, PSTRING, PSTRING, INT, PINT, PINT, PINT, PLONG)
FCALLSCFUN3(INT, HE5_PTinqpointF,HE5_PTINQPOINT, he5_inqpoint, PSTRING, PSTRING, PLONG)
FCALLSCFUN4(INT, HE5_PTattrinfoF,HE5_PTATTRINFO, he5_ptattrinfo, INT, PSTRING, PINT, PLONG)
FCALLSCFUN4(INT, HE5_PTgrpattrinfoF,HE5_PTGRPATTRINFO, he5_ptgrpattrinfo, INT, PSTRING, PINT, PLONG)
FCALLSCFUN5(INT, HE5_PTlocattrinfoF,HE5_PTLOCATTRINFO, he5_ptlocattrinfo, INT, PSTRING, PSTRING, PINT, PLONG)
FCALLSCFUN3(LONG, HE5_PTinqattrsF,HE5_PTINQATTRS, he5_ptinqattrs, INT, PSTRING, PLONG)
FCALLSCFUN3(LONG, HE5_PTinqgrpattrsF,HE5_PTINQGRPATTRS, he5_ptinqgrpattrs, INT, PSTRING, PLONG)
FCALLSCFUN4(LONG, HE5_PTinqlocattrsF,HE5_PTINQLOCATTRS, he5_ptinqlocattrs, INT, PSTRING, PSTRING, PLONG)
FCALLSCFUN3(INT, HE5_PTnumtypeinfoF, HE5_PTNUMTYPEINFO, he5_ptnumtypeinfo, INT, INT, INTV)

/* Point utility routines */
FCALLSCFUN5(LONG, HE5_PTc2fort,HE5_PTC2FORT, he5_ptc2fort, LONGV, INT, INT, PVOID, PVOID)
FCALLSCFUN5(LONG, HE5_PTfort2c,HE5_PTFORT2C, he5_ptfort2c, LONGV, INT, INT, PVOID, PVOID)


#endif







