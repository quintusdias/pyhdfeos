/*
 ----------------------------------------------------------------------------
 |    Copyright (C) 1999 Emergent IT Inc. and Raytheon Systems Company      |
 |                                                                          |
 |  Permission to use, modify, and distribute this software and its         |
 |  documentation for any purpose without fee is hereby granted, provided   |
 |  that the above copyright notice appear in all copies and that both that |
 |  copyright notice and this permission notice appear in supporting        |
 |                          documentation.                                  |
 ----------------------------------------------------------------------------
*/

#include <HE5_HdfEosDef.h>

/* The modification that needs _IGNORE_DIMSCALE_ preprocessor macro definition.
 It can be defined somewhere other than in EHapi.c, such as when configure script is executed.

Problem ~
We want to make existing HDF-EOS5 files read by the netCDF-4 library. Although
the netCDF-4 library is similar to the HDF-EOS5 library, the netCDF-4 library
cannot read HDF-EOS5 files mainly due to the lack of netCDF-4 dimensions. 
So, we added netCDF-4 dimensions to existing HDF-EOS5 files.

Attaching a netCDF-4 dimension scale to an HDF-EOS5 field introduces an
attribute, "DIMENSION_LIST".  Unfortunately, the type class of "DIMENSION_LIST"
is H5T_VLEN, and its element is of H5T_STD_REF_OBJ type. This type is not
allowed by the HDF-EOS5 library; so, users may get errors when accessing
that attribute. For example, HE5_GDlocattrinfo() fails saying

   Cannot retrieve information about Attribute "DIMENSION_LIST" associated
   with the "..." field.

Goal ~
Assuming that users do not have hard-coded list of attributes, they will call
HE5_GDinqlocattrs() to get the attribute names attached to a field. If
"DIMENSION_LIST" is not contained in the attribute names, users will not know
that the problematic "DIMENSION_LIST" attribute exists, which prevents users
from accessing that attribute.

Implementation Details ~
HE5_GDinqlocattrs() is the function users call to get the list of attribute
names. This HE5_GDinqlocattrs() will finally call HE5_EHobj_info() for each 
attribute, and we modified HE5_EHobj_info() function so that this function 
ignores an attribute if
   its class is H5T_VLEN, and
   its base type is H5T_STD_REF_OBJ

If these two conditions are satisfied, HE5_EHobj_info() will return immediately
without appending this attribute name to the list of attribute names.

*/

#ifndef _IGNORE_DIMSCALE_
#define _IGNORE_DIMSCALE_
#endif

#define HE5_EHIDOFFSET    67108864
#define HE5_VERSIONLEN          32
#define HE5_NEOSHDF            200
#define HE5_DYNAMIC_BLKSIZE    32000 /* size of each Structure 
					metadata block */
#define MetaBlocks             32 /* Max number of 32000 Structure 
				     metadata blocks */

/*int HDFEOS5ver_flg ; *//* will be set to 1 if the HDF-EOS5 version 
			    is later than 5.1.7, otherwise to 0 */
/*int     HE5_STATIC_BLKSIZE = HE5_BLKSIZE;*/ /* will be used, after mdifications
						 in EHopen, for buffer size memory
						 allocatins of StructMetadata.0 for
						 static method */
static int error_flg = 0;

typedef struct
{
  int               active;    /*  File open flag   */
  
  hid_t             HDFfid;    /*  HDF-EOS file ID  */
  hid_t             gid;       /* "HDFEOS" group ID */
  
  uintn             flags;     /*  File access flag */
  
  char              *filename; /*  File name        */
  int               DYNAMIC_flag; /*  Flage for checking for the compatibality
				      of StruvtMetadata with the version of
				      HDF-EOS5 */
  int               HE5_STATIC_BLKSIZE; /* will be used, after mdifications
					   in EHopen, for buffer size memory
					   allocatins of StructMetadata.0 for
					   static method */

}HE5_EHXTable;

HE5_EHXTable HE5_HeosTable[HE5_NEOSHDF];

typedef struct
{
  int               metafile_active;
  char              *saved_metabuf;
  long              metabuf_length;
  long              nmeta;
  int               metaread_flag;
}HE5_SAVED_METABUF;

HE5_SAVED_METABUF Stored_meta[HE5_NEOSHDF];

/* FORTRAN WRAPPER PROTOTYPES */

int   
HE5_EHidinfoF(int fileID, int *fid, int *gid);
double 
HE5_EHconvAngF(double inAngle, int code);
int
HE5_EHwrglatt(int fileID, char *attrname, int ntype, long fortcount[], void *datbuf);
int    
HE5_EHrdglatt(int fileID, char *attrname, void *datbuf);
int
HE5_EHglattinf(int fileID, char *attrname, int *ntype, long *fortcount);
long 
HE5_EHinqglatts(int fileID, char *attrnames, long *strbufsize);
int   
HE5_EHrdwrfileF(char *extfilename, uintn Flags, int ntype, long nelmnts, void *data);
int
HE5_EHinqglbtype(int fileID, char *attrname, int *Type, int *Class, int *Order, long *size);
herr_t 
HE5_EHinsertmeta_Static(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[]);
herr_t 
HE5_EHinsertmeta_Dynamic(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[]);
char  *
HE5_EHmetagroup_Static(hid_t fid , char *structname, char *structcode, char *groupname, char *metaptrs[]);
char  *
HE5_EHmetagroup_Dynamic(hid_t fid , char *structname, char *structcode, char *groupname, char *metaptrs[]);
herr_t
HE5_EHupdatemeta_Static(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[]);
herr_t
HE5_EHupdatemeta_Dynamic(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[]);
herr_t HE5_EHgetnmeta(hid_t gid, long *nmeta);


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|  FUNCTION:      HE5_EHchkptr                                                |
|  DESCRIPTION :  Checks if the passed pointer != NULL                        |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS:                                                                    |
|      p          void *              Pointer                                 |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  05/01/01   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_EHchkptr(void *p, char *name)
{
  herr_t   status = SUCCEED;
  char     errbuf[HE5_HDFE_ERRBUFSIZE];
  
  if(p == NULL)
    {
      status = FAIL;
      sprintf(errbuf,"INPUT ERROR: Parameter-pointer %s == NULL.\n", name);
      H5Epush(__FILE__, "HE5_EHchkptr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  return(status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|  FUNCTION:      HE5_EHchkname                                               |
|  DESCRIPTION :  Checks if the passed string doesn't contain illegal         |
|                                             characters                      |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS:                                                                    |
|      p          void *              Pointer                                 |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  06/01/01   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_EHchkname(char *p, char *name)
{
  herr_t   status = SUCCEED;
  char     errbuf[HE5_HDFE_ERRBUFSIZE];
 
  if(p == NULL)
    {
	  status = FAIL;
	  sprintf(errbuf,"HDF-EOS WARNING: The input parameter \"%s\" is a null pointer.\n", name);
	  H5Epush(__FILE__, "HE5_EHchkname", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  if( strchr(p,',') || strchr(p,';') || strchr(p,'/') )
	{
	  status = FAIL;
	  sprintf(errbuf,"HDF-EOS WARNING: The input string named \"%s\" contains illegal character.\n", name);
	  H5Epush(__FILE__, "HE5_EHchkname", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
  
  return(status);
}
 


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|  FUNCTION: HE5_EHdtype2mtype                                                |
|  DESCRIPTION : Takes file data type ID, converts it to a memory data type ID|
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      mtype      hid_t               Memory data type ID                     |
|  INPUTS:                                                                    |
|      dtype      hid_t               File data type ID                       |
|  OUTPUTS:                                                                   |
|             None                                                            |
|  NOTES:                                                                     |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  07/12/00   A.Muslimov    Original development.                             |
|  June 02    S.Zhao        Added an H5T_NATIVE_CHAR data type ID.            |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t 
HE5_EHdtype2mtype(hid_t dtype)
{
  hid_t      mtype   = FAIL;
  hid_t      nativetype;
  char       *errbuf = NULL;/* Error message buffer */


  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char *) calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHdtype2mtype", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
	  HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
	  return(FAIL);
    }

  nativetype = H5Tget_native_type(dtype, H5T_DIR_ASCEND);
  /* We cannot return nativetype because the caller will not 
     call H5Tclose() for it. */
  if (nativetype == FAIL) 
    {
      sprintf(errbuf,"The datatype \"%d\" is not supported (unknown).\n", (int)dtype);
      H5Epush(__FILE__, "HE5_EHdtype2mtype", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
    }
  else 
    {
      int i;
      hid_t typelists[] = { H5T_NATIVE_CHAR, H5T_NATIVE_SCHAR, H5T_NATIVE_UCHAR, H5T_NATIVE_SHORT, H5T_NATIVE_USHORT, H5T_NATIVE_INT, H5T_NATIVE_UINT, H5T_NATIVE_LONG, H5T_NATIVE_ULONG, H5T_NATIVE_LLONG, H5T_NATIVE_ULLONG, H5T_NATIVE_FLOAT, H5T_NATIVE_DOUBLE, H5T_NATIVE_LDOUBLE };
      for (i = 0; i < sizeof(typelists) / sizeof(typelists[0]); ++i) {
	if (H5Tequal(typelists[i], nativetype) > 0) {
	  mtype = typelists[i];
	  break;
	}
      }
      if (mtype == FAIL) {
	sprintf(errbuf,"The datatype \"%d\" is not supported (unknown).\n", (int)dtype);
	H5Epush(__FILE__, "HE5_EHdtype2mtype", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      }
      
      H5Tclose(nativetype);
  }
 
  free(errbuf);

  return(mtype);
}


/* Data type WRAPPERS */
/* ================== */


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|  FUNCTION: HE5_EHhid2long                                                   |
|  DESCRIPTION : Takes a scalar variable of 'hid_t' type, converts it to a    |
|                  scalar variable of 'long' type, and returns the value.     |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   long                                                        |
|  INPUTS:                                                                    |
|      invalue    hid_t                                                       |
|  OUTPUTS:                                                                   |
|             None                                                            |
|  NOTES:                                                                     |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHhid2long(hid_t invalue)
{
  long  outvalue = FAIL;
  
  if (sizeof(hid_t) == sizeof(int))
	outvalue = HE5_EHint2long(invalue);
  else if (sizeof(hid_t) == sizeof(long))
	outvalue = invalue;
  
  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHint2long                                                   |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'int' type, converts it to a      |
|                  scalar variable of 'long' type, and returns the value.     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   long                                                        |
|  INPUTS:                                                                    |
|      invalue    int                                                         |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHint2long(int invalue)
{
  herr_t    status   = FAIL;
  long      outvalue = FAIL;
  long      buf      = FAIL;


  memmove(&buf,&invalue,sizeof(int));

  status = H5Tconvert(H5T_NATIVE_INT, H5T_NATIVE_LONG, 1, &buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHint2long", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"int\" to \"long\" data type.");
	  return(status);
	}
 
  memmove(&outvalue,&buf,sizeof(long));

  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhid2int                                                    |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hid_t' type, converts it to a    |
|                  scalar variable of 'int' type, and returns the value.      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   int                                                         |
|  INPUTS:                                                                    |
|      invalue    hid_t                                                       |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_EHhid2int(hid_t invalue)
{
  int  outvalue = FAIL;
  
  if (sizeof(hid_t) == sizeof(int))
	outvalue = invalue;
  else if (sizeof(hid_t) == sizeof(long))
	outvalue = HE5_EHlong2int(invalue);
  
  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHint2hid                                                    |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'int' type, converts it to a      |
|                  scalar variable of 'hid_t' type, and returns the value.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hid_t                                                       |
|  INPUTS:                                                                    |
|      invalue    int                                                         |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t 
HE5_EHint2hid(int invalue)
{
  hid_t  outvalue = FAIL;
  
  if (sizeof(hid_t) == sizeof(int))
	outvalue = invalue;
  else if (sizeof(hid_t) == sizeof(long))
	outvalue = HE5_EHint2long(invalue);
  
  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHlong2hid                                                   |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'long' type, converts it to a     |
|                  scalar variable of 'hid_t' type, and returns the value.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hid_t                                                       |
|  INPUTS:                                                                    |
|      invalue    long                                                        |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t 
HE5_EHlong2hid(long invalue)
{
  hid_t  outvalue = FAIL;
  
  if (sizeof(hid_t) == sizeof(int))
	outvalue = HE5_EHlong2int(invalue);
  else if (sizeof(hid_t) == sizeof(long))
	outvalue = invalue;
  
  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHlong2int                                                   |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'long' type, converts it to a     |
|                  scalar variable of 'int' type, and returns the value.      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   int                                                         |
|  INPUTS:                                                                    |
|      invalue    long                                                        |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int     					    
HE5_EHlong2int(long invalue)
{
  
  herr_t                  status   = FAIL;
  
  int                     outvalue = FAIL;

  long                    *buf     = (long *)NULL;

  
  buf = (long *)calloc(1,sizeof(long));
		   
  memmove(buf,&invalue,sizeof(long));
				 
  status = H5Tconvert(H5T_NATIVE_LONG, H5T_NATIVE_INT, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHlong2int", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"long\" to \"int\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,sizeof(int));
				 
  free(buf);
	  
  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhid2hsize                                                  |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hid_t' type, converts it to a    |
|                  scalar variable of 'hsize_t' type, and returns the value.  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hsize_t                                                     |
|  INPUTS:                                                                    |
|      invalue    hid_t                                                       |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hsize_t 
HE5_EHhid2hsize(hid_t invalue)
{
  hsize_t  outvalue = 0;
  
  if (sizeof(hid_t) == sizeof(int))
	outvalue = HE5_EHint2hsize(invalue);
  else if (sizeof(hid_t) == sizeof(long) )
	outvalue = HE5_EHlong2hsize(invalue);

  return(outvalue);  
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHint2hsize                                                  |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'int' type, converts it to a      |
|                  scalar variable of 'hsize_t' type, and returns the value.  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hsize_t                                                     |
|  INPUTS:                                                                    |
|      invalue    int                                                         |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hsize_t      					    
HE5_EHint2hsize(int invalue)
{
  
  herr_t   status       = FAIL;
	
  int      *buf         = (int *)NULL;

  hsize_t  outvalue     = 0;

  size_t   maxsize      = 0;

  maxsize = MAX(sizeof(int),H5Tget_size(H5T_NATIVE_HSIZE));

  buf = (int *)calloc(1,maxsize);

  memmove(buf,&invalue, sizeof(int));
				 
  status = H5Tconvert(H5T_NATIVE_INT, H5T_NATIVE_HSIZE, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHint2hsize", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"int\" to \"hsize_t\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_HSIZE));

  free(buf);
	  
  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHlong2hsize                                                 |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'long' type, converts it to a     |
|                  scalar variable of 'hsize_t' type, and returns the value.  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hsize_t                                                     |
|  INPUTS:                                                                    |
|      invalue    long                                                        |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hsize_t       					    
HE5_EHlong2hsize(long invalue)
{
  
  herr_t   status       = FAIL;

  long     *buf         = (long *)NULL;

  hsize_t  outvalue     = 0;

  size_t   maxsize      = 0;

  maxsize = MAX(H5Tget_size(H5T_NATIVE_LONG),H5Tget_size(H5T_NATIVE_HSIZE));

  buf = (long *)calloc(1,maxsize);

  memmove(buf,&invalue,sizeof(long));
				 
  status = H5Tconvert(H5T_NATIVE_LONG, H5T_NATIVE_HSIZE, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHlong2hsize", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"long\" to \"hsize_t\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_HSIZE));
	 
  free(buf);
 
	  
  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhsize2hid                                                  |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hsize_t' type, converts it to a  |
|                  scalar variable of 'hid_t' type, and returns the value.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hid_t                                                       |
|  INPUTS:                                                                    |
|      invalue    hsize_t                                                     |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t      					    
HE5_EHhsize2hid(hsize_t invalue)
{
  hid_t  outvalue = FAIL;
  
  if (sizeof(hid_t) == sizeof(int))
	outvalue = HE5_EHhsize2int(invalue);
  else if (sizeof(hid_t) == sizeof(long) )
	outvalue = HE5_EHhsize2long(invalue);

  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhsize2long                                                 |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hsize_t' type, converts it to a  |
|                  scalar variable of 'long'  type, and returns the value.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   long                                                        |
|  INPUTS:                                                                    |
|      invalue    hsize_t                                                     |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHhsize2long(hsize_t invalue)
{
  
  herr_t   status       = FAIL;
  
  long     *buf         = (long *)NULL;
  long     outvalue     = FAIL;

  size_t   maxsize      = 0;


  maxsize = MAX(H5Tget_size(H5T_NATIVE_LONG),H5Tget_size(H5T_NATIVE_HSIZE));

  buf = (long *)calloc(1,maxsize);

  memmove(buf,&invalue,H5Tget_size(H5T_NATIVE_HSIZE));
				 
  status = H5Tconvert(H5T_NATIVE_HSIZE, H5T_NATIVE_LONG, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHhsize2long", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"hsize_t\" to \"long\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_LONG));
  
  free(buf);
	  
  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhsize2int                                                  |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hsize_t' type, converts it to a  |
|                  scalar variable of 'int'  type,  and returns the value.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   int                                                         |
|  INPUTS:                                                                    |
|      invalue    hsize_t                                                     |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_EHhsize2int(hsize_t invalue)
{
  
  herr_t  status       = FAIL;

  int     *buf         = (int *)NULL;
  int     outvalue     = FAIL;

  size_t  maxsize      = 0;

  maxsize = MAX(H5Tget_size(H5T_NATIVE_INT),H5Tget_size(H5T_NATIVE_HSIZE));

  buf = (int *)calloc(1,maxsize);

  memmove(buf,&invalue,H5Tget_size(H5T_NATIVE_HSIZE));
				 
  status = H5Tconvert(H5T_NATIVE_HSIZE, H5T_NATIVE_INT, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHhsize2int", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"hsize_t\" to \"int\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_INT));
	 
  free(buf);
	  
  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhid2hssize                                                 |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hid_t' type, converts it to a    |
|                  scalar variable of 'hssize_t' type, and returns the value. |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hssize_t                                                    |
|  INPUTS:                                                                    |
|      invalue    hid_t                                                       |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hssize_t      					    
HE5_EHhid2hssize(hid_t invalue)
{

  hssize_t  outvalue    = FAIL;
  
  if (sizeof(hid_t) == sizeof(int))
	outvalue = HE5_EHint2hssize(invalue);
  else if (sizeof(hid_t) == sizeof(long) )
	outvalue = HE5_EHlong2hssize(invalue);

  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHint2hssize                                                 |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'int' type, converts it to a      |
|                  scalar variable of 'hssize_t' type, and returns the value. |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hssize_t                                                    |
|  INPUTS:                                                                    |
|      invalue    int                                                         |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hssize_t      					    
HE5_EHint2hssize(int invalue)
{
  herr_t   status      = FAIL;
  
  int      *buf        = (int *)NULL;
  
  hssize_t outvalue    = FAIL;
  
  size_t   maxsize     = 0;   


  maxsize = MAX(sizeof(int),H5Tget_size(H5T_NATIVE_HSSIZE));

  buf = (int *)calloc(1,maxsize);
  
  memmove(buf,&invalue, sizeof(int));
  
  status = H5Tconvert(H5T_NATIVE_INT, H5T_NATIVE_HSSIZE, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHint2hssize", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"int\" to \"hssize\" data type.");
	  return(status);
	}
  
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_HSSIZE));
  
  free(buf);
  
  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHlong2hssize                                                |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'int' type, converts it to a      |
|                  scalar variable of 'hssize_t' type, and returns the value. |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hssize_t                                                    |
|  INPUTS:                                                                    |
|      invalue    long                                                        |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hssize_t
HE5_EHlong2hssize(long invalue)
{
  
  herr_t   status      = FAIL;

  long     *buf        = (long *)NULL;

  hssize_t outvalue    = FAIL;

  size_t   maxsize     = 0; 

  maxsize = MAX(sizeof(long),H5Tget_size(H5T_NATIVE_HSSIZE));

  buf = (long *)calloc(1,maxsize);

  memmove(buf,&invalue,sizeof(long));
				 
  status = H5Tconvert(H5T_NATIVE_LONG, H5T_NATIVE_HSSIZE, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHint2ullong", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"long\" to \"hssize_t\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_HSSIZE));
	 
  free(buf);
	  
  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhssize2hid                                                 |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hsize_t' type, converts it to a  |
|                  scalar variable of 'hid_t' type, and returns the value.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hid_t                                                       |
|  INPUTS:                                                                    |
|      invalue    hssize_t                                                    |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t      					    
HE5_EHhssize2hid(hssize_t invalue)
{
  hid_t  outvalue = FAIL;
  
  if (sizeof(hid_t) == sizeof(int))
	outvalue = HE5_EHhssize2int(invalue);
  else if (sizeof(hid_t) == sizeof(long) )
	outvalue = HE5_EHhssize2long(invalue);
  
  return(outvalue);  
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhssize2long                                                |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hsize_t' type, converts it to a  |
|                  scalar variable of 'long' type, and returns the value.     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   long                                                        |
|  INPUTS:                                                                    |
|      invalue    hssize_t                                                    |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHhssize2long(hssize_t invalue)
{
  
  herr_t   status       = FAIL;

  long     *buf         = (long *)NULL;
  long     outvalue     = FAIL;

  size_t   maxsize      = 0;

  maxsize = MAX(H5Tget_size(H5T_NATIVE_LONG),H5Tget_size(H5T_NATIVE_HSSIZE));

  buf = (long *)calloc(1,maxsize);

  memmove(buf,&invalue,H5Tget_size(H5T_NATIVE_HSSIZE));
				 
  status = H5Tconvert(H5T_NATIVE_HSSIZE, H5T_NATIVE_LONG, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHhssize2long", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"hssize_t\" to \"long\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_LONG));
	 
  free(buf);
	  
  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhssize2int                                                 |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hsize_t' type, converts it to a  |
|                  scalar variable of 'int' type, and returns the value.      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   int                                                         |
|  INPUTS:                                                                    |
|      invalue    hssize_t                                                    |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int  
HE5_EHhssize2int(hssize_t invalue)
{
  
  herr_t  status       = FAIL;

  int     *buf         = (int *)NULL;
  int     outvalue     = FAIL;

  size_t  maxsize      = 0;

  maxsize = MAX(H5Tget_size(H5T_NATIVE_INT),H5Tget_size(H5T_NATIVE_HSSIZE));

  buf = (int *)calloc(1,maxsize);

  memmove(buf,&invalue,H5Tget_size(H5T_NATIVE_HSSIZE));
				 
  status = H5Tconvert(H5T_NATIVE_HSSIZE, H5T_NATIVE_INT, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHhssize2int", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"hssize_t\" to \"int\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_INT));
	 
  free(buf);
	  
  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHint2ullong                                                 |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'int' type, converts it to a      |
|                  scalar variable of 'ullong' type, and returns the value.   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   ullong                                                      |
|  INPUTS:                                                                    |
|      invalue    int                                                         |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  Sept. 04   S.Zhao        Changed "long long" to "LONGLONG" for Windows.    |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
unsigned LONGLONG
HE5_EHint2ullong(int invalue)
{

  herr_t              status   = FAIL;
  unsigned LONGLONG   outvalue = 0;
  unsigned LONGLONG   buf      = 0;

  memmove(&buf,&invalue,sizeof(int));
  
  status = H5Tconvert(H5T_NATIVE_INT, H5T_NATIVE_ULLONG, 1, &buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHint2ullong", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"int\" to \"unsigned long long\" data type.");
	  return(status);
	}

  memmove(&outvalue,&buf,sizeof(unsigned LONGLONG));

  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHullong2long                                                |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'ullong' type, converts it to a   |
|                  scalar variable of 'long' type, and returns the value.     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   long                                                        |
|  INPUTS:                                                                    |
|      invalue    ullong                                                      |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  Sept. 04   S.Zhao        Changed "long long" to "LONGLONG" for Windows.    |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHullong2long(unsigned LONGLONG invalue)
{

  herr_t                  status   = FAIL;

  long                    outvalue = FAIL;
  long                    *buf     =  (long *)NULL;

  size_t                  maxsize  = 0;

  maxsize = MAX(sizeof(unsigned LONGLONG),sizeof(long));

  buf = (long *)calloc(1,maxsize);

  memmove(buf,&invalue,sizeof(unsigned LONGLONG));

  status = H5Tconvert(H5T_NATIVE_ULLONG, H5T_NATIVE_LONG, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHullong2long", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"unsigned long long\" to \"long\" data type.");
	  return(status);
	}
  
  memmove(&outvalue,buf,sizeof(long));

  free(buf);

  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhssize2hsize                                               |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hssize_t' type, converts it to a |
|                  scalar variable of 'hsize_t' type, and returns the value.  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hsize_t                                                     |
|  INPUTS:                                                                    |
|      invalue    hssize_t                                                    |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  Sept. 04   S.Zhao        Changed "long long" to "LONGLONG" for Windows.    |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hsize_t  
HE5_EHhssize2hsize(hssize_t invalue)
{
  
  herr_t     status       = FAIL;

  LONGLONG   *buf         = (LONGLONG *)NULL;

  hsize_t    outvalue     = 0;

  size_t     maxsize      = 0;

  maxsize = MAX(H5Tget_size(H5T_NATIVE_HSIZE),H5Tget_size(H5T_NATIVE_HSSIZE));

  buf = (LONGLONG *)calloc(1,maxsize);

  memmove(buf,&invalue,H5Tget_size(H5T_NATIVE_HSSIZE));
				 
  status = H5Tconvert(H5T_NATIVE_HSSIZE, H5T_NATIVE_HSIZE, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHhssize2hsize", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"hssize_t\" to \"hsize\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_HSIZE));
	 
  free(buf);
	  
  return(outvalue);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHhsize2hssize                                               |
|                                                                             |
|  DESCRIPTION : Takes a scalar variable of 'hsize_t' type, converts it to a  |
|                  scalar variable of 'hssize_t' type, and returns the value. |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|      outvalue   hssize_t                                                    |
|  INPUTS:                                                                    |
|      invalue    hsize_t                                                     |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ================================================= |
|  04/25/00   A.Muslimov    Original development.                             |
|  Sept. 04   S.Zhao        Changed "long long" to "LONGLONG" for Windows.    |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hssize_t  
HE5_EHhsize2hssize(hsize_t invalue)
{
  
  herr_t     status     = FAIL;

  LONGLONG   *buf       = (LONGLONG *)NULL;

  hssize_t   outvalue   = FAIL;

  size_t     maxsize    = 0;

  maxsize = MAX(H5Tget_size(H5T_NATIVE_HSIZE),H5Tget_size(H5T_NATIVE_HSSIZE));

  buf = (LONGLONG *)calloc(1,maxsize);

  memmove(buf,&invalue,H5Tget_size(H5T_NATIVE_HSIZE));
				 
  status = H5Tconvert(H5T_NATIVE_HSIZE, H5T_NATIVE_HSSIZE, 1, buf, NULL, H5P_DEFAULT);
  if (status == FAIL)
	{
	  H5Epush(__FILE__, "HE5_EHhsize2hssize", __LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot convert \"hsize_t\" to \"hssize\" data type.");
	  return(status);
	}
				 
  memmove(&outvalue,buf,H5Tget_size(H5T_NATIVE_HSSIZE));
	 
  free(buf);
	  
  return(outvalue);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHopen                                                       |
|                                                                             |
|  DESCRIPTION: opens HDF-EOS file and returns file handle                    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  fid            hid_t   None        HDF-EOS file ID                         |
|                                                                             |
|  INPUTS:                                                                    |
|  filename       char*   None        HDF-EOS file name string                |
|                                                                             |
|  flags          uintn   None        File access Flag                        |
|                                                                             |
|                 valid flags:        H5F_ACC_RDONLY                          |
|                                     H5F_ACC_RDWR                            |
|                                     H5F_ACC_TRUNC                           |
|                                                                             |
|                                                                             |
|  access_id      hid_t   None        File access property list ID (or        |
|                                     H5P_DEFAULT for the default I/O access  |
|                                     parameters.                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:    The dataset StructMetadata is not extandible. The next release   |
|            (V5.?)  will be supporting 'variable-length strings'.            |
|                                                                             |
|                                -----------                                  |
|                               | Root "/"  |                                 |
|                                -----------                                  |
|                               /           \                                 |
|               ----------------------    --------------------                |
|               | HDFEOS INFORMATION |    |      HDFEOS      |                |
|               ----------------------    --------------------                |
|               /            |            /   / \   \       \                 |
|              /             |           /   /   \   \       \                |
|    <HDFEOSVersion> [StructMetadata.0] /   /     \   \       \               |
|                                      /   |       |   \       \              |
|                            ----------  ------ ------- ------- --------      |
|                           |ADDITIONAL| |GRID| |POINT| |SWATH| |  ZA  |      |
|                            ----------  ------ ------- ------- --------      |
|                                /         :       :        :       :         |
|                               /        [dts]   [dts]    [dts]   [dts]       |
|                      -----------------                                      |
|                      |FILE_ATTRIBUTES|                                      |
|                      -----------------                                      |
|                             :                                               |
|                          <attrs>                                            |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  July 99  A.Muslimov    Modified to conform to the HDFEOS file structure    |
|  07.15.99  A.Muslimov   Implemented creating of two separate groups HDFEOS, |
|                         and HDFEOS INFORMATION. Added the calls to H5Tcopy  |
|                         and H5Tset_size(). Modified a block "Create dataset |
|                         StructMetadata.0 ... " to allocate memory for the   |
|                         BLKSIZE elements of metabuf array.                  |
|  9/24/99   A.Muslimov   Added some more error handling after function calls.|
|  02/07/00  A.Muslimov   Added error handling after function calls and       |
|                           "free(errbuf)".                                   |
|  May 01    A.Muslimov   Added "ADDITIONAL/FILE_ATTRIBUTES" group.           |
|  Nov 01    A.Muslimov   Changed the data space/datatype of Version attribute|
|                          Retaylored the OPEN/CREATE blocks.                 |
|  Jun 02    S.Zhao       Added some warnings for "ADDITIONAL/FILE_ATTRIBUTES"|
|                         groups.                                             |
|  05/17/04  Abe Taaheri  Modified to adopt alocation of metabuf in chunks of |
|                         HE5_DYNAMIC_BLKSIZE                                 |
|  Nov 04    S.Zhao       Modified to save tsize in HE5_HeosTable[HE5_NEOSHDF]|
|  Sep 06    Abe Taaheri  Added initialize of hdfeosVersion                   |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t
HE5_EHopen(const char *filename, uintn flags,  hid_t access_id)
{
  hid_t           fid       = FAIL;/* HDF-EOS file ID (RETURN VALUE)     */
  hid_t           HDFfid    = FAIL;/* HDF file ID                        */
  hid_t           sid1      = FAIL;/* Data space ID                      */
  hid_t           sid2      = FAIL;/* Data space ID                      */
  hid_t           atype     = FAIL;/* Data type ID                       */
  hid_t           attid     = FAIL;/* "HDFEOSVersion" Attribute ID       */
  hid_t           datid     = FAIL;/* "StructuralMetadata.0" dataset ID  */
  hid_t           gid       = FAIL;/* "HDFEOS" group ID                  */
  hid_t           ggid      = FAIL;/* "HDFEOS INFORMATION" group ID      */
  hid_t           adid      = FAIL;/* "ADDITIONAL" group ID              */
  hid_t           fagid     = FAIL;/* "FILE_ATTRIBUTES" group ID         */
  hid_t           i;		   /* Loop index                         */
  hid_t           iver;		   /* Loop index                         */
  herr_t          status    = FAIL;/* Status variable                    */

  int             nfileopen =  0;  /* "Number of files opened" Flag      */

  size_t          size      =  0;               /* Data size (bytes)     */

  char            *errbuf   = (char *)NULL;	    /* Error message buffer  */
  char            *metabuf  = (char *)NULL;	    /* Pointer to SM buffer  */
  char            hdfeosVersion[HE5_VERSIONLEN];/* HDFEOS version string */
  char            *version_buf = NULL;
  char            version_buf1[13];
  size_t          tsize     = 0;   /* size of dataset                       */
  int             HDFEOS5ver_flg ; /* will be set to 1 if the HDF-EOS5 version 
				      is later than 5.1.7, otherwise to 0 */
  int             newfile_num;

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
	  HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
	  return(FAIL);
    }

  for (i = 0; i < HE5_NEOSHDF; i++)
    {
          if (HE5_HeosTable[i].active == 0)
             Stored_meta[i].metaread_flag = 0;
    }

  /* Determine number of files currently opened */
  /* ------------------------------------------ */
  for (i = 0; i < HE5_NEOSHDF; i++)
	nfileopen += HE5_HeosTable[i].active;

  /* Setup file interface */
  /* -------------------- */
  if (nfileopen < HE5_NEOSHDF)
    {
      /* Open HDF-EOS file */
      /* ================= */
      if (flags == H5F_ACC_RDWR || flags == H5F_ACC_RDONLY)
	{
	  H5Eset_auto(NULL,NULL);
	  HDFfid = H5Fopen(filename, flags, access_id);
	  if(HDFfid < 0)
	    {
	      sprintf(errbuf,"File \"%s\" cannot be opened. \n", filename);  
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	}
      else if (flags == H5F_ACC_TRUNC)
	{
	  /* H5Eset_auto(NULL,NULL); */
	  HDFfid = H5Fcreate(filename,flags,H5P_DEFAULT,H5P_DEFAULT);
	  if(HDFfid < 0)
	    {
	      sprintf(errbuf,"File \"%s\" cannot be created. \n", filename);  
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_FILE,H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	}
      else 
	{
	  sprintf(errbuf,"Invalid file access flag. \n");  
	  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_FILE,H5E_CANTOPENFILE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);		  
	}
      
      if(flags == H5F_ACC_RDONLY ) 
	{
	  /* open group "HDFEOS" */ 
	  /* =================== */ 
	  gid = H5Gopen(HDFfid, "HDFEOS");
	  if(gid < 0)
	    {
	      sprintf(errbuf, "Group \"HDFEOS\" does not exist in \"%s\" file. \n", filename);
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  /* open group "ADDITIONAL" */ 
	  /* ======================= */  
	  adid = H5Gopen(gid, "ADDITIONAL");
	  if(adid < 0)
	    {
	      sprintf(errbuf, "Warning: Group \"ADDITIONAL\" does not exist in \"%s\" file, the hdfeos5 files produced by hdfeos5.0 or an earlier version. \n", filename);
	      printf("Warning: Group \"ADDITIONAL\" does not exist in \"%s\" file, the hdfeos5 files produced by hdfeos5.0 or an earlier version. \n", filename);
	      
	    }
	  else if(adid > 0)
	    {
	      
	      /* open group "FILE_ATTRIBUTES" */ 
	      /* ============================ */ 
	      fagid = H5Gopen(adid, "FILE_ATTRIBUTES");
	      if(fagid < 0)
		{
		  sprintf(errbuf, "Warning: Group \"FILE_ATTRIBUTES\" does not exist in \"%s\" file. \n", filename);
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(FAIL);
		} 
	    }
	  
	  /* open group "HDFEOS INFORMATION" */ 
	  /* =============================== */ 
	  ggid = H5Gopen(HDFfid,"HDFEOS INFORMATION");
	  if(ggid < 0)
	    {
	      sprintf(errbuf, "Group \"HDFEOS INFORMATION\" does not exist in \"%s\" file. \n", filename);
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    } 
	}
      else if (flags == H5F_ACC_RDWR)
	{
	  /* open group "HDFEOS" */
	  /* =================== */
	  gid = H5Gopen(HDFfid, "HDFEOS");
	  if(gid < 0)
	    {
	      sprintf(errbuf, "Group \"HDFEOS\" does not exist in \"%s\" file. \n", filename);
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  /* open group "ADDITIONAL" */
	  /* ======================= */
	  adid = H5Gopen(gid, "ADDITIONAL");
	  if(adid < 0)
	    {
	      sprintf(errbuf, "Warning: Group \"ADDITIONAL\" does not exist in \"%s\" file, need to create it. \n", filename);
	      printf("Warning: Group \"ADDITIONAL\" does not exist in \"%s\" file, need to create it. \n", filename);
	      
	      /* Create a group "ADDITIONAL" */
	      /* =========================== */
	      adid = H5Gcreate(gid, "ADDITIONAL", 0);
	      if (adid < 0)
		{
		  sprintf(errbuf, "Cannot create \"ADDITIONAL\" group. \n");
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CANTINIT, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(FAIL);
		}
	    }
	  
	  /* open group "FILE_ATTRIBUTES" */
	  /* ============================ */
	  fagid = H5Gopen(adid, "FILE_ATTRIBUTES");
	  if(fagid < 0)
	    {
	      sprintf(errbuf, "Group \"FILE_ATTRIBUTES\" does not exist in \"%s\" file, need to create it. \n", filename);
	      printf("Warning: Group \"FILE_ATTRIBUTES\" does not exist in \"%s\" file, need to create it. \n", filename);
	      
	      /* Create a group "FILE_ATTRIBUTES" */
	      /* ================================ */
	      fagid = H5Gcreate(adid, "FILE_ATTRIBUTES", 0);
	      if (fagid < 0)
		{
		  sprintf(errbuf, "Cannot create \"FILE_ATTRIBUTES\" group. \n");
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CANTINIT, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(FAIL);
		}
	    }
	  
	  /* open group "HDFEOS INFORMATION" */
	  /* =============================== */
	  ggid = H5Gopen(HDFfid,"HDFEOS INFORMATION");
	  if(ggid < 0)
	    {
	      sprintf(errbuf, "Group \"HDFEOS INFORMATION\" does not exist in \"%s\" file. \n", filename)
		;
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	}
      else if (flags == H5F_ACC_TRUNC)
	{
	  /* Create a group "HDFEOS" */
	  /* ======================= */
	  gid = H5Gcreate(HDFfid, "HDFEOS", 0);
	  if (gid < 0)
	    {
	      sprintf(errbuf, "Cannot create \"HDFEOS\" group. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  /* Create a group "ADDITIONAL" */
	  /* =========================== */
	  adid = H5Gcreate(gid, "ADDITIONAL", 0);
	  if (adid < 0)
	    {
	      sprintf(errbuf, "Cannot create \"ADDITIONAL\" group. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  /* Create a group "FILE_ATTRIBUTES" */
	  /* ================================ */
	  fagid = H5Gcreate(adid, "FILE_ATTRIBUTES", 0);
	  if (fagid < 0)
	    {
	      sprintf(errbuf, "Cannot create \"FILE_ATTRIBUTES\" group. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  /* Create a group "HDFEOS INFORMATION" */
	  /* =================================== */
	  ggid = H5Gcreate(HDFfid, "HDFEOS INFORMATION", 0);
	  if (ggid == FAIL)
	    {
	      sprintf(errbuf, "Cannot create \"HDFEOS INFORMATION\" group. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  /* Initialize hdfeosVersion */
	  
	  for( iver=0; iver < HE5_VERSIONLEN; iver++)
	    hdfeosVersion[iver]='\0';
	  

	  /* Set HDFEOS version number in file */
	  /* ================================= */ 
	  sprintf(hdfeosVersion, "%s.%s", "HDFEOS_5", HE5_HDFEOSVERSION); 
	  
	  /* create attribute "HDFEOSVersion" under "HDFEOS INFORMATION */
	  /* ========================================================== */
	  sid1 = H5Screate(H5S_SCALAR); 
	  if (sid1 == FAIL)
	    {
	      sprintf(errbuf, "Cannot create dataspace for \"HDFEOSVersion\" attribute. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASPACE, H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  atype = H5Tcopy(H5T_C_S1);
	  status = H5Tset_size(atype,HE5_VERSIONLEN);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot set the total size for atomic datatype. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATATYPE, H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  
	  /* Set up "HDFEOSVersion" attribute */
	  /* -------------------------------- */
	  attid = H5Acreate(ggid, "HDFEOSVersion", atype, sid1, H5P_DEFAULT); 
	  if (attid == FAIL)
	    {
	      sprintf(errbuf, "Cannot create \"HDFEOSVersion\" attribute. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_ATTR, H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  
	  /* Write attribute */
	  /* --------------- */
	  status = H5Awrite(attid, atype, hdfeosVersion); 
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write in data to the \"HDFEOSVerion\" attribute. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);  
	      return(FAIL);
	    }
	  
	  /*
           ******************************* 
	   *  Setup Structural Metadata  *
	   *******************************
	   */
	  metabuf = (char *)calloc(HE5_DYNAMIC_BLKSIZE, sizeof(char ));
	  if (metabuf == NULL)
	    {
	      sprintf(errbuf, "Cannot allocate memory for the metadata buffer. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      
	      return(FAIL);
	    }
	  
	  strcpy(metabuf, "GROUP=SwathStructure\n");
	  strcat(metabuf, "END_GROUP=SwathStructure\n");
	  strcat(metabuf, "GROUP=GridStructure\n");
	  strcat(metabuf, "END_GROUP=GridStructure\n");
	  strcat(metabuf, "GROUP=PointStructure\n");
	  strcat(metabuf, "END_GROUP=PointStructure\n");
	  strcat(metabuf, "GROUP=ZaStructure\n");
	  strcat(metabuf, "END_GROUP=ZaStructure\n");
	  strcat(metabuf, "END\n");
	  
	  /* 
	     -----------------------------------------------
	    | Create dataset "StructMetadata.0" and attach |
	    |    it to the "HDFEOS INFORMATION"  group     | 
	     ------------------------------------------------
	  */
	  
	  /* metalen = strlen(metabuf) + 1; */
	  /* Create dataspace */
	  /* ---------------- */
	  sid2    = H5Screate(H5S_SCALAR);
	  if (sid2 == FAIL)
	    {
	      sprintf(errbuf, "Cannot create dataspace for \"StructMetadata.0\" dataset. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASPACE, H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      free(metabuf);
	      return(FAIL);
	    }
	  
	  atype   = H5Tcopy(H5T_C_S1);
	  
	  /* size    = metalen; */
	  size    = HE5_DYNAMIC_BLKSIZE;		  
	  /* Set data type size */
	  /* ------------------ */
	  status  = H5Tset_size(atype,size);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot set the total size for atomic datatype. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATATYPE, H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      free(metabuf);
	      return(FAIL);
	    }
	  
	  metabuf[ HE5_DYNAMIC_BLKSIZE - 1 ] = '\0';
	  
	  /* Create "StructMetadata" dataset */
	  /* ------------------------------- */
	  datid   = H5Dcreate(ggid,"StructMetadata.0", atype, sid2, H5P_DEFAULT);
	  if (datid == FAIL)
	    {
	      sprintf(errbuf, "Cannot create \"StructMetadata.0\" dataset. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASET, H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      free(metabuf);			  
	      return(FAIL);
	    }
	  
	  /* ----------------------------------------------------------- */
	  /* ! The dataset extension for a string is not supported yet ! */
	  /*    the corresponding call is  H5Dextend(datid, &metalen);   */
	  /* ----------------------------------------------------------- */ 
	  
	  
	  /* 
	     -------------------------------------
	     |  Write data to "StructMetadata.0" | 
	     -------------------------------------
	  */      
	  status = H5Dwrite(datid,atype,H5S_ALL,H5S_ALL,H5P_DEFAULT,metabuf);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot write in data to the \"StructMetadata.0\" dataset. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      free(metabuf);			  
	      return(FAIL);
	    }		  
	  
	  free(metabuf);
	  
	  /* Release IDs */
	  /* ----------- */
	  status = H5Sclose(sid1);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataspace ID. \n");
	      H5Epush(__FILE__, "HE5_HE5_EHopen", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  status = H5Aclose(attid);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the attribute ID. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_ATTR, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	  status = H5Sclose(sid2);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataspace ID. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);			  
	      return(FAIL);
	    }
	  status = H5Dclose(datid);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataset ID. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      return(FAIL);
	    }
	}
      
      /* Close "HDFEOS INFORMATION" group */
      /* -------------------------------- */
      status = H5Gclose(ggid);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the \"HDFEOS INFORMATION\" group ID. \n");
	  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}	  
      
      /* Close "ADDITIONAL" group */
      /* ------------------------ */
      status = H5Gclose(adid);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the \"ADDITIONAL\" group ID. \n");
	  /*
	    H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);		  
	    return(FAIL);
	  */
	}
      
      /* Close "FILE_ATTRIBUTES" group */
      /* ----------------------------- */
      status = H5Gclose(fagid);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the \"FILE_ATTRIBUTES\" group ID. \n");
	  /*
	    H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);		  
	    return(FAIL);
	  */
	}
      
      
      /* Assign HDFEOS fid # & Load HDF fid tables */
      /* ========================================= */
      for (i = 0; i < HE5_NEOSHDF; i++)
	{
	  if (HE5_HeosTable[i].active == 0)
	    {
	      HE5_HeosTable[i].flags           = flags;
	      HE5_HeosTable[i].active          = 1;
	      HE5_HeosTable[i].gid             = gid;
	      HE5_HeosTable[i].HDFfid          = HDFfid;
	      fid                              = i + HE5_EHIDOFFSET;
	      HE5_HeosTable[i].filename = (char *)calloc((strlen(filename)+1), sizeof(char));
	      if (HE5_HeosTable[i].filename == NULL)
		{
		  sprintf(errbuf, "Cannot allocate memory for file name. \n");
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(FAIL);
		}
	      strcpy(HE5_HeosTable[i].filename, filename);
              newfile_num = i;
	      break;
	    }
	}
      
      if (flags == H5F_ACC_RDWR || flags == H5F_ACC_RDONLY)
	{
	  /* if version is before 5.1.7 set DYNAMIC_SMB
	     global flag to 1, Otherwise to 0.
	     
	     Starting with version 5.1.8 we use dynamic 
	     memory allocation for StructMetadata buffer */
	  
	  /* Allocate memory for error message buffer */
	  /* ---------------------------------------- */
	  version_buf = (char * )calloc( 64, sizeof(char));
	  if(version_buf == NULL)
	    {
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
	      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
	      return(FAIL);
	    }
	  
	  status = HE5_EHgetversion(fid, version_buf);
	  
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot get version. \n");
	      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      free(version_buf);
	      return(FAIL);
	    }
	  if(strlen(version_buf) < 12)
	    {
	      strcpy(version_buf1, version_buf);
	    }
	  else
	    {
	      strncpy(version_buf1, version_buf, 12);
	      version_buf1[12] = '\0';
	    }
	  
	  if ( strcmp(version_buf1, "HDFEOS_5.0") == 0 ||
	       strcmp(version_buf1, "HDFEOS_5.1") == 0 ||
	       strcmp(version_buf1, "HDFEOS_5.1.2") == 0 ||
	       strcmp(version_buf1, "HDFEOS_5.1.3") == 0 ||
	       strcmp(version_buf1, "HDFEOS_5.1.4") == 0 ||
	       strcmp(version_buf1, "HDFEOS_5.1.5") == 0 ||
	       strcmp(version_buf1, "HDFEOS_5.1.6") == 0 ||
	       strcmp(version_buf1, "HDFEOS_5.1.7") == 0 )
	    {
	      HDFEOS5ver_flg = 1; /* Static StructMetadata type */
	    }
	  else
	    {
	      HDFEOS5ver_flg = 0; /* Dynamic StructMetadata type */
	    }
	  free(version_buf);
	  version_buf = NULL;

	  if(HDFEOS5ver_flg == 1) /* if StructMetadata is static type, get
				     size of StructMetadata.0 and save it
				     in global parameter for later use */
	    {
	      /* Open "HDFEOS INFORMATION" group to get its zise for backward 
		 compatibality issue */
	      /* =========================================================== */
	      ggid     = H5Gopen(HDFfid, "HDFEOS INFORMATION");
	      if(ggid == FAIL)
		{
		  status = FAIL;
		  sprintf(errbuf,"Cannot open \"HDFEOS INFORMATION\" group. \n");
		  H5Epush(__FILE__, "HE5_EHopen",  __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  
		  return(status);
		}
	      
	      
	      /*
	       *--------------------------------------------------------------*
	       * Open dataset "StructMetadata.0", get datatype and space ID   *
	       *--------------------------------------------------------------*
	       */
	      datid = H5Dopen(ggid, "StructMetadata.0");
	      if( datid == FAIL)
		{
		  status = FAIL;
		  sprintf(errbuf,"Cannot open \"StructMetadata.0\" dataset. \n");
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  
		  return(status);
		}
	      
	      atype   = H5Dget_type(datid);
	      
	      if( atype == FAIL)
		{
		  status = FAIL;
		  sprintf(errbuf,"Cannot get the dataset datatype. \n");
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  
		  return(status);
		}
	      
	      /* Get dataset size */
	      /* ---------------- */
	      tsize   = H5Tget_size(atype);
	      if ( tsize == 0 )
		{
		  status = FAIL;
		  sprintf(errbuf, "The datatype is of ZERO size. \n"); 
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  
		  return( status );
		}

              HE5_HeosTable[newfile_num].HE5_STATIC_BLKSIZE = tsize;

	      /* Save tsize in global parameter for later use */

	      /*HE5_STATIC_BLKSIZE = tsize;*/
	      
	      status = H5Tclose(atype); 
	      if( status == FAIL)
		{
		  sprintf(errbuf, "Cannot release the data type ID.\n");
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  
		  return(status);
		}
	      status = H5Dclose(datid);
	      if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot release the dataset ID.\n");
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  
		  return(status);
		}
	      status = H5Gclose(ggid);
	      if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot release the group ID.\n");
		  H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  
		  return(status);
		}
	    }
	}
      else
	{
	  HDFEOS5ver_flg = 0; /* Dynamic StructMetadata type */
	}
      /* Load table with the flag */
      /* ========================================= */
      for (i = 0; i < HE5_NEOSHDF; i++)
	{
	  if ((HE5_HeosTable[i].active == 1) && 
	      (strcmp(HE5_HeosTable[i].filename, filename) == 0) && 
              (fid == i + HE5_EHIDOFFSET))
	    {
	      HE5_HeosTable[i].DYNAMIC_flag   = HDFEOS5ver_flg;

	      break;
	    }
	}
    }
  else
    {
      /* Too many files opened */
      /* --------------------- */
      fid = FAIL;
      sprintf(errbuf,"Illegal attempt to open more than %d files simultaneously. \n", HE5_NEOSHDF);
      H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  free(errbuf);
  errbuf = NULL;
  
  return(fid);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHchkfid                                                     |
|                                                                             |
|  DESCRIPTION: Checks for valid file id and returns HDF file ID and          |
|               "HDFEOS" group id                                             |
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
|  ======   ============  =================================================   |
|  July 99  A.Muslimov    Modified to conform to the HDFEOS file structure    |
|  9/29/99  A.Muslimov    Added dynamic memory allocation for error message   |
|                         buffer.                                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_EHchkfid(hid_t fid, const char *name,  hid_t *HDFfid,  hid_t *grpID, uintn *access)
{
  herr_t     status = FAIL;		    /* routine return status variable */

  hid_t      fid0   = 0;		    /* HDFEOS file ID-offset          */  

  char       *errbuf;		  	    /* Error message buffer           */


  CHECKPOINTER(name);

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char *) calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHchkfid", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
	  HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
	  return(FAIL);
    }

  /* Check for valid HDFEOS file ID range */
  /* ------------------------------------ */
  if (fid < HE5_EHIDOFFSET || fid > HE5_NEOSHDF + HE5_EHIDOFFSET)
    {
	  status = FAIL;
	  sprintf(errbuf,"Invalid file ID: %d. ID should range from %d to  %d .\n", fid, HE5_EHIDOFFSET, HE5_NEOSHDF + HE5_EHIDOFFSET);
	  H5Epush(__FILE__, "HE5_EHchkfid", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf); 
	  HE5_EHprint(errbuf, __FILE__, __LINE__); 
    } 
  else
    {
	  /* Compute "reduced" file ID */
	  /* ------------------------- */
	  fid0 = fid % HE5_EHIDOFFSET;
	  if( HE5_HeosTable[ fid0 ].active == 0) 
        {
		  status = FAIL;
		  sprintf(errbuf,"HE5_EHchkid: File ID %d not active (%s).\n",fid, name);
		  H5Epush(__FILE__, "HE5_EHchkfid", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf); 
		  HE5_EHprint(errbuf, __FILE__, __LINE__); 
        }
	  else
        {
		  /* Get HDF file ID, group ID and file access from external arrays */
		  /* ============================================================== */ 
		  *HDFfid = HE5_HeosTable[ fid0 ].HDFfid;
		  *grpID  = HE5_HeosTable[ fid0 ].gid;
		  *access = HE5_HeosTable[ fid0 ].flags;
		  status  = SUCCEED;
        }
    }

  free(errbuf);
  errbuf = NULL;

 COMPLETION:
  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHidinfo                                                     |
|                                                                             |
|  DESCRIPTION: Gets HDF ID and group ID from HDF-EOS ID                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS file ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  HDFfid         hid_t               HDF File ID                             |
|  gid            hid_t               group ID                                |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_EHidinfo(hid_t fid, hid_t *HDFfid, hid_t *gid)
{
  herr_t          status = FAIL;	/* routine return status variable */
  uintn           access = 0;	    /* file access code               */

  /* Call HE5_EHchkfid to get HDF-EOS interface IDs */
  /* ---------------------------------------------- */
  status = HE5_EHchkfid(fid, "HE5_EHidinfo", HDFfid, gid, &access);

  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHfilename                                                   |
|                                                                             |
|  DESCRIPTION: Returns HDF filename                                          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t   None       return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS file id                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  filename       char*               HDF-EOS file name                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHfilename(hid_t  fid, char *filename)
{
  herr_t       status = SUCCEED;  /* routine return status variable */

  if( filename == NULL || HE5_HeosTable[fid % HE5_EHIDOFFSET].filename == NULL) 
	status = FAIL;
  else 
	strcpy(filename, HE5_HeosTable[fid % HE5_EHIDOFFSET].filename);

  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHgetversion                                                 |
|                                                                             |
|  DESCRIPTION: Returns HDF-EOS version string                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS file id                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  version        char*               HDF-EOS version string                  |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  8/23/99  A. Muslimov   Initialized status to -1, and checked for the       |
|                         statuses returned by H5Tclose() and H5Aclose().     |
|  9/29/99  A.Muslimov    Added dynamic memory allocation for error message   |
|                         buffer and  atttribute name string.                 |
|  2/01/00  A.Muslimov    Fixed a bug associated with a pointing to the wrong |
|                         group and aborting a call.                          |
|  2/07/00  A.Muslimov    Added free() calls.                                 |
|  Nov  01  A.Muslimov    Commented out two blocks, and changed the attribute |
|                          data type to H5T_NATIVE_CHAR.                      |
|  Aug 02   S.Zhao        Fixed a bug regarding 'HDFEOSVersion' represented   |
|                         as a series of integers, not a character string.    |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHgetversion(hid_t fid, char *version)
{
  herr_t      status   = FAIL;  /* routine return status variable */

  uintn       access   =  0;	/* Access code                    */

  hid_t       gid      = FAIL;	/* "HDFEOS" group ID              */ 
  hid_t       att_id   = FAIL;	/* attribute ID                   */
  hid_t       HDFfid   = FAIL;	/* HDF-EOS file ID                */
  hid_t       InfGrpID = FAIL;	/* "HDFEOS INFORMATION" group ID  */
  hid_t       atype    = FAIL;  /* Attribute data type ID         */
  hid_t       mtype    = FAIL;  /* Attribute memory data type ID  */

  char        *errbuf   = (char *)NULL;/* error message buffer    */

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE,  sizeof(char));
  if(errbuf == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
	  HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
	  return(FAIL);
    }

  /* Get HDF-EOS file ID and "HDFEOS" group ID */
  /* ========================================= */
  status = HE5_EHchkfid(fid,"HE5_EHgetversion", &HDFfid, &gid, &access);
  if(status == FAIL)
    {
	  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_FILE, H5E_NOTFOUND, "Invalid file ID. \n");
	  HE5_EHprint("Error: Invalid file ID, occured", __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
    }

  /* Open "HDFEOS/HDFEOS INFORMATION" group */
  /* -------------------------------------- */
  InfGrpID = H5Gopen(HDFfid, "HDFEOS INFORMATION");
  if(InfGrpID == FAIL)
    {
	  status = FAIL;
	  sprintf(errbuf, "Cannot open \"HDFEOS INFORMATION\" group. \n");
	  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(status);
    }        

  if(status == SUCCEED)
    {
	  /* Open attribute "HDFEOSVersion" */
	  /* ============================== */
	  att_id = H5Aopen_name(InfGrpID, "HDFEOSVersion");
	  if(att_id == FAIL)
		{
		  status = FAIL;
		  sprintf(errbuf,"Cannot get the \"HDFEOSVersion\" attribute ID.");
		  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(status);
		}

	  atype  = H5Aget_type(att_id);
	  if( atype == FAIL)
		{
		  status = FAIL;
		  sprintf(errbuf,"Cannot get the \"HDFEOSVersion\" attribute data type ID.");
		  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(status);
		}
	  
	  /* Get the memory data type ID */
	  /* --------------------------- */
	  mtype = HE5_EHdtype2mtype(atype);

          if (mtype < 0)
          {
          	atype  = H5Tcopy(H5T_C_S1);
          	if( atype == FAIL)
                {
                  status = FAIL;
                  sprintf(errbuf,"Cannot copy the \"HDFEOSVersion\" attribute datatype.");
                  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
                  HE5_EHprint(errbuf, __FILE__, __LINE__);
                  free(errbuf);
 
                  return(status);
                }
 
          	status = H5Tset_size(atype, HE5_VERSIONLEN);
          	if( status == FAIL)
                {
                  sprintf(errbuf,"Cannot set the total size for the \"HDFEOSVersion\" attribute.");
                  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
                  HE5_EHprint(errbuf, __FILE__, __LINE__);
                  free(errbuf);
 
                  return(status);
                }
          	/* Read out version attribute */
          	/* ========================== */
          	status = H5Aread(att_id, atype, version);
          	if( status == FAIL)
                {
                  sprintf(errbuf,"Cannot read out the \"HDFEOSVersion\" attribute.");
                  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
                  HE5_EHprint(errbuf, __FILE__, __LINE__);
                  free(errbuf);
 
                  return(status);
                }
          }
	  else
          {
	  	/* Read out version attribute */
	  	/* ========================== */
	  	status = H5Aread(att_id, mtype, (void *)version);
	  	if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot read out the \"HDFEOSVersion\" attribute.");
		  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(status);
		}
          }
       
	  /* Release the attribute datatype ID */
	  /* --------------------------------- */
	  status = H5Tclose(atype);
	  if( status == FAIL)
	    {
		  sprintf(errbuf,"Cannot release the \"HDFEOSVersion\" attribute datatype ID.");
		  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(status);
		}
			
	  /* Release the attribute ID */
	  /* ------------------------ */
	  status = H5Aclose(att_id);
	  if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot release the \"HDFEOSVersion\" attribute ID.");
		  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_ATTR, H5E_CLOSEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(status);
		}

	  status = H5Gclose(InfGrpID);
	  if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot release the \"HDFEOS INFORMATION\" group ID.");
		  H5Epush(__FILE__, "HE5_EHgetversion", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  return(status);
		}
    }

  free(errbuf);
  errbuf = NULL;
    
  return (status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHconvAng                                                    |
|                                                                             |
|  DESCRIPTION: Angle conversion Utility                                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  outAngle       double              Output Angle value                      |
|                                                                             |
|  INPUTS:                                                                    |
|  inAngle        double              Input Angle value                       |
|  code           int                 Conversion code                         |
|                                       HDFE_RAD_DEG (0)                      |
|                                       HDFE_DEG_RAD (1)                      |
|                                       HDFE_DMS_DEG (2)                      |
|                                       HDFE_DEG_DMS (3)                      |
|                                       HDFE_RAD_DMS (4)                      |
|                                       HDFE_DMS_RAD (5)                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|     None                                                                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jun 96   Joel Gales    Original Programmer                                 |
|  Feb 97   Joel Gales    Correct "60" min & "60" sec in _DMS conversion      |
|  Feb 06   Abe Taaheri   Modified checking sec not to exceed 60.             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
double 
HE5_EHconvAng(double inAngle, int code)
{
    long      min = 0;        /* Truncated Minutes      */
    long      deg = 0;        /* Truncated Degrees      */

    double    sec      = 0.;  /* Seconds                */
    double    outAngle = 0.;  /* Angle in desired units */
    double    pi  = 3.14159265358979324;/* Pi           */
    double    r2d = 180 / pi;     /* Rad-deg conversion */
    double    d2r = 1 / r2d;      /* Deg-rad conversion */

    switch (code)
    {

        /* Convert radians to degrees */
        /* -------------------------- */
    case HE5_HDFE_RAD_DEG:
        outAngle = inAngle * r2d;
        break;

        /* Convert degrees to radians */
        /* -------------------------- */
    case HE5_HDFE_DEG_RAD:
        outAngle = inAngle * d2r;
        break;


        /* Convert packed degrees to degrees */
        /* --------------------------------- */
    case HE5_HDFE_DMS_DEG:
        deg = (long)(inAngle / 1000000);
        min = (long)((inAngle - deg * 1000000) / 1000);
        sec = (inAngle - deg * 1000000 - min * 1000);
        outAngle = deg + min / 60.0 + sec / 3600.0;
        break;


        /* Convert degrees to packed degrees */
        /* --------------------------------- */
    case HE5_HDFE_DEG_DMS:
        deg = (long)inAngle;
        min = (long)((inAngle - deg) * 60);
        sec = (inAngle - deg - min / 60.0) * 3600;
	/*
        if ((int)sec == 60)
        {
            sec = sec - 60;
            min = min + 1;
        }
	*/
	if ( fabs(sec - 0.0) < 1.e-7 )
	  {
	    sec = 0.0;
	  }

        if ( (fabs(sec - 60) < 1.e-7 ) || ( sec > 60.0 ))
        {
	  sec = sec - 60;
	  min = min + 1;
	  if(sec < 0.0)
	    {
	      sec = 0.0;
	    }
        }
        if (min == 60)
        {
            min = min - 60;
            deg = deg + 1;
        }
        outAngle = deg * 1000000 + min * 1000 + sec;
        break;


        /* Convert radians to packed degrees */
        /* --------------------------------- */
    case HE5_HDFE_RAD_DMS:
        inAngle = inAngle * r2d;
        deg = (long)inAngle;
        min = (long)((inAngle - deg) * 60);
        sec = ((inAngle - deg - min / 60.0) * 3600);
	/*
        if ((int)sec == 60)
        {
            sec = sec - 60;
            min = min + 1;
        }
	*/
	if ( fabs(sec - 0.0) < 1.e-7 )
	  {
	    sec = 0.0;
	  }

        if ( (fabs(sec - 60) < 1.e-7 ) || ( sec > 60.0 ))
        {
	  sec = sec - 60;
	  min = min + 1;
	  if(sec < 0.0)
	    {
	      sec = 0.0;
	    }
        }
        if (min == 60)
        {
            min = min - 60;
            deg = deg + 1;
        }
        outAngle = deg * 1000000 + min * 1000 + sec;
        break;


        /* Convert packed degrees to radians */
        /* --------------------------------- */
    case HE5_HDFE_DMS_RAD:
        deg = (long)(inAngle / 1000000);
        min = (long)((inAngle - deg * 1000000) / 1000);
        sec = (inAngle - deg * 1000000 - min * 1000);
        outAngle = deg + min / 60.0 + sec / 3600.0;
        outAngle = outAngle * d2r;
        break;
    }
    return (outAngle);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHparsestr                                                   |
|                                                                             |
|  DESCRIPTION: String Parser Utility                                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  count          long                Number of string entries                |
|                                                                             |
|  INPUTS:                                                                    |
|  instring       char*               Input string                            |
|  delim          char                string delimitor                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  pntr           char*               Pointer array to beginning of each      |
|                                     string entry                            |
|  len            size_t              Array of string entry lengths           |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date       Programmer   Description                                       |
|  ========   ============  ================================================= |
|  04/19/00   A.Muslimov    Changed type of 'slen','i','prevDelimPos' and     |
|                              'len[]' from long to size_t.                   |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHparsestr(const char *instring, char delim, char *pntr[], size_t len[])
{
  long           count        = 0;/* Number of elements in string list */

  herr_t         status   = FAIL; /* Return status variable            */
  
  size_t         slen         = 0;/* String length                     */
  size_t         i;               /* Loop index                        */
  size_t         prevDelimPos = 0;/* Previous delimitor position       */
  
  char           *delimitor   = (char *)NULL;/* Pointer to delimitor   */
  
  CHECKPOINTER(instring);

  /* Get length of input string list & Point to first delimitor */
  /* ---------------------------------------------------------- */
  slen      = strlen(instring);
  delimitor = strchr(instring, delim);
  
  /* If NULL string set count to zero otherwise set to 1 */
  /* --------------------------------------------------- */
  count = (slen == 0) ? 0 : 1;
  
  /* if string pointers are requested set first one to beginning of string */
  /* --------------------------------------------------------------------- */
  if (&pntr[0] != NULL)
	pntr[0] = (char *)instring;
  
  /* If delimitor not found ... */
  /* -------------------------- */
  if (delimitor == NULL)
    {
	  /* if string length requested then set to input string length */
	  /* ---------------------------------------------------------- */
	  if (len != NULL)
		len[0] = slen;
	}
  else
	/* Delimitors Found */
	/* ---------------- */
	{
	  /* Loop through all characters in string */
	  /* ------------------------------------- */
	  for (i = 1; i < slen; i++)
        {
		  /* If character is a delimitor ... */
		  /* ------------------------------- */
		  if (instring[i] == delim)
            {

			  /* If string pointer requested */
			  /* --------------------------- */
			  if (&pntr[0] != NULL)
                {
				  /* if requested then compute string length of entry */
				  /* ------------------------------------------------ */
				  if (len != NULL)
					len[count - 1] = i - prevDelimPos;

				  /* Point to beginning of string entry */
				  /* ---------------------------------- */
				  pntr[count] = (char *)instring + i + 1;
                }
			  /* Reset previous delimitor position and increment counter */
			  /* ------------------------------------------------------- */
			  prevDelimPos = i + 1;
			  count++;
            }
        }

	  /* Compute string length of last entry */
	  /* ----------------------------------- */
	  if (&pntr[0] != NULL && len != NULL)
		len[count - 1] = i - prevDelimPos;
    }
  
 COMPLETION:
  return(count);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHstrwithin                                                  |
|                                                                             |
|  DESCRIPTION: Searchs for string within target string                       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  indx           long                Element index (0 - based)               |
|                                                                             |
|  INPUTS:                                                                    |
|  target         char                Target string                           |
|  search         char                Search string                           |
|  delim          char                Delimitor                               |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  10/05/99 A.Muslimov    Added dynamical memory allocation for the buffers.  |
|  10/18/99 A.Muslimov    Replaced memcpy() by memmove() to avoid a problem   |
|                         when arguments 1 and 2 overlap in memory.           |
|  02/07/00 A.Muslimov    Added free() calls before return(FAIL) and more     |
|                         error handlings.                                    |
|  04/19/00 A.Muslimov    Changed type of '*slen' from long to size_t.        |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHstrwithin(char *target, char *search, char delim)
{

  long         indx     = FAIL;       /* Index of searched element (RETURN) */
  long         nentries = 0;	      /* Number of entries in search string */

  herr_t       status   = FAIL;       /* Return status variable             */

  size_t       *slen = (size_t *)NULL;/* Pointer to string length array     */

  int          found = 0;		      /* Target string found flag           */

  char         **ptr;			      /* Pointer to string pointer array    */
  char         *buffer = (char *)NULL;/* Buffer to hold "test" string entry */
  char         *errbuf = (char *)NULL;/* buffer for error message           */
     

  CHECKPOINTER(target);
  CHECKPOINTER(search);

  /* Allocate memory for error message buffer */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHstrwithin", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
	  HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);

	  return(FAIL);
    }

  /* Allocate memory for buffer */
  buffer = (char * )calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if(buffer == NULL)
    {
	  sprintf(errbuf,"Cannot allocate memory for buffer.");
	  H5Epush(__FILE__, "HE5_EHstrwithin", __LINE__, H5E_FILE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
     
	  return(FAIL);
    }
 

  /* Count number of entries in search string list */
  /* --------------------------------------------- */
  nentries = HE5_EHparsestr(search, delim, NULL, NULL);
  if(nentries == 0)
    {
	  sprintf(errbuf,"Input test string has no entries.");
	  H5Epush(__FILE__, "HE5_EHstrwithin", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(buffer);
     
	  return(FAIL);
    }


  /* Allocate string pointer and length arrays */
  /* ----------------------------------------- */
  ptr = (char **) calloc(nentries, sizeof(char *));
  if(ptr == NULL)
    {
	  sprintf(errbuf,"Cannot allocate memory for a string pointer.");
	  H5Epush(__FILE__, "HE5_EHstrwithin", __LINE__, H5E_FILE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(buffer);
     
	  return(FAIL);
    }

  slen = (size_t *) calloc(nentries, sizeof(size_t));
  if(slen == NULL)
    {
	  sprintf(errbuf,"Cannot allocate memory for a string length pointer.");
	  H5Epush(__FILE__, "HE5_EHstrwithin", __LINE__, H5E_FILE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(buffer);
	  free(ptr);
     
	  return(FAIL);
    }

  /* Parse search string */
  /* ------------------- */
  nentries = HE5_EHparsestr(search, delim, ptr, slen);
  if(nentries == 0)
    {
	  sprintf(errbuf,"Input test string has no entries.");
	  H5Epush(__FILE__, "HE5_EHstrwithin", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(buffer);
	  free(ptr);
	  free(slen);
     
	  return(FAIL);
    }

  /* Loop through all elements in search string list */
  /* ----------------------------------------------- */
  for (indx = 0; indx < nentries; indx++)
    {
	  /* Copy string entry into buffer */
	  /* ----------------------------- */
	  memmove(buffer, ptr[indx], slen[indx]);
	  buffer[slen[indx]] = 0;
	  

	  /* Compare target string with string entry */
	  /* --------------------------------------- */
	  if (strcmp(target, buffer) == 0)
        {
		  found = 1;
		  break;
        }
    }
  
  /* If not found set return to FAIL */
  /* ------------------------------- */
  if (found == 0)
	indx = FAIL;


  free(slen);
  free(ptr);
  free(errbuf);
  free(buffer);
 
 COMPLETION:   
  return(indx);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHloadliststr                                                |
|                                                                             |
|  DESCRIPTION: Builds list string from string array                          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ptr            char                String pointer array                    |
|  nentries       long                Number of string array elements         |
|  delim          char                Delimitor                               |
|                                                                             |
|  OUTPUTS:                                                                   |
|  liststr        char                Output list string                      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  10/18/99 A.Muslimov    Replaced memcpy() by memmove() to avoid a problem   |
|                         when arguments 1 and 2 overlap in memory.           |
|  04/19/00 A.Muslimov    Changed 'slen', 'off' types from long to size_t.    |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHloadliststr(char *ptr[], long nentries, char *liststr, char delim)
{
  herr_t          status = SUCCEED;   /* routine return status variable   */
  
  long            i;                  /* Loop index                       */
  
  size_t          off  = 0;           /* Position of next entry in a list */
  size_t          slen = 0;           /* String entry length              */
  
  char            dstr[2];            /* string containing "delim" value  */
  
  dstr[0] = delim;
  dstr[1] = '\0';
  
  
  /* Loop through all entries in string array */
  /* ---------------------------------------- */
  for (i = 0; i < nentries; i++)
    {
	  /* Get string length of string array entry */
	  /* --------------------------------------- */
	  slen = strlen(ptr[i]);

	  /* Copy string entry to string list */
	  /* -------------------------------- */
	  memmove(liststr + off, ptr[i], slen + 1);

	  /* Concatenate with delimitor */
	  /* -------------------------- */
	  if (i != nentries - 1)
        {
		  strcat(liststr, dstr);
        }
	  /* Get position of next entry for string list */
	  /* ------------------------------------------ */
	  off += slen + 1;
	}
  
  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHrevflds                                                    |
|                                                                             |
|  DESCRIPTION: Reverses elements in a string list                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  dimlist        char*               Original dimension list                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|  revdimlist     char*               Reversed dimension list                 |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  04/19/00 A.Muslimov    Changed type of 'slen' from long to size_t.         |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHrevflds(char *dimlist, char *revdimlist)
{
  herr_t       status    = FAIL;      /* routine return status variable     */
  
  long         indx      = 0;         /* Loop index                         */
  long         nentries  = 0;         /* Number of entries in search string */
  
  size_t       *slen = (size_t *)NULL;/* Pointer to string length array     */
  
  char         **ptr;                 /* Pointer to string pointer array    */
  char         *tempPtr     = (char *)NULL;/* Temporary string pointer      */
  char         *tempdimlist = (char *)NULL;/* Temporary dimension list      */
  

  CHECKPOINTER(dimlist);
  
  /* Copy dimlist into temp dimlist */
  /* ------------------------------ */
  tempdimlist = (char *)calloc((strlen(dimlist) + 1), sizeof(char) );
  if(tempdimlist == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHrevflds", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
	  HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	  return(FAIL);
    }

  strcpy(tempdimlist, dimlist);
    
  /* Count number of entries in search string list */
  /* --------------------------------------------- */
  nentries = HE5_EHparsestr(tempdimlist, ',', NULL, NULL);
  
  /* Allocate string pointer and length arrays */
  /* ----------------------------------------- */
  ptr = (char **) calloc(nentries, sizeof(char *));
  if(ptr == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHrevflds", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
	  HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	  if(tempdimlist != NULL) free(tempdimlist);
	  return(FAIL);
    }
  
  slen = (size_t *)calloc(nentries, sizeof(size_t));
  if(slen == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHrevflds", __LINE__, H5E_RESOURCE,H5E_NOSPACE, "Can not allocate memory");
	  HE5_EHprint("Error: Can not allocate memory, occured", __FILE__, __LINE__);
	  if (ptr != NULL) free(ptr);
	  if (tempdimlist != NULL) free(tempdimlist);
	  return(FAIL);
    }
  
  
  /* Parse search string */
  /* ------------------- */
  nentries = HE5_EHparsestr(tempdimlist, ',', ptr, slen);
  
  /* Reverse entries in string pointer array */
  /* --------------------------------------- */
  for (indx = 0; indx < nentries / 2; indx++)
    {
	  tempPtr   = ptr[indx];
	  ptr[indx] = ptr[nentries - 1 - indx];
	  ptr[nentries - 1 - indx] = tempPtr;
    }
  
  
  /* Replace comma delimitors by nulls */
  /* --------------------------------- */
  for (indx = 0; indx < nentries - 1; indx++)
    {
	  *(ptr[indx] - 1) = 0;
    }
  
  
  /* Build new string list */
  /* --------------------- */
  status = HE5_EHloadliststr(ptr, nentries, revdimlist, ',');
  
  free(slen);
  free(ptr);
  free(tempdimlist);

 COMPLETION:  
  return(status);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHcntOBJECT                                                  |
|                                                                             |
|  DESCRIPTION: Determines number of OBJECTs in metadata GROUP                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  count          long                Number of OBJECTs in GROUP              |
|                                                                             |
|  INPUTS:                                                                    |
|  metabur        char                Begin & end metadata pointer array      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
| END_PROLOG                                                                  |
-----------------------------------------------------------------------------*/
long 
HE5_EHcntOBJECT(char *metabuf[])
{
  long           count    = 0;           /* Counter (RETURN)                */

  herr_t         status   = FAIL;        /* Return status variable          */
  
  char           *metaptr = (char *)NULL;/* Beginning of metadata section   */
  char           *endptr  = (char *)NULL;/* End of metadata section         */
  char           *tempptr = (char *)NULL;/* Pointer within metadata section */
  
  CHECKPOINTER(metabuf[0]);
  CHECKPOINTER(metabuf[1]);
  
  /* Get Pointers to beginning and ending of metadata section */
  /* -------------------------------------------------------- */
  metaptr = metabuf[0];
  endptr  = metabuf[1];
  
  
  /* Find number of "END_OBJECT" strings within section */
  /* -------------------------------------------------- */
  tempptr = metaptr;
  
  while (tempptr < endptr && tempptr != NULL)
    {
	  tempptr = strstr(tempptr + 1, "END_OBJECT");
	  count++;
    }
  count--;

 COMPLETION:  
  return(count);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHcntGROUP                                                   |
|                                                                             |
|  DESCRIPTION: Determines number of GROUPs in metadata GROUP                 |
|                                                                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  count          long                Number of GROUPs in GROUP               |
|                                                                             |
|  INPUTS:                                                                    |
|  metabur        char                Begin & end metadata pointer array      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long  
HE5_EHcntGROUP(char *metabuf[])
{
  long            count    = 0;           /* Counter (RETURN)                */

  herr_t          status   = FAIL;        /* Return status variable          */

  char            *metaptr = (char *)NULL;/* Beginning of metadata section   */
  char            *endptr  = (char *)NULL;/* End of metadata section         */
  char            *tempptr = (char *)NULL;/* Pointer within metadata section */
  
  CHECKPOINTER(metabuf[0]);
  CHECKPOINTER(metabuf[1]);
  
  /* Get Pointers to beginning and ending of metadata section */
  /* -------------------------------------------------------- */
  metaptr = metabuf[0];
  endptr  = metabuf[1];
  
  /* Find number of "END_GROUP" strings within section */
  /* ------------------------------------------------- */
  tempptr = metaptr;
  
  while (tempptr < endptr && tempptr != NULL)
    {
	  tempptr = strstr(tempptr + 1, "END_GROUP");
	  count++;
    }
  count--;

 COMPLETION:  
  return (count);
}





/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHmetalist                                                   |
|                                                                             |
|  DESCRIPTION: Converts string list to metadata list                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  instring       char*               Input string list                       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  outstring      char*               Output metadata string                  |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  10/18/99 A.Muslimov    Replaced memcpy() by memmove() to avoid a problem   |
|                         when arguments 1 and 2 overlap in memory.           |
|  04/19/00 A.Muslimov    Changed type of '*slen', 'listlen' from long to     |
|                            size_t.                                          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHmetalist(char *instring, char *outstring)
{

  herr_t        status   = FAIL;           /* routine return status variable     */
  
  long          i;                         /* Loop index                         */
  long          nentries =  0;             /* Number of entries in search string */
  
  size_t        listlen  =  1;             /* String list length                 */
  size_t        *slen    = (size_t *)NULL; /* Pointer to string length array     */
  
  char          **ptr;                     /* Pointer to string pointer array    */
  

  CHECKPOINTER(instring);

  /* Count number of entries in search string list */
  /* --------------------------------------------- */
  nentries = HE5_EHparsestr(instring, ',', NULL, NULL);
  if(nentries == 0)
    {
	  H5Epush(__FILE__, "HE5_EHmetalist", __LINE__, H5E_FUNC, H5E_BADVALUE, "NULL input string");
	  HE5_EHprint("Error: NULL input string, occured", __FILE__, __LINE__);
	  return(FAIL);
    }
  
  
  /* Allocate string pointer  */
  /* ------------------------ */
  ptr = (char **) calloc(nentries, sizeof(char *));
  if(ptr == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHmetalist", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory. \n");
	  HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	  return(FAIL);
    }


  /* Allocate length array */
  /* --------------------- */
  slen = (size_t *) calloc(nentries, sizeof(size_t));
  if(slen == (size_t *)NULL)
    {
	  H5Epush(__FILE__, "HE5_EHmetalist", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory. \n");
	  HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	  if (ptr != NULL) free(ptr);
	  return(FAIL);
    }
  
  
  /* Parse input string */
  /* ------------------ */
  nentries = HE5_EHparsestr(instring, ',', ptr, slen);
  if(nentries == 0)
    {
	  H5Epush(__FILE__, "HE5_EHmetalist", __LINE__, H5E_ARGS, H5E_BADVALUE, "NULL input string. \n");
	  HE5_EHprint("Error: NULL input string, occured", __FILE__, __LINE__);
	  if (ptr != NULL) free(ptr);
	  if (slen != NULL) free(slen);
	  return(FAIL);
    }
  
  /* Start output string with leading "(" */
  /* ------------------------------------ */
  strcpy(outstring, "(");
  
  /* Loop through all entries */
  /* ------------------------ */
  for (i = 0; i < nentries; i++)
    {
	  /* Add double quote (") to output string */
	  /* ------------------------------------- */
	  strcat(outstring, "\"");
	  listlen++;

	  /* Add input string entry to output string */
	  /* --------------------------------------- */
	  memmove(outstring + listlen, ptr[i], slen[i]);
	  listlen += slen[i];
	  outstring[listlen] = 0;


	  /* Add closing double quote (") to output string */
	  /* --------------------------------------------- */
	  strcat(outstring, "\"");
	  listlen++;
	  outstring[listlen] = 0;


	  /* Add comma delimitor to output string */
	  /* ------------------------------------ */
	  if (i != (nentries - 1))
        {
		  strcat(outstring, ",");
		  listlen++;
        }
	  /* Place null terminator in output string */
	  /* -------------------------------------- */
	  outstring[listlen] = 0;
    }
  
  
  /* End output string with trailing ")" */
  /* ----------------------------------- */
  strcat(outstring, ")");
  status = SUCCEED;
  
  free(ptr);
  free(slen);
  
 COMPLETION:
  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHinsertmeta                                                 |
|                                                                             |
|  DESCRIPTION: Writes metadata                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|    status       herr_t  None        return status variable (0 for success,  |
|                                          -1 for failure)                    |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|    fid          hid_t   None        HDF-EOS file ID                         |
|    structname   char*   None        Object name string                      |
|    structcode   char*   None        "s" for a swath, "g" for a grid,        |
|                                     "p" for a point, and "z" for a za.      |
|    metacode     long    None        Code of a metadata block to insert to.  |
|    metastr      char*   None        Buffer containing metadata information  |
|                                      to insert.                             |
|    metadata[]   hsize_t None        Array of data values                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|    None                                                                     |
|                                                                             |
|  NOTES:   There is memory allocation of HE5_STATIC_BLKSIZE chunk for the    |
|           dataset "StructMetadata.0" when the buffer is allocated           |
|           statically (old scheme), or memory allocation is in cuncks        |
|           of HE5_DYNAMIC_BLKSIZE (new scheme)  allocated dynamically.       |
|           The chuncks will be written to datasets "StructMetadata.0",       |
|           "StructMetadata.1", "StructMetadata.2", etc.                      |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  05/17/04   Abe Taaheri  Initial version                                    |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHinsertmeta(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[]) 
{
  herr_t   status = SUCCEED;
  int      HDFEOS5ver_flg ; /* will be set to 1 if the HDF-EOS5 version 
			       is later than 5.1.7, otherwise to 0 */
  /* Get HDFEOS5ver_flg flag for the file */
  /* --------------- */
  HDFEOS5ver_flg = HE5_HeosTable[fid%HE5_EHIDOFFSET].DYNAMIC_flag;

  if(HDFEOS5ver_flg == 1)
    {
      status = HE5_EHinsertmeta_Static(fid, structname, structcode, metacode, 
				       metastr, metadata);
    }
  else if(HDFEOS5ver_flg == 0)
    {
      status = HE5_EHinsertmeta_Dynamic(fid, structname, structcode, metacode, 
					metastr, metadata);
    }

  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHinsertmeta_Static                                          |
|                                                                             |
|  DESCRIPTION: Writes metadata using a static buffer                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|    status       herr_t  None        return status variable (0 for success,  |
|                                          -1 for failure)                    |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|    fid          hid_t   None        HDF-EOS file ID                         |
|    structname   char*   None        Object name string                      |
|    structcode   char*   None        "s" for a swath, "g" for a grid,        |
|                                     "p" for a point, and "z" for a za.      |
|    metacode     long    None        Code of a metadata block to insert to.  |
|    metastr      char*   None        Buffer containing metadata information  |
|                                      to insert.                             |
|    metadata[]   hsize_t None        Array of data values                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|    None                                                                     |
|                                                                             |
|  NOTES:   There is memory allocation of HE5_STATIC_BLKSIZE chunk for the    |
|           dataset "StructMetadata.0"                                        |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  07/16/99 A.Muslimov    Replaced the 1st argument by fid (file ID). Did     |
|                         the same change in the call to H5Dopen(). Modified  |
|                         the first two calls to HDF5 lib.func. and commented |
|                         out the fourth and fifth calls. Allocated a fixed   |
|                         chunk of memory of BLKSIZE for the output metabuf[] |
|                         array.                                              |
|  08/26/99 A.Muslimov    Added if() condition after the H5Dread() H5Dwrite() |
|                         calls.                                              |
|  09/07/99 A.Muslimov    Allocated a BLKSIZE bytes for the newbuf[] array.   |
|  09/13/99 A.Muslimov    Checked for the value (nentries) returned by        |
|                         EHparsestr(). Checked for the status returned by    |
|                         EHmetalist().                                       |
|  9/29/99  A.Muslimov    Added dynamic memory allocation for error message   |
|                         buffer.                                             |
|  01/06/00 A.Muslimov    Changed the datatype of parameter "metadata[]"      |
|                         from int32_t to hsize_t to allow passing the        |
|                         Unlimited dimension value. Modified the correspon - |
|                         ding sprintf() calls.                               |
| 02/07/00  A.Muslimov    Added free() before return(FAIL).                   |
| 04/19/00  A.Muslimov    Changed type of 'slen' from long to size_t.         |
| 09/05/00  A.Muslimov    Added conversion of 'fid' to 'HDFfid'. In the call  |
|                         H5Gopen(fid, ...) replaced 'fid' by 'HDFfid'.       |
| 12/12/00  A.Muslimov    Updated to add "case 5" ("Profile Fields").         |
| Jan 2004  S.Zhao        Added a FAIL status for redefining a dimension.     |
| Mar 2004  S.Zhao        Modified for redefining a dimension.                |
| May 2005  S.Zhao        Added data type string designators in the dataset   |
|                         "StructMetadata.0".                                 |
|  Feb 06   Abe Taaheri   Casted Switch argument to long for HP11             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHinsertmeta_Static(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[]) 
{
  herr_t         status   = FAIL;    /* routine return status variable            */

  int            i;                  /* Loop index                                */
 
  hid_t          gid      = FAIL;    /* "HDFEOS INFORMATION" group ID             */
  hid_t          meta_id  = FAIL;    /* "StructuralMetadat.0" group ID            */
  hid_t          atype    = FAIL;    /* Data type ID                              */
  hid_t          aspace   = FAIL;    /* Data space ID                             */
  hid_t          HDFfid   = FAIL;    /* HDF5 file ID                              */

  size_t         slen[HE5_DTSETRANKMAX];/* String length array                    */
  
  hsize_t        tempdim  =  0;      /* Temporary variable                        */
  
  long           count    = 0;       /* Objects/Groups counter                    */
  long           offset   = 0;       /* Offset insertion position of new metadata */
  long           nentries = 0;       /* Number of comma separated string entries  */
  
  char           *metabuf = NULL;    /* Pointer to Structural Metadata array      */
  char           *begptr  = NULL;    /* Pointer to beginning of metadata section  */
  char           *metaptr = NULL;    /* Metadata pointer                          */
  char           *prevmetaptr = NULL;/* Previous position of metadata pointer     */
  char           *ptr[HE5_DTSETRANKMAX];/* String pointer array                   */
  char           *metaArr[2]={NULL,NULL};/* Array of metadata positions           */
  char           *colon   = NULL;    /* Colon position                            */
  char           *colon2  = NULL;    /* 2nd colon position                        */
  char           *slash   = NULL;    /* Slash postion                             */
  char           *utlstr  = NULL;    /* Utility string                            */
  char           *utlstr1 = NULL;    /* Utility string 1                          */
  char           *utlstr2 = NULL;    /* Utility string 2                          */
  char           *newbuf  = NULL;    /* updated metadata string                   */
  char           *type    = NULL;    /* data type string                          */
  char           *errbuf  = NULL;    /* Error message buffer                      */
  char           *dimptr  = NULL;    /* Dimension pointer                         */
  char           *newmetastr = NULL; /* Modified metadata string                  */
  char           *endptr  = NULL;    /* Pointer to ending of metadata section     */
  char           *newmetaptr = NULL; /* Updated metadata pointer                  */
  char           *begptr2  = NULL;   /* Pointer 2 to beginning of metadata section*/
  int            len1, len2, difflen = 0;

  

  CHECKPOINTER(structname);
  CHECKPOINTER(structcode);
  CHECKPOINTER(metastr);


  /* Get HDF file ID */
  /* --------------- */
  HDFfid = HE5_HeosTable[fid%HE5_EHIDOFFSET].HDFfid;

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
	  HE5_EHprint("Error: Cannot allocate memory for error buffe, occuredr", __FILE__, __LINE__);
	  return(FAIL);
    }
  
  /* Allocate memory for data type string */
  /* ------------------------------------ */
  type = (char * )calloc( HE5_HDFE_TYPESTRSIZE, sizeof(char));
  if(type == NULL)
    {
	  sprintf(errbuf,"Cannot allocate memory for data type string. \n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
     
	  return(FAIL);
    }
  
  
  /* Open "HDFEOS INFORMATION" group */
  /* =============================== */
  gid     = H5Gopen(HDFfid, "HDFEOS INFORMATION");
  if(gid == FAIL)
	{
      status = FAIL;
      sprintf(errbuf,"Cannot open \"HDFEOS INFORMATION\" group. \n");
      H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
      
      return(status);
	}
  
  
  /*
   *--------------------------------------------------------------*
   * Open dataset "StructMetadata.0", get datatype and space ID   *
   *--------------------------------------------------------------*
   */
  meta_id = H5Dopen(gid, "StructMetadata.0"); 
  if( meta_id == FAIL)
	{
      status = FAIL;
      sprintf(errbuf,"Cannot open \"StructMetadata.0\" dataset. \n");
      H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);

      return(status);
	}
  atype   = H5Dget_type(meta_id);
  if( atype == FAIL)
    {
      status = FAIL;
      sprintf(errbuf,"Cannot get the dataset datatype. \n");
      H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);

      return(status);
    }
  aspace  = H5Dget_space(meta_id);
  if( aspace == FAIL )
    {
      status = FAIL;
      sprintf(errbuf,"Cannot get the dataset dataspace. \n");
      H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);

      return(status);
    }
  
  
  /*
	 
     NOTE: this call is now supported for a string and is used now 
           in HE5_EHattrinfo to get the number of elements for string array
	npoints = H5Sget_simple_extent_npoints(aspace);
	   it will be implemented for memory allocation for metabuf[] array
	 
   */
   
  /*
   *-------------------------------------------------------------*
   *   Allocate memory for the output array and Read attribute   *
   *-------------------------------------------------------------*
   */
  
  metabuf  = (char *)calloc( HE5_HeosTable[fid%HE5_EHIDOFFSET].HE5_STATIC_BLKSIZE, sizeof(char));
  if( metabuf == NULL )
    {
	  sprintf(errbuf,"Cannot allocate memory for meta buffer. \n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  status = FAIL;
	  free(errbuf);
	  free(type);

	  return(status);
    }
  
  utlstr   = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if( utlstr == NULL )
    {
	  sprintf(errbuf,"Cannot allocate memory for utility string buffer. \n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  status = FAIL;
	  free(errbuf);
	  free(type);
	  free(metabuf);
	
	  return(status);

    }
  
  utlstr2  = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if( utlstr2 == NULL )
    {
	  sprintf(errbuf,"Cannot allocate memory for the 2nd utility string buffer. \n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  status = FAIL;
	  free(errbuf);
	  free(type);
	  free(metabuf);
	  free(utlstr);
	
	  return(status);

    }
  
  status = H5Dread(meta_id, atype,H5S_ALL, H5S_ALL, H5P_DEFAULT,metabuf);
  if( status == FAIL)
    {
	  sprintf(errbuf, "Cannot read structural metadata. \n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(type);
	  free(metabuf);
	  free(utlstr);
	  free(utlstr2);

	  return(status);
    }
  
  
  
  /* Find HDF-EOS structure "root" group in metadata */
  /* ----------------------------------------------- */
  /* Setup proper search string */
  if (strcmp(structcode, "s") == 0)
    {
	  strcpy(utlstr, "GROUP=SwathStructure");
    }    
  else if (strcmp(structcode, "g") == 0)
    {
	  strcpy(utlstr, "GROUP=GridStructure");
    }
  else if (strcmp(structcode, "p") == 0)
    {
	  strcpy(utlstr, "GROUP=PointStructure");
    }
  else if (strcmp(structcode, "z") == 0)
    {
          strcpy(utlstr, "GROUP=ZaStructure");
    }
  

  /* Use string search routine (strstr) to move through metadata */
  /* ----------------------------------------------------------- */
  metaptr = strstr(metabuf, utlstr);

  if ((int)abs((int)metacode) == 0)
   {
     utlstr1  = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
     if( utlstr1 == NULL )
      {
          sprintf(errbuf,"Cannot allocate memory for the 2d utility string buffer. \n");
          H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          status = FAIL;
          free(errbuf);
          free(type);
          free(metabuf);
          free(utlstr);
          free(utlstr2);
 
          return(status);
 
      }

     if (strcmp(structcode, "s") == 0)
      {
          strcpy(utlstr1, "END_GROUP=SwathStructure");
      }
     else if (strcmp(structcode, "g") == 0)
      {
          strcpy(utlstr1, "END_GROUP=GridStructure");
      }
     else if (strcmp(structcode, "p") == 0)
      {
          strcpy(utlstr1, "END_GROUP=PointStructure");
      }
     else if (strcmp(structcode, "z") == 0)
      {
          strcpy(utlstr1, "END_GROUP=ZaStructure");
      }

     newmetaptr = (char *)malloc(sizeof(char)*(strlen(metaptr)+1));
     strcpy(newmetaptr, metaptr);
     endptr = strstr(newmetaptr, utlstr1);
     endptr++;
     *endptr = '\0';
     free(utlstr1);
     utlstr1 = NULL;

   }

  /* Find specific (named) structure */
  /* ------------------------------- */
  if (metacode < 1000)
	{
	  /* Save current metadata pointer */
	  /* ----------------------------- */
	  prevmetaptr = metaptr;
		
	  /* First loop for "old-style" (non-ODL) metadata string */
	  /* ---------------------------------------------------- */
	  if (strcmp(structcode, "s") == 0)
		{
		  sprintf(utlstr, "%s%s%s", "SwathName=\"", structname, "\"");
		}
	  else if (strcmp(structcode, "g") == 0)
		{
		  sprintf(utlstr, "%s%s%s", "GridName=\"", structname, "\"");
		}
	  else if (strcmp(structcode, "p") == 0)
		{
		  sprintf(utlstr, "%s%s%s", "PointName=\"", structname, "\"");
		}
          else if (strcmp(structcode, "z") == 0)
                {
                  sprintf(utlstr, "%s%s%s", "ZaName=\"", structname, "\"");
                }
	
        
	  /* Perform string search */
	  /* --------------------- */
	  metaptr = strstr(metaptr, utlstr);
          if ((int)abs((int)metacode) == 0)
           {
             len1 = strlen(newmetaptr);
             newmetaptr = strstr(newmetaptr, utlstr);
             len2 = strlen(newmetaptr);
             difflen = len1 - len2;
           }


	  /*
	   *--------------------------------------------------------------------*
	   * If not found then return to previous position in metadata and look *
	   *             for "new-style" (ODL) metadata string                  *
	   *--------------------------------------------------------------------*
	   */         
	  if (metaptr == NULL)
		{
		  sprintf(utlstr, "%s%s%s", "GROUP=\"", structname, "\"");
		  metaptr = strstr(prevmetaptr, utlstr);
		}
	}
  

  /* Searching for geo fields (3), data fields (4), profile fields (5), */
  /* or point fields (11) convert type code to string designator        */
  /* ------------------------------------------------------------------ */
  if (metacode == 3 || metacode == 4 || metacode == 5 || metacode == 11)
    {
#ifndef HP9000 /* HP11 has problem with hsize_t type as switch argumet */
      switch (metadata[0])
#else
	switch ((long)metadata[0])
#endif
          {
          case 0:
              strcpy(type, "H5T_NATIVE_INT");
              break;
          case 1:
              strcpy(type, "H5T_NATIVE_UINT");
              break;
          case 2:
              strcpy(type, "H5T_NATIVE_SHORT");
              break;
          case 3:
              strcpy(type, "H5T_NATIVE_USHORT");
              break;
          case 4:
              strcpy(type, "H5T_NATIVE_SCHAR");
              break;
          case 5:
              strcpy(type, "H5T_NATIVE_UCHAR");
              break;
          case 6:
              strcpy(type, "H5T_NATIVE_LONG");
              break;
          case 7:
              strcpy(type, "H5T_NATIVE_ULONG");
              break;
          case 8:
              strcpy(type, "H5T_NATIVE_LLONG");
              break;
          case 9:
              strcpy(type, "H5T_NATIVE_ULLONG");
              break;
          case 10:
              strcpy(type, "H5T_NATIVE_FLOAT");
              break;
          case 11:
              strcpy(type, "H5T_NATIVE_DOUBLE");
              break;
          case 12:
              strcpy(type, "H5T_NATIVE_LDOUBLE");
              break;
          case 13:
              strcpy(type, "H5T_NATIVE_INT8");
              break;
          case 14:
              strcpy(type, "H5T_NATIVE_UINT8");
              break;
          case 15:
              strcpy(type, "H5T_NATIVE_INT16");
              break;
          case 16:
              strcpy(type, "H5T_NATIVE_UINT16");
              break;
          case 17:
              strcpy(type, "H5T_NATIVE_INT32");
              break;
          case 18:
              strcpy(type, "H5T_NATIVE_UINT32");
              break;
          case 19:
              strcpy(type, "H5T_NATIVE_INT64");
              break;
          case 20:
              strcpy(type, "H5T_NATIVE_UINT64");
              break;
          case 21:
              strcpy(type, "H5T_NATIVE_B8");
              break;
          case 22:
              strcpy(type, "H5T_NATIVE_B16");
              break;
          case 23:
              strcpy(type, "H5T_NATIVE_B32");
              break;
          case 24:
              strcpy(type, "H5T_NATIVE_B64");
              break;
          case 25:
              strcpy(type, "H5T_NATIVE_HSIZE");
              break;
          case 26:
              strcpy(type, "H5T_NATIVE_HERR");
              break;
          case 27:
              strcpy(type, "H5T_NATIVE_HBOOL");
              break;
          case 56:
              strcpy(type, "H5T_NATIVE_CHAR");
              break;
          case 57:
              strcpy(type, "HE5T_CHARSTRING");
              break;
          }
    }

  
  /* Metadata Section Switch */
  /* ----------------------- */
  switch ((int)abs((int)metacode))
	{
	  
    case 0:

	  /* Dimension Section */
	  /* ================= */
          /* Find beginning of metadata section */
          /* ---------------------------------- */
          strcpy(utlstr, "\t\tGROUP=Dimension");
          begptr = strstr(metaptr, utlstr);
          begptr2 = strstr(newmetaptr, utlstr);

          /* Search for dimension name */
          /* ------------------------- */
          newmetastr = (char *)malloc(sizeof(char)*strlen(metastr)+6);
          strcpy(newmetastr, "\"");       
          strcat(newmetastr, metastr);
          strcat(newmetastr, "\""); 
          dimptr = strstr(begptr2, newmetastr);
          free(newmetastr); 
          newmetastr = NULL;

          if (dimptr != NULL)
		{
                  status = FAIL;
          	  sprintf(errbuf,"Cannot redefine the dimension \"%s\". \n", metastr);
                  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
                  HE5_EHprint(errbuf, __FILE__, __LINE__);
                  free(errbuf);
                  free(type);
                  free(metabuf);
                  free(utlstr);
                  free(utlstr2);
                  free(newmetaptr - difflen);
                  return(status);
		}
	  else
		{
                  /* Find ending of metadata section */
                  /* ------------------------------- */
	  	  strcpy(utlstr, "\t\tEND_GROUP=Dimension");
	  	  metaptr = strstr(metaptr, utlstr);
		
	  	  /* Count number of existing entries and increment */
	  	  /* ---------------------------------------------- */
	  	  metaArr[0] = begptr;
	  	  metaArr[1] = metaptr;
	  	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  	  /* Build metadata entry string */
	  	  /* --------------------------- */
	  	  sprintf(utlstr, "%s%li%s%s%s%li%s%li%s", "\t\t\tOBJECT=Dimension_",count,"\n\t\t\t\tDimensionName=\"",&metastr[0],"\"\n\t\t\t\tSize=", (long)metadata[0],"\n\t\t\tEND_OBJECT=Dimension_", count, "\n");
		
		}

          if (newmetaptr != NULL)
           {
              free(newmetaptr - difflen);
              newmetaptr = NULL;
           }

	  break;
		
		
    case 1:

	  /* DimensionMap Section */
	  /* ==================== */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=DimensionMap");
	  begptr = strstr(metaptr, utlstr);

	  strcpy(utlstr, "\t\tEND_GROUP=DimensionMap");
	  metaptr = strstr(metaptr, utlstr);


	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

	  /* Initialize slen[] array */
	  /* ----------------------- */
	  for (i = 0; i < HE5_DTSETRANKMAX; i++)
		slen[ i ] = 0;

	  /* Find slash within input mapping string and replace with NULL */
	  /* ------------------------------------------------------------ */
	  nentries = HE5_EHparsestr(metastr, '/', ptr, slen);
	  if( nentries == 0)
		{
		  status = FAIL;
		  sprintf(errbuf,"Input metadata string has no entries. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);
			
		  return(status);
		}

	  metastr[slen[0]] = 0;
		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%li%s%li%s%li%s",
			  "\t\t\tOBJECT=DimensionMap_", count,
			  "\n\t\t\t\tGeoDimension=\"", &metastr[0],
			  "\"\n\t\t\t\tDataDimension=\"", &metastr[slen[0] + 1],
			  "\"\n\t\t\t\tOffset=", (long)metadata[0],
			  "\n\t\t\t\tIncrement=", (long)metadata[1],
			  "\n\t\t\tEND_OBJECT=DimensionMap_", count, "\n");
	  break;

	case 2:

	  /* IndexDimensionMap Section */
	  /* ========================= */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=IndexDimensionMap");
	  begptr = strstr(metaptr, utlstr);
		
	  strcpy(utlstr, "\t\tEND_GROUP=IndexDimensionMap");
	  metaptr = strstr(metaptr, utlstr);
		
	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;
		

	  /* Find slash within input mapping string and replace with NULL */
	  /* ------------------------------------------------------------ */
	  nentries = HE5_EHparsestr(metastr, '/', ptr, slen);
	  if( nentries == 0)
		{
		  status = FAIL;
		  sprintf(errbuf,"The input metadata string has no entries. \n");
		  H5Epush(__FILE__,"HE5_EHinsertmeta_Static", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);
			
		  return(status);
		}
	  
	  metastr[slen[0]] = 0;

		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%li%s",
			  "\t\t\tOBJECT=IndexDimensionMap_", count,
			  "\n\t\t\t\tGeoDimension=\"", &metastr[0],
			  "\"\n\t\t\t\tDataDimension=\"", &metastr[slen[0] + 1],
			  "\"\n\t\t\tEND_OBJECT=IndexDimensionMap_", count, "\n");
	  break;
		
		

	case 3:

	  /* Geolocation Fields Section */
	  /* ========================== */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=GeoField");
	  begptr = strstr(metaptr, utlstr);
		
	  strcpy(utlstr, "\t\tEND_GROUP=GeoField");
	  metaptr = strstr(metaptr, utlstr);
		
	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;
		

	  /* Find colon (parse off field name) */
	  /* --------------------------------- */
	  colon  = strchr(metastr, ':');
	  *colon = 0;
		
	  /* Search for next colon (compression and/or tiling parameters) */
	  /* ------------------------------------------------------------ */
	  colon2 = strchr(colon + 1, ':');
	  if (colon2 != NULL)
		{
		  *colon2 = 0;
		}

	  /* Make metadata string list for dimension list */
	  /* -------------------------------------------- */
	  status = HE5_EHmetalist(colon + 1, utlstr2);
	  if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot convert the string list to metadata list. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);
			
		  return(status);
		}
		
		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%s",
			  "\t\t\tOBJECT=GeoField_", count,
			  "\n\t\t\t\tGeoFieldName=\"", metastr,
			  "\"\n\t\t\t\tDataType=", type,
			  "\n\t\t\t\tDimList=", utlstr2);


	  /* If compression and/or tiling parameters add to string */
	  /* ----------------------------------------------------- */
	  if (colon2 != NULL)
		{
		  strcat(utlstr, colon2 + 1);
		}

	  /* Add END_OBJECT terminator to metadata string */
	  /* -------------------------------------------- */
	  sprintf(utlstr2, "%s%li%s", "\n\t\t\tEND_OBJECT=GeoField_", count, "\n");
	  strcat(utlstr, utlstr2);

	  break;

		
		
	case 4:

	  /* Data Fields Section */
	  /* =================== */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=DataField");
	  begptr = strstr(metaptr, utlstr);

	  strcpy(utlstr, "\t\tEND_GROUP=DataField");
	  metaptr = strstr(metaptr, utlstr);

		
	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  /* Find colon (parse off field name) */
	  /* --------------------------------- */
	  colon  = strchr(metastr, ':');
	  *colon = 0;
		
		
	  /* Search for next colon (compression and/or tiling parameters) */
	  /* ------------------------------------------------------------ */
	  colon2 = strchr(colon + 1, ':');
	  if (colon2 != NULL)
		{
		  *colon2 = 0;
		}

	  /* Make metadata string list from dimension list */
	  /* --------------------------------------------- */
	  status = HE5_EHmetalist(colon + 1, utlstr2);
	  if( status == FAIL)
		{
		  sprintf(errbuf, "Cannot convert the string list to metadata list. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);

		  return(status);
		}
		
		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%s",
			  "\t\t\tOBJECT=DataField_", count,
			  "\n\t\t\t\tDataFieldName=\"", metastr,
			  "\"\n\t\t\t\tDataType=", type,
			  "\n\t\t\t\tDimList=", utlstr2);

		

	  /* If compression and/or tiling parameters add to string */
	  /* ----------------------------------------------------- */
	  if (colon2 != NULL)
		{
		  strcat(utlstr, colon2 + 1);
		}

	  /* Add END_OBJECT terminator to metadata string */
	  /* -------------------------------------------- */
	  sprintf(utlstr2, "%s%li%s", "\n\t\t\tEND_OBJECT=DataField_", count, "\n");
	  strcat(utlstr, utlstr2);
		
	  break;
	
	case 5:
	  
	  /* Profile Fields Section */
	  /* ====================== */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=ProfileField");
	  begptr = strstr(metaptr, utlstr);

	  strcpy(utlstr, "\t\tEND_GROUP=ProfileField");
	  metaptr = strstr(metaptr, utlstr);

		
	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  /* Find colon (parse off profile name) */
	  /* ----------------------------------- */
	  colon  = strchr(metastr, ':');
	  *colon = 0;
		
	  /* Search for next colon (compression and/or tiling parameters) */
	  /* ------------------------------------------------------------ */
	  colon2 = strchr(colon + 1, ':');
	  if (colon2 != NULL)
		{
		  *colon2 = 0;
		}

	  /* Make metadata string list from dimension list */
	  /* --------------------------------------------- */
	  status = HE5_EHmetalist(colon + 1, utlstr2);
	  if( status == FAIL)
		{
		  sprintf(errbuf, "Cannot convert the string list to metadata list. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);

		  return(status);
		}
		
		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%s",
			  "\t\t\tOBJECT=ProfileField_", count,
			  "\n\t\t\t\tProfileFieldName=\"", metastr,
			  "\"\n\t\t\t\tDataType=", type,
			  "\n\t\t\t\tDimList=", utlstr2);

		

	  /* If compression and/or tiling parameters add to string */
	  /* ----------------------------------------------------- */
	  if (colon2 != NULL)
		{
		  strcat(utlstr, colon2 + 1);
		}

	  /* Add END_OBJECT terminator to metadata string */
	  /* -------------------------------------------- */
	  sprintf(utlstr2, "%s%li%s", "\n\t\t\tEND_OBJECT=ProfileField_", count, "\n");
	  strcat(utlstr, utlstr2);
		
	  break;
		

	case 10:

	  /* Point Level Section */
	  /* =================== */		
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=Level");
	  begptr = strstr(metaptr, utlstr);
		
	  strcpy(utlstr, "\n\t\tEND_GROUP=Level");
	  metaptr = strstr(metaptr, utlstr) + 1;
		

	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntGROUP(metaArr);

		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%li%s",
			  "\t\t\tGROUP=Level_", count,
			  "\n\t\t\t\tLevelName=\"", metastr,
			  "\"\n\t\t\tEND_GROUP=Level_", count, "\n");
	  break;
		
		
	case 11:

	  /* Point Field Section */
	  /* =================== */		
	  /* Find colon (parse off point field name) */
	  /* --------------------------------------- */
	  colon  = strchr(metastr, ':');
	  *colon = 0;
		
		
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\t\t\tLevelName=\"");
	  strcat(utlstr, colon + 1);
	  begptr = strstr(metaptr, utlstr);
		
	  strcpy(utlstr, "\t\t\tEND_GROUP=Level_");
	  metaptr = strstr(begptr, utlstr);
		

	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%li%s%li%s",
			  "\t\t\t\tOBJECT=PointField_", count,
			  "\n\t\t\t\t\tPointFieldName=\"", metastr,
			  "\"\n\t\t\t\t\tDataType=", type,
			  "\n\t\t\t\t\tOrder=", (long)metadata[1],
			  "\n\t\t\t\tEND_OBJECT=PointField_", count, "\n");
	  break;
		


	case 12:

	  /* LevelLink Section */
	  /* ================= */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=LevelLink");
	  begptr = strstr(metaptr, utlstr);

	  strcpy(utlstr, "\t\tEND_GROUP=LevelLink");
	  metaptr = strstr(metaptr, utlstr);
		

	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  /* Find colon (parse off parent/child level names from link field) */
	  /* --------------------------------------------------------------- */
	  colon = strchr(metastr, ':');
	  *colon = 0;
		

	  /* Find slash (divide parent and child levels) */
	  /* ------------------------------------------- */
	  slash  = strchr(metastr, '/');
	  *slash = 0;


	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%s%s%li%s",
			  "\t\t\tOBJECT=LevelLink_", count,
			  "\n\t\t\t\tParent=\"", metastr,
			  "\"\n\t\t\t\tChild=\"", slash + 1,
			  "\"\n\t\t\t\tLinkField=\"", colon + 1,
			  "\"\n\t\t\tEND_OBJECT=LevelLink_", count, "\n");
		
	  break;
		
	case 101:
	  /* Position metadata pointer for Grid proj parms, pix reg, origin */
	  /* -------------------------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=Dimension");
	  metaptr = strstr(metaptr, utlstr);
	  strcpy(utlstr, metastr);
		
	  break;
		
	case 1001:
	  /* Position metadata pointer for new swath structure (SWcreate) */
	  /* ------------------------------------------------------------ */
	  strcpy(utlstr, "END_GROUP=SwathStructure");
	  metaptr = strstr(metaptr, utlstr);
	  strcpy(utlstr, metastr);
		
	  break;
		

	case 1002:
	  /* Position metadata pointer for new grid structure (GDcreate) */
	  /* ----------------------------------------------------------- */
	  strcpy(utlstr, "END_GROUP=GridStructure");
	  metaptr = strstr(metaptr, utlstr);
	  strcpy(utlstr, metastr);
	  break;
	  
	  
	case 1003:
	  /* Position metadata pointer for new point structure (PTcreate) */
	  /* ------------------------------------------------------------ */
	  strcpy(utlstr, "END_GROUP=PointStructure");
	  metaptr = strstr(metaptr, utlstr);
	  strcpy(utlstr, metastr);
	  break;
	  
        case 1004:
          /* Position metadata pointer for new za structure (ZAcreate) */
          /* ------------------------------------------------------------ */
          strcpy(utlstr, "END_GROUP=ZaStructure");
          metaptr = strstr(metaptr, utlstr);
          strcpy(utlstr, metastr);
 
          break;

	default:
	  {
		sprintf(errbuf, "Unknown metacode.\n");
		H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	  }
	  break;
	  
	}
  
  /* Get offset of entry postion within existing metadata */
  /* ---------------------------------------------------- */
  offset = (long)(metaptr - metabuf);
  
  /* Reallocate memory for a new metadata array, newbuf */
  /* ------------------------------------------------------ */
  newbuf = (char *) calloc(HE5_HeosTable[fid%HE5_EHIDOFFSET].HE5_STATIC_BLKSIZE, sizeof(char));
  if(newbuf==NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory for a new metadata string.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  status = FAIL;
	  free(errbuf);
	  free(type);
	  free(metabuf);
	  free(utlstr);
	  free(utlstr2);
		
	  return(status);
	}
  
  /* Copy new metadata string to newbuf array */
  /* ---------------------------------------- */
  for(i = 0; i < offset; i++)
	newbuf[ i ] = metabuf[ i ];
  
  newbuf[ offset ] = 0;
  
  strcat( newbuf, utlstr);
  strcat( newbuf, metaptr);
  
  tempdim = (hsize_t)strlen( newbuf );
  
  tempdim++;
  
  /*
   *-----------------------------------------------------------------*
   *     Write metadata back  to dataset "StructMetadata.0"          *
   *-----------------------------------------------------------------*
   */
  
  
  /*   H5Sset_extent_simple(aspace, 1, &tempdim, 0 ); NOT SUPPORTED YET !!! */ 
  /*   H5Dextend(meta_id, &tempdim);                  NOT SUPPORTED YET !!! */ 
  
  
  status  = H5Dwrite(meta_id, atype, H5S_ALL, H5S_ALL, H5P_DEFAULT, newbuf); 
  if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot write in structural metadata.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(type);
	  free(metabuf);
	  free(utlstr);
	  free(utlstr2);
	  free(newbuf);
	
	  return(status);
	}
  
  free(metabuf);
  free(newbuf);
  free(utlstr);
  free(utlstr2);
  free(type);
  
  metabuf = NULL;
  newbuf  = NULL;
  utlstr2 = NULL;
  utlstr  = NULL; 
  type    = NULL;
  
  status = H5Sclose(aspace); 
  if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return(status);
	}
  
  status = H5Tclose(atype); 
  if( status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data type ID.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return(status);
	}
  status = H5Dclose(meta_id);
  if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the dataset ID.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return(status);
	}
  status = H5Gclose(gid);
  if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the group ID.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Static", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return(status);
	}
  
  free(errbuf);
  errbuf = NULL;

 COMPLETION:  

  return status;
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHinsertmeta_Dynamic                                         |
|                                                                             |
|  DESCRIPTION: Writes metadata using dynamic buffer                          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|    status       herr_t  None        return status variable (0 for success,  |
|                                          -1 for failure)                    |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|    fid          hid_t   None        HDF-EOS file ID                         |
|    structname   char*   None        Object name string                      |
|    structcode   char*   None        "s" for a swath, "g" for a grid,        |
|                                     "p" for a point, and "z" for a za.      |
|    metacode     long    None        Code of a metadata block to insert to.  |
|    metastr      char*   None        Buffer containing metadata information  |
|                                      to insert.                             |
|    metadata[]   hsize_t None        Array of data values                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|    None                                                                     |
|                                                                             |
|  NOTES:   memory allocation is in cuncks of HE5_DYNAMIC_BLKSIZE (new scheme)|
|           allocated dynamically.                                            |
|           The chuncks will be written to datasets "StructMetadata.0",       |
|           "StructMetadata.1", "StructMetadata.2", etc.                      |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  05/17/04  Abe Taaheri  Added to support dynamic allocation of metadata     |
|                         buffer                                              |
|  May 05    S.Zhao       Added data type string designators in the dataset   |
|                         "StructMetadata.X".                                 |
|  Feb 06    Abe Taaheri  Casted Switch argument to long for HP11             |
|  Oct 11    Abe Taaheri  Corrected end of structre metadata by adding one    |
|                         more NULL character to the end of it (without this  |
|                         change a garbage character was present at the end   |
|                         of last piece of multiple 32000 size  structre      |
|                         metadatas).                                         |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHinsertmeta_Dynamic(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[]) 
{
  herr_t         status   = FAIL;    /* routine return status variable            */

  int            i;                  /* Loop index                                */
 
  hid_t          gid      = FAIL;    /* "HDFEOS INFORMATION" group ID             */
  hid_t          meta_id[MetaBlocks]  = {FAIL};    /* "StructuralMetadat.0" group ID            */
  hid_t          atype[MetaBlocks]    = {FAIL};    /* Data type ID                              */
  hid_t          aspace[MetaBlocks]   = {FAIL};    /* Data space ID                             */
  hid_t          HDFfid   = FAIL;    /* HDF5 file ID                              */

  size_t         slen[HE5_DTSETRANKMAX];/* String length array                    */
  
  hsize_t        tempdim  =  0;      /* Temporary variable                        */
  
  long           count    = 0;       /* Objects/Groups counter                    */
  long           offset   = 0;       /* Offset insertion position of new metadata */
  long           nentries = 0;       /* Number of comma separated string entries  */
  
  char           *metabuf = NULL;    /* Pointer to Structural Metadata array      */
  char           *begptr  = NULL;    /* Pointer to beginning of metadata section  */
  char           *metaptr = NULL;    /* Metadata pointer                          */
  char           *prevmetaptr = NULL;/* Previous position of metadata pointer     */
  char           *ptr[HE5_DTSETRANKMAX];/* String pointer array                   */
  char           *metaArr[2]={NULL,NULL};/* Array of metadata positions           */
  char           *colon   = NULL;    /* Colon position                            */
  char           *colon2  = NULL;    /* 2nd colon position                        */
  char           *slash   = NULL;    /* Slash postion                             */
  char           *utlstr  = NULL;    /* Utility string                            */
  char           *utlstr1 = NULL;    /* Utility string 1                          */
  char           *utlstr2 = NULL;    /* Utility string 2                          */
  char           *newbuf  = NULL;    /* updated metadata string                   */
  char           *type    = NULL;    /* data type string                          */
  char           *errbuf  = NULL;    /* Error message buffer                      */
  char           *dimptr  = NULL;    /* Dimension pointer                         */
  char           *newmetastr = NULL; /* Modified metadata string                  */
  char           *endptr  = NULL;    /* Pointer to ending of metadata section     */
  char           *newmetaptr = NULL; /* Updated metadata pointer                  */
  char           *begptr2  = NULL;   /* Pointer 2 to beginning of metadata section*/
  int            len1, len2, difflen = 0;

  long           nmeta;
  /*char           *metobjectlist;*/
  /*long           *metstrbufsize = 0;*/
  char           metutlstr[32];
  int            ism;
  long           metalen, seglen;
  size_t         size      =  0;     /* Data size (bytes)     */

  CHECKPOINTER(structname);
  CHECKPOINTER(structcode);
  CHECKPOINTER(metastr);


  Stored_meta[fid%HE5_EHIDOFFSET].metaread_flag = 0;

  /* Get HDF file ID */
  /* --------------- */
  HDFfid = HE5_HeosTable[fid%HE5_EHIDOFFSET].HDFfid;

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
	  HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
	  return(FAIL);
    }
  
  /* Allocate memory for data type string */
  /* ------------------------------------ */
  type = (char * )calloc( HE5_HDFE_TYPESTRSIZE, sizeof(char));
  if(type == NULL)
    {
	  sprintf(errbuf,"Cannot allocate memory for data type string. \n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
     
	  return(FAIL);
    }
  
  /* Open "HDFEOS INFORMATION" group */
  /* =============================== */
  gid     = H5Gopen(HDFfid, "HDFEOS INFORMATION");
  if(gid == FAIL)
    {
      status = FAIL;
      sprintf(errbuf,"Cannot open \"HDFEOS INFORMATION\" group. \n");
      H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
      
      return(status);
    }
  
  
  /* Determine number of structural metadata "sections" */
  /* -------------------------------------------------- */
  nmeta = 0;
  status= HE5_EHgetnmeta(gid, &nmeta);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot find any StructMetadata.X dataset. \n");
      H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      
      return(FAIL);
    }

    /* Allocate space for metadata (in units of 32000 bytes) */
    /* ----------------------------------------------------- */
    metabuf = (char *) calloc(HE5_DYNAMIC_BLKSIZE * nmeta, sizeof(char ));
    if(metabuf == NULL)
    { 
      sprintf(errbuf, "Cannot allocate memory for the metadata buffer. \n");
      H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);

      return(FAIL);
    }

    /* Read structural metadata from all StructMetadata datasets */
    /* --------------------------------------------------------- */
    for (ism = 0; ism < nmeta; ism++)
      {
	/*
	 *--------------------------------------------------------------*
	 * Open dataset "StructMetadata.X", get datatype and space ID   *
	 *--------------------------------------------------------------*
	 */
	sprintf(metutlstr, "%s%d", "StructMetadata.", ism);
	meta_id[ism] = H5Dopen(gid, metutlstr); 
	metalen = strlen(metabuf);
	if( meta_id[ism] == FAIL)
	  {
	    status = FAIL;
	    sprintf(errbuf,"Cannot open %s dataset. \n",metutlstr);
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    
	    return(status);
	  }
	atype[ism]   = H5Dget_type(meta_id[ism]);
	if( atype[ism] == FAIL)
	  {
	    status = FAIL;
	    sprintf(errbuf,"Cannot get the dataset datatype. \n");
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    status = H5Dclose(meta_id[ism]);
	    return(status);
	  }
	aspace[ism] = H5Dget_space(meta_id[ism]);
	if( aspace[ism] == FAIL )
	  {
	    status = FAIL;
	    sprintf(errbuf,"Cannot get the dataset dataspace. \n");
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    status = H5Tclose(atype[ism]); 
	    status = H5Dclose(meta_id[ism]);
	    return(status);
	  }
	
	
	/*
	  
     NOTE: this call is now supported for a string and is used now 
           in HE5_EHattrinfo to get the number of elements for string array
	npoints = H5Sget_simple_extent_npoints(aspace);
	it will be implemented for memory allocation for metabuf[] array
	
	*/
	
	
	
	/*
	 *-------------------------------------------------------------*
	 *   Allocate memory for the output array and Read attribute   *
	 *-------------------------------------------------------------*
	 */
	
	utlstr   = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
	if( utlstr == NULL )
	  {
	    sprintf(errbuf,"Cannot allocate memory for utility string buffer. \n");
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    status = FAIL;
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    
	    return(status);
	    
	  }
	
	utlstr2  = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
	if( utlstr2 == NULL )
	  {
	    sprintf(errbuf,"Cannot allocate memory for the 2d utility string buffer. \n");
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    status = FAIL;
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    free(utlstr);
	    
	    return(status);
	    
	  }
	
	status = H5Dread(meta_id[ism], atype[ism],H5S_ALL, H5S_ALL, H5P_DEFAULT, 
			 metabuf + metalen);
	if( status == FAIL)
	  {
	    sprintf(errbuf, "Cannot read structural metadata. \n");
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    free(utlstr);
	    free(utlstr2);
	    
	    return(status);
	  }
      }
    
    /* Determine length (# of characters) of metadata */
    /* ---------------------------------------------- */
    metalen = strlen(metabuf);
    
    /* Find HDF-EOS structure "root" group in metadata */
    /* ----------------------------------------------- */
    /* Setup proper search string */
    if (strcmp(structcode, "s") == 0)
      {
	strcpy(utlstr, "GROUP=SwathStructure");
      }    
    else if (strcmp(structcode, "g") == 0)
      {
	strcpy(utlstr, "GROUP=GridStructure");
      }
    else if (strcmp(structcode, "p") == 0)
      {
	strcpy(utlstr, "GROUP=PointStructure");
      }
    else if (strcmp(structcode, "z") == 0)
      {
	strcpy(utlstr, "GROUP=ZaStructure");
      }
    
    
    /* Use string search routine (strstr) to move through metadata */
    /* ----------------------------------------------------------- */
    metaptr = strstr(metabuf, utlstr);
    
    if ((int)abs((int)metacode) == 0)
      {
	utlstr1  = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
	if( utlstr1 == NULL )
	  {
	    sprintf(errbuf,"Cannot allocate memory for the 2d utility string buffer. \n");
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    status = FAIL;
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    free(utlstr);
	    free(utlstr2);
	    
	    return(status);
	    
	  }
	
	if (strcmp(structcode, "s") == 0)
	  {
	    strcpy(utlstr1, "END_GROUP=SwathStructure");
	  }
	else if (strcmp(structcode, "g") == 0)
	  {
	    strcpy(utlstr1, "END_GROUP=GridStructure");
	  }
	else if (strcmp(structcode, "p") == 0)
	  {
	    strcpy(utlstr1, "END_GROUP=PointStructure");
	  }
	else if (strcmp(structcode, "z") == 0)
	  {
	    strcpy(utlstr1, "END_GROUP=ZaStructure");
	  }
	
	newmetaptr = (char *)malloc(sizeof(char)*(strlen(metaptr)+1));
	strcpy(newmetaptr, metaptr);
	endptr = strstr(newmetaptr, utlstr1);
	endptr++;
	*endptr = '\0';
	free(utlstr1);
	utlstr1 = NULL;
	
      }
    
    /* Find specific (named) structure */
    /* ------------------------------- */
    if (metacode < 1000)
      {
	/* Save current metadata pointer */
	/* ----------------------------- */
	prevmetaptr = metaptr;
	
	/* First loop for "old-style" (non-ODL) metadata string */
	/* ---------------------------------------------------- */
	if (strcmp(structcode, "s") == 0)
	  {
	    sprintf(utlstr, "%s%s%s", "SwathName=\"", structname, "\"");
	  }
	else if (strcmp(structcode, "g") == 0)
	  {
	    sprintf(utlstr, "%s%s%s", "GridName=\"", structname, "\"");
	  }
	else if (strcmp(structcode, "p") == 0)
	  {
	    sprintf(utlstr, "%s%s%s", "PointName=\"", structname, "\"");
	  }
	else if (strcmp(structcode, "z") == 0)
	  {
	    sprintf(utlstr, "%s%s%s", "ZaName=\"", structname, "\"");
	  }
	  
	
	/* Perform string search */
	/* --------------------- */
	metaptr = strstr(metaptr, utlstr);
	if ((int)abs((int)metacode) == 0)
	  {
	    len1 = strlen(newmetaptr);
	    newmetaptr = strstr(newmetaptr, utlstr);
	    len2 = strlen(newmetaptr);
	    difflen = len1 - len2;
	  }
	
	
	/*
	 *--------------------------------------------------------------------*
	 * If not found then return to previous position in metadata and look *
	 *             for "new-style" (ODL) metadata string                  *
	 *--------------------------------------------------------------------*
	 */         
	if (metaptr == NULL)
	  {
	    sprintf(utlstr, "%s%s%s", "GROUP=\"", structname, "\"");
	    metaptr = strstr(prevmetaptr, utlstr);
	  }
      }


    /* Searching for geo fields (3), data fields (4), profile fields (5), */
    /* or point fields (11) convert type code to string designator        */
    /* ------------------------------------------------------------------ */
    if (metacode == 3 || metacode == 4 || metacode == 5 || metacode == 11)
      {
#ifndef HP9000 /* HP11 has problem with hsize_t type as switch argumet */
      switch (metadata[0])
#else
	switch ((long)metadata[0])
#endif
          {
          case 0:
              strcpy(type, "H5T_NATIVE_INT");
              break;
          case 1:
              strcpy(type, "H5T_NATIVE_UINT");
              break;
          case 2:
              strcpy(type, "H5T_NATIVE_SHORT");
              break;
          case 3:
              strcpy(type, "H5T_NATIVE_USHORT");
              break;
          case 4:
              strcpy(type, "H5T_NATIVE_SCHAR");
              break;
          case 5:
              strcpy(type, "H5T_NATIVE_UCHAR");
              break;
          case 6:
              strcpy(type, "H5T_NATIVE_LONG");
              break;
          case 7:
              strcpy(type, "H5T_NATIVE_ULONG");
              break;
          case 8:
              strcpy(type, "H5T_NATIVE_LLONG");
              break;
          case 9:
              strcpy(type, "H5T_NATIVE_ULLONG");
              break;
          case 10:
              strcpy(type, "H5T_NATIVE_FLOAT");
              break;
          case 11:
              strcpy(type, "H5T_NATIVE_DOUBLE");
              break;
          case 12:
              strcpy(type, "H5T_NATIVE_LDOUBLE");
              break;
          case 13:
              strcpy(type, "H5T_NATIVE_INT8");
              break;
          case 14:
              strcpy(type, "H5T_NATIVE_UINT8");
              break;
          case 15:
              strcpy(type, "H5T_NATIVE_INT16");
              break;
          case 16:
              strcpy(type, "H5T_NATIVE_UINT16");
              break;
          case 17:
              strcpy(type, "H5T_NATIVE_INT32");
              break;
          case 18:
              strcpy(type, "H5T_NATIVE_UINT32");
              break;
          case 19:
              strcpy(type, "H5T_NATIVE_INT64");
              break;
          case 20:
              strcpy(type, "H5T_NATIVE_UINT64");
              break;
          case 21:
              strcpy(type, "H5T_NATIVE_B8");
              break;
          case 22:
              strcpy(type, "H5T_NATIVE_B16");
              break;
          case 23:
              strcpy(type, "H5T_NATIVE_B32");
              break;
          case 24:
              strcpy(type, "H5T_NATIVE_B64");
              break;
          case 25:
              strcpy(type, "H5T_NATIVE_HSIZE");
              break;
          case 26:
              strcpy(type, "H5T_NATIVE_HERR");
              break;
          case 27:
              strcpy(type, "H5T_NATIVE_HBOOL");
              break;
          case 56:
              strcpy(type, "H5T_NATIVE_CHAR");
              break;
          case 57:
              strcpy(type, "HE5T_CHARSTRING");
              break;
          }
      }

    
    /* Metadata Section Switch */
    /* ----------------------- */
    switch ((int)abs((int)metacode))
      {
	
      case 0:

	  /* Dimension Section */
	  /* ================= */
          /* Find beginning of metadata section */
          /* ---------------------------------- */
         strcpy(utlstr, "\t\tGROUP=Dimension");
          begptr = strstr(metaptr, utlstr);
          begptr2 = strstr(newmetaptr, utlstr);

          /* Search for dimension name */
          /* ------------------------- */
          newmetastr = (char *)malloc(sizeof(char)*strlen(metastr)+6);
          strcpy(newmetastr, "\"");       
          strcat(newmetastr, metastr);
          strcat(newmetastr, "\""); 
          dimptr = strstr(begptr2, newmetastr);
          free(newmetastr); 
          newmetastr = NULL;

          if (dimptr != NULL)
		{
                  status = FAIL;
          	  sprintf(errbuf,"Cannot redefine the dimension \"%s\". \n", metastr);
                  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
                  HE5_EHprint(errbuf, __FILE__, __LINE__);
                  free(errbuf);
                  free(type);
                  free(metabuf);
                  free(utlstr);
                  free(utlstr2);
                  free(newmetaptr - difflen);
                  return(status);
		}
	  else
		{
                  /* Find ending of metadata section */
                  /* ------------------------------- */
	  	  strcpy(utlstr, "\t\tEND_GROUP=Dimension");
	  	  metaptr = strstr(metaptr, utlstr);
		
	  	  /* Count number of existing entries and increment */
	  	  /* ---------------------------------------------- */
	  	  metaArr[0] = begptr;
	  	  metaArr[1] = metaptr;
	  	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  	  /* Build metadata entry string */
	  	  /* --------------------------- */
	  	  sprintf(utlstr, "%s%li%s%s%s%li%s%li%s", "\t\t\tOBJECT=Dimension_",count,"\n\t\t\t\tDimensionName=\"",&metastr[0],"\"\n\t\t\t\tSize=", (long)metadata[0],"\n\t\t\tEND_OBJECT=Dimension_", count, "\n");
		
		}

          if (newmetaptr != NULL)
           {
              free(newmetaptr - difflen);
              newmetaptr = NULL;
           }

	  break;
		
		
    case 1:

	  /* DimensionMap Section */
	  /* ==================== */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=DimensionMap");
	  begptr = strstr(metaptr, utlstr);

	  strcpy(utlstr, "\t\tEND_GROUP=DimensionMap");
	  metaptr = strstr(metaptr, utlstr);


	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

	  /* Initialize slen[] array */
	  /* ----------------------- */
	  for (i = 0; i < HE5_DTSETRANKMAX; i++)
		slen[ i ] = 0;

	  /* Find slash within input mapping string and replace with NULL */
	  /* ------------------------------------------------------------ */
	  nentries = HE5_EHparsestr(metastr, '/', ptr, slen);
	  if( nentries == 0)
		{
		  status = FAIL;
		  sprintf(errbuf,"Input metadata string has no entries. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);
			
		  return(status);
		}

	  metastr[slen[0]] = 0;
		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%li%s%li%s%li%s",
			  "\t\t\tOBJECT=DimensionMap_", count,
			  "\n\t\t\t\tGeoDimension=\"", &metastr[0],
			  "\"\n\t\t\t\tDataDimension=\"", &metastr[slen[0] + 1],
			  "\"\n\t\t\t\tOffset=", (long)metadata[0],
			  "\n\t\t\t\tIncrement=", (long)metadata[1],
			  "\n\t\t\tEND_OBJECT=DimensionMap_", count, "\n");
	  break;

	case 2:

	  /* IndexDimensionMap Section */
	  /* ========================= */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=IndexDimensionMap");
	  begptr = strstr(metaptr, utlstr);
		
	  strcpy(utlstr, "\t\tEND_GROUP=IndexDimensionMap");
	  metaptr = strstr(metaptr, utlstr);
		
	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;
		

	  /* Find slash within input mapping string and replace with NULL */
	  /* ------------------------------------------------------------ */
	  nentries = HE5_EHparsestr(metastr, '/', ptr, slen);
	  if( nentries == 0)
		{
		  status = FAIL;
		  sprintf(errbuf,"The input metadata string has no entries. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);
			
		  return(status);
		}
	  
	  metastr[slen[0]] = 0;

		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%li%s",
			  "\t\t\tOBJECT=IndexDimensionMap_", count,
			  "\n\t\t\t\tGeoDimension=\"", &metastr[0],
			  "\"\n\t\t\t\tDataDimension=\"", &metastr[slen[0] + 1],
			  "\"\n\t\t\tEND_OBJECT=IndexDimensionMap_", count, "\n");
	  break;
		
		

	case 3:

	  /* Geolocation Fields Section */
	  /* ========================== */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=GeoField");
	  begptr = strstr(metaptr, utlstr);
		
	  strcpy(utlstr, "\t\tEND_GROUP=GeoField");
	  metaptr = strstr(metaptr, utlstr);
		
	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;
		

	  /* Find colon (parse off field name) */
	  /* --------------------------------- */
	  colon  = strchr(metastr, ':');
	  *colon = 0;
		
	  /* Search for next colon (compression and/or tiling parameters) */
	  /* ------------------------------------------------------------ */
	  colon2 = strchr(colon + 1, ':');
	  if (colon2 != NULL)
		{
		  *colon2 = 0;
		}

	  /* Make metadata string list for dimension list */
	  /* -------------------------------------------- */
	  status = HE5_EHmetalist(colon + 1, utlstr2);
	  if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot convert the string list to metadata list. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);
			
		  return(status);
		}
		
		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%s",
			  "\t\t\tOBJECT=GeoField_", count,
			  "\n\t\t\t\tGeoFieldName=\"", metastr,
			  "\"\n\t\t\t\tDataType=", type,
			  "\n\t\t\t\tDimList=", utlstr2);


	  /* If compression and/or tiling parameters add to string */
	  /* ----------------------------------------------------- */
	  if (colon2 != NULL)
		{
		  strcat(utlstr, colon2 + 1);
		}

	  /* Add END_OBJECT terminator to metadata string */
	  /* -------------------------------------------- */
	  sprintf(utlstr2, "%s%li%s", "\n\t\t\tEND_OBJECT=GeoField_", count, "\n");
	  strcat(utlstr, utlstr2);

	  break;

		
		
	case 4:

	  /* Data Fields Section */
	  /* =================== */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=DataField");
	  begptr = strstr(metaptr, utlstr);

	  strcpy(utlstr, "\t\tEND_GROUP=DataField");
	  metaptr = strstr(metaptr, utlstr);

		
	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  /* Find colon (parse off field name) */
	  /* --------------------------------- */
	  colon  = strchr(metastr, ':');
	  *colon = 0;
		
		
	  /* Search for next colon (compression and/or tiling parameters) */
	  /* ------------------------------------------------------------ */
	  colon2 = strchr(colon + 1, ':');
	  if (colon2 != NULL)
		{
		  *colon2 = 0;
		}

	  /* Make metadata string list from dimension list */
	  /* --------------------------------------------- */
	  status = HE5_EHmetalist(colon + 1, utlstr2);
	  if( status == FAIL)
		{
		  sprintf(errbuf, "Cannot convert the string list to metadata list. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);

		  return(status);
		}
		
		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%s",
			  "\t\t\tOBJECT=DataField_", count,
			  "\n\t\t\t\tDataFieldName=\"", metastr,
			  "\"\n\t\t\t\tDataType=", type,
			  "\n\t\t\t\tDimList=", utlstr2);

		

	  /* If compression and/or tiling parameters add to string */
	  /* ----------------------------------------------------- */
	  if (colon2 != NULL)
		{
		  strcat(utlstr, colon2 + 1);
		}

	  /* Add END_OBJECT terminator to metadata string */
	  /* -------------------------------------------- */
	  sprintf(utlstr2, "%s%li%s", "\n\t\t\tEND_OBJECT=DataField_", count, "\n");
	  strcat(utlstr, utlstr2);
		
	  break;
	
	case 5:
	  
	  /* Profile Fields Section */
	  /* ====================== */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=ProfileField");
	  begptr = strstr(metaptr, utlstr);

	  strcpy(utlstr, "\t\tEND_GROUP=ProfileField");
	  metaptr = strstr(metaptr, utlstr);

		
	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  /* Find colon (parse off profile name) */
	  /* ----------------------------------- */
	  colon  = strchr(metastr, ':');
	  *colon = 0;
		
	  /* Search for next colon (compression and/or tiling parameters) */
	  /* ------------------------------------------------------------ */
	  colon2 = strchr(colon + 1, ':');
	  if (colon2 != NULL)
		{
		  *colon2 = 0;
		}

	  /* Make metadata string list from dimension list */
	  /* --------------------------------------------- */
	  status = HE5_EHmetalist(colon + 1, utlstr2);
	  if( status == FAIL)
		{
		  sprintf(errbuf, "Cannot convert the string list to metadata list. \n");
		  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
		  free(type);
		  free(metabuf);
		  free(utlstr);
		  free(utlstr2);

		  return(status);
		}
		
		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%s",
			  "\t\t\tOBJECT=ProfileField_", count,
			  "\n\t\t\t\tProfileFieldName=\"", metastr,
			  "\"\n\t\t\t\tDataType=", type,
			  "\n\t\t\t\tDimList=", utlstr2);

		

	  /* If compression and/or tiling parameters add to string */
	  /* ----------------------------------------------------- */
	  if (colon2 != NULL)
		{
		  strcat(utlstr, colon2 + 1);
		}

	  /* Add END_OBJECT terminator to metadata string */
	  /* -------------------------------------------- */
	  sprintf(utlstr2, "%s%li%s", "\n\t\t\tEND_OBJECT=ProfileField_", count, "\n");
	  strcat(utlstr, utlstr2);
		
	  break;
		

	case 10:

	  /* Point Level Section */
	  /* =================== */		
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=Level");
	  begptr = strstr(metaptr, utlstr);
		
	  strcpy(utlstr, "\n\t\tEND_GROUP=Level");
	  metaptr = strstr(metaptr, utlstr) + 1;
		

	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntGROUP(metaArr);

		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%li%s",
			  "\t\t\tGROUP=Level_", count,
			  "\n\t\t\t\tLevelName=\"", metastr,
			  "\"\n\t\t\tEND_GROUP=Level_", count, "\n");
	  break;
		
		
	case 11:

	  /* Point Field Section */
	  /* =================== */		
	  /* Find colon (parse off point field name) */
	  /* --------------------------------------- */
	  colon  = strchr(metastr, ':');
	  *colon = 0;
		
		
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\t\t\tLevelName=\"");
	  strcat(utlstr, colon + 1);
	  begptr = strstr(metaptr, utlstr);
		
	  strcpy(utlstr, "\t\t\tEND_GROUP=Level_");
	  metaptr = strstr(begptr, utlstr);
		

	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%li%s%li%s",
			  "\t\t\t\tOBJECT=PointField_", count,
			  "\n\t\t\t\t\tPointFieldName=\"", metastr,
			  "\"\n\t\t\t\t\tDataType=", type,
			  "\n\t\t\t\t\tOrder=", (long)metadata[1],
			  "\n\t\t\t\tEND_OBJECT=PointField_", count, "\n");
	  break;
		


	case 12:

	  /* LevelLink Section */
	  /* ================= */
	  /* Find beginning and ending of metadata section */
	  /* --------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=LevelLink");
	  begptr = strstr(metaptr, utlstr);

	  strcpy(utlstr, "\t\tEND_GROUP=LevelLink");
	  metaptr = strstr(metaptr, utlstr);
		

	  /* Count number of existing entries and increment */
	  /* ---------------------------------------------- */
	  metaArr[0] = begptr;
	  metaArr[1] = metaptr;
	  count      = HE5_EHcntOBJECT(metaArr) + 1;

		
	  /* Find colon (parse off parent/child level names from link field) */
	  /* --------------------------------------------------------------- */
	  colon = strchr(metastr, ':');
	  *colon = 0;
		

	  /* Find slash (divide parent and child levels) */
	  /* ------------------------------------------- */
	  slash  = strchr(metastr, '/');
	  *slash = 0;


	  /* Build metadata entry string */
	  /* --------------------------- */
	  sprintf(utlstr, "%s%li%s%s%s%s%s%s%s%li%s",
			  "\t\t\tOBJECT=LevelLink_", count,
			  "\n\t\t\t\tParent=\"", metastr,
			  "\"\n\t\t\t\tChild=\"", slash + 1,
			  "\"\n\t\t\t\tLinkField=\"", colon + 1,
			  "\"\n\t\t\tEND_OBJECT=LevelLink_", count, "\n");
		
	  break;
		
	case 101:
	  /* Position metadata pointer for Grid proj parms, pix reg, origin */
	  /* -------------------------------------------------------------- */
	  strcpy(utlstr, "\t\tGROUP=Dimension");
	  metaptr = strstr(metaptr, utlstr);
	  strcpy(utlstr, metastr);
		
	  break;
		
	case 1001:
	  /* Position metadata pointer for new swath structure (SWcreate) */
	  /* ------------------------------------------------------------ */
	  strcpy(utlstr, "END_GROUP=SwathStructure");
	  metaptr = strstr(metaptr, utlstr);
	  strcpy(utlstr, metastr);
		
	  break;
		

	case 1002:
	  /* Position metadata pointer for new grid structure (GDcreate) */
	  /* ----------------------------------------------------------- */
	  strcpy(utlstr, "END_GROUP=GridStructure");
	  metaptr = strstr(metaptr, utlstr);
	  strcpy(utlstr, metastr);
	  break;
	  
	  
	case 1003:
	  /* Position metadata pointer for new point structure (PTcreate) */
	  /* ------------------------------------------------------------ */
	  strcpy(utlstr, "END_GROUP=PointStructure");
	  metaptr = strstr(metaptr, utlstr);
	  strcpy(utlstr, metastr);
	  break;
	  
        case 1004:
          /* Position metadata pointer for new za structure (ZAcreate) */
          /* ------------------------------------------------------------ */
          strcpy(utlstr, "END_GROUP=ZaStructure");
          metaptr = strstr(metaptr, utlstr);
          strcpy(utlstr, metastr);
 
          break;

	default:
	  {
		sprintf(errbuf, "Unknown metacode.\n");
		H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	  }
	  break;
	  
	}

    /* Get length of metadata string to insert */
    /* --------------------------------------- */
    seglen = strlen(utlstr);
    
    /* Get offset of entry postion within existing metadata */
    /* ---------------------------------------------------- */
    offset = (long)(metaptr - metabuf);
    
    /* If end of new metadata string outside of current metadata buffer ... */
    /* -------------------------------------------------------------------- */
    if (metalen + seglen > HE5_DYNAMIC_BLKSIZE * nmeta - 1)
      {
	/* Reallocate metadata buffer with additional HE5_DYNAMIC_BLKSIZE bytes */
	/* -------------------------------------------------------------------- */
	newbuf = (char *) calloc(HE5_DYNAMIC_BLKSIZE * (nmeta + 1), sizeof(char));
	
	if(newbuf == NULL)
	  { 
	    sprintf(errbuf, "Cannot allocate memory for a new metadata string.\n");
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    status = FAIL;
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    free(utlstr);
	    free(utlstr2);
	    
	    return(status);
	  }
	
	/* Increment metadata section counter */
	/* ---------------------------------- */
	nmeta++;
	sprintf(metutlstr, "%s%ld", "StructMetadata.", nmeta - 1);
	
	/* and create meta_id, atype and aspace for the new block */
	
	/* Create dataspace first*/
	/* ---------------- */
	aspace[nmeta - 1] = H5Screate(H5S_SCALAR);
	if (aspace[nmeta - 1] == FAIL)
	  {
	    sprintf(errbuf, "Cannot create dataspace for %s dataset. \n",metutlstr);
	    H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASPACE, H5E_CANTCREATE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    free(utlstr);
	    free(utlstr2);
	    return(FAIL);
	  }
	
	atype[nmeta - 1] = H5Tcopy(H5T_C_S1);
	
	size    = HE5_DYNAMIC_BLKSIZE;		  
	/* Set data type size */
	/* ------------------ */
	status  = H5Tset_size(atype[nmeta - 1], size);
	if (status == FAIL)
	  {
	    sprintf(errbuf, "Cannot set the total size for atomic datatype. \n");
	    H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATATYPE, H5E_CANTCREATE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    free(utlstr);
	    free(utlstr2);
	    return(FAIL);
	  }
	
	/* Create "StructMetadata" dataset */
	/* ------------------------------- */
	meta_id[nmeta - 1] = H5Dcreate(gid, metutlstr, atype[nmeta - 1], 
				       aspace[nmeta - 1], H5P_DEFAULT);
	if (meta_id[nmeta - 1] == FAIL)
	  {
	    sprintf(errbuf, "Cannot create %s dataset. \n", metutlstr);
	    H5Epush(__FILE__, "HE5_EHopen", __LINE__, H5E_DATASET, H5E_CANTCREATE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    free(utlstr);
	    free(utlstr2);
	    return(FAIL);
	  }

	/* Reposition metadata pointer (entry position) */
	/* -------------------------------------------- */
	metaptr = metabuf + offset;
      }
    else
      {
	newbuf = (char *) calloc(HE5_DYNAMIC_BLKSIZE * (nmeta), sizeof(char));
	
	if(newbuf == NULL)
	  { 
	    sprintf(errbuf, "Cannot allocate memory for a new metadata string.\n");
	    H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    status = FAIL;
	    free(errbuf);
	    free(type);
	    free(metabuf);
	    free(utlstr);
	    free(utlstr2);
	    
	    return(status);
	  }
	
	/* Reposition metadata pointer (entry position) */
	/* -------------------------------------------- */
	metaptr = metabuf + offset;
      }
  
  /* Copy new metadata string to newbuf array */
  /* ---------------------------------------- */
  for(i = 0; i < offset; i++)
	newbuf[ i ] = metabuf[ i ];
  
  newbuf[ offset ] = 0;
  
  strcat( newbuf, utlstr);
  strcat( newbuf, metaptr);
  
  /* set to null character remaining of the metabuf */
  
  memset((newbuf + metalen + seglen), '\0', (nmeta*HE5_DYNAMIC_BLKSIZE - (metalen +
                                                                seglen)));
  /* Add new null string terminator */
  /* ------------------------------ */
  newbuf[metalen + seglen] = 0;
  
  tempdim = (hsize_t)strlen( newbuf );
  
  tempdim++;
  
  /*
   *-----------------------------------------------------------------*
   *     Write metadata back  to dataset "StructMetadata.X"          *
   *-----------------------------------------------------------------*
   */
  
  
  /*   H5Sset_extent_simple(aspace, 1, &tempdim, 0 ); NOT SUPPORTED YET !!! */ 
  /*   H5Dextend(meta_id, &tempdim);                  NOT SUPPORTED YET !!! */ 
  
  for (ism = 0; ism < nmeta; ism++)
    {
      status  = H5Dwrite(meta_id[ism], atype[ism], H5S_ALL, H5S_ALL, H5P_DEFAULT, newbuf + ism * HE5_DYNAMIC_BLKSIZE); 
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot write in structural metadata.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(type);
	  free(metabuf);
	  free(utlstr);
	  free(utlstr2);
	  free(newbuf);
	  
	  return(status);
	}
    }

  free(metabuf);
  free(newbuf);
  free(utlstr);
  free(utlstr2);
  free(type);
  
  metabuf = NULL;
  newbuf  = NULL;
  utlstr2 = NULL;
  utlstr  = NULL; 
  type    = NULL;

  for (ism = 0; ism < nmeta; ism++)
    {  
      status = H5Sclose(aspace[ism]); 
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  
	  return(status);
	}
      
      status = H5Tclose(atype[ism]); 
      if( status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data type ID.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  
	  return(status);
	}
      status = H5Dclose(meta_id[ism]);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the dataset ID.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return(status);
	}
    }
  status = H5Gclose(gid);
  if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the group ID.\n");
	  H5Epush(__FILE__, "HE5_EHinsertmeta_Dynamic", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return(status);
	}
  
  free(errbuf);
  errbuf = NULL;

 COMPLETION:  

  return status;
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHgetmetavalue                                               |
|                                                                             |
|  DESCRIPTION: Returns metadata value                                        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  metaptrs       char                Begin and end of metadata section       |
|  parameter      char                parameter to access                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|  metaptr        char                Ptr to (updated) beginning of metadata  |
|  retstr         char                return string containing value          |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  10/18/99 A.Muslimov    Replaced memcpy() by memmove() to avoid a problem   |
|                         when arguments 1 and 2 overlap in memory.           |
|  4/19/00  A.Muslimov    Changed type of 'slen' from int to size_t.          |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_EHgetmetavalue(char *metaptrs[], char *parameter, char *retstr)
{
  herr_t        status   = FAIL;		/* routine return status variable */
  
  size_t        slen     = 0;		    /* String length                  */
  
  char          *newline = (char *)NULL;/* Position of new line character */
  char          *sptr    = (char *)NULL;/* String pointer within metadata */
  
  CHECKPOINTER(metaptrs[0]);
  CHECKPOINTER(metaptrs[1]);
  CHECKPOINTER(parameter);

  
  /* Get string length of parameter string + 1 */
  /* ----------------------------------------- */
  slen = strlen(parameter) + 1;
  
  /* Build search string (parameter string + "=") */
  /* -------------------------------------------- */
  strcpy(retstr, parameter);
  strcat(retstr, "=");
  
  /* Search for string within metadata (beginning at metaptrs[0]) */
  /* ------------------------------------------------------------ */
  sptr = strstr(metaptrs[0], retstr);
  
  /* If string found within desired section ... */
  /* ------------------------------------------ */
  if (sptr != NULL && sptr < metaptrs[1])
    {
	  /* Store position of string within metadata */
	  /* ---------------------------------------- */
	  metaptrs[0] = sptr;

	  /* Find newline "\n" character */
	  /* --------------------------- */
	  newline = strchr(metaptrs[0], '\n');

	  /* Copy from "=" to "\n" (exclusive) into return string */
	  /* ---------------------------------------------------- */
	  memmove(retstr, metaptrs[0] + slen, newline - metaptrs[0] - slen);

	  /* Terminate return string with null */
	  /* --------------------------------- */
	  retstr[newline - metaptrs[0] - slen] = 0;
	  status = SUCCEED;
	} 
  else
	{
	  /*
	   * if parameter string not found within section, null return string
	   * and set status to FAIL.
	   */
	  retstr[0] = 0;
	  status = FAIL;
	}
  
 COMPLETION:
  return (status);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHmetagroup                                                  |
|                                                                             |
|  DESCRIPTION: Returns pointers to beginning and end of metadata group       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  metabuf        char*    None       Pointer to HDF-EOS object in metadata   |
|                                                                             |
|  INPUTS:                                                                    |
|  gid            hid_t    None       group ID                                |
|  structname     char*    None       HDF-EOS structure name                  |
|  structcode     char*    None       Structure code ("s/g/p/z")              |
|  groupname      char*    None       Metadata group name                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|  metaptrs       char*    None       pointers to begin and end of metadata   |
|                                                                             |
|  NOTES:  Allocates memory of fixed size (= HE5_STATIC_BLKSIZE)              |
|          for metabuf[] array (old scheme), or allocates buffer in chuncks   |
|          of HE5_DYNAMIC_BLKSIZE (new scheme).                               |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  05/17/04  Abe Taaheri  Initial version                                     |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
char  *
HE5_EHmetagroup(hid_t fid , char *structname, char *structcode, char *groupname, char *metaptrs[])
{
  char          *metabuf=(char *)NULL;      /* Ptr to StrucMetadata (SM)    */
  int      HDFEOS5ver_flg ; /* will be set to 1 if the HDF-EOS5 version 
			       is later than 5.1.7, otherwise to 0 */
  /* Get HDFEOS5ver_flg flag for the file */
  /* --------------- */
  HDFEOS5ver_flg = HE5_HeosTable[fid%HE5_EHIDOFFSET].DYNAMIC_flag;

  if(HDFEOS5ver_flg == 1)
    {
      metabuf = (char *)HE5_EHmetagroup_Static(fid , structname, structcode, 
					       groupname, metaptrs);
    }
  else if(HDFEOS5ver_flg == 0)
    {
      metabuf = (char *)HE5_EHmetagroup_Dynamic(fid , structname, structcode, 
						groupname, metaptrs);

    }

  return(metabuf);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHmetagroup_Static                                           |
|                                                                             |
|  DESCRIPTION: Returns pointers to beginning and end of metadata group       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  metabuf        char*    None       Pointer to HDF-EOS object in metadata   |
|                                                                             |
|  INPUTS:                                                                    |
|  gid            hid_t    None       group ID                                |
|  structname     char*    None       HDF-EOS structure name                  |
|  structcode     char*    None       Structure code ("s/g/p/z")              |
|  groupname      char*    None       Metadata group name                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|  metaptrs       char*    None       pointers to begin and end of metadata   |
|                                                                             |
|  NOTES:        A.M.: Temporarily allocates memory of fixed size (= BLKSIZE) |
|                      for metabuf[] array. Also allocates memory of fixed    |
|                      size (= HDFE_UTLBUFSIZE) for utlstr[].                 |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  3/30/99                Make metadata ODL compliant                         |
|  7/28/99  A.Muslimov    Replaced the 1st argument of EHmetagroup by fid     |
|                         (file ID). Did the same change in the call to       |
|                         H5Gopen(). Added error handling right after the     |
|                         call to H5Dread().                                  |
|  9/02/99  A.Muslimov    Replaced the call in 'OPEN "HDFEOS INFORMATION"     |
|                         GROUP' block by two calls to pass the HDFfid        |
|                         instead of fid to H5Gopen().                        |
|  9/29/99  A.Muslimov    Added dynamic memory allocation for error message   |
|                         buffer.                                             |
|  02/07/00 A.Muslimov    Added free() calls before return(NULL).             |
|  July 02  S.Zhao        Added H5Gclose(gid) to release the "HDFEOS          |
|                         INFORMATION" group ID.                              |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
char  *
HE5_EHmetagroup_Static(hid_t fid , char *structname, char *structcode, char *groupname, char *metaptrs[])
{

  herr_t     status      = FAIL;/* return status variable                          */
    
  hid_t      meta_id     = FAIL;/*  metadata ID                                    */
  hid_t      gid         = FAIL;/* "HDFEOS INFORMATION" group ID                   */
  hid_t      atype       = FAIL;/* datatype ID                                     */
  hid_t      aspace      = FAIL;/* dataspace ID                                    */
  hid_t      HDFfid      = FAIL;/* HDF-EOS file ID                                 */
  hid_t      grpID       = FAIL;/* HDFEOS group ID                                 */
 
  size_t     tsize       = 0;	/* number of elements in a dataset                 */

  uintn      access      = 0;	/* read-write access mode                          */ 
    
  char       *metabuf  = NULL;	/* Pointer (handle) to structural metadata         */
  char       *endptr   = NULL;	/* Pointer to end of metadata section              */
  char       *metaptr  = NULL;	/* Metadata pointer                                */
  char       *prevmetaptr =NULL;/* Previous position of metadata pointer           */
  char       *utlstr   = NULL;	/* Utility string                                  */
  char       *errbuf   = NULL;  /* buffer for error message                        */


  CHECKPOINTER(structname);
  CHECKPOINTER(structcode);

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
	{
	  H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
	  HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
		
	  return(NULL);
	}

  /* Check HDF-EOS file ID */
  /* --------------------- */
  status = HE5_EHchkfid(fid, "HE5_EHmetagroup_Static", &HDFfid, &grpID, &access);
  if ( status == FAIL )
	{
	  sprintf(errbuf, "Checking for file ID failed.\n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Static",   __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return(NULL);
	}

  /*
   *-------------------------------------------------------------------------*
   *               O P E N  "HDFEOS INFORMATION" G R O U P                   *
   *-------------------------------------------------------------------------*
   */
  gid = H5Gopen(HDFfid,"HDFEOS INFORMATION");
  if ( gid == FAIL )
	{
	  sprintf(errbuf,"Cannot open \"HDFEOS INFORMATION\" group. \n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_OHDR, H5E_NOTFOUND,errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return( NULL );
	}
    
  /* 
   *--------------------------------------------------------------------------*
   *                    Open dataset "StructMetadata.0"                       *
   *--------------------------------------------------------------------------*
   */
  meta_id = H5Dopen(gid, "StructMetadata.0");
  if ( meta_id == FAIL )
	{
	  sprintf(errbuf,"Cannot open \"StructMetadata.0\" dataset. \n"); 
	  H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return( NULL );
	}
	
  /* Get dataset type */
  /* ---------------- */
  atype   = H5Dget_type(meta_id);
  if ( atype == FAIL )
	{
	  sprintf(errbuf,"Cannot get datatype ID. \n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return( NULL );
	}
	
  /* Get dataset size */
  /* ---------------- */
  tsize   = H5Tget_size(atype);
  if ( tsize == 0 )
	{
	  sprintf(errbuf, "The datatype is of ZERO size. \n"); 
	  H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return( NULL );
	}
	
  /* Get dataset space */
  /* ----------------- */
  aspace  = H5Dget_space(meta_id);
  if ( aspace == FAIL )
	{
	  sprintf(errbuf,"Cannot get the data space ID. \n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return( NULL );
	}
	
  /* -------------------------------------------------------------- */
  /* Get attribute size                                             */
  /* NOTE: this call is now supported for a string and is used now  */
  /* in HE5_EHattrinfoto get the number of elements for string array*/
  /* npoints = H5Sget_simple_extent_npoints(aspace);                */
  /* It'll be implemented for memory allocation for metabuf[] array */
  /* -------------------------------------------------------------- */

	
  /* Allocate memory */
  /* --------------- */
  metabuf = (char *)calloc( HE5_HeosTable[fid%HE5_EHIDOFFSET].HE5_STATIC_BLKSIZE, sizeof(tsize) );
  if( metabuf==NULL)
    {
      sprintf(errbuf,"Can not allocate memory for meta bufer. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
	
      return(NULL);
    }

  utlstr  = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(tsize) );
  if( utlstr==NULL)
    {
      sprintf(errbuf,"Can not allocate memory for utility string. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(metabuf);
		
      return(NULL);
    }

  /* Read the metadata buffer */
  /* ------------------------ */
  status = H5Dread(meta_id,atype,H5S_ALL, H5S_ALL, H5P_DEFAULT, metabuf);
  if (status == FAIL )
    {
      metabuf = NULL;
      sprintf(errbuf,"Cannot read the metadata buffer. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(metabuf);
      free(utlstr);
	    
      return(NULL);
    }

	
  /* Find HDF-EOS structure "root" group in metadata */
  /* ----------------------------------------------- */
	
  /* Setup proper search string */
  /* -------------------------- */
  if (strcmp(structcode, "s") == 0)
    strcpy(utlstr, "GROUP=SwathStructure");
  else if (strcmp(structcode, "g") == 0)
    strcpy(utlstr, "GROUP=GridStructure");
  else if (strcmp(structcode, "p") == 0)
    strcpy(utlstr, "GROUP=PointStructure");
  else if (strcmp(structcode, "z") == 0)
    strcpy(utlstr, "GROUP=ZaStructure");
	
  /* Use string search routine (strstr) to move through metadata */
  /* ----------------------------------------------------------- */
  metaptr = strstr(metabuf, utlstr);
	
  /* Save current metadata pointer */
  /* ----------------------------- */
  prevmetaptr = metaptr;
	

  /* First loop for "old-style" (non-ODL) metadata string */
  /* ---------------------------------------------------- */
  if (strcmp(structcode, "s") == 0)
    sprintf(utlstr, "%s%s%s", "SwathName=\"", structname, "\"");
  else if (strcmp(structcode, "g") == 0)
    sprintf(utlstr, "%s%s%s", "GridName=\"", structname, "\"");
  else if (strcmp(structcode, "p") == 0)
    sprintf(utlstr, "%s%s%s", "PointName=\"", structname, "\"");
  else if (strcmp(structcode, "z") == 0)
    sprintf(utlstr, "%s%s%s", "ZaName=\"", structname, "\"");
    
  /* Do string search */
  /* ---------------- */
  metaptr = strstr(metaptr, utlstr);
	
  /*
   * If not found then return to previous position in metadata and look for
   * "new-style" (ODL) metadata string
   */
  if (metaptr == NULL)
    {
      sprintf(utlstr, "%s%s%s", "GROUP=\"", structname, "\"");
      metaptr = strstr(prevmetaptr, utlstr);
    }
	
  /* Find group within structure */
  /* --------------------------- */
  if (groupname != NULL)
    {
      sprintf(utlstr, "%s%s", "GROUP=", groupname);
      metaptr = strstr(metaptr, utlstr);
		
      sprintf(utlstr, "%s%s", "\t\tEND_GROUP=", groupname);
      endptr = strstr(metaptr, utlstr);
    } else
    {
      /* If groupname == NULL then find end of structure in metadata */
      /* ----------------------------------------------------------- */
      sprintf(utlstr, "%s", "\n\tEND_GROUP=");
      endptr = strstr(metaptr, utlstr);
    }
	

  /* Return beginning and ending pointers */
  /* ------------------------------------ */
  metaptrs[0] = metaptr;
  metaptrs[1] = endptr;

  /* Release the dataspace ID */
  /* ------------------------ */
  status = H5Sclose(aspace);
  if (status == FAIL )
    {
      sprintf(errbuf,  "Cannot release the dataspace ID. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(metabuf);
      free(utlstr);
		
      return(NULL);
    }

  /* Release the datatype ID */
  /* ----------------------- */
  status = H5Tclose(atype);
  if (status == FAIL )
    {
      sprintf(errbuf,"Cannot release the datatype ID. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(metabuf);
      free(utlstr);
		
      return(NULL);
    }

  /* Release the dataset ID */
  /* ---------------------- */
  status = H5Dclose(meta_id);
  if (status == FAIL )
    {
      sprintf(errbuf,"Cannot release the dataset ID. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(metabuf);
      free(utlstr);
		
      return(NULL);
    }

  /* Release the "HDFEOS INFORMATION" group ID */
  /* ----------------------------------------- */ 
  status = H5Gclose(gid);
  if (status == FAIL )
    {
      sprintf(errbuf,"Cannot release the \"HDFEOS INFORMATION\" group ID. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Static", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(metabuf);
      free(utlstr);

      return(NULL);
    }

  free(errbuf);
  free(utlstr);
    
 COMPLETION:
  return(metabuf);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHmetagroup_Dynamic                                          |
|                                                                             |
|  DESCRIPTION: Returns pointers to beginning and end of metadata group       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  metabuf        char*    None       Pointer to HDF-EOS object in metadata   |
|                                                                             |
|  INPUTS:                                                                    |
|  gid            hid_t    None       group ID                                |
|  structname     char*    None       HDF-EOS structure name                  |
|  structcode     char*    None       Structure code ("s/g/p/z")              |
|  groupname      char*    None       Metadata group name                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|  metaptrs       char*    None       pointers to begin and end of metadata   |
|                                                                             |
|  NOTES:  Allocates buffer in chuncks of HE5_DYNAMIC_BLKSIZE (new scheme).   |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  05/17/04 Abe Taaheri   Modified the version with static buffer allocation  |
|                         to adopt dynamic buffer allocation in chuncks of    |
|                         HE5_DYNAMIC_BLKSIZE                                 |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
char  *
HE5_EHmetagroup_Dynamic(hid_t fid , char *structname, char *structcode, char *groupname, char *metaptrs[])
{

  herr_t     status      = FAIL;/* return status variable                          */
    
  hid_t      meta_id[MetaBlocks]     = {FAIL};/*  metadata ID                                    */
  hid_t      gid         = FAIL;/* "HDFEOS INFORMATION" group ID                   */
  hid_t      atype[MetaBlocks]       = {FAIL};/* datatype ID                                     */
  hid_t      aspace[MetaBlocks]      = {FAIL};/* dataspace ID                                    */
  hid_t      HDFfid      = FAIL;/* HDF-EOS file ID                                 */
  hid_t      grpID       = FAIL;/* HDFEOS group ID                                 */
 
  size_t     tsizeblk[MetaBlocks]    = {0};   /* number of elements in a dataset                 */
  size_t     tsize       = 0;	/* number of elements in a dataset                 */

  uintn      access      = 0;	/* read-write access mode                          */ 
    
  char       *metabuf  = NULL;	/* Pointer (handle) to structural metadata         */
  char       *endptr   = NULL;	/* Pointer to end of metadata section              */
  char       *metaptr  = NULL;	/* Metadata pointer                                */
  char       *prevmetaptr =NULL;/* Previous position of metadata pointer           */
  char       *utlstr   = NULL;	/* Utility string                                  */
  char       *errbuf   = NULL;  /* buffer for error message                        */
  int            ism;
  long           nmeta;
  /*long           *metstrbufsize = 0;*/
  char           metutlstr[32];
  long           metalen;

  CHECKPOINTER(structname);
  CHECKPOINTER(structcode);

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
		
      return(NULL);
    }

  /* Check HDF-EOS file ID */
  /* --------------------- */
  status = HE5_EHchkfid(fid, "HE5_EHmetagroup_Dynamic", &HDFfid, &grpID, &access);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for file ID failed.\n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic",  __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
		
      return(NULL);
    }

  /*
   *-------------------------------------------------------------------------*
   *               O P E N  "HDFEOS INFORMATION" G R O U P                   *
   *-------------------------------------------------------------------------*
   */
  gid = H5Gopen(HDFfid,"HDFEOS INFORMATION");
  if ( gid == FAIL )
    {
      sprintf(errbuf,"Cannot open \"HDFEOS INFORMATION\" group. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
		
      return( NULL );
    }
    
  /* 
   *--------------------------------------------------------------------------*
   *                    Open dataset "StructMetadata.X"                       *
   *--------------------------------------------------------------------------*
   */
  
  /* Determine number of structural metadata "sections" */
  /* -------------------------------------------------- */
  nmeta = 0;
  status= HE5_EHgetnmeta(gid, &nmeta);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot find any StructMetadata.X dataset. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      
      return(NULL);
    }
  for (ism = 0; ism < nmeta; ism++)
    {
      sprintf(metutlstr, "%s%d", "StructMetadata.", ism);
      meta_id[ism] = H5Dopen(gid, metutlstr);
      if ( meta_id[ism] == FAIL )
	{
	  sprintf(errbuf,"Cannot open %s dataset. \n", metutlstr); 
	  H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  
	  return( NULL );
	}
      
      /* Get dataset type */
      /* ---------------- */
      atype[ism]   = H5Dget_type(meta_id[ism]);
      if ( atype[ism] == FAIL )
	{
	  sprintf(errbuf,"Cannot get datatype ID. \n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  
	  return( NULL );
	}
      
      /* Get dataset size */
      /* ---------------- */
      tsizeblk[ism]   = H5Tget_size(atype[ism]);
      if ( tsizeblk[ism] == 0 )
	{
	  sprintf(errbuf, "The datatype is of ZERO size for StructMetadata block (zero based) %d. \n", ism); 
	  H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  
	  return( NULL );
	}
      
      /* Get dataset space */
      /* ----------------- */
      aspace[ism]  = H5Dget_space(meta_id[ism]);
      if ( aspace[ism] == FAIL )
	{
	  sprintf(errbuf,"Cannot get the data space ID for StructMetadata block (zero based) %d. \n", ism);
	  H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  
	  return( NULL );
	}
    }
  tsize = tsizeblk[0];
  for (ism = 0; ism < nmeta; ism++)
    {
      if(tsizeblk[ism] != tsize)
	{
	  sprintf(errbuf,"Data sizes are not the same for StructMetadata blocks. \n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  
	  return( NULL );
	}
    }
  /* --------------------------------------------------------------  */
  /* Get attribute size                                              */
  /* NOTE: this call is now supported for a string and is used now   */
  /* in HE5_EHattrinfo to get the number of elements for string array*/
  /* npoints = H5Sget_simple_extent_npoints(aspace);                 */
  /* It'll be implemented for memory allocation for metabuf[] array  */
  /* --------------------------------------------------------------  */

	
  /* Allocate memory */
  /* --------------- */
  metabuf = (char *)calloc( HE5_DYNAMIC_BLKSIZE * nmeta, sizeof(tsize) );
  if( metabuf==NULL)
    {
      sprintf(errbuf,"Can not allocate memory for meta bufer. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
	
      return(NULL);
    }

  utlstr  = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(tsize) );
  if( utlstr==NULL)
    {
      sprintf(errbuf,"Can not allocate memory for utility string. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(metabuf);
		
      return(NULL);
    }

  /* Read the metadata buffer */
  /* ------------------------ */
  metalen = 0;
  if (Stored_meta[fid%HE5_EHIDOFFSET].metaread_flag == 0)
    {
      for (ism = 0; ism < nmeta; ism++)
	{
	  status = H5Dread(meta_id[ism],atype[ism],H5S_ALL, H5S_ALL, H5P_DEFAULT, metabuf + metalen);
	  if (status == FAIL )
	    {
	      metabuf = NULL;
	      sprintf(errbuf,"Cannot read the metadata buffer. \n");
	      H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__,  H5E_DATASET, H5E_READERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      free(metabuf);
	      free(utlstr);
	  
	      return(NULL);
	    }

	  /* Determine length (# of characters) of metadata */
	  /* ---------------------------------------------- */
	  metalen = strlen(metabuf);
	}

      Stored_meta[fid%HE5_EHIDOFFSET].metabuf_length = strlen(metabuf);
      Stored_meta[fid%HE5_EHIDOFFSET].saved_metabuf = (char *)realloc(Stored_meta[fid%HE5_EHIDOFFSET].saved_metabuf,(Stored_meta[fid%HE5_EHIDOFFSET].metabuf_length+1)*sizeof(tsize));
      strcpy(Stored_meta[fid%HE5_EHIDOFFSET].saved_metabuf, metabuf);
      Stored_meta[fid%HE5_EHIDOFFSET].nmeta = nmeta;
      Stored_meta[fid%HE5_EHIDOFFSET].metaread_flag = 1;

    }
  else
    {
      strcpy(metabuf,Stored_meta[fid%HE5_EHIDOFFSET].saved_metabuf);
    }


  /* Find HDF-EOS structure "root" group in metadata */
  /* ----------------------------------------------- */
	
  /* Setup proper search string */
  /* -------------------------- */
  if (strcmp(structcode, "s") == 0)
    strcpy(utlstr, "GROUP=SwathStructure");
  else if (strcmp(structcode, "g") == 0)
    strcpy(utlstr, "GROUP=GridStructure");
  else if (strcmp(structcode, "p") == 0)
    strcpy(utlstr, "GROUP=PointStructure");
  else if (strcmp(structcode, "z") == 0)
    strcpy(utlstr, "GROUP=ZaStructure");
	
  /* Use string search routine (strstr) to move through metadata */
  /* ----------------------------------------------------------- */
  metaptr = strstr(metabuf, utlstr);
	
  /* Save current metadata pointer */
  /* ----------------------------- */
  prevmetaptr = metaptr;
	

  /* First loop for "old-style" (non-ODL) metadata string */
  /* ---------------------------------------------------- */
  if (strcmp(structcode, "s") == 0)
    sprintf(utlstr, "%s%s%s", "SwathName=\"", structname, "\"");
  else if (strcmp(structcode, "g") == 0)
    sprintf(utlstr, "%s%s%s", "GridName=\"", structname, "\"");
  else if (strcmp(structcode, "p") == 0)
    sprintf(utlstr, "%s%s%s", "PointName=\"", structname, "\"");
  else if (strcmp(structcode, "z") == 0)
    sprintf(utlstr, "%s%s%s", "ZaName=\"", structname, "\"");
    
  /* Do string search */
  /* ---------------- */
  metaptr = strstr(metaptr, utlstr);
	
  /*
   * If not found then return to previous position in metadata and look for
   * "new-style" (ODL) metadata string
   */
  if (metaptr == NULL)
    {
      sprintf(utlstr, "%s%s%s", "GROUP=\"", structname, "\"");
      metaptr = strstr(prevmetaptr, utlstr);
    }
	
  /* Find group within structure */
  /* --------------------------- */
  if (groupname != NULL)
    {
      sprintf(utlstr, "%s%s", "GROUP=", groupname);
      metaptr = strstr(metaptr, utlstr);
		
      sprintf(utlstr, "%s%s", "\t\tEND_GROUP=", groupname);
      endptr = strstr(metaptr, utlstr);
    } else
    {
      /* If groupname == NULL then find end of structure in metadata */
      /* ----------------------------------------------------------- */
      sprintf(utlstr, "%s", "\n\tEND_GROUP=");
      endptr = strstr(metaptr, utlstr);
    }
	

  /* Return beginning and ending pointers */
  /* ------------------------------------ */
  metaptrs[0] = metaptr;
  metaptrs[1] = endptr;

  /* Release the dataspace ID */
  /* ------------------------ */
  for (ism = 0; ism < nmeta; ism++)
    {
      status = H5Sclose(aspace[ism]);
      if (status == FAIL )
	{
	  sprintf(errbuf,  "Cannot release the dataspace ID. \n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(metabuf);
	  free(utlstr);
	  
	  return(NULL);
	}
      
      /* Release the datatype ID */
      /* ----------------------- */
      status = H5Tclose(atype[ism]);
      if (status == FAIL )
	{
	  sprintf(errbuf,"Cannot release the datatype ID. \n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(metabuf);
	  free(utlstr);
	  
	  return(NULL);
	}
      
      /* Release the dataset ID */
      /* ---------------------- */
      status = H5Dclose(meta_id[ism]);
      if (status == FAIL )
	{
	  sprintf(errbuf,"Cannot release the dataset ID. \n");
	  H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(metabuf);
	  free(utlstr);
	  
	  return(NULL);
	}
    }
  /* Release the "HDFEOS INFORMATION" group ID */
  /* ----------------------------------------- */ 
  status = H5Gclose(gid);
  if (status == FAIL )
    {
      sprintf(errbuf,"Cannot release the \"HDFEOS INFORMATION\" group ID. \n");
      H5Epush(__FILE__, "HE5_EHmetagroup_Dynamic", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(metabuf);
      free(utlstr);
      
      return(NULL);
    }
  
  free(errbuf);
  free(utlstr);
    
 COMPLETION:
  return(metabuf);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHbisect                                                     |
|                                                                             |
|  DESCRIPTION: Finds root of function using bisection                        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  func()         double              Function to bisect                      |
|  funcParms      double              Function parameters (fixed)             |
|  nParms         long                Number of function parameters           |
|  limLft         double              Lower limit of function arguement       |
|  limRgt         double              Upper limit of function arguement       |
|  convCrit       double              Convergence criterion                   |
|                                                                             |
|  OUTPUTS:                                                                   |
|  root           double              Function root                           |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_EHbisect(double(*func) (double[]), double funcParms[], long nParms, double limLft, double limRgt, double convCrit, double *root)
{
  herr_t         status  = SUCCEED;/* routine return status variable    */

  int            i;                /* Loop index                        */

  double         midPnt       = 0.;/* Mid-point value                   */
  double         newmidPnt    = 0.;/* New mid-point value               */
  double         funcLft      = 0.;/* Function value at left-hand limit */
  double         funcMid      = 0.;/* Function value at mid-point       */
  double         funcRgt      = 0.;/* Function value at right-hand limit*/
  double         *parms=(double *)NULL;/* Function parameters           */


  /* Allocate space for function parameters */
  /* -------------------------------------- */
  parms = (double *)calloc(nParms + 1, sizeof(double));
  if(parms == NULL)
    {
      H5Epush(__FILE__,"HE5_EHbisect", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
      HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
      return(FAIL);
    }
	
  /* Copy (fixed) function parameters */
  /* -------------------------------- */
  for (i = 0; i < nParms; i++)
    parms[i + 1] = funcParms[i];

  /* Copy left-hand limit to "floating" parameter */
  /* -------------------------------------------- */
  parms[0] = limLft;

  /* Determine function value */
  /* ------------------------ */
  funcLft = (*func) (parms);

  /* Copy right-hand limit to "floating" parameter */
  /* --------------------------------------------- */
  parms[0] = limRgt;

  /* Determine function value */
  /* ------------------------ */
  funcRgt = (*func) (parms);

  /* If left and right limits function values of same sign then no root */
  /* ------------------------------------------------------------------ */
  if (funcLft * funcRgt > 0)
    {
      if (parms != NULL) free(parms);
      return (FAIL);
    }
  /* Compute (initial) mid-point */
  /* --------------------------- */
  newmidPnt = 0.5 * (limLft + limRgt);
	
  /* Bisection Loop */
  /* -------------- */
  while (1)
    {
      /* Compute function at new mid-point */
      /* --------------------------------- */
      midPnt   = newmidPnt;
      parms[0] = midPnt;
      funcMid  = (*func) (parms);
		
      /* If left limit same sign as mid-point move it to mid-point */
      /* --------------------------------------------------------- */
      if (funcLft * funcMid > 0.0)
	{
	  limLft = midPnt;
	}else
	{
	  /* Otherwise move over right-hand limit */
	  /* ------------------------------------ */
	  limRgt = midPnt;
	}
		
		
      /* Compute new mid-point */
      /* --------------------- */
      newmidPnt = 0.5 * (limLft + limRgt);
		
      /* If relative change in midpoint < convergence crit then exit loop */
      /* ---------------------------------------------------------------- */
      if (fabs((newmidPnt - midPnt) / midPnt) < convCrit)
	break;
    }
	
  /* Save root */
  /* --------- */
  *root = newmidPnt;
	
  free(parms);
	
  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|  FUNCTION: HE5_EHattr                                                       |
|                                                                             |
|  DESCRIPTION: Reads/Writes attributes for HDF-EOS structures                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS file ID                         |
|  attrname       char                attribute name                          |
|  numbertype     hid_t               attribute HDF numbertype                |
|  count[]        hsize_t             Number of attribute elements            |
|  wrcode         char                Read/Write Code "w/r"                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|                                                                             |
|  Date        Programmer    Description                                      |
|  =========   ============  ==============================================   |
|  July 1999   A.Muslimov    Changed the sequence of calls in "write attri-   |
|                            bute" block (the change is based on examples     |
|                            provided in "Introduction to HDF5 Release 1.0"   |
|                            Chapter 4, Paragraph 8).                         |
|  9/29/99  A.Muslimov       Added dynamic memory allocation for error message|
|                            buffer. Added error handlings after the function |
|                            calls.                                           |
|  12/13/99 A.Muslimov       Fixed a bug associated with the attempt to       |
|                            close the data space outside the block in which  |
|                            it was created. Put a call to H5Sclose() inside  |
|                            this block and separated the rest by else{}      |
|                            block.                                           |
|  07/12/00 A.Muslimov       Added conversion of disk/file data type ID into  |
|                            memory data type ID before H5Aread() call.       |
|  July 02  S.Zhao           Updated for writing a character string attribute.|
|  Mar  04  S.Zhao           Modified for a character string attribute.       |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHattr(hid_t  grpID, const char *attrname, hid_t numbertype, hsize_t count[], char *wrcode, void *datbuf)
{
  herr_t      status  = FAIL;        /* Return status variable    */

  int         RANK    = 1;	         /* Rank of attribute dataset */     

  hid_t       attid   = FAIL;        /* Attribute ID              */
  hid_t       sid     = FAIL;        /* dataspace ID              */
  hid_t       atype   = FAIL;        /* attribute datatype ID     */
  hid_t       mtype   = FAIL;        /* memory data type ID       */

  char        *errbuf = (char *)NULL;/* Error message buffer      */
  size_t      size    = 0;
    
  CHECKPOINTER(attrname);

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Probe , if Attribute does not exist, Create it */
  /* ---------------------------------------------- */
  H5E_BEGIN_TRY
    {
      attid = H5Aopen_name(grpID,attrname);
    }
  H5E_END_TRY;
	
  /* Write Attribute Section */
  /* ----------------------- */
  if (strcmp(wrcode, "w") == 0)
    {
      if( attid == FAIL)
	{
	  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR)
	      || (numbertype == H5T_C_S1))
	    {
	      /* Create attribute dataspace */
	      /* -------------------------- */
	      sid = H5Screate(H5S_SCALAR);
	      if(sid == FAIL)
		{
		  status = FAIL;
		  sprintf(errbuf, "Cannot create the data space for attribute \"%s\".\n", attrname);
		  H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_DATASPACE, H5E_CANTCREATE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);

		  return(status);
		}      
	      numbertype = H5Tcopy(H5T_C_S1);
	      if( (size_t)count[0] > strlen(datbuf) )
		{
		  size = strlen(datbuf);
		}
	      else
		{
		  size = (size_t)count[0];
		}
	      status = H5Tset_size(numbertype, size);
	      if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot set the total size for the attribute \"%s\".\n", attrname);
		  H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);

		  return(status);
		} 
	    }
	  else
	    {

	      /* Create attribute dataspace */
	      /* -------------------------- */
	      sid    = H5Screate(H5S_SIMPLE);
	      if(sid == FAIL)
		{
		  status = FAIL;
		  sprintf(errbuf, "Cannot create the data space for attribute \"%s\".\n", attrname);
		  H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_DATASPACE, H5E_CANTCREATE, errbuf  );
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
				
		  return(status);
		}

	      /* Extend the dataspace */
	      /* -------------------- */
	      status = H5Sset_extent_simple(sid,RANK,count,NULL);
	      if(status == FAIL)
		{
		  sprintf(errbuf, "Cannot extend data space.\n");
		  H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf  );
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
	    
		  return(status);
		}
	    }

	  /* Create the attribute */
	  /* -------------------- */
	  attid  = H5Acreate(grpID,attrname,numbertype,sid,H5P_DEFAULT);
	  if(attid == FAIL)
	    {
	      status = FAIL;
	      sprintf(errbuf, "Cannot create the attribute \"%s\".\n",attrname);
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ATTR, H5E_CANTCREATE, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
				
	      return(status);
	    }

	  /* Write attribute data */
	  /* -------------------- */
	  status = H5Awrite(attid,numbertype,datbuf);
	  if(status == FAIL)
	    { 
	      status = FAIL;
	      sprintf(errbuf, "Cannot write in data to the attribute.\n");
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	   
	      return(status);
	    }
		  
	  /* Release the dataspace ID */
	  /* ------------------------ */
	  status = H5Sclose(sid);
	  if(status == FAIL)
	    { 
	      sprintf(errbuf, "Cannot release the data space ID.\n");
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
				
	      return(status);
	    }

	  /* Release the attribute ID */
	  /* ------------------------ */
	  status = H5Aclose(attid);
	  if(status == FAIL)
	    { 
	      sprintf(errbuf, "Cannot release the attribute ID.\n");
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ATTR, H5E_CLOSEERROR, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
				
	      return(status);
	    }

	}
      else 
	{
	  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR)
	      || (numbertype == H5T_C_S1))
	    {
	      numbertype = H5Tcopy(H5T_C_S1);
	      if( (size_t)count[0] > strlen(datbuf) )
		{
		  size = strlen(datbuf);
		}
	      else
		{
		  size = (size_t)count[0];
		}
	      status = H5Tset_size(numbertype, size);
	      if( status == FAIL)
		{
		  sprintf(errbuf,"Cannot set the total size for the attribute \"%s\".\n", attrname);
		  H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(errbuf);
 
		  return(status);
		}
	    }

	  /* Write data to the attribute */
	  /* --------------------------- */
	  status = H5Awrite(attid,numbertype,datbuf);
	  if(status == FAIL)
	    { 
	      status = FAIL;
	      sprintf(errbuf, "Cannot write in data to the attribute.\n");
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
				
	      return(status);
	    }
		  
	  /* Release the attribute ID */
	  /* ------------------------ */
	  status = H5Aclose(attid);
	  if(status == FAIL)
	    { 
	      sprintf(errbuf, "Cannot release the attribute ID.\n");
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ATTR, H5E_CLOSEERROR, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
				
	      return(status);
	    }

	}
		
		
    }
	
  /* Read Attribute Section */
  /* ---------------------- */
  else if (strcmp(wrcode, "r") == 0)
    {
      if(attid == FAIL)
	{
	  status = FAIL;
	  sprintf(errbuf, "Attribute \"%s\" not defined.", attrname);
	  H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf  );
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(status);
	}
      else
	{
	  atype  = H5Aget_type(attid);
	  if(atype == FAIL)
	    { 
	      status = FAIL;
	      sprintf(errbuf, "Cannot get the data type ID for attribute \"%s\".\n",attrname);
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
				
	      return(status);
	    }
			

          /* Get the memory data type ID */
          /* --------------------------- */
          mtype = HE5_EHdtype2mtype(atype);

          if (mtype < 0)
	    {
              mtype = atype;
	    }

	  /* Read attribute */
	  /* -------------- */
	  status = H5Aread(attid, mtype, datbuf);
	  if(status == FAIL)
	    { 
	      sprintf(errbuf, "Cannot read out data from the attribute \"%s\".\n",attrname);
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
		
	      return(status);
	    }
       

	  /* Release the attribute datatype ID */
	  /* --------------------------------- */
	  status = H5Tclose(atype);
	  if(status == FAIL)
	    { 
	      sprintf(errbuf, "Cannot release the data type ID.\n");
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
				
	      return(status);
	    }
			
	  /* Release the attribute ID */
	  /* ------------------------ */
	  status = H5Aclose(attid);
	  if(status == FAIL)
	    { 
	      sprintf(errbuf, "Cannot release the attribute ID.\n");
	      H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_ATTR, H5E_CLOSEERROR, errbuf  );
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
				
	      return(status);
	    }
			
	}
		
    }
    
  free(errbuf);

 COMPLETION:    
  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHattrinfo                                                   |
|                                                                             |
|  DESCRIPTION: Returns numbertype and count of given HDF-EOS attribute       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  grpID          hid_t               Attribute group ID                      |
|  attrname       char*               attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datatype       hid_t*              Attribute Data type ID                  |
|  count          hsize_t *           Number of attribute elements            |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|                                                                             |
|  9/29/99  A.Muslimov    Added dynamic memory allocation for error message   |
|                         buffer.                                             |
|  Feb 03   S.Zhao        Changed the type of 'datatype' from an H5T_class_t  |
|                         to an hid_t.                                        |
|  Mar 04   S.Zhao        Modified for a character string attribute.          |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHattrinfo( hid_t grpID, const char *attrname, hid_t *datatype, hsize_t *count)
{
  herr_t      status     = FAIL;         /* Return status variable  */
  
  hid_t       aspace     = FAIL;         /* attribute data space ID */
  hid_t       atype      = FAIL;         /* attribute data type ID  */
  hid_t       attid      = FAIL;         /* attribute object ID     */

  H5T_class_t  classid = H5T_NO_CLASS;   /* data type class ID      */
  
  char        *errbuf    = (char  *)NULL;/* Error message buffer    */
  
  CHECKPOINTER(attrname);

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  
  /* Try to open up specified attribute  */
  /* ----------------------------------  */
  H5E_BEGIN_TRY
    {
      attid = H5Aopen_name(grpID, attrname);
    }
  H5E_END_TRY;
  
  if( attid == FAIL )
    {
      status = FAIL;
      sprintf(errbuf, "Attribute \"%s\" not defined.", attrname);
      H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
		
      return(status);
    }
  
  /* Get attribute data type ID */
  /* -------------------------- */
  atype  = H5Aget_type(attid);
  if(atype == FAIL)
    { 
      status = FAIL;
      sprintf(errbuf, "Cannot get the data type ID for attribute \"%s\".\n",attrname);
      H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf  );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
		
      return(status);
    }
  /* Get attribute data space ID */
  /* --------------------------- */
  aspace = H5Aget_space(attid);
  if(aspace == FAIL)
    { 
      status = FAIL;
      sprintf(errbuf, "Cannot get the data space ID for attribute \"%s\".\n",attrname);
      H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf  );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
		
      return(status);
    }
  
  /* Get attribute Data type class ID */
  /* -------------------------------- */
  classid    = H5Tget_class(atype);
  if( classid == FAIL )
    {
      status = FAIL;
      sprintf(errbuf,"Cannot get the data type class ID for attribute \"%s\".\n", attrname);
      H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
		
      return(status);
    }
  
  /* Get the number of attribute elements */
  /* ------------------------------------ */
  if (classid == H5T_STRING)
    {
      *count = H5Sget_simple_extent_npoints(aspace);
      if( *count == 0)
	{
          status = FAIL;
          sprintf(errbuf,"Cannot get the size of string in attribute \"%s\".\n", attrname);
          H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          free(errbuf);
          return(status);
	}
      *datatype = HE5T_CHARSTRING;
    }
  else
    {
      *count = H5Sget_simple_extent_npoints(aspace);
      if( *count == 0)
	{
	  status = FAIL;
	  sprintf(errbuf,"Cannot get the number of elements in attribute \"%s\".\n", attrname);
	  H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
		
	  return(status);
	}
      *datatype = HE5_EHdtype2numtype(atype);
      if(*datatype == FAIL)
        {
          status = FAIL;
          sprintf(errbuf,"Cannot get the number type in attribute \"%s\".\n", attrname);
          H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          free(errbuf);
          return(status);
        }
        
    }

  
  /* Release the data type ID */
  /* ------------------------ */
  status = H5Tclose(atype);
  if(status == FAIL)
    { 
      sprintf(errbuf, "Cannot release the data type ID.\n");
      H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf  );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
		
      return(status);
    }
  
  /* Release the data space ID */
  /* ------------------------- */	
  status = H5Sclose(aspace);
  if(status == FAIL)
    { 
      sprintf(errbuf, "Cannot release the data space ID.\n");
      H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf  );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
		
      return(status);
    }
  
  /* Release the attribute ID */
  /* ------------------------ */
  status = H5Aclose(attid);
  if(status == FAIL)
    { 
      sprintf(errbuf, "Cannot release the attribute ID.\n");
      H5Epush(__FILE__, "HE5_EHattrinfo", __LINE__, H5E_ATTR, H5E_CLOSEERROR, errbuf  );
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);

      return(status);
    }
  
  free(errbuf);
  errbuf = NULL;
  
 COMPLETION:
  return(status);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHobj_info                                                   |
|                                                                             |
|  DESCRIPTION: Returns number and names of objects in HDF-EOS structure      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS:                                                                    |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|   10/15/99 A.Muslimov   Replaced memcmp() by strncmp() to fix an ABR        |
|                         associated with the the first if() block for        |
|                         name string containing less than 13 characters.     |
|   04/19/00 A.Muslimov   Changed type of 'slen' from long to size_t.         |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_EHobj_info(hid_t loc_id, const char *name, void *opdata)
{
  size_t        slen     = 0;
    
  HE5_OBJINFO   *ptr;     

  char          *indxstr = "_INDEXMAP:";
  char          *bsom    = "_BLKSOM";
  char          *lvwr    = "_LevelWritten";

#ifdef _IGNORE_DIMSCALE_
  hid_t		attid    = FAIL;
  hid_t		atype    = FAIL;
  H5T_class_t	atclass;
  hid_t		suptype  = FAIL;
  char		errbuf[HE5_HDFE_ERRBUFSIZE];
#endif

  ptr = (HE5_OBJINFO *)opdata;
   
#ifdef _IGNORE_DIMSCALE_
  H5E_BEGIN_TRY
  {
      attid = H5Aopen_name(loc_id,name);
  }
  H5E_END_TRY;
  if ( attid != FAIL )
  {
      /* Check if the type class is H5T_VLEN, and its base type is H5T_STD_REF_OBJ. */
      atype  = H5Aget_type(attid);
      if ( atype == FAIL)
      { 
	  sprintf(errbuf, "Cannot get the data type ID for attribute \"%s\".\n", name);
	  H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf  );
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  H5Aclose(attid);
	  return 0;
      }
      atclass = H5Tget_class(atype);
      if ( atclass == H5T_NO_CLASS )
      {
	  sprintf(errbuf, "Cannot get the data type class for attribute \"%s\".\n", name);
	  H5Epush(__FILE__, "HE5_EHattr", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf  );
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  H5Aclose(attid);
	  return 0;
      }
      else if ( atclass == H5T_VLEN )
      {
	  suptype = H5Tget_super(atype);
	  if ( suptype >= 0 )
	  {
	      if ( H5Tequal(suptype, H5T_STD_REF_OBJ) == TRUE )
	      {
		  /* Do not append the attribute name because this attribute is not a proper
		   * attribute like "DIMENSION_LIST". */
		  H5Aclose(attid);
		  return 0;
	      }
	  }
      }
      H5Aclose(attid);
  }

#endif
   
  if (  strncmp(name, indxstr, strlen(indxstr)) != 0 && strncmp(name, bsom, strlen(bsom)) != 0  &&  strncmp(name, lvwr, strlen(lvwr)) != 0  )
    {

      /* Get the number of objects */
      /* ------------------------- */
      ptr->count++;
		
      /* Compute string length of object entry */
      /* ------------------------------------- */
      slen = ( ptr->count == 1) ? strlen(name) : strlen(name) + 1;
      ptr->strsize = ptr->strsize + slen;

      /* Get comma separated name list */
      /* ----------------------------- */
      if( ptr->count == 1)
	{ 
	  ptr->name = (char *)calloc( (strlen(name)+1),  sizeof(char) );
	  strcpy( ptr->name, name);
	}
      else 
	{
	  ptr->name = (char *)realloc( ptr->name, sizeof(char)*(ptr->strsize+1));
	  strcat( ptr->name, ",");
	  strcat( ptr->name, name);
	}
    }
  
  return 0;
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHinquire                                                    |
|                                                                             |
|  DESCRIPTION: Returns number and names of HDF-EOS structures in file        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nobj           long                Number of HDF-EOS structures in file    |
|                                                                             |
|  INPUTS:                                                                    |
|  filename       char                HDF-EOS filename                        |
|  grpname        char                Object Group name("SWATH/GRID/POINT/ZA")|
|                                                                             |
|  OUTPUTS:                                                                   |
|  objectlist     char                List of object names (comma-separated)  |
|  strbufsize     long                Length of objectlist                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  9/01/99  A.Muslimov    Checked for the statuses returned by H5Gclose() and |
|                         H5Fclose() calls.                                   |
|  9/29/99  A.Muslimov    Added dynamic memory allocation for error message   |
|                         buffer.                                             |
|  Nov. 00  A.Muslimov    Disabled error message and set return value to      |
|                         "0" if a specified group is not found.              |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHinquire(const char  *filename,  char *grpname, char *objectlist, long *strbufsize)
{
  long            nobj    = FAIL;/* Number of objects in the group   */

  herr_t          status  = FAIL;/* Return status variable           */
    
  HE5_OBJINFO     inout;         /* Output "info" data structure     */

  hid_t           HDFfid  = FAIL;/* HDFEOS file ID                   */
  hid_t           heos_id = FAIL;/* Group ID for a specified group   */

  int             index   = FAIL;/* Return value from an operator    */

  char            *errbuf;		 /* Buffer for error message         */

  CHECKPOINTER(filename);
  CHECKPOINTER(grpname);

  /* Initialize the data */
  /* ------------------- */
  inout.strsize = 0;
  inout.count   = 0;
  inout.name    = (char *)NULL;
	

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHinquire", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Try to open HDFEOS file for read-only */
  /* ------------------------------------- */
  H5E_BEGIN_TRY
    {
      HDFfid = H5Fopen(filename, H5F_ACC_RDONLY, H5P_DEFAULT);   
    }
  H5E_END_TRY;
  if(HDFfid == FAIL )  
    {
      sprintf(errbuf, " Cannot open \"%s\" file. Check the file name. \n", filename);
      H5Epush(__FILE__, "HE5_EHinquire", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      nobj = FAIL;
      free(errbuf);		
      return(nobj);
    }

  /* Try to open  group "grpname" */
  /* ---------------------------  */
  H5E_BEGIN_TRY
    {
      heos_id = H5Gopen(HDFfid, grpname);
    }
  H5E_END_TRY;
  if(heos_id == FAIL)
    {
      nobj = 0;
      H5Fclose(HDFfid); 
      free(errbuf);		
      return(nobj);
    }

	
  /* Loop to get hdfeos object information */
  /* ------------------------------------- */
  index = H5Giterate(heos_id, grpname,  NULL, HE5_EHobj_info, &inout);
  if( index == FAIL)
    {
      H5Epush(__FILE__, "HE5_EHinquire",__LINE__, H5E_FUNC, H5E_CANTINIT, "Cannot get HDFEOS object information. \n");
      HE5_EHprint("Error: Cannot get HDFEOS object information, occured", __FILE__, __LINE__);
      nobj = FAIL;
      goto done;
    }
	
  if(objectlist != (char *)NULL)
    memmove(objectlist, inout.name, strlen(inout.name));
  
  if(strbufsize != (long *)NULL) 
    *strbufsize = inout.strsize;
  
  nobj = inout.count;
  
 done:
  
  /* inout.name is allocated in HE5_EHobj_info() */
  /* ------------------------------------------- */
  if( inout.name != NULL)
    free( inout.name);
  
  
  /* Release group ID */
  /* ---------------- */
  status = H5Gclose(heos_id);
  if( status == FAIL)
    {
      sprintf(errbuf, "Cannot close the group \"%s\". \n", grpname);
      H5Epush(__FILE__, "HE5_EHinquire",__LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      errbuf = NULL;
      return(nobj);
    }

  /* Release file ID */
  /* --------------- */
  status = H5Fclose(HDFfid);
  if( status == FAIL)
    {
      sprintf(errbuf, "Cannot close the HDFEOS file \"%s\" . \n", filename);
      H5Epush(__FILE__, "HE5_EHinquire",__LINE__, H5E_FILE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  free(errbuf);
  errbuf = NULL;

 COMPLETION:  
  return(nobj);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHattrcat                                                    |
|                                                                             |
|  DESCRIPTION: Returns a listing of attributes within an HDF-EOS structure   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nobj           long                Number of attributes in a struct        |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS file ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  grpname        char*               Group name                              |
|  objectlist     char*               Attribute names in a struct             |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  NOTES:  attributes "INDXMAP:..", "_FV_", "_BLKSOM:.." are not counted.     |
|                                                                             |
|  9/29/99  A.Muslimov    Added dynamic memory allocation for error message   |
|                         buffer.                                             |
|  2/1/00   A.Muslimov    Added H5Dopen(dts_id, ...) call for the case of     |
|                         accessing dataset.                                  |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHattrcat(hid_t fid,  char *grpname, char *objectlist, long *strbufsize)
{
  long            nobj    = FAIL;/* number of attributes in a structure */

  HE5_OBJINFO     inout;

  hid_t           heos_id = FAIL;/* Group ID of a specified group       */
  hid_t           HDFfid  = FAIL;
  hid_t           dts_id  = FAIL;/* Dataset ID of a specified dataset   */
    
  herr_t          status  = FAIL;/* return status variable              */

  int             index   = FAIL;/* return value of index               */

  char            *errbuf = NULL;/* buffer for error message            */

  CHECKPOINTER(grpname);

  /* Initialize the data */
  /* ------------------- */
  inout.strsize = 0;			 /* object list size                    */
  inout.count   = 0;			 /* number of objects                   */
  inout.name    = NULL;			 /* onject name (comma separated) list  */

  /* Get HDF file ID */
  /* --------------- */
  HDFfid = HE5_HeosTable[fid%HE5_EHIDOFFSET].HDFfid;

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHattrcat", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
	

  /* Try to open group with name "grpname" */
  /* ------------------------------------- */
  H5E_BEGIN_TRY
    {
      heos_id = H5Gopen(HDFfid, grpname);
    }
  H5E_END_TRY;
	
  if(heos_id == FAIL)
    {
		
      /* Try to open dataset with name "grpname" */
      /* --------------------------------------- */ 
      H5E_BEGIN_TRY
	{
	  dts_id = H5Dopen(HDFfid, grpname);
	}
      H5E_END_TRY;
		
      if(dts_id == FAIL)
	{
	  sprintf(errbuf, "No group/dataset named \"%s\" exists. \n", grpname);
	  H5Epush(__FILE__, "HE5_EHattrcat", __LINE__, H5E_FILE, H5E_CANTOPENOBJ, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  nobj = FAIL;
	  free(errbuf);
		  
	  return(nobj);
	}
	  
    }
	
  /* Loop to get attributes information */
  /* ---------------------------------- */
  if (heos_id != FAIL)
    index = H5Aiterate(heos_id,  NULL, HE5_EHobj_info, &inout);
	
  if (dts_id != FAIL)
    index = H5Aiterate(dts_id,  NULL, HE5_EHobj_info, &inout);
  if( index == FAIL)
    {
      sprintf(errbuf,"Cannot get attribute information. \n");
      H5Epush(__FILE__, "HE5_EHattrcat", __LINE__, H5E_ATTR, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      nobj = FAIL;
      goto done;
    }

  if(objectlist != NULL && inout.name != NULL ) 
    strcpy( objectlist, inout.name);
  if(strbufsize != NULL)  
    *strbufsize = inout.strsize;
  nobj = inout.count;
	
 done:

  /* inout.name is allocated in HE5_EHobj_info() */
  /* ------------------------------------------- */
  if( inout.name != NULL) free( inout.name);
	

  if (heos_id != FAIL)
    {
      status = H5Gclose(heos_id);
      if( status == FAIL )
	{
	  sprintf(errbuf, "Cannot release the group ID.\n");
	  H5Epush(__FILE__, "HE5_EHattrcat",__LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
    }
	
  if (dts_id != FAIL)
    {
      status = H5Dclose(dts_id);
      if( status == FAIL )
	{
	  sprintf(errbuf, "Cannot release the dataset ID.\n");
	  H5Epush(__FILE__, "HE5_EHattrcat",__LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
    }
    
  free(errbuf);
  errbuf = NULL;
    
 COMPLETION:
  return(nobj);
}





/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHdatasetcat                                                 |
|                                                                             |
|  DESCRIPTION: Returns a listing of datasets within an HDF-EOS structure     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS file ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  9/29/99  A.Muslimov    Added dynamic memory allocation for error message   |
|                         buffer.                                             |
|                                                                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHdatasetcat(hid_t fid,  char *grpname, char *objectlist, long *strbufsize)
{
  long            nobj    = FAIL;/* Number of datasets (RETURN)  */   

  herr_t          status  = FAIL;/* Return status variable       */ 

  int             index   = FAIL;/* Return value of an operation */
    
  HE5_OBJINFO     inout;

  hid_t           heos_id = FAIL;/* Group ID                     */     
  hid_t           HDFfid  = FAIL;/* HDF-EOS file ID              */ 

  char            *errbuf;		 /* buffer for error message     */


  CHECKPOINTER(grpname);

  /* Initialize data */
  /* --------------- */
  inout.strsize = 0;			/*object list size*/
  inout.count   = 0;			/*number of objects*/
  inout.name    = NULL;			/*onject name list, seperated by comma*/

  /* Get HDF file ID */
  /* --------------- */
  HDFfid = HE5_HeosTable[fid%HE5_EHIDOFFSET].HDFfid;

  /* Allocate memory for error message buffer */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHdatasetcat", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

	
  /* Try to open group "grpname" */
  /* --------------------------- */
  H5E_BEGIN_TRY
    {
      heos_id = H5Gopen(HDFfid, grpname);
    }
  H5E_END_TRY;
  if(heos_id == FAIL)
    {
      sprintf(errbuf, "Cannot open \"%s\" group. Group may not exist. \n", grpname);
      H5Epush(__FILE__, "HE5_EHdatasetcat", __LINE__, H5E_FILE, H5E_CANTOPENOBJ, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      nobj = FAIL;
      free(errbuf);
		
      return(nobj);
    }

  /* Loop to get datasets information */
  /* -------------------------------- */
  index = H5Giterate(heos_id, grpname,  NULL, HE5_EHobj_info, &inout);
  if( index == FAIL )
    {
      sprintf(errbuf,"Cannot get datasets information.\n");
      H5Epush(__FILE__, "HE5_EHdatasetcat", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      nobj = FAIL;
      goto done;
    }

  if(objectlist != NULL)  strcpy( objectlist, inout.name);
  if(strbufsize != NULL)  *strbufsize = inout.strsize;
  nobj = inout.count;
	
 done:
  /* inout.name is allocated in HE5_EHobj_info() */
  if( inout.name != NULL) free( inout.name);
	
  status = H5Gclose(heos_id);
  if( status == FAIL )
    {
      sprintf(errbuf, "Cannot release the group ID.\n");
      H5Epush(__FILE__, "HE5_EHdatasetcat",__LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
	
  free(errbuf);
  errbuf = NULL;
	

 COMPLETION:	
  return(nobj);
}





/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHclose                                                      |
|                                                                             |
|  DESCRIPTION: close HDF-EOS file and returns status                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  INPUTS: fid    hid_t   None        HDF-EOS file ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|   =======  ==========   =================================================   |
|   July 99  A.Muslimov   Modified to conform to the HDFEOS file structure    |
|   9/01/99  A.Muslimov   Added error handling right after the calls to       |
|                         H5Gclose() and H5Fclose().                          |
|   9/29/99  A.Muslimov   Added dynamic memory allocation for error message   |
|                         buffer.                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_EHclose(hid_t fid)
{
  herr_t      status  = FAIL;/* routine return status variable  */
  
  hid_t       idx     = FAIL;/* HDF-EOS file index (reduced ID) */
  
  char        *errbuf = NULL;/* Error message  buffer           */
  
  /*
  int i;
  ssize_t nobj,nobj1,nobj2;
  hid_t obj_id_list[100];
  */

   /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHclose", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
    
  /* Check for valid HDFEOS file ID range */
  /* ------------------------------------ */
  if (fid >= HE5_EHIDOFFSET && fid < HE5_NEOSHDF + HE5_EHIDOFFSET)
    {
      /* Compute "reduced" file ID */
      /* ------------------------- */
      idx = fid % HE5_EHIDOFFSET;
		
      /* Close "HDFEOS" group */
      /* -------------------- */
      status = H5Gclose(HE5_HeosTable[ idx ].gid);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release \"HDFEOS\" group ID. \n");
	  H5Epush(__FILE__, "HE5_EHclose", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);

	  return(FAIL);
	}

      /* To see if user has forgotten to use detach somewhere in his/her code and because 
	 of that there is any open IDs before reaching this point,user can uncomment out the 
	 following few lines of code (and a few declerations above), recompile HDFEOS5, and link 
	 the library to the user code */

      /*
      nobj1 = H5Fget_obj_count(HE5_HeosTable[idx].HDFfid,
                               H5F_OBJ_DATASET |  H5F_OBJ_GROUP |  H5F_OBJ_DATATYPE | H5F_OBJ_ATTR);
      nobj2 = H5Fget_obj_count(HE5_HeosTable[idx].HDFfid,H5F_OBJ_ALL);
      printf("nobj1=%d\n",(int)nobj1);
      printf("nobj2=%d\n",(int)nobj2);
      nobj = H5Fget_obj_ids(HE5_HeosTable[idx].HDFfid,
			    H5F_OBJ_DATASET |  H5F_OBJ_GROUP |  H5F_OBJ_DATATYPE | H5F_OBJ_ATTR , 100, obj_id_list);

      printf("nobj=%d\n",(int)nobj);
      for(i=0; i<nobj; i++)
	{
	  char name[1024];
	  memset(name, 0, 1024);
	  ssize_t charsread;
	  charsread = H5Iget_name(obj_id_list[i], name,1024);
	  printf("obj_id_lis=%d name =%s\n", obj_id_list[i], name);
	}
      */

      /* Close HDF-EOS file */
      /* ------------------ */
      status = H5Fclose(HE5_HeosTable[idx].HDFfid);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the file ID. \n");
	  H5Epush(__FILE__, "HE5_EHclose", __LINE__, H5E_FILE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
			
	  return(FAIL);
	}
		
      /* Clear out external array entries */
      /* -------------------------------- */
      HE5_HeosTable[ idx ].active      =   0;
      HE5_HeosTable[ idx ].flags       =   0;
      HE5_HeosTable[ idx ].HDFfid      =   FAIL;
      HE5_HeosTable[ idx ].gid         =   FAIL;
      if(HE5_HeosTable[ idx ].filename != NULL)
	{
	  free(HE5_HeosTable[ idx ].filename);
	  HE5_HeosTable[ idx ].filename = NULL;
	}
      if(Stored_meta[ idx ].saved_metabuf != NULL)
	{
	  free(Stored_meta[ idx ].saved_metabuf);
	  Stored_meta[ idx ].saved_metabuf = NULL;
	}
    }
  else
    {
      status = FAIL;
      sprintf(errbuf,"Invalid file ID: %d . ID should range from %d to %d . \n", fid, HE5_EHIDOFFSET, HE5_NEOSHDF + HE5_EHIDOFFSET);
      H5Epush(__FILE__,  "HE5_EHclose", __LINE__, H5E_FILE, H5E_BADFILE, errbuf); 
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  free(errbuf);
  errbuf = NULL;
  
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHconvdatatype                                               |
|                                                                             |
|  DESCRIPTION: Returns native data types for C functions                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  cdatatype      hid_t               datatype uesd in C functions            |
|                                                                             |
|  INPUTS:                                                                    |
|  fdatatype      int                 code for datatypes                      |
|                                                                             |
|  OUTPUTS:       none                                                        |
|                                                                             |
|  11/06/99      Abe Taaheri                                                  |
|  Aug 00        A.Muslimov           Updated                                 |
|  Mar 02        A.Muslimov           Added more data type tags               |
|  Mar 04        S.Zhao               Added a data type tag HE5T_CHARSTRING   |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t
HE5_EHconvdatatype(int fdatatype)
{
  hid_t      cdatatype = FAIL;            /* return data type ID  */

  char       errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer */
    
  /* Check the input datatype flag and set data type ID */
  /* -------------------------------------------------- */
  if(fdatatype == HE5T_NATIVE_INT || fdatatype == 0)
    cdatatype = H5T_NATIVE_INT;
  else if(fdatatype == HE5T_NATIVE_UINT || fdatatype == 1)
    cdatatype = H5T_NATIVE_UINT;
  else if(fdatatype == HE5T_NATIVE_SHORT || fdatatype == 2)
    cdatatype = H5T_NATIVE_SHORT;
  else if(fdatatype == HE5T_NATIVE_USHORT || fdatatype == 3)
    cdatatype = H5T_NATIVE_USHORT;
  else if(fdatatype == HE5T_NATIVE_SCHAR || fdatatype == 4)
    cdatatype = H5T_NATIVE_SCHAR;
  else if(fdatatype == HE5T_NATIVE_UCHAR || fdatatype == 5)
    cdatatype = H5T_NATIVE_UCHAR;
  else if(fdatatype == HE5T_NATIVE_LONG || fdatatype == 6)
    cdatatype = H5T_NATIVE_LONG;
  else if(fdatatype == HE5T_NATIVE_ULONG || fdatatype == 7)
    cdatatype = H5T_NATIVE_ULONG;
  else if(fdatatype == HE5T_NATIVE_LLONG || fdatatype == 8)
    cdatatype = H5T_NATIVE_LLONG;
  else if(fdatatype == HE5T_NATIVE_ULLONG || fdatatype == 9)
    cdatatype = H5T_NATIVE_ULLONG;
  else if(fdatatype == HE5T_NATIVE_FLOAT || fdatatype == 10)
    cdatatype = H5T_NATIVE_FLOAT;
  else if(fdatatype == HE5T_NATIVE_DOUBLE || fdatatype == 11)
    cdatatype = H5T_NATIVE_DOUBLE;
  else if(fdatatype == HE5T_NATIVE_LDOUBLE || fdatatype == 12)
    cdatatype = H5T_NATIVE_LDOUBLE;
  else if(fdatatype == HE5T_NATIVE_INT8 || fdatatype == 13)
    {
      cdatatype = H5Tcopy(H5T_NATIVE_INT);
      H5Tset_size(cdatatype, 1);
    }
  else if(fdatatype == HE5T_NATIVE_UINT8 || fdatatype == 14)
    {
      cdatatype = H5Tcopy(H5T_NATIVE_UINT);
      H5Tset_size(cdatatype, 1);
    }
  else if(fdatatype == HE5T_NATIVE_INT16 || fdatatype == 15)
    {
      cdatatype = H5Tcopy(H5T_NATIVE_INT);
      H5Tset_size(cdatatype, 2);
    }
  else if(fdatatype == HE5T_NATIVE_UINT16 || fdatatype == 16)
    {
      cdatatype = H5Tcopy(H5T_NATIVE_UINT);
      H5Tset_size(cdatatype, 2);
    }
  else if(fdatatype == HE5T_NATIVE_INT32 || fdatatype == 17)
    {
      cdatatype = H5Tcopy(H5T_NATIVE_INT);
      H5Tset_size(cdatatype, 4);
    }
  else if(fdatatype == HE5T_NATIVE_UINT32 || fdatatype == 18)
    {
      cdatatype = H5Tcopy(H5T_NATIVE_UINT);
      H5Tset_size(cdatatype, 4);
    }
  else if(fdatatype == HE5T_NATIVE_INT64 || fdatatype == 19)
    {
      cdatatype = H5Tcopy(H5T_NATIVE_INT);
      H5Tset_size(cdatatype, 8);
    }
  else if(fdatatype == HE5T_NATIVE_UINT64 || fdatatype == 20)
    {
      cdatatype = H5Tcopy(H5T_NATIVE_UINT);
      H5Tset_size(cdatatype, 8);
    }
  else if(fdatatype == HE5T_NATIVE_B8 || fdatatype == 21)
    cdatatype = H5T_NATIVE_B8;
  else if(fdatatype == HE5T_NATIVE_B16 || fdatatype == 22)
    cdatatype = H5T_NATIVE_B16;
  else if(fdatatype == HE5T_NATIVE_B32 || fdatatype == 23)
    cdatatype = H5T_NATIVE_B32;
  else if(fdatatype == HE5T_NATIVE_B64 || fdatatype == 24)
    cdatatype = H5T_NATIVE_B64;
  else if(fdatatype == HE5T_NATIVE_HSIZE || fdatatype == 25)
    cdatatype = H5T_NATIVE_HSIZE;
  else if(fdatatype == HE5T_NATIVE_HERR || fdatatype == 26)
    cdatatype = H5T_NATIVE_HERR;
  else if(fdatatype == HE5T_NATIVE_HBOOL || fdatatype == 27)
    cdatatype = H5T_NATIVE_HBOOL;
  else if(fdatatype == HE5T_STD_I8BE || fdatatype == 28 || fdatatype == HE5T_STD_I8LE || fdatatype == 29)
    cdatatype = H5T_NATIVE_SCHAR;
  else if(fdatatype == HE5T_STD_I16BE || fdatatype == 30 || fdatatype == HE5T_STD_I16LE || fdatatype == 31)
    cdatatype = H5T_NATIVE_SHORT;
  else if(fdatatype == HE5T_STD_I32BE || fdatatype == 32 || fdatatype == HE5T_STD_I32LE || fdatatype == 33)
    cdatatype = H5T_NATIVE_INT;
  else if(fdatatype == HE5T_STD_I64BE || fdatatype == 34 || fdatatype == HE5T_STD_I64LE || fdatatype == 35)
    cdatatype = H5T_NATIVE_LONG;
  else if(fdatatype == HE5T_STD_U8BE || fdatatype == 36 || fdatatype == HE5T_STD_U8LE || fdatatype == 37)
    cdatatype = H5T_NATIVE_UCHAR;
  else if(fdatatype == HE5T_STD_U16BE || fdatatype == 38 || fdatatype == HE5T_STD_U16LE || fdatatype == 39)
    cdatatype = H5T_NATIVE_USHORT;
  else if(fdatatype == HE5T_STD_U32BE || fdatatype == 40 || fdatatype == HE5T_STD_U32LE || fdatatype == 41)
    cdatatype = H5T_NATIVE_UINT;
  else if(fdatatype == HE5T_STD_U64BE || fdatatype == 42 || fdatatype == HE5T_STD_U64LE || fdatatype == 43)
    cdatatype = H5T_NATIVE_ULONG;
  else if(fdatatype == HE5T_STD_B8BE || fdatatype == 44)
    cdatatype = H5T_STD_B8BE;
  else if(fdatatype == HE5T_STD_B8LE || fdatatype == 45)
    cdatatype = H5T_STD_B8LE;
  else if(fdatatype == HE5T_STD_B16BE || fdatatype == 46)
    cdatatype = H5T_STD_B16BE;
  else if(fdatatype == HE5T_STD_B16LE || fdatatype == 47)
    cdatatype = H5T_STD_B16LE;
  else if(fdatatype == HE5T_STD_B32BE || fdatatype == 48)
    cdatatype = H5T_STD_B32BE;
  else if(fdatatype == HE5T_STD_B32LE || fdatatype == 49)
    cdatatype = H5T_STD_B32LE;
  else if(fdatatype == HE5T_STD_B64BE || fdatatype == 50)
    cdatatype = H5T_STD_B64BE;
  else if(fdatatype == HE5T_STD_B64LE || fdatatype == 51)
    cdatatype = H5T_STD_B64LE;
  else if(fdatatype == HE5T_IEEE_F32BE || fdatatype == 52 || fdatatype == HE5T_IEEE_F32LE || fdatatype == 53)
    cdatatype = H5T_NATIVE_FLOAT;
  else if(fdatatype == HE5T_IEEE_F64BE || fdatatype == 54 || fdatatype == HE5T_IEEE_F64LE || fdatatype == 55)
    cdatatype = H5T_NATIVE_DOUBLE;
  else if(fdatatype == HE5T_NATIVE_CHAR || fdatatype == 56)
    cdatatype = H5T_NATIVE_CHAR;
  else if(fdatatype == HE5T_CHARSTRING || fdatatype == 57)
    cdatatype = H5T_C_S1;
  /* The error will be issued after the call */
  /*
  else
    {
      sprintf(errbuf,"Invalid FORTRAN datatype flag (\"%d\").\n", fdatatype);
      H5Epush(__FILE__,  "HE5_EHconvdatatype", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf); 
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  */
  return(cdatatype);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHdtype2numtype                                              |
|                                                                             |
|  DESCRIPTION: Takes file data type ID, and converts it to a number type ID  |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|  numtype         hid_t              Number type ID                          |
|                                                                             |
|  INPUTS:                                                                    |
|  dtype           hid_t              File data type ID                       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jun 02    S.Zhao       Original development                                |
|  Dec 03    S.Zhao       Updated                                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t
HE5_EHdtype2numtype(hid_t dtype)
{
  hid_t      numtype = FAIL;
  /*  char       *errbuf = NULL;*/    /* Error message buffer */
  char       errbuf[HE5_HDFE_ERRBUFSIZE];
 
 
  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  /*
    errbuf = (char *) calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
    if(errbuf == NULL)
    {
    H5Epush(__FILE__, "HE5_EHdtype2numtype", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
    HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
    return(FAIL);
    }
  */
 
  if ((dtype == HE5T_CHARSTRING) ||
      H5Tequal(dtype,H5T_C_S1))
    numtype = HE5T_CHARSTRING;
  else if(H5Tequal(dtype, H5T_NATIVE_INT) ||
          H5Tequal(dtype,H5T_STD_I32BE) ||
          H5Tequal(dtype,H5T_STD_I32LE))
    numtype = HE5T_NATIVE_INT;
  else if(H5Tequal(dtype, H5T_NATIVE_UINT) ||
          H5Tequal(dtype,H5T_STD_U32BE) ||
          H5Tequal(dtype,H5T_STD_U32LE))
    numtype = HE5T_NATIVE_UINT;
  else if(H5Tequal(dtype, H5T_NATIVE_SHORT))
    numtype = HE5T_NATIVE_SHORT;
  else if(H5Tequal(dtype, H5T_NATIVE_USHORT))
    numtype = HE5T_NATIVE_USHORT;
  else if(H5Tequal(dtype, H5T_NATIVE_SCHAR))
    numtype = HE5T_NATIVE_SCHAR;
  else if(H5Tequal(dtype, H5T_NATIVE_UCHAR))
    numtype = HE5T_NATIVE_UCHAR;
  else if(H5Tequal(dtype, H5T_NATIVE_LONG) ||
          H5Tequal(dtype,H5T_STD_I64BE) ||
          H5Tequal(dtype,H5T_STD_I64LE))
    numtype = HE5T_NATIVE_LONG;
  else if(H5Tequal(dtype, H5T_NATIVE_ULONG) ||
          H5Tequal(dtype,H5T_STD_U64BE) ||
          H5Tequal(dtype,H5T_STD_U64LE))
    numtype = HE5T_NATIVE_ULONG;
  else if(H5Tequal(dtype, H5T_NATIVE_LLONG))
    numtype = HE5T_NATIVE_LLONG;
  else if(H5Tequal(dtype, H5T_NATIVE_ULLONG))
    numtype = HE5T_NATIVE_ULLONG;
  else if(H5Tequal(dtype, H5T_NATIVE_FLOAT) ||
          H5Tequal(dtype,H5T_IEEE_F32BE) ||
          H5Tequal(dtype,H5T_IEEE_F32LE))
    numtype = HE5T_NATIVE_FLOAT;
  else if(H5Tequal(dtype, H5T_NATIVE_DOUBLE) ||
          H5Tequal(dtype,H5T_IEEE_F64BE) ||
          H5Tequal(dtype,H5T_IEEE_F64LE))
    numtype = HE5T_NATIVE_DOUBLE;
  else if(H5Tequal(dtype, H5T_NATIVE_LDOUBLE))
    numtype = HE5T_NATIVE_LDOUBLE;
  else if(H5Tequal(dtype, H5T_NATIVE_INT8) ||
          H5Tequal(dtype,H5T_STD_I8BE) ||
          H5Tequal(dtype,H5T_STD_I8LE)) 
    numtype = HE5T_NATIVE_INT8;
  else if(H5Tequal(dtype, H5T_NATIVE_UINT8) ||
          H5Tequal(dtype,H5T_STD_U8BE) ||
          H5Tequal(dtype,H5T_STD_U8LE))
    numtype = HE5T_NATIVE_UINT8;
  else if(H5Tequal(dtype, H5T_NATIVE_INT16) ||
          H5Tequal(dtype,H5T_STD_I16BE) ||
          H5Tequal(dtype,H5T_STD_I16LE))
    numtype = HE5T_NATIVE_INT16;
  else if(H5Tequal(dtype, H5T_NATIVE_UINT16) ||
          H5Tequal(dtype,H5T_STD_U16BE) ||
          H5Tequal(dtype,H5T_STD_U16LE))
    numtype = HE5T_NATIVE_UINT16;
  else if(H5Tequal(dtype, H5T_NATIVE_INT32))
    numtype = HE5T_NATIVE_INT32;
  else if(H5Tequal(dtype, H5T_NATIVE_UINT32))
    numtype = HE5T_NATIVE_UINT32;
  else if(H5Tequal(dtype, H5T_NATIVE_INT64))
    numtype = HE5T_NATIVE_INT64;
  else if(H5Tequal(dtype, H5T_NATIVE_UINT64))
    numtype = HE5T_NATIVE_UINT64;
  else if(H5Tequal(dtype, H5T_NATIVE_B8) ||
          H5Tequal(dtype,H5T_STD_B8BE) ||
          H5Tequal(dtype,H5T_STD_B8LE))
    numtype = HE5T_NATIVE_B8;
  else if(H5Tequal(dtype, H5T_NATIVE_B16) ||
          H5Tequal(dtype,H5T_STD_B16BE) ||
          H5Tequal(dtype,H5T_STD_B16LE))
    numtype = HE5T_NATIVE_B16;
  else if(H5Tequal(dtype, H5T_NATIVE_B32) ||
          H5Tequal(dtype,H5T_STD_B32BE) ||
          H5Tequal(dtype,H5T_STD_B32LE))
    numtype = HE5T_NATIVE_B32;
  else if(H5Tequal(dtype, H5T_NATIVE_B64) ||
          H5Tequal(dtype,H5T_STD_B64BE) ||
          H5Tequal(dtype,H5T_STD_B64LE))
    numtype = HE5T_NATIVE_B64;
  else if(H5Tequal(dtype, H5T_NATIVE_HSIZE))
    numtype = HE5T_NATIVE_HSIZE;
  else if(H5Tequal(dtype, H5T_NATIVE_HERR))
    numtype = HE5T_NATIVE_HERR;
  else if(H5Tequal(dtype, H5T_NATIVE_HBOOL))
    numtype = HE5T_NATIVE_HBOOL;
  else if (H5Tequal(dtype, H5T_NATIVE_CHAR))
    numtype = HE5T_NATIVE_CHAR;
  else {
    numtype = FAIL;
    sprintf(errbuf,"The datatype \"%d\" is not supported (unknown).\n", (int)dtype);
    H5Epush(__FILE__, "HE5_EHdtype2numtype", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
    HE5_EHprint(errbuf, __FILE__, __LINE__);
  }
 
  /*free(errbuf);*/
 
  return(numtype);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHwriteglbattr                                               |
|                                                                             |
|  DESCRIPTION: Writes/updates attribute associated with the "FILE_ATTRIBUTES"|
|               group in a Swath/Grid/Point/ZA.                               |
|                                                                             |
|                                -----------                                  |
|                               | Root "/"  |                                 |
|                                -----------                                  |
|                               /           \                                 |
|               ----------------------    ---------------------               |
|               | HDFEOS INFORMATION |    |       HDFEOS      |               |
|               ----------------------    ---------------------               |
|               /            |            /   / \   \        \                |
|              /             |           /   /   \   \        \               |
|    <HDFEOSVersion> [StructMetadata.0] /   /     \   \        \              |
|                                      /   |       |   \        \             |
|                            ----------  ------ ------- ------- --------      |
|                           |ADDITIONAL| |GRID| |POINT| |SWATH| |  ZA  |      |
|                            ----------  ------ ------- ------- --------      |
|                                /         :       :        :       :         |
|                               /        [dts]   [dts]    [dts]   [dts]       |
|                      -----------------                                      |
|                      |FILE_ATTRIBUTES|                                      |
|                      -----------------                                      |
|                               :                                             |
| Writes/Updates data here=> <attrs>                                          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDFEOS File ID                          |
|  attrname       char                attribute name                          |
|  numbertype     hid_t               attribute dataset datatype ID           |
|  count[]        hsize_t             Number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 01   A.Muslimov    Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_EHwriteglbattr(hid_t fid, const char *attrname, hid_t numbertype, hsize_t  count[], void *datbuf)
{
  herr_t     status          = FAIL;     /* Return status variable     */

  uintn      access          =  0;	     /* Access code                */

  hid_t      gid             = FAIL;     /* "HDFEOS" group ID          */

  hid_t      AddGrpID        = FAIL;     /* "ADDITIONAL" group ID      */
  hid_t      FileAttrGrpID   = FAIL;     /* "FILE_ATTRIBUTES" group ID */
  hid_t      HDFfid          = FAIL;	 /* HDF-EOS file ID            */
   
  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer       */

  CHECKNAME(attrname);
  CHECKPOINTER(count);
  CHECKPOINTER(datbuf);


  /* Get HDF-EOS file ID and "HDFEOS" group ID */
  /* ========================================= */
  status = HE5_EHchkfid(fid,"HE5_EHwriteglbattr", &HDFfid, &gid, &access);
  if(status == FAIL)
    {
      H5Epush(__FILE__, "HE5_EHwriteglbattr", __LINE__, H5E_FILE, H5E_NOTFOUND, "Invalid file ID. \n");
      HE5_EHprint("Error: Invalid file ID, occured", __FILE__, __LINE__);
     
      return(FAIL);
    }

  if (status == SUCCEED)
    {
      /* Get "ADDITIONAL" group ID */
      /* ------------------------- */
      AddGrpID = H5Gopen(gid, "ADDITIONAL");
      if(AddGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"ADDITIONAL\" group. \n");
	  H5Epush(__FILE__, "HE5_EHwriteglbattr", __LINE__, H5E_OHDR, H5E_NOTFOUND,errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	} 


      /* Get "FILE_ATTRIBUTES" group ID and call HE5_EHattr to perform I/O */
      /* ----------------------------------------------------------------- */
      FileAttrGrpID = H5Gopen(AddGrpID, "FILE_ATTRIBUTES");
      if(FileAttrGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"FILE_ATTRIBUTES\" group. \n");
	  H5Epush(__FILE__, "HE5_EHwriteglbattr", __LINE__, H5E_OHDR, H5E_NOTFOUND,errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	} 

      status = HE5_EHattr(FileAttrGrpID, attrname, numbertype, count,"w", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write Attribute \"%s\"to the \"FILE_ATTRIBUTES\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_EHwriteglbattr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Gclose(AddGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"ADDITIONAL\" group ID. \n");
	  H5Epush(__FILE__, "HE5_EHwriteglbattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Gclose(FileAttrGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"FILE_ATTRIBUTES\" group ID. \n");
	  H5Epush(__FILE__, "HE5_EHwriteglbattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_EHreadglbattr                                                |
|                                                                             |
|  DESCRIPTION: Reads attribute from the "FILE_ATTRIBUTES" group in a file.   |
|                                                                             |
|                                                                             |
|                                -----------                                  |
|                                | Root "/"|                                  |
|                                -----------                                  |
|                               /           \                                 |
|               ----------------------    ---------------------               |
|               | HDFEOS INFORMATION |    |       HDFEOS      |               |
|               ----------------------    ---------------------               |
|               /            |            /   / \   \        \                |
|              /             |           /   /   \   \        \               |
|    <HDFEOSVersion> [StructMetadata.0] /   /     \   \        \              |
|                                      /   |       |   \        \             |
|                            ----------  ------ ------- ------- --------      |
|                           |ADDITIONAL| |GRID| |POINT| |SWATH| |  ZA  |      |
|                            ----------  ------ ------- ------- --------      |
|                                /         :       :        :       :         |
|                               /        [dts]   [dts]    [dts]   [dts]       |
|                      -----------------                                      |
|                      |FILE_ATTRIBUTES|                                      |
|                      -----------------                                      |
|                             :                                               |
| Reads attributes from => <attrs>                                            |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               file ID                                 |
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
|  May 01   A.Muslimov    Original development.                               |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_EHreadglbattr(hid_t fid, const char *attrname, void * datbuf)
{
  herr_t      status          = FAIL;     /* return status variable     */
  
  uintn       access          =  0;	      /* Access code                */

  hid_t       gid             = FAIL;     /* "HDFEOS" group ID          */
  hid_t       AddGrpID        = FAIL;     /* "ADDITIONAL" group ID      */
  hid_t       FileAttrGrpID   = FAIL;     /* "FILE_ATTRIBUTES" group ID */
  hid_t       HDFfid          = FAIL;	  /* HDF-EOS file ID            */
  hid_t       ntype           = FAIL;     /* Data type ID               */
  
  hsize_t     count[]         = { 0 };    /* # of attribute elements    */
  
  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer       */
  
  CHECKPOINTER(attrname);


  /* Get HDF-EOS file ID and "HDFEOS" group ID */
  /* ========================================= */
  status = HE5_EHchkfid(fid,"HE5_EHreadglbattr", &HDFfid, &gid, &access);
  if (status == SUCCEED)
    {
      /* Get "ADDITIONAL"  group ID  */
      /* --------------------------- */
      AddGrpID = H5Gopen(gid, "ADDITIONAL");
      if(AddGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"ADDITIONAL\" group.\n");
	  H5Epush(__FILE__, "HE5_EHreadglbattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
        }

      /* Get "FILE_ATTRIBUTES"  group ID and call HE5_EHattr to perform I/O */
      /* ------------------------------------------------------------------ */
      FileAttrGrpID = H5Gopen(AddGrpID, "FILE_ATTRIBUTES");
      if(FileAttrGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"FILE_ATTRIBUTES\" group.\n");
	  H5Epush(__FILE__, "HE5_EHreadglbattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
        }

      status = HE5_EHattr(FileAttrGrpID,attrname,ntype,count,"r", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot read Attribute \"%s\" from the \"FILE_ATTRIBUTES\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_EHreadglbattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }

      status = H5Gclose(AddGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"ADDITIONAL\" group ID.\n");
	  H5Epush(__FILE__,"HE5_EHreadglbattr", __LINE__,  H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Gclose(FileAttrGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"FILE_ATTRIBUTES\" group ID.\n");
	  H5Epush(__FILE__, "HE5_EHreadglbattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_EHglbattrinfo                                                |
|                                                                             |
|  DESCRIPTION: Retrieves information about attributes in "FILE_ATTRIBUTES"   |
|                  group within a file.                                       |
|                                                                             |
|                                -----------                                  |
|                                | Root "/"|                                  |
|                                -----------                                  |
|                               /           \                                 |
|               ----------------------    ---------------------               |
|               | HDFEOS INFORMATION |    |       HDFEOS      |               |
|               ----------------------    ---------------------               |
|               /            |            /   / \   \        \                |
|              /             |           /   /   \   \        \               |
|    <HDFEOSVersion> [StructMetadata.0] /   /     \   \        \              |
|                                      /   |       |   \        \             |
|                            ----------  ------ ------- ------- --------      |
|                           |ADDITIONAL| |GRID| |POINT| |SWATH| |  ZA  |      |
|                            ----------  ------ ------- ------- --------      |
|                                /         :       :        :       :         |
|                               /        [dts]   [dts]    [dts]   [dts]       |
|                      -----------------                                      |
|                      |FILE_ATTRIBUTES|                                      |
|                      -----------------                                      |
|                             :                                               |
|                          <attrs>                                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               file ID                                 |
|  attrname       const char*         attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          hid_t               data type ID                            |
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
|  May 01   A.Muslimov    Original development.                               |
|  Feb 03   S.Zhao        Changed the type of 'ntype' from an H5T_class_t to  |
|                         an hid_t.                                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_EHglbattrinfo(hid_t fid, const char *attrname, hid_t *ntype, hsize_t *count)
{
  herr_t     status          = FAIL;     /* Return status variable     */

  uintn      access          =  0;	     /* Access code                */

  hid_t      gid             = FAIL;     /* "HDFEOS" group ID          */
  hid_t      AddGrpID        = FAIL;     /* "ADDITIONAL" group ID      */
  hid_t      FileAttrGrpID   = FAIL;     /* "FILE_ATTRIBUTES" group ID */
  hid_t      HDFfid          = FAIL;	 /* HDF-EOS file ID            */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/*  Error message buffer      */

  CHECKPOINTER(attrname);


  /* Get HDF-EOS file ID and "HDFEOS" group ID */
  /* ========================================= */
  status = HE5_EHchkfid(fid,"HE5_EHglbattrinfo", &HDFfid, &gid, &access);
  if (status == SUCCEED)
    {
      /* Get "ADDITIONAL"  group ID */
      /* -------------------------- */
      AddGrpID = H5Gopen(gid, "ADDITIONAL");
      if(AddGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"ADDITIONAL\" group.\n");
	  H5Epush(__FILE__, "HE5_EHglbattrinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Get "FILE_ATTRIBUTES"  group ID and call HE5_EHattrinfo */
      /* ------------------------------------------------------- */
      FileAttrGrpID = H5Gopen(AddGrpID, "FILE_ATTRIBUTES");
      if(FileAttrGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"FILE_ATTRIBUTES\" group.\n");
	  H5Epush(__FILE__,"HE5_EHglbattrinfo", __LINE__,  H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      status = HE5_EHattrinfo(FileAttrGrpID,attrname,ntype,count);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot retrieve information about Attribute \"%s\" in the \"FILE_ATTRIBUTES\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_EHglbattrinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Gclose(AddGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"ADDITIONAL\" group ID.\n");
	  H5Epush(__FILE__, "HE5_EHglbattrinfo", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Gclose(FileAttrGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"FILE_ATTRIBUTES\" group ID.\n");
	  H5Epush(__FILE__, "HE5_EHglbattrinfo", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_EHinqglbattrs                                                |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list in "FILE_ATTRIBUTES" group                    |
|                                                                             |
|                                -----------                                  |
|                                | Root "/"|                                  |
|                                -----------                                  |
|                               /           \                                 |
|               ----------------------    --------------------                |
|               | HDFEOS INFORMATION |    |      HDFEOS      |                |
|               ----------------------    --------------------                |
|               /            |            /   / \   \       \                 |
|              /             |           /   /   \   \       \                |
|    <HDFEOSVersion> [StructMetadata.0] /   /     \   \       \               |
|                                      /   |       |   \       \              |
|                            ----------  ------ ------- ------- --------      |
|                           |ADDITIONAL| |GRID| |POINT| |SWATH| |  ZA  |      |
|                            ----------  ------ ------- ------- --------      |
|                                /         :       :        :       :         |
|                               /        [dts]   [dts]    [dts]   [dts]       |
|                      -----------------                                      |
|                      |FILE_ATTRIBUTES|                                      |
|                      -----------------                                      |
|                             :                                               |
|                          <attrs>                                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in                 |
|                                        "FILE_ATTRIBUTES" group.             |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               file ID                                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in "FILE_ATTRIBUTES"    |
|                                         group (Comma-separated list)        |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 01   A.Muslimov    Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHinqglbattrs(hid_t fid, char *attrnames, long *strbufsize)
{
  long            nattr           = FAIL;     /* Number of attributes     */

  herr_t          status          = FAIL;     /* Return status variable   */

  uintn           access          =  0;	      /* Access code              */

  hid_t           gid             = FAIL;     /* "HDFEOS" group ID        */
  hid_t           HDFfid          = FAIL;	  /* HDF-EOS file ID          */
    
  char            *grpname = (char *)NULL;    /* Group name string        */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */


  /* Get HDF-EOS file ID and "HDFEOS" group ID */
  /* ========================================= */
  status = HE5_EHchkfid(fid,"HE5_EHinqglbattrs", &HDFfid, &gid, &access);
  if(status == SUCCEED )
    {
      grpname = (char *)calloc(HE5_HDFE_NAMBUFSIZE, sizeof(char) );
      if( grpname == NULL)
        {
          H5Epush(__FILE__, "HE5_EHinqgrpattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
          HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
          return(FAIL);
        }

      /* Set the Group name */
      /* ------------------ */
      strcpy(grpname,"/HDFEOS/ADDITIONAL/FILE_ATTRIBUTES");
	
      /* search group with grpname for the number of attributes */
      /* ------------------------------------------------------ */ 
      nattr = HE5_EHattrcat(fid, grpname, attrnames, strbufsize);
      if ( nattr < 0 )
	{
	  sprintf(errbuf, "Cannot find the attributes. \n");
	  H5Epush(__FILE__, "HE5_EHinqglbattrs",   __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	}

      if (grpname != NULL) free(grpname);
    }
  
  return (nattr);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHinqglbdatatype                                             |
|                                                                             |
|  DESCRIPTION: Inquires the data type of attributes in "FILE_ATTRIBUTES"     |
|               group within a file.                                          |
|                                                                             |
|                                -----------                                  |
|                                | Root "/"|                                  |
|                                -----------                                  |
|                               /           \                                 |
|               ----------------------    --------------------                |
|               | HDFEOS INFORMATION |    |      HDFEOS      |                |
|               ----------------------    --------------------                |
|               /            |            /   / \   \       \                 |
|              /             |           /   /   \   \       \                |
|    <HDFEOSVersion> [StructMetadata.0] /   /     \   \       \               |
|                                      /   |       |   \       \              |
|                            ----------  ------ ------- ------- --------      |
|                           |ADDITIONAL| |GRID| |POINT| |SWATH| |  ZA  |      |
|                            ----------  ------ ------- ------- --------      |
|                                /         :       :        :       :         |
|                               /        [dts]   [dts]    [dts]   [dts]       |
|                      -----------------                                      |
|                      |FILE_ATTRIBUTES|                                      |
|                      -----------------                                      |
|                             :                                               |
|                          <attrs>                                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               file ID                                 |
|  attrname       const char*         attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datatype       hid_t                                                       |
|  classid        H5T_class_t                                                 |
|  order          H5T_order_t                                                 |
|  size           size_t                                                      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Mar 03    S.Zhao       Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_EHinqglbdatatype(hid_t fid, const char *attrname, hid_t *datatype, H5T_class_t *classid, H5T_order_t *order, size_t *size)
{
  herr_t     status          = FAIL;     /* Return status variable     */
 
  uintn      access          =  0;       /* Access code                */
 
  hid_t      gid             = FAIL;     /* "HDFEOS" group ID          */
  hid_t      AddGrpID        = FAIL;     /* "ADDITIONAL" group ID      */
  hid_t      FileAttrGrpID   = FAIL;     /* "FILE_ATTRIBUTES" group ID */
  hid_t      HDFfid          = FAIL;     /* HDF-EOS file ID            */
  hid_t      attr            = FAIL;     /* attribute dataset ID       */
 
  char       errbuf[HE5_HDFE_ERRBUFSIZE];/*  Error message buffer      */
 
  CHECKPOINTER(attrname);
 
 
  /* Get HDF-EOS file ID and "HDFEOS" group ID */
  /* ========================================= */
  status = HE5_EHchkfid(fid,"HE5_EHinqglbdatatype", &HDFfid, &gid, &access);
  if (status == SUCCEED)
    {
      /* Get "ADDITIONAL"  group ID */
      /* -------------------------- */
      AddGrpID = H5Gopen(gid, "ADDITIONAL");
      if(AddGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"ADDITIONAL\" group.\n");
	  H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
 
      /* Get "FILE_ATTRIBUTES"  group ID */
      /* ------------------------------- */
      FileAttrGrpID = H5Gopen(AddGrpID, "FILE_ATTRIBUTES");
      if(FileAttrGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"FILE_ATTRIBUTES\" group.\n");
	  H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
 
      /* Get data type information about global attribute */
      /* ------------------------------------------------ */
      attr = H5Aopen_name(FileAttrGrpID,attrname);
      if (attr == FAIL)
	{
	  sprintf(errbuf, "Cannot open the \"%s\" attribute.\n", attrname);
	  H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      *datatype = H5Aget_type(attr);
      if (*datatype == FAIL)
	{
	  sprintf(errbuf, "Cannot get the data type for the \"%s\" attribute.\n", attrname);
	  H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      *classid = H5Tget_class(*datatype);
      if (*classid == H5T_NO_CLASS)
	{
	  sprintf(errbuf, "Cannot get the data type class ID for the \"%s\" attribute.\n", attrname);
	  H5Epush(__FILE__,"HE5_EHinqglbdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      *order = H5Tget_order(*datatype);
      if (*order == H5T_ORDER_ERROR)
	{
	  sprintf(errbuf, "Cannot get the data type order for the \"%s\" attribute.\n", attrname);
	  H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	} 

      if (*classid == H5T_STRING)
	{
          *size = H5Tget_size(*datatype);
          if (*size == 0)
	    {
	      sprintf(errbuf, "Cannot get the data type size for the \"%s\" string attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}
      else
	{
          *size = H5Tget_size(*datatype);
          if (*size == 0)
	    {
	      sprintf(errbuf, "Cannot get the data type size for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}

      status = H5Aclose(attr);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the dataset ID for the \"%s\" attribute.\n", attrname);
	  H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR , errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	} 

      status = H5Gclose(AddGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"ADDITIONAL\" group ID.\n");
	  H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
 
      status = H5Gclose(FileAttrGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"FILE_ATTRIBUTES\" group ID.\n");
	  H5Epush(__FILE__, "HE5_EHinqglbdatatype", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_EHupdatemeta                                                 |
|                                                                             |
|  DESCRIPTION: Updates metadata                                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|    status       herr_t  None        return status variable (0 for success,  |
|                                          -1 for failure)                    |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|    fid          hid_t   None        HDF-EOS file ID                         |
|    structname   char*   None        Object name string                      |
|    structcode   char*   None        "s" for a swath, "g" for a grid,        |
|                                     "p" for a point, and "z" for a za.      |
|    metacode     long    None        Code of a metadata block to insert to.  |
|    metastr      char*   None        Buffer containing metadata information  |
|                                      to insert.                             |
|    metadata[]   hsize_t None        Array of data values                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|    None                                                                     |
|                                                                             |
|  NOTES:   Allocates memory of fixed size (= HE5_STATIC_BLKSIZE)             |
|           for metabuf[] array (old scheme), or allocates buffer in chuncks  |
|           of HE5_DYNAMIC_BLKSIZE (new scheme).                              |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  05/17/04  Abe Taaheri  Initial version                                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_EHupdatemeta(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[])
{
  herr_t   status = SUCCEED;
  int      HDFEOS5ver_flg ; /* will be set to 1 if the HDF-EOS5 version 
			       is later than 5.1.7, otherwise to 0 */
  /* Get HDFEOS5ver_flg flag for the file */
  /* --------------- */
  HDFEOS5ver_flg = HE5_HeosTable[fid%HE5_EHIDOFFSET].DYNAMIC_flag;

  if(HDFEOS5ver_flg == 1)
    {
      status = HE5_EHupdatemeta_Static(fid, structname, structcode, metacode,
				       metastr, metadata);
    }
  else if(HDFEOS5ver_flg == 0)
    {
      status = HE5_EHupdatemeta_Dynamic(fid, structname, structcode, metacode,
					metastr, metadata);
    }

  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHupdatemeta_Static                                          |
|                                                                             |
|  DESCRIPTION: Updates metadata                                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|    status       herr_t  None        return status variable (0 for success,  |
|                                          -1 for failure)                    |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|    fid          hid_t   None        HDF-EOS file ID                         |
|    structname   char*   None        Object name string                      |
|    structcode   char*   None        "s" for a swath, "g" for a grid,        |
|                                     "p" for a point, and "z" for a za.      |
|    metacode     long    None        Code of a metadata block to insert to.  |
|    metastr      char*   None        Buffer containing metadata information  |
|                                      to insert.                             |
|    metadata[]   hsize_t None        Array of data values                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|    None                                                                     |
|                                                                             |
|  NOTES:   There is memory allocation of BLKSIZE chunk for the               |
|           dataset "StructMetadata.0"                                        |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Oct. 02  S. ZHAO       Original development                                |
|  Dec. 02  H. LO         Update dimension size                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_EHupdatemeta_Static(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[])
{
  herr_t         status   = FAIL;    /* routine return status variable            */
 
  int            i;                  /* Loop index                                */
 
  hid_t          gid      = FAIL;    /* "HDFEOS INFORMATION" group ID             */
  hid_t          meta_id  = FAIL;    /* "StructuralMetadat.0" group ID            */
  hid_t          atype    = FAIL;    /* Data type ID                              */
  hid_t          aspace   = FAIL;    /* Data space ID                             */
  hid_t          HDFfid   = FAIL;    /* HDF5 file ID                              */
 
  hsize_t        tempdim  =  0;      /* Temporary variable                        */
 
  long           count = 0;          /* Objects/Groups counter                    */
 
  char           *metabuf = NULL;    /* Pointer to Structural Metadata array      */
  char           *metaptr = NULL;    /* Metadata pointer                          */
  char           *prevmetaptr = NULL;/* Previous position of metadata pointer     */
  char           *utlstr  = NULL;    /* Utility string                            */
  char           *utlstr2 = NULL;    /* Utility string 2                          */
  char           *newbuf  = NULL;    /* updated metadata string                   */
  char           *type    = NULL;    /* data type string                          */
  char           *errbuf  = NULL;    /* Error message buffer                      */
 
  char           *meta_dim_name = NULL;
  char           *utl_dim_name  = NULL;
  int            ii, jj, i1, j1;
 
  CHECKPOINTER(structname);
  CHECKPOINTER(structcode);
  CHECKPOINTER(metastr);
 
 
 
  /* Get HDF file ID */
  /* --------------- */
  HDFfid = HE5_HeosTable[fid%HE5_EHIDOFFSET].HDFfid;
 
  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Allocate memory for data type string */
  /* ------------------------------------ */
  type = (char * )calloc( HE5_HDFE_TYPESTRSIZE, sizeof(char));
  if(type == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for data type string. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
 
      return(FAIL);
    }
 
 
  /* Open "HDFEOS INFORMATION" group */
  /* =============================== */
  gid     = H5Gopen(HDFfid, "HDFEOS INFORMATION");
  if(gid == FAIL)
    {
      status = FAIL;
      sprintf(errbuf,"Cannot open \"HDFEOS INFORMATION\" group. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
 
      return(status);
    }
 
 
  /*
   *--------------------------------------------------------------*
   * Open dataset "StructMetadata.0", get datatype and space ID   *
   *--------------------------------------------------------------*
   */
  meta_id = H5Dopen(gid, "StructMetadata.0");
  if( meta_id == FAIL)
    {
      status = FAIL;
      sprintf(errbuf,"Cannot open \"StructMetadata.0\" dataset. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
 
      return(status);
    }
  atype   = H5Dget_type(meta_id);
  if( atype == FAIL)
    {
      status = FAIL;
      sprintf(errbuf,"Cannot get the dataset datatype. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
 
      return(status);
    }
  aspace  = H5Dget_space(meta_id);
  if( aspace == FAIL )
    {
      status = FAIL;
      sprintf(errbuf,"Cannot get the dataset dataspace. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
 
      return(status);
    }
 
 
  /*
   *-------------------------------------------------------------*
   *   Allocate memory for the output array and Read attribute   *
   *-------------------------------------------------------------*
   */
 
  metabuf  = (char *)calloc( HE5_HeosTable[fid%HE5_EHIDOFFSET].HE5_STATIC_BLKSIZE, sizeof(char));
  if( metabuf == NULL )
    {
      sprintf(errbuf,"Cannot allocate memory for meta buffer. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
      free(errbuf);
      free(type);
 
      return(status);
    }
 
  utlstr   = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if( utlstr == NULL )
    {
      sprintf(errbuf,"Cannot allocate memory for utility string buffer. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
      free(errbuf);
      free(type);
      free(metabuf);
 
      return(status);
 
    }
 
  utlstr2  = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if( utlstr2 == NULL )
    {
      sprintf(errbuf,"Cannot allocate memory for the 2d utility string buffer. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
      free(errbuf);
      free(type);
      free(metabuf);
      free(utlstr);
 
      return(status);
 
    }
 
  status = H5Dread(meta_id, atype,H5S_ALL, H5S_ALL, H5P_DEFAULT,metabuf);
  if( status == FAIL)
    {
      sprintf(errbuf, "Cannot read structural metadata. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
      free(metabuf);
      free(utlstr);
      free(utlstr2);
 
      return(status);
    }
 
 
 
  /* Find HDF-EOS structure "root" group in metadata */
  /* ----------------------------------------------- */
  /* Setup proper search string */
  if (strcmp(structcode, "s") == 0)
    {
      strcpy(utlstr, "GROUP=SwathStructure");
    }
  else if (strcmp(structcode, "g") == 0)
    {
      strcpy(utlstr, "GROUP=GridStructure");
    }
  else if (strcmp(structcode, "p") == 0)
    {
      strcpy(utlstr, "GROUP=PointStructure");
    }
  else if (strcmp(structcode, "z") == 0)
    {
      strcpy(utlstr, "GROUP=ZaStructure");
    }

 
  /* Use string search routine (strstr) to move through metadata */
  /* ----------------------------------------------------------- */
  metaptr = strstr(metabuf, utlstr);
 
  /* Find specific (named) structure */
  /* ------------------------------- */
  if (metacode < 1000)
    {
      /* Save current metadata pointer */
      /* ----------------------------- */
      prevmetaptr = metaptr;
 
      /* First loop for "old-style" (non-ODL) metadata string */
      /* ---------------------------------------------------- */
      if (strcmp(structcode, "s") == 0)
	{
	  sprintf(utlstr, "%s%s%s", "SwathName=\"", structname, "\"");
	}
      else if (strcmp(structcode, "g") == 0)
	{
	  sprintf(utlstr, "%s%s%s", "GridName=\"", structname, "\"");
	}
      else if (strcmp(structcode, "p") == 0)
	{
	  sprintf(utlstr, "%s%s%s", "PointName=\"", structname, "\"");
	}
      else if (strcmp(structcode, "z") == 0)
	{
	  sprintf(utlstr, "%s%s%s", "ZaName=\"", structname, "\"");
	}


      /* Perform string search */
      /* --------------------- */
 
      metaptr = strstr(metaptr, utlstr);
 
      /*
       *--------------------------------------------------------------------*
       * If not found then return to previous position in metadata and look *
       *             for "new-style" (ODL) metadata string                  *
       *--------------------------------------------------------------------*
       */
      if (metaptr == NULL)
	{
	  sprintf(utlstr, "%s%s%s", "GROUP=\"", structname, "\"");
	  metaptr = strstr(prevmetaptr, utlstr);
	}
    }
 
 
  /* Metadata Section Switch */
  /* ----------------------- */
  switch ((int)abs((int)metacode))
    {
 
    case 0:
 
      /* Dimension Section */
      /* ================= */

      /* Find beginning and end of metadata section */
      /* ------------------------------------------ */
      strcpy(utlstr, "\t\tGROUP=Dimension");
      /* begptr = strstr(metaptr, utlstr);*/

      strcpy(utlstr, "\t\tEND_GROUP=Dimension");
      metaptr = strstr(metaptr, utlstr);
 
      count = 1;
 
 
      /* Build metadata entry string */
      /* --------------------------- */
      sprintf(utlstr, "%s%li%s%s%s%li%s%li%s", "\t\t\tOBJECT=Dimension_",count,"\n\t\t\t\tDimensionName=\"",&metastr[0],"\"\n\t\t\t\tSize=", (long)metadata[0],"\n\t\t\tEND_OBJECT=Dimension_", count, "\n");
 
      break;
 
 
    default:
      {
	sprintf(errbuf, "Unknown metacode.\n");
	H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	HE5_EHprint(errbuf, __FILE__, __LINE__);
      }
      break;
 
    }
 
 
  /* Reallocate memory for a new metadata array, newbuf */
  /* ------------------------------------------------------ */
  newbuf = (char *) calloc(HE5_HeosTable[fid%HE5_EHIDOFFSET].HE5_STATIC_BLKSIZE, sizeof(char));
  if(newbuf==NULL)
    { 
      sprintf(errbuf, "Cannot allocate memory for a new metadata string.\n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
      free(errbuf);
      free(type);
      free(metabuf);
      free(utlstr);
      free(utlstr2);
 
      return(status);
    }
 
  /* Copy new metadata string to newbuf array */
  /* ---------------------------------------- */
  strncpy(newbuf, prevmetaptr, strlen(prevmetaptr));
  newbuf[strlen(prevmetaptr)] = '\0';
 
  meta_dim_name = strstr(newbuf, metastr);
  utl_dim_name = strstr(utlstr, metastr);
 
  ii = strlen(newbuf) - strlen(meta_dim_name);
  jj = strlen(utlstr) - strlen(utl_dim_name);
 
 
  for(i = 1; i < jj+1; i++)
    {
      i1 = ii+i;
      j1 = jj+i;
      newbuf[i1] = utlstr[j1];
    }
 
  tempdim = (hsize_t)strlen( newbuf );
 
  tempdim++;
 
  /*
   *-----------------------------------------------------------------*
   *     Write updated metadata back  to dataset "StructMetadata.0"  *
   *-----------------------------------------------------------------*
   */
 
  status  = H5Dwrite(meta_id, atype, H5S_ALL, H5S_ALL, H5P_DEFAULT, newbuf);
  if( status == FAIL)
    {
      sprintf(errbuf,"Cannot write in structural metadata.\n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
      free(metabuf);
      free(utlstr);
      free(utlstr2);
      free(newbuf);
 
      return(status);
    }
 
  free(metabuf);
  free(newbuf);
  free(utlstr);
  free(utlstr2);
  free(type);
 
  metabuf = NULL;
  newbuf  = NULL;
  utlstr2 = NULL;
  utlstr  = NULL;
  type    = NULL;
 
  status = H5Sclose(aspace);
  if( status == FAIL)
    {
      sprintf(errbuf,"Cannot release the data space ID.\n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
 
      return(status);
    }
 
  status = H5Tclose(atype);
  if( status == FAIL)
    {
      sprintf(errbuf, "Cannot release the data type ID.\n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
 
      return(status);
    }
  status = H5Dclose(meta_id);
  if( status == FAIL)
    {
      sprintf(errbuf,"Cannot release the dataset ID.\n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
 
      return(status);
    }
  status = H5Gclose(gid);
  if( status == FAIL)
    {
      sprintf(errbuf,"Cannot release the group ID.\n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Static", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
 
      return(status);
    }
 
  free(errbuf);
  errbuf = NULL;
 
 COMPLETION:
  return status;
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHupdatemeta_Dynamic                                         |
|                                                                             |
|  DESCRIPTION: Updates metadata                                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|                                                                             |
|    status       herr_t  None        return status variable (0 for success,  |
|                                          -1 for failure)                    |
|                                                                             |
|  INPUTS:                                                                    |
|                                                                             |
|    fid          hid_t   None        HDF-EOS file ID                         |
|    structname   char*   None        Object name string                      |
|    structcode   char*   None        "s" for a swath, "g" for a grid,        |
|                                     "p" for a point, and "z" for a za.      |
|    metacode     long    None        Code of a metadata block to insert to.  |
|    metastr      char*   None        Buffer containing metadata information  |
|                                      to insert.                             |
|    metadata[]   hsize_t None        Array of data values                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|    None                                                                     |
|                                                                             |
|  NOTES:   Allocates buffer in chuncks of HE5_DYNAMIC_BLKSIZE (new scheme).  |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  05/17/04 Abe Taaheri   Modified the version with static buffer allocation  |
|                         to adopt dynamic buffer allocation in chuncks of    |
|                         HE5_DYNAMIC_BLKSIZE                                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_EHupdatemeta_Dynamic(hid_t fid, const char *structname, char *structcode, long metacode, char *metastr, hsize_t metadata[])
{
  herr_t         status   = FAIL;    /* routine return status variable            */
 
  int            i;                  /* Loop index                                */
 
  hid_t          gid      = FAIL;    /* "HDFEOS INFORMATION" group ID             */
  hid_t          meta_id[MetaBlocks]  = {FAIL};    /* "StructuralMetadat.0" group ID            */
  hid_t          atype[MetaBlocks]    = {FAIL};    /* Data type ID                              */
  hid_t          aspace[MetaBlocks]   = {FAIL};    /* Data space ID                             */
  hid_t          HDFfid   = FAIL;    /* HDF5 file ID                              */
 
  hsize_t        tempdim  =  0;      /* Temporary variable                        */
 
  long           count = 0;          /* Objects/Groups counter                    */
 
  char           *metabuf = NULL;    /* Pointer to Structural Metadata array      */
  char           *metaptr = NULL;    /* Metadata pointer                          */
  char           *prevmetaptr = NULL;/* Previous position of metadata pointer     */
  char           *utlstr  = NULL;    /* Utility string                            */
  char           *utlstr2 = NULL;    /* Utility string 2                          */
  char           *newbuf  = NULL;    /* updated metadata string                   */
  char           *type    = NULL;    /* data type string                          */
  char           *errbuf  = NULL;    /* Error message buffer                      */
 
  char           *meta_dim_name = NULL;
  char           *utl_dim_name  = NULL;
  int            ii, jj, i1, j1;
  long           nmeta;
  /*long           *metstrbufsize = 0;*/
  char           metutlstr[32];
  int            ism;
  long           metalen;

 
  CHECKPOINTER(structname);
  CHECKPOINTER(structcode);
  CHECKPOINTER(metastr);
 
 
 
  /* Get HDF file ID */
  /* --------------- */
  HDFfid = HE5_HeosTable[fid%HE5_EHIDOFFSET].HDFfid;
 
  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Allocate memory for data type string */
  /* ------------------------------------ */
  type = (char * )calloc( HE5_HDFE_TYPESTRSIZE, sizeof(char));
  if(type == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for data type string. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
 
      return(FAIL);
    }
 
  /* Open "HDFEOS INFORMATION" group */
  /* =============================== */
  gid     = H5Gopen(HDFfid, "HDFEOS INFORMATION");
  if(gid == FAIL)
    {
      status = FAIL;
      sprintf(errbuf,"Cannot open \"HDFEOS INFORMATION\" group. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(type);
 
      return(status);
    }

  
  /* Determine number of structural metadata "sections" */
  /* -------------------------------------------------- */
  nmeta = 0;
  status= HE5_EHgetnmeta(gid, &nmeta);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot find any StructMetadata.X dataset. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      
      return(FAIL);
    }
  
  for (ism = 0; ism < nmeta; ism++)
    {
      
      /*
       *--------------------------------------------------------------*
       * Open dataset "StructMetadata.X", get datatype and space ID   *
       *--------------------------------------------------------------*
       */
      sprintf(metutlstr, "%s%d", "StructMetadata.", ism);
      meta_id[ism] = H5Dopen(gid, metutlstr); 
      if( meta_id[ism] == FAIL)
        {
	  status = FAIL;
	  sprintf(errbuf,"Cannot open %s dataset. \n", metutlstr);
	  H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(type);
	  
	  return(status);
        }
      atype[ism]   = H5Dget_type(meta_id[ism]);
      if( atype[ism] == FAIL)
        {
	  status = FAIL;
	  sprintf(errbuf,"Cannot get the dataset datatype. \n");
	  H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(type);
	  
	  return(status);
        }
      aspace[ism]  = H5Dget_space(meta_id[ism]);
      if( aspace[ism] == FAIL )
        {
	  status = FAIL;
	  sprintf(errbuf,"Cannot get the dataset dataspace. \n");
	  H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(type);
	  
	  return(status);
        }
    }
 
  /*
   *-------------------------------------------------------------*
   *   Allocate memory for the output array and Read attribute   *
   *-------------------------------------------------------------*
   */
 
  metabuf  = (char *)calloc( HE5_DYNAMIC_BLKSIZE * nmeta, sizeof(char));
  if( metabuf == NULL )
    {
      sprintf(errbuf,"Cannot allocate memory for meta buffer. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
      free(errbuf);
      free(type);
 
      return(status);
    }
 
  utlstr   = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if( utlstr == NULL )
    {
      sprintf(errbuf,"Cannot allocate memory for utility string buffer. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
      free(errbuf);
      free(type);
      free(metabuf);
 
      return(status);
 
    }
 
  utlstr2  = (char *)calloc( HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if( utlstr2 == NULL )
    {
      sprintf(errbuf,"Cannot allocate memory for the 2d utility string buffer. \n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
      free(errbuf);
      free(type);
      free(metabuf);
      free(utlstr);
 
      return(status);
 
    }

  metalen = 0;
  for (ism = 0; ism < nmeta; ism++)
    {
      status = H5Dread(meta_id[ism], atype[ism],H5S_ALL, H5S_ALL, H5P_DEFAULT,metabuf + metalen);
      if( status == FAIL)
	{
          sprintf(errbuf, "Cannot read structural metadata. \n");
          H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          free(errbuf);
          free(type);
          free(metabuf);
          free(utlstr);
          free(utlstr2);
 
          return(status);
	}
    
      /* Determine length (# of characters) of metadata */
      /* ---------------------------------------------- */
      metalen = strlen(metabuf);
 
    }
 
 
  /* Find HDF-EOS structure "root" group in metadata */
  /* ----------------------------------------------- */
  /* Setup proper search string */
  if (strcmp(structcode, "s") == 0)
    {
      strcpy(utlstr, "GROUP=SwathStructure");
    }
  else if (strcmp(structcode, "g") == 0)
    {
      strcpy(utlstr, "GROUP=GridStructure");
    }
  else if (strcmp(structcode, "p") == 0)
    {
      strcpy(utlstr, "GROUP=PointStructure");
    }
  else if (strcmp(structcode, "z") == 0)
    {
      strcpy(utlstr, "GROUP=ZaStructure");
    }

 
  /* Use string search routine (strstr) to move through metadata */
  /* ----------------------------------------------------------- */
  metaptr = strstr(metabuf, utlstr);
 
  /* Find specific (named) structure */
  /* ------------------------------- */
  if (metacode < 1000)
    {
      /* Save current metadata pointer */
      /* ----------------------------- */
      prevmetaptr = metaptr;
 
      /* First loop for "old-style" (non-ODL) metadata string */
      /* ---------------------------------------------------- */
      if (strcmp(structcode, "s") == 0)
	{
	  sprintf(utlstr, "%s%s%s", "SwathName=\"", structname, "\"");
	}
      else if (strcmp(structcode, "g") == 0)
	{
	  sprintf(utlstr, "%s%s%s", "GridName=\"", structname, "\"");
	}
      else if (strcmp(structcode, "p") == 0)
	{
	  sprintf(utlstr, "%s%s%s", "PointName=\"", structname, "\"");
	}
      else if (strcmp(structcode, "z") == 0)
	{
	  sprintf(utlstr, "%s%s%s", "ZaName=\"", structname, "\"");
	}


      /* Perform string search */
      /* --------------------- */
 
      metaptr = strstr(metaptr, utlstr);
 
      /*
       *--------------------------------------------------------------------*
       * If not found then return to previous position in metadata and look *
       *             for "new-style" (ODL) metadata string                  *
       *--------------------------------------------------------------------*
       */
      if (metaptr == NULL)
	{
	  sprintf(utlstr, "%s%s%s", "GROUP=\"", structname, "\"");
	  metaptr = strstr(prevmetaptr, utlstr);
	}
    }
 
 
  /* Metadata Section Switch */
  /* ----------------------- */
  switch ((int)abs((int)metacode))
    {
 
    case 0:
 
      /* Dimension Section */
      /* ================= */

      /* Find beginning and end of metadata section */
      /* ------------------------------------------ */
      strcpy(utlstr, "\t\tGROUP=Dimension");
      /* begptr = strstr(metaptr, utlstr);*/

      strcpy(utlstr, "\t\tEND_GROUP=Dimension");
      metaptr = strstr(metaptr, utlstr);
 
      count = 1;
 
 
      /* Build metadata entry string */
      /* --------------------------- */
      sprintf(utlstr, "%s%li%s%s%s%li%s%li%s", "\t\t\tOBJECT=Dimension_",count,"\n\t\t\t\tDimensionName=\"",&metastr[0],"\"\n\t\t\t\tSize=", (long)metadata[0],"\n\t\t\tEND_OBJECT=Dimension_", count, "\n");
 
      break;
 
 
    default:
      {
	sprintf(errbuf, "Unknown metacode.\n");
	H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	HE5_EHprint(errbuf, __FILE__, __LINE__);
      }
      break;
 
    }
 
  /* The size of the whole structure metadat will not change 
     during the the update (????),
     so no need for extending newbuf by one more block size */

  /* Reallocate memory for a new metadata array, newbuf */
  /* ------------------------------------------------------ */
  newbuf = (char *) calloc(HE5_DYNAMIC_BLKSIZE * (nmeta), sizeof(char));
  if(newbuf==NULL)
    { 
      sprintf(errbuf, "Cannot allocate memory for a new metadata string.\n");
      H5Epush(__FILE__,"HE5_EHupdatemeta_Dynamic", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
      free(errbuf);
      free(type);
      free(metabuf);
      free(utlstr);
      free(utlstr2);
 
      return(status);
    }
 
  /* Copy new metadata string to newbuf array */
  /* ---------------------------------------- */
  strncpy(newbuf, prevmetaptr, strlen(prevmetaptr));
  newbuf[strlen(prevmetaptr)] = '\0';
 
  meta_dim_name = strstr(newbuf, metastr);
  utl_dim_name = strstr(utlstr, metastr);
 
  ii = strlen(newbuf) - strlen(meta_dim_name);
  jj = strlen(utlstr) - strlen(utl_dim_name);
 
 
  for(i = 1; i < jj+1; i++)
    {
      i1 = ii+i;
      j1 = jj+i;
      newbuf[i1] = utlstr[j1];
    }
 
  tempdim = (hsize_t)strlen( newbuf );
 
  tempdim++;
 
  /*
   *-----------------------------------------------------------------*
   *     Write updated metadata back  to dataset "StructMetadata.X"  *
   *-----------------------------------------------------------------*
   */

  for (ism = 0; ism < nmeta; ism++)
    {
      status  = H5Dwrite(meta_id[ism], atype[ism], H5S_ALL, H5S_ALL, H5P_DEFAULT, newbuf + ism * HE5_DYNAMIC_BLKSIZE);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot write in structural metadata.\n");
	  H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(type);
	  free(metabuf);
	  free(utlstr);
	  free(utlstr2);
	  free(newbuf);
	   
	  return(status);
	}
    }
  free(metabuf);
  free(newbuf);
  free(utlstr);
  free(utlstr2);
  free(type);
   
  metabuf = NULL;
  newbuf  = NULL;
  utlstr2 = NULL;
  utlstr  = NULL;
  type    = NULL;
   
  for (ism = 0; ism < nmeta; ism++)
    {
      status = H5Sclose(aspace[ism]);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	   
	  return(status);
	}
       
      status = H5Tclose(atype[ism]);
      if( status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data type ID.\n");
	  H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	   
	  return(status);
	}
      status = H5Dclose(meta_id[ism]);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the dataset ID.\n");
	  H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	   
	  return(status);
	}
    }
  status = H5Gclose(gid);
  if( status == FAIL)
    {
      sprintf(errbuf,"Cannot release the group ID.\n");
      H5Epush(__FILE__, "HE5_EHupdatemeta_Dynamic", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
       
      return(status);
    }
   
  free(errbuf);
  errbuf = NULL;
   
 COMPLETION:
  return status;
}




/* 
******************************************************************************
|                                                                            |
|          F  O  R  T  R  A  N  7 7      W  R  A  P  P  E  R  S              |
|                                                                            |
******************************************************************************
*/


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHidinfoF  (FORTRAN wrapper)                                 |
|                                                                             |
|  DESCRIPTION: Gets HDF ID and group ID from HDF-EOS ID                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int     None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 HDF-EOS file ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fileID         int                 HDF File ID                             |
|  gid            int                 group ID                                |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Apr 01   A. Muslimov   Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_EHidinfoF(int FileID, int *fileID, int *gid)
{
  int          ret    = FAIL;/* routine return status variable */

  hid_t        fid    = FAIL;/* Input hdf5 type file ID        */ 
  hid_t        HDFfid = FAIL;/* Output hdf5 file ID            */ 
  hid_t        Gid    = FAIL;/* HDF-EOS group ID               */

  herr_t       status = FAIL; /* Return status                 */

  char         *errbuf;		  /* Error message buffer          */


  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char *) calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHidinfoF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  fid = (hid_t)FileID;

  /* Call HE5_EHidinfo to get HDF-EOS interface IDs */
  /* ---------------------------------------------- */
  status = HE5_EHidinfo(fid,&HDFfid,&Gid);
  if(status == FAIL)
    {
      sprintf(errbuf, "Error calling \"HE5_EHidinfo\" from FORTAN wrapper.\n");
      H5Epush(__FILE__, "HE5_EHidinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  *fileID = (int)HDFfid;
  *gid    = (int)Gid;

  free(errbuf);

  ret=(int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHconvAngF   (FORTRAN wrapper)                               |
|                                                                             |
|  DESCRIPTION: Angle conversion Utility                                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  outAngle       double              Output Angle value                      |
|                                                                             |
|  INPUTS:                                                                    |
|  inAngle        double              Input Angle value                       |
|  code           int                 Conversion code                         |
|                                       HDFE_RAD_DEG (0)                      |
|                                       HDFE_DEG_RAD (1)                      |
|                                       HDFE_DMS_DEG (2)                      |
|                                       HDFE_DEG_DMS (3)                      |
|                                       HDFE_RAD_DMS (4)                      |
|                                       HDFE_DMS_RAD (5)                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|     None                                                                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Apr 01   A.Muslimov    Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
double 
HE5_EHconvAngF(double inAngle, int code)
{
  double    outAngle = 0.;          /* Angle in desired units   */
  
  char      *errbuf  = (char *)NULL;/* error message buffer     */

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE,  sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHconvAngF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer. \n");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
    }

  /* Call HE5_EHconvAngF */
  /* ------------------- */
  outAngle = HE5_EHconvAng(inAngle,code);

  if(errbuf != NULL) free(errbuf);
  return(outAngle);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHwrglatt                                                    |
|                                                                             |
|  DESCRIPTION: FORTRAN wrapper to (Over)Write global attribute in a file.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 file ID                                 |
|  attrname       char*               Global attribute name                   |
|  numtype        int                 HDFEOS datatype Flag                    |
|  fortcount[]    long                Number of global attribute elements     |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                   It is assumed that fortcout has just one element to agree |
|                   with EHattr() call.                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 01   A.Muslimov    Original Programmer                                 |
|  Nov 02   S.Zhao        Updated for writing character string attributes.    |
|  Mar 04   S.Zhao        Modified for a character string attribute.          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_EHwrglatt(int FileID, char *attrname, int numtype, long fortcount[], void *datbuf)
{
  int          ret    = FAIL;            /* (int) Return status variable      */
  int		   rank   = 1;               /* Rank variable                     */
  int		   i;                        /* Loop index                        */

  herr_t	   status     = FAIL;        /* routine return status variable    */
  
  hid_t        fileID     = FAIL;        /* HDF5 type File ID                 */
  hid_t        numbertype = FAIL;        /* HDF5 data type ID                 */
             
  hsize_t      *count  = (hsize_t *)NULL;/* Pointer to count array (C order)  */
  
  char         *errbuf = (char *)NULL;   /* error message buffer              */
  char         *tempbuf = (char *)NULL;  /* temp buffer                       */
  
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHwrglatt", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get HDF5 data type ID */
  /* --------------------- */
  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 type data type ID.\n");
      H5Epush(__FILE__, "HE5_EHwrglatt", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }


  count = (hsize_t *)calloc(rank, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_EHwrglatt", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Reverse order of dimensions (FORTRAN -> C) */
  /* ------------------------------------------ */
  for (i = 0; i < rank; i++)
    count[i] = (hsize_t)fortcount[rank - 1 - i];
  
  
  fileID = (hid_t)FileID;

  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR) || (numbertype == H5T_C_S1))
    {
      if (strlen(datbuf) < count[0])
	{
	  sprintf(errbuf,"Size of databuf is less than the number of global attribute elements.\n");
	  H5Epush(__FILE__, "HE5_EHwrglatt", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
	  H5Epush(__FILE__, "HE5_EHwrglatt", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}

      strncpy(tempbuf, datbuf, count[0]);
      tempbuf[count[0]] = '\0';
 
      status = HE5_EHwriteglbattr(fileID, attrname, numbertype, count, tempbuf);
      if(status == FAIL)
	{
          sprintf(errbuf,"Cannot write data to the global attribute.\n");
          H5Epush(__FILE__, "HE5_EHwrglatt", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
      status = HE5_EHwriteglbattr(fileID, attrname, numbertype, count, datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write data to the global attribute.\n");
	  H5Epush(__FILE__, "HE5_EHwrglatt", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
|  FUNCTION: HE5_EHrdglatt     (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION: Reads attribute from the "FILE_ATTRIBUTES" group in a file.   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 file ID                                 |
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
|  May 01   A.Muslimov    Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_EHrdglatt(int FileID, char *attrname, void *datbuf)
{
  int         ret             = FAIL;     /* (int) return status    */

  herr_t      status          = FAIL;     /* return status variable */
  
  hid_t       fileID          = FAIL;     /* HDF5 type File ID      */
  
  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
  
  fileID = (hid_t)FileID;

  status = HE5_EHreadglbattr(fileID,attrname,datbuf);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot read Attribute \"%s\" from the \"FILE_ATTRIBUTES\" group.\n", attrname);
      H5Epush(__FILE__, "HE5_EHrdglatt", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret = (int)status;
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHglattinfo                                                   |
|                                                                             |
|  DESCRIPTION:  FORTRAN wrapper for HE5_EHglbattrinfo()                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 File ID                                 |
|  attrname       char*               Attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  numbertype     int                 Attribute HDFEOS data type Flag         |
|  fortcount      long                Number of attribute elements            |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 01   A.Muslimov    Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
|                                                                             |
-----------------------------------------------------------------------------*/
int
HE5_EHglattinf(int FileID, char *attrname, int *numbertype, long *fortcount)
{
  int	   	  ret     = FAIL;	        /* (int) Return status variable   */

  herr_t   	  status  = FAIL;           /* routine return status variable */

  hid_t       *ntype  = (hid_t *)NULL;  /* Data type ID                   */

  hid_t       fileID  = FAIL;           /* HDF5 type File ID              */

  hsize_t     *count  = (hsize_t *)NULL;/* Pointer to count  (C order)    */
  
  char        *errbuf = (char *)NULL;   /* error message buffer           */
  
  /* Allocate memory for error message buffers */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHglattinf", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *)calloc(1, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_EHglattinf", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  ntype = (hid_t *)calloc(1, sizeof(hid_t));
  if( ntype == (hid_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for ntype.\n");
      H5Epush(__FILE__, "HE5_EHglattinf", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(count);
      return(FAIL);
    }


  fileID = (hid_t)FileID;

  status = HE5_EHglbattrinfo(fileID, attrname, ntype, count);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about global attribute from \"FILE_ATTRIBUTES\" group.\n");
      H5Epush(__FILE__, "HE5_EHglattinf", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(count);
      free(ntype);
      return(FAIL);
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
|  FUNCTION: HE5_EHinqglatts (FORTRAN wrapper)                                |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list from "FILE_ATTRIBUTES" group                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                No. of attributes in "FILE_ATTRIBUTES"  |
|                                       group.                                |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 file ID                                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in "FILE_ATTRIBUTES"    |
|                                      group (Comma-separated list)           |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  OUTPUTS:                                                                   |
|             None                                                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 01   A.Muslimov    Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_EHinqglatts(int FileID, char *attrnames, long *strbufsize)
{
  long            nattr   = FAIL;             /* Number of attributes (return) */

  hid_t           fileID  = FAIL;             /* HDF5 type file ID             */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer          */

  fileID = (hid_t)FileID;
  
  nattr = HE5_EHinqglbattrs(fileID, attrnames, strbufsize);
  if ( nattr < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the attributes. \n");
      H5Epush(__FILE__, "HE5_EHinqglatts", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  return(nattr);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHinqglbtype                                                 |
|                                                                             |
|  DESCRIPTION:  FORTRAN wrapper for HE5_EHinqglbdatatype()                   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 File ID                                 |
|  attrname       char*               Attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  Type         int                                                           |
|  Class        int                                                           |
|  Order        int                                                           |
|  size         long                                                          |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Mar 03    S.Zhao       Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
|                                                                             |
-----------------------------------------------------------------------------*/
int
HE5_EHinqglbtype(int FileID, char *attrname, int *Type, int *Class, int *Order, long *size)
{
  int              ret      = FAIL;               /* (int) status variable  */
 
  herr_t           status   = FAIL;               /* return status variable */
 
  size_t           *tsize   = (size_t *)NULL;     /* data size variable     */
 
  hid_t        *typeID  = (hid_t *)NULL;      /* HDF5 Data type ID      */
  hid_t        fileID  = FAIL;                /* HDF5 type File ID      */
 
  H5T_class_t  *classID = (H5T_class_t *)NULL;/*  Data type class ID    */
  H5T_order_t  *order   = (H5T_order_t *)NULL;/* Byte order of datatype */
 
  char         *errbuf  = (char *)NULL;       /* error message buffer   */
 
 
  /* Allocate memory for error message buffers */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHinqglbtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  typeID = (hid_t *)calloc(1, sizeof(hid_t));
  if(typeID == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for typeID.\n");
      H5Epush(__FILE__,"HE5_EHinqglbtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
 
  tsize = (size_t *)calloc(1, sizeof(size_t));
  if(tsize == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for tsize.\n");
      H5Epush(__FILE__, "HE5_EHinqglbtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(typeID);
      return(FAIL);
    }
 
  classID = (H5T_class_t *)calloc(1, sizeof(H5T_class_t));
  if(classID == (H5T_class_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for classID.\n");
      H5Epush(__FILE__, "HE5_EHinqglbtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
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
      H5Epush(__FILE__, "HE5_EHinqglbtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(typeID);
      free(tsize);
      free(classID);
      return(FAIL);
    }
 
  fileID = (hid_t)FileID;
 
  /* Get data type information */
  /* ------------------------- */
  status = HE5_EHinqglbdatatype(fileID, attrname, typeID, classID, order, tsize);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about data type.\n");
      H5Epush(__FILE__, "HE5_EHinqglbtype", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(typeID);
      free(tsize);
      free(classID);
      free(order);
      return(FAIL);
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
|  FUNCTION: HE5_EHrdwrfileF       (FORTRAN wrapper)                          |
|                                                                             |
|  DESCRIPTION: Opens and reads or writes an external binary file which is in |
|               non-Fortran format.  This is needed for HDF-EOS5 routines     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  extfilename    char                External file name                      |
|  Flags          uintn               Access Code                             |
|  ntype          int                 HDF numbertype of field datatype        |
|  nelmnts        long                Number of datatype elements in data     |
|                                     buffer.                                 |
|  data           char*               Data buffer                             |
|                                                                             |
|                                                                             |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jun 01   C.Praderas    Original development.                               |
|  Dec 05   T.Roegner     NCR 44092 - Added CYGWIN capability                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_EHrdwrfileF(char *extfilename, uintn Flags, int ntype, long nelmnts, void *data)
{
  herr_t            status    = SUCCEED;/* routine return status variable */

  int               ret       = SUCCEED;/* (int) return status variable   */

  long              datatype_size = 0;   /* Size of datatype */   
  long              numread       = 0;
  long              numwrite      = 0;

  char              *access_code = (char *)NULL;
  char              *errbuf = (char *)NULL;/*Error message buffer   */
  
  FILE              *fptr;


  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_EHrdwrfileF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate memory for access code */
  /* ------------------------------- */
  access_code = (char * )calloc( 3, sizeof(char));
  if(access_code == NULL)
    {
      H5Epush(__FILE__, "HE5_EHrdwrfileF", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for access_code buffer.");
      HE5_EHprint("Error: Cannot allocate memory for access_code buffer, occured", __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Determine access_code */
  /* --------------------- */
  switch (Flags)
    {
    case 0:
      { 
	strcpy(access_code, "r+");
	access_code[2] = 0;
      }
      break;
    case 1:
      {
	strcpy(access_code, "r");
	access_code[1] = 0;
      }
      break;
    case 2:
      {
	strcpy(access_code, "w");
	access_code[1] = 0;
      }
      break;		  
    default:
      break;  
    } 	

  /* Open external file */
  /* ------------------ */
  fptr = fopen( extfilename, access_code );
  if(fptr == NULL)
    {
      sprintf(errbuf, "Cannot open external file:\"%s\".\n",extfilename) ;
      H5Epush(__FILE__, "HE5_EHrdwrfileF", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(access_code);
      return(FAIL);		 
    }

  /* Determine datatype size */
  /* ----------------------- */
  switch( ntype )
    {
    case 0:
      {	datatype_size = sizeof(int); }
      break;  
    case 1:
      {	datatype_size = sizeof(float); }
      break;  
    case 2:
      {	datatype_size = sizeof(double); }
      break;  
    case 3:
      {	datatype_size = 1; }
      break;  
    case 4:
      {	datatype_size = 2; }
      break;  
    case 5:
      {	datatype_size = sizeof(char); }
      break;  
    case 6:
      {	datatype_size = sizeof(long); }
      break;  
    default:
      break;  
    }

  /* Read or write data to external file */
  /* ----------------------------------- */
  if( Flags == 0 || Flags == 2 )
    {
      numwrite= fwrite( data, datatype_size, nelmnts, fptr );
      if(numwrite <= 0)
	{
	  sprintf(errbuf, "Cannot write to file:\"%s\".\n",extfilename) ;
	  H5Epush(__FILE__, "HE5_EHrdwrfileF", __LINE__, H5E_FILE, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(access_code);
	  return(FAIL);		 
	}
    }

  if( Flags == 1 )
    {
      numread = fread( data, datatype_size, nelmnts, fptr );
      if(numread <= 0)
	{
	  sprintf(errbuf, "Cannot read from file:\"%s\".\n",extfilename) ;
	  H5Epush(__FILE__, "HE5_EHrdwrfileF", __LINE__, H5E_FILE, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(access_code);
	  return(FAIL);		 
	}
    }

  /* Close external file */
  /* ------------------- */
  fclose(fptr);

  free(errbuf);
  free(access_code);

  ret = (int)status;
  return(ret);
}



herr_t HE5_EHgetnmeta(hid_t gid, long *nmeta)
{
  char           metutlstr[32];
  hid_t          status = -1;
  *nmeta = 0;
  while (1)
    {
      /* Search for "StructMetadata.x" attribute */
      /* --------------------------------------- */
      sprintf(metutlstr, "%s%ld", "StructMetadata.", *nmeta);
      H5Eset_auto(NULL,NULL);
      status =  H5Dopen(gid, metutlstr); 


      /* If found then increment metadata section counter else exit loop */
      /* --------------------------------------------------------------- */
      if (status != -1)
	{
	  *nmeta = *nmeta + 1;
	  H5Dclose(status);
	  status = 0;
	} else
	{
	  status = 0;
	  break;
	}
    }

  if(*nmeta < 1)
    { 
      return(FAIL);
    }

  return(status);
}

hsize_t HE5_EHset_error_on(int flag, int err_level)
{
  H5Eclear();
  if (flag == 0 || flag == 1 || flag == 2)
    {
      error_flg = flag;
    }
  else
    {
      error_flg = 0;
    }
  return(error_flg);
}

long HE5_EHset_error_onF(int flag, int err_level)
{
  hsize_t status;
  status = HE5_EHset_error_on(flag, err_level);

  return((long)status);
}


herr_t HE5_EHprint(char *errbuf, const char *file, unsigned line)
{
  if (error_flg == 0)
    {
      H5Eprint(stderr);
    }
  else if(error_flg == 1)
    {
      fprintf(stderr, "%s in %s on line %d \n", errbuf, file, line);
    }
  H5Eclear();
  return(SUCCEED);
}

#ifdef H5_HAVE_FILTER_SZIP

/*-----------------------------------------------------------------------------
| Function:    HE5_szip_can_encode                                            |
|                                                                             |
|  Purpose:     Retrieve the filter config flags for szip, tell if            |
|               encoder is available.                                         |
|                                                                             |
|  Return:      1:  decode+encode is enabled                                  |
|               0:  only decode is enabled                                    |
|              -1: other                                                      |
|                                                                             |
|  Programmer:                                                                |
|                                                                             |
|  Modifications:                                                             |
|                                                                             |
-----------------------------------------------------------------------------*/
int HE5_szip_can_encode(void )
{

  herr_t       status;
  unsigned int filter_config_flags;
  
  status =H5Zget_filter_info(H5Z_FILTER_SZIP, &filter_config_flags);
  if ((filter_config_flags &
       (H5Z_FILTER_CONFIG_ENCODE_ENABLED|H5Z_FILTER_CONFIG_DECODE_ENABLED)) == 0) {
    /* filter present but neither encode nor decode is supported (???) */
    return -1;
  } else if ((filter_config_flags &
	      (H5Z_FILTER_CONFIG_ENCODE_ENABLED|H5Z_FILTER_CONFIG_DECODE_ENABLED)) ==
	     H5Z_FILTER_CONFIG_DECODE_ENABLED) {
    /* decoder only: read but not write */
    return 0;
  } else if ((filter_config_flags &
	      (H5Z_FILTER_CONFIG_ENCODE_ENABLED|H5Z_FILTER_CONFIG_DECODE_ENABLED)) ==
	     H5Z_FILTER_CONFIG_ENCODE_ENABLED) {
    /* encoder only: write but not read (???) */
    return -1;
  } else if ((filter_config_flags &
	      (H5Z_FILTER_CONFIG_ENCODE_ENABLED|H5Z_FILTER_CONFIG_DECODE_ENABLED)) ==
	     (H5Z_FILTER_CONFIG_ENCODE_ENABLED|H5Z_FILTER_CONFIG_DECODE_ENABLED)) {
    return 1;
  }
  return(-1);
}


#endif /* H5_HAVE_FILTER_SZIP */


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_EHHEisHE5                                                    |
|                                                                             |
|  DESCRIPTION: Find out if hdf file is HDF-EOS5 file (HDF5 based hdf-eos)    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units   Description                               |
|  ============   ======  =======  =====================================      |
|  retstatus      int              return TRUE  (or 1) if file is HE5         |
|                                       (can be opened by EHopen and has      |
|                                        SWATH, GRID, ZA, or POINT objects )  |
|                                  return FALSE (or 0) otherwise              |
|                                  return -1 if file does not exist           |
|                                                                             |
|  INPUTS:                                                                    |
|  filename        char            Input file name                            |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES: Although any HDF4 file with ECS and structure metadata is HDFEOS    |
|         file, but in practice we will consider a file as HDFEOS5 file if it |
|         is HDF5 based and contains one or more of the objects GRID, SWATH,  |
|         ZA, and POINT.                                                      |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Dec 13   Abe Taaheri   Original Programmer                                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_EHHEisHE5(char *filename)
{
  int       status = 0;	        /* return status variable for open calls */
  int       retstatus = 0;	/* routine return status variable        */
  hid_t     fid;
  herr_t    cstatus;	        /* return status variable for close calls */
  FILE      *fp;
  int       nGrid  = FAIL;     /* Number of grid structures in file */
  int       nSwath = FAIL;     /* Number of swaths in file */
  int       nPoint = FAIL;     /* Number of point structures in file */
  int       nZA    = FAIL;     /* Number of zas in file */

  if ((fp = fopen(filename,"r")) == NULL)
    {
      /* file does not exist */
      return(-1);
    }
  else
    {
      fclose( fp );
    }

  /* Open file using EHopen                   */
  /* If fails, then file is not HE5 file      */
  /* ---------------------------------------- */
  
  HE5_EHset_error_on(2, 0);/* suppress both HDFEOS5 and  HDF5 error repoets */

  /* open HDF-EOS5 file for reading */
  fid = HE5_EHopen( filename, H5F_ACC_RDONLY, H5P_DEFAULT );
  if ( fid == FAIL )
    {
      return(0); /* File is not HDF5 or HDFEOS5 file */
    }
  else
    {
      cstatus = HE5_EHclose(fid);
    }
  
  /* File is  HDF5 or HDFEOS5 file. See if it is HDFEOS5 */

  /* Call "HE5_EHinquire" routine for grid */
  /* ------------------------------------- */
  nGrid = (int)HE5_EHinquire(filename, "/HDFEOS/GRIDS", NULL, NULL);
  if(nGrid == FAIL )
    {
      /* Call "HE5_EHinquire" routine for swath */
      /* -------------------------------------- */
      nSwath = (int)HE5_EHinquire(filename, "/HDFEOS/SWATHS", NULL, NULL);
      if(nSwath == FAIL )
	{
	  /* Call HE5_EHinquire for point */
	  /* ---------------------------- */
	  nPoint = (int)HE5_EHinquire(filename, "/HDFEOS/POINTS", NULL, NULL);
	  if(nPoint == FAIL )
	    {
	      /* Call "HE5_EHinquire" routine for za */
	      /* ----------------------------------- */
	      nZA = HE5_EHinquire(filename, "/HDFEOS/ZAS",  NULL, NULL);
	      if ( nZA == FAIL )
		{
		  retstatus = 0;
		}
	      else
		{
		  retstatus = 1;
		}
	    }
	}
      else
	{
	  retstatus = 1;
	}
    }
  else
    {
      retstatus = 1;
    }

  HE5_EHset_error_on(0, 0);/* reset error report to default  HDF5 errors only*/

  if( retstatus == 0 )
    {
      /* Failed to open file using GDopen, SWopen, ZAopen, and PTopen */
      /* File is not HE5 file */
      return(0);
    }
  else
    {
      /* File is HE5 file */
      return(1);
    }
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

/* FORTRAN Macros */

FCALLSCFUN3(INT, HE5_EHidinfoF, HE5_EHIDINFO, he5_ehidinfo, INT, PINT,  PINT)
FCALLSCFUN2(DOUBLE, HE5_EHconvAngF, HE5_EHCONVANG, he5_ehconvang, DOUBLE, INT)
FCALLSCFUN5(INT, HE5_EHwrglatt, HE5_EHWRGLATT, he5_ehwrglatt, INT, STRING, INT, LONGV,PVOID)
FCALLSCFUN3(INT, HE5_EHrdglatt, HE5_EHRDGLATT, he5_ehrdglatt,INT,STRING,PVOID)
FCALLSCFUN4(INT, HE5_EHglattinf, HE5_EHGLATTINF, he5_ehglattinf, INT, STRING, PINT, PLONG)
FCALLSCFUN3(LONG, HE5_EHinqglatts, HE5_EHINQGLATTS, he5_ehinqglatts,INT,PSTRING,PLONG)
FCALLSCFUN5(INT, HE5_EHrdwrfileF, HE5_EHRDWRFILE, he5_ehrdwrfile, STRING, INT, INT, LONG, PVOID)
FCALLSCFUN6(INT, HE5_EHinqglbtype, HE5_EHINQGLBTYPE, he5_ehinqglbtype, INT, STRING, PINT, PINT, PINT, PLONG)
FCALLSCFUN2(LONG, HE5_EHset_error_onF, HE5_EHSET_ERROR_ONF, he5_ehset_error_onf, INT, INT)
FCALLSCFUN2(INT, HE5_EHgetversion, HE5_EHGETVERSION, he5_ehgetver, INT, PSTRING)
FCALLSCFUN1(INT, HE5_EHHEisHE5, HE5_EHHEISHE5, he5_ehheishe5, STRING)

#ifdef H5_HAVE_FILTER_SZIP
FCALLSCFUN0(INT, HE5_szip_can_encode, HE5_SZIP_CAN_ENCODE, he5_szip_can_encode)
#endif

#endif














