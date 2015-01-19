/*
 ----------------------------------------------------------------------------
 |    Copyright (C) 2002  Emergent IT Inc.  and Raytheon Systems Company    |
 |                                                                          |
 |  Permission to use, modify, and distribute this software and its         |
 |  documentation for any purpose without fee is hereby granted, provided   |
 |  that the above copyright notice appear in all copies and that both that |
 |  copyright notice and this permission notice appear in supporting        |
 |                          documentation.                                  |
 ----------------------------------------------------------------------------
 */

#include <HE5_HdfEosDef.h>


#define HE5_ZAIDOFFSET 671088642
#define HE5_NZA              200
 
/* ZA Structure External Arrays */
struct HE5_zaStructure
{
  hid_t         fid;                     /* HDF-EOS file ID                  */
  hid_t         obj_id;                  /* "ZAS" group ID                   */
  hid_t         za_id;                   /* specified za group ID            */
  hid_t         data_id;                 /* "Data Fields" group ID           */
  hid_t         plist;                   /* current dataset creation property*/
 
  int           active;                  /* Flag: file active or not         */
  int           compcode;                /* compression mode                 */
  int           compparm[5];             /* compression level                */
 
  HE5_DTSinfo   *ddataset;               /* pointer to Data field info       */
 
  long          nDFLD;                   /* number of data fields            */
 
  char          zaname[HE5_OBJNAMELENMAX];/* za name                         */
};
 
struct HE5_zaStructure HE5_ZAXZa[HE5_NZA];
 

/* INTERNAL FUNCTION PROTOTYPES */

int HE5_szip_can_encode(void );
static herr_t
HE5_ZAchkzaid(hid_t zaID, const char *routname, hid_t *fid, hid_t *gid,  long *idx);
static long
HE5_ZAinqfield(hid_t zaID, char *datafield, char *fieldlist, int rank[], hid_t ntype[]);
static size_t
HE5_ZAfieldsizeID(hid_t zaID, char *fieldname, hid_t *fieldID);
static herr_t
HE5_ZAdefinefield(hid_t zaID, char *datafield, const char *fieldname, char *dimlist, char *maxdimlist, hid_t typeID);
static herr_t
HE5_ZAwrrdfield(hid_t zaID, char *fieldname, char *code, const hssize_t start[], const hsize_t stride[], const hsize_t count[],  void * datbuf);
static herr_t
HE5_ZAfldinfo(hid_t zaID, char *datafield, char *fieldname, int *rank, hsize_t dims[], hid_t ntype[], char *dimlist, char *maxdimlist);
static int 
HE5_ZAfldnameinfo(hid_t zaID, char *fieldname, char *fldactualname);


/* FORTRAN WRAPPER PROTOTYPES */


/* File/ZA access routines */

int
HE5_ZAopenF(char *filename, int flags);
int 
HE5_ZAcreateF(int fileID, char *zaname);
int 
HE5_ZAattachF(int fileID, char *zaname);
int
HE5_ZAdetachF(int zaID);
int 
HE5_ZAcloseF(int fileID);


/* Definition routines */

int
HE5_ZAdefdimF(int zaID,  char *dimname, long dim);
int 
HE5_ZAdefineF(int zaID, char *fieldname, char *fortdimlist, char *fortmaxdimlist, int numtype);
int
HE5_ZAdefchunkF(int zaID, int ndims, long *dim);
int   
HE5_ZAdefcompF(int zaID, int compcode,  int *compparm);
int   
HE5_ZAdefcomchunkF(int zaID, int compcode, int *compparm, int rank, long *dim);
int    
HE5_ZAfldrenameF(int zaID, char *oldfieldname, char *newfieldname);


/* I/O routines */

int
HE5_ZAwriteF(int zaID, char *fieldname, long fortstart[], long fortstride[], long fortedge[], void *data);
int
HE5_ZAwritecharF(int zaID, char *fieldname, int elemlen, int numelem, long fortstart[], long fortstride[], long fortedge[], void *data);
int
HE5_ZAreadF(int zaID, char *fieldname, long fortstart[],  long fortstride[], long fortedge[], void *buffer);
int
HE5_ZAreadcharF(int zaID, char *fieldname, int elemlen, int numelem, long fortstart[],  long fortstride[], long fortedge[], void *buffer);
int
HE5_ZAsetfill(int zaID, char *fieldname, int ntype, void *fillval);
int
HE5_ZAgetfill(int zaID, char *fieldname, void *fillval);
int
HE5_ZAwrattr(int zaID, char *attrname, int ntype, long fortcount[], void * datbuf);
int
HE5_ZAwrgattr(int zaID, char *attrname, int ntype, long fortcount[], void *datbuf);
int
HE5_ZAwrlattr(int zaID, char *fieldname, char *attrname, int ntype, long fortcount[], void *datbuf);
int    
HE5_ZArdattr(int zaID, char *attrname, void *datbuf);
int    
HE5_ZArdgattr(int zaID, char *attrname, void *datbuf);
int    
HE5_ZArdlattr(int zaID, char *fieldname, char *attrname, void *datbuf);
int    
HE5_ZAsetaliasF(int zaID, char *fieldname, char *fortaliaslist);
int    
HE5_ZAdropaliasF(int zaID, int fldgroup, char *aliasname);


/* Inquiry routines */

long 
HE5_ZAinqzaF(char *filename, char *zalist, long *strbufsize);
long 
HE5_ZAnentriesF(int zaID, int entrycode, long *strbufsize);
long 
HE5_ZAdiminfoF(int zaID, char *dimname);
int
HE5_ZAinfoF(int zaID, char *fieldname, int *rank, long dims[], int *ntype, char *fortdimlist, char *fortmaxdimlist);
long
HE5_ZAinqdimsF(int zaID, char *dimlist, long dims[]);
long 
HE5_ZAinquireF(int zaID, char *fieldlist, int rank[], int ntype[]);
int
HE5_ZAatinfo(int zaID, char *attrname, int *ntype, long *fortcount);
int
HE5_ZAgatinfo(int zaID, char *attrname, int *ntype, long *fortcount);
int
HE5_ZAlatinfo(int zaID, char *fieldname, char *attrname, int *ntype, long *fortcount);
long 
HE5_ZAinqattrsF(int zaID, char *attrnames, long *strbufsize);
long 
HE5_ZAinqgattrs(int zaID, char *attrnames, long *strbufsize);
long 
HE5_ZAinqlattrs(int zaID, char *fieldname, char *attrnames, long *strbufsize);
int 
HE5_ZAcompinfoF(int zaID, char *fieldname, int *compcode, int compparm[]);
int   
HE5_ZAidtype(int zaID, char *fieldname, char *attrname, int fieldgroup, int *Type, int *Class, int *Order, long *size);
int    
HE5_ZAaliasinfoF(int zaID, int fldgroup, char *aliasname, int *length, char *buffer);
long
HE5_ZAinqfldaliasF(int zaID, char *fldalias, long *strbufsize);
int
HE5_ZAchunkinfoF(int zaID, char *fieldname, int *ndims, long dims[]);
long
HE5_ZAgetaliaslistF(int zaID, int fldgroup, char *aliaslist, long *strbufsize);


/* EXTERNAL DATA FILE INTERFACE */
 
int
HE5_ZAsetextdataF(int zaID, char *fortfilelist, long offset[], long size[]);
int
HE5_ZAgetextdataF(int zaID, char *fieldname, long namelength, char *fortfilelist, long offset[], long size[]);

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAopen                                                       |
|                                                                             |
|  DESCRIPTION: Opens or creates HDF-EOS file in order to create, read,       |
|               or write a ZA.                                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|     fid         hid_t    None       HDF-EOS file ID                         |
|                                                                             |
|  INPUTS:                                                                    |
|     filename    char*    None       File name string                        |
|     flags       uintn    None       File access code                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|     None                                                                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date       Programmer   Description                                       |
|  ========   ============  ================================================= |
|  Jul 02      S.ZHAO       Original development                              |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t
HE5_ZAopen(const char *filename, uintn flags)
{
  hid_t     fid      = FAIL;/* hdf5 type file ID      */
 
  char      *errbuf  = NULL;/* Error message buffer   */
 
 
  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAopen", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
 
      return(FAIL);
    }
 
  /* Call HE5_EHopen to open file */
  /* ---------------------------- */
  fid = HE5_EHopen(filename,flags, H5P_DEFAULT );
  if(fid == FAIL)
    {
      sprintf(errbuf, "Cannot open the file \"%s\". Check the file name. \n", filename);
      H5Epush(__FILE__, "HE5_ZAopen", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
 
  free(errbuf);
 
  return (fid);
}
 

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAcreate                                                     |
|                                                                             |
|  DESCRIPTION: Creates a new ZA structure and returns ZA ID                  |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|   zaID          hid_t        None        ZA structure ID                    |
|                                                                             |
|  INPUTS:                                                                    |
|   fid           hid_t        None        HDF-EOS file ID                    |
|   zaname        const char*  None        za name string                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|   None                                                                      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t
HE5_ZAcreate(hid_t fid, const char *zaname)
{
  hid_t           zaID           = FAIL;  /* return value of ZA ID          */
  hid_t           HDFfid         = FAIL;  /* HDF-EOS file ID                */
  hid_t           gid            = FAIL;  /* "HDFEOS" group ID              */
  hid_t           za_id          = FAIL;  /* HDF-EOS ZA ID                  */
  hid_t           data_id        = FAIL;  /* "Data Fields" group ID         */
  hid_t           ZAS_ID         = FAIL;  /* "ZAS" group ID                 */
  hid_t           i;                      /* Loop index                     */
 
  int             nzaopen        = 0;     /* # of za structures open        */
 
  herr_t          status         = FAIL;  /* routine return status variable */
 
  uintn           access         = 0;     /* Read/Write file access code    */
 
  long            nza            = FAIL;  /* za counter                     */
 
  char            *errbuf =(char *)NULL;  /* Buffer for error message       */
  char            *utlbuf =(char *)NULL;  /* Utility buffer                 */
  char            *utlbuf2=(char *)NULL;  /* Utility buffer 2               */
 
  HE5_LOCK;
  CHECKNAME(zaname);
 
  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf = (char *)calloc( HE5_HDFE_DIMBUFSIZE, sizeof(char ) ) ;
  if ( errbuf == NULL )
    {
      H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
 
  /*
   * Check HDF-EOS file ID, get  HDFEOS group ID  and
   * access code
   */
  status = HE5_EHchkfid(fid, "HE5_ZAcreate", &HDFfid,  &gid, &access);
  /* Check file access */
  /* -------------------------- */
  if ( access == 0 )
    {
      status = FAIL;
      sprintf(errbuf, "Cannot call HE5_ZAcreate on a file opened read-only.\n");
      H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for file ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
 
      return(FAIL);
    }
 

  /* Check zaname for length */
  /* ----------------------- */
  if ( strlen(zaname) > HE5_OBJNAMELENMAX)
    {
      sprintf(errbuf, "za name \"%s\" must be less than %d characters.\n", zaname, HE5_OBJNAMELENMAX);
      H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
 
      return(FAIL);
    }
 
 
  /* Determine number of zas currently opened */
  /* ---------------------------------------- */
  for (i = 0; i < HE5_NZA; i++)
    nzaopen += HE5_ZAXZa[i].active;
 
  if(nzaopen <  HE5_NZA)
    {
 
      /* Probe , check if "ZAS" group exists */
      /* ----------------------------------- */
      H5E_BEGIN_TRY {
	ZAS_ID = H5Gopen(gid, "ZAS");
      }
      H5E_END_TRY;
 
      if(ZAS_ID == FAIL )
	{
	  nza = 0;
 
	  /* Create the "ZAS" group */
	  /* ---------------------- */
	  ZAS_ID = H5Gcreate(gid,"ZAS",0);
	  if(ZAS_ID == FAIL)
	    {
	      sprintf(errbuf,"Cannot create the \"ZAS\" group.\n");
	      H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_OHDR, H5E_CANTCREATE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
 
	      return(FAIL);
	    }
 
	}
      else
	{
	  /* Probe , check if zaname group exist */
	  /* ----------------------------------- */
	  H5E_BEGIN_TRY{
	    za_id = H5Gopen(ZAS_ID, zaname);
	  }
	  H5E_END_TRY;
 
	  if( za_id == FAIL )
	    {
	      /* Get the number of zas in "ZAS" group */
	      /* ------------------------------------ */
	      nza = HE5_EHdatasetcat(fid, "/HDFEOS/ZAS", NULL, NULL);
	      if (nza == FAIL)
		{
		  sprintf(errbuf,"Cannot retrieve the number of zas.\n");
		  H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  status = H5Gclose(ZAS_ID);
		  free(errbuf);
 
		  return(FAIL);
		}
 
	    }
	  else
	    {
	      sprintf(errbuf, "za \"%s\" already exists.\n", zaname);
	      H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_OHDR, H5E_EXISTS, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      status = H5Gclose(ZAS_ID);
	      free(errbuf);
 
	      return(FAIL);
	    }
 
	}
 
 
      /* za does not exist */
      /* ----------------- */
 
 
      /* Create zaname group */
      /* ------------------- */
      za_id = H5Gcreate( ZAS_ID, zaname, 0);
      if(za_id == FAIL)
	{
	  sprintf(errbuf,"Cannot create the \"%s\" group.\n", zaname);
	  H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_OHDR, H5E_CANTCREATE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  status = H5Gclose(ZAS_ID);
	  free(errbuf);
 
	  return(FAIL);
	}
 
 
      /* Create /ZA/zaname/Data Fields group */
      /* ----------------------------------- */
      data_id = H5Gcreate( za_id, "Data Fields", 0);
      if(data_id == FAIL)
	{
	  sprintf(errbuf,"Cannot create the \"Data Fields\" group.\n");
	  H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_OHDR, H5E_CANTCREATE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  status = H5Gclose(ZAS_ID);
	  free(errbuf);
 
	  return(FAIL);
	}
 
 
      /* Allocate memory for utility buffers */
      /* ----------------------------------- */
      utlbuf  = (char *)calloc(HE5_HDFE_UTLBUFSIZE, sizeof(char ) ) ;
      if ( utlbuf == NULL )
	{
	  H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for utility buffer.\n");
	  HE5_EHprint("Error: Cannot allocate memory for utility buffer, occured", __FILE__, __LINE__);
	  status = H5Gclose(ZAS_ID);
	  free(errbuf);
 
	  return(FAIL);
	}
      utlbuf2 = (char *)calloc(HE5_HDFE_UTLBUFSIZE,  sizeof(char ) ) ;
      if ( utlbuf2 == NULL )
	{
	  H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_RESOURCE, H5E_NOSPACE,"Cannot allocate memory for utility buffer2.\n");
	  HE5_EHprint("Error: Cannot allocate memory for utility buffer2, occured", __FILE__, __LINE__);
	  status = H5Gclose(ZAS_ID);
	  free(errbuf);
	  free(utlbuf);
 
	  return(FAIL);
	}
 
 
      /* Establish za in "Structural MetaData" Block */
      /* ------------------------------------------- */
      sprintf(utlbuf, "%s%li%s%s%s", "\tGROUP=ZA_", nza + 1, "\n\t\tZaName=\"", zaname, "\"\n");
      strcat(utlbuf, "\t\tGROUP=Dimension\n");
      strcat(utlbuf, "\t\tEND_GROUP=Dimension\n");
      strcat(utlbuf, "\t\tGROUP=DimensionMap\n");
      strcat(utlbuf, "\t\tEND_GROUP=DimensionMap\n");
      strcat(utlbuf, "\t\tGROUP=IndexDimensionMap\n");
      strcat(utlbuf, "\t\tEND_GROUP=IndexDimensionMap\n");
      strcat(utlbuf, "\t\tGROUP=DataField\n");
      strcat(utlbuf, "\t\tEND_GROUP=DataField\n");
      sprintf(utlbuf2, "%s%li%s","\tEND_GROUP=ZA_", nza + 1, "\n");
      strcat(utlbuf, utlbuf2);
 
      /* Insert metadata buffer to the "StructMetadata" */
      /* ---------------------------------------------- */
      status = HE5_EHinsertmeta(fid, (char*)zaname, "z", 1004L, utlbuf, NULL);
      if (status == FAIL )
	{
	  sprintf(errbuf,"Cannot insert metadata buffer to \"StructMetadata\".\n");
	  H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  status = H5Gclose(ZAS_ID);
	  free(errbuf);
	  free(utlbuf);
	  free(utlbuf2);
 
	  return(FAIL);
	}
 
      /* Release "ZAS" group ID */
      /* ---------------------- */
 
      status = H5Gclose(ZAS_ID);
      if(status == FAIL )
	{
	  sprintf(errbuf,"Cannot release the \"ZAS\" group ID.\n");
	  H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  status = H5Gclose(ZAS_ID);
	  free(utlbuf);
	  free(utlbuf2);
	  free(errbuf);
 
	  return(FAIL);
	}
 
 
      /* Assign zaID & Load za and ZAXZa table entries */
      /* --------------------------------------------- */
      for (i = 0; i < HE5_NZA; i++)
	{
	  if (HE5_ZAXZa[i].active == 0)
	    {
	      /*
	       * Set za_id, Set za entry active, Store root group
	       * ID, Store sub group IDs, Store HDF-EOS file ID
	       */
	      HE5_ZAXZa[i].active                     = 1;
	      HE5_ZAXZa[i].za_id                      = za_id;
	      HE5_ZAXZa[i].data_id                    = data_id;
	      HE5_ZAXZa[i].fid                        = fid;
	      HE5_ZAXZa[i].obj_id                     = ZAS_ID;
	      HE5_ZAXZa[i].plist                      = FAIL;
	      strcpy(HE5_ZAXZa[i].zaname, zaname);
	      HE5_ZAXZa[i].compcode                   = HE5_HDFE_COMP_NONE;
	      zaID                                    = i + HE5_ZAIDOFFSET;
	      break;
	    }
	}
    }
  else
    {
      /* Too many files opened */
      /* --------------------- */
      zaID = FAIL;
      sprintf(errbuf,"\n Illegal attempt to open more than %d zas simultaneously.\n", HE5_NZA);
      H5Epush(__FILE__, "HE5_ZAcreate", __LINE__, H5E_OHDR, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
 
 
  /* Release allocated memory */
  /* ------------------------ */
  free(errbuf);
  free(utlbuf);
  free(utlbuf2);
 
 COMPLETION:
 
  HE5_UNLOCK;
  return (zaID);
}
 


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAattach                                                     |
|                                                                             |
|  DESCRIPTION:  Attaches to an existing za within the file.                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  zaID           hid_t               ZA structure ID                         |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               HDF-EOS file ID                         |
|  zaname         const char*         ZA structure name                       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|             The iteration over the "Data Fields" group has the followings   |
|             side effect:  it counts as an object (which is supposed to be   |
|             a dataset) the group "Ragged Arrays". So the external structure |
|             "ddataset" gets filled with  a fake member and nDFLD gets       |
|             incremented by one. This shouldn't affect the other interfacing |
|             routines unless someone tries to  access the datasets in "Data  |
|             Fields" group directly through their IDs retrieved from external|
|             "ddataset" structure, or use "nDFLD" data member of to retrieve |
|             the actual number of data sets (fields).  To avoid potential    |
|             problems, the dataset counter is decremented by one and the     |
|             corresponding error checking are inserted to handle this case.  |
|                                                                             |
|                                                                             |
|   Date        Programmer    Description                                     |
|  =========   ============  ==============================================   |
|  Jul 02       S.ZHAO        Original development                            |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t
HE5_ZAattach(hid_t fid, const char *zaname)
{
 
  hid_t           zaID       = FAIL;/* ZA ID (return value)          */
  hid_t           i;                /* Loop index                    */
  hid_t           emptyslot  =  0;  /* Spare za index                */
 
  herr_t          status     = FAIL;/* Return status variable        */
 
  int             nzaopen =  0;     /* # of za structures open       */
  int             idx        = FAIL;/* return value from an operator */
 
  uintn           acs        = 0;   /* Read/Write file access code   */
 
  hid_t           HDFfid     = FAIL;/* HDF-EOS file ID               */
  hid_t           datid      = FAIL;/* Dataset ID                    */
  hid_t           gid        = FAIL;/* "HDFEOS" group ID             */
  hid_t           data_id    = FAIL;/* "Data Fields" Group ID        */
  hid_t           za_id      = FAIL;/* za ID                         */
 
 
  HE5_OBJINFO     inout;         /* IN/OUT operator data             */
 
  char            *grpname =NULL;/* Ptr to a group  name string      */
  char            *tmpname =NULL;/* ..... temporary name   ...       */
  char            *namebuf =NULL;/* ..... temporary buffer name ...  */
  char            *fldname =NULL;/* ..... temporary field name ...   */
  char            *comma   =NULL;
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer*/
 
 
  HE5_LOCK;
  CHECKPOINTER(zaname);
 
  /* Check HDF-EOS file ID, get back HDF file ID and access code */
  /* ----------------------------------------------------------- */
  status = HE5_EHchkfid(fid, "HE5_ZAattach", &HDFfid, &gid, &acs);
  if ( status == FAIL )
    {
      H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_ARGS, H5E_BADRANGE, "Checking for file ID failed.\n");
      HE5_EHprint("Error: Checking for file ID failed, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Determine number of zas currently opened */
  /* ---------------------------------------- */
  for (i = 0; i < HE5_NZA; i++)
    nzaopen += HE5_ZAXZa[i].active;
 
  /* Check there is a room for more */
  /* ------------------------------ */
  if (nzaopen < HE5_NZA)
    {
      /* Search for group "/ZAS/zaname" */
      /* ============================== */
      grpname = (char *) calloc( (strlen(zaname)+40), sizeof(char) );
      if (grpname == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory for grpname.\n") ;
	  H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
 
	  return(FAIL);
	}
 
      strcpy( grpname, "/HDFEOS/ZAS/");
      strcat( grpname, zaname);
 
      /*
       * If group with zaname found, open the group, load tables
       */
      H5E_BEGIN_TRY{
	za_id = H5Gopen(HDFfid, grpname);
      }
      H5E_END_TRY;
 
      if( za_id == FAIL )
	{
	  sprintf(errbuf, "za \"%s\" does not exist within HDF-EOS file.\n",zaname) ;
	  H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
 
	  return(FAIL);
	}
 
 
      /* Attach to "Data Fields" */
      /* ======================= */
      data_id = H5Gopen( za_id, "Data Fields");
      if (data_id == FAIL)
	{
	  sprintf(errbuf, "Cannot open the \"Data Fields\" group. \n") ;
	  H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
 
	  return(FAIL);
	}
 
 
      /* Setup External Arrays */
      /* --------------------- */
      for (i = 0; i < HE5_NZA; i++)
	{
	  /* Find empty entry in array */
	  /* ------------------------- */
	  if (HE5_ZAXZa[i].active == 0)
	    {
	      /* Set IDs, codes., etc */
	      /* ==================== */
	      HE5_ZAXZa[i].active           = 1;
	      HE5_ZAXZa[i].data_id          = data_id;
	      HE5_ZAXZa[i].fid              = fid;
	      HE5_ZAXZa[i].za_id            = za_id;
	      HE5_ZAXZa[i].plist            = FAIL;
	      HE5_ZAXZa[i].compcode         = HE5_HDFE_COMP_NONE;
 
	      zaID                          = i + HE5_ZAIDOFFSET;
	      strcpy(HE5_ZAXZa[i].zaname, zaname);
	      break;
	    }
	}
      emptyslot = i;
 
      /*  Access za "Data Fields" datasets   */
      /* ----------------------------------- */
      tmpname = (char *) calloc( (strlen(grpname)+ 20), sizeof(char));
      if (tmpname == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory for tmpname. \n") ;
	  H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
 
	  return(FAIL);
	}
 
      strcpy(tmpname,grpname);
      strcat(tmpname,"/Data Fields");
 
 
      /* Get the number of OBJECTS in "Data Fields" group */
      /* ================================================ */
      inout.count   = 0;
      inout.name    = NULL;
      inout.strsize = 0;
 
      idx = H5Giterate(data_id, tmpname , NULL, HE5_EHobj_info, &inout);
      if ( idx == FAIL )
	{
	  sprintf(errbuf, "Iteration of \"HE5_EHobj_info()\" operation failed. \n") ;
	  H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  free(tmpname);
 
	  return(FAIL);
	}
 
      free(tmpname);
 
      /* Set the data members of external structure */
      /* ========================================== */
      HE5_ZAXZa[emptyslot].nDFLD = inout.count;
 
      /*------------------------------------------------------*/
      /* Loop through all data field datasets and open it     */
      /* Allocate space for namebuf, copy inout.name into it, */
      /*              and append comma                        */
      /*------------------------------------------------------*/
      if( inout.name != NULL && inout.count > 0 )
	{
	  /* Allocate memory to ddataset struct */
	  /* ---------------------------------- */
	  HE5_ZAXZa[emptyslot].ddataset = (HE5_DTSinfo *)realloc((void *)HE5_ZAXZa[emptyslot].ddataset,(HE5_ZAXZa[emptyslot].nDFLD) * sizeof(HE5_DTSinfo));
 
	  /* Allocate memory to the list of dataset names */
	  /* -------------------------------------------- */
	  namebuf = (char *) calloc(strlen(inout.name) + 2, sizeof(char));
	  if (namebuf == NULL)
	    {
	      sprintf(errbuf, "Cannot allocate memory for namebuf. \n") ;
	      H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(grpname);
 
	      return(FAIL);
	    }
 
	  strcpy(namebuf, inout.name);
	  strcat(namebuf, ",");
 
	  /* Find comma */
	  /* ---------- */
	  comma = strchr(namebuf, ',');
 
	  i = 0;
 
	  /* Parse the list of dataset names */
	  /* ------------------------------- */
	  while (comma != NULL)
	    {
	      /* Allocate memory for fldname buffer */
	      /* ---------------------------------- */
	      fldname = (char *) calloc(comma - namebuf + 1, sizeof(char));
	      if (fldname == NULL)
		{
		  sprintf(errbuf, "Cannot allocate memory for fldname. \n") ;
		  H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(grpname);
		  free(namebuf);
 
		  return(FAIL);
		}
 
	      /* Copy field list entry to fldname */
	      /* -------------------------------- */
	      memmove(fldname, namebuf, comma - namebuf);
	      fldname[comma-namebuf]=0;
 
	      /* open dataset fldname */
	      /* -------------------- */
	      H5E_BEGIN_TRY{
		datid = H5Dopen(data_id, fldname);
	      }H5E_END_TRY;
	      if ( datid == FAIL )
		{
		  sprintf(errbuf, "Cannot open the dataset \"%s\". \n",fldname );
		  H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(grpname);
		  free(namebuf);
		  free(fldname);
 
		  return(FAIL);
		}
 
	      /* allocate memory to the dataset name */
	      /* ----------------------------------- */
	      HE5_ZAXZa[emptyslot].ddataset[i].name = (char *)calloc((strlen(fldname)+1), sizeof(char));
	      if (HE5_ZAXZa[emptyslot].ddataset[i].name == NULL)
		{
		  sprintf(errbuf, "Cannot allocate memory for the dataset name. \n") ;
		  H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(grpname);
		  free(namebuf);
		  free(fldname);
 
		  return(FAIL);
		}
 
	      /* Set the data members of external structure */
	      /* ========================================== */
	      HE5_ZAXZa[emptyslot].ddataset[i].ID = datid;
	      strcpy(HE5_ZAXZa[emptyslot].ddataset[i].name, fldname);
 
 
	      /* Go to next field entry, find next comma, ... */
	      /* ============================================ */
	      memmove(namebuf, comma + 1, strlen(comma + 1) + 1);
	      comma = strchr(namebuf, ',');
	      if (fldname != NULL) free(fldname);
	      i++;
 
	    }
 
	  if (namebuf != NULL) free(namebuf);
	  if(inout.name != NULL ) free( inout.name);
 
	} /* end if(inout.name!=NULL) */
    }
  else
    {
      /* Too many zas opened */
      /* ------------------- */
      zaID = FAIL;
      sprintf(errbuf,"\n Illegal attempt to open more than %d zas simultaneously. \n", HE5_NZA);
      H5Epush(__FILE__, "HE5_ZAattach", __LINE__, H5E_OHDR, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
 
 COMPLETION:
  HE5_UNLOCK;
  free (grpname);
  return(zaID);
}
 


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAchkzaid                                                    |
|                                                                             |
|  DESCRIPTION: Checks for valid zaID and returns file ID and                 |
|               "HDFEOS" group ID                                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  routname       const char*         Routine name                            |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fid            hid_t*              HDF-EOS file ID                         |
|  gid            hid_t*              "HDFEOS" Group ID                       |
|  idx            long*               za index                                |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t
HE5_ZAchkzaid(hid_t zaID, const char *routname, hid_t *fid, hid_t *gid, long *idx )
{
  herr_t            status    = FAIL;/* routine return status variable */
 
  uintn             access    =  0;      /* Read/Write access code         */
 
  hid_t             HDFfid    = FAIL;/* HDF-EOS file ID                */
  hid_t             idOffset  = HE5_ZAIDOFFSET;
 
  char              message1[] = "Invalid za id: %d in routine \"%s\".  ID must be >= %d and < %d.\n";
  char              message2[] = "za id %d in routine \"%s\" not active.\n";
  char              errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer */
 
 
  /* Check for valid za ID */
  /* --------------------- */
  if (zaID < idOffset || zaID >= HE5_NZA + idOffset)
    {
      status = FAIL;
      sprintf(errbuf, message1, zaID, routname, idOffset, HE5_NZA + idOffset);
      H5Epush(__FILE__, "HE5_ZAchkzaid", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  else
    {
      /* Check for active za ID */
      /* ---------------------- */
      if (HE5_ZAXZa[zaID % idOffset].active == 0)
        {
	  status = FAIL;
	  sprintf(errbuf,  message2, zaID, routname);
	  H5Epush(__FILE__, "HE5_ZAchkzaid", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
        }
      else
        {
	  *idx = HE5_EHhid2long(zaID % idOffset);
	  if (*idx == FAIL)
	    {
	      status = FAIL;
	      sprintf(errbuf,"Cannot set up the za index.\n");
	      H5Epush(__FILE__, "HE5_ZAchkzaid", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	    }
 
	  /* Get HDF file ID &  "HDFEOS" group ID */
	  /* ------------------------------------ */
	  *fid   = HE5_ZAXZa[*idx].fid;
	  status = HE5_EHchkfid(*fid,"", &HDFfid, gid, &access);
	  if (status == FAIL)
	    {
	      sprintf(errbuf,"Checking for file ID failed.\n");
	      H5Epush(__FILE__, "HE5_ZAchkzaid", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	    }
        }
    }
  return (status);
}
 


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefdim                                                     |
|                                                                             |
|  DESCRIPTION: Defines numerical value of dimension                          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        Return status  (0) SUCCEED, (-1) FAIL   |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  dimname        char*               dimension name                          |
|  dim            hsize_t             dimension size                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAdefdim(hid_t zaID,  char *dimname, hsize_t dim)
{
  herr_t       status    = FAIL;/* routine return status variable  */
  
  hid_t        fid       = FAIL;/* HDF-EOS file ID                 */
  hid_t        gid       = FAIL;/* "HDFEOS" group ID               */
  
  long         idx       = FAIL;/* za index                        */
  
  char         errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */
  
  HE5_LOCK;
  CHECKNAME(dimname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdefdim", &fid, &gid, &idx); 
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAdefdim", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Make certain that dim != 0 */
  /* -------------------------- */
  if ( dim == 0 )
    {
      status = FAIL;
      sprintf(errbuf, "Invalid (zero) dimension size.\n");
      H5Epush(__FILE__, "HE5_ZAdefdim", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  
  /* Write dimension to the dataset "StructMetadata.0" */
  /* ------------------------------------------------- */
  status = HE5_EHinsertmeta(fid, HE5_ZAXZa[idx].zaname,"z", 0L, dimname, &dim);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot insert the value for \"%s\" dimension into Metadata.\n", dimname);
      H5Epush(__FILE__, "HE5_ZAdefdim", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

 COMPLETION:
  HE5_UNLOCK;
  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefdimscale                                                |
|                                                                             |
|  DESCRIPTION: Defines dimension scale for all fields that use the given     |
|               dimesion                                                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  dimname        char                dim name                                |
|  dimsize        hsize_t             Dimemsion size value                    |
|  numbertype_in  hid_t               dim scale data type such as DFNT_INT32, |
|                                     DFNT_FLOAT64, DFNT_FLOAT32, etc.        |
|  data           void                data buffer for write                   |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES: This function finds out which field in a ZA has the dimension       |
|         and sets dimension scale for that field calling HE5_ZAsetdimscale() |
|         User must detach ZA after creating fields and attach again before   |
|         calling the routine.                                                |
|                                                                             |
|                                                                             |
|   Date      Programmer    Description                                       |
|  ========   ============  ===============================================   |
|  Dec 2013   Abe Taaheri   Original Programmer                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAdefdimscale(hid_t zaID,  char *dimname,
		  const hsize_t dimsize, hid_t numbertype_in, void * data)
{
  herr_t       status    = FAIL;/* routine return status variable  */
  hid_t        fid       = FAIL;/* HDF-EOS file ID                 */
  hid_t        gid       = FAIL;/* "HDFEOS" group ID               */
  long         idx       = FAIL;/* Swath index                     */
  char         errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */
  char         *fieldlist = NULL;
  long         strbufsize;
  long         nflds   = FAIL;

  HE5_LOCK;
  CHECKNAME(dimname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and Swath index */
  /* ------------------------------------------------------ */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdefdimscale", &fid, &gid, &idx); 
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for Za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAdefdimscale", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Make certain that dimsize != 0 */
  /* ------------------------------ */
  if ( dimsize == 0 )
    {
      status = FAIL;
      sprintf(errbuf, "Invalid (zero) dimension size.\n");
      H5Epush(__FILE__, "HE5_ZAdefdimscale", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }

  /* Loop over all fields in the ZA and find out if 
     field's dimension list  has the dimension name. 
     If true then set the dimension scale for the dim 
     in that field
  */

  /* Inquire Data Fields first*/
  /* ------------------------ */
  nflds = HE5_ZAnentries(zaID, HE5_HDFE_NENTDFLD, &strbufsize);
  if (nflds == FAIL)
    {
      sprintf(errbuf, "Cannot get the number of fields in \"Data Fields\" group. \n");
      H5Epush(__FILE__, "HE5_ZAdefdimscale", __LINE__, 
	      H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  else
    {
      fieldlist = (char *) calloc(strbufsize + 1, sizeof(char));
      if(fieldlist == NULL)
	{
	  sprintf(errbuf,"Cannot allocate memory.\n");
	  H5Epush(__FILE__, "HE5_ZAdefdimscale", __LINE__, 
		  H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      nflds = HE5_ZAinqfield(zaID, "Data Fields", fieldlist, NULL, NULL);
      if (nflds == FAIL)
	{
	  sprintf(errbuf, "Cannot get the list of fields in \"Data Fields\" group. \n");
	  H5Epush(__FILE__, "HE5_ZAdefdimscale", __LINE__, 
		  H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(fieldlist);
	  return(FAIL);
	}

      if(nflds > 0)
	{
	  size_t        fldnmlen[HE5_FLDNUMBERMAX];      /* Array of namelengths */
	  char          *fldnm[HE5_FLDNUMBERMAX];        /* Array of names       */
	  char          tempdimlist[HE5_HDFE_DIMBUFSIZE];/* Dimension list       */
	  int           rank     = FAIL;                 /* Rank of dataset      */
	  hsize_t	tempdims[HE5_DTSETRANKMAX];      /* Dimension sizes array*/
	  hid_t         dtype[1] = {FAIL};               /* Data type            */
	  long          ntflds   =  0;                   /* field counter        */
	  int           i;
	  char          *tempfield = NULL;
	  unsigned int  Dimindex;

	  ntflds = HE5_EHparsestr(fieldlist, ',', fldnm,fldnmlen);
	  if(ntflds != FAIL)
	    {
	      for(i = 0; i < ntflds; i++)
		{
		  tempfield      = (char *)calloc(fldnmlen[i] + 1, sizeof(char));
		  memmove(tempfield,fldnm[i],fldnmlen[i]);
		  tempfield[fldnmlen[i]]='\0';

		  /* for this field see if the dimlist contains dimname */
		  status = HE5_ZAfldinfo(zaID, "Data Fields", tempfield, &rank, tempdims, 
					   dtype, tempdimlist, NULL);

		  if (status != SUCCEED)
		    {
		      sprintf(errbuf, "Field \"%s\" not found.\n", tempfield);
		      H5Epush(__FILE__, "HE5_ZAdefdimscale", __LINE__, H5E_ARGS, 
			      H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      free(tempfield);
		      free(fieldlist);
		      return(FAIL);
		    }

		  Dimindex = FAIL;
		  Dimindex = HE5_EHstrwithin(dimname, tempdimlist, ',');
		  if (Dimindex == FAIL) /* dimension not found */
		    {
		      continue; /*dimlist does not contain dim name */
		    }
		  else
		    {
		      /* call HE5_ZAsetdimscle */
		      status = HE5_ZAsetdimscale(zaID, (char *)tempfield, 
						 dimname,
						 dimsize, 
						 numbertype_in, 
						 data);
		      if ( status == FAIL )
			{
			  sprintf(errbuf,"Cannot set dimension scale %s for the field %s. \n", 
				  dimname, tempfield);
			  H5Epush(__FILE__, "HE5_ZAdefdimscale", __LINE__, 
				  H5E_OHDR, H5E_NOTFOUND, errbuf);
			  HE5_EHprint(errbuf, __FILE__, __LINE__);
			  free(tempfield);
			  free(fieldlist);
			  return(status);
			}
		    }
		}
	      free(tempfield);
	      tempfield = NULL;
	    }
	}
      free(fieldlist);
      fieldlist = NULL;
    }


 COMPLETION:
  HE5_UNLOCK;
  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAsetdimscale                                                |
|                                                                             |
|  DESCRIPTION: Defines a dimension scale for a fields dimension in a ZA      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char                field name                              |
|  dimname        char                Dimension name                          |
|  dimsize        hsize_t             Dimemsion size value                    |
|  numbertype_in  hid_t               dim scale data type such as DFNT_INT32, |
|                                     DFNT_FLOAT64, DFNT_FLOAT32, etc.        |
|  data           void                data buffer for write                   |
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
|  May 10    Abe Taaheri  Original Programmer                                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAsetdimscale(hid_t zaID, char *fieldname, char *dimname,
		  const hsize_t dimsize, hid_t numbertype_in, void * data)
{
  herr_t         status;
  hid_t          ntype;
  hsize_t        dims[HE5_DTSETRANKMAX];
  int            rankSDS;
  hid_t          field_ntype[1] = {FAIL};       /* number types       */ 
  char           dimlist[HE5_HDFE_DIMBUFSIZE];
  char           maxdimlist[HE5_HDFE_DIMBUFSIZE];

  hid_t          fid    = FAIL; 	       /* HDF-EOS file ID     */
  hid_t          gid    = FAIL;	               /* "HDFEOS"  group ID  */
  hid_t          did;
  hid_t          dsid;
  hid_t          fspace   = FAIL;              /* File dataspace ID   */
  unsigned int   Dimindex;
  hid_t          dsplist  = FAIL;              /* Property list ID     */
  H5D_layout_t   layout   = H5D_LAYOUT_ERROR;  /* Type of storage      */
  hid_t          data_space  = FAIL;           /* dataspace ID         */
  long           idx         = FAIL;           /* Grid index           */
  char          *errbuf      = (char *)NULL;   /* buffer for error message*/
  void          *data_buff;
  int            i, found;
  int            rankds;
  hsize_t        dimsds[1];

  HE5_LOCK;
  CHECKNAME(fieldname);
  CHECKNAME(dimname);
  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer ", __FILE__, __LINE__);
      return(FAIL);
    }
    
  /* Convert HDF-EOS5 datatype to HDF5 datatype */
  if (
      H5Tequal(numbertype_in, H5T_NATIVE_CHAR)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_SCHAR)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_UCHAR)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_SHORT)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_USHORT) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_INT)    == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_UINT)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LONG)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_ULONG)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LLONG)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_ULLONG) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_FLOAT)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_DOUBLE) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LDOUBLE)== SUCCEED ||
      H5Tequal(numbertype_in, H5T_C_S1) == SUCCEED)
    {
      ntype = numbertype_in;
    }
  else
    {
      ntype = HE5_EHconvdatatype((int) numbertype_in);
    }

  if(ntype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 type data type ID for dimscale\n");
      H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /*
**********************************************************
*  Check for proper ZA ID and return HDF-EOS file ID,    * 
*        "HDFEOS" group ID and ZA index                  *  
**********************************************************
*/
  status = HE5_ZAchkzaid(zaID, "HE5_ZAsetdimscale", &fid, &gid, &idx);
  if(status == FAIL)
    {
      sprintf(errbuf,"Checking for ZA ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  if (status == 0)
    {
      /* Check that field exists */
      /* ----------------------- */
      status = HE5_ZAinfo(zaID, fieldname, &rankSDS, dims, field_ntype, dimlist, maxdimlist);

      if (status != 0)
	{
	  sprintf(errbuf, "Fieldname \"%s\" does not exist.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__,  H5E_ARGS, H5E_BADRANGE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}
      
      data_buff = data;
      
      /* Loop through all datasets in ZA */
      /* ------------------------------ */
	for (i = 0; i < HE5_ZAXZa[idx].nDFLD; i++)
	  {
	    /* Get dataset name */
	    if( strcmp(fieldname, HE5_ZAXZa[ idx ].ddataset[ i ].name) == 0 )
	      {
		found = 1; 
		did = HE5_ZAXZa[ idx ].ddataset[ i ].ID;
		break;
	      } 
	  }
	
	if(found != 1) /* did not find fieldname */
	  {
	    sprintf(errbuf, "Fieldname \"%s\" does not exist.\n", fieldname);
	    H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__,  H5E_ARGS, H5E_BADRANGE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    return(FAIL);
	  }
	/* Dimension index */

	Dimindex = FAIL;
	Dimindex = HE5_EHstrwithin(dimname, dimlist, ',');
	
	if (Dimindex == FAIL) /* dimension not found */
	  {
	    status = FAIL;
	    sprintf( errbuf, "Dimname \"%s\" does not exist for field \"%s\".\n", 
		     dimname, fieldname );
	    H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    free(errbuf);
	    return(status);
	  }
	
	/* If dimension found ... */
	/* Found the dimname; Use its dimid to set Dimension Scale*/
	
	/* See if dimscale dataset already exist */
	
	dsid = H5Dopen(HE5_ZAXZa[idx].za_id, dimname);
	if(dsid < 0)
	  {
	    /* create Dim Scale dataset */
	    /* Try to open the " group */
	    /* =================================== */
	    
	    dsplist = H5Pcreate(H5P_DATASET_CREATE);
	    if(dsplist == FAIL)
	      {
		sprintf(errbuf,"Cannot create a new property list.\n");  
		H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		free(errbuf);
		return(FAIL);
	      }
	    /* Get layout information */
	    /* ---------------------- */		
	    layout = H5Pget_layout(dsplist);
	    if(layout == H5D_LAYOUT_ERROR)
	      {
		sprintf(errbuf,"Cannot get layout information for the dataset.\n");  
		H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		free(errbuf);
		return(FAIL);
	      }

	    rankds = 1;
	    dimsds[0] = dims[Dimindex];

	    data_space = H5Screate_simple(rankds, dimsds, NULL);
	    if(data_space == FAIL)
	      {
		sprintf(errbuf,"Cannot create dataspace for the dimension scale dataset.\n");  
		H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		free(errbuf);
		return(FAIL);
	      }
	    
	    dsid = H5Dcreate(HE5_ZAXZa[idx].za_id,dimname,ntype, data_space, dsplist);
	    
	    if( dsid == FAIL )
	      {
		sprintf(errbuf, "Cannot create dataset for Dimension Scale \"%s\" field. \n", dimname);
		H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		free(errbuf);
		return(FAIL);
	      }

	    /* write data_buff to it */

	    /* Get the field space ID */
	    /* --------------------- */
	    fspace = H5Dget_space(dsid);
	    if ( fspace == FAIL)
	      {
		sprintf(errbuf, "Cannot get the file data space ID.\n");
		H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		free(errbuf);
		return(FAIL);
	      }

	    status = H5Dwrite(dsid, ntype, data_space, fspace,  H5P_DEFAULT, data_buff);
	    if( status == FAIL )
	      {
		sprintf(errbuf,"Cannot write data to the dataset.\n");
		H5Epush(__FILE__, "HE5_ZAsetdimscale", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		free(errbuf);
		return(FAIL);
	      }  
	  }

	/* Dataset exist. See if it is Dimension Scale already. If not make it */
	if ((H5DSis_scale(dsid)) <= 0 )
	  {
	    status = H5DSset_scale(dsid, dimname);
	    if( status < 0)
	      {
		status = -1;
		free(errbuf);
		goto COMPLETION;
	      }
	    
	    if(H5DSattach_scale(did, dsid, Dimindex) < 0)
	      {
		status = -1;
		free(errbuf);
		goto COMPLETION;
	      }
	  }
	else
	  {
	    if(H5DSattach_scale(did, dsid, Dimindex) < 0)
	      {
		status = -1;
		free(errbuf);
		goto COMPLETION;
	      }
	  }
   
	if( H5Dclose(dsid) < 0)
	  {
	    status = -1;
	    free(errbuf);
	    goto COMPLETION;
	  }
    }
  free(errbuf);
 COMPLETION:  
  HE5_UNLOCK;
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAgetdimscale                                                |
|                                                                             |
|  DESCRIPTION: Get dimension scale for a dimension of a field in a ZA        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         long                return databuffsize  SUCCEED, (-1) FAIL |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char                field name                              |
|  dimname        char                dim name                                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  dimsize        hsize_t             dimension size                          |
|  ntype          hid_t               dtata type of dimension scale           |
|  databuff       void                data buffer for read                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 10    Abe Taaheri  Original Programmer                                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAgetdimscale(hid_t zaID, char *fieldname, char *dimname, 
		  hsize_t *dimsize,hid_t *ntype, void * databuff)
{
  int             i,j;		                   /* Loop index             */
  long            status;	                   /* routine return status  */
  long            idx    = FAIL;                   /* ZA index               */
  hid_t           fid    = FAIL; 	           /* HDF-EOS file ID        */
  hid_t           gid    = FAIL;	           /* "HDFEOS"  group ID     */
  hid_t           did    = FAIL;                   /* dataset ID             */
  hid_t           dsid   = FAIL;                   /* dimscale dataset ID    */
  hid_t           dspace = FAIL;                   /* data space ID          */
  hid_t           mspace = FAIL;                   /* memory data space ID   */
  hid_t           dtype  = FAIL;                   /* data type ID           */
  hid_t           mtype  = FAIL;                   /* memory data type ID    */
  H5T_class_t     classid = H5T_NO_CLASS;          /* Data type class ID     */
  hsize_t         dimsds[HE5_DTSETRANKMAX];        /* Field dimensions       */
  char           *errbuf = (char *)NULL;           /* buff for error message */
  long            size   = 0;                      /* data buffer size(bytes)*/
  int             rankds;
  size_t          sd_type_size = 0;	           /* Size of a data type    */
  unsigned int    Dimindex;
  int             rankSDS;
  hid_t           *field_ntype = (hid_t *)NULL;    /* number types           */
  char            dimlist[HE5_HDFE_DIMBUFSIZE];
  char            maxdimlist[HE5_HDFE_DIMBUFSIZE];
  hsize_t         dims[HE5_DTSETRANKMAX];
  int             found = 0;
  htri_t          str_is_variable; /* boolean: TRUE if string is variable 
				      lengeth FALSE if string is fixed length
				      -1 if error in H5Tis_variavle_str() */
  HE5_LOCK;
  CHECKNAME(fieldname);
  CHECKNAME(dimname);


  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_FILE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /*
**********************************************************
*  Check for proper ZA ID and return HDF-EOS file ID,    * 
*        "HDFEOS" group ID and ZA index                  *  
**********************************************************
*/
  status = HE5_ZAchkzaid(zaID, "HE5_ZAgetdimscale", &fid, &gid, &idx);
  if(status == FAIL)
    {
      sprintf(errbuf,"Checking for ZA ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* See if dimscale dataset exist */

  dsid = H5Dopen(HE5_ZAXZa[idx].za_id, dimname);
  if(dsid < 0)
    {
      status = FAIL;
      sprintf( errbuf, "Dimension scale dataset \"%s\" does not exist.\n", 
	       dimname);
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(status);
    }

  /* Loop through all datasets in ZA */
  /* ------------------------------ */
  for (i = 0; i < HE5_ZAXZa[idx].nDFLD; i++)
    {
      /* Get dataset name */
      if( strcmp(fieldname, HE5_ZAXZa[ idx ].ddataset[ i ].name) == 0 )
	{
	  found = 1; 
	  did = HE5_ZAXZa[ idx ].ddataset[ i ].ID;
	  break;
	}
    }
  if(found != 1) /* did not find fieldname */
    {
      sprintf(errbuf, "Fieldname \"%s\" does not exist.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__,  H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Check that field exists */
  /* ----------------------- */
  status = HE5_ZAinfo(zaID, fieldname, &rankSDS, dims, field_ntype, dimlist, maxdimlist);
  
  if (status != 0)
    {
      sprintf(errbuf, "Fieldname \"%s\" does not exist.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__,  H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* find the index of the dimension in the field */
  Dimindex = FAIL;
  Dimindex = HE5_EHstrwithin(dimname, dimlist, ',');
  if (Dimindex == FAIL) /* dimension not found */
    {
      status = FAIL;
      sprintf( errbuf, "Dimname \"%s\" does not exist for field \"%s\".\n", 
	       dimname, fieldname );
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(status);
    }

  /* check that dimname is dimension scale name for the field dimname */
  status = H5DSis_attached(did, dsid, Dimindex);

  if( status < 0)
    {
      status = FAIL;
      sprintf( errbuf, "Dimname \"%s\" is not dimension scale for a dimension in the field \"%s\".\n", dimname, fieldname );
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(status);
    }

  /* OK. dsid is a dimension scale for did. It is time to read Dimension 
     Scale dataset for output */

  /* get size first ... */
  /* ------------------ */
  dtype = H5Dget_type(dsid);
  if ( dtype == FAIL )
    {
      size = 0;
      sprintf(errbuf, "Cannot get data type ID.\n");
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Get data type size (bytes) */
  /* -------------------------- */   
  sd_type_size = H5Tget_size(dtype);
  if ( sd_type_size == 0 )
    {
      sprintf(errbuf, "Cannot retrieve data type size.\n");
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Get data space ID */
  /* ----------------- */
  dspace = H5Dget_space( dsid );
  if (dspace == FAIL)
    {
      sprintf(errbuf, "Cannot get the dataspace ID for the \"%s\" dataset.\n", dimname);
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  /* Get dataset rank */
  /* ---------------- */
  rankds = H5Sget_simple_extent_ndims(dspace);
  if ( rankds == FAIL )
    {
      sprintf(errbuf, "Cannot get the dataset rank for the \"%s\" dataset.\n", dimname);
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  /* Get dataset dimension sizes */
  /* --------------------------- */
  status = H5Sget_simple_extent_dims(dspace,dimsds, NULL);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot get the dataset dimension sizes for the \"%s\" dataset.\n", dimname);
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(status);
    }

  /*----------------------------------------------------------------------*/

  /* Get data class ID */
  /* ----------------- */
  classid       = H5Tget_class(dtype);
  if (classid == H5T_NO_CLASS)
    {
      sprintf(errbuf, "Cannot get the data type class ID for \"%s\" dataset.",
	      dimname);
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, 
	      H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      if (errbuf  != NULL) free(errbuf);
      return(FAIL);
    }
  
  if (classid == H5T_STRING)
    {
      /* HE5T_CHARSTRING has variable length for data fields */
      str_is_variable = H5Tis_variable_str(dtype);
      if(str_is_variable == TRUE)
	{
	  *ntype = HE5T_CHARSTRING;
	}
      else if(str_is_variable == FALSE)
	{
	  *ntype = HE5T_NATIVE_CHAR;
	}
      else
	{
	  sprintf(errbuf, "Failed to see if string field is varaible or fixed length for the \"%s\" field.\n",dimname);
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, 
		  H5E_NOTFOUND, errbuf);
	  
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (errbuf  != NULL) free(errbuf);
	  return(FAIL);
	}
    }
  else
    {
      *ntype = HE5_EHdtype2numtype(dtype);
      if (*ntype == FAIL)
	{
	  sprintf(errbuf, "Cannot get the number type for \"%s\" dataset.", 
		  dimname);
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, 
		  H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (errbuf  != NULL) free(errbuf);
	  return(FAIL);
	}
    }
  /*----------------------------------------------------------------------*/

  status = H5Tclose(dtype);
  if ( status == FAIL )
    {
      sprintf(errbuf,"Cannot release the datatype ID.\n");
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  /* Release the dataspace ID */
  /* ------------------------ */
  status = H5Sclose(dspace);
  if ( status == FAIL)
    {
      sprintf(errbuf, "Cannot release the dataset ID for the \"%s\" dataset.\n", dimname);
      H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(status);
    }

  size = (long)dimsds[ 0 ];
  *dimsize = (hsize_t)size;

  for (j = 1; j < rankds; j++)
    size *= (long)dimsds[j];
 
  size *= (long)sd_type_size;

  /* If data values are requested ... */
  /* -------------------------------- */
  if (databuff != NULL)
    {
      /* Create a data space in memory */
      /* ----------------------------- */ 		
      mspace = H5Screate_simple(rankds, (const hsize_t *)dimsds, NULL);
      if ( mspace == FAIL )
	{
	  sprintf(errbuf,"Cannot create the data space.\n");
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATASPACE, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}

      dspace = H5Dget_space(dsid );
      if (dspace == FAIL)
	{
	  sprintf(errbuf, "Cannot get the dataspace ID for the \"%s\" dataset.\n", dimname);
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}
		
      dtype = H5Dget_type(dsid);
      if ( dtype == FAIL )
	{
	  sprintf(errbuf,"Cannot get the datatype ID.\n");
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}

      /* Get the memory data type ID */
      /* --------------------------- */
      mtype = HE5_EHdtype2mtype(dtype);
      if ( mtype == FAIL )
	{
	  sprintf(errbuf,"Cannot get the memory data type.\n");
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}

      /* Read the selected points into the buffer */
      /* ---------------------------------------  */
      status = H5Dread(dsid, mtype, mspace, dspace, H5P_DEFAULT, databuff);
      if ( status == FAIL )
	{
	  sprintf(errbuf,"Cannot read out the data from the dataset.\n");
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}
				
      status = H5Tclose(dtype);
      if ( status == FAIL )
	{
	  sprintf(errbuf,"Cannot release the datatype ID.\n");
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}
		
      status = H5Sclose(mspace);
      if ( status == FAIL )
	{
	  sprintf(errbuf,"Cannot release the memory data space ID.\n");
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}
		
      status = H5Sclose(dspace);
      if ( status == FAIL )
	{
	  sprintf(errbuf,"Cannot release the file data space ID.\n");
	  H5Epush(__FILE__, "HE5_ZAgetdimscale", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
	}
    }

  free(errbuf);
 COMPLETION:
  HE5_UNLOCK;
  return (size);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAreaddscaleattr                                             |
|                                                                             |
|  DESCRIPTION: Reads attribute associated with a dimension scale field       |
|               from a ZA.                                                    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t   None        HDF-EOS type ZA  ID                     |
|  fieldname      char                field name                              |
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
|  Jun 10   Abe Taaheri   Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t  
HE5_ZAreaddscaleattr(hid_t zaID, const char *fieldname, const char *attrname, void *datbuf)
{
  herr_t     status        = FAIL;/* routine return status variable */

  hid_t      ntype         = FAIL;/* hdf5 type data type ID         */
  hid_t      fid           = FAIL;/* HDF-EOS file ID                */
  hid_t      gid           = FAIL;/* "HDFEOS" group ID              */
  hid_t      fieldID       = FAIL;/* Field-related dataset ID       */
     
  long       idx           = FAIL;/* ZA index                       */
     
  hsize_t    count[]={0};	  /* array with the number of elements */

  char       errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer   */


  HE5_LOCK;
  CHECKPOINTER(fieldname);
  CHECKPOINTER(attrname);

  /*
**********************************************************
*  Check for proper ZA ID and return HDF-EOS file ID,    * 
*        "HDFEOS" group ID and ZA index                  *  
**********************************************************
*/
  status = HE5_ZAchkzaid(zaID, "HE5_ZAreaddscaleattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get field ID */
      /* ------------ */
      fieldID = H5Dopen(HE5_ZAXZa[idx].za_id, fieldname);
      if(fieldID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAreaddscaleattr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
		
      /* Call EHattr to perform I/O */
      /* -------------------------- */
      status = HE5_EHattr(fieldID, attrname, ntype, count, "r", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot read Attribute \"%s\" associated wth the \"%s\" field.\n", attrname,fieldname);
	  H5Epush(__FILE__, "HE5_ZAreaddscaleattr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
        
      /* Release dataset ID */
      /* ------------------ */
      status = H5Dclose(fieldID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAreaddscaleattr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION:  HE5_ZAwritedscaleattr                                           |
|                                                                             |
|  DESCRIPTION: Writes/updates attribute associated with a dimension scale    |
|               field in a ZA.                                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char*               dimension scale SDS name                |
|  attrname       char*               attribute name                          |
|  numtype        hid_t               attribute datatype ID                   |
|  count[]        hsize_t             Number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 10   Abe Taaheri   Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAwritedscaleattr(hid_t zaID, const char *fieldname, const char *attrname, hid_t numtype, hsize_t  count[],  void *datbuf)
{
  herr_t     status        = FAIL;/* routine return status variable */
  
  hid_t      fid           = FAIL;/* HDF-EOS file ID                */
  hid_t      gid           = FAIL;/* "HDFEOS" group ID              */
  hid_t      fieldID       = FAIL;/* Field-related dataset ID       */
  
  long       idx           = FAIL;/* ZA index                       */
  
  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer    */
  hid_t      ntype;
  
  HE5_LOCK;
  CHECKPOINTER(fieldname);
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
  
  /*
**********************************************************
*  Check for proper ZA ID and return HDF-EOS file ID,    * 
*        "HDFEOS" group ID and  ZA index                 *  
**********************************************************
*/
  status = HE5_ZAchkzaid(zaID, "HE5_ZAwritedscaleattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      
      /* Get field ID */
      /* ------------ */
      fieldID = H5Dopen(HE5_ZAXZa[idx].za_id, fieldname);
      if(fieldID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAwritedscaleattr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      
      /* Call EHattr to perform I/O */
      /* -------------------------- */
      status = HE5_EHattr(fieldID, attrname, ntype, count,"w", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write Attribute \"%s\" for the \"%s\" field.\n", attrname, fieldname);
	  H5Epush(__FILE__, "HE5_ZAwritedscaleattr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
      
      status = H5Dclose(fieldID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAwritedscaleattr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_ZAdscaleattrinfo                                             |
|                                                                             |
|  DESCRIPTION: Retrieves information about dimension scale attribute         |
|               (attribute associated with a specified dimension scale field) |
|                in a ZA.                                                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t     None      HDF-EOS type ZA  ID                     |
|  fieldname      char*               field name                              |
|  attrname       char*               attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          hid_t               attribute data type ID                  |
|  count          hsize_t             Number of attribute elements            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 10   Abe Taaheri   Original Development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_ZAdscaleattrinfo(hid_t zaID, const char *fieldname, const char *attrname, hid_t *ntype, hsize_t *count)
{
  herr_t     status        = FAIL;/* routine return status variable */
  
  hid_t      fid           = FAIL;/* HDF-EOS file ID                */
  hid_t      gid           = FAIL;/* "HDFEOS" group ID              */
  hid_t      fieldID       = FAIL;/* Field-related dataset ID       */
  
  long       idx           = FAIL;/* ZA index                       */
  
  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer    */
  
  CHECKPOINTER(fieldname);
  CHECKPOINTER(attrname);
  
  /*
**********************************************************
*  Check for proper ZA ID and return HDF-EOS file ID,    * 
*        "HDFEOS" group ID and ZA index                  *  
**********************************************************
*/
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdscaleattrinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get field ID */
      /* ------------ */
      fieldID = H5Dopen(HE5_ZAXZa[idx].za_id, fieldname);
      if(fieldID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAdscaleattrinfo", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      
      /* Call EHattrinfo */
      /* --------------- */
      if(strcmp(attrname,"REFERENCE_LIST") !=0)/* do not need ntype for 
						  internal dimension
						  scale attribute */
	{
	  status = HE5_EHattrinfo(fieldID, attrname, ntype, count);
	  if(status == FAIL)
	    {
	      sprintf(errbuf,"Cannot retrieve information about Attribute \"%s\" associated with the \"%s\" field.\n", attrname, fieldname);
	      H5Epush(__FILE__, "HE5_ZAdscaleattrinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
	}
      else
	{
	  status = -1;
	  *ntype = -1;
	  *count = 0;
	  sprintf(errbuf,"Will not retrieve information about internal Attribute \"%s\" associated with the dimension scale \"%s\" .\n", attrname, fieldname);
	  H5Epush(__FILE__, "HE5_ZAdscaleattrinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Dclose(fieldID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAdscaleattrinfo", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_ZAinqdscaleattrs                                             |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list associated with a specified dimension scale   |
|                in a ZA.                                                     |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                number of attributes (-1 if fails)      |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t   None        HDF-EOS type ZA  ID                     |
|  fieldname      char                dimension scale name                    |
|  attrnames      char                attribute name(s)                       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  strbufsize     long                String length of attribute (in bytes)   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|  None                                                                       |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  June 10   Abe Taaheri  Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqdscaleattrs(hid_t zaID, const char *fieldname, char *attrnames, long *strbufsize)
{
  long       nattr         =  0;  /* Number of attributes (return)  */
  long       idx           = FAIL;/* Grid index                     */    
  
  herr_t     status        = FAIL;/* routine return status variable */
  
  hid_t      fid           = FAIL;/* HDF-EOS file ID                */
  hid_t      gid           = FAIL;/* "HDFEOS" group ID              */
  
  char       *dtsname      = NULL;/* Buffer for dataset name        */
  char       errbuf[HE5_HDFE_ERRBUFSIZE];
  
  CHECKPOINTER(fieldname);
  
  /*
**********************************************************
*  Check for proper ZA ID and return HDF-EOS file ID,  * 
*        "HDFEOS" group ID and Grid index                *  
**********************************************************
*/
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinqdscaleattrs", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf,"Checking for ZA ID failed.\n" );
      H5Epush(__FILE__, "HE5_ZAinqdscaleattrs", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  dtsname = (char *) calloc(HE5_HDFE_NAMBUFSIZE, 1);
  if( dtsname == NULL)
    {
      sprintf(errbuf, "Can not allocate memory.\n");
      H5Epush(__FILE__, "HE5_ZAinqdscaleattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
	
  strcpy(dtsname,"/HDFEOS/ZAS/");
  strcat(dtsname, HE5_ZAXZa[idx].zaname);
  strcat(dtsname,"/");
  strcat(dtsname,fieldname);

  nattr = HE5_EHattrcat(fid, dtsname, attrnames, strbufsize);
	
  if (dtsname != NULL) free(dtsname);
    
 COMPLETION:
  return(nattr);
	
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdiminfo                                                    |
|                                                                             |
|  DESCRIPTION: Returns size in bytes of named dimension                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  size           hsize_t             size of dimension                       |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  dimname        char*               dimension name                          |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hsize_t
HE5_ZAdiminfo(hid_t zaID, char *dimname)
{
  hsize_t       size                = 0;   /* Dimension size (return value) */
 
  herr_t        status              = FAIL;/* status variable               */
 
  hid_t         fid                 = FAIL;/* HDF-EOS file ID               */
  hid_t         gid                 = FAIL;/* "HDFEOS" group ID             */
 
  long          idx                 = FAIL;/* za index                      */
 
  char          *metabuf=(char *)NULL;      /* Ptr to StrucMetadata (SM)    */
  char          *metaptrs[2]={NULL,NULL};   /* Ptrs to the start/end of SM  */
  char          utlstr[HE5_HDFE_UTLBUFSIZE];/* Utility string buffer        */
  char          errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer         */
 
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdiminfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get pointers to "Dimension" section within SM */
      /* --------------------------------------------- */
      metabuf = (char *)HE5_EHmetagroup(fid, HE5_ZAXZa[idx].zaname, "z", "Dimension", metaptrs);
 
      /* Search for dimension name (surrounded by quotes) */
      /* ------------------------------------------------ */
      sprintf(utlstr, "%s%s%s", "\"", dimname, "\"\n");
      metaptrs[0] = strstr(metaptrs[0], utlstr);
 
      /*
       * If dimension found within za structure then get dimension value
       */
      if (metaptrs[0] < metaptrs[1] && metaptrs[0] != NULL)
        {
	  /* Set "end" ptr at the end of dimension definition entry */
	  /* ------------------------------------------------------ */
	  metaptrs[1] = strstr(metaptrs[0], "\t\t\tEND_OBJECT");
 
	  /* Get the value of "Size" entry */
	  /* ----------------------------- */
	  status = HE5_EHgetmetavalue(metaptrs, "Size", utlstr);
	  if (status == SUCCEED)
	    {
	      size = atol(utlstr);
	      if (size == 0)
		{
		  sprintf(errbuf, "Returning zero \"Size\" value from Metadata.\n");
		  H5Epush(__FILE__, "HE5_ZAdiminfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
	    }
	  else
            {
	      sprintf(errbuf, "The \"Size\" string not found in metadata. \n");
	      H5Epush(__FILE__, "HE5_ZAdiminfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	    }
        }
      else
        {
	  sprintf(errbuf, "The Dimension \"%s\" not found in metadata. \n", dimname);
	  H5Epush(__FILE__, "HE5_ZAdiminfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
        }
 
      if (metabuf != NULL) free(metabuf);
    }
 
  return(size);
}
 


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAcompinfo                                                   |
|                                                                             |
|  DESCRIPTION: Retrieves compression information about a field               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        Return status  (0) SUCCEED, (-1) FAIL   |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fldname        char*               field name                              |
|  compcode       int*                compression code                        |
|  compparm       int                 compression parameters                  |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_ZAcompinfo(hid_t zaID, char *fldname, int *compcode, int compparm[])
{
  herr_t          status   = FAIL;                   /* Return status variable      */
  herr_t          statmeta = FAIL;                   /* Status variable             */

  int             i;			             /* Loop Index                  */
  int             nameflag = FAIL;                   /* Name flag (0-alias,1-actual)*/
  int             fldgroup = FAIL;                   /* Field group flag            */

  hid_t           fid      = FAIL;                   /* HDF-EOS file ID             */
  hid_t           gid      = FAIL;                   /* "HDFEOS" group ID           */

  long            idx      = FAIL;                   /* za index                    */

  char            fieldname[HE5_HDFE_NAMBUFSIZE];    /* Field name buffer           */ 
  char            fldactualname[HE5_HDFE_NAMBUFSIZE];/* Actual name of a field      */ 
  char            *metabuf = (char *)NULL;           /* Pntr to StrucMetadata (SM)  */
  char            *metaptrs[2]={NULL,NULL};          /* Ptrs to the start/end of SM */
  char            *utlstr = (char *)NULL;            /* Utility string buffer       */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];       /* Error message buffer        */
  char            *HDFcomp[18] = {"HE5_HDFE_COMP_NONE", "HE5_HDFE_COMP_RLE", "HE5_HDFE_COMP_NBIT", "HE5_HDFE_COMP_SKPHUFF", "HE5_HDFE_COMP_DEFLATE", "HE5_HDFE_COMP_SZIP_CHIP", "HE5_HDFE_COMP_SZIP_K13", "HE5_HDFE_COMP_SZIP_EC", "HE5_HDFE_COMP_SZIP_NN", "HE5_HDFE_COMP_SZIP_K13orEC", "HE5_HDFE_COMP_SZIP_K13orNN", "HE5_HDFE_COMP_SHUF_DEFLATE", "HE5_HDFE_COMP_SHUF_SZIP_CHIP", "HE5_HDFE_COMP_SHUF_SZIP_K13", "HE5_HDFE_COMP_SHUF_SZIP_EC", "HE5_HDFE_COMP_SHUF_SZIP_NN", "HE5_HDFE_COMP_SHUF_SZIP_K13orEC", "HE5_HDFE_COMP_SHUF_SZIP_K13orNN"};  /* Compression method Codes    */


  CHECKPOINTER(fldname);
  
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAcompinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Allocate space for utility string */
      /* --------------------------------- */
      utlstr = (char *)calloc(HE5_HDFE_DIMBUFSIZE, sizeof(char));
      if ( utlstr == NULL )
	{
	  sprintf( errbuf,"Cannot allocate memory for utility string.\n");
	  H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  
	  return(FAIL);
	}

      /* Call HE5_ZAfldnameinfo() to get actual field name */
      /* ------------------------------------------------- */
      nameflag = HE5_ZAfldnameinfo(zaID, fldname, fldactualname);
      if ( nameflag == FAIL )
	{
	  sprintf(errbuf, "Cannot get the actual name of the field.\n");
	  H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (utlstr != NULL) free(utlstr);
	  return(FAIL);
	}
  
      /* If fieldname is alias, then change it to the actual name */
      /* -------------------------------------------------------- */
      if (nameflag == FALSE)
	{
	  strcpy(fieldname,"");
	  strcpy(fieldname,fldactualname);
	}

      if (nameflag == TRUE)
	{
	  strcpy(fieldname,"");
	  strcpy(fieldname,fldname);
	}

      /* Get the field group code */
      /* ------------------------ */
      fldgroup = HE5_ZAfldsrch(zaID,fieldname,NULL,NULL,NULL,NULL);
      if (fldgroup == HE5_HDFE_DATAGROUP)
	{
	  /* Get pointers to "DataField" section within SM */
	  /* --------------------------------------------- */
	  metabuf = (char *)HE5_EHmetagroup(fid, HE5_ZAXZa[idx].zaname, "z", "DataField", metaptrs);
		  
	  /* Search for field */
	  /* ---------------- */
	  sprintf(utlstr, "%s%s%s", "\"", fieldname, "\"\n");
	  metaptrs[0] = strstr(metaptrs[0], utlstr);
	  
	}
      else
	{
	  sprintf(errbuf , "Cannot get the field group code.\n");
	  H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (metabuf != NULL) free(metabuf);
	  if (utlstr != NULL) free(utlstr);
	  return(FAIL);
	}

	  
      /* If field found and user wants compression code ... */
      /* -------------------------------------------------- */
      if (metaptrs[0] < metaptrs[1] && metaptrs[0] != NULL)
	{
	  if (compcode != NULL)
	    {
	      /* Set endptr at end of field's definition entry */
	      /* --------------------------------------------- */
	      metaptrs[1] = strstr(metaptrs[0], "\t\t\tEND_OBJECT");
			  
	      /* Get compression type */
	      /* -------------------- */
	      statmeta = HE5_EHgetmetavalue(metaptrs, "CompressionType", utlstr);
			  
	      /*
	       * Default is no compression if "CompressionType" string not
	       * in metadata
	       */
	      *compcode = HE5_HDFE_COMP_NONE;
			  
	      /* If compression code is found ... */
	      /* -------------------------------- */
	      if (statmeta == SUCCEED)
		{
		  /* Loop through compression types until match */
		  /* ------------------------------------------ */
		  for (i = 0; i < 11; i++)
		    {
		      if (strcmp(utlstr, HDFcomp[i]) == 0)
			{
			  *compcode = i;
			  break;
			}
		    }
		}
	    }
			  
	  /* If user wants compression parameters ... */
	  /* ---------------------------------------- */
	  if (compparm != NULL && compcode != NULL)
	    {
	      /* Initialize to zero */
	      /* ------------------ */
	      for (i = 0; i < 5; i++)
		compparm[i] = 0;
			  
	      /* Get compression parameters */
	      /* -------------------------- */
	      if (*compcode == HE5_HDFE_COMP_NBIT)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "CompressionParams", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "(%d,%d,%d,%d)", &compparm[0], &compparm[1], &compparm[2], &compparm[3]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"\"CompressionParams\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_DEFLATE)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "DeflateLevel", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"DeflateLevel\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SZIP_CHIP)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SZIP_K13)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SZIP_EC)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SZIP_NN)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SZIP_K13orEC)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SZIP_K13orNN)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n"
			       );
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SHUF_DEFLATE)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "DeflateLevel", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"DeflateLevel\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SHUF_SZIP_CHIP)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SHUF_SZIP_K13)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SHUF_SZIP_EC)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SHUF_SZIP_NN)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SHUF_SZIP_K13orEC)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}
	      else if (*compcode == HE5_HDFE_COMP_SHUF_SZIP_K13orNN)
		{
		  statmeta = HE5_EHgetmetavalue(metaptrs, "BlockSize", utlstr);
		  if (statmeta == SUCCEED)
		    {
		      sscanf(utlstr, "%d", &compparm[0]);
		    }
		  else
		    {
		      status = FAIL;
		      sprintf( errbuf,"The \"BlockSize\" string not found in metadata.\n");
		      H5Epush(__FILE__, "HE5_ZAcompinfo", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		    }
		}

	    }
		  
	}


      if (metabuf != NULL) free(metabuf);
      if (utlstr != NULL) free(utlstr);
    }

 COMPLETION:
  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAfldnameinfo                                                |
|                                                                             |
|  DESCRIPTION: Retrieves actual name of the field                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nameFlag       int     None        return (1) actual name, (0) alias,      |
|                                                (-1) FAIL                    |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char*               name of field                           |
|                                                                             |
|  OUTPUTS:                                                                   |
|  actualname     char*               field actual name                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static int
HE5_ZAfldnameinfo(int zaID, char *fieldname, char *actualname)
{
  int         nameFlag       = FAIL;/* Return value of flag            */
  int         length         = 0;   /* String length                   */
  int         fldgroup       = FAIL;/* Field group code                */

  hid_t       fid            = FAIL;/* HDF-EOS file ID                 */
  hid_t       gid            = FAIL;/* "HDFEOS" group ID               */
  hid_t       groupID        = FAIL;/* data field group ID             */

  long        idx            = FAIL;/* za index                        */

  H5G_stat_t  *statbuf = (H5G_stat_t *)NULL;/* buffer for link info    */

  herr_t      status         = FAIL;/* routine return status variable  */

  char        *namebuf       = NULL;/* Buffer for an actual field name */
  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer      */
   
  CHECKPOINTER(fieldname); 
  
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAfldnameinfo", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAfldnameinfo", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  statbuf = (H5G_stat_t *)calloc(1, sizeof(H5G_stat_t ));

  /* Try to get information about specified field */
  /* -------------------------------------------- */
  H5E_BEGIN_TRY {
    groupID  = HE5_ZAXZa[idx].data_id;
    fldgroup = HE5_HDFE_DATAGROUP;
    status   = H5Gget_objinfo(groupID, fieldname, 0, statbuf);
  }
  H5E_END_TRY;
  if (status == FAIL)
    {
      nameFlag = FAIL;
      sprintf(errbuf, "Cannot find \"%s\" field.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAfldnameinfo", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(statbuf);
      return(nameFlag);
    }
  
  /* If the field name is an alias */
  /* ----------------------------- */
  if (statbuf->type == H5G_LINK)
    nameFlag = FALSE;
  else
    nameFlag = TRUE;

  if (nameFlag == FALSE)
    {	
      if (actualname != NULL)
	{
	  status = HE5_ZAaliasinfo(zaID, fldgroup, fieldname, &length, NULL);
	  if ( status == FAIL )
	    {
	      sprintf(errbuf, "Cannot get information about alias \"%s\".\n", fieldname);
	      H5Epush(__FILE__, "HE5_ZAfldnameinfo", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
		  
	  namebuf = (char *)calloc(length, sizeof(char));
	  if ( namebuf == NULL )
	    {
	      sprintf(errbuf, "Cannot allocate memory for namebuf.\n");
	      H5Epush(__FILE__, "HE5_ZAfldnameinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
		 
	  /* Retrieve information about alias */
	  /* -------------------------------- */
	  status = HE5_ZAaliasinfo(zaID, fldgroup, fieldname, &length, namebuf);
	  if ( status == FAIL )
	    {
	      sprintf(errbuf, "Cannot get information about alias \"%s\".\n", fieldname);
	      H5Epush(__FILE__, "HE5_ZAfldnameinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(namebuf);
	      return(FAIL);
	    }
		  
	  strcpy(actualname,"");
	  strcpy(actualname,namebuf);  
	  free(namebuf);
	}
    }
  
  if (nameFlag == TRUE)
    {
      if (actualname != NULL)
	{
	  strcpy(actualname,"");
	  strcpy(actualname, fieldname);
	}
    }
  
  free(statbuf);

 COMPLETION:  
  return(nameFlag);
} 



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAfldinfo                                                    |
|                                                                             |
|  DESCRIPTION: Retrieves information about specified ZA structure            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  datafield      char*               data field                              |
|  za_name        char*               name of a specific ZA data set          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  rank           int*                rank of ZA data set                     |
|  dims           hsize_t             array of dimension sizes                |
|  numbertype     hid_t               number type                             |
|  dimlist        char*               dimension list                          |
|  maxdimlist     char*               maximum dimension list                  |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Oct 03    S.ZHAO       Added the H5Tclose(datatype) call.                  |
|  Jan 04    S.Zhao       Modified to enable a character string dataset.      |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t
HE5_ZAfldinfo(int zaID, char *datafield, char *za_name, int *rank, hsize_t dims[], hid_t numbertype[], char *dimlist, char *maxdimlist)
{
  herr_t     status         = FAIL;/* routine return status variable                  */
  herr_t     statmeta       = FAIL;/* EHgetmetavalue return status                    */
  herr_t     Dstatus        = FAIL;/* Status for H5Dclose                             */
 
  hid_t      fid            = FAIL;/* HDF-EOS file ID                                 */
  hid_t      gid            = FAIL;/* "HDFEOS" group ID                               */
  hid_t      dspace         = FAIL;/* "fieldname" Dataspace ID                        */
  hid_t      fieldID        = FAIL;/* "fieldname" Dataset ID                          */
  hid_t      groupid        = FAIL;/* group ID                                        */
  hid_t      datasetid      = FAIL;/* dataset ID                                      */
  hid_t      datatype       = FAIL;/* data type ID                                    */
 
  H5T_class_t  classid = H5T_NO_CLASS;/* data type class ID                           */
 
  long       ndims          = 0;   /* Number of dimensions                            */
  long       idx            = FAIL;/* Index of a za                                   */
  long       i              = 0;   /* Loop index                                      */
 
  size_t     slen[HE5_DTSETRANKMAX];            /* Length of each entry in parsed string  */
 
  char       *metabuf       = (char *)NULL;     /* Ptr to structural metadata (SM)        */
  char       *metaptrs[2]   = {NULL, NULL};     /* Ptrs to the begining and end of SM     */
  char       utlstr[HE5_HDFE_UTLBUFSIZE];       /* Utility string                         */
  char       *ptr[HE5_DTSETRANKMAX];            /* String pointers for parsed string      */
  char       dimstr[HE5_HDFE_DIMBUFSIZE];       /* Individual dimension entry string      */
  char       maxdimstr[HE5_HDFE_DIMBUFSIZE];    /* Individual max. dimension entry string */
  char       errbuf[HE5_HDFE_ERRBUFSIZE];       /* Error message buffer                   */
  htri_t     str_is_variable;                   /* boolean: TRUE if string is variable 
						   lengeth FALSE if string is fixed length
						   -1 if error in H5Tis_variavle_str()    */
 
  CHECKPOINTER(datafield);
  CHECKPOINTER(za_name);
 
  /* Initialize rank to FAIL (error) */
  /* ------------------------------- */
  *rank = FAIL;
 
  /* Initialize slen[] array */
  /* ----------------------- */
  for ( i = 0; i < HE5_DTSETRANKMAX; i++)
    slen[i] = 0;
 
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAfldinfo", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
 
  /* Reset status to FAIL */
  /* -------------------- */
  status = FAIL;
 
  /* Get pointers to appropriate "Field" section within SM */
  /* ----------------------------------------------------- */
  metabuf = (char *)HE5_EHmetagroup(fid, HE5_ZAXZa[idx].zaname, "z", "DataField", metaptrs);
 
  if ( metabuf == NULL )
    {
      status = FAIL;
      sprintf(errbuf, "Cannot get pointer to  metabuf.\n");
      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
 
 
  sprintf(utlstr, "%s%s%s", "\"", za_name, "\"\n");
 
  metaptrs[0] = strstr(metaptrs[0], utlstr);
 
  /* If field is found in "Data Fields" group */
  /* ---------------------------------------- */
  if (metaptrs[0] < metaptrs[1] && metaptrs[0] != NULL)
    {
      /* Get DataType string */
      /* ------------------- */
      statmeta = HE5_EHgetmetavalue(metaptrs, "DataType", utlstr);
 
      /* Convert to data type class ID */
      /* ----------------------------- */
      if (statmeta == SUCCEED)
        {
	  groupid = HE5_ZAXZa[idx].data_id;
 
	  /* Open the field dataset */
	  /* ---------------------- */
	  datasetid     = H5Dopen(groupid, za_name);
	  if (datasetid == FAIL)
	    {
	      status = FAIL;
	      sprintf(errbuf, "Cannot open the dataset for the \"%s\" field.\n",za_name);
	      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	      if (metabuf != NULL) free(metabuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
 
	  /* Get the data type ID */
	  /* -------------------- */
	  datatype      = H5Dget_type(datasetid);
	  if (datatype == FAIL)
	    {
	      status = FAIL;
	      sprintf(errbuf, "Cannot get the data type for the \"%s\" field.\n",za_name);
	      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      if (metabuf != NULL) free(metabuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
 
	  /* Get the data type class ID */
	  /* -------------------------- */
	  classid       = H5Tget_class(datatype);
	  if (classid == H5T_NO_CLASS)
	    {
	      status = FAIL;
	      sprintf(errbuf, "Cannot get the data type class ID for the \"%s\" field.\n",za_name);
	      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      if (metabuf != NULL) free(metabuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
 
	  if (classid == H5T_STRING)
	    {
	      /* HE5T_CHARSTRING has variable length for data fields */
	      str_is_variable = H5Tis_variable_str(datatype);
	      if(str_is_variable == TRUE)
		{
		  numbertype[0] = HE5T_CHARSTRING;
		}
	      else if(str_is_variable == FALSE)
		{
		  numbertype[0] = HE5T_NATIVE_CHAR;
		}
	      else
		{
		  status = FAIL;
		  sprintf(errbuf, "Failed to see if string field is varaible or fixed length for the \"%s\" field.\n",za_name);
		  H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
			      
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  if (metabuf != NULL) free(metabuf);
		  return(status);
		}
			  
	      /*HE5_ZAgetstringtype(zaID, fieldname,classid);*/
	      /*numbertype[0] = HE5T_CHARSTRING;*/
	    }
	  else
	    {
	      numbertype[0] = HE5_EHdtype2numtype(datatype);
	      if (numbertype[0] == FAIL)
		{
		  status = FAIL;
		  sprintf(errbuf, "Cannot get the number type for the \"%s\" field.\n",za_name);
		  H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
		  if (metabuf != NULL) free(metabuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
	    }
 
	  /* Release the datatype ID */
	  /* ----------------------- */
	  status       = H5Tclose(datatype);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the datatype ID for the \"%s\" field.\n",za_name);
	      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	      if (metabuf != NULL) free(metabuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }

	  /* Release the dataset ID */
	  /* ---------------------- */
	  Dstatus       = H5Dclose(datasetid);
	  if (Dstatus == FAIL)
	    {
	      status = FAIL;
	      sprintf(errbuf, "Cannot release the dataset ID for the \"%s\" field.\n",za_name);
	      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	      if (metabuf != NULL) free(metabuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
        }
      else
        {
	  status = FAIL;
	  sprintf(errbuf, "The \"DataType\" string not found in metadata. \n");
	  H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASET, H5E_SEEKERROR, errbuf);
	  if (metabuf != NULL) free(metabuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }
 
      /* Get "DimList" string, trim off "(" and ")" */
      /* ------------------------------------------ */
      statmeta = HE5_EHgetmetavalue(metaptrs, "DimList", utlstr);
      if (statmeta == SUCCEED)
        {
	  memmove(utlstr, utlstr + 1, strlen(utlstr) - 2);
	  utlstr[strlen(utlstr) - 2] = 0;
 
	  /* Parse trimmed "DimList" string and get rank */
	  /* ------------------------------------------- */
	  ndims = HE5_EHparsestr(utlstr, ',', ptr, slen);
	  *rank = HE5_EHlong2int(ndims);
	  if (*rank == FAIL)
	    {
	      sprintf(errbuf, "Cannot convert from \"long\" to \"int\" data type. \n");
	      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      if (metabuf != NULL) free(metabuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
        }
      else
        {
	  status = FAIL;
	  sprintf(errbuf, "The \"DimList\" string not found in metadata. \n");
	  H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASET, H5E_SEEKERROR, errbuf);
	  if (metabuf != NULL) free(metabuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }
 
      /* If "DimList" IS REQUESTED ...  */
      /* -----------------------------  */
      if (dimlist != NULL)
	{
	  strcpy(dimstr,"");
	  strcpy(dimlist,"");
 
	  /*
	   * Copy each entry in DimList and remove leading and trailing quotes.
	   * Get dimension sizes and concatenate dimension names to dimension list.
	   */
	  for (i = 0; i < ndims; i++)
	    {
	      memmove(dimstr, ptr[i] + 1, slen[i] - 2);
	      dimstr[slen[i] - 2] = 0;
	      if (i > 0)
		{
		  strcat(dimlist,",");
		}
	      strcat(dimlist, dimstr);
	    }
	}
 
 
      /* If "MaxdimList" IS REQUESTED  ...      */
      /* -------------------------------------- */
      if(maxdimlist != NULL)
	{
	  strcpy(utlstr,"");
	  statmeta = HE5_EHgetmetavalue(metaptrs,"MaxdimList", utlstr);
	  if (statmeta == SUCCEED)
	    {
	      memmove(utlstr, utlstr + 1, strlen(utlstr) - 2);
	      utlstr[strlen(utlstr) - 2] = 0;
	      ndims = HE5_EHparsestr(utlstr,',', ptr, slen);
	    }
	  else
	    {
	      status = FAIL;
	      sprintf(errbuf, "The \"MaxdimList\" string not found in metadata. \n");
	      H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASET, H5E_SEEKERROR, errbuf);
	      if (metabuf != NULL) free(metabuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
 
	  strcpy(maxdimstr,"");
	  strcpy(maxdimlist,"");
 
	  for (i = 0; i < ndims; i++)
	    {
	      memmove(maxdimstr, ptr[i] + 1, slen[i] - 2);
	      maxdimstr[slen[i] - 2] = 0;
	      if (maxdimlist != NULL)
		{
		  if (i > 0)
		    {
		      strcat(maxdimlist,",");
		    }
		  strcat(maxdimlist, maxdimstr);
		}
	    }
	}
         
 
      /* Get field ID */
      /* ------------ */
      HE5_ZAfieldsizeID( zaID, za_name, &fieldID);
         
      /* Get the data space ID */
      /* --------------------- */
      dspace = H5Dget_space( fieldID );
      if(dspace == FAIL)
	{
	  status = FAIL;
	  sprintf(errbuf, "Cannot get the data space ID.\n");
	  H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	  if (metabuf != NULL) free(metabuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
 
      /* Get the dataset rank */
      /* -------------------- */
      *rank  = H5Sget_simple_extent_ndims(dspace);
      if( *rank == FAIL)
	{
	  status = FAIL;
	  sprintf(errbuf, "Cannot get the rank of the dataset.\n");
	  H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  if (metabuf != NULL) free(metabuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
 
      /* Get the dataset dimension sizes */
      /* ------------------------------- */
      status = H5Sget_simple_extent_dims(dspace, dims, NULL);
      if( status == FAIL)
	{
	  sprintf(errbuf, "Cannot get the dimension array of the dataset.\n");
	  H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  if (metabuf != NULL) free(metabuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
 
      /* Release the data space ID */
      /* ------------------------- */
      status = H5Sclose(dspace);
      if( status == FAIL)
	{
	  sprintf(errbuf, "Cannot release the data space ID.\n");
	  H5Epush(__FILE__, "HE5_ZAfldinfo", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	  if (metabuf != NULL) free(metabuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }
 
  if (metabuf != NULL) free(metabuf);
 
 COMPLETION:
  return(status);
}
 


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinfo                                                       |
|                                                                             |
|  DESCRIPTION: Wrapper around HE5_ZAfldinfo                                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  za_name        char*               za name                                 |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  rank           int*                rank of field (# of dims)               |
|  dims           hsize_t             field dimensions                        |
|  dtype          hid_t               data type ID                            |
|  dimlist        char*               field dimension list                    |
|  maxdimlist     char*               field maximum dimension list            |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date       Programmer    Description                                      |
|  =========   ============  ================================================ |
|  Jul 02       S.ZHAO       Original development                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_ZAinfo(hid_t zaID, char *za_name, int *rank, hsize_t dims[], hid_t dtype[], char *dimlist, char *maxdimlist)
{

  herr_t       status      = FAIL;/* routine return status variable */

  int          nameflag    = FAIL;/* Actual(1)/Alias(0) field name  */   

  hid_t        fid         = FAIL;/* HDF-EOS file ID                */
  hid_t        gid         = FAIL;/* "HDFEOS" group ID              */
  
  long         idx         = FAIL;/* za index                       */

  char         fieldname[HE5_HDFE_NAMBUFSIZE];    /* field name buffer    */
  char         fldactualname[HE5_HDFE_NAMBUFSIZE];/* Actual field name    */  
  char         errbuf[HE5_HDFE_ERRBUFSIZE];       /* Error message buffer */
  
  CHECKPOINTER(za_name);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Call HE5_ZAfldnameinfo() to get actual field name */
      /* ------------------------------------------------- */
      nameflag = HE5_ZAfldnameinfo(zaID, za_name, fldactualname);
      if ( nameflag == FAIL )
	{
	  sprintf(errbuf, "Cannot get actual name of the field.\n");
	  H5Epush(__FILE__, "HE5_ZAinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Name is not alias */
      /* ----------------- */
      if (nameflag == TRUE)
	{
	  strcpy(fieldname,"");
	  strcpy(fieldname,za_name);
	}

      /* Name is alias */
      /* ------------- */
      if (nameflag == FALSE)
	{
	  strcpy(fieldname,"");
	  strcpy(fieldname,fldactualname);
	}

      /* Check for field within Data Fields */
      /* ---------------------------------- */
      status = HE5_ZAfldinfo(zaID, "Data Fields", fieldname, rank, dims, dtype, dimlist, maxdimlist);

      /* If not there either then can't be found */
      /* --------------------------------------- */
      if (status == FAIL)
        {
	  sprintf(errbuf,"Field \"%s\" not found. Check out the field name. \n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAinfo", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
        }	  
    }

 COMPLETION:  
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefchunk                                                   |
|                                                                             |
|  DESCRIPTION: Defines size of chunk used to store a chunked layout dataset  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  rank           int                 rank of a dataset                       |
|  dim            const hsize_t*      pointer to the array containing sizes   |
|                                     of each dimension of a chunk            |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|  Date        Programmer    Description                                      |
|  =========   ============  ==============================================   |
|  Jul 02       S.ZHAO       Original development                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAdefchunk(hid_t zaID, int rank,  const hsize_t *dim)
{
  herr_t          status   = FAIL;/* Return status variable */

  hid_t           fid      = FAIL;/* HDF-EOS file ID        */
  hid_t           gid      = FAIL;/* "HDFEOS" group ID      */

  long            idx      = FAIL;/* za index               */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */

  HE5_LOCK;
  CHECKPOINTER(dim);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdefchunk", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* get the current dataset creation property ID from external array  */
      /* if current property instance has not been created, then create it */
      if( HE5_ZAXZa[idx].plist == FAIL){
	HE5_ZAXZa[idx].plist = H5Pcreate(H5P_DATASET_CREATE);

	if (HE5_ZAXZa[idx].plist == FAIL){
	  sprintf(errbuf, "Cannot create the property list. \n");
	  H5Epush(__FILE__, "HE5_ZAdefchunk", __LINE__, H5E_PLIST, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
      }

      /* Set layout */
      /* ---------- */
      status = H5Pset_layout( HE5_ZAXZa[idx].plist, H5D_CHUNKED);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot set the \"CHUNKED\" type of storage. \n");
	  H5Epush(__FILE__, "HE5_ZAdefchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      /* Set chunking */
      /* ------------ */
      status = H5Pset_chunk( HE5_ZAXZa[idx].plist, rank, dim); 
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot set the sizes of chunks. \n");
	  H5Epush(__FILE__, "HE5_ZAdefchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
    }

 COMPLETION:
  HE5_UNLOCK;
  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefcomp                                                    |
|                                                                             |
|  DESCRIPTION: Defines compression type and parameters                       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  compcode       int                 compression code                        |
|  compparm       int*                compression parameters                  |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:   Before calling this function, storage must be CHUNKED             |
|                      Values of compression code                             |
|                         HDFE_COMP_NONE                  0                   |
|                         HDFE_COMP_RLE                   1                   |
|                         HDFE_COMP_NBIT                  2                   |
|                         HDFE_COMP_SKPHUFF               3                   |
|                         HDFE_COMP_DEFLATE               4                   |
|                         HDFE_COMP_SZIP_EC               5                   |
|                         HDFE_COMP_SZIP_NN               6                   |
|                         HDFE_COMP_SZIP_K13              7                   |
|                         HDFE_COMP_SZIP_CHIP             8                   |
|                         HDFE_COMP_SZIP_K13orEC          9                   |
|                         HDFE_COMP_SZIP_K13orNN          10                  |
|                         HDFE_COMP_SHUF_DEFLATE          11                  |
|                         HDFE_COMP_SHUF_SZIP_CHIP        12                  |
|                         HDFE_COMP_SHUF_SZIP_K13         13                  |
|                         HDFE_COMP_SHUF_SZIP_EC          14                  |
|                         HDFE_COMP_SHUF_SZIP_NN          15                  |
|                         HDFE_COMP_SHUF_SZIP_K13orEC     16                  |
|                         HDFE_COMP_SHUF_SZIP_K13orNN     17                  |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Aug 03    S.Zhao       Added Szip compression methods.                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_ZAdefcomp(hid_t zaID, int compcode,  int *compparm)
{
  herr_t          status   = FAIL; /* routine return status variable   */

  hid_t           fid      = FAIL; /* HDF-EOS file ID                  */
  hid_t           gid      = FAIL; /* "HDFEOS" group ID                */

  long            idx      = FAIL; /* za index                         */

  H5D_layout_t    layout = H5D_LAYOUT_ERROR; /* Storage layout type    */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */
  int             can_encode = 0;        /* szip encoder presence flag     */

  HE5_LOCK;
  CHECKPOINTER(compparm);

  /* Check if compression code is valid */
  /* ---------------------------------- */
  if( compcode != HE5_HDFE_COMP_DEFLATE && compcode != HE5_HDFE_COMP_NONE &&
      compcode != HE5_HDFE_COMP_SZIP_CHIP && compcode != HE5_HDFE_COMP_SZIP_K13 &&
      compcode != HE5_HDFE_COMP_SZIP_EC && compcode != HE5_HDFE_COMP_SZIP_NN &&
      compcode != HE5_HDFE_COMP_SZIP_K13orEC && compcode != HE5_HDFE_COMP_SZIP_K13orNN &&
      compcode != HE5_HDFE_COMP_SHUF_DEFLATE && compcode != HE5_HDFE_COMP_SHUF_SZIP_CHIP &&
      compcode != HE5_HDFE_COMP_SHUF_SZIP_K13 && compcode != HE5_HDFE_COMP_SHUF_SZIP_EC &&
      compcode != HE5_HDFE_COMP_SHUF_SZIP_NN && compcode != HE5_HDFE_COMP_SHUF_SZIP_K13orEC &&
      compcode != HE5_HDFE_COMP_SHUF_SZIP_K13orNN )
    {
      status = FAIL;
      sprintf(errbuf,"Invalid/unsupported compression code. \n");
      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
 
  if( compcode == HE5_HDFE_COMP_DEFLATE || compcode == HE5_HDFE_COMP_NONE || compcode == HE5_HDFE_COMP_SHUF_DEFLATE)
    {
      /* Check GZIP compression level */
      /* ---------------------------- */
      if( compparm[0] < 0 || compparm[0] > 9)
	{
	  status = FAIL;
	  sprintf(errbuf,"Invalid GZIP compression level. \n");
	  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }
  else
    {
      /* Check SZIP compression block size */
      /* --------------------------------- */
      if( compparm[0] != 2 && compparm[0] != 4 && compparm[0] != 6 && compparm[0] != 8 &&
	  compparm[0] != 10 && compparm[0] != 12 && compparm[0] != 14 && compparm[0] != 16 &&
	  compparm[0] != 18 && compparm[0] != 20 && compparm[0] != 22 && compparm[0] != 24 &&
	  compparm[0] != 26 && compparm[0] != 28 && compparm[0] != 30 && compparm[0] != 32)
	{
	  status = FAIL;
	  sprintf(errbuf,"Invalid SZIP compression block size. \n");
	  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }
 
#ifdef H5_HAVE_FILTER_SZIP
  can_encode = HE5_szip_can_encode();
#endif

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdefcomp", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* If the current property list is not set up */
      /* ------------------------------------------ */
      if( HE5_ZAXZa[idx].plist == FAIL)
	{
	  status = FAIL;
	  sprintf(errbuf,"Unable to compress, dataset storage layout has not been set to CHUNKED. \n");
	  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_PLIST, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
      else 
	{
	  layout = H5Pget_layout(HE5_ZAXZa[idx].plist);
	  if (layout == H5D_LAYOUT_ERROR)
	    {
	      status = FAIL;
	      sprintf(errbuf, "Cannot get the layout of the raw data. \n");
	      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_PLIST, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
		  
	  if( layout != H5D_CHUNKED )
	    status =  H5Pset_layout(HE5_ZAXZa[idx].plist, H5D_CHUNKED);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot set the \"CHUNKED\" type of storage of the raw data. \n");
	      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(status);
	    }
 	   
		  
	  switch (compcode)
	    {
	      /* Set NBIT compression parameters in compression external array */
	      /* ------------------------------------------------------------- */ 
	    case HE5_HDFE_COMP_NBIT:
			  
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
	      HE5_ZAXZa[idx].compparm[1] = compparm[1];
	      HE5_ZAXZa[idx].compparm[2] = compparm[2];
	      HE5_ZAXZa[idx].compparm[3] = compparm[3];
			  
	      break;
			  
	    case HE5_HDFE_COMP_DEFLATE:
			  
	      /* ------------------------------------------------- */
	      /* Set compression method to H5D_COMPRESS_DEFLATE    */
	      /*       and compression level to "compparm[0]"      */
	      /* ------------------------------------------------- */
	      status = H5Pset_deflate(HE5_ZAXZa[idx].plist, compparm[0]);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set GZIP compresssion method and level. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
                         
	      /* ------------------------------------------------- */
	      /* Set GZIP compression method and compression       */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
			  
	      break;

	    case HE5_HDFE_COMP_SZIP_CHIP:
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_CHIP_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_CHIP compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set SZIP_CHIP compression method and compression  */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SZIP_K13:
  	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_K13 compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif

	      /* ------------------------------------------------- */
	      /* Set SZIP_K13 compression method and compression   */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SZIP_EC:
  	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_EC_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_EC compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set SZIP_EC compression method and compression    */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SZIP_NN:
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{ 
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_NN_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_NN compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set SZIP_NN compression method and compression    */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SZIP_K13orEC:
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{ 
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK|H5_SZIP_EC_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_K13orEC compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set SZIP_K13orEC compression method and           */
	      /*    compression parameters in external array       */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SZIP_K13orNN:
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{ 
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK|H5_SZIP_NN_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_K13orNN compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set SZIP_K13orNN compression method and           */
	      /*    compression parameters in external array       */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SHUF_DEFLATE:
                         
	      status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set the shuffling method. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}

	      status = H5Pset_deflate(HE5_ZAXZa[idx].plist, compparm[0]);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set GZIP compresssion method and level. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
 
	      /* ------------------------------------------------- */
	      /* Set shuffling+GZIP method and compression         */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SHUF_SZIP_CHIP:
 
	      status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set the shuffling method. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_CHIP_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_CHIP compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set shuffling+SZIP_CHIP method and compression    */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SHUF_SZIP_K13:
 
	      status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set the shuffling method. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_K13 compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set shuffling+SZIP_K13 method and compression     */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SHUF_SZIP_EC:
 
	      status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set the shuffling method. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_EC_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_EC compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set shuffling+SZIP_EC method and compression      */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SHUF_SZIP_NN:
 
	      status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set the shuffling method. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_NN_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_NN compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set shuffling+SZIP_NN method and compression      */
	      /*    parameters in external array                   */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SHUF_SZIP_K13orEC:
 
	      status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set the shuffling method. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK|H5_SZIP_EC_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_K13orEC compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set shuffling+SZIP_K13orEC method and             */
	      /*    compression parameters in external array       */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;
 
	    case HE5_HDFE_COMP_SHUF_SZIP_K13orNN:
 
	      status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot set the shuffling method. \n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  return(status);
		}
 	      
#ifdef H5_HAVE_FILTER_SZIP
	      if(can_encode == 1)
		{
		  status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK|H5_SZIP_NN_OPTION_MASK, compparm[0]);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot set SZIP_K13orNN compresssion method and block size. \n");
		      H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      return(status);
		    }
		} 
	      else
		{
		  sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		  H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		}
#endif
 
	      /* ------------------------------------------------- */
	      /* Set shuffling+SZIP_K13orNN method and             */
	      /*    compression parameters in external array       */
	      /* ------------------------------------------------- */
	      HE5_ZAXZa[idx].compcode = compcode;
	      HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	      break;

	    case HE5_HDFE_COMP_NONE:

	      HE5_ZAXZa[idx].compparm[0] = compparm[0];

	      break;
			  
	    default:
	      {
		status = FAIL;
		sprintf(errbuf, "Invalid compression method \"%d\" . \n", compcode);
		H5Epush(__FILE__, "HE5_ZAdefcomp", __LINE__, H5E_ARGS, H5E_UNSUPPORTED, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
	      break;
			  
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
|  FUNCTION: HE5_ZAdefcomchunk                                                |
|                                                                             |
|  DESCRIPTION: Defines compression type and parameters and sets chunking     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  compcode       int                 compression code                        |
|  compparm       int*                compression parameters                  |
|  rank           int                 rank of a dataset                       |
|  dim            const hsize_t*      pointer to the array containing sizes   |
|                                     of each dimension of a chunk            |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:   Before calling this function, storage must be CHUNKED             |
|                      Values of compression code                             |
|                         HDFE_COMP_NONE                  0                   |
|                         HDFE_COMP_RLE                   1                   |
|                         HDFE_COMP_NBIT                  2                   |
|                         HDFE_COMP_SKPHUFF               3                   |
|                         HDFE_COMP_DEFLATE               4                   |
|                         HDFE_COMP_SZIP_EC               5                   |
|                         HDFE_COMP_SZIP_NN               6                   |
|                         HDFE_COMP_SZIP_K13              7                   |
|                         HDFE_COMP_SZIP_CHIP             8                   |
|                         HDFE_COMP_SZIP_K13orEC          9                   |
|                         HDFE_COMP_SZIP_K13orNN          10                  |
|                         HDFE_COMP_SHUF_DEFLATE          11                  |
|                         HDFE_COMP_SHUF_SZIP_CHIP        12                  |
|                         HDFE_COMP_SHUF_SZIP_K13         13                  |
|                         HDFE_COMP_SHUF_SZIP_EC          14                  |
|                         HDFE_COMP_SHUF_SZIP_NN          15                  |
|                         HDFE_COMP_SHUF_SZIP_K13orEC     16                  |
|                         HDFE_COMP_SHUF_SZIP_K13orNN     17                  |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Aug 03    S.Zhao       Added Szip compression methods.                     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
  herr_t   
    HE5_ZAdefcomchunk(hid_t zaID, int compcode, int *compparm, int rank, const hsize_t *dim)
  {
    herr_t          status   = FAIL;            /* Return status variable */

    int             i        = 0;               /* Loop index             */

    hid_t           fid      = FAIL;            /* HDF-EOS file ID        */
    hid_t           gid      = FAIL;            /* "HDFEOS" group ID      */

    hsize_t         dims[HE5_DTSETRANKMAX];     /* Default dim. sizes     */

    long            idx      = FAIL;            /* za index               */

    H5D_layout_t    layout = H5D_LAYOUT_ERROR;  /* Storage layout type    */

    char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
    int             can_encode;        /* szip encoder presence flag     */

    HE5_LOCK;
    CHECKPOINTER(compparm);
    CHECKPOINTER(dim);

    /* Check if compression code is valid */
    /* ---------------------------------- */
    if( compcode != HE5_HDFE_COMP_DEFLATE && compcode != HE5_HDFE_COMP_NONE &&
	compcode != HE5_HDFE_COMP_SZIP_CHIP && compcode != HE5_HDFE_COMP_SZIP_K13 &&
	compcode != HE5_HDFE_COMP_SZIP_EC && compcode != HE5_HDFE_COMP_SZIP_NN &&
	compcode != HE5_HDFE_COMP_SZIP_K13orEC && compcode != HE5_HDFE_COMP_SZIP_K13orNN &&
	compcode != HE5_HDFE_COMP_SHUF_DEFLATE && compcode != HE5_HDFE_COMP_SHUF_SZIP_CHIP &&
	compcode != HE5_HDFE_COMP_SHUF_SZIP_K13 && compcode != HE5_HDFE_COMP_SHUF_SZIP_EC &&
	compcode != HE5_HDFE_COMP_SHUF_SZIP_NN && compcode != HE5_HDFE_COMP_SHUF_SZIP_K13orEC &&
	compcode != HE5_HDFE_COMP_SHUF_SZIP_K13orNN) 
      {
	status = FAIL;
	sprintf(errbuf, "Invalid/unsupported compression code. \n");
	H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	HE5_EHprint(errbuf, __FILE__, __LINE__);
	return(status);
      }
 
    if( compcode == HE5_HDFE_COMP_DEFLATE || compcode == HE5_HDFE_COMP_NONE || compcode == HE5_HDFE_COMP_SHUF_DEFLATE)
      {
	/* Check GZIP compression level */
	/* ---------------------------- */
	if( compparm[0] < 0 || compparm[0] > 9)
	  {
	    status = FAIL;
	    sprintf(errbuf,"Invalid GZIP compression level. \n");
	    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    return(status);
	  }
      }
    else
      {
	/* Check SZIP compression block size */
	/* --------------------------------- */
	if( compparm[0] != 2 && compparm[0] != 4 && compparm[0] != 6 && compparm[0] != 8 &&
	    compparm[0] != 10 && compparm[0] != 12 && compparm[0] != 14 && compparm[0] != 16 &&
	    compparm[0] != 18 && compparm[0] != 20 && compparm[0] != 22 && compparm[0] != 24 &&
	    compparm[0] != 26 && compparm[0] != 28 && compparm[0] != 30 && compparm[0] != 32)
	  {
	    status = FAIL;
	    sprintf(errbuf,"Invalid SZIP compression block size. \n");
	    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    return(status);
	  }
      }

#ifdef H5_HAVE_FILTER_SZIP
    can_encode = HE5_szip_can_encode();
#endif

    /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
    /* --------------------------------------------------- */
    status = HE5_ZAchkzaid(zaID, "HE5_ZAdefcomchunk", &fid, &gid, &idx);
    if (status == SUCCEED)
      {
	/* Initialize dims[] array */
	/* ----------------------- */
	for (i = 0; i < HE5_DTSETRANKMAX; i++)
	  dims[i] = 0;

	/* get the current dataset creation property ID from external array  */
	/* ----------------------------------------------------------------- */
	if(HE5_ZAXZa[idx].plist == FAIL)
	  {
	    /* create property list */
	    /* -------------------- */
	    HE5_ZAXZa[idx].plist = H5Pcreate(H5P_DATASET_CREATE);

	    /* set layout to "H5D_CHUNKED" */
	    /* --------------------------- */
	    status = H5Pset_layout(HE5_ZAXZa[idx].plist, H5D_CHUNKED);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the \"CHUNKED\" type of storage. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_STORAGE, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	  }
	else 
	  {
	    /* Get layout of the raw data */
	    /* -------------------------- */
	    layout = H5Pget_layout(HE5_ZAXZa[idx].plist);
	    if (layout == H5D_LAYOUT_ERROR)
	      {
		status = FAIL;
		sprintf(errbuf, "Cannot get the layout of the raw data. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_STORAGE, H5E_NOTFOUND, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
		  
	    if( layout != H5D_CHUNKED )
	      {
			
		/* set layout to "H5D_CHUNKED" */
		/* --------------------------- */
		status = H5Pset_layout( HE5_ZAXZa[idx].plist, H5D_CHUNKED);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set the \"CHUNKED\" type of storage. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_STORAGE, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      }
	  }
	  
	/* set chunking */
	/* ------------ */
	if ( rank > 0 && dim != (hsize_t *)NULL)
	  {
	    status = H5Pset_chunk(HE5_ZAXZa[idx].plist, rank, dim); 
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the sizes of chunks. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	  }
	else if ( rank > 0 && dim == (hsize_t *)NULL)
	  {
	    /* Set default chunk sizes */
	    /* ----------------------- */
	    for (i = 0; i < rank; i++)
	      dims[ i ] = HE5_CHUNKSIZE;
		  
	    status = H5Pset_chunk(HE5_ZAXZa[idx].plist, rank, dims); 
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the sizes of chunks. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	  }
	else
	  {
	    sprintf(errbuf, "Invalid dataset rank,\"%d\" . \n", rank);
	    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	    return(FAIL);
	  }
	  
	  
	switch (compcode)
	  {
	    /* Set NBIT compression parameters in compression external array */
	    /* ------------------------------------------------------------- */ 
	  case HE5_HDFE_COMP_NBIT:
		  
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
	    HE5_ZAXZa[idx].compparm[1] = compparm[1];
	    HE5_ZAXZa[idx].compparm[2] = compparm[2];
	    HE5_ZAXZa[idx].compparm[3] = compparm[3];
		  
	    break;
		  
	  case HE5_HDFE_COMP_DEFLATE:
		  
	    /* ------------------------------------------------- */
	    /* Set compression method to H5D_COMPRESS_DEFLATE    */
	    /*       and compression level to "compparm[0]"      */
	    /* ------------------------------------------------- */
	    status = H5Pset_deflate(HE5_ZAXZa[idx].plist, compparm[0]);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set GZIP compresssion method and level. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
 
	    /* ------------------------------------------------- */
	    /* Set GZIP compression method and compression       */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
		  
	    break;

	  case HE5_HDFE_COMP_SZIP_CHIP:
 	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      { 
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_CHIP_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_CHIP compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
 
	    /* ------------------------------------------------- */
	    /* Set SZIP_CHIP compression method and compression  */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SZIP_K13:
 	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      {
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_K13 compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
 
	    /* ------------------------------------------------- */
	    /* Set SZIP_K13 compression method and compression   */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SZIP_EC:
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      { 
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_EC_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_EC compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
  
	    /* ------------------------------------------------- */
	    /* Set SZIP_EC compression method and compression    */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SZIP_NN:
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      { 
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_NN_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_NN compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
  
	    /* ------------------------------------------------- */
	    /* Set SZIP_NN compression method and compression    */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SZIP_K13orEC:
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      { 
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK|H5_SZIP_EC_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_K13orEC compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
  
	    /* ------------------------------------------------- */
	    /* Set SZIP_K13orEC compression method and           */
	    /*    compression parameters in external array       */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SZIP_K13orNN:
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      { 
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK|H5_SZIP_NN_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_K13orNN compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
  
	    /* ------------------------------------------------- */
	    /* Set SZIP_K13orNN compression method and           */
	    /*    compression parameters in external array       */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;

	  case HE5_HDFE_COMP_SHUF_DEFLATE:
                 
	    status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the shuffling method. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }

	    status = H5Pset_deflate(HE5_ZAXZa[idx].plist, compparm[0]);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set GZIP compresssion method and level. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
 
	    /* ------------------------------------------------- */
	    /* Set shuffling+GZIP method and compression         */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SHUF_SZIP_CHIP:
 
	    status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the shuffling method. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      {
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_CHIP_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_CHIP compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
 
	    /* ------------------------------------------------- */
	    /* Set shuffling+SZIP_CHIP method and compression    */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SHUF_SZIP_K13:
 
	    status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the shuffling method. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      {
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_K13 compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
 
	    /* ------------------------------------------------- */
	    /* Set shuffling+SZIP_K13 method and compression     */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SHUF_SZIP_EC:
 
	    status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the shuffling method. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      {
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_EC_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_EC compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
 
	    /* ------------------------------------------------- */
	    /* Set shuffling+SZIP_EC method and compression      */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SHUF_SZIP_NN:
 
	    status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the shuffling method. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      {
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_NN_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_NN compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
	      
	    /* ------------------------------------------------- */
	    /* Set shuffling+SZIP_NN method and compression      */
	    /*    parameters in external array                   */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SHUF_SZIP_K13orEC:
 
	    status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the shuffling method. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      {
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK|H5_SZIP_EC_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_K13orEC compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
 
	    /* ------------------------------------------------- */
	    /* Set shuffling+SZIP_K13orEC method and             */
	    /*    compression parameters in external array       */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;
 
	  case HE5_HDFE_COMP_SHUF_SZIP_K13orNN:
 
	    status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the shuffling method. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }
	      
#ifdef H5_HAVE_FILTER_SZIP
	    if(can_encode == 1)
	      {
		status = H5Pset_szip(HE5_ZAXZa[idx].plist, H5_SZIP_ALLOW_K13_OPTION_MASK|H5_SZIP_NN_OPTION_MASK, compparm[0]);
		if (status == FAIL)
		  {
		    sprintf(errbuf, "Cannot set SZIP_K13orNN compresssion method and block size. \n");
		    H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		    HE5_EHprint(errbuf, __FILE__, __LINE__);
		    return(status);
		  }
	      } 
	    else
	      {
		sprintf(errbuf, "Szip does not have encoder; szip compression won't apply to datafields.\n");
		H5Epush(__FILE__, "HE5_ZAdefcompchunk", __LINE__, H5E_RESOURCE, H5E_CANTENCODE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
	      }
#endif
	
	    /* ------------------------------------------------- */
	    /* Set shuffling+SZIP_K13orNN method and             */
	    /*    compression parameters in external array       */
	    /* ------------------------------------------------- */
	    HE5_ZAXZa[idx].compcode = compcode;
	    HE5_ZAXZa[idx].compparm[0] = compparm[0];
 
	    break;

	  case HE5_HDFE_COMP_NONE:

	    status = H5Pset_shuffle(HE5_ZAXZa[idx].plist);
	    if (status == FAIL)
	      {
		sprintf(errbuf, "Cannot set the shuffling method. \n");
		H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
		HE5_EHprint(errbuf, __FILE__, __LINE__);
		return(status);
	      }

	    HE5_ZAXZa[idx].compparm[0] = compparm[0];

	    break;
		  
	  default:
	    {
	      status = FAIL;
	      sprintf(errbuf, "Invalid (unsupported) compression method \"%d\". \n", compcode);
	      H5Epush(__FILE__, "HE5_ZAdefcomchunk", __LINE__, H5E_ARGS, H5E_UNSUPPORTED, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	    }
	    break;
		  
	  }
      }  
  COMPLETION:
    HE5_UNLOCK;
    return(status);
  }



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefinefield                                                |
|                                                                             |
|  DESCRIPTION: Defines data field within ZA structure                        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  datafield      char*               data field                              |
|  za_name        const char*         za name                                 |
|  dimlist        char*               Dimension (comma-separated)list         |
|  maxdimlist     char*               Maximum Dimension (comma-separated) list|
|  numbertype     hid_t               field data type ID                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:  H5S_UNLIMITED is -1, not 0  => unlimited extendiable dimention     |
|           size = -1.                                                        |
|          If maxdimlist = NULL, then dimlist string is copied to the         |
|          maxdimlist string, and corresponding information is written to     |
|          the metadata section.                                              |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Jan 04    S.Zhao       Added to create a character string dataset.         |
|  Mar 04    S.Zhao       Modified for a character string dataset.            |
|  Apr 04    S.Zhao       Modified for a character string dataset.            |
|  May 05    S.Zhao       Added HE5_EHdtype2numtype() function call.          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t
HE5_ZAdefinefield(hid_t zaID, char *datafield, const char *za_name, char *dimlist, char *maxdimlist, hid_t numbertype_in)
{
  herr_t          status      = FAIL; /* Return status variable             */
 
  int             i           = 0;    /* Loop index                         */
  int             foundAllDim = 1;    /* found all dimensions flag          */
  int             append      = FALSE;/* "Is field appendable?" flag        */
  int             first       = 1;    /* first entry flag                   */
  int             compcode    = FAIL; /* Compression code                   */
  int             rank        = 0;    /* Field rank                         */
  int             maxrank     = 0;    /* max dim  rank                      */
 
  hid_t           fid        = FAIL;/* HDF-EOS file ID                      */
  hid_t           gid        = FAIL;/* "HDFEOS" group ID                    */
  hid_t           data_space = FAIL;/* dataspace ID                         */
  hid_t           dataset    = FAIL;/* dataset ID                           */
  hid_t           heos_gid   = FAIL;/* "ZAS" group ID                       */
  hid_t           ntype      = FAIL;/* Number type ID                       */
  hid_t           numtype    = FAIL;
  hid_t           numbertype;
  hsize_t         metavalue = 0;        /* Metavalue to insert              */
  hsize_t         dims[HE5_DTSETRANKMAX];   /* Dimension size array         */
  hsize_t         maxdims[HE5_DTSETRANKMAX];/* Dimension size array         */
  hsize_t         dimsize   = 0;        /* Dimension size                   */
  hsize_t         count[]   = { 1 };    /* number of elements to insert     */
 
  long            idx       = FAIL;     /* za index                         */
 
  H5D_layout_t    layout = H5D_LAYOUT_ERROR; /* Storage layout for raw data */
 
  void            *value;
 
  size_t          tsize     = 0;                   /* Size of a datatype in bytes  */
  int             attr      = 0;                   /* attribute value              */
 
  char            *dimbuf   = (char *)NULL;        /* Dimension buffer             */
  char            *comma    = (char *)NULL;        /* Pointer to comma             */
  char            *dimcheck = (char *)NULL;        /* Dimension check buffer       */
  char            utlbuf[HE5_HDFE_UTLBUFSIZE];     /* Utility buffer               */
  char            utlbuf2[HE5_HDFE_UTLBUFSIZE];    /* Utility buffer 2             */
  char            errbuf1[HE5_HDFE_ERRBUFSIZE];    /* Error message buffer 1       */
  char            errbuf2[HE5_HDFE_ERRBUFSIZE];    /* Error message buffer 2       */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];     /* Error message buffer         */
  char            maxdimlstbuf[HE5_HDFE_DIMBUFSIZE];
  char            dimlstbuf[HE5_HDFE_DIMBUFSIZE];
  char            compparmbuf[HE5_HDFE_DIMBUFSIZE];/* Compression parmeter         */
  char            *HDFcomp[18] = {"HE5_HDFE_COMP_NONE", "HE5_HDFE_COMP_RLE", "HE5_HDFE_COMP_NBIT", "HE5_HDFE_COMP_SKPHUFF", "HE5_HDFE_COMP_DEFLATE", "HE5_HDFE_COMP_SZIP_CHIP", "HE5_HDFE_COMP_SZIP_K13", "HE5_HDFE_COMP_SZIP_EC", "HE5_HDFE_COMP_SZIP_NN", "HE5_HDFE_COMP_SZIP_K13orEC", "HE5_HDFE_COMP_SZIP_K13orNN", "HE5_HDFE_COMP_SHUF_DEFLATE", "HE5_HDFE_COMP_SHUF_SZIP_CHIP", "HE5_HDFE_COMP_SHUF_SZIP_K13", "HE5_HDFE_COMP_SHUF_SZIP_EC", "HE5_HDFE_COMP_SHUF_SZIP_NN", "HE5_HDFE_COMP_SHUF_SZIP_K13orEC", "HE5_HDFE_COMP_SHUF_SZIP_K13orNN"};   /* Compression code names       */
 
  CHECKNAME(za_name);
  CHECKPOINTER(datafield);
  CHECKPOINTER(dimlist);
 

  /* Convert HDF-EOS5 datatype to HDF5 datatype */
  if (
      H5Tequal(numbertype_in, H5T_NATIVE_CHAR)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_SCHAR)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_UCHAR)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_SHORT)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_USHORT) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_INT)    == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_UINT)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LONG)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_ULONG)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LLONG)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_ULLONG) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_FLOAT)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_DOUBLE) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LDOUBLE)== SUCCEED ||
      H5Tequal(numbertype_in, H5T_C_S1) == SUCCEED)
    {
      numbertype = numbertype_in;
    }
  else
    {
      numbertype = HE5_EHconvdatatype((int) numbertype_in);
    }

  if(numbertype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 type data type ID for defining field.\n");
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Setup error message strings */
  /* --------------------------- */
  strcpy(errbuf1, "HE5_ZAXZDname array too small.\nPlease increase ");
  strcat(errbuf1, "size of HE5_HDFE_NAMBUFSIZE in \"HE5_HdfEosDef.h\".\n");
  strcpy(errbuf2, "HE5_ZAXZDdims array too small.\nPlease increase ");
  strcat(errbuf2, "size of HE5_HDFE_DIMBUFSIZE in \"HE5_HdfEosDef.h\".\n");
       
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdefinefield", &fid, &heos_gid, &idx);
  if(status == FAIL ) return(status);
       
  /* initialize dims and maxdims array */
  /* --------------------------------- */
  for(i = 0; i < HE5_DTSETRANKMAX; i++)
    {
      dims[i]    = 0;
      maxdims[i] = 0;
    }
 
 
  /* Allocate space for dimbuf, copy dimlist into it, & append comma */
  /* --------------------------------------------------------------- */
  dimbuf = (char *) calloc(strlen(dimlist) + 64, sizeof(char));
  if(dimbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
      HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  /*
********************************************************************
*   C H E C K  T H E   C O N T E N T   O F   dimlist  S T R I N G  *
********************************************************************
*/
 
  strcpy(dimbuf, dimlist);
  strcat(dimbuf, ",");
 
  /* Find comma */
  /* ---------- */
  comma = strchr(dimbuf, ',');
       
  /*
   * Loop through entries in dimension list to make sure they are
   * defined in za
   */
  while (comma != NULL)
    {
      /* Copy dimension list entry to dimcheck */
      /* ------------------------------------- */
      dimcheck = (char *) calloc(comma - dimbuf + 1, sizeof(char));
      if(dimcheck == NULL)
	{
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory. \n");
	  HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	  free(dimbuf);
	  return(FAIL);
	}
      memmove(dimcheck, dimbuf, comma - dimbuf);
 
      /* Get dimension size */
      /* ------------------ */
      dimsize = HE5_ZAdiminfo(zaID, dimcheck);
      if (dimsize > 0)
	{
	  dims[rank] = dimsize;
	  rank++;
	}
      else
	{
	  /*
	   * If dimension list entry not found - set error return
	   * status, append name to utility buffer for error report
	   */
	  status = FAIL;
	  foundAllDim = 0;
	  if (first == 1)
	    strcpy(utlbuf, dimcheck);
	  else
	    {
	      strcat(utlbuf, ",");
	      strcat(utlbuf, dimcheck);
	    }
	  first = 0;
	}
 
      /*
       * Go to next dimension entry, find next comma, & free up
       * dimcheck buffer
       */
      memmove(dimbuf, comma + 1, strlen(comma + 1) + 1);
      comma = strchr(dimbuf, ',');
      free(dimcheck);
 
    }
  free(dimbuf);
       
  /* If no dimensions found, report error */
  /* ------------------------------------ */
  if (foundAllDim == 0)
    {
      status = FAIL;
      sprintf(errbuf, "Dimension(s) \"%s\" not found for \"%s\" field. \n", utlbuf, za_name);
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return (status);
    }
 
 
  /*
************************************************************************
*   C H E C K  T H E   C O N T E N T   O F   maxdimlist   S T R I N G  *
************************************************************************
*/
       
 
  /* Allocate space for dimbuf, copy maxdimlist into it, & append comma */
  /* ------------------------------------------------------------------ */
  if(maxdimlist != NULL)
    {
      dimbuf = (char *) calloc(strlen(maxdimlist) + 64, sizeof(char));
      if(dimbuf == NULL)
	{
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory. \n");
	  HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	  return(FAIL);
 
	}
 
      /* Copy "maxdimlist" to "dimbuf", & append comma */
      /* --------------------------------------------- */
      strcpy(dimbuf, maxdimlist);
      strcat(dimbuf, ",");
 
      /* Find comma */
      /* ---------- */
      comma = strchr(dimbuf, ',');
 
      /*
       * Loop through entries in dimension list to make sure they are
       *                     defined in za
       */
      while (comma != NULL)
	{
	  /* Copy dimension list entry to dimcheck */
	  /* ------------------------------------- */
	  dimcheck = (char *) calloc(comma - dimbuf + 1, sizeof(char));
	  if(dimcheck == NULL)
	    {
	      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory. \n");
	      HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	      free(dimbuf);
	      return(FAIL);
	    }
	  memmove(dimcheck, dimbuf, comma - dimbuf);
 
	  /* Get dimension size */
	  /* ------------------ */
	  dimsize = HE5_ZAdiminfo(zaID, dimcheck);
	  if ( (dimsize > 0 && strcmp(dimcheck,"Unlim") != 0 ) || (dimsize == H5S_UNLIMITED && strcmp(dimcheck, "Unlim") == 0 )  )
	    {
	      maxdims[maxrank] = dimsize;
	      maxrank++;
	    }
	  else
	    {
	      /*
	       * If dimension list entry not found - set error return
	       * status, append name to utility buffer for error report
	       */
	      status = FAIL;
	      foundAllDim = 0;
	      if (first == 1)
		strcpy(utlbuf, dimcheck);
	      else
		{
		  strcat(utlbuf, ",");
		  strcat(utlbuf, dimcheck);
		}
	      first = 0;
	    }
 
	  /*
	   * Go to next dimension entry, find next comma, & free up
	   * dimcheck buffer
	   */
	  memmove(dimbuf, comma + 1, strlen(comma + 1) + 1);
	  comma = strchr(dimbuf, ',');
	  free(dimcheck);
	}
 
      free(dimbuf);
 
      /* If no dimensions found, report error */
      /* ------------------------------------ */
      if (foundAllDim == 0)
	{
	  status = FAIL;
	  sprintf(errbuf, "Dimension(s) \"%s\" not found for \"%s\" field. \n", utlbuf, za_name);
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return (status);
	}
 
 
      /* If maxrank !=rank  then report error */
      /* ------------------------------------ */
      if ( maxrank != rank )
	{
	  status = FAIL;
	  sprintf(errbuf,"Dimension rank doesn't match Maximum dimension rank. \n");
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return (status);
	}
    }
  /* "maxlist == NULL"  ==> maxdims[]= dims[] */
  /* ---------------------------------------- */
  else
    {
      for(i = 0; i < rank; i++ )
	maxdims[ i ] = dims[ i ];
    }
 
 
  /* Find out if the dataset dimension is appendable */
  /* ----------------------------------------------- */
  for(i = 0; i < rank; i++)
    {
      if( dims[i] == maxdims[i] )
	append = FALSE;
      else if ( (dims[i] < maxdims[i]) || (maxdims[i] == H5S_UNLIMITED))
	{
	  append = TRUE;
	  break;
	}
      else
	{
	  status = FAIL;
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADRANGE, "Maximum dimension size is smaller than Dimension size. \n");
	  HE5_EHprint("Error: Maximum dimension size is smaller than Dimension size, occured", __FILE__, __LINE__);
	  return(status);
	}
    }
 
 
  /* Check for valid data type ID ("numbertype") */
  /* ------------------------------------------- */
  if (numbertype != HE5T_CHARSTRING)
    {
      if (H5Tequal(numbertype, H5T_NATIVE_CHAR)   != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_SCHAR)  != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_UCHAR)  != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_SHORT)  != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_USHORT) != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_INT)    != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_UINT)   != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_LONG)   != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_ULONG)  != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_LLONG)  != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_ULLONG) != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_FLOAT)  != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_DOUBLE) != SUCCEED &&
          H5Tequal(numbertype, H5T_NATIVE_LDOUBLE)!= SUCCEED &&
          H5Tequal(numbertype, H5T_C_S1) != SUCCEED
          )
        {
          H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, "Invalid (unsupported) data type. \n");
          HE5_EHprint("Error: Invalid (unsupported) data type, occured", __FILE__, __LINE__);
          return(FAIL);
        }
    }
 
 
  /* Get the field group ID */
  /*----------------------- */
  if (strcmp(datafield, "Data Fields") == 0)
    {
      gid = HE5_ZAXZa[idx].data_id;
    }
  else
    {
      sprintf(errbuf,"Invalid (unsupported) field group. \n");
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return (FAIL);
    }
 
  /* Create dataspace then create dataset */
  /* ------------------------------------ */
  if( HE5_ZAXZa[idx].plist == FAIL )
    {
      HE5_ZAXZa[idx].plist = H5Pcreate(H5P_DATASET_CREATE);
    }
 
  /* Get dataset layout */
  /* ------------------ */
  layout = H5Pget_layout(HE5_ZAXZa[idx].plist);
  if( layout == H5D_LAYOUT_ERROR)
    {
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADVALUE, "Cannot get dataset layout.\n");
      HE5_EHprint("Error: Cannot get dataset layout, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  if(append == FALSE)
    data_space = H5Screate_simple(rank, dims, NULL);
  else
    {
      if( layout == H5D_CHUNKED)
	data_space = H5Screate_simple(rank, dims, maxdims);
      else
	{
	  status = FAIL;
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADVALUE, "Appendable dataset MUST BE CHUNKED first.\n");
	  HE5_EHprint("Error: Appendable dataset MUST BE CHUNKED first, occured", __FILE__, __LINE__);
	  return(status);
	}
    }
 
  /* Get current compression code */
  /* ---------------------------- */
  compcode = HE5_ZAXZa[idx].compcode;

  /* Note: in inquiry routines HE5T_CHARSTRING is distinguished 
     from HE5T_NATIVE_CHAR for the field data (not attributes) based
     on whether string has variable or fixed length as set below.
     The field data of type HE5T_NATIVE_CHAR has fixed length of 1, and
     the field is array of characters, not strings. However, HE5T_CHARSTRING
     sets array of vaiable length strings for the field data.
     Currently HE5_EHattr treats HE5T_NATIVE_CHAR, HE5T_CHARSTRING, and
     H5T_C_S1 as fixed (any size) for attributes. 
  */
  ntype = numbertype;
  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_C_S1))
    {
      ntype = H5Tcopy(H5T_C_S1);
      status = H5Tset_size(ntype, H5T_VARIABLE);
      if( status == FAIL)
	{
	  sprintf(errbuf, "Cannot set the total size for \"%s\" field. \n", za_name);
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
 
      /* Create dataset */
      /* -------------- */
      dataset = H5Dcreate(gid, za_name, ntype, data_space, HE5_ZAXZa[idx].plist);
      if (dataset == FAIL)
        {
          status = FAIL;
          sprintf(errbuf, "Cannot create dataset for \"%s\" field. \n", za_name);
          H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(status);
        }
 
    }
  else if (numbertype == H5T_NATIVE_CHAR)
    {
      ntype = H5Tcopy(H5T_C_S1);
      status = H5Tset_size(ntype, 1);
      if( status == FAIL)
	{
	  sprintf(errbuf, "Cannot set the total size for \"%s\" field. \n", za_name);
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
 
      /* Create dataset */
      /* -------------- */
      dataset = H5Dcreate(gid, za_name, ntype, data_space, HE5_ZAXZa[idx].plist);
      if (dataset == FAIL)
        {
          status = FAIL;
          sprintf(errbuf, "Cannot create dataset for \"%s\" field. \n", za_name);
          H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(status);
        }
 
    }

  else
    {
      /* Create dataset */
      /* -------------- */
      dataset = H5Dcreate(gid, za_name, numbertype, data_space, HE5_ZAXZa[idx].plist);
      if (dataset == FAIL)
        {
          status = FAIL;
          sprintf(errbuf, "Cannot create dataset for \"%s\" field. \n", za_name);
          H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(status);
        }
    }
 
  /* Extend the dataset. assure that dataset is at least dims */
  /* -------------------------------------------------------- */
  if( append == TRUE)
    {
      status = H5Dextend(dataset,dims);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot extend the dataset for \"%s\" field.\n", za_name);
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
    }
 
 
  /* load external array */
  /* ------------------- */

  /* store dataset IDs and dataset name */
  /* ---------------------------------- */
  if (HE5_ZAXZa[idx].nDFLD > 0)
    {
      /* Allocate memory to ddataset struct */
      /* ---------------------------------- */
      HE5_ZAXZa[idx].ddataset = (HE5_DTSinfo *)realloc((void *)HE5_ZAXZa[idx].ddataset,(HE5_ZAXZa[idx].nDFLD + 1) * sizeof(HE5_DTSinfo));
 
    }
  else
    {
      HE5_ZAXZa[idx].ddataset = (HE5_DTSinfo *)calloc(1, sizeof(HE5_DTSinfo));
      if (HE5_ZAXZa[idx].ddataset == NULL )
	{
	  status = FAIL;
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory. \n");
	  HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	  return(status);
	}
 
    }
 
  /* Allocate memory to name */
  /* ----------------------- */
  HE5_ZAXZa[idx].ddataset[HE5_ZAXZa[idx].nDFLD].name = (char *)calloc( (strlen(za_name)+1), sizeof(char) );
  if (HE5_ZAXZa[idx].ddataset[HE5_ZAXZa[idx].nDFLD].name == NULL)
    {
      status = FAIL;
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory.\n");
      HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
      return(status);
    }
 
  /* load table */
  /* ---------- */
  HE5_ZAXZa[idx].ddataset[HE5_ZAXZa[idx].nDFLD].ID = dataset;
  strcpy(HE5_ZAXZa[idx].ddataset[HE5_ZAXZa[idx].nDFLD].name, za_name);
  HE5_ZAXZa[idx].nDFLD++;
 
  if ((numbertype != HE5T_CHARSTRING) && (numbertype != H5T_NATIVE_CHAR) && (numbertype != H5T_C_S1))
    { 
      /* Get the size of a datatype in bytes */
      /* ----------------------------------- */
      tsize = H5Tget_size( numbertype);
      if( tsize == 0)
        {
          sprintf(errbuf, "Cannot get the size of a datatype. \n");
          H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return (FAIL);
        }
 
      /* allocate memory for "fill" value */
      /* -------------------------------- */
      value = (void *)calloc(1, tsize);
      if( value == (void *)NULL)
        {
          sprintf(errbuf, "Cannot allocate memory for fill value.\n");
          H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return (FAIL);
        }
 
      /* Try to retrieve the fill value (in case it's already defined) */
      /* ------------------------------------------------------------- */
      H5E_BEGIN_TRY {
        status = H5Pget_fill_value(HE5_ZAXZa[idx].plist,  numbertype,  value);
      }
      H5E_END_TRY;
       
      /* Store fill value in the dataset attribute "_FillValue" */
      /* -----------------------------------------------------  */
      if( status != FAIL)
        {
 
          status = HE5_EHattr( dataset, "_FillValue", numbertype, count, "w", value);
          if( status == FAIL )
	    {
	      sprintf(errbuf, "Cannot store fill value in the attribute \"_FillValue\".");
	      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(value);
	      return (status);
	    }
 
        }
 
      free( value);
       
    }
 
  /*
******************************************************************
*                  SET   UP  METADATA  STRINGS                   *
******************************************************************
*/
       
 
  /*  set up "DimList" string content */
  /*  ------------------------------- */
  strcpy(utlbuf,"");
  sprintf(utlbuf, "%s%s%s", za_name,":",dimlist);
 
  /* set up "MaxdimList"  string content */
  /* ----------------------------------- */
  if ( maxdimlist != NULL)
    {
      status = HE5_EHmetalist(maxdimlist,maxdimlstbuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot convert the input \"%s\" list to the metadata list. \n", maxdimlist);
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return (status);
	}
      sprintf(utlbuf2,"%s%s",":\n\t\t\t\tMaxdimList=", maxdimlstbuf);
      strcat(utlbuf,utlbuf2);
 
    }
  if (maxdimlist == NULL)
    {
      status = HE5_EHmetalist(dimlist,dimlstbuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot convert the input \"%s\" list to the metadata list.\n", dimlist);
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return (status);
	}
      sprintf(utlbuf2,"%s%s",":\n\t\t\t\tMaxdimList=", dimlstbuf);
      strcat(utlbuf,utlbuf2);
 
    }
 
  /* setup "CompressionType" & "CompressionParams" strings content */
  /* ------------------------------------------------------------- */
  if (compcode != HE5_HDFE_COMP_NONE)
    {
      sprintf(utlbuf2,"%s%s","\n\t\t\t\tCompressionType=", HDFcomp[compcode]);
 
      switch (compcode)
	{
	case HE5_HDFE_COMP_NBIT:
 
	  sprintf(compparmbuf,
		  "%s%d,%d,%d,%d%s",
		  "\n\t\t\t\tCompressionParams=(",
		  HE5_ZAXZa[idx].compparm[0],
		  HE5_ZAXZa[idx].compparm[1],
		  HE5_ZAXZa[idx].compparm[2],
		  HE5_ZAXZa[idx].compparm[3], ")");
	  strcat(utlbuf2, compparmbuf);
	  break;
 
 
	case HE5_HDFE_COMP_DEFLATE:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tDeflateLevel=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SZIP_CHIP:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SZIP_K13:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SZIP_EC:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SZIP_NN:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SZIP_K13orEC:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SZIP_K13orNN:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;

	case HE5_HDFE_COMP_SHUF_DEFLATE:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tDeflateLevel=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SHUF_SZIP_CHIP:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SHUF_SZIP_K13:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SHUF_SZIP_EC:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SHUF_SZIP_NN:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SHUF_SZIP_K13orEC:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;
 
	case HE5_HDFE_COMP_SHUF_SZIP_K13orNN:
 
	  sprintf(compparmbuf,"%s%d","\n\t\t\t\tBlockSize=", HE5_ZAXZa[idx].compparm[0]);
	  strcat(utlbuf2, compparmbuf);
	  break;

	default:
	  {
	    sprintf(errbuf,"Compression code \"%d\" is not supported. \n", compcode);
	    H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_ARGS, H5E_UNSUPPORTED, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	  }
 
	  break;
 
 
	}
 
      /* Concatanate compression parameters with compression code */
      /* -------------------------------------------------------- */
      strcat(utlbuf, utlbuf2);
    }
 
 
  /*
******************************************************************
*   Insert metadata information to Structural Metadata section   *
******************************************************************
*/
  numtype = HE5_EHdtype2numtype(numbertype);
  if (numtype == FAIL)
    {
      sprintf(errbuf, "Cannot get the number type ID. \n");
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  else if (numtype == 0)
    {
      metavalue = 0;
    }
  else
    {
      metavalue = HE5_EHhid2hsize(numtype);
      if (metavalue == 0)
	{
	  sprintf(errbuf, "Cannot convert \"hid_t\" to \"hsize_t\" data type. \n");
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }

  status = HE5_EHinsertmeta(fid, HE5_ZAXZa[idx].zaname, "z", 4L, utlbuf, &metavalue);
  if (status == FAIL)
    {
      sprintf(errbuf, "Cannot insert metadata for \"%s\" field.\n", za_name);
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
 
  /* Release data space ID */
  /* --------------------- */
  status =  H5Sclose(data_space);
  if (status == FAIL)
    {
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, "Cannot release the data space ID.\n");
      HE5_EHprint("Error: Cannot release the data space ID, occured", __FILE__, __LINE__);
      return(status);
    }
 
  /* Release property list ID */
  /* ------------------------ */
  status = H5Pclose(HE5_ZAXZa[idx].plist);
  if (status == FAIL)
    {
      H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_PLIST, H5E_CLOSEERROR, "Cannot release the property list ID.\n");
      HE5_EHprint("Error: Cannot release the property list ID, occured", __FILE__, __LINE__);
      return(status);
    }
 
  /* Reset external array entry for next dataset creation */
  /* ==================================================== */
  HE5_ZAXZa[idx].plist    = FAIL;
  HE5_ZAXZa[idx].compcode = HE5_HDFE_COMP_NONE;
 
  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_C_S1))
    {
      attr = HE5T_CHARSTRING;
      status = HE5_ZAwritelocattr(zaID, za_name, "ARRAYOFSTRINGS", H5T_NATIVE_INT, count, &attr);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write attribute to the field \"%s\".",za_name) ;
	  H5Epush(__FILE__, "HE5_ZAdefinefield", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}
 
    }
 
 COMPLETION:
  return(status);
}
 


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefine                                                     |
|                                                                             |
|  DESCRIPTION: Defines data field within ZA structure (wrapper)              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  za_name        const char*         za name                                 |
|  dimlist        char*               Dimension (comma-separated)list         |
|  maxdimlist     char*               Maximum Dimension (comma-separated) list|
|  dtype          hid_t               field data type ID                      |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAdefine(hid_t zaID, const char *za_name, char *dimlist, char *maxdimlist, hid_t dtype)
{
  herr_t            status     = FAIL;          /* routine return status variable */
 
  HE5_LOCK;
  CHECKNAME(za_name);
  CHECKPOINTER(dimlist);
 
  /* Call HE5_ZAdefinefield routine  */
  /* ==============================  */
  status = HE5_ZAdefinefield(zaID, "Data Fields", za_name, dimlist,maxdimlist,dtype);
 
 COMPLETION:
  HE5_UNLOCK;
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwritedatameta                                              |
|                                                                             |
|  DESCRIPTION: Defines structural metadata for pre-existing data             |
|               field within za structure                                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      const char*         fieldname                               |
|  dimlist        char*               Dimension list (comma-separated list)   |
|  mvalue         hid_t               metavalue to insert                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  May 05    S.Zhao       Added HE5_EHdtype2numtype() function call.          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAwritedatameta(hid_t zaID, const char *fieldname, char *dimlist, hid_t mvalue)
{
  herr_t        status     = FAIL;          /* Return status variable */

  hid_t         fid        = FAIL;          /* HDF-EOS file ID        */
  hid_t         gid        = FAIL;          /* "HDFEOS" group ID      */

  hsize_t       metavalue  =  0;            /* Metavalue to insert    */

  long          idx        = FAIL;          /* za index               */

  char          errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
  char          utlbuf[HE5_HDFE_DIMBUFSIZE];/* Utility buffer         */
  hid_t         numtype    = FAIL;          /* Number type ID         */

  HE5_LOCK;
  CHECKNAME(fieldname);
  CHECKPOINTER(dimlist);
 
  numtype = HE5_EHdtype2numtype(mvalue);
  if (numtype == FAIL)
    {
      sprintf(errbuf, "Cannot get the number type ID. \n");
      H5Epush(__FILE__, "HE5_ZAwritedatameta", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  else if (numtype == 0)
    {
      metavalue = 0;
    }
  else
    {
      /* Convert 'numtype' to 'metavalue' */
      /* -------------------------------- */
      metavalue = HE5_EHhid2hsize(numtype);
      if ( metavalue == 0 )
	{
	  sprintf(errbuf, "Cannot convert metadata value. \n");
	  H5Epush(__FILE__, "HE5_ZAwritedatameta", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAwritedatameta", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Setup and write field metadata */
      /* ------------------------------ */
      sprintf(utlbuf, "%s%s%s", fieldname, ":", dimlist);
      status = HE5_EHinsertmeta(gid, HE5_ZAXZa[idx].zaname, "z", 4L, utlbuf, &metavalue);
      if ( status == FAIL )
	{
	  sprintf(errbuf, "Cannot insert metadata value. \n");
	  H5Epush(__FILE__, "HE5_ZAwritedatameta", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwriteattr                                                  |
|                                                                             |
|  DESCRIPTION: Writes/updates attribute in a za.                             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  attrname       const char*         attribute name                          |
|  numbertype     hid_t               attribute dataset datatype ID           |
|  count[]        hsize_t             Number of attribute elements            |
|  datbuf         void*               I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_ZAwriteattr(hid_t zaID, const char *attrname, hid_t numtype, hsize_t count[], void *datbuf)
{
  herr_t     status     = FAIL;          /* Return status variable  */

  hid_t      fid        = FAIL;          /* HDF-EOS file ID         */
  hid_t      gid        = FAIL;          /* "HDFEOS" group ID       */

  long       idx        = FAIL;          /* za index                */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer    */
  hid_t      numbertype;

  HE5_LOCK;
  CHECKNAME(attrname);
  CHECKPOINTER(count);
  CHECKPOINTER(datbuf);

  /* 
     if numtype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use numtype itself
  */
  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      numbertype = numtype;
    }
  
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAwriteattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Call HE5_EHattr to perform I/O */
      /* ------------------------------ */
      status = HE5_EHattr(HE5_ZAXZa[idx].za_id, attrname, numbertype, count,"w", datbuf);
      if ( status == FAIL )
	{
	  sprintf(errbuf, "Cannot write the attribute value. \n");
	  H5Epush(__FILE__, "HE5_ZAwriteattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
    }
  
 COMPLETION:
  HE5_UNLOCK;
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAreadattr                                                   |
|                                                                             |
|  DESCRIPTION: Reads attribute from a za.                                    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  attrname       const char*         attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void*               I/O buffer                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAreadattr(hid_t zaID, const char *attrname, void *datbuf)
{
  herr_t      status  = FAIL;             /* Return status variable  */

  hid_t       fid     = FAIL;             /* HDF-EOS file ID         */
  hid_t       gid     = FAIL;             /* "HDFEOS" group ID       */
  hid_t       ntype   = FAIL;             /* Data type ID            */  

  long        idx     = FAIL;             /* za index                */
  
  hsize_t     count[] ={0};               /* Number of elements      */

  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer    */

  HE5_LOCK;
  CHECKPOINTER(attrname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAreadattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Call HE5_EHattr to perform I/O */
      /* ------------------------------ */
      status = HE5_EHattr(HE5_ZAXZa[idx].za_id, attrname, ntype, count,"r", datbuf);
      if ( status == FAIL )
	{
	  sprintf(errbuf, "Cannot read the attribute value. \n");
	  H5Epush(__FILE__, "HE5_ZAreadattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAattrinfo                                                   |
|                                                                             |
|  DESCRIPTION: Returns information about a za attribute                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  attrname       const char*         attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          hid_t*              Data type ID                            |
|  count          hsize_t*            Number of attribute elements            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date       Programmer   Description                                       |
|  ========   ============  ================================================= |
|  Jul 02      S.ZHAO       Original development                              |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAattrinfo(hid_t zaID, const char *attrname, hid_t *ntype, hsize_t *count)
{
  herr_t        status  = FAIL;             /* Return status variable  */

  hid_t         fid     = FAIL;             /* HDF-EOS file ID         */
  hid_t         gid     = FAIL;             /* "HDFEOS" group ID       */

  long          idx     = FAIL;             /* za index                */

  char          errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer    */

  CHECKPOINTER(attrname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAattrinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get attribute group ID and call HE5_EHattrinfo */
      /* ---------------------------------------------- */
      status = HE5_EHattrinfo(HE5_ZAXZa[idx].za_id, attrname, ntype, count);
      if ( status == FAIL )
	{
	  sprintf(errbuf, "Cannot retrieve information about attribute. \n");
	  H5Epush(__FILE__, "HE5_ZAattrinfo", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}
    }

 COMPLETION:
  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqattrs                                                   |
|                                                                             |
|  DESCRIPTION: Retrieves information about attributes defined in ZA          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in za struct       |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char*               Attribute names in za struct            |
|                                     (Comma-separated list)                  |
|  strbufsize     long*               Attributes name list string length      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqattrs(hid_t zaID, char *attrnames, long *strbufsize)
{
  long            nattr    = FAIL;/* Number of attributes (return)    */
  long            idx      = FAIL;/* za index                         */

  herr_t          status   = FAIL;/* routine return status variable   */

  hid_t           fid      = FAIL;/* HDF-EOS file ID                  */
  hid_t           gid      = FAIL;/* za group ID                      */

  char            *grpname = (char *)NULL;/* Group name string        */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */


  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinqattrs", &fid, &gid, &idx);
  if(status == SUCCEED )
    {
      grpname = (char *)calloc( HE5_HDFE_NAMBUFSIZE, sizeof(char) );
      if( grpname == NULL)
        {
          H5Epush(__FILE__, "HE5_ZAinqattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory");
          HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
          return(FAIL);
        }

      strcpy(grpname,"/HDFEOS/ZAS/");
      strcat(grpname, HE5_ZAXZa[idx].zaname);

      nattr = HE5_EHattrcat(fid,  grpname, attrnames, strbufsize);
      if ( nattr == FAIL )
	{
	  sprintf(errbuf, "Cannot find the attributes. \n");
	  H5Epush(__FILE__, "HE5_ZAinqattrs", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	}

      if (grpname != NULL) free(grpname);

    }

  return (nattr);
}


#define REMQUOTE					\
							\
  memmove(utlstr, utlstr + 1, strlen(utlstr) - 2);	\
  utlstr[strlen(utlstr) - 2] = 0;



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqdims                                                    |
|                                                                             |
|  DESCRIPTION: Returns dimension names and values defined in ZA structure    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nDim           long                Number of defined dimensions            |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  dimlist        char*               Dimension names (comma-separated)       |
|  dims           hsize_t             Dimension values                        |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAinqdims(hid_t zaID, char *dimlist, hsize_t dims[])
{
  herr_t         status    = FAIL;/* routine return status variable      */
 
  hid_t          fid       = FAIL;/* HDF-EOS file ID                     */
  hid_t          gid       = FAIL;/* "HDFEOS" group ID                   */
 
  long           idx       = FAIL;/* ZA index                         */
  long           nDim      =  0;  /* Number of dimensions                */
 
  hsize_t        size      =  0;  /* Dimension size                      */
 
  char           *metabuf = (char *)NULL;     /* Pointer to SM           */
  char           *metaptrs[2]={NULL,NULL};    /* Ptrs to start/end of SM */
  char           utlstr[HE5_HDFE_UTLBUFSIZE]; /* Utility string          */
  char           errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer    */
 
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and ZA index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinqdims", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for ZA ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAinqdims", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* If dimension names or sizes are desired ... */
  /* ------------------------------------------- */
  if (dimlist != NULL || dims != NULL)
    {
      /* Get pointers to "Dimension" section within SM */
      /* --------------------------------------------- */
      metabuf = (char *)HE5_EHmetagroup(fid, HE5_ZAXZa[idx].zaname, "z", "Dimension", metaptrs);
 
      /* If dimension names are desired then "clear" name buffer */
      /* ------------------------------------------------------- */
      if (dimlist != NULL)
	dimlist[0] = 0;
 
      /* Begin loop through dimension entries in metadata */
      /* ------------------------------------------------ */
      while (1)
	{
	  /* Search for OBJECT string */
	  /* ------------------------ */
	  metaptrs[0] = strstr(metaptrs[0], "\t\tOBJECT=");
	  /* If found within "Dimension" metadata section ... */
	  /* ------------------------------------------------ */
	  if (metaptrs[0] < metaptrs[1] && metaptrs[0] != NULL)
	    {
	      /* Get Dimension Name (if desired) */
	      /* ------------------------------- */
	      if (dimlist != NULL)
		{
		  /* Check 1st for old meta data then new */
		  /* ------------------------------------ */
		  HE5_EHgetmetavalue(metaptrs, "OBJECT", utlstr);
 
		  /*
		   * If OBJECT value begins with double quote then old
		   * metadata, dimension name is OBJECT value.
		   * Otherwise search for "DimensionName" string
		   */
		  if (utlstr[0] != '"')
		    {
		      metaptrs[0] = strstr(metaptrs[0], "\t\t\t\tDimensionName=");
		      HE5_EHgetmetavalue(metaptrs, "DimensionName", utlstr);
		    }
 
		  /* Strip off double quotes */
		  /* ----------------------- */
		  REMQUOTE
 
		    /* If not first name then add comma delimitor */
		    /* ------------------------------------------ */
		    if (nDim > 0)
		      {
			strcat(dimlist, ",");
		      }
 
		  /* Add dimension name to dimension list */
		  /* ------------------------------------ */
		  strcat(dimlist, utlstr);
		}
 
	      /* Get Dimension Size (if desired) */
	      /* ------------------------------- */
	      if (dims != NULL)
		{
		  HE5_EHgetmetavalue(metaptrs, "Size", utlstr);
		  size = (hsize_t)atol(utlstr);
		  dims[ nDim ] = size;
		}
	      /* Increment dimension counter */
	      /* --------------------------- */
	      nDim++;
	    }
	  else
	    /* No more dimensions found */
	    /* ------------------------ */
	    break;
	}
 
      free(metabuf);
    }
 
  return(nDim);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqfield                                                   |
|                                                                             |
|  DESCRIPTION: Returns fieldnames, ranks and numbertypes defined in ZA.      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nFld           long                Number of data fields in ZA             |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  datafield      char*               data field                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fieldlist      char*               Field names (comma-separated)           |
|  rank           int                 Array of ranks                          |
|  numbertype     hid_t               Array of HDF number types               |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Oct 03    S.ZHAO       Added the H5Tclose(datatype) call.                  |
|  Jan 04    S.Zhao       Modified to enable a character string dataset.      |
|  Mar 04    S.Zhao       Modified for a character string dataset.            |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static long
HE5_ZAinqfield(hid_t zaID, char *datafield, char *fieldlist, int rank[], hid_t numbertype[])
{
  long          nFld       =  0;       /* Return number of mappings  */
  long          i          =  0;       /* Loop index                 */
  long          idx        =  FAIL;    /* za index                   */
  long          ntflds     =  0;       /* field counter              */
 
  herr_t        status     = FAIL;/* routine return status variable  */
  herr_t        Dstatus    = FAIL;/* Status for H5Dclose             */
 
  hid_t         fid        = FAIL;/* HDF-EOS file ID                 */
  hid_t         gid        = FAIL;/* "HDFEOS" group ID               */
  hid_t         datasetid  = FAIL;/* Dataset ID                      */
  hid_t         datatype   = FAIL;/* Data type ID                    */
  hid_t         groupid    = FAIL;/* Group ID                        */
 
  H5T_class_t   classid    = H5T_NO_CLASS;   /* Data type class ID   */
 
  size_t        slen[HE5_DTSETRANKMAX];      /* String length array  */
  size_t        fldnmlen[HE5_FLDNUMBERMAX];  /* Array of namelengths */
 
  char          *fldnm[HE5_FLDNUMBERMAX];    /* Array of names       */
  char          *metabuf = (char *)NULL;     /* Pntr to SM           */
  char          *metaptrs[2]={NULL,NULL};    /* Start/end of SM      */
  char          utlstr[HE5_HDFE_UTLBUFSIZE]; /* Utility string       */
  char          utlstr2[HE5_HDFE_UTLBUFSIZE];/* Utility string 2     */
  char          *ptr[HE5_DTSETRANKMAX];      /* String pointer array */
  char          *tempfield = (char *)NULL;
  char          errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer */
  htri_t        str_is_variable;             /* boolean: TRUE if 
						string is variable 
						lengeth FALSE if 
						string is fixed length
						-1 if error in 
						H5Tis_variavle_str() */
 
  CHECKPOINTER(datafield);
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinqfield", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Initialize slen[] array */
  /* ----------------------- */
  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    slen[i] = 0;
 
  /* Initialize fldnmlen[] array */
  /* --------------------------- */
  for (i = 0; i < HE5_FLDNUMBERMAX; i++)
    fldnmlen[i] = 0;
 
 
  /* If field names, ranks,  or number types desired ... */
  /* --------------------------------------------------- */
  if (fieldlist != NULL || rank != NULL || numbertype != NULL)
    {
      /* Get pointers to "DataField" section within SM */
      /* --------------------------------------------- */
      metabuf = (char *)HE5_EHmetagroup( fid, HE5_ZAXZa[idx].zaname, "z", "DataField", metaptrs);
      strcpy(utlstr2, "DataFieldName");
         
 
      /* If field names are desired then "clear" name buffer */
      /* --------------------------------------------------- */
      if (fieldlist != NULL)
	fieldlist[0] = 0;
         
      /* Begin loop through mapping entries in metadata */
      /* ---------------------------------------------- */
      while (1)
	{
	  /* Search for OBJECT string */
	  /* ------------------------ */
	  metaptrs[0] = strstr(metaptrs[0], "\t\tOBJECT=");
 
	  /* If found within "Data" Field metadata section ... */
	  /* ------------------------------------------------- */
	  if (metaptrs[0] < metaptrs[1] && metaptrs[0] != NULL)
	    {
	      /* Get field names (if desired) */
	      /* ---------------------------- */
	      if (fieldlist != NULL)
		{
		  /* Check 1st for old meta data then new */
		  /* ------------------------------------ */
		  HE5_EHgetmetavalue(metaptrs, "OBJECT", utlstr);
 
		  /*
		   * If OBJECT value begins with double quote then old
		   * metadata, field name is OBJECT value. Otherwise
		   * search for "DataFieldName" string
		   */
 
		  if (utlstr[0] != '"')
		    {
		      strcpy(utlstr, "\t\t\t\t");
		      strcat(utlstr, utlstr2);
		      strcat(utlstr, "=");
		      metaptrs[0] = strstr(metaptrs[0], utlstr);
		      HE5_EHgetmetavalue(metaptrs, utlstr2, utlstr);
		    }
 
		  /* Strip off double quotes */
		  /* ----------------------- */
		  REMQUOTE
 
		    /* Add to fieldlist */
		    /* ---------------- */
		    if (nFld > 0)
		      strcat(fieldlist, ",");
 
		  strcat(fieldlist, utlstr);
 
		}
 
	      /* If numbertype  are desired then "clear" name buffer */
	      /* --------------------------------------------------- */
	      if (numbertype != NULL)
		{
		  ntflds = HE5_EHparsestr(fieldlist, ',', fldnm,fldnmlen);
		  if(ntflds != FAIL)
		    {
		      groupid = HE5_ZAXZa[idx].data_id;
 
		      for(i = 0; i < ntflds; i++)
			{
			  tempfield      = (char *)calloc(fldnmlen[i] + 1, sizeof(char));
			  memmove(tempfield,fldnm[i],fldnmlen[i]);
 
			  /* Open the dataset */
			  /* ---------------- */
			  datasetid = H5Dopen(groupid, tempfield);
			  if ( datasetid == FAIL )
			    {
			      sprintf(errbuf, "Cannot open the dataset \"%s\". \n", tempfield);
			      H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
			      HE5_EHprint(errbuf, __FILE__, __LINE__);
			      if (metabuf != NULL) free(metabuf);
			      free(tempfield);
			      return(FAIL);
			    }
 
			  /* Get the data type */
			  /* ----------------- */
			  datatype = H5Dget_type(datasetid);
			  if ( datatype == FAIL )
			    {
			      sprintf(errbuf, "Cannot get the data type ID. \n");
			      H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
			      HE5_EHprint(errbuf, __FILE__, __LINE__);
			      if (metabuf != NULL) free(metabuf);
			      free(tempfield);
			      return(FAIL);
			    }
 
			  /* Get the data type class ID */
			  /* -------------------------- */
			  classid        = H5Tget_class(datatype);
			  if ( classid == H5T_NO_CLASS )
			    {
			      sprintf(errbuf, "Cannot get the data type class ID. \n");
			      H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
			      HE5_EHprint(errbuf, __FILE__, __LINE__);
			      if (metabuf != NULL) free(metabuf);
			      free(tempfield);
			      return(FAIL);
			    }
 
			  if (classid == H5T_STRING)
			    {
			      /* HE5T_CHARSTRING has variable length for data fields */
			      str_is_variable = H5Tis_variable_str(datatype);
			      if(str_is_variable == TRUE)
				{
				  numbertype[i] = HE5T_CHARSTRING;
				}
			      else if(str_is_variable == FALSE)
				{
				  numbertype[i] = HE5T_NATIVE_CHAR;
				}
			      else
				{
				  status = FAIL;
				  sprintf(errbuf, "Failed to see if string field is varaible or fixed length.\n");
				  H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
							  
				  HE5_EHprint(errbuf, __FILE__, __LINE__);
				  if (metabuf != NULL) free(metabuf);
				  free(tempfield);
				  return(status);
				}
						      
			      /*HE5_ZAgetstringtype(zaID, fieldname,classid);*/
			      /*numbertype[i] = HE5T_CHARSTRING;*/
			    }
			  else
			    {
			      numbertype[i] = HE5_EHdtype2numtype(datatype);
			      if ( numbertype[i] == FAIL )
				{
				  sprintf(errbuf, "Cannot get the number type ID. \n");
				  H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
				  HE5_EHprint(errbuf, __FILE__, __LINE__);
				  if (metabuf != NULL) free(metabuf);
				  free(tempfield);
				  return(FAIL);
				}
			    }
						  
			  /* Release the datatype ID */
			  /* ----------------------- */
			  status = H5Tclose(datatype);
			  if ( status == FAIL )
			    {
			      sprintf(errbuf, "Cannot close the datatype \"%s\". \n", tempfield);
			      H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
			      HE5_EHprint(errbuf, __FILE__, __LINE__);
			      if (metabuf != NULL) free(metabuf);
			      free(tempfield);
			      return(FAIL);
			    }
 
			  /* Release the dataset ID */
			  /* ---------------------- */
			  Dstatus        = H5Dclose(datasetid);
			  if ( Dstatus == FAIL )
			    {
			      sprintf(errbuf, "Cannot close the dataset \"%s\". \n", tempfield);
			      H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
			      HE5_EHprint(errbuf, __FILE__, __LINE__);
			      if (metabuf != NULL) free(metabuf);
			      free(tempfield);
			      return(FAIL);
			    }
			  free(tempfield);
			}
		    }
		}
 
 
	      /*
	       * Get Rank (if desired) by counting # of dimensions in
	       * "DimList" string
	       */
	      if (rank != NULL)
		{
		  HE5_EHgetmetavalue(metaptrs, "DimList", utlstr);
		  rank[nFld] = HE5_EHlong2int(HE5_EHparsestr(utlstr, ',', ptr, slen) );
		  if (rank[nFld] == FAIL )
		    {
		      sprintf(errbuf, "Cannot convert data type. \n");
		      H5Epush(__FILE__, "HE5_ZAinqfield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      if(metabuf != NULL) free(metabuf);
		      return(FAIL);
		    }
		}
 
	      /* Increment field counter */
	      /* ----------------------- */
	      nFld++;
 
	    }
	  else
	    /* No more fields found */
	    /* -------------------- */
	    break;
	}
         
      if (metabuf != NULL) free(metabuf);
    }
 
 COMPLETION: 
  return(nFld);
}
 
 

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinquire                                                    |
|                                                                             |
|  DESCRIPTION: Inquires about data fields in za                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nflds          long                Number of data fields in za             |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  za_name_list   char*               Field names (comma-separated)           |
|  rank           int                 Array of ranks                          |
|  dtype          hid_t               data type ID                            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAinquire(hid_t zaID, char *za_name_list, int rank[], hid_t dtype[])
{
  long            nflds   = FAIL;             /* Number of Data fields */
 
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */
 
 
  /* Call "HE5_ZAinqfield" routine to get number of fields */
  /* ----------------------------------------------------- */
  nflds = HE5_ZAinqfield(zaID, "Data Fields", za_name_list, NULL, NULL);
  if (nflds == FAIL)
    {
      sprintf(errbuf, "Cannot get the number of fields in \"Data Fields\" group. \n");
      H5Epush(__FILE__, "HE5_ZAinquire", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Call "HE5_ZAinqfield" routine to get other parameters */
  /* ----------------------------------------------------- */
  nflds = HE5_ZAinqfield(zaID, "Data Fields", za_name_list, rank, dtype);
  if (nflds == FAIL)
    {
      sprintf(errbuf, "Inquiry about fields in \"Data Fields\" group fails. \n");
      H5Epush(__FILE__, "HE5_ZAinquire", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  return(nflds);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqdatatype                                                |
|                                                                             |
|  DESCRIPTION: Inquires about data field in za                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t               return status (0) SUCCEED, (-1) FAIL   |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t                                                       |
|  fieldname      const char*                                                 |
|  attrname       const char*                                                 |
|  group          int                 Group flag:                             |
|                                       ...DATAGROUP - for the field          |
|                                       ...ATTRGROUP, ...GRPATTRGROUP,        |
|                                       ...LOCATTRGROUP - for the attribute   |
|  OUTPUTS:                                                                   |
|  datatype       hid_t*                                                      |
|  classid        H5T_class_t*                                                |
|  order          H5T_order_t*                                                |
|  size           size_t*                                                     |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAinqdatatype(hid_t zaID, const char *fieldname, const char *attrname, int group, hid_t *datatype, H5T_class_t *classid, H5T_order_t *order, size_t *size)
{
 
  herr_t      status     = FAIL;	      /* routine return status */

  int         fldgroup   = FAIL;              /* Field group flag      */

  hid_t       datasetid  = FAIL;	      /* Dataset ID            */
  hid_t       fid        = FAIL;	      /* HDF-EOS file ID       */
  hid_t       gid        = FAIL;	      /* "HDFEOS" group ID     */
  hid_t       attr       = FAIL;	      /* attribute dataset ID  */
  hid_t       fldgroupID = FAIL;	      /* Field group ID        */
  hid_t       fieldID    = FAIL;	      /* Field dataset ID      */         

  long        idx        = FAIL;	      /* za index              */

  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */


  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinqdatatype", &fid, &gid, &idx);
  if(status == SUCCEED)
    {
      if(group == HE5_HDFE_DATAGROUP)
	fldgroupID = HE5_ZAXZa[idx].data_id;
      else if(group == HE5_HDFE_ATTRGROUP)
	fldgroupID = HE5_ZAXZa[idx].za_id;
      else if(group == HE5_HDFE_GRPATTRGROUP)
	fldgroupID = HE5_ZAXZa[idx].data_id;
      else if(group == HE5_HDFE_LOCATTRGROUP)
	{
	  /* Get the field group flag */
	  /* ------------------------ */
	  fldgroup = HE5_ZAfldsrch(zaID,(char *)fieldname,NULL,NULL,NULL,NULL);
	  if(fldgroup == FAIL)
	    {
	      sprintf(errbuf,"Cannot get the group flag for \"%s\" field.\n",fieldname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
		  
	  if (fldgroup == HE5_HDFE_DATAGROUP)
	    fldgroupID = HE5_ZAXZa[idx].data_id;

	}
      else
	{
	  sprintf(errbuf, "Invalid group flag (\"%d\"). \n", group);
	  H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_BADVALUE , errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
		
      /* Get data type information for the field */
      /* --------------------------------------- */
      if( group == HE5_HDFE_DATAGROUP )
	{
	  datasetid = H5Dopen(fldgroupID, fieldname);
	  if (datasetid == FAIL)
	    {
	      sprintf(errbuf, "Cannot open the dataset for the \"%s\" field.\n", fieldname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATASET, H5E_NOTFOUND , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }
			
	  *datatype = H5Dget_type(datasetid);
	  if (*datatype == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the datatype for the \"%s\" field.\n", fieldname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	   
	      return(FAIL);
	    }

	  *classid  = H5Tget_class(*datatype);
	  if (*classid == H5T_NO_CLASS)
	    {
	      sprintf(errbuf, "Cannot get the data type class ID for the \"%s\" field.\n", fieldname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	   
	      return(FAIL);
	    }

	  *order    = H5Tget_order(*datatype);
	  if (*order == H5T_ORDER_ERROR)
	    {
	      sprintf(errbuf, "Cannot get the data type order for the \"%s\" field.\n", fieldname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }
			
	  if (*classid == H5T_STRING)
	    {
	      *size = H5Tget_size(*datatype);
	    }
	  else
	    {
	      *size     = H5Tget_size(*datatype);
	      if (*size == 0)
		{
		  sprintf(errbuf, "Cannot get the data type size for the \"%s\" field.\n", fieldname);
		  H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
				
		  return(FAIL);
		}
	    }
			
	  status = H5Dclose(datasetid);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataset ID for the \"%s\" field.\n", fieldname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }
	}
      /* Get data type information about global/group attribute */
      /* ------------------------------------------------------ */
      else if(group == HE5_HDFE_ATTRGROUP || group == HE5_HDFE_GRPATTRGROUP)
	{
	  attr      = H5Aopen_name(fldgroupID, attrname);
	  if (attr == FAIL)
	    {
	      sprintf(errbuf, "Cannot open the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	   
	      return(FAIL);
	    }

	  *datatype = H5Aget_type(attr);
	  if (*datatype == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the data type for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }
			
	  *classid  = H5Tget_class(*datatype);
	  if (*classid == H5T_NO_CLASS)
	    {
	      sprintf(errbuf, "Cannot get the data type class ID for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	   
	      return(FAIL);
	    }

	  *order    = H5Tget_order(*datatype);
	  if (*order == H5T_ORDER_ERROR)
	    {
	      sprintf(errbuf, "Cannot get the data type order for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }

	  if (*classid == H5T_STRING)
	    {
	      *size = H5Tget_size(*datatype);
	    }
	  else
	    {
	      *size     = H5Tget_size(*datatype);
	      if (*size == 0)
		{
		  sprintf(errbuf, "Cannot get the data type size for the \"%s\" attribute.\n", attrname);
		  H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
				
		  return(FAIL);
		}
	    }
			
	  status = H5Aclose(attr);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataset ID for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }
			
	}
      /* Get data type information for local attribute */
      /* --------------------------------------------- */
      else if(group == HE5_HDFE_LOCATTRGROUP)
	{

	  fieldID = H5Dopen(fldgroupID, fieldname);
	  if(fieldID == FAIL)
	    {
	      sprintf(errbuf,"Cannot open the \"%s\" field dataset.\n", fieldname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  attr = H5Aopen_name(fieldID, attrname);
	  if (attr == FAIL)
	    {
	      sprintf(errbuf, "Cannot open the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	   
	      return(FAIL);
	    }

	  *datatype = H5Aget_type(attr);
	  if (*datatype == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the data type for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }
			
	  *classid  = H5Tget_class(*datatype);
	  if (*classid == H5T_NO_CLASS)
	    {
	      sprintf(errbuf, "Cannot get the data type class ID for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	   
	      return(FAIL);
	    }

	  *order    = H5Tget_order(*datatype);
	  if (*order == H5T_ORDER_ERROR)
	    {
	      sprintf(errbuf, "Cannot get the data type order for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }

	  if (*classid == H5T_STRING)
	    {
	      *size = H5Tget_size(*datatype);
	    }
	  else
	    {
	      *size     = H5Tget_size(*datatype);
	      if (*size == 0)
		{
		  sprintf(errbuf, "Cannot get the data type size for the \"%s\" attribute.\n", attrname);
		  H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_FUNC, H5E_CANTINIT , errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
				
		  return(FAIL);
		}
	    }
			
	  status = H5Aclose(attr);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataset ID for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR , errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
				
	      return(FAIL);
	    }

	  status = H5Dclose(fieldID);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the dataset ID for the \"%s\" attribute.\n", attrname);
	      H5Epush(__FILE__, "HE5_ZAinqdatatype", __LINE__, H5E_DATASET, H5E_CLOSEERROR , errbuf);
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
|  FUNCTION: HE5_ZAchunkinfo                                                  |
|                                                                             |
|  DESCRIPTION: Retrieves chunking information                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t  None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fldname        char*               name of field                           |
|                                                                             |
|  OUTPUTS:                                                                   |
|  chunk_rank     int*                rank of a dataset                       |
|  chunk_dims     hsize_t             the array containing sizes              |
|                                     of each dimension of a chunk            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|  Date        Programmer    Description                                      |
|  =========   ============  ==============================================   |
|  Dec 03      S.Zhao        Original development                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAchunkinfo(hid_t zaID, char *fldname, int *chunk_rank,  hsize_t chunk_dims[])
{
  herr_t          status   = FAIL;/* Return status variable */
 
  hid_t           fid      = FAIL;/* HDF-EOS file ID        */
  hid_t           gid      = FAIL;/* "HDFEOS" group ID      */
  long            idx      = FAIL;/* ZA index               */
  hid_t           fieldID  = FAIL;/* field dataset ID       */
  hid_t           plist    = FAIL;/* field dataset property list ID   */
  hid_t           dspace   = FAIL;/* "fieldname" Dataspace ID         */
 
  int             ndims    = 0;   /* Number of dimensions   */
 
  H5D_layout_t    layout = H5D_LAYOUT_ERROR; /* Storage layout type   */
 
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */
 
 
  HE5_LOCK;
  CHECKPOINTER(fldname);
 
  /* Initialize chunk_rank to FAIL (error) */
  /* ------------------------------------- */
  *chunk_rank = FAIL;
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and ZA index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAchunkinfo", &fid, &gid, &idx);
  if (status == FAIL)
    {
      sprintf(errbuf,"Checking for valid ZA ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
 
  /* Check out the field group and get field ID */
  /* ------------------------------------------ */
  HE5_ZAfldsrch(zaID,fldname,&fieldID,NULL,NULL,NULL);
  if(fieldID == FAIL)
    {
      sprintf(errbuf, "Cannot get the field ID for the \"%s\" field.\n", fldname);
      H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Get the property list ID */
  /* ------------------------ */
  plist = H5Dget_create_plist(fieldID);
  if (plist == FAIL)
    {
      sprintf(errbuf, "Cannot get the property list ID for the \"%s\" data field.\n", fldname);
      H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_PLIST, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Get the data space ID */
  /* --------------------- */
  dspace = H5Dget_space(fieldID);
  if(dspace == FAIL)
    {
      sprintf(errbuf, "Cannot get the data space ID for the \"%s\" data field.\n", fldname);
      H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Get the dataset rank */
  /* -------------------- */
  ndims  = H5Sget_simple_extent_ndims(dspace);
  if(ndims == FAIL)
    {
      sprintf(errbuf, "Cannot get the rank of the dataset.\n");
      H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Get Layout */
  /* ---------- */
  layout = H5Pget_layout(plist);
  if (layout == H5D_LAYOUT_ERROR)
    {
      sprintf(errbuf, "Cannot get the layout of the raw data. \n");
      H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_PLIST, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  else if (layout == H5D_CHUNKED)
    {
      /* Get chunking */
      /* ------------ */
      *chunk_rank = H5Pget_chunk(plist, ndims, chunk_dims);
      if (*chunk_rank == FAIL)
	{
	  sprintf(errbuf, "Cannot get the sizes of chunks. \n");
	  H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }
  else
    {
      *chunk_rank = 0;
    }
 
  /* Release property list ID */
  /* ------------------------ */
  status = H5Pclose(plist);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release property list ID. \n");
      H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Release the data space ID */
  /* ------------------------- */
  status = H5Sclose(dspace);
  if( status == FAIL)
    {
      sprintf(errbuf, "Cannot release the data space ID.\n");
      H5Epush(__FILE__, "HE5_ZAchunkinfo", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
 
 COMPLETION:
  HE5_UNLOCK;
  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAnentries                                                   |
|                                                                             |
|  DESCRIPTION: Returns number of entries and string buffer size              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nEntries       long                Number of entries                       |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  entrycode      int                 Entry code:                             |
|                                                                             |
|                                     HE5_HDFE_NENTDIM  (0)                   |
|                                     HE5_HDFE_NENTMAP  (1)                   |
|                                     HE5_HDFE_NENTIMAP (2)                   |
|                                     HE5_HDFE_NENTDFLD (4)                   |
|                                                                             |
|  OUTPUTS:                                                                   |
|  strbufsize     long*               Length of comma-separated list          |
|                                     (Does not include null-terminator)      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAnentries(hid_t zaID, int entrycode, long *strbufsize)
{
  herr_t          status     = FAIL;/* routine return status variable  */  
  
  hid_t           fid        = FAIL;/* HDF-EOS file ID                 */
  hid_t           gid        = FAIL;/* "HDFEOS" group ID               */
  
  long            idx        = FAIL;/* za index                        */
  long            nEntries   = 0;   /* Number of entries               */
  long            metaflag   = FAIL;/* Old (0), New (1) metadata flag) */
  long            nVal       = 0;   /* Number of strings to search for */
  long            i;                /* Loop index                      */
  
  char            *metabuf = (char *)NULL;    /* Ptr to SM             */
  char            *metaptrs[2]={NULL,NULL};   /* Start/end of SM       */
  char            utlstr[HE5_HDFE_UTLBUFSIZE];/* Utility string        */
  char            valName[2][32];             /* Strings to search for */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */
  


  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAnentries", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Zero out string buffer size */
      /* --------------------------- */
      *strbufsize = 0;
       
      /*
       * Get pointer to relevant section within SM and Get names of
       * metadata strings to inquire about
       */
      switch (entrycode)
        {
        case HE5_HDFE_NENTDIM:
	  /* Dimensions */
	  /* ---------- */
	  {
	    metabuf = (char *)HE5_EHmetagroup(fid, HE5_ZAXZa[idx].zaname, "z", "Dimension", metaptrs);
	    nVal = 1;
	    strcpy(&valName[0][0], "DimensionName");
	  }
	  break;
		  
        case HE5_HDFE_NENTMAP:
	  /* Dimension Maps */
	  /* -------------- */
	  {
	    metabuf = (char *)HE5_EHmetagroup(fid, HE5_ZAXZa[idx].zaname, "z", "DimensionMap", metaptrs);
	    nVal = 1;
	    strcpy(&valName[0][0], "DataDimension");
	  }
	  break;

        case HE5_HDFE_NENTIMAP:
	  /* Indexed Dimension Maps */
	  /* ---------------------- */
	  {
	    metabuf = (char *)HE5_EHmetagroup(fid, HE5_ZAXZa[idx].zaname, "z","IndexDimensionMap", metaptrs);
	    nVal = 1;
	    strcpy(&valName[0][0], "DataDimension");
	  }
	  break;
		  
        case HE5_HDFE_NENTDFLD:
	  /* Data Fields */
	  /* ----------- */
	  {
	    metabuf = (char *)HE5_EHmetagroup(fid, HE5_ZAXZa[idx].zaname, "z", "DataField", metaptrs);
	    nVal = 1;
	    strcpy(&valName[0][0], "DataFieldName");
	  }
	  break;

	default:
	  {
	    sprintf(errbuf,"Invalid (unsupported) entry code \"%d\". \n", entrycode);       
	    H5Epush(__FILE__, "HE5_ZAnentries", __LINE__, H5E_ARGS, H5E_UNSUPPORTED, errbuf);
	    HE5_EHprint(errbuf, __FILE__, __LINE__);
	  }
	  break;
        }

	  
      if ( metabuf == NULL )
	{
	  nEntries = FAIL;
	  sprintf(errbuf,"Cannot get the number of entries. \n");
	  H5Epush(__FILE__, "HE5_ZAnentries", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(nEntries);
	}
	  


      /*
       * Check for presence of 'GROUP="' string If found then old metadata,
       * search on OBJECT string
       */
      metaflag = (strstr(metabuf, "GROUP=\"") == NULL) ? 1 : 0;
      if (metaflag == 0)
        {
	  nVal = 1;
	  strcpy(&valName[0][0], "\t\tOBJECT");
        }

      /* Begin loop through entries in metadata */
      /* -------------------------------------- */
      while (1)
        {
	  /* Search for first string */
	  /* ----------------------- */
	  strcpy(utlstr, &valName[0][0]);
	  strcat(utlstr, "=");
	  metaptrs[0] = strstr(metaptrs[0], utlstr);

	  /* If found within relevant metadata section ... */
	  /* --------------------------------------------- */
	  if (metaptrs[0] < metaptrs[1] && metaptrs[0] != NULL)
            {
	      for (i = 0; i < nVal; i++)
                {
		  /*
		   * Get all string values Don't count quotes
		   */
		  status = HE5_EHgetmetavalue(metaptrs, &valName[i][0], utlstr);
		  if ( status == FAIL )
		    {
		      nEntries = FAIL;
		      sprintf(errbuf,"Cannot get metavalue. \n");
		      H5Epush(__FILE__, "HE5_ZAnentries", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      free(metabuf);
			    
		      return(nEntries);
		    }


		  *strbufsize += strlen(utlstr) - 2;
                }
	      /* Increment number of entries */
	      /* --------------------------- */
	      nEntries++;

	      /* Go to end of OBJECT */
	      /* ------------------- */
	      metaptrs[0] = strstr(metaptrs[0], "END_OBJECT");
            }
	  else
	    /* No more entries found */
	    /* --------------------- */
            {
	      break;
            }
        }
      if (metabuf != NULL) free(metabuf);


      /* Count comma separators & slashes (if mappings) */
      /* ---------------------------------------------- */
      if (nEntries > 0)
        {
	  *strbufsize += nEntries - 1;
	  *strbufsize += (nVal - 1) * nEntries;
        }
    }
  
  return(nEntries);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqza                                                      |
|                                                                             |
|  DESCRIPTION: Returns number and names of ZA structures in file             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nZA            long                Number of ZA structures in file         |
|                                                                             |
|  INPUTS:                                                                    |
|  filename       const char*         HDF-EOS filename                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  zalist         char*               List of za names (comma-separated)      |
|  strbufsize     long*               Length of zalist                        |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqza(const char *filename, char *zalist, long *strbufsize)
{
  long       nZA   = FAIL;            /* Number of zas in file */

  herr_t     status   = FAIL;            /* Return status variable   */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */  

  CHECKPOINTER(filename);

  /* Call "HE5_EHinquire" routine */
  /* ---------------------------- */
  nZA = HE5_EHinquire(filename,  "/HDFEOS/ZAS", zalist, strbufsize);
  if ( nZA == FAIL )
    {
      sprintf(errbuf,"Cannot get information about ZA. \n");
      H5Epush(__FILE__, "HE5_ZAinqza", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }								 

 COMPLETION:
  return(nZA);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwrrdfield                                                  |
|                                                                             |
|  DESCRIPTION: Writes/Reads fields                                           |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fldname        char*               field name                              |
|  code           char*               Write/Read code (w/r)                   |
|  start          hssize_t            start array                             |
|  stride         hsize_t             stride array                            |
|  count          hsize_t             count array                             |
|  datbuf         void                data buffer for read                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void                data buffer for write                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Jan 04    S.Zhao       Added to write/read a character string dataset.     |
|  Apr 04    S.Zhao       Modified for a character string dataset.            |
|  Feb 05    S.Zhao       Replaced the "count" array by the "dims" array if   |
|                         it was declared larger than the dimension size.     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static herr_t    
HE5_ZAwrrdfield(hid_t zaID, char *fldname, char *code, const hssize_t start[], const hsize_t stride[], const hsize_t count[],  void *datbuf)
{    
  herr_t          status   = FAIL; /* routine return status variable     */    

  int             i;   		   /* Loop index                         */
  int             startf   =  0;   /* flag (if start is NULL)            */
  int             countf   =  0;   /* flag (if counf is NULL)            */
  int             bothf    =  0;   /* flag (if start and count are NULL) */
  int             append   = FALSE;/* FLAG (if field is appendible)      */
  int             rank     = FAIL; /* Rank of dataset                    */
  int             maxrank  =  0;   /* Rank dummy variable                */
  int             nameflag = FAIL; /* Actual (1)/Alias (0) field name    */
   
  hid_t           fid      = FAIL;/* HDF-EOS file ID                     */
  hid_t           fieldID  = FAIL;/* fieldname Dataset ID                */
  hid_t           fspace   = FAIL;/* file data space ID                  */
  hid_t           mid      = FAIL;/* memory data space ID                */
  hid_t           gid      = FAIL;/* "HDFEOS" group ID                   */
  hid_t           tid      = FAIL;/* datatype ID                         */
  hid_t           mtype    = FAIL;/* Memory data type ID                 */

  long            idx      = FAIL;/* za index                            */
  
  hsize_t         dims[HE5_DTSETRANKMAX];   /* Field dimensions          */
  hsize_t         maxdims[HE5_DTSETRANKMAX];/* Field maximum dimensions  */
  hssize_t        *tstart =(hssize_t *)NULL;/* Not NULL "start" value    */
  hsize_t         *tcount = (hsize_t *)NULL;/* Not NULL "count" value    */
  hsize_t         *lcount = (hsize_t *)NULL;
  hsize_t         dimsize = 0;		    /* Field dimension size      */
  hsize_t         size[HE5_DTSETRANKMAX];   /* Extended dimension size   */

  hid_t           ntype[1]  = {FAIL};       /* Datatype ID               */

  char            *dimbuf   = (char *)NULL;  
  char            *comma    = (char *)NULL;
  char            *dimcheck = (char *)NULL;
  char            fieldname[HE5_HDFE_NAMBUFSIZE];    /* Field name buffer      */
  char            fldactualname[HE5_HDFE_NAMBUFSIZE];/* Actual field name      */     
  char            maxdimlist[HE5_HDFE_DIMBUFSIZE];   /* Maximum dimension list */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];       /* Error message buffer   */  

  H5T_class_t     classid = H5T_NO_CLASS;            /* data type class ID     */

  int             ibuf;                /* Loop index                        */
  char            **chardatbuf = NULL; /* string data buffer                */
  int             maxlen = 0;          /* max length in array of string     */
  int             strsize = 0;         /* each str length in array of str   */
  hsize_t         numattr[1];          /* number of attribute elements      */
  int             mycount = 0;         /* flag                              */
  long            strbufsize1;         /* string buffer size                */
  long            strbufsize2;         /* string buffer size                */
  long            nattr = 0;           /* number of attributes              */
  char            *attrlist1 = (char *)NULL;/* attribute list               */
  char            *attrlist2 = (char *)NULL;/* attribute list               */
  char            *attptr = (char *)NULL;   /* attribute pointer            */

  CHECKNAME(fldname);
  
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAwrrdfield", &fid, &gid, &idx);  
  if (status == FAIL) return(status);

  	  
  /* Call HE5_ZAfldnameinfo() to get actual field name */
  /* ------------------------------------------------- */
  nameflag = HE5_ZAfldnameinfo(zaID, fldname, fldactualname);
  if ( nameflag == FAIL )
    {
      sprintf(errbuf, "Cannot get actual name of the field.\n");
      H5Epush(__FILE__,  "HE5_ZAwrrdfield", __LINE__,H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Name is not alias */
  /* ----------------- */
  if (nameflag == TRUE)
    {
      strcpy(fieldname,"");
      strcpy(fieldname,fldname);
    }

  /* Name is an alias */
  /* ---------------- */
  if (nameflag == FALSE)
    {
      strcpy(fieldname,"");
      strcpy(fieldname,fldactualname);
    }


  /* Initialize dims[]/maxdims[]/size[] arrays */
  /* ----------------------------------------- */
  for (i = 0; i < HE5_DTSETRANKMAX; i++)
    {
      dims[i]    = 0;
      maxdims[i] = 0;
      size[i]    = 0;
    }
  
  /* Check whether fieldname is in dataset (multi-dim field) */
  /* ------------------------------------------------------- */
  status = HE5_ZAinfo(zaID, fieldname, &rank, dims, ntype, NULL, maxdimlist);
  if( status == FAIL )
    {
      sprintf(errbuf,"Cannot get information about \"%s\" field. \n", fieldname);
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }


  /* Find out if the field is appendable */
  /* ----------------------------------- */
  dimbuf = (char *) calloc(strlen(maxdimlist) + 64, sizeof(char));
  if(dimbuf == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory.\n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  strcpy(dimbuf,maxdimlist);
  strcat(dimbuf,",");
  comma = strchr(dimbuf,',');
  
  while(comma != NULL)
    {
      dimcheck = (char *) calloc(comma - dimbuf + 1, sizeof(char));
      if(dimcheck == NULL)
	{
	  sprintf(errbuf,"Cannot allocate memory.\n");
	  H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(dimbuf);
	  return(FAIL);
	}
      memmove(dimcheck,dimbuf,comma-dimbuf);

      /* Get dimension size */
      /* ------------------ */
      dimsize = HE5_ZAdiminfo(zaID, dimcheck);
      if ( (dimsize > 0 && strcmp(dimcheck,"Unlim") != 0 ) || (dimsize == H5S_UNLIMITED && strcmp(dimcheck,"Unlim") == 0 )  )
	{
	  maxdims[maxrank] = dimsize;
	  maxrank++;
	}
      memmove(dimbuf,comma+1,strlen(comma+1)+1);
      comma = strchr(dimbuf,',');
      free(dimcheck);
    }

  free(dimbuf);
  
  for(i = 0; i < rank; i++)
    {
      if(   dims[i] == maxdims[i] )
	append = FALSE;
      else if ( (dims[i] < maxdims[i]) || (maxdims[i] == H5S_UNLIMITED))
	{
	  append = TRUE;
	  break;
	}
      else
	{
	  sprintf(errbuf,"Maximum dimension size is smaller than dimension size.\n");
	  H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_ARGS, H5E_BADRANGE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }        
  

  if(start == NULL)
    {
      tstart = (hssize_t *)calloc(rank, sizeof(hssize_t));
      for(i = 0; i < rank; i++)
	tstart[ i ] = 0;
      startf = 1;
    }
  if(count == NULL)
    {
      tcount = (hsize_t *)calloc(rank, sizeof(hsize_t));
      for(i = 0; i < rank; i++)
	tcount[ i ] = dims[ i ];
      if(startf == 1)
	bothf = 1;
      else
	countf = 1;
    }
  else
    {
      lcount =  (hsize_t *)calloc(rank, sizeof(hsize_t));
      for(i = 0; i < rank; i++)
	{
	  if(count[ i ] > dims[ i ])
	    lcount[ i ] = dims[ i ];
	  else
	    lcount[ i ] = count[ i ];
	}
    }
  
  if (append == TRUE)
    {
      /* Get the field-related dataset ID */
      /* -------------------------------- */
      HE5_ZAfieldsizeID(zaID, fieldname, &fieldID);
	  
      if(bothf == 1)
	status = H5Dextend(fieldID, tcount);
      else if(startf == 1)
	status = H5Dextend(fieldID, lcount);
      else if(countf == 1)
	status = H5Dextend(fieldID, tcount);
      else
	{
	  for(i = 0; i < rank; i++)
	    size[ i ] = start[ i ] + lcount[ i ];
		 
	  /* Extend the dataset size */
	  /* ----------------------- */
	  status = H5Dextend(fieldID, size);
	}

      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot extend the dataset. \n");
	  H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (start == NULL)
	    {
	      if (tstart != NULL)
		free(tstart);
	    }
	  if (count == NULL)
	    {
	      if (tcount != NULL)
		free(tcount);
	    }
	  if (lcount != NULL)
	    free(lcount);

	  return(FAIL);
	}
    }
  
  if (append == FALSE)
    {   
      /* Get the field-related dataset ID */
      /* -------------------------------- */
      HE5_ZAfieldsizeID(zaID, fieldname, &fieldID);
    }
  
  /* Get the file space ID */
  /* --------------------- */
  fspace = H5Dget_space(fieldID );
  if( fspace == FAIL)
    {
      sprintf(errbuf,"Cannot get data space ID. \n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      if (start == NULL)
	{
	  if (tstart != NULL) 
	    free(tstart);
	}
      if (count == NULL)
	{
	  if (tcount != NULL) 
	    free(tcount);
	}
      if (lcount != NULL)
	free(lcount);
	  
      return(FAIL);
    }
  
  /* Select the hyperslab */
  /* -------------------- */
  if(bothf == 1)
    status = H5Sselect_hyperslab( fspace, H5S_SELECT_SET, (const hsize_t *)tstart,stride,tcount,NULL);
  else if(startf == 1)
    status = H5Sselect_hyperslab( fspace, H5S_SELECT_SET, (const hsize_t *)tstart,stride,lcount,NULL);
  else if(countf == 1)
    status = H5Sselect_hyperslab( fspace, H5S_SELECT_SET, (const hsize_t *)start,stride,tcount,NULL);
  else
    status = H5Sselect_hyperslab( fspace, H5S_SELECT_SET, (const hsize_t *)start,stride,lcount,NULL) ;
  
  if( status == FAIL )
    {
      sprintf(errbuf,"Cannot select a hyperslab. \n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      if (start == NULL)
	{
	  if (tstart != NULL) 
	    free(tstart);
	}
      if (count == NULL)
	{
	  if (tcount != NULL) 
	    free(tcount);
	}
      if (lcount != NULL)
	free(lcount);
		
      return(FAIL);
    }
  
  
  /* Define memory space */
  /* =================== */
  if(countf == 1 || bothf == 1)
    mid = H5Screate_simple(rank, tcount, NULL);
  else
    mid = H5Screate_simple(rank, lcount, NULL);
  
  if( mid == FAIL)
    {
      sprintf(errbuf,"Cannot create the data space.\n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASPACE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      if (start == NULL)
	{
	  if (tstart != NULL) 
	    free(tstart);
	}
      if (count == NULL)
	{
	  if (tcount != NULL) 
	    free(tcount);
	}
      if (lcount != NULL)
	free(lcount);
		
      return(FAIL);
    }
  
  
  /* Get the data type ID */
  /* -------------------- */
  tid = H5Dget_type(fieldID );
  if( tid == FAIL )
    {
      sprintf(errbuf,"Cannot get the data type ID.\n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      if (start == NULL)
	{
	  if (tstart != NULL) 
	    free(tstart);
	}
      if (count == NULL)
	{
	  if (tcount != NULL) 
	    free(tcount);
	}
      if (lcount != NULL)
	free(lcount);
		
      return(FAIL);
    }
  
  /* Get the data type class ID */
  /* -------------------------- */
  classid = H5Tget_class(tid);
  if (classid == H5T_NO_CLASS)
    {
      sprintf(errbuf,"Cannot get the data type class ID.\n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      if (start == NULL)
	{
	  if (tstart != NULL)
	    free(tstart);
	}
      if (count == NULL)
	{
	  if (tcount != NULL)
	    free(tcount);
	}
      if (lcount != NULL)
	free(lcount);
 
      return(FAIL);
    }
  
  /* Write/read whole data to/from the dataset */
  /* ========================================= */
  if( strcmp(code,"w") == 0)
    {
      strbufsize1 = 0;
      nattr = HE5_ZAinqlocattrs(zaID, fieldname, NULL, &strbufsize1);
      if ( nattr < 0 )
	{
	  sprintf(errbuf, "Cannot retrieve the attributes. \n");
	  H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (start == NULL)
	    {
	      if (tstart != NULL)
		free(tstart);
	    }
	  if (count == NULL)
	    {
	      if (tcount != NULL)
		free(tcount);
	    }
	  if (lcount != NULL)
	    free(lcount);
 
	  return(FAIL);
 
	}
 
      attrlist1 = (char *) calloc(strbufsize1 + 2, sizeof(char));
      if (attrlist1 == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory for attrlist1.\n") ;
	  H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (start == NULL)
	    {
	      if (tstart != NULL)
		free(tstart);
	    }
	  if (count == NULL)
	    {
	      if (tcount != NULL)
		free(tcount);
	    }
	  if (lcount != NULL)
	    free(lcount);
 
	  return(FAIL);
 
	}
 
      nattr = HE5_ZAinqlocattrs(zaID, fieldname, attrlist1, &strbufsize1);
      if ( nattr < 0 )
	{
	  sprintf(errbuf, "Cannot retrieve the attributes. \n");
	  H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (attrlist1 != NULL)
	    {
	      free(attrlist1);
	      attrlist1 = NULL;
	    }
	  if (start == NULL)
	    {
	      if (tstart != NULL)
		free(tstart);
	    }
	  if (count == NULL)
	    {
	      if (tcount != NULL)
		free(tcount);
	    }
	  if (lcount != NULL)
	    free(lcount);
 
	  return(FAIL);
 
	}
 
      attptr = strstr(attrlist1,"ARRAYOFSTRINGS");
      if ( (classid == H5T_STRING) && (attptr == NULL) )
	{
	  status = H5Dwrite(fieldID, tid, mid, fspace,  H5P_DEFAULT, datbuf);
	  if( status == FAIL )
	    {
	      sprintf(errbuf,"Cannot write data to the dataset.\n");
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (attrlist1 != NULL)
		{
		  free(attrlist1);
		  attrlist1 = NULL;
		}
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
 
	      return(status);
	    }
	}
 
      else if ( (classid == H5T_STRING) && (attptr != NULL) )
	{
	  strbufsize2 = 0;
	  nattr = HE5_ZAinqlocattrs(zaID, fieldname, NULL, &strbufsize2);
	  if ( nattr < 0 )
	    {
	      sprintf(errbuf, "Cannot retrieve the attributes. \n");
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (attrlist1 != NULL)
		{
		  free(attrlist1);
		  attrlist1 = NULL;
		}
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
 
	      return(FAIL);
 
	    }
 
	  attrlist2 = (char *) calloc(strbufsize2 + 2, sizeof(char));
	  if (attrlist2 == NULL)
	    {
	      sprintf(errbuf, "Cannot allocate memory for attrlist2.\n") ;
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (attrlist1 != NULL)
		{
		  free(attrlist1);
		  attrlist1 = NULL;
		}
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
 
	      return(FAIL);
 
	    }
 
	  nattr = HE5_ZAinqlocattrs(zaID, fieldname, attrlist2, &strbufsize2);
	  if ( nattr < 0 )
	    {
	      sprintf(errbuf, "Cannot retrieve the attributes. \n");
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (attrlist1 != NULL)
		{
		  free(attrlist1);
		  attrlist1 = NULL;
		}
	      if (attrlist2 != NULL)
		{
		  free(attrlist2);
		  attrlist2 = NULL;
		}
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
 
	      return(FAIL);
 
	    }
 
	  if ( (nattr == 0) || ((attptr = strstr(attrlist2,"StringLengthAttribute")) == NULL) )
	    {
	      chardatbuf = (char **)datbuf;
	      if (count == NULL) mycount = tcount[0];
	      if (count != NULL) mycount = *lcount;
	      for (ibuf = 0; ibuf < mycount; ibuf++)
		{
		  if (chardatbuf[ibuf] == NULL)
		    strsize = 0;
		  else
		    strsize = strlen(chardatbuf[ibuf]);
		  if (strsize > maxlen)
		    maxlen = strsize;
 
		}
 
	      numattr[0] = 1;
	      status = HE5_ZAwritelocattr(zaID, fieldname, "StringLengthAttribute", H5T_NATIVE_INT, numattr, &maxlen);
	      if (status == FAIL)
		{
		  sprintf(errbuf, "Cannot write attribute to the field \"%s\".",fieldname) ;
		  H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  if (attrlist1 != NULL)
		    {
		      free(attrlist1);
		      attrlist1 = NULL;
		    }
		  if (attrlist2 != NULL)
		    {
		      free(attrlist2);
		      attrlist2 = NULL;
		    }
		  if (start == NULL)
		    {
		      if (tstart != NULL)
			free(tstart);
		    }
		  if (count == NULL)
		    {
		      if (tcount != NULL)
			free(tcount);
		    }
		  if (lcount != NULL)
		    free(lcount);
 
		  return(status);
		}
 
	    }
 
	  status = H5Dwrite(fieldID, tid, mid, fspace,  H5P_DEFAULT, datbuf);
	  if( status == FAIL )
	    {
	      sprintf(errbuf,"Cannot write data to the dataset.\n");
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (attrlist1 != NULL)
		{
		  free(attrlist1);
		  attrlist1 = NULL;
		}
	      if (attrlist2 != NULL)
		{
		  free(attrlist2);
		  attrlist2 = NULL;
		}
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
 
	      return(status);
	    }
	}
      else
	{
	  /* Get memory data type ID */
	  /* ----------------------- */
	  mtype  = HE5_EHdtype2mtype(tid);
	  if( mtype == FAIL )
	    {
	      sprintf(errbuf,"Cannot convert to memory data type.\n");
	      status = FAIL;
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (attrlist1 != NULL)
		{
		  free(attrlist1);
		  attrlist1 = NULL;
		}
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
 
	      return(status);
	    }
 
	  status = H5Dwrite(fieldID, mtype, mid, fspace, H5P_DEFAULT, datbuf);
	  if( status == FAIL )
	    {
	      sprintf(errbuf,"Cannot write data to the dataset.\n");
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (attrlist1 != NULL)
		{
		  free(attrlist1);
		  attrlist1 = NULL;
		}
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
 
	      return(status);
	    }
	}
 
      if (attrlist1 != NULL)
	{
	  free(attrlist1);
	  attrlist1 = NULL;
	}
      if (attrlist2 != NULL)
	{
	  free(attrlist2);
	  attrlist2 = NULL;
	}
 
    }
  else
    {
      if (classid == H5T_STRING)
	{
	  /* Read the data buffer */
	  /* -------------------- */
	  status = H5Dread(fieldID, tid, mid, fspace , H5P_DEFAULT, datbuf);
	  if( status == FAIL )
	    {
	      sprintf(errbuf,"Cannot read data from the dataset.\n");
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
 
	      return(status);
	    }
 
	}
      else
	{

	  /* Get memory data type ID */
	  /* ----------------------- */
          mtype  = HE5_EHdtype2mtype(tid);
	  if( mtype == FAIL )
	    {
	      sprintf(errbuf,"Cannot convert to memory data type.\n");
	      status = FAIL;
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (start == NULL)
		{
		  if (tstart != NULL)
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL)
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
		  
	      return(status);
	    }

	  /* Read the data buffer */
	  /* -------------------- */
	  status = H5Dread(fieldID, mtype, mid, fspace , H5P_DEFAULT, datbuf);
	  if( status == FAIL )
	    {
	      sprintf(errbuf,"Cannot read data from the dataset.\n");
	      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      if (start == NULL)
		{
		  if (tstart != NULL) 
		    free(tstart);
		}
	      if (count == NULL)
		{
		  if (tcount != NULL) 
		    free(tcount);
		}
	      if (lcount != NULL)
		free(lcount);
			
	      return(status);
	    }
	}

    }
  
  if ( count  == NULL)
    {	
      if(tcount != NULL) free(tcount);
    }
  if( start == NULL)
    {	
      if(tstart != NULL) free(tstart);
    }
  if (lcount != NULL)
    free(lcount);
  
  /* Release data space ID */
  /* --------------------- */  
  status = H5Sclose(mid); 
  if( status == FAIL )
    {
      sprintf(errbuf,"Cannot release memory data space ID.\n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
		
      return(status);
    }

  /* Release data space ID */
  /* --------------------- */   
  status = H5Sclose(fspace);
  if( status == FAIL )
    {
      sprintf(errbuf,"Cannot release the file data space ID.\n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
		
      return(status);
    }

  /* Release data type ID */
  /* -------------------- */    
  status = H5Tclose(tid);
  if( status == FAIL )
    {
      sprintf(errbuf,"Cannot release the data type ID.\n");
      H5Epush(__FILE__, "HE5_ZAwrrdfield", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
	  
      return(status);
    }
  
 COMPLETION:
  return(status);

}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwrite                                                      |
|                                                                             |
|  DESCRIPTION: Writes data to field                                          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  za_name        char*               za name                                 |
|  start          hssize_t            start array                             |
|  stride         hsize_t             stride array                            |
|  count          hsize_t             count array                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  datbuf         void*               data buffer for write                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_ZAwrite(hid_t zaID, char *za_name, const hssize_t start[], const hsize_t stride[], const hsize_t count[],  void *datbuf)
{
  herr_t   status  = FAIL;/* routine return status variable  */

  HE5_LOCK;
  CHECKNAME(za_name);
  CHECKPOINTER(datbuf);

  status = HE5_ZAwrrdfield(zaID, za_name, "w", start, stride, count, datbuf);

 COMPLETION:
  HE5_UNLOCK;
  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAread                                                       |
|                                                                             |
|  DESCRIPTION: Reads data from field                                         |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  za_name        char*               za name                                 |
|  start          hssize_t            start array                             |
|  stride         hsize_t             stride array                            |
|  count          hsize_t             count array                             |
|  datbuf         void*               data buffer for read                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|     None                                                                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_ZAread(hid_t  zaID, char *za_name, const hssize_t start[], const hsize_t stride[], const hsize_t count[],  void *datbuf)
{
  
  herr_t   status  = FAIL;/* routine return status variable    */
  
  HE5_LOCK;
  CHECKPOINTER(za_name);
  status = HE5_ZAwrrdfield(zaID, za_name, "r", start, stride, count, datbuf);
  
 COMPLETION:
  HE5_UNLOCK;
  return(status);
}





/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAfldsrch                                                    |
|                                                                             |
|  DESCRIPTION: Retrieves information about field DATASET                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  fldgroup        int                 Flag representing the group:           |
|                                      "Data Fields". (-1) FAILS.             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fldname        char                field name                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fieldID        hid_t                Field dataset ID                       |
|  rank           int                  dataset Rank                           |
|  dims           hsize_t              Dimensions of field                    |
|  typeID         hid_t                Data type ID                           |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_ZAfldsrch(hid_t zaID, char *fldname, hid_t *fieldID, int *rank,  hsize_t dims[], hid_t *typeID)
{
  int      fldgroup  = FAIL;       /* Field group flag  (return value)   */
  int      i;                      /* Loop index                         */
  int      nameflag  = FAIL;       /* Alias (0) /actual (1) name flag    */

  herr_t   status    = FAIL;       /* routine return status variable     */
  
  hid_t    idx       = FAIL;       /* za index                           */
  hid_t    idOffset  = HE5_ZAIDOFFSET;/* za ID offset                    */
  hid_t    spaceID   = FAIL;       /* File data space ID                 */

  char     fieldname[HE5_HDFE_NAMBUFSIZE];    /* Buffer for name of a field  */ 
  char     fldactualname[HE5_HDFE_NAMBUFSIZE];/* Actual name of a field      */ 
  char     fldtype   ='n';             /* Internal group discriminating flag */
  char     name[HE5_HDFE_NAMBUFSIZE];  /* Field Name buffer                  */
  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Buffer for error message           */


  CHECKPOINTER(fldname);

  /* Compute "reduced" za ID */
  /* ----------------------- */
  idx = zaID % idOffset;

  
  /* Call HE5_ZAfldnameinfo() to get actual field name */
  /* ------------------------------------------------- */
  nameflag = HE5_ZAfldnameinfo(zaID, fldname, fldactualname);
  if (nameflag == FAIL)
    {
      sprintf(errbuf, "Cannot get actual name of the field.\n");
      H5Epush(__FILE__, "HE5_ZAfldsrch", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  if (nameflag == FALSE)
    {
      strcpy(fieldname,"");
      strcpy(fieldname,fldactualname);
    }

  if (nameflag == TRUE)
    {
      strcpy(fieldname,"");
      strcpy(fieldname,fldname);
    }
  

  /* Loop through all data datasets in za */
  /* ------------------------------------ */
  for (i = 0; i < HE5_ZAXZa[idx].nDFLD; i++)
    {
      /* get dataset name */
      /* ---------------- */
      strcpy(name, HE5_ZAXZa[idx].ddataset[i].name);
      if( strcmp(fieldname, name) == 0 )
	{
	  fldtype ='d';
	  fldgroup   = HE5_HDFE_DATAGROUP;
	  break;
	}
    }
  

  if (fieldID != NULL)
    {

      /* Get dataset ID */
      /* ============== */
      if( fldtype =='d')
	*fieldID    = HE5_ZAXZa[idx].ddataset[i].ID;
      else
	{
	  sprintf(errbuf,"Cannot find \"%s\" field in za. Check the spelling.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAfldsrch", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      /* Get data space ID */
      /* ----------------- */
      spaceID  = H5Dget_space(*fieldID);
      if (spaceID == FAIL)
	{
	  sprintf(errbuf,"Cannot get the data space ID.\n");
	  H5Epush(__FILE__, "HE5_ZAfldsrch", __LINE__, H5E_DATASPACE, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      if (rank != NULL)
	{
	  /* Get dataset rank */
	  /* ---------------- */
	  *rank  = H5Sget_simple_extent_ndims(spaceID);
	  if (*rank == FAIL)
	    {
	      sprintf(errbuf,"Cannot get the dataset rank.\n");
	      H5Epush(__FILE__, "HE5_ZAfldsrch", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	}
	  
      if (typeID != NULL || dims != NULL)
	{
		  
	  /* Get data type ID */
	  /* ---------------- */
	  *typeID = H5Dget_type(*fieldID);
	  if (*typeID == FAIL)
	    {
	      sprintf(errbuf,"Cannot get the data type ID.\n");
	      H5Epush(__FILE__, "HE5_ZAfldsrch", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }

	  /* Get dimension sizes */
	  /* ------------------- */
	  status      = H5Sget_simple_extent_dims(spaceID, dims, NULL);
	  if (status == FAIL)
	    {
	      sprintf(errbuf,"Cannot get the data space dimension sizes.\n");
	      H5Epush(__FILE__, "HE5_ZAfldsrch", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
		  
	  /* Release data type ID */
	  /* -------------------- */
	  status = H5Tclose(*typeID);
	  if (status == FAIL)
	    {
	      sprintf(errbuf,"Cannot release the data type ID.\n");
	      H5Epush(__FILE__, "HE5_ZAfldsrch", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	  
	  /* Release data space ID */
	  /* --------------------- */
	  status = H5Sclose(spaceID);
	  if (status == FAIL)
	    {
	      sprintf(errbuf,"Cannot release the data space ID.\n");
	      H5Epush(__FILE__, "HE5_ZAfldsrch", __LINE__, H5E_DATASPACE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
		  
	}
	  
    }
  
 COMPLETION:
  return(fldgroup);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdetach                                                     |
|                                                                             |
|  DESCRIPTION: Detachs za structure and performs housekeeping                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAdetach(hid_t zaID)
{
  herr_t         status  = FAIL;             /* Return status variable */
 
  long           idx     = FAIL;             /* za index               */
 
  hid_t          fid     = FAIL;             /* HDF-EOS file ID        */
  hid_t          gid     = FAIL;             /* "HDFEOS" group ID      */
 
  int            k;                          /* Loop indices           */
 
  char           errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
 
  HE5_LOCK;
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdetach", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
 
      /* "Detach" from previously attached "Data Fields" datasets */
      /* -------------------------------------------------------- */
      if( HE5_ZAXZa[idx].nDFLD > 0 )
	{
	  for (k = 0; k < HE5_ZAXZa[idx].nDFLD; k++)
	    {
 
	      if ( HE5_ZAXZa[idx].ddataset[k].ID > 0 )
		{
		  status = H5Dclose(HE5_ZAXZa[idx].ddataset[k].ID);
		  if (status == FAIL)
		    {
		      sprintf(errbuf, "Cannot release the data dataset ID. \n") ;
		      H5Epush(__FILE__, "HE5_ZAdetach", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
 
		      return(status);
		    }
		}
	      if(HE5_ZAXZa[idx].ddataset[k].name != NULL)
		free(HE5_ZAXZa[idx].ddataset[k].name);
 
	    }
	  if (HE5_ZAXZa[idx].ddataset != NULL)
	    free(HE5_ZAXZa[idx].ddataset);
	}
 
 
      /* Detach from the za groups */
      /* ------------------------- */
      if ( HE5_ZAXZa[idx].data_id > 0 )
	{
	  status = H5Gclose(HE5_ZAXZa[idx].data_id);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the \"Data Fields\" group ID. \n") ;
	      H5Epush(__FILE__, "HE5_ZAdetach", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
 
	      return(status);
	    }
	}
 
 
      if ( HE5_ZAXZa[idx].za_id > 0 )
	{
	  status = H5Gclose(HE5_ZAXZa[idx].za_id);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the za group ID. \n") ;
	      H5Epush(__FILE__, "HE5_ZAdetach", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
 
	      return(status);
	    }
	}
 
 
      /* Delete entries from External Arrays */
      /* ----------------------------------- */
      HE5_ZAXZa[idx].active       = 0;
 
      HE5_ZAXZa[idx].data_id      = 0;
 
      HE5_ZAXZa[idx].ddataset     = NULL;
 
      HE5_ZAXZa[idx].fid          = 0;
      HE5_ZAXZa[idx].za_id        = 0;
      HE5_ZAXZa[idx].obj_id       = 0;
 
      HE5_ZAXZa[idx].nDFLD        = 0;
 
    }
 
 COMPLETION:
  HE5_UNLOCK;
  return (status);
}
 



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAsetfillvalue                                               |
|                                                                             |
|  DESCRIPTION: Sets fill value for the specified field.                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char*               field name                              |
|  numbertype     hid_t               number type                             |
|  fillval        void*               fill value                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Mar 04    S.Zhao       Modified for a character string dataset.            |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_ZAsetfillvalue(hid_t zaID, char *fieldname, hid_t numbertype_in, void *fillval)
{
  herr_t          status       = FAIL;/* return status variable       */
  
  hid_t           fid          = FAIL;/* HDF-EOS file ID              */
  hid_t           gid          = FAIL;/* "HDFEOS" group ID            */
  
  long            idx          = FAIL;/* za index                     */
  
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */
  hid_t           numbertype;

  HE5_LOCK;
  CHECKPOINTER(fillval);


  /* Convert HDF-EOS5 datatype to HDF5 datatype */
  if (
      H5Tequal(numbertype_in, H5T_NATIVE_CHAR)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_SCHAR)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_UCHAR)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_SHORT)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_USHORT) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_INT)    == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_UINT)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LONG)   == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_ULONG)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LLONG)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_ULLONG) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_FLOAT)  == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_DOUBLE) == SUCCEED ||
      H5Tequal(numbertype_in, H5T_NATIVE_LDOUBLE)== SUCCEED ||
      H5Tequal(numbertype_in, H5T_C_S1) == SUCCEED)
    {
      numbertype = numbertype_in;
    }
  else
    {
      numbertype = HE5_EHconvdatatype((int) numbertype_in);
    }

  if(numbertype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 type data type ID for fillvalue.\n");
      H5Epush(__FILE__, "HE5_ZAsetfillvalue", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }



  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAsetfillvalue", &fid, &gid, &idx);
  if(status == FAIL)
    {
      sprintf(errbuf,"Checking for valid za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAsetfillvalue", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  
  /* Get current dataset creation property list */
  /* ------------------------------------------ */  
  if( HE5_ZAXZa[idx].plist == FAIL)
    {
      HE5_ZAXZa[idx].plist = H5Pcreate(H5P_DATASET_CREATE);
      if (HE5_ZAXZa[idx].plist == FAIL)
	{
	  sprintf(errbuf,"Cannot create the dataset property list.\n");
	  H5Epush(__FILE__, "HE5_ZAsetfillvalue", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }

  /* Set fill value */
  /* -------------- */  
  if ((numbertype != HE5T_CHARSTRING) && (numbertype != H5T_NATIVE_CHAR) && (numbertype != H5T_C_S1))
    {
      status = H5Pset_fill_value(HE5_ZAXZa[idx].plist, numbertype, fillval);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot set fill value for \"%s\" field.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAsetfillvalue", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
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
|  FUNCTION: HE5_ZAgetfillvalue                                               |
|                                                                             |
|  DESCRIPTION: Retrieves fill value for a specified field.                   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char*               field name                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fillval        void*               fill value                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_ZAgetfillvalue(hid_t zaID, char *fieldname, void *fillval)
{
  herr_t          status   = FAIL;/* routine return status variable   */

  hid_t           fid      = FAIL;/* HDF-EOS file ID                  */
  hid_t           gid      = FAIL;/* "HDFEOS" group ID                */
  hid_t           fieldID  = FAIL;/* field dataset ID                 */
  hid_t           tid      = FAIL;/* field dataset data type ID       */
  hid_t           plist    = FAIL;/* field dataset property list ID   */
 
  long            idx      = FAIL;/* za index                         */
  
  hsize_t         count[1] = { 1 };/* number of attribute elements    */
    
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */
    

  HE5_LOCK;

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAgetfillvalue", &fid, &gid, &idx);
  if(status == FAIL)
    {
      sprintf(errbuf,"Checking for valid za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAgetfillvalue", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
  
  /* Check out the field group and get field ID */
  /* ------------------------------------------ */
  HE5_ZAfldsrch(zaID,fieldname,&fieldID,NULL,NULL,NULL);
  if( fieldID == FAIL )
    {
      sprintf(errbuf, "Cannot get the field ID for the \"%s\" field.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetfillvalue", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
   
  /* Get the property list ID */
  /* ------------------------ */
  plist = H5Dget_create_plist(fieldID);
  if ( plist == FAIL )
    {
      sprintf(errbuf, "Cannot get the property list ID for the \"%s\" data field.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetfillvalue", __LINE__, H5E_PLIST, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
	
  /* Get the datatype ID */ 
  /* ------------------- */
  tid   = H5Dget_type(fieldID);
  if ( tid == FAIL )
    {
      sprintf(errbuf, "Cannot get the data type ID for the \"%s\" data field.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetfillvalue", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
	
  /* Read out a fill value from the attribute */
  /* ---------------------------------------- */ 
  status = HE5_EHattr(fieldID, "_FillValue", tid, count, "r", fillval); 
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot get fill value for the  \"%s\" field.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetfillvalue", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
	
  /* Release data type ID */
  /* -------------------- */
  status = H5Tclose(tid);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release data type ID for the \"%s\" dataset.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetfillvalue", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
	
  /* Release property list ID */
  /* ------------------------ */
  status = H5Pclose(plist);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release property list ID for the \"%s\" dataset.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetfillvalue", __LINE__, H5E_PLIST, H5E_CLOSEERROR,errbuf);
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
|  FUNCTION: HE5_ZAclose                                                      |
|                                                                             |
|  DESCRIPTION: Closes HDF-EOS file                                           |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  fid            hid_t               File ID                                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAclose(hid_t fid)
{
  herr_t   status = FAIL;/* routine return status variable */
 
  /* Call HE5_EHclose to perform file close */
  /* -------------------------------------- */
  status = HE5_EHclose(fid);
 
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAfieldsizeID                                                |
|                                                                             |
|  DESCRIPTION:  Returns the size (in bytes) of field-related data type,      |
|                otherwise returns 0.                                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  size           size_t              Size of field data type                 |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char                fieldname                               |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fieldID        hid_t               field-related dataset ID                |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
static size_t
HE5_ZAfieldsizeID( hid_t zaID, char *fieldname, hid_t  *fieldID)
{
  size_t         size      =  0;             /* Size of field data type  */
 
  herr_t         status    = FAIL;           /* Return status variable   */
 
  int            i;                          /* Loop index               */
  int            found     =  0;             /* Flag                     */
 
  hid_t          fid       = FAIL;           /* HDF-EOS file ID          */
  hid_t          gid       = FAIL;           /* "HDFEOS" group ID        */
  hid_t          dtype     = FAIL;           /* datatype ID              */
 
  long           idx       = FAIL;           /* ZA index                 */
 
  char           errbuf[HE5_HDFE_ERRBUFSIZE];/* Buffer for error message */
 
  CHECKPOINTER(fieldname);
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and ZA index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAfieldsizeID", &fid, &gid, &idx);
 
  /* Loop through all data fields in ZA */
  /* ---------------------------------- */
  for (i = 0; i < HE5_ZAXZa[idx].nDFLD; i++)
    {
      if( strcmp(fieldname, HE5_ZAXZa[idx].ddataset[i].name) == 0 )
	{
	  /* get dataset ID */
	  /* -------------- */
	  *fieldID = HE5_ZAXZa[idx].ddataset[i].ID;
 
	  dtype    = H5Dget_type(*fieldID);
	  if (dtype == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the dataset data type. \n");
	      H5Epush(__FILE__, "HE5_ZAfieldsizeID", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(0);
	    }
 
	  size     = H5Tget_size(dtype);
	  if (size == 0)
	    {
	      sprintf(errbuf, "Cannot get the data type size. \n");
	      H5Epush(__FILE__, "HE5_ZAfieldsizeID", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(0);
	    }
 
	  status   = H5Tclose(dtype);
	  if (status == FAIL)
	    {
	      sprintf(errbuf, "Cannot release the data type ID. \n");
	      H5Epush(__FILE__, "HE5_ZAfieldsizeID", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(0);
	    }
	  found = 1;
	  break;
	}
    }
  if(  found == 0 )
    {
      sprintf(errbuf, "field \"%s\" is not in ZA. \n", fieldname);
      H5Epush(__FILE__, "HE5_ZAfieldsizeID", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
 
 COMPLETION:
  return(size);
}




/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwritegrpattr                                               |
|                                                                             |
|  DESCRIPTION: Writes/updates attribute associated with the "Data Fields"    |
|               group in a za.                                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  attrname       char                attribute name                          |
|  numbertype     hid_t               attribute dataset datatype ID           |
|  count[]        hsize_t             Number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAwritegrpattr(hid_t zaID, const char *attrname, hid_t numtype, hsize_t  count[], void *datbuf)
{
  herr_t     status          = FAIL;     /* Return status variable */

  hid_t      fid             = FAIL;     /* HDF-EOS file ID        */
  hid_t      gid             = FAIL;     /* "HDFEOS" group ID      */
  hid_t      DataFieldsGrpID = FAIL;     /* "Data Fields" group ID */
    
  long       idx             = FAIL;     /* za index               */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
  hid_t      numbertype;

  HE5_LOCK;
  CHECKNAME(attrname);
  CHECKPOINTER(count);
  CHECKPOINTER(datbuf);

  /* 
     if ntype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use ntype itself
  */
  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      numbertype = numtype;
    }
  
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAwritegrpattr", &fid, &gid, &idx);

  if (status == SUCCEED)
    {
      /* Get "Data Fields" group ID and call HE5_EHattr to perform I/O */
      /* ------------------------------------------------------------- */
      DataFieldsGrpID = H5Gopen(HE5_ZAXZa[idx].za_id, "Data Fields");
      if(DataFieldsGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data Fields\" group ID. \n");
	  H5Epush(__FILE__, "HE5_ZAwritegrpattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	} 

      status = HE5_EHattr(DataFieldsGrpID, attrname, numbertype, count,"w", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write Attribute \"%s\"to the \"Data Fields\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_ZAwritegrpattr", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Gclose(DataFieldsGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data Fields\" group ID. \n");
	  H5Epush(__FILE__, "HE5_ZAwritegrpattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_ZAreadgrpattr                                                |
|                                                                             |
|  DESCRIPTION: Reads attribute from the "Data Fields" group in a za.         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
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
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAreadgrpattr(hid_t zaID, const char *attrname, void * datbuf)
{
  herr_t      status          = FAIL;     /* return status variable   */
  
  hid_t       fid             = FAIL;     /* HDF-EOS file ID          */
  hid_t       gid             = FAIL;     /* "HDFEOS" group ID        */
  hid_t       ntype           = FAIL;     /* Data type ID             */
  hid_t       DataFieldsGrpID = FAIL;     /* "Data Fields" group ID   */
  
  long        idx             = FAIL;     /* za index                 */
  
  hsize_t     count[]         = { 0 };    /* # of attribute elements  */
  
  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */
  

  HE5_LOCK;
  CHECKPOINTER(attrname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAreadgrpattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Get "Data Fields"  group ID and call HE5_EHattr to perform I/O */
      /* -------------------------------------------------------------- */
      DataFieldsGrpID = H5Gopen(HE5_ZAXZa[idx].za_id, "Data Fields");
      if(DataFieldsGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data Fields\" group ID.\n");
	  H5Epush(__FILE__, "HE5_ZAreadgrpattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
        }

      status = HE5_EHattr(DataFieldsGrpID,attrname,ntype,count,"r", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot read Attribute \"%s\" from the \"Data Fields\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_ZAreadgrpattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
        }

      status = H5Gclose(DataFieldsGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data Fields\" group ID.\n");
	  H5Epush(__FILE__, "HE5_ZAreadgrpattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_ZAgrpattrinfo                                                |
|                                                                             |
|  DESCRIPTION: Retrieves information about attributes in "Data Fields" group.|
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  attrname       const char*         attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          hid_t               data type ID                            |
|  count          hsize_t             Number of attribute elements            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAgrpattrinfo(hid_t zaID, const char *attrname, hid_t *ntype, hsize_t *count)
{
  herr_t     status          = FAIL;     /* routine return status variable */

  hid_t      fid             = FAIL;     /* HDF-EOS file ID                */
  hid_t      gid             = FAIL;     /* "HDFEOS" group ID              */
  hid_t      DataFieldsGrpID = FAIL;     /* "Data Fields" group ID         */

  long       idx             = FAIL;     /* za index                       */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/*  Error message buffer          */

  CHECKPOINTER(attrname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAgrpattrinfo", &fid, &gid, &idx);	
  if (status == SUCCEED)
    {
      /* Get "Data Fields"  group ID and call HE5_EHattrinfo */
      /* --------------------------------------------------- */
      DataFieldsGrpID = H5Gopen(HE5_ZAXZa[idx].za_id, "Data Fields");
      if(DataFieldsGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"Data Fields\" group ID.\n");
	  H5Epush(__FILE__, "HE5_ZAgrpattrinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      status = HE5_EHattrinfo(DataFieldsGrpID,attrname,ntype,count);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot retrieve information about Attribute \"%s\" in the \"Data Fields\" group.\n", attrname);
	  H5Epush(__FILE__, "HE5_ZAgrpattrinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(status);
	}

      status = H5Gclose(DataFieldsGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"Data Fields\" group ID.\n");
	  H5Epush(__FILE__, "HE5_ZAgrpattrinfo", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
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
|  FUNCTION: HE5_ZAinqgrpattrs                                                |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list in "Data Fields" group                        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in "Data Fields"   |
|                                       group.                                |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in "Data Fields" group  |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqgrpattrs(hid_t zaID, char *attrnames, long *strbufsize)
{
  long            nattr   = FAIL;          /* Number of attributes (return)  */
  long            idx     = FAIL;          /* za index                       */

  herr_t          status  = FAIL;          /* Return status variable         */

  hid_t           fid     = FAIL;          /* HDF-EOS file ID                */
  hid_t           gid     = FAIL;          /* "ZAS" group ID                 */
    
  char            *grpname = (char *)NULL; /* Group name string              */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer        */



  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinqgrpattrs", &fid, &gid, &idx);
  if(status == SUCCEED )
    {
      grpname = (char *)calloc(HE5_HDFE_NAMBUFSIZE, sizeof(char) );
      if( grpname == NULL)
        {
          H5Epush(__FILE__, "HE5_ZAinqgrpattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Can not allocate memory");
          HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
          return(FAIL);
        }

      strcpy(grpname,"/HDFEOS/ZAS/");
      strcat(grpname, HE5_ZAXZa[idx].zaname);
      strcat(grpname,"/Data Fields");
	
      /* search group with grpname for the number of attributes */
      /* ------------------------------------------------------ */ 
      nattr = HE5_EHattrcat(fid,  grpname, attrnames, strbufsize);
      if ( nattr < 0 )
	{
	  sprintf(errbuf, "Cannot find the attributes. \n");
	  H5Epush(__FILE__, "HE5_ZAinqgrpattrs", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
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
|  FUNCTION: HE5_ZAwritelocattr                                               |
|                                                                             |
|  DESCRIPTION: Writes/updates attribute associated with a specified field    |
|               in a za.                                                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char                field name                              |
|  attrname       char                attribute name                          |
|  numbertype     hid_t               attribute dataset datatype ID           |
|  count[]        hsize_t             Number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAwritelocattr(hid_t zaID, const char *fieldname, const char *attrname, hid_t numtype, hsize_t  count[], void *datbuf)
{
  herr_t     status          =  FAIL;    /* Return status variable */

  int        fldgroup        =  FAIL;    /* Field group flag       */
  
  hid_t      fid             =  FAIL;    /* HDF-EOS file ID        */
  hid_t      gid             =  FAIL;    /* "HDFEOS" group ID      */
  hid_t      FieldGrpID      =  FAIL;    /* Field group ID         */
  hid_t      fieldID         =  FAIL;    /* Field dataset ID       */
  
  long       idx             =  FAIL;    /* za index               */
  
  char       *grpname = (char *)NULL;    /* Group name buffer      */    
  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
  hid_t      numbertype;

  HE5_LOCK;
  CHECKPOINTER(fieldname);
  CHECKNAME(attrname);
  CHECKPOINTER(count);
  CHECKPOINTER(datbuf);

  /* 
     if ntype is HE5 numbertype, rather than H5 numbertype, then convert
     it, otherwise use ntype itself
  */
  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      numbertype = numtype;
    }

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAwritelocattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Allocate memory for the group name buffer */
      /* ----------------------------------------- */
      grpname = (char *)calloc(64, sizeof(char) );
      if( grpname == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for the group name buffer.\n");
          H5Epush(__FILE__, "HE5_ZAwritelocattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(FAIL);
        }

      /* Get the field group flag */
      /* ------------------------ */
      fldgroup = HE5_ZAfldsrch(zaID,(char *)fieldname,NULL,NULL,NULL,NULL);
      if(fldgroup == FAIL)
	{
	  sprintf(errbuf,"Cannot get the group flag for \"%s\" field.\n",fieldname);
	  H5Epush(__FILE__, "HE5_ZAwritelocattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}

      if (fldgroup == HE5_HDFE_DATAGROUP)
	strcpy(grpname,"Data Fields");


      /* Get the Field group ID */
      /* ---------------------- */
      FieldGrpID = H5Gopen(HE5_ZAXZa[idx].za_id, grpname);
      if(FieldGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" group.\n", grpname);
	  H5Epush(__FILE__, "HE5_ZAwritelocattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}

      /* Get field ID and call HE5_EHattr to perform I/O */
      /* ----------------------------------------------- */
      fieldID = H5Dopen(FieldGrpID, fieldname);
      if(fieldID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" field dataset.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAwritelocattr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}

      /* Call EHattr() to perform I/O */
      /* ---------------------------- */
      status = HE5_EHattr(fieldID, attrname, numbertype, count,"w", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write Attribute \"%s\" for the \"%s\" field.\n", attrname, fieldname);
	  H5Epush(__FILE__, "HE5_ZAwritelocattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}

      /* Release dataset ID */
      /* ------------------ */
      status = H5Dclose(fieldID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAwritelocattr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}

      /* Release group ID */
      /* ---------------- */
      status = H5Gclose(FieldGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" group ID.\n",grpname);
	  H5Epush(__FILE__, "HE5_ZAwritelocattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}

      free(grpname);
    }
  
 COMPLETION:
  HE5_UNLOCK;
  return (status);	
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAreadlocattr                                                |
|                                                                             |
|  DESCRIPTION: Reads attribute associated with a specified field in a za.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char                fieldname                               |
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
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t    
HE5_ZAreadlocattr(hid_t zaID, const char *fieldname, const char *attrname, void * datbuf)
{
  herr_t      status          =  FAIL;    /* return status variable   */

  int         fldgroup        =  FAIL;    /* Field group flag         */

  hid_t       fid             =  FAIL;    /* HDF-EOS file ID          */
  hid_t       gid             =  FAIL;    /* "HDFEOS" group ID        */
  hid_t       ntype           =  FAIL;    /* Data type ID             */
  hid_t       FieldGrpID      =  FAIL;    /* Field group ID           */
  hid_t       fieldID         =  FAIL;    /* Field dataset ID         */
  
  long        idx             =  FAIL;    /* za index                 */
  
  hsize_t     count[]         = { 0 };    /* number of elements       */
  
  char        *grpname = (char *)NULL;    /* Group name buffer        */
  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */


  HE5_LOCK;
  CHECKPOINTER(fieldname);
  CHECKPOINTER(attrname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAreadlocattr", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Allocate memory for the group name buffer */
      /* ----------------------------------------- */
      grpname = (char *)calloc(64, sizeof(char) );
      if( grpname == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for the group name buffer.\n");
          H5Epush(__FILE__, "HE5_ZAreadlocattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(FAIL);
        }

      /* Get the field group flag */
      /* ------------------------ */
      fldgroup = HE5_ZAfldsrch(zaID,(char *)fieldname,NULL,NULL,NULL,NULL);
      if(fldgroup == FAIL)
	{
	  sprintf(errbuf,"Cannot get the group flag for \"%s\" field. \n",fieldname);
	  H5Epush(__FILE__, "HE5_ZAreadlocattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}

      if (fldgroup == HE5_HDFE_DATAGROUP)
	strcpy(grpname,"Data Fields");


      /* Get Field  group ID  */
      /* -------------------- */
      FieldGrpID = H5Gopen(HE5_ZAXZa[idx].za_id, grpname);
      if(FieldGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" group.\n", grpname);
	  H5Epush(__FILE__, "HE5_ZAreadlocattr", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}

      /* Get field ID and call HE5_EHattr to perform I/O */
      /* ----------------------------------------------- */
      fieldID = H5Dopen(FieldGrpID, fieldname);
      if(fieldID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" field dataset.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAreadlocattr", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}
        
      /* Call EHattr() to perform I/O */
      /* ---------------------------- */
      status = HE5_EHattr(fieldID,attrname,ntype,count,"r", datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot read Attribute \"%s\" associated wth the \"%s\" field.\n", attrname,fieldname);
	  H5Epush(__FILE__, "HE5_ZAreadlocattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}

      /* Release the dataset ID */
      /* ---------------------- */
      status = H5Dclose(fieldID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAreadlocattr", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}	

      /* Release the group ID */
      /* -------------------- */
      status = H5Gclose(FieldGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" group ID.\n", grpname);
	  H5Epush(__FILE__, "HE5_ZAreadlocattr", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}
	
      free(grpname);
    }

 COMPLETION:
  HE5_UNLOCK;
  return (status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAlocattrinfo                                                |
|                                                                             |
|  DESCRIPTION: Retrievs information about attribute associated with a        |
|                 specified field in the "Data Fields" group.                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char*               field name                              |
|  attrname       const char*         attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          hid_t               data type ID                            |
|  count          hsize_t             Number of attribute elements            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t   
HE5_ZAlocattrinfo(hid_t zaID, const char *fieldname, const char *attrname, hid_t *ntype, hsize_t *count)
{
  herr_t     status          =  FAIL;    /* Return status variable */
  
  int        fldgroup        =  FAIL;    /* Field group flag       */

  hid_t      fid             =  FAIL;    /* HDF-EOS file ID        */
  hid_t      gid             =  FAIL;    /* "HDFEOS" group ID      */
  hid_t      FieldGrpID      =  FAIL;    /* Field group ID         */
  hid_t      fieldID         =  FAIL;    /* Field dataset ID       */

  long       idx             =  FAIL;    /* za index               */
  
  char       *grpname  = (char *)NULL;   /* Group name buffer      */
  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
  
  CHECKPOINTER(fieldname);
  CHECKPOINTER(attrname); 

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */ 
  status = HE5_ZAchkzaid(zaID, "HE5_ZAlocattrinfo", &fid, &gid, &idx);
  if (status == SUCCEED)
    {
      /* Allocate memory for the group name buffer */
      /* ----------------------------------------- */
      grpname = (char *)calloc(64, sizeof(char) );
      if( grpname == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for the group name buffer.\n");
          H5Epush(__FILE__, "HE5_ZAlocattrinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(FAIL);
        }

      /* Get the field group flag */
      /* ------------------------ */
      fldgroup = HE5_ZAfldsrch(zaID,(char *)fieldname,NULL,NULL,NULL,NULL);
      if(fldgroup == FAIL)
	{
	  sprintf(errbuf,"Cannot get the group flag for \"%s\" field. \n",fieldname);
	  H5Epush(__FILE__, "HE5_ZAlocattrinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}

      if (fldgroup == HE5_HDFE_DATAGROUP)
	strcpy(grpname,"Data Fields");

      /* Get Field  group ID  */
      /* -------------------  */
      FieldGrpID = H5Gopen(HE5_ZAXZa[idx].za_id, grpname);
      if(FieldGrpID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" group.\n", grpname);
	  H5Epush(__FILE__, "HE5_ZAlocattrinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}

      /* Get field ID */
      /* ------------ */
      fieldID = H5Dopen(FieldGrpID, fieldname);
      if(fieldID == FAIL)
	{
	  sprintf(errbuf,"Cannot open the \"%s\" field dataset.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAlocattrinfo", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(FAIL);
	}

      /* Call EHattrinfo() to perform I/O */
      /* -------------------------------- */
      status = HE5_EHattrinfo(fieldID,attrname,ntype,count);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot retrieve information about Attribute \"%s\" in the \"%s\" group.\n", attrname, grpname);
	  H5Epush(__FILE__, "HE5_ZAlocattrinfo", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}

      /* Release dataset ID */
      /* ------------------ */
      status = H5Dclose(fieldID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the \"%s\" field dataset ID.\n", fieldname);
	  H5Epush(__FILE__, "HE5_ZAlocattrinfo", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}	

      /* Release group ID */
      /* ---------------- */
      status = H5Gclose(FieldGrpID);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot release the field group ID.\n");
	  H5Epush(__FILE__, "HE5_ZAlocattrinfo", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  return(status);
	}	
	  
      free(grpname);	  
    }

 COMPLETION:
  return (status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqlocattrs                                                |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list associated with a specified field in the      |
|                "Data Fields" group.                                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in "Data Fields"   |
|                                       group.                                |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  fieldname      char                field name                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in "Data Fields" group  |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqlocattrs(hid_t zaID, const char *fieldname, char *attrnames, long *strbufsize)
{
  long            nattr       = FAIL;         /* Number of attributes     */
  long            idx         = FAIL;         /* za index                 */

  herr_t          status      = FAIL;         /* Return status variable   */

  int             fldgroup    =  FAIL;        /* Field group flag         */ 

  hid_t           fid         = FAIL;         /* HDF-EOS file ID          */
  hid_t           gid         = FAIL;         /* "ZAS" group ID           */
    
  char            *dtsname    = (char *)NULL; /* Dataset name string      */
  char            *grpname    = (char *)NULL; /* Group name buffer        */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */  

  CHECKPOINTER(fieldname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinqlocattrs", &fid, &gid, &idx);
  if(status == SUCCEED )
    {
      /* Allocate memory for the dataset name buffer */
      /* ------------------------------------------- */
      dtsname = (char *)calloc(HE5_HDFE_NAMBUFSIZE, sizeof(char) );
      if( dtsname == NULL)
        {
	  sprintf(errbuf, "Cannot allocate memory for the dataset name buffer. \n");
          H5Epush(__FILE__, "HE5_ZAinqlocattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(FAIL);
        }

      /* Allocate memory for the group name buffer */
      /* ----------------------------------------- */
      grpname = (char *)calloc(64, sizeof(char) );
      if( grpname == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for the group name buffer.\n");
          H5Epush(__FILE__, "HE5_ZAinqlocattrs", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(dtsname);
          return(FAIL);
        }

      /* Get the field group flag */
      /* ------------------------ */
      fldgroup = HE5_ZAfldsrch(zaID,(char *)fieldname,NULL,NULL,NULL,NULL);
      if(fldgroup == FAIL)
	{
	  sprintf(errbuf,"Cannot get the group flag for \"%s\" field.\n",fieldname);
	  H5Epush(__FILE__, "HE5_ZAinqlocattrs", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	  free(dtsname);
	  return(FAIL);
	}

      if (fldgroup == HE5_HDFE_DATAGROUP)
	strcpy(grpname,"/Data Fields/");

      strcpy(dtsname,"/HDFEOS/ZAS/");
      strcat(dtsname, HE5_ZAXZa[idx].zaname);
      strcat(dtsname, grpname);
      strcat(dtsname, fieldname);
	  
      /* search group with grpname for the number of attributes */
      /* ------------------------------------------------------ */ 
      nattr = HE5_EHattrcat(fid,  dtsname, attrnames, strbufsize);
      if ( nattr < 0 )
	{
	  sprintf(errbuf, "Cannot find the attributes. \n");
	  H5Epush(__FILE__, "HE5_ZAinqlocattrs", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	}

      if (dtsname != NULL) free(dtsname);
      if (grpname != NULL) free(grpname);
    }
  
 COMPLETION:
  return(nattr);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAsetalias                                                   |
|                                                                             |
|  DESCRIPTION:  Defines alias for a specified field name                     |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  status         herr_t                   Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    zaID         hid_t                    ZA structure ID                    |
|    fieldname    char                     Original field name                |
|                                                                             |
|  OUTPUTS:                                                                   |
|    aliaslist    char                     List of aliases for the field name |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_ZAsetalias(hid_t zaID, char *fieldname, const char *aliaslist)
{
  herr_t   status     = FAIL;          /* return status variable      */

  int      fldgroup   = FAIL;          /* Field group flag            */

  hid_t    fid        = FAIL;          /* HDF-EOS file ID             */
  hid_t    gid        = FAIL;          /* "ZAS"  group ID             */
  hid_t    groupID    = FAIL;          /* Field group ID              */

  long     idx        = FAIL;          /* za index                    */

  char     *buf       = NULL;          /* Pointer to temporary buffer */
  char     *comma     = NULL;          /* Pointer to comma            */
  char     *aliasname = NULL;          /* Pointer to the alias name   */
  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer        */


  HE5_LOCK;
  CHECKPOINTER(fieldname);
  CHECKPOINTER(aliaslist);


  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAsetalias", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAsetalias", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Find out the field group */
  /* ------------------------ */
  fldgroup = HE5_ZAfldsrch(zaID,fieldname,NULL,NULL,NULL,NULL);

  if (fldgroup == HE5_HDFE_DATAGROUP)
    groupID = HE5_ZAXZa[idx].data_id;

  else
    {	  
      sprintf(errbuf, "Cannot find \"%s\" field in za. \n", fieldname);
      H5Epush(__FILE__, "HE5_ZAsetalias", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Allocate memory for temporary buffer */
  /* ------------------------------------ */
  buf = (char *)calloc(strlen(aliaslist)+64, sizeof(char));
  if (buf == NULL)
    {
      sprintf(errbuf, "Cannot allocate memory for \"buf\". \n");
      H5Epush(__FILE__, "HE5_ZAsetalias", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  

  /* Make a list like "name1,name2,name3," */
  /* ------------------------------------- */
  strcpy(buf,aliaslist);
  strcat(buf,",");

  /* Find first comma and make pointer pointing to it */
  /* ------------------------------------------------ */
  comma = strchr(buf, ',');
  while(comma != NULL)
    {
      /* Allocate memory for individual entry */
      /* ------------------------------------ */
      aliasname = (char *)calloc(comma-buf+1, sizeof(char));
      if (aliasname == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory for \"aliasname\". \n");
	  H5Epush(__FILE__, "HE5_ZAsetalias", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (buf != NULL) free(buf);
	  return(FAIL);
	}
	  
      /* Pick up an individual entry and put it to the "aliasname" */
      /* --------------------------------------------------------- */
      memmove(aliasname,buf,comma-buf);
  
      /* Create a symbolic link for a specified object */
      /* --------------------------------------------- */
      status = H5Glink(groupID, H5G_LINK_SOFT, fieldname, aliasname);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot create an alias \"%s\" for \"%s\" field. \n", aliasname, fieldname);
	  H5Epush(__FILE__, "HE5_ZAsetalias", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  if (aliasname != NULL) free(aliasname);
	  if (buf != NULL) free(buf);
	  return(FAIL);
	}
  
      /* Go to the next name entry */
      /* ------------------------- */
      memmove(buf,comma+1,strlen(comma+1)+1);
      comma = strchr(buf, ',');
      if (aliasname != NULL) free(aliasname);
    }
  
  if (buf != NULL) free(buf);
	  
 COMPLETION:
  HE5_UNLOCK;
  return(status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdropalias                                                  |
|                                                                             |
|  DESCRIPTION:  Removes the alias for a specified field name                 |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  status         herr_t                   Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    zaID         hid_t                    ZA structure ID                    |
|    aliasname    char                     Alias name to remove               |
|    fldgroup     int                      Field group flag                   |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_ZAdropalias(hid_t zaID, int fldgroup, const char *aliasname)
{  
  herr_t   status     = FAIL;          /* return status variable   */

  hid_t    fid        = FAIL;          /* HDF-EOS file ID          */
  hid_t    gid        = FAIL;          /* "ZAS"  group ID          */
  hid_t    groupID    = FAIL;          /* Field group ID           */

  long     idx        = FAIL;          /* za index                 */

  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */


  HE5_LOCK;
  CHECKPOINTER(aliasname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAdropalias", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAdropalias", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get Group ID */
  /* ------------ */
  if (fldgroup == HE5_HDFE_DATAGROUP)
    groupID = HE5_ZAXZa[idx].data_id;
  else
    {	  
      sprintf(errbuf, "Invalid input group flag. \n");
      H5Epush(__FILE__, "HE5_ZAdropalias", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Destroy a symbolic link */
  /* ----------------------- */
  status = H5Gunlink(groupID, aliasname);
  if (status == FAIL)
    {	  
      sprintf(errbuf, "Cannot remove alias named \"%s\". \n", aliasname);
      H5Epush(__FILE__, "HE5_ZAdropalias", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

 COMPLETION:
  HE5_UNLOCK;
  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqfldalias                                                |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of fields & aliases and string length of|
|                fields & aliases list in "Data Fields" group                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nfldalias      long                Number of fields & aliases in "Data     |
|                                     Fields" group.                          |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               za structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fldalias       char*               Fields & aliases names in "Data Fields" |
|                                     group (Comma-separated list)            |
|  strbufsize     long*               Fields & aliases name list string length|
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Apr 03    S.Zhao       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAinqfldalias(hid_t zaID, char *fldalias, long *strbufsize)
{
  long            nfldalias   = FAIL;/* Number of fields & aliases (return)  */
  long            idx     = FAIL;    /* za index                       */
 
  herr_t          status  = FAIL;    /* Return status variable         */
 
  hid_t           fid     = FAIL;    /* HDF-EOS file ID                */
  hid_t           gid     = FAIL;    /* "ZA" group ID                  */
 
  char            *grpname = (char *)NULL; /* Group name string        */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */
 
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAinqfldalias", &fid, &gid, &idx);
  if(status == SUCCEED )
    {
      grpname = (char *)calloc(HE5_HDFE_NAMBUFSIZE, sizeof(char) );
      if( grpname == NULL)
	{
	  H5Epush(__FILE__, "HE5_ZAinqfldalias", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Can not allocate memory");
	  HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
	  return(FAIL);
	}
 
      strcpy(grpname,"/HDFEOS/ZAS/");
      strcat(grpname, HE5_ZAXZa[idx].zaname);
      strcat(grpname,"/Data Fields");
 
      /* search group with grpname for the number of fields & aliases */
      /* ------------------------------------------------------------ */
      nfldalias = HE5_EHdatasetcat(fid,  grpname, fldalias, strbufsize);
      if ( nfldalias < 0 )
	{
	  sprintf(errbuf, "Cannot find the fields & aliases. \n");
	  H5Epush(__FILE__, "HE5_ZAinqfldalias", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(grpname);
	}
 
      if (grpname != NULL) free(grpname);
    }
  else
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAinqfldalias", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  return (nfldalias);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAaliasinfo                                                  |
|                                                                             |
|  DESCRIPTION:  Retrieves information about field aliases                    |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  status         herr_t                   Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    zaID         hid_t                    ZA structure ID                    |
|    aliasname    char                     alias name                         |
|    fldgroup     int                      Field group flag                   |
|                                                                             |
|  OUTPUTS:                                                                   |
|    length       int                      Buffer size                        |
|    buffer       char                     Buffer with original field name    |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_ZAaliasinfo(hid_t zaID, int fldgroup, const char *aliasname, int *length, char *buffer)
{
  herr_t        status     = FAIL;               /* return status variable   */

  int           type       = FAIL;               /* type-value of an object  */

  size_t        size       = 0;                  /* Size of the name buffer  */

  hid_t         fid        = FAIL;               /* HDF-EOS file ID          */
  hid_t         gid        = FAIL;               /* "ZAS"  group ID          */
  hid_t         groupID    = FAIL;               /* Field group ID           */

  long          idx        = FAIL;               /* za index                 */

  H5G_stat_t    *statbuf   = (H5G_stat_t *)NULL; /* pointer to a structure   */

  char          errbuf[HE5_HDFE_ERRBUFSIZE];     /* Error message buffer     */

  CHECKPOINTER(aliasname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAaliasinfo", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAaliasinfo", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  /* Get the group ID */
  /* ---------------- */
  if (fldgroup == HE5_HDFE_DATAGROUP)
    groupID = HE5_ZAXZa[idx].data_id;
  else
    {	  
      sprintf(errbuf, "Invalid input group flag. \n");
      H5Epush(__FILE__, "HE5_ZAaliasinfo", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  if (buffer == NULL)
    {
      /* Get the string length of a "real" name  */
      /* --------------------------------------  */
      statbuf = (H5G_stat_t *)calloc(1, sizeof(H5G_stat_t));   
      if (statbuf == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory for \"statbuf\". \n");
	  H5Epush(__FILE__, "HE5_ZAaliasinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* Get the Object Info */
      /* ------------------- */
      status = H5Gget_objinfo(groupID, aliasname, 0, statbuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot get information about \"%s\" alias. \n", aliasname);
	  H5Epush(__FILE__, "HE5_ZAaliasinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}

      /* LINK name length with a null terminator */
      /* --------------------------------------- */
      size = statbuf->linklen;

      /* Object type (dataset, link, etc) */
      /* -------------------------------- */
      type   = statbuf->type;
  
      /* Make sure that the object is a LINK */
      /* ----------------------------------- */
      if (type != H5G_LINK)
	{
	  sprintf(errbuf,"The object named \"%s\" is not a symbolic link.\n", aliasname);
	  H5Epush(__FILE__, "HE5_ZAaliasinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
	  
      *length = (int)size;
	  
      if ( statbuf != (H5G_stat_t *)NULL ) free(statbuf);
	  
    }
  
  if (buffer != NULL)
    {
      size = (size_t)*length;
	  
      /* Get the "real" name by an "alias" */
      /* --------------------------------- */
      status = H5Gget_linkval(groupID, aliasname, size, buffer);
      if (status == FAIL) 
	{
	  sprintf(errbuf,"Cannot get the real name for the alias named \"%s\".\n", aliasname);
	  H5Epush(__FILE__, "HE5_ZAaliasinfo", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  return(FAIL);
	}
    }
  
 COMPLETION:
  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAgetaliaslist                                               |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of aliases and the list of aliases in   |
|                "Data Fields" group                                          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nalias         long                number of aliases in "Data Fields"      |
|                                     group                                   |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               za structure ID                         |
|  fldgroup       int                 field group flag                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  aliaslist      char*               list of aliases in "Data Fields" group  |
|                                     (comma-separated list)                  |
|  strbufsize     long*               length of aliases list                  |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 05    S.Zhao       Original development                                |
|  Mar 06    Abe Taaheri  Added code to get alias strbufsize only when user   |
|                         passes aliaslist as NULL pointer                    |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAgetaliaslist(hid_t zaID, int fldgroup, char *aliaslist, long *strbufsize)
{
  long            nalias  = FAIL;             /* Number of aliases (return)  */
  long            idx     = FAIL;             /* za index                    */

  herr_t          status  = FAIL;             /* Return status variable      */

  hid_t           fid     = FAIL;             /* HDF-EOS file ID             */
  hid_t           gid     = FAIL;             /* "ZAS" group ID              */

  char            *fieldlist = (char *)NULL;  /* List of Data fields         */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer        */
  long            fldstrbuf = FAIL;           /* Buffer size for Data fields */
  long            nflds   = FAIL;             /* Number of Data fields       */
  long            nfldalias = FAIL;           /* Number of fields & aliases  */
  char            *fldalias = (char *)NULL;   /* List of fields & aliases    */
  long            fldaliasbuf = FAIL;         /* Buffer size for fields & 
						 aliases */

  char            *buff   = (char *)NULL;     /* Pointer to temporary buffer */
  char            *comma  = (char *)NULL;     /* Pointer to comma            */
  char            *comma1 = (char *)NULL;     /* Pointer to comma            */
  char            *nameptr = (char *)NULL;    /* Pointer to each name        */
  char            *tmpbuf  = (char *)NULL;    /* Pointer to temp buffer      */
  long            nameflag = FAIL;            /* Name flag                   */

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAgetaliaslist", &fid, &gid, &idx);
  if(status == SUCCEED )
    {
      /* If aliaslist and strbufsize desired */
      /* ----------------------------------- */
      if (fldgroup == HE5_HDFE_DATAGROUP)    
        {
	  
          /* Call "HE5_ZAnentries" routine to get number of fields */
          /* ----------------------------------------------------- */
          nflds = HE5_ZAnentries(zaID, HE5_HDFE_NENTDFLD, &fldstrbuf);
          if (nflds == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the number of fields in \"Data Fields\" group. \n");
	      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
		      H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	  
          /* Allocate memory for fields list buffer */
          /* -------------------------------------- */
          fieldlist = (char *) calloc(fldstrbuf+6, sizeof(char));
          if(fieldlist == NULL)
	    {
	      sprintf(errbuf,"Cannot allocate memory.\n");
	      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
		      H5E_RESOURCE, H5E_NOSPACE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      return(FAIL);
	    }
	  
          /* Call "HE5_ZAinquire" routine to get the list of Data fields */
          /* ----------------------------------------------------------- */
          nflds = HE5_ZAinquire(zaID, fieldlist, NULL, NULL);
          if (nflds == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the list of fields in \"Data Fields\" group. \n");
	      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
		      H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(fieldlist);
	      return(FAIL);
	    }
	  
          /* Call "HE5_ZAinqfldalias" routine to get number of fields & 
	     aliases */
          /* -------------------------------------------------------- */
          nfldalias = HE5_ZAinqfldalias(zaID, NULL, &fldaliasbuf);
          if (nfldalias == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the number of fields & aliases in \"Data Fields\" group. \n");
	      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
		      H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(fieldlist);
	      return(FAIL);
	    }
	  
          /* Allocate memory for fields & aliases list buffer */
          /* ------------------------------------------------ */
          fldalias = (char *) calloc(fldaliasbuf+6, sizeof(char));
          if(fldalias == NULL)
	    {
	      sprintf(errbuf,"Cannot allocate memory.\n");
	      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
		      H5E_RESOURCE, H5E_NOSPACE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(fieldlist);
	      return(FAIL);
	    }
	  
          /* Call "HE5_ZAinqfldalias" routine to get the list of fields & 
	     aliases */
          /* ---------------------------------------------------------- */
          nfldalias = HE5_ZAinqfldalias(zaID, fldalias, &fldaliasbuf);
          if (nfldalias == FAIL)
	    {
	      sprintf(errbuf, "Cannot get the list of fields & aliases in \"Data Fields\" group. \n");
	      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
		      H5E_ARGS, H5E_BADVALUE, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(fieldlist);
	      free(fldalias);
	      return(FAIL);
	    }
	  
          *strbufsize = strlen(fldalias) - strlen(fieldlist);
          if (*strbufsize <= 0)
	    {
	      sprintf(errbuf, "Cannot find the aliases.\n");
	      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
		      H5E_OHDR, H5E_NOTFOUND, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(fieldlist);
	      free(fldalias);
	      return(FAIL);
	    }
          else
	    {
	      nalias = 0;
	      
	      /* Allocate memory for temporary buffer */
	      /* ------------------------------------ */
	      buff = (char *)calloc(strlen(fldalias)+6, sizeof(char));
	      if (buff == NULL)
		{
		  sprintf(errbuf, "Cannot allocate memory for \"buff\". \n");
		  H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
			  H5E_RESOURCE, H5E_NOSPACE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(fieldlist);
		  free(fldalias);
		  return(FAIL);
		}
	      
	      /* Allocate memory for aliases */
	      /* --------------------------- */
	      tmpbuf = (char *)calloc(*strbufsize+6, sizeof(char));
	      if (tmpbuf == NULL)
		{
		  sprintf(errbuf, "Cannot allocate memory for \"tmpbuf\". \n");
		  H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
			  H5E_RESOURCE, H5E_NOSPACE, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  free(fieldlist);
		  free(fldalias);
		  free(buff);
		  return(FAIL);
		}
	      
	      strcpy(buff, fldalias);
	      strcat(buff, ",");
	      strcat(fieldlist, ",");
	      
	      /* Find first comma and make pointer pointing to it */
	      /* ------------------------------------------------ */
	      comma = strchr(buff, ',');
	      while(comma != NULL)
		{
		  /* Allocate memory for individual entry */
		  /* ------------------------------------ */
		  nameptr = (char *)calloc(comma-buff+6, sizeof(char));
		  if (nameptr == NULL)
		    {
		      sprintf(errbuf, "Cannot allocate memory for \"nameptr\". \n");
		      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
			      H5E_RESOURCE, H5E_NOSPACE, errbuf);
		      HE5_EHprint(errbuf, __FILE__, __LINE__);
		      free(fieldlist);
		      free(fldalias);
		      free(buff);
		      free(tmpbuf);
		      return(FAIL);
		    }
		  
		  /* Pick up an individual entry and put it to the "nameptr" */
		  /* ------------------------------------------------------- */
		  memmove(nameptr,buff,comma-buff);
		  strcat(nameptr, ",");
		  
		  nameflag = (strstr(fieldlist,nameptr) == NULL) ? 1 : 0;
		  if (nameflag == 1)
		    {
		      comma1 = strchr(tmpbuf, ',');
		      if(comma1 == NULL)
			strcpy(tmpbuf, nameptr);
		      else
			strcat(tmpbuf, nameptr);
		      
		      nalias++;
		    }
		  
		  memmove(buff,comma+1,strlen(comma+1)+1);
		  comma = strchr(buff, ',');
		  
		  if (nameptr != NULL) free(nameptr);
		}
	    }
	  
	  if(aliaslist == NULL)/* just return buffersize 
				  and number of aliases */
	    {
	      if(nalias > 0 )
		{
		  if (fieldlist != NULL) free(fieldlist);
		  if (fldalias != NULL) free(fldalias);
		  if (buff != NULL) free(buff);
		  if (tmpbuf != NULL) free(tmpbuf);
		  return (nalias);
		}
	      else
		{
		  sprintf(errbuf, "Cannot find the aliases.\n");
		  H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
			  H5E_OHDR, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  if(fieldlist != NULL) free(fieldlist);
		  if(fldalias != NULL) free(fldalias);
		  if (buff != NULL) free(buff);
		  if (tmpbuf != NULL) free(tmpbuf);
		  return(FAIL);
		}
	    }
	  else
	    {
	      if(nalias > 0 )
		{
		  strncpy(aliaslist, tmpbuf, *strbufsize-1);
		  aliaslist[*strbufsize-1] = '\0';
		}
	      else
		{
		  sprintf(errbuf, "Cannot find the aliases.\n");
		  H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
			  H5E_OHDR, H5E_NOTFOUND, errbuf);
		  HE5_EHprint(errbuf, __FILE__, __LINE__);
		  if(fieldlist != NULL) free(fieldlist);
		  if(fldalias != NULL) free(fldalias);
		  if (buff != NULL) free(buff);
		  if (tmpbuf != NULL) free(tmpbuf);
		  return(FAIL);
		}
	    }
        }
      else
        {
          sprintf(errbuf, "Invalid input field group flag. \n");
          H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
		  H5E_OHDR, H5E_NOTFOUND, errbuf);
          HE5_EHprint(errbuf, __FILE__, __LINE__);
          return(FAIL);
        }
    }
  else
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAgetaliaslist", __LINE__, 
	      H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  if (fieldlist != NULL) free(fieldlist);
  if (fldalias != NULL) free(fldalias);
  if (buff != NULL) free(buff);
  if (tmpbuf != NULL) free(tmpbuf);
  
  return (nalias);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAfldrename                                                  |
|                                                                             |
|  DESCRIPTION:  Changes the field name                                       |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  status         herr_t                   Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    zaID         hid_t                    ZA structure ID                    |
|    oldfieldname char                     Original field name                |
|                                                                             |
|  OUTPUTS:                                                                   |
|    newfieldname char                     New field name                     |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Aug 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_ZAfldrename(hid_t zaID, char *oldfieldname, const char *newfieldname)
{
  
  herr_t   status     = FAIL;          /* Return status variable */

  int      fldgroup   = FAIL;          /* Field group flag       */
  int      k;                          /* Loop index             */

  hid_t    fid        = FAIL;          /* HDF-EOS file ID        */
  hid_t    gid        = FAIL;          /* "ZAS"  group ID        */
  hid_t    groupID    = FAIL;          /* Field group ID         */

  long     idx        = FAIL;          /* za index               */

  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */



  HE5_LOCK;
  CHECKPOINTER(oldfieldname);
  CHECKNAME(newfieldname);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAfldrename", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAfldrename", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Find out the field group */
  /* ------------------------ */
  fldgroup = HE5_ZAfldsrch(zaID,oldfieldname,NULL,NULL,NULL,NULL);
  if (fldgroup == HE5_HDFE_DATAGROUP)
    groupID = HE5_ZAXZa[idx].data_id;
  else
    {	  
      sprintf(errbuf, "Error returning field group flag. \n");
      H5Epush(__FILE__, "HE5_ZAfldrename", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Rename the field */
  /* ---------------- */
  status = H5Gmove(groupID, oldfieldname, newfieldname);
  if (status == FAIL)
    {	  
      sprintf(errbuf, "Cannot change the field name from \"%s\" to \"%s\". \n", oldfieldname, newfieldname);
      H5Epush(__FILE__, "HE5_ZAfldrename", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Update Table */
  /* ------------ */
  if (fldgroup == HE5_HDFE_DATAGROUP)
    {
      for (k = 0; k < HE5_ZAXZa[idx].nDFLD; k++)
	{
	  if (strcmp(HE5_ZAXZa[idx].ddataset[k].name, oldfieldname) == 0 )
	    strcpy(HE5_ZAXZa[idx].ddataset[k].name, newfieldname);
	}
    }
  
 COMPLETION:
  HE5_UNLOCK;
  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAmountexternal                                              |
|                                                                             |
|  DESCRIPTION:  Mounts external data file onto "Data Fields"                 |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|    extfID       hid_t                    External file ID                   |
|                                                                             |
|  INPUTS:                                                                    |
|    zaID         hid_t                    ZA structure ID                    |
|    fldgroup     int                      Group flag                         |
|    extfilename  char                     External File Name                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Aug 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
hid_t 
HE5_ZAmountexternal(hid_t zaID, int fldgroup, const char *extfilename)
{
  
  hid_t    extfID     = FAIL;           /* External file ID (return)   */
  hid_t    fid        = FAIL;           /* HDF-EOS file ID             */
  hid_t    gid        = FAIL;           /* "ZAS"  group ID             */
  herr_t   status     = FAIL;           /* Return status variable      */

  long     idx        = FAIL;           /* za index                    */

  char     grpname[HE5_HDFE_NAMBUFSIZE];/* Group name buffer           */
  char     errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer        */


  HE5_LOCK; 
  CHECKNAME(extfilename);

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAmountexternal", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAmountexternal", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  strcpy(grpname,"/HDFEOS/ZAS/");
  strcat(grpname, HE5_ZAXZa[idx].zaname);

  if (fldgroup == HE5_HDFE_DATAGROUP)
    strcat(grpname,"/Data Fields");


  /* Open external data file */
  /* ----------------------- */
  extfID = H5Fopen(extfilename, H5F_ACC_RDONLY, H5P_DEFAULT);
  if ( extfID == FAIL )
    {
      sprintf(errbuf, "Cannot open external file named \"%s\". \n", extfilename);
      H5Epush(__FILE__, "HE5_ZAmountexternal", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }



  /* Mount external file */
  /* ------------------- */
  status = H5Fmount(fid, grpname, extfID, H5P_DEFAULT);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot mount external file onto \"%s\" group. \n", grpname);
      H5Epush(__FILE__, "HE5_ZAmountexternal", __LINE__, H5E_FILE, H5E_MOUNT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

 COMPLETION:  
  HE5_UNLOCK;
  return(extfID);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION:     ZAunmount                                                    |
|                                                                             |
|  DESCRIPTION:  Unmounts external data file from "Data Fields"               |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  status         herr_t                   Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    zaID         hid_t                    ZA structure ID                    |
|    fldgroup     int                      Group flag                         |
|    fileID       hid_t                    ID of file  to be unmounted        |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Aug 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t 
HE5_ZAunmount(hid_t zaID, int fldgroup, hid_t fileID)
{
  herr_t   status     = FAIL;           /* Return status variable */

  hid_t    fid        = FAIL;           /* HDF-EOS file ID        */
  hid_t    gid        = FAIL;           /* "ZAS"  group ID        */

  long     idx        = FAIL;           /* za index               */

  char     grpname[HE5_HDFE_NAMBUFSIZE];/* Group name buffer      */
  char     errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer   */


  HE5_LOCK;

  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAunmount", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAunmount", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  strcpy(grpname,"/HDFEOS/ZAS/");
  strcat(grpname, HE5_ZAXZa[idx].zaname);

  if (fldgroup == HE5_HDFE_DATAGROUP)
    strcat(grpname,"/Data Fields");


  status = H5Funmount(fid, grpname);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot unmount external file from \"%s\" group. \n", grpname);
      H5Epush(__FILE__, "HE5_ZAunmount", __LINE__, H5E_FILE, H5E_MOUNT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  status = H5Fclose(fileID);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot close external file. \n");
      H5Epush(__FILE__, "HE5_ZAunmount", __LINE__, H5E_FILE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

 COMPLETION:
  HE5_UNLOCK;
  return(status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAreadexternal                                               |
|                                                                             |
|  DESCRIPTION:  Reads external dataset                                       |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  status         herr_t                   Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    zaID         hid_t                    ZA structure ID                    |
|    fldgroup     int                      Group flag                         |
|    fieldname    const char*              Field name to read                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|    buffer       void*                    Output data bufer                  |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Aug 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAreadexternal(hid_t zaID, int fldgroup, const char *fieldname, void *buffer)
{
  herr_t   status     = FAIL;/* return status variable      */
 
  hid_t    fid        = FAIL;/* HDF-EOS file ID             */
  hid_t    gid        = FAIL;/* "ZAS"  group ID             */
  hid_t    groupID    = FAIL;/* Group ID                    */
  hid_t    datasetID  = FAIL;/* Dataset ID                  */
  hid_t    typeID     = FAIL;/* File data type ID           */
  hid_t    mtypeID    = FAIL;/* Memory datatype ID          */
 
  long     idx        = FAIL;/* za index                    */
 
  char     grpname[HE5_HDFE_NAMBUFSIZE];/* Group name buffer    */
  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */
 
 
  HE5_LOCK;
  CHECKPOINTER(fieldname);
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAreadexternal", &fid, &gid, &idx);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Checking for za ID failed. \n");
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  strcpy(grpname,"/HDFEOS/ZAS/");
  strcat(grpname, HE5_ZAXZa[idx].zaname);
 
  if (fldgroup == HE5_HDFE_DATAGROUP)
    strcat(grpname,"/Data Fields");
 
 
  /* Open the group containing mounted file */
  /* -------------------------------------- */
  groupID = H5Gopen(fid, grpname);
  if ( groupID == FAIL )
    {
      sprintf(errbuf, "Cannot open group containing mounted file. \n");
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Open the dataset */
  /* ---------------- */
  datasetID = H5Dopen(groupID, fieldname);
  if ( datasetID == FAIL )
    {
      sprintf(errbuf, "Cannot open mounted dataset. \n");
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Get the data type ID */
  /* -------------------- */
  typeID = H5Dget_type(datasetID);
  if ( typeID == FAIL )
    {
      sprintf(errbuf, "Cannot get the data type ID of  mounted dataset. \n");
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Get memory data type ID */
  /* ----------------------- */
  mtypeID    = HE5_EHdtype2mtype(typeID);
  if (mtypeID == FAIL)
    {
      sprintf(errbuf, "Cannot convert to memory data type.") ;
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Read the mounted dataset */
  /* ------------------------ */
  status = H5Dread(datasetID, mtypeID, H5S_ALL, H5S_ALL, H5P_DEFAULT, buffer);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot read mounted data.") ;
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
 
  /* Release data type ID */
  /* -------------------- */
  status = H5Tclose(typeID);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot release data type ID.") ;
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_DATATYPE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Release dataset ID */
  /* ------------------ */
  status = H5Dclose(datasetID);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot release dataset ID.") ;
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_DATASET, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Release the group ID */
  /* -------------------- */
  status = H5Gclose(groupID);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot release group ID.") ;
      H5Epush(__FILE__, "HE5_ZAreadexternal", __LINE__, H5E_OHDR, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
 COMPLETION:
  HE5_UNLOCK;
  return(status);
}
 
 
/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAsetextdata                                                 |
|                                                                             |
|  DESCRIPTION: Sets external data files.                                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           hid_t               ZA structure ID                         |
|  filelist       const char*         list of external files                  |
|  offset         off_t               array of offsets (in bytes) from the    |
|                                     beginning of the file to the location   |
|                                     in the file where the data starts       |
|  size           hsize_t             array of sizes (in bytes) reserved in   |
|                                     the file for the data.                  |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Aug 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
herr_t
HE5_ZAsetextdata(hid_t zaID, const char *filelist, off_t offset[], hsize_t size[])
{
  herr_t          status       = FAIL;        /* return status variable */
 
  int             i;                          /* Loop index             */
 
  hid_t           fid          = FAIL;        /* HDF-EOS file ID        */
  hid_t           gid          = FAIL;        /* "HDFEOS" group ID      */
 
  long            idx          = FAIL;        /* za index               */
 
  char            *namebuf     = (char *)NULL;/* File list buffer       */
  char            *filename    = (char *)NULL;/* File name buffer       */
  char            *comma       = (char *)NULL;/* Pointer to comma       */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
 
 
  HE5_LOCK;
  CHECKPOINTER(filelist);
  CHECKPOINTER(offset);
  CHECKPOINTER(size);
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAsetextdata", &fid, &gid, &idx);
  if(status == FAIL)
    {
      sprintf(errbuf,"Checking for valid za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAsetextdata", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
 
  /* Get current dataset creation property list */
  /* ------------------------------------------ */
  if( HE5_ZAXZa[idx].plist == FAIL)
    HE5_ZAXZa[idx].plist = H5Pcreate(H5P_DATASET_CREATE);
 
 
  /* Allocate space for namebuf, copy dimlist into it, & append comma */
  /* ---------------------------------------------------------------- */
  namebuf = (char *)calloc(strlen(filelist) + 64, sizeof(char));
  if(namebuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAsetextdata", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory.");
      HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
 
  /*
********************************************************************
*   C H E C K  T H E   C O N T E N T   O F  filelist  S T R I N G  *
********************************************************************
*/
 
  strcpy(namebuf, filelist);
  strcat(namebuf, ",");
 
  /* Find comma */
  /* ---------- */
  comma = strchr(namebuf, ',');
 
  i = 0;
 
  /* Parse the list of file names */
  /* ---------------------------- */
  while (comma != NULL)
    {
      /* Allocate memory for filename buffer */
      /* ----------------------------------- */
      filename = (char *) calloc(comma - namebuf + 1, sizeof(char));
      if (filename == NULL)
	{
	  sprintf(errbuf, "Cannot allocate memory for filename. \n") ;
	  H5Epush(__FILE__, "HE5_ZAsetextdata", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(namebuf);
 
	  return(FAIL);
	}
 
      /* Copy file list entry to filename */
      /* -------------------------------- */
      memmove(filename, namebuf, comma - namebuf);
      filename[comma-namebuf] = 0;
 
      /* Set the property list */
      /* --------------------- */
      status = H5Pset_external(HE5_ZAXZa[idx].plist, filename, offset[i], size[i]);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot set external dataset property list.\n");
	  H5Epush(__FILE__, "HE5_ZAsetextdata", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(namebuf);
	  free(filename);
	  return(status);
	}
 
 
      /* Go to next file  entry, find next comma, ... */
      /* ============================================ */
      memmove(namebuf, comma + 1, strlen(comma + 1) + 1);
      comma = strchr(namebuf, ',');
 
      if (filename != NULL) free(filename);
 
      i++;
    }
 
  if (namebuf != NULL) free(namebuf);
 
 COMPLETION:
  HE5_UNLOCK;
  return (status);
}
 
 
 
/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAgetextdata                                                 |
|                                                                             |
|  DESCRIPTION: Gets external data files information.                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nfiles         int                 number of external files  SUCCEED,      |
|                                               (-1) FAIL                     |
|  INPUTS:                                                                    |
|  zaID           hid_t               za structure ID                         |
|  fieldname      char*               External field name                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|  namelength     size_t              Length of each name entry               |
|  filelist       char*               List of file names                      |
|  offset         off_t               array of offsets (in bytes) from the    |
|                                     beginning of the file to the location   |
|                                     in the file where the data starts       |
|  size           hsize_t             array of sizes (in bytes) reserved in   |
|                                     the file for the data.                  |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Aug 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_ZAgetextdata(hid_t zaID, char *fieldname, size_t namelength, char *filelist, off_t offset[], hsize_t size[])
{
  int             nfiles       = FAIL;        /* Number of ext. files   */
  int             i;                          /* Loop index             */
 
  size_t          slen         = 0;           /* String length          */
 
  herr_t          status       = FAIL;        /* return status variable */
 
  hid_t           fid          = FAIL;        /* HDF-EOS file ID        */
  hid_t           gid          = FAIL;        /* "HDFEOS" group ID      */
  hid_t           fieldID      = FAIL;        /* Data field ID          */
  hid_t           plist        = FAIL;        /* Property list ID       */
 
  off_t           off          = 0;           /* Offset of data segment */
 
  hsize_t         sz           = 0;           /* Size of data segment   */
 
  long            idx          = FAIL;        /* za index               */
 
  char            *filename    = (char *)NULL;/* File name buffer       */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */
 
 
 
  HE5_LOCK;
  CHECKPOINTER(fieldname);
 
  /* Get HDF-EOS file ID, "HDFEOS" group ID and za index */
  /* --------------------------------------------------- */
  status = HE5_ZAchkzaid(zaID, "HE5_ZAgetextdata", &fid, &gid, &idx);
  if(status == FAIL)
    {
      sprintf(errbuf,"Checking for valid za ID failed.\n");
      H5Epush(__FILE__, "HE5_ZAgetextdata", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(status);
    }
 
  /* Check out the field group and get field ID */
  /* ------------------------------------------ */
  HE5_ZAfldsrch(zaID,fieldname,&fieldID,NULL,NULL,NULL);
  if( fieldID == FAIL )
    {
      sprintf(errbuf, "Cannot get the field ID for the \"%s\" field.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetextdata", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Get the property list ID */
  /* ------------------------ */
  plist = H5Dget_create_plist(fieldID);
  if ( plist == FAIL )
    {
      sprintf(errbuf, "Cannot get the property list ID for the \"%s\" data field.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetextdata", __LINE__, H5E_PLIST, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Get the number of external files */
  /* -------------------------------- */
  nfiles = H5Pget_external_count(plist);
  if ( nfiles < 0 )
    {
      sprintf(errbuf, "Cannot get the number of external files.\n");
      H5Epush(__FILE__, "HE5_ZAgetextdata", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
 
  /* Allocate memory for file name */
  /* ----------------------------- */
  filename = (char *)calloc(HE5_HDFE_NAMBUFSIZE, sizeof(char));
  if(filename == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAgetextdata", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory.");
      HE5_EHprint("Error: Cannot allocate memory, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  strcpy(filelist,"");
 
  /* Loop through all external files */
  /* ------------------------------- */
  for (i = 0; i < nfiles; i++)
    {
      strcpy(filename,"");
 
      /* Get the file name, offset, and size */
      /* ----------------------------------- */
      status = H5Pget_external(plist, i, namelength, filename, &off, &sz);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot get information about external file.\n");
	  H5Epush(__FILE__, "HE5_ZAgetextdata", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(filename);
	  return(FAIL);
	}
 
      offset[i] = off;
      size[i]   = sz;
       
      strcat(filelist, filename);
 
      /* Append comma */
      /* ------------ */
      if (nfiles > 1 && i < nfiles - 1)
	strcat(filelist,",");
 
    }
 
  slen = strlen(filelist);
 
  filelist[slen] = 0;
 
  if (filename != NULL) free(filename);
 
 
  /* Release property list ID */
  /* ------------------------ */
  status = H5Pclose(plist);
  if (status == FAIL )
    {
      sprintf(errbuf, "Cannot release property list ID.\n");
      H5Epush(__FILE__, "HE5_ZAgetextdata", __LINE__, H5E_PLIST, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
 COMPLETION:
  HE5_UNLOCK;
  return (nfiles);
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
|  FUNCTION: HE5_ZAopenF (FORTRAN wrapper)                                    |
|                                                                             |
|  DESCRIPTION: Opens or creates HDF-EOS file in order to create, read, or    |
|               write a ZA.                                                   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|     FileID      int     None        HDF-EOS file ID                         |
|                                                                             |
|  INPUTS:                                                                    |
|     filename    char*    None       File name string                        |
|     Flags       int      None       File access code                        |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|     None                                                                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date       Programmer   Description                                       |
|  ========   ============  ================================================= |
|  Jul 02      S.ZHAO       Original development                              |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_ZAopenF(char *filename, int Flags)
{
  hid_t     fid     = FAIL;/* HDF5 file ID          */

  int       FileID  = FAIL;/* hdf-eos file ID       */
  
  uintn     flags   = 9999;/* HDF5 file access code */

  char      *errbuf = NULL;/* Error message buffer  */


  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf  = (char * )calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAopenF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory to error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);

      return(FAIL);
    }  

  /* Set up the file access flag */
  /* --------------------------- */
  if (Flags == HE5F_ACC_RDWR || Flags == HDF5_ACC_RDWR) flags = H5F_ACC_RDWR;
  else if (Flags == HE5F_ACC_RDONLY || Flags == HDF5_ACC_RDONLY) flags = H5F_ACC_RDONLY;
  else if (Flags == HE5F_ACC_TRUNC || Flags == HDF5_ACC_CREATE)  flags = H5F_ACC_TRUNC;
  else
    {
      sprintf(errbuf, "File access flag is not supported. \n");
      H5Epush(__FILE__, "HE5_ZAopenF", __LINE__, H5E_ARGS, H5E_UNSUPPORTED, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }  
	


  /* Call HE5_EHopen to open file */
  /* ---------------------------- */
  fid = HE5_EHopen(filename, flags, H5P_DEFAULT);
  if(fid == FAIL)
    {
      sprintf(errbuf, "Cannot open the file \"%s\". Check the file name. \n", filename);
      H5Epush(__FILE__, "HE5_ZAopenF", __LINE__, H5E_FILE, H5E_CANTOPENFILE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }  

  free(errbuf);

  FileID = (int)fid;
  return(FileID);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAcreateF (FORTRAN wrapper)                                  |
|                                                                             |
|  DESCRIPTION: Creates a new za structure and returns za ID                  |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|   zaID          int          None        ZA structure ID                    |
|                                                                             |
|  INPUTS:                                                                    |
|   FileID        int          None        HDF-EOS file ID                    |
|   zaname        char*        None        za name string                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|   None                                                                      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_ZAcreateF(int FileID, char *zaname)
{
  hid_t           zaID           = FAIL;/* return value of za ID           */
  hid_t           fid            = FAIL;/* HDF-EOS file ID                 */

  int             ZaID           = FAIL;/* int Za ID                       */
  
  char            *errbuf =(char *)NULL;/* Buffer for error message        */

  /* Allocate memory for error buffer */
  /* -------------------------------- */
  errbuf = (char *)calloc(HE5_HDFE_DIMBUFSIZE, sizeof(char ) ) ;
  if ( errbuf == NULL )
    {
      H5Epush(__FILE__, "HE5_ZAcreateF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  

  fid = (hid_t)FileID;
  
  zaID = HE5_ZAcreate(fid, zaname);
  if (zaID == FAIL)
    {
      sprintf(errbuf,"Cannot create za. \n");
      H5Epush(__FILE__, "HE5_ZAcreateF", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
	
  ZaID = (int)zaID;	
  
  /* Deallocate memory */
  /* ----------------- */
  free(errbuf);
  
  return(ZaID);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAattachF     (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION:  Attaches to an existing za within the file.                  |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ZaID           int                 ZA structure ID                         |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 HDF-EOS file ID                         |
|  zaname         char                ZA structure name                       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date        Programmer    Description                                     |
|  =========   ============  ==============================================   |
|  Jul 02       S.ZHAO       Original development                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_ZAattachF(int FileID, char *zaname)
{
  int             ZaID       = FAIL;/* int za ID (return)            */

  hid_t           zaID       = FAIL;/* za ID (return value)          */ 
  hid_t           fid        = FAIL;/* HDF-EOS file ID               */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer*/

  fid = (hid_t)FileID;

  zaID = HE5_ZAattach(fid, zaname);
  if (zaID == FAIL)
    {
      sprintf(errbuf,"Cannot attach to the za \"%s\". \n", zaname);
      H5Epush(__FILE__, "HE5_ZAattachF", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  ZaID = (int)zaID;

  return (ZaID);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdetachF  (FORTRAN wrapper)                                 |
|                                                                             |
|  DESCRIPTION: Detachs za structure and performs housekeeping                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZAdetachF(int ZaID)
{
  int            ret     = FAIL;/* int return status              */

  hid_t          zaID    = FAIL;/* HDF-EOS za ID                  */
  herr_t         status  = FAIL;/* routine return status variable */

  char           errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */

  zaID = (hid_t)ZaID;

  status = HE5_ZAdetach(zaID);
  if (status == FAIL)
    {
      sprintf(errbuf,"Cannot detach from the za. \n");
      H5Epush(__FILE__, "HE5_ZAdetachF", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  ret = (int)status;	
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAcloseF    (FORTRAN wrapper)                                |
|                                                                             |
|  DESCRIPTION: Closes HDF-EOS file                                           |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  FileID         int                 File ID                                 |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZAcloseF(int FileID)
{
  int      ret    = FAIL;/* int return status variable     */

  hid_t    fid    = FAIL;/* HDF-EOS file ID                */

  herr_t   status = FAIL;/* routine return status variable */

  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */


  fid = (hid_t)FileID;
  status = HE5_ZAclose(fid);
  if (status == FAIL)
    {
      sprintf(errbuf,"Cannot close the file. \n");
      H5Epush(__FILE__, "HE5_ZAcloseF", __LINE__, H5E_FILE, H5E_CLOSEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefdimF                                                    |
|                                                                             |
|  DESCRIPTION: Defines numerical value of dimension (FORTRAN wrapper)        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  dimname        char                dimension name                          |
|  dim            long                dimension size                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAdefdimF(int ZaID, char *dimname, long dim)
{
  int       ret       = SUCCEED;/* (int) return status variable  */ 

  hid_t     zaID      = FAIL;   /* HDF5 type za ID               */

  herr_t	status    = SUCCEED;/* HDF5 type return status       */
 
  hsize_t	tdim      = 0;      /* temporary(dimension) variable */
  
  char		*errbuf   = (char *)NULL;/* Error message buffer     */
  
  
  errbuf = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  
  tdim    = (hsize_t)dim;	
  zaID = (hid_t)ZaID;

  status = HE5_ZAdefdim(zaID, dimname, tdim);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Error calling HE5_ZAdefdim from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_ZAdefdimF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
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
|  FUNCTION: HE5_ZAdefineF                                                    |
|                                                                             |
|                                                                             |
|  DESCRIPTION: Defines data field within ZA structure (FORTRAN)              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         herr_t              return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           int                 ZA structure ID                         |
|  fieldname      char                fieldname                               |
|  fortdimlist    char                Dimension list (comma-separated list)   |
|                                         FORTRAN dimesion order              |
|  fortmaxdimlist char                Max Dimension list (FORTRAN order)      |
|  numtype        int                 field type                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAdefineF(int zaID, char *fieldname, char *fortdimlist, char *fortmaxdimlist, int numtype)
{
  int       ret        = SUCCEED;/* (int) return status variable   */  

  herr_t    status     = SUCCEED;/* routine return status variable */

  hid_t     ZaID       = FAIL;   /* HDF5 type za ID                */
  hid_t	    numbertype = FAIL;   /* HDF5 type data type ID         */
  
  char  	*dimlist   = (char *)NULL;/* Dimension list (C order)  */
  char		*maxdimlist= (char *)NULL;/* Max. dimension list (C)   */
  char		*errbuf    = (char *)NULL;/* error message buffer      */

  ZaID = (hid_t)zaID;

  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      H5Epush(__FILE__, "HE5_ZAdefineF", __LINE__, H5E_DATATYPE, H5E_NOTFOUND, "Cannot convert datatype for FORTRAN wrapper.");
      HE5_EHprint("Error: Cannot convert datatype for FORTRAN wrapper, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  if(strcmp(fortmaxdimlist,"") == 0)
    fortmaxdimlist = NULL;
  
  
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAdefineF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Allocate space for C order dimension list */
  /* ----------------------------------------- */
  dimlist = (char *)calloc(strlen(fortdimlist) + 1, sizeof(char));
  if(dimlist == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for dimlist.\n");
      H5Epush(__FILE__, "HE5_ZAdefineF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  /* Reverse entries in dimension list (FORTRAN -> C) */
  /* ------------------------------------------------ */
  status = HE5_EHrevflds(fortdimlist, dimlist);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot reverse entries in dimension list.\n");
      H5Epush(__FILE__, "HE5_ZAdefineF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(dimlist);
      return(FAIL);
    }

  if (fortmaxdimlist != NULL)
    {
      /* Allocate space for C order max. dim. list */
      /* ----------------------------------------- */
      maxdimlist = (char *)calloc(strlen(fortmaxdimlist) + 1, sizeof(char));
      if(maxdimlist == NULL)
	{
	  sprintf(errbuf,"Cannot allocate memory for maxdimlist.\n");
	  H5Epush(__FILE__, "HE5_ZAdefineF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(dimlist);
	  free(errbuf);
	  return(FAIL);
	}


      /* Reverse entries in dimension list (FORTRAN -> C) */
      /* ------------------------------------------------ */
      status = HE5_EHrevflds(fortmaxdimlist, maxdimlist);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot reverse entries in maximum dimension list.\n");
	  H5Epush(__FILE__, "HE5_ZAdefineF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  free(dimlist);
	  free(maxdimlist);
	  return(FAIL);
	}
    }
  else
    {
      maxdimlist = NULL;
    }
	  
  
  /* Call Define Field routine */
  /* ------------------------- */
  status = HE5_ZAdefinefield(ZaID, "Data Fields", fieldname, dimlist, maxdimlist, numbertype);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot define \"%s\" field.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAdefineF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(dimlist);
      if (maxdimlist!=NULL) 
	free(maxdimlist);
	  
      return(FAIL);
    }
  
  free(dimlist);
  if (maxdimlist != NULL) 
    free(maxdimlist);
  
  free(errbuf);

  ret = (int)status;
  return (ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefcompF      (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION: Defines compression type and parameters                       |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int     None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  compcode       int                 compression code                        |
|  compparm       int                 compression parameters                  |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:   Before calling this function, storage must be CHUNKED             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_ZAdefcompF(int ZaID, int compcode,  int *compparm)
{
  int             ret      = FAIL; /* routine return status variable   */

  herr_t          status   = FAIL; /* routine return status variable   */

  hid_t           zaID     = FAIL; /* HDF5 type  za ID                 */

  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer */

  zaID = (hid_t)ZaID;

  status = HE5_ZAdefcomp(zaID, compcode, compparm);
  if(status == FAIL)
    {
      sprintf(errbuf,"Error calling ZAdefcomp() from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAdefcompF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret = (int)status;
  return(ret);

}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefchunkF                                                  |
|                                                                             |
|  DESCRIPTION: FORTRAN wrapper for ZAdefchunk                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  ndims          int                 rank of a dataset                       |
|  *dim           long                pointer to the array containing sizes   |
|                                     of each dimension of a chunk            |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date         Programmer    Description                                    |
|  =========   ============  ==============================================   |
|  Jul 02       S.ZHAO       Original development                             |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAdefchunkF(int ZaID, int ndims, long *dim)
{
  int		ret       = FAIL;           /* return status        */
  int		j = 0;                      /* Loop index           */

  herr_t	status    = FAIL;           /* return status        */

  hid_t         zaID      = FAIL;           /* HDF5 type za ID      */

  hsize_t	*tempdims = (hsize_t *)NULL;/* Temporary variable   */
  
  char		*errbuf   = (char *)NULL;   /* error message buffer */

  /* Allocate memory for error message buffer */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAdefchunkF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Allocate space for C order dimension list */
  /* ----------------------------------------- */
  tempdims = (hsize_t *)calloc(ndims, sizeof(hsize_t));
  if(tempdims == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for tempdims.\n");
      H5Epush(__FILE__, "HE5_ZAdefchunkF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  
  /* Change from FORTRAN to C dimension order */
  /* ---------------------------------------- */
  for (j = 0; j < ndims; j++)
    tempdims[j] = (hsize_t)dim[ndims - 1 - j];


  zaID = (hid_t)ZaID;

  status = HE5_ZAdefchunk(zaID, ndims, tempdims);
  if(status == FAIL)
    {
      sprintf(errbuf,"Error calling ZAdefchunk() from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAdefchunkF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(tempdims);
      return(FAIL);
    }

  
  free(tempdims);
  free(errbuf);
  
  ret = (int)status;
  return(ret);
 
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAcompinfoF  (FORTRAN wrapper)                               |
|                                                                             |
|  DESCRIPTION: Retrieves compression information about a field               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int     None        Return status  (0) SUCCEED, (-1) FAIL   |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      const char*         Field name                              |
|  compcode       int*                Compression code                        |
|  compparm       int                 Compression parameters                  |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_ZAcompinfoF(int ZaID, char *fieldname, int *compcode, int compparm[])
{
  int             ret      = FAIL;        /* Return status variable       */

  herr_t          status   = FAIL;        /* Return status variable       */

  hid_t           zaID     = FAIL;        /* HDF5 type za ID              */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */


  zaID = (hid_t)ZaID;

  status = HE5_ZAcompinfo(zaID, fieldname, compcode, compparm);
  if (status == FAIL)
    {
      sprintf(errbuf , "Compression information for \"%s\" not found.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAcompinfoF", __LINE__, H5E_BTREE, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwritecharF                                                 |
|                                                                             |
|  DESCRIPTION: Writes data to a character string field (FORTRAN wrapper      |
|               around ZAwrite)                                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char                fieldname                               |
|  elemlen        int                 each element length in array of string  |
|  numelem        int                 number of elements in declared buffer   |
|                                     array                                   |
|  fortstart      long                start array                             |
|  fortstride     long                stride array                            |
|  fortedge       long                edge array                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  data           void                data buffer for write                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Feb 04    S.ZHAO       For an array of character string dataset.           |
|  Jun 04    S.Zhao       Added one more argument (numelem).                  |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAwritecharF(int ZaID, char *fieldname, int elemlen, int numelem, long fortstart[], long fortstride[], long fortedge[], void *data)
{
  int          ret      = SUCCEED;      /* int return status variable        */
  int          i;                       /* Loop index                        */
  int          rank     = 0;            /* Field rank                        */
 
  herr_t       status   = SUCCEED;      /* routine return status variable    */
 
  hid_t        ntype[1] = {FAIL};       /* Field datatype ID                 */
 
  hid_t        zaID     = FAIL;         /* HDF-EOS za ID                     */
 
  hssize_t     *start  = (hssize_t *)NULL;/* Pointer to start array (C order)*/
 
  hsize_t      dims[HE5_DTSETRANKMAX];   /* Field dimensions                 */
  hsize_t      *stride = (hsize_t *)NULL;/* Pointer to stride array (C order)*/
  hsize_t      *edge   = (hsize_t *)NULL;/* Pointer to edge array (C order)  */
 
  char         *errbuf = (char *)NULL;   /* error message buffer             */
  char         **strdatabuf = NULL;      /* string data buffer               */
  int          nelem = 0;               /* number of elements in array of str*/
  int          strsize = 0;              /* each str length in array of str  */
  char         *blankPtr = (char *)NULL; /* pointer to blank character       */
  char         *tmpString = (char *)NULL;/* pointer to temp string           */
  int          attr = 0;                 /* attribute value                  */
  hsize_t      count[1];                 /* number of attribute elements     */
 
  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf  = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAwritecharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  zaID = (hid_t)ZaID;
 
  /* Initialize dims[] array */
  /* ----------------------- */
  for ( i = 0; i < HE5_DTSETRANKMAX; i++)
    dims[ i ] = 0;
 
  /* Get field info  */
  /* --------------- */
  status = HE5_ZAinfo(zaID, fieldname, &rank, dims, ntype, NULL, NULL);
  if (status != FAIL)
    {
      start = (hssize_t *)calloc(rank, sizeof(hssize_t));
      if(start == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for start.\n");
	  H5Epush(__FILE__, "HE5_ZAwritecharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
        }
 
      stride = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(stride == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for stride.\n");
	  H5Epush(__FILE__, "HE5_ZAwritecharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(errbuf);
	  return(FAIL);
        }
 
      edge = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(edge == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for edge.\n");
	  H5Epush(__FILE__, "HE5_ZAwritecharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(errbuf);
	  return(FAIL);
        }
 
      /* Reverse order of dimensions (FORTRAN -> C) */
      /* ------------------------------------------ */
      for (i = 0; i < rank; i++)
        {
	  start[i]  = (hssize_t)fortstart[rank - 1 - i];
	  stride[i] = (hsize_t)fortstride[rank - 1 - i];
	  edge[i]   = (hsize_t)fortedge[rank - 1 - i];
        }
 
      strsize = elemlen;
      nelem = dims[0];

      tmpString = (char *)calloc(sizeof(char),strsize+1);
      if(tmpString == NULL)
	{
	  sprintf(errbuf,"Cannot allocate memory for tmpString.\n");
	  H5Epush(__FILE__, "HE5_ZAwritecharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  return(FAIL);
	}
 
      strdatabuf = (char **)malloc(nelem*sizeof(char *));
      for (i = 0; i<nelem; i++)
	{
	  strdatabuf[i] = NULL;
	  strdatabuf[i] = (char *)malloc((strsize+1)*sizeof(char));
	  strncpy(strdatabuf[i],(char *)data+(strsize*i), strsize);
	  strdatabuf[i][strsize] = '\0';
	  strcpy(tmpString,strdatabuf[i]);
 
	  /* Find the first non blank character from the end */
	  /* ----------------------------------------------- */
	  blankPtr = tmpString + strsize -1;
	  while (*blankPtr == ' ')
	    {
	      blankPtr--;
	    }
 
	  /* Turn it into a C string */
	  /* ----------------------- */
	  blankPtr++;
	  *blankPtr = '\0';
 
	  strcpy(strdatabuf[i], tmpString);
	}
 
      free(tmpString);

      count[0] = 1;
      attr = strsize;
      status = HE5_ZAwritelocattr(zaID, fieldname, "StringLengthAttribute", H5T_NATIVE_INT, count, &attr);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write attribute to the field \"%s\".",fieldname) ;
	  H5Epush(__FILE__, "HE5_ZAwritecharF", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  if (strdatabuf != NULL)
	    {
	      for (i = 0; i<nelem; i++)
		{
		  if (strdatabuf[i] != NULL)
		    {
		      free (strdatabuf[i]);
		      strdatabuf[i] = NULL;
		    }
		}
	      free (strdatabuf);
	      strdatabuf = NULL;
	    }
	  return(FAIL);
	}
 
      status = HE5_ZAwrrdfield(zaID, fieldname, "w", start, stride, edge, strdatabuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot write data to the field \"%s\".",fieldname) ;
	  H5Epush(__FILE__, "HE5_ZAwritecharF", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  if (strdatabuf != NULL)
	    {
	      for (i = 0; i<nelem; i++)
		{
		  if (strdatabuf[i] != NULL)
		    {
		      free (strdatabuf[i]);
		      strdatabuf[i] = NULL;
		    }
		}
	      free (strdatabuf);
	      strdatabuf = NULL;
	    }
	  return(FAIL);
	}
 
      free(start);
      free(stride);
      free(edge);
      free(errbuf);
      if (strdatabuf != NULL)
	{
	  for (i = 0; i<nelem; i++)
	    {
	      if (strdatabuf[i] != NULL)
		{
		  free (strdatabuf[i]);
		  strdatabuf[i] = NULL;
		}
	    }
	  free (strdatabuf);
	  strdatabuf = NULL;
	}

    }
  else
    {
      sprintf(errbuf, "Error calling HE5_ZAinfo() from FORTRAN wrapper. \n") ;
      H5Epush(__FILE__, "HE5_ZAwritecharF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
 
      free(errbuf);
    }
 
  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwriteF                                                     |
|                                                                             |
|  DESCRIPTION: Writes data to field (FORTRAN wrapper around ZAwrite)         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char                fieldname                               |
|  fortstart      long                start array                             |
|  fortstride     long                stride array                            |
|  fortedge       long                edge array                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  data           void                data buffer for write                   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Feb 05    S.Zhao       Replaced the "myedge" array by the "dims" array if  |
|                         it was larger than the dimension size.              |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAwriteF(int ZaID, char *fieldname, long fortstart[], long fortstride[], long fortedge[], void *data)
{
  int             ret      = SUCCEED;    /* int return status variable     */
  int             i;                     /* Loop index                     */
  int		  rank     = 0;          /* Field rank                     */

  herr_t          status   = SUCCEED;    /* routine return status variable */

  hid_t  	  ntype[1] = {FAIL};     /* Field datatype ID              */
 
  hid_t           zaID     = FAIL;       /* HDF-EOS za ID                  */
    
  hssize_t        *start  = (hssize_t *)NULL;/* Pointer to start array (C order)  */

  hsize_t         dims[HE5_DTSETRANKMAX]; /* Field dimensions              */
  hsize_t         *stride = (hsize_t *)NULL; /* Pointer to stride array (C order) */
  hsize_t         *edge   = (hsize_t *)NULL; /* Pointer to edge array (C order)   */
  
  char            *errbuf = (char *)NULL; /* error message buffer          */
  hsize_t         *myedge = (hsize_t *)NULL;
  

  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf  = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAwriteF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  zaID = (hid_t)ZaID;

  /* Initialize dims[] array */
  /* ----------------------- */
  for ( i = 0; i < HE5_DTSETRANKMAX; i++)
    dims[ i ] = 0;

  /* Get field info  */
  /* --------------- */
  status = HE5_ZAinfo(zaID, fieldname, &rank, dims, ntype, NULL, NULL);
  if (status != FAIL)
    {
      start = (hssize_t *)calloc(rank, sizeof(hssize_t));
      if(start == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for start.\n");
	  H5Epush(__FILE__, "HE5_ZAwriteF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
        }

      stride = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(stride == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for stride.\n");
	  H5Epush(__FILE__, "HE5_ZAwriteF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(errbuf);
	  return(FAIL);
        }

      edge = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(edge == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for edge.\n");
	  H5Epush(__FILE__, "HE5_ZAwriteF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(errbuf);
	  return(FAIL);
        }
 
      /* Reverse order of dimensions (FORTRAN -> C) */
      /* ------------------------------------------ */
      for (i = 0; i < rank; i++)
        {
	  start[i]  = (hssize_t)fortstart[rank - 1 - i];
	  stride[i] = (hsize_t)fortstride[rank - 1 - i];
	  edge[i]   = (hsize_t)fortedge[rank - 1 - i];
        }

      myedge = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(myedge == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for edge.\n");
	  H5Epush(__FILE__, "HE5_ZAwriteF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  return(FAIL);
        }

      for (i = 0; i < rank; i++)
        {
	  if(edge[i] > dims[i])
	    myedge[i] = dims[i];
	  else
	    myedge[i] = edge[i];
        }

      status = HE5_ZAwrrdfield(zaID, fieldname, "w", start, stride, myedge, data);
      if (status == FAIL)
        {
	  sprintf(errbuf, "Cannot write data to the field \"%s\".",fieldname) ;
	  H5Epush(__FILE__, "HE5_ZAwriteF", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(myedge);
	  free(errbuf);
	  return(FAIL);
        }
 
      free(start);
      free(stride);
      free(edge);
      free(myedge);
      free(errbuf);

    }
  else
    {
      sprintf(errbuf, "Error calling HE5_ZAinfo() from FORTRAN wrapper. \n") ;
      H5Epush(__FILE__, "HE5_ZAwriteF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      status = FAIL;
 
      free(errbuf);
    }
 
  ret = (int)status;
  return(ret);
}

 

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwrattr                                                     |
|                                                                             |
|  DESCRIPTION: FORTRAN wrapper to write/update attribute in a za.            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  attrname       char*               attribute name                          |
|  numtype        int                 attribute HDF numbertype                |
|  fortcount[]    long                number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Mar 04    S.Zhao       Modified for a character string attribute.          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAwrattr(int ZaID, char *attrname, int numtype, long fortcount[], void * datbuf)
{
  int       ret     = FAIL;/* int return status variable                 */
  int       rank    = 1;   /* Note: It is assumed that fortcout has just */
			   /* one element to agree with EHattr function  */
  int		i;             /* Loop index                             */

  hid_t     zaID    = FAIL;/* HDF-EOS za ID                              */
  hid_t     numbertype = FAIL;/* HDF5 data type ID                       */ 

  herr_t	status     = FAIL;/* routine return status variable          */
  
  hsize_t	*count     = (hsize_t *)NULL;/* Pointer to count array (C order)*/
    
  char      *errbuf = (char *)NULL;   /* error message buffer            */
  char      *tempbuf = (char *)NULL;  /* temp buffer                     */
  
  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      H5Epush(__FILE__, "HE5_ZAwrattr", __LINE__, H5E_DATATYPE, H5E_BADVALUE, "Error calling HE5_EHconvdatatype() from FORTRAN wrapper.");
      HE5_EHprint("Error: Error calling HE5_EHconvdatatype() from FORTRAN wrapper, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf  = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAwrattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate memory for "count" array */
  /* --------------------------------- */
  count = (hsize_t *)calloc(rank, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_ZAwrattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Reverse order of dimensions (FORTRAN -> C) */
  /* ------------------------------------------ */
  for (i = 0; i < rank; i++)
    {
      count[i] = (hsize_t)fortcount[rank - 1 - i];
    }
  
  zaID = (hid_t)ZaID;

  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR) || (numbertype == H5T_C_S1))
    {
      if (strlen(datbuf) < count[0])
	{
	  sprintf(errbuf,"Size of databuf is less than the number of attribute elements.\n");
	  H5Epush(__FILE__, "HE5_ZAwrattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
	  H5Epush(__FILE__, "HE5_ZAwrattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      strncpy(tempbuf, datbuf, count[0]);
      tempbuf[count[0]] = '\0';
 
      /* Call ZAwriteattr() */
      /* ------------------ */
      status = HE5_ZAwriteattr(zaID, attrname, numbertype, count, tempbuf);
      if( status == FAIL)
	{
          sprintf(errbuf,"Cannot write data to the attribute \"%s\".\n", attrname);
          H5Epush(__FILE__, "HE5_ZAwrattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
      /* Call ZAwriteattr() */
      /* ------------------ */
      status = HE5_ZAwriteattr(zaID, attrname, numbertype, count, datbuf);
      if( status == FAIL)
	{
	  sprintf(errbuf,"Cannot write data to the attribute \"%s\".\n", attrname);
	  H5Epush(__FILE__, "HE5_ZAwrattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
|  FUNCTION: HE5_ZAsetfill  (FORTRAN wrapper)                                 |
|                                                                             |
|  DESCRIPTION: Sets fill value for the specified field.                      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char*               field name                              |
|  fillval        void*               fill value                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAsetfill(int ZaID, char *fieldname, int numtype, void *fillval)
{
  int          ret        = FAIL;/* int return status variable  */ 

  herr_t       status     = FAIL;/* return status variable      */
 
  hid_t        numbertype = FAIL;/* HDF5 data tyope ID          */
  hid_t        zaID       = FAIL;/* HDF5 za ID                  */

  char         errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */

  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 type data type ID from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAsetfill", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  zaID = (hid_t)ZaID;
  status = HE5_ZAsetfillvalue(zaID, fieldname, numbertype, fillval);
  if (status == FAIL)
    {
      sprintf(errbuf,"Cannot set fill value for the field \"%s\".\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAsetfill", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret = (int)status;
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAgetfill   (FORTRAN wrapper)                                |
|                                                                             |
|  DESCRIPTION: Retrieves fill value for a specified field.                   |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char*               field name                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fillval        void*               fill value                              |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_ZAgetfill(int ZaID, char *fieldname, void *fillval)
{
  int             ret      = FAIL;/* routine return status variable   */

  herr_t          status   = FAIL;/* routine return status variable   */

  hid_t           zaID     = FAIL;/* HDF5 type za ID                  */
    
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */

  zaID = (hid_t)ZaID;

  status = HE5_ZAgetfillvalue(zaID, fieldname, fillval);
  if (status == FAIL)
    {
      sprintf(errbuf,"Cannot get fill value for the field \"%s\".\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAgetfill", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret = (int)status;
  return (ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAreadF                                                      |
|                                                                             |
|  DESCRIPTION: Reads data from field (FORTRAN wrapper around ZAread)         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char                fieldname                               |
|  fortstart      long                start array                             |
|  fortstride     long                stride array                            |
|  fortedge       long                edge array                              |
|  buffer         void                data buffer for read                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|     None                                                                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAreadF(int ZaID, char *fieldname, long fortstart[], long fortstride[], long fortedge[], void *buffer)
 
{
  int	      ret      = SUCCEED;       /* routine return status variable    */
  int	      i        = 0;	        /* Loop index                        */
  int	      rank     = 0;             /* Field rank                        */

  hid_t       ntype[1] = {FAIL};        /* Field data type ID                */

  hid_t       zaID     = FAIL;          /* za HDF5 type  ID                  */

  herr_t      status   = SUCCEED;       /* routine return status variable    */
    
  hssize_t    *start  = (hssize_t *)NULL;/* Pointer to start array (C order) */
  hsize_t     dims[HE5_DTSETRANKMAX];    /* Field dimensions                 */
  hsize_t     *stride = (hsize_t *)NULL; /* Pointer to stride array (C order)*/
  hsize_t     *edge   = (hsize_t *)NULL; /* Pointer to edge array (C order)  */
  
  char	      *errbuf = (char *)NULL;    /* error message buffer             */
  
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAreadF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  zaID = (hid_t)ZaID;
  
  /* Initialize dims[] array */
  /* ----------------------- */
  for ( i = 0; i < HE5_DTSETRANKMAX; i++)
    dims[ i ] = 0;

  /* Get field info */
  /* -------------- */
  status = HE5_ZAinfo(zaID, fieldname, &rank, dims, ntype, NULL,NULL);  
  if (status != FAIL)
    {
      start =  (hssize_t *)calloc(rank, sizeof(hssize_t));
      if(start == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for start.\n");
	  H5Epush(__FILE__, "HE5_ZAreadF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
        }
      stride = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(stride == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for stride.\n");
	  H5Epush(__FILE__, "HE5_ZAreadF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(errbuf);
	  return(FAIL);
        }
      edge = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(edge == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for edge.\n");
	  H5Epush(__FILE__, "HE5_ZAreadF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(errbuf);
	  return(FAIL);
        }
 
      /* Reverse order of dimensions (FORTRAN -> C) */
      /* ------------------------------------------ */
      for (i = 0; i < rank; i++)
        {
	  start[i]  = (hssize_t)fortstart[rank - 1 - i];
	  stride[i] = (hsize_t)fortstride[rank - 1 - i];
	  edge[i]   = (hsize_t)fortedge[rank - 1 - i];
        }
 

      /* Read data from the field */
      /* ------------------------ */
      status = HE5_ZAwrrdfield(zaID, fieldname, "r", start, stride, edge, buffer);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot read data from the \"%s\" field.",fieldname) ;
	  H5Epush(__FILE__, "HE5_ZAreadF", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  return(FAIL);
	}

      free(start);
      free(stride);
      free(edge);
      free(errbuf);

    }
  else
    {
      sprintf(errbuf, "Fieldname \"%s\" does not exist.",fieldname) ;
      H5Epush(__FILE__, "HE5_ZAreadF", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      status = FAIL;
    }
  

  ret = (int)status;
  return(ret);
}
 

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAreadcharF                                                  |
|                                                                             |
|  DESCRIPTION: Reads data from a character string field (FORTRAN wrapper     |
|               around ZAread)                                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char                fieldname                               |
|  elemlen        int                 each element length in array of string  |
|  numelem        int                 number of elements in declared buffer   |
|                                     array                                   |
|  fortstart      long                start array                             |
|  fortstride     long                stride array                            |
|  fortedge       long                edge array                              |
|  buffer         void                data buffer for read                    |
|                                                                             |
|  OUTPUTS:                                                                   |
|     None                                                                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Apr 04    S.Zhao       For an array of character string dataset.           |
|  Jun 04   S.Zhao        Added two more arguments (elemlen and numelem).     |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAreadcharF(int ZaID, char *fieldname, int elemlen, int numelem, long fortstart[], long fortstride[], long fortedge[], void *buffer)
 
{
  int         ret      = SUCCEED;     /* routine return status variable      */
  int         i        = 0;           /* Loop index                          */
  int         rank     = 0;           /* Field rank                          */
 
  hid_t       ntype[1] = {FAIL};      /* Field data type ID                  */
 
  hid_t       zaID     = FAIL;        /* za HDF5 type  ID                    */
 
  herr_t      status   = SUCCEED;     /* routine return status variable      */
 
  hssize_t    *start  = (hssize_t *)NULL;/* Pointer to start array (C order) */
  hsize_t     dims[HE5_DTSETRANKMAX];    /* Field dimensions                 */
  hsize_t     *stride = (hsize_t *)NULL; /* Pointer to stride array (C order)*/
  hsize_t     *edge   = (hsize_t *)NULL; /* Pointer to edge array (C order)  */
 
  char        *errbuf = (char *)NULL; /* error message buffer                */
  char        **strdatabuf = NULL;    /* string data buffer                  */
  int         stlen = 0;              /* whole string array length           */
  int         nelem = 0;              /* number of elements in array of str  */
  int         strsize = 0;            /* each str length in array of str     */
  int         attr = 0;               /* attribute value                     */
  int         j = 0;                  /* Loop index                          */

  int         num_elem_passed_out;
  int         numread;

 
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  zaID = (hid_t)ZaID;
 
  /* Initialize dims[] array */
  /* ----------------------- */
  for ( i = 0; i < HE5_DTSETRANKMAX; i++)
    dims[ i ] = 0;
 
  /* Get field info */
  /* -------------- */
  status = HE5_ZAinfo(zaID, fieldname, &rank, dims, ntype, NULL,NULL);
  if (status != FAIL)
    {
      start =  (hssize_t *)calloc(rank, sizeof(hssize_t));
      if(start == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for start.\n");
	  H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(errbuf);
	  return(FAIL);
        }
      stride = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(stride == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for stride.\n");
	  H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(errbuf);
	  return(FAIL);
        }
      edge = (hsize_t *)calloc(rank, sizeof(hsize_t));
      if(edge == NULL)
        {
	  sprintf(errbuf,"Cannot allocate memory for edge.\n");
	  H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(errbuf);
	  return(FAIL);
        }
 
      /* Reverse order of dimensions (FORTRAN -> C) */
      /* ------------------------------------------ */
      for (i = 0; i < rank; i++)
        {
	  start[i]  = (hssize_t)fortstart[rank - 1 - i];
	  stride[i] = (hsize_t)fortstride[rank - 1 - i];
	  edge[i]   = (hsize_t)fortedge[rank - 1 - i];
        }
 

      status = HE5_ZAreadlocattr(zaID, fieldname, "StringLengthAttribute", &attr);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot read attribute from the field \"%s\".",fieldname) ;
	  H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  return(FAIL);
	}
 
      if(elemlen < attr )
	{
	  sprintf(errbuf, "Element length passed in is not correct. Should be same as in declration");
	  H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  return(FAIL);
	}

      strsize = attr;
      stlen = dims[0] * strsize;
      nelem = stlen / strsize;
      strdatabuf = (char **)malloc(nelem*sizeof(char *));
      for (i = 0; i<nelem; i++)
	{
	  strdatabuf[i] = NULL;
	  strdatabuf[i] = (char *)malloc((strsize+1)*sizeof(char));
	}
 
      /* Read data from the field */
      /* ------------------------ */
      status = HE5_ZAwrrdfield(zaID, fieldname, "r", start, stride, edge, strdatabuf);
      if (status == FAIL)
	{
	  sprintf(errbuf, "Cannot read data from the \"%s\" field.",fieldname) ;
	  H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  if (strdatabuf != NULL)
	    {
	      for (i = 0; i<nelem; i++)
		{
		  if (strdatabuf[i] != NULL)
		    {
		      free (strdatabuf[i]);
		      strdatabuf[i] = NULL;
		    }
		}
	      free (strdatabuf);
	      strdatabuf = NULL;
	    }
	  return(FAIL);
	}
      if(numelem == 0)
	{
	  sprintf(errbuf, "Number of elements passed in cannot be zero.") ;
	  H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  if (strdatabuf != NULL)
	    {
	      for (i = 0; i<nelem; i++)
		{
		  if (strdatabuf[i] != NULL)
		    {
		      free (strdatabuf[i]);
		      strdatabuf[i] = NULL;
		    }
		}
	      free (strdatabuf);
	      strdatabuf = NULL;
	    }
	  return(FAIL);
	}
 
      if(edge != NULL)
	{
	  numread = edge[0];
	}
      else
	{
	  numread = (dims[0] - start[0])/stride[0];
	}
 
      if(numread <= 0)
	{
	  sprintf(errbuf, "Number of elements passed in cannot be zero.") ;
	  H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_DATASET, H5E_READERROR, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(start);
	  free(stride);
	  free(edge);
	  free(errbuf);
	  if (strdatabuf != NULL)
	    {
	      for (i = 0; i<nelem; i++)
		{
		  if (strdatabuf[i] != NULL)
		    {
		      free (strdatabuf[i]);
		      strdatabuf[i] = NULL;
		    }
		}
	      free (strdatabuf);
	      strdatabuf = NULL;
	    }
	  return(FAIL);
	}
      if(numread <=  numelem)
	{
	  num_elem_passed_out = numread;
	}
      else
	{
	  num_elem_passed_out = numelem;
	}
 
      stlen = strlen(strdatabuf[0]);
      strcpy((char *)buffer,strdatabuf[0]);
      for (j = stlen; j < elemlen; j++)
	{
	  strcat((char *)buffer," ");
	}
      for (i = 1; i < num_elem_passed_out; i++)
	{
	  strcat((char *)buffer,strdatabuf[i]);
	  stlen = strlen(strdatabuf[i]);
	  for (j = stlen; j < elemlen; j++)
	    {
	      strcat(buffer," ");
	    }
	}
 
      free(start);
      free(stride);
      free(edge);
      free(errbuf);
      if (strdatabuf != NULL)
	{
	  for (i = 0; i<nelem; i++)
	    {
	      if (strdatabuf[i] != NULL)
		{
		  free (strdatabuf[i]);
		  strdatabuf[i] = NULL;
		}
	    }
	  free (strdatabuf);
	  strdatabuf = NULL;
	}       
 
    }
  else
    {
      sprintf(errbuf, "Fieldname \"%s\" does not exist.",fieldname) ;
      H5Epush(__FILE__, "HE5_ZAreadcharF", __LINE__, H5E_DATASET, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      status = FAIL;
    }
 
 
  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZArdattr  (FORTRAN wrapper)                                  |
|                                                                             |
|  DESCRIPTION: Reads attribute from a za.                                    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
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
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZArdattr(int ZaID, char *attrname, void *datbuf)
{
  int         ret     = FAIL;/* routine return status variable    */

  herr_t      status  = FAIL;/* routine return status variable    */

  hid_t       zaID    = FAIL;/* HDF-EOS za ID                     */

  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */

  zaID = (hid_t)ZaID;

  /* Read the attribute buffer */
  /* ------------------------- */
  status = HE5_ZAreadattr(zaID, attrname, datbuf);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot read the attribute value. \n");
      H5Epush(__FILE__, "HE5_ZArdattr", __LINE__, H5E_ATTR, H5E_READERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  ret = (int)status;
  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZArdgattr     (FORTRAN wrapper)                              |
|                                                                             |
|  DESCRIPTION: Reads attribute from the "Data Fields" group in a za.         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
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
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZArdgattr(int ZaID, char *attrname, void *datbuf)
{
  int         ret             = FAIL;   /* (int) return status      */

  herr_t      status          = FAIL;   /* return status variable   */
  
  hid_t       zaID            = FAIL;   /* HDF5 type za ID          */
  
  char        errbuf[HE5_HDFE_ERRBUFSIZE];  /* Error message buffer */
  
  zaID = (hid_t)ZaID;

  status = HE5_ZAreadgrpattr(zaID,attrname,datbuf);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot read Attribute \"%s\" from the \"Data Fields\" group.\n", attrname);
      H5Epush(__FILE__, "HE5_ZArdgattr", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret = (int)status;
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwrlattr                                                    |
|                                                                             |
|  DESCRIPTION: FORTRAN wrapper to write/update local attribute in a za.      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char*               field with local attribute              |
|  attrname       char*               local attribute name                    |
|  numtype        int                 local attribute HDF numbertype          |
|  fortcount[]    long                number of group attribute elements      |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Mar 04    S.Zhao       Modified for a character string attribute.          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAwrlattr(int ZaID, char *fieldname, char *attrname, int numtype, long fortcount[], void *datbuf)
{
  int	       ret  = FAIL; /* routine return status variable               */
  int 	       rank = 1;    /* Note: It is assumed that fortcout has just   */
			      /* one element to agree with EHattr function  */
  int	       i = 0;

  herr_t       status = FAIL;           /* routine return status variable   */
   
  hsize_t      *count = (hsize_t *)NULL;/* Pointer to count array (C order) */
 
  hid_t        zaID   = FAIL;           /* za HDF5 type ID                  */ 
  hid_t        numbertype = FAIL;           /* HDF5 dta type ID             */
  
  char        *errbuf = (char *)NULL;   /* error message buffer             */
  char        *tempbuf = (char *)NULL;  /* temp buffer                      */


  /* Get HDF5 type data type ID */
  /* -------------------------- */
  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 data type ID.\n");
      H5Epush(__FILE__, "HE5_ZAwrlattr", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Allocate memory for error message buffers */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAwrlattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *)calloc(rank, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_ZAwrlattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  
  /* Reverse order of dimensions (FORTRAN -> C) */
  /* ------------------------------------------ */
  for (i = 0; i < rank; i++)
    {
      count[i] = (hsize_t)fortcount[rank - 1 - i];
    }
  
  zaID = (hid_t)ZaID;

  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR) || (numbertype == H5T_C_S1))
    {
      if (strlen(datbuf) < count[0])
	{
	  sprintf(errbuf,"Size of databuf is less than the number of local attribute elements.\n");
	  H5Epush(__FILE__, "HE5_ZAwrlattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
	  H5Epush(__FILE__, "HE5_ZAwrlattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      strncpy(tempbuf, datbuf, count[0]);
      tempbuf[count[0]] = '\0';
 
      /* Write local attribute buffer */
      /* ---------------------------- */
      status = HE5_ZAwritelocattr(zaID, fieldname, attrname, numbertype, count, tempbuf);
      if(status == FAIL)
	{
          sprintf(errbuf,"Cannot write local attribute value.\n");
          H5Epush(__FILE__, "HE5_ZAwrlattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
      status = HE5_ZAwritelocattr(zaID, fieldname, attrname, numbertype, count, datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write local attribute value.\n");
	  H5Epush(__FILE__, "HE5_ZAwrlattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
|  FUNCTION: HE5_ZArdlattr    (FORTRAN  wrapper)                              |
|                                                                             |
|  DESCRIPTION: Reads attribute associated with a specified field in a za.    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char                fieldname                               |
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
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZArdlattr(int ZaID, char *fieldname, char *attrname, void *datbuf)
{
  int         ret             =  FAIL;/* return status variable   */

  herr_t      status          =  FAIL;/* return status variable   */

  hid_t       zaID            =  FAIL;/* HDF5 type za  ID         */
  
  char        errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */

  zaID = (hid_t)ZaID;

  status = HE5_ZAreadlocattr(zaID,fieldname, attrname, datbuf);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot read Attribute \"%s\" associated wth the \"%s\" field.\n", attrname,fieldname);
      H5Epush(__FILE__, "HE5_ZArdlattr", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  ret  = (int)status;
  return(ret);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqdimsF                                                   |
|                                                                             |
|  DESCRIPTION: Returns dimension names and values defined in ZA structure    |
|               (FORTRAN Wrapper)                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nDim           long                Number of defined dimensions            |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  dimlist        char*               Dimension names (comma-separated)       |
|  dims           long                Dimension values                        |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAinqdimsF(int ZaID, char *dimlist, long dims[])
{
  int		i          = 0;   /* Loop index                    */
  
  hid_t         zaID       = FAIL;/* HDF5 type za ID               */ 
 
  long		nDim       = FAIL;/* (return) number of dimensions */
  long		strbufsize = 0;   /* String size variable          */
  
  hsize_t	*tdims     = (hsize_t *)NULL;/* Dimension variable */

  char      errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */


  zaID = (hid_t)ZaID;  
  
  nDim = HE5_ZAnentries(zaID, 0, &strbufsize);
  if(nDim == FAIL)
    {
      sprintf(errbuf,"Error calling ZAnentries() from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAinqdimsF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  tdims = (hsize_t *)calloc(nDim, sizeof(hsize_t));
  if(tdims == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for tdims.\n");
      H5Epush(__FILE__, "HE5_ZAinqdimsF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 

  nDim = HE5_ZAinqdims(zaID, dimlist, tdims);
  if(nDim == FAIL)
    {
      sprintf(errbuf,"Error calling HE5_ZAinqdims() from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAinqdimsF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(tdims);
      return(FAIL);
    }
  
  
  /* Convert data type */
  /* ----------------- */
  for(i = 0; i < nDim; i++)
    dims[i] = (long)tdims[i];
  

  free(tdims);
  
  return (nDim); 
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinquireF      (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION: Inquires about data fields in za                              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nflds          long                Number of data fields in za             |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fieldlist      char                Field names (comma-separated)           |
|  rank           int                 Array of ranks                          |
|  ntype          int                 data type class ID                      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinquireF(int ZaID, char *fieldlist, int rank[], int ntype[])
{
  long            nflds   = FAIL;               /* Number of Data fields  */
  long            i;                            /* Looop index            */
  
  hid_t           zaID    = FAIL;               /* HDF5 type za ID        */

  hid_t           *dtype  = (hid_t *)NULL;      /* Data type ID           */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];  /* Error message buffer   */


  zaID = (hid_t)ZaID;

  /* Call "HE5_ZAinquire" routine to get number of fields */
  /* ---------------------------------------------------- */
  nflds = HE5_ZAinquire(zaID, fieldlist, NULL, NULL);
  if (nflds == FAIL)
    {
      sprintf(errbuf, "Cannot get the number of fields in \"Data Fields\" group. \n");
      H5Epush(__FILE__, "HE5_ZAinquireF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  dtype = (hid_t *)calloc(nflds, sizeof(hid_t));
  if (dtype == NULL)
    {
      sprintf(errbuf, "Cannot allocate memory. \n");
      H5Epush(__FILE__, "HE5_ZAinquireF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }


  nflds = HE5_ZAinquire(zaID, fieldlist, rank, dtype);
  if (nflds == FAIL)
    {
      sprintf(errbuf, "Cannot get the number of fields in \"Data Fields\" group. \n");
      H5Epush(__FILE__, "HE5_ZAinquireF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(dtype);
      return(FAIL);
    }

  for (i = 0; i < nflds; i++)
    ntype[i] = (int)dtype[i];


  free(dtype);
  return(nflds);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdiminfoF                                                   |
|                                                                             |
|  DESCRIPTION: FORTRAN wrapper for ZAdiminfo to retrieve size of specified   |
|               dimension.                                                    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  size           long    None        Size of dimension                       |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int     None        ZA structure ID                         |
|  dimname        char*   None        Dimension name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAdiminfoF(int ZaID, char *dimname)
{
  long        size    = 0;                 /* dimension size  (return) */

  hid_t       zaID    = FAIL;              /* HDF5 data type ID        */  

  hsize_t     tsize   = 0;                 /* size variable            */
  
  char        errbuf[HE5_HDFE_ERRBUFSIZE]; /* Error message buffer     */

  zaID = (hid_t)ZaID;

  tsize = HE5_ZAdiminfo(zaID, dimname);
  if (tsize == 0)
    {
      sprintf(errbuf, "Error calling ZAdiminfo() from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_ZAdiminfoF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  size = (long)tsize;
  
  return(size);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinfoF                                                      |
|                                                                             |
|  DESCRIPTION: FORTRAN wrapper around ZAinfo                                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure id                         |
|  fieldname      char                name of field                           |
|                                                                             |
|  OUTPUTS:                                                                   |
|  rank           long                rank of field (# of dims)               |
|  dims           long                field dimensions                        |
|  ntype          int                 field number type                       |
|  fortdimlist    char                field dimension list (FORTRAN order)    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAinfoF(int ZaID, char *fieldname, int *rank, long dims[], int *ntype, char *fortdimlist, char *fortmaxdimlist)
{
  int		   ret      = SUCCEED;    /* routine return status variable */
  int		   j        = 0;          /* Loop Index                     */

  herr_t	   status   = SUCCEED;    /* routine return status variable */

  hid_t            zaID     = FAIL;       /* HDF5 type data type ID         */   
  
  hid_t            dtype[1] = {FAIL};     /* Data type ID                   */

  hsize_t	   tempdims[HE5_DTSETRANKMAX];/* Dimension sizes array      */       
  hsize_t 	   swap     = 0;              /* Temporary dimension size   */

  char		   *errbuf  = (char *)NULL;   /* error message buffer       */
 
  /* Dimension lists in C order */
  /* -------------------------- */
  char		   *dimlist    = (char *)NULL;
  char		   *maxdimlist = (char *)NULL;

  
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Allocate space for C order dimension list */
  /* ----------------------------------------- */
  maxdimlist = (char *)calloc(HE5_HDFE_UTLBUFSIZE, sizeof(char));
  dimlist    = (char *)calloc(HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if(dimlist == NULL || maxdimlist == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for input list.\n");
      H5Epush(__FILE__, "HE5_ZAinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  zaID = (hid_t)ZaID;

  /* Initialize tempdims[] array */
  /* --------------------------- */
  for ( j = 0; j < HE5_DTSETRANKMAX; j++)
    tempdims[ j ] = 0;

  /* Get field info */
  /* -------------- */
  status = HE5_ZAinfo(zaID, fieldname, rank, tempdims, dtype, dimlist, maxdimlist);
  if (status == SUCCEED)
    {
      for (j = 0; j < *rank / 2; j++)
        {
	  swap = tempdims[*rank - 1 - j];
	  tempdims[*rank - 1 - j] = tempdims[j];
	  tempdims[j] = swap;
        }
      /* change the datatype */
      /* ------------------- */
      for (j = 0; j < *rank; j++)
	dims[j] = (long)tempdims[j];

      /* Reverse order of dimensions entries in dimension list */
      /* ----------------------------------------------------- */
      if(fortdimlist != (char *)NULL)
	{
	  status = HE5_EHrevflds(dimlist, fortdimlist);
	  if(status == FAIL)
	    {
	      sprintf(errbuf,"Cannot reverse entries in dimension list.\n");
	      H5Epush(__FILE__, "HE5_ZAinfoF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      free(dimlist);
	      free(maxdimlist);
	      return(FAIL);
	    }
	}
      if(fortmaxdimlist != (char *)NULL)
	{
	  status = HE5_EHrevflds(maxdimlist, fortmaxdimlist);
	  if(status == FAIL)
	    {
	      sprintf(errbuf,"Cannot reverse entries in maximum dimension list.\n");
	      H5Epush(__FILE__, "HE5_ZAinfoF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
	      HE5_EHprint(errbuf, __FILE__, __LINE__);
	      free(errbuf);
	      free(dimlist);
	      free(maxdimlist);
	      return(FAIL);
	    }
		  
	}
    }

  *ntype = (int)dtype[0];

  free(maxdimlist);
  free(dimlist);
  free(errbuf);

  ret = (int)status;
  return(ret); 
}
  
 

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAchunkinfoF                                                 |
|                                                                             |
|  DESCRIPTION: FORTRAN wrapper for ZAchunkinfo                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char*               name of field                           |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ndims          int*                rank of a dataset                       |
|  dims           long                the array containing sizes              |
|                                     of each dimension of a chunk            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date         Programmer    Description                                    |
|   =========    ============  ============================================   |
|   Dec 03       S.Zhao        Original development                           |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAchunkinfoF(int ZaID, char *fieldname, int *ndims, long dims[])
{
  int           ret       = FAIL;           /* return status        */
  int           j         = 0;              /* Loop index           */
 
  herr_t        status    = FAIL;           /* return status        */
 
  hid_t         zaID      = FAIL;           /* HDF5 type za ID      */
 
  hsize_t       tempdims[HE5_DTSETRANKMAX]; /* Temporary dimension sizes array */
  hsize_t       swap      = 0;              /* Temporary dimension size   */
 
  char          *errbuf   = (char *)NULL;   /* error message buffer */
 
  /* Allocate memory for error message buffer */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAchunkinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Initialize tempdims[] array */
  /* --------------------------- */
  for ( j = 0; j < HE5_DTSETRANKMAX; j++)
    tempdims[ j ] = 0;
 
  zaID = (hid_t)ZaID;
 
  status = HE5_ZAchunkinfo(zaID, fieldname, ndims, tempdims);
  if(status == FAIL)
    {
      sprintf(errbuf,"Error calling ZAchunkinfo() from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAchunkinfoF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
 
  for (j = 0; j < *ndims / 2; j++)
    {
      swap = tempdims[*ndims - 1 - j];
      tempdims[*ndims - 1 - j] = tempdims[j];
      tempdims[j] = swap;
    }
 
  for (j = 0; j < *ndims; j++)
    dims[j] = (long)tempdims[j];
 
  free(errbuf);
 
  ret = (int)status;
  return(ret);
 
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwrgattr                                                    |
|                                                                             |
|  DESCRIPTION: FORTRAN wrapper to write/update group attribute in a za.      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  attrname       char*               group attribute name                    |
|  numtype        int                 group attribute HDF numbertype          |
|  fortcount[]    long                number of group attribute elements      |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                   It is assumed that fortcout has just one element to agree |
|                   with EHattr() call.                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|  Mar 04    S.Zhao       Modified for a character string attribute.          |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAwrgattr(int ZaID, char *attrname, int numtype, long fortcount[], void *datbuf)
{
  int          ret    = FAIL;       /* (int) return status variable      */
  int	       rank   = 1;          /* Rank variable                     */
  int	       i;                   /* Loop index                        */

  herr_t       status     = FAIL;   /* routine return status variable    */
  
  hid_t        zaID       = FAIL;   /* HDF5 type za ID                   */
  hid_t        numbertype = FAIL;   /* HDF5 data type ID                 */
             
  hsize_t      *count  = (hsize_t *)NULL;/* Pointer to count array (C order) */
  
  char         *errbuf = (char *)NULL;   /* error message buffer         */
  char         *tempbuf = (char *)NULL;  /* temp buffer                  */
  
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char *)calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAwrgattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  /* Get HDF5 data type ID */
  /* --------------------- */
  numbertype = HE5_EHconvdatatype(numtype);
  if(numbertype == FAIL)
    {
      sprintf(errbuf,"Cannot convert to HDF5 type data type ID.\n");
      H5Epush(__FILE__, "HE5_ZAwrgattr", __LINE__, H5E_DATATYPE, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }


  count = (hsize_t *)calloc(rank, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_ZAwrgattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Reverse order of dimensions (FORTRAN -> C) */
  /* ------------------------------------------ */
  for (i = 0; i < rank; i++)
    count[i] = (hsize_t)fortcount[rank - 1 - i];
  
  
  zaID = (hid_t)ZaID;

  if ((numbertype == HE5T_CHARSTRING) || (numbertype == H5T_NATIVE_CHAR) || (numbertype == H5T_C_S1))
    {
      if (strlen(datbuf) < count[0])
	{
	  sprintf(errbuf,"Size of databuf is less than the number of group attribute elements.\n");
	  H5Epush(__FILE__, "HE5_ZAwrgattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
	  H5Epush(__FILE__, "HE5_ZAwrgattr", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
	  HE5_EHprint(errbuf, __FILE__, __LINE__);
	  free(count);
	  free(errbuf);
	  return(FAIL);
	}
 
      strncpy(tempbuf, datbuf, count[0]);
      tempbuf[count[0]] = '\0';
 
      status = HE5_ZAwritegrpattr(zaID, attrname, numbertype, count, tempbuf);
      if(status == FAIL)
	{
          sprintf(errbuf,"Cannot write group attribute value.\n");
          H5Epush(__FILE__, "HE5_ZAwrgattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
      status = HE5_ZAwritegrpattr(zaID, attrname, numbertype, count, datbuf);
      if(status == FAIL)
	{
	  sprintf(errbuf,"Cannot write group attribute value.\n");
	  H5Epush(__FILE__, "HE5_ZAwrgattr", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
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
|  FUNCTION: HE5_ZAatinfo                                                     |
|                                                                             |
|  DESCRIPTION:  FORTRAN wrapper for ZAattrinfo                               |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  attrname       char*               attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  numbertype     int                 attribute HDF numbertype                |
|  fortcount      long                Number of attribute elements            |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
|                                                                             |
-----------------------------------------------------------------------------*/
int
HE5_ZAatinfo(int ZaID, char *attrname, int *numbertype, long *fortcount)
{
  int              ret      = FAIL;        /* (int) return status variable   */

  herr_t	   status   = FAIL;	   /* return status variable         */

  hid_t            *ntype   = (hid_t *)NULL;/* Data type ID                  */

  hid_t            zaID     = FAIL;         /* HDF5 type za ID               */

  hsize_t          *count   = (hsize_t *)NULL;/* Pointer to count (C order)  */
  
  char             *errbuf  = (char *)NULL;   /* error message buffer        */
  
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *) calloc(1, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_ZAatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  ntype = (hid_t *) calloc(1, sizeof(hid_t));
  if(ntype == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for ntype.\n");
      H5Epush(__FILE__, "HE5_ZAatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(count);
      return(FAIL);
    }


  zaID = (hid_t)ZaID;

  status = HE5_ZAattrinfo(zaID, attrname, ntype, count);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about attribute \"%s\".\n", attrname);
      H5Epush(__FILE__, "HE5_ZAatinfo", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
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
|  FUNCTION: HE5_ZAgatinfo                                                    |
|                                                                             |
|  DESCRIPTION:  FORTRAN wrapper for ZAgrpattrinfo                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  attrname       char*               attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  numbertype     int                 attribute HDF numbertype                |
|  fortcount      long                Number of attribute elements            |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
|                                                                             |
-----------------------------------------------------------------------------*/
int
HE5_ZAgatinfo(int ZaID, char *attrname, int *numbertype, long *fortcount)
{
  int	   	  ret     = FAIL;	    /* (int) return status variable   */

  herr_t   	  status  = FAIL;           /* routine return status variable */

  hid_t       *ntype  = (hid_t *)NULL;  /* Data type ID                   */

  hid_t       zaID    = FAIL;           /* HDF5 type za ID                */

  hsize_t     *count  = (hsize_t *)NULL;/* Pointer to count  (C order)    */
  
  char        *errbuf = (char *)NULL;   /* error message buffer           */
  
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAgatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *)calloc(1, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_ZAgatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  ntype = (hid_t *)calloc(1, sizeof(hid_t));
  if( ntype == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for ntype.\n");
      H5Epush(__FILE__, "HE5_ZAgatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(count);
      return(FAIL);
    }


  zaID = (hid_t)ZaID;

  status = HE5_ZAgrpattrinfo(zaID, attrname, ntype, count);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about group attribute.\n");
      H5Epush(__FILE__, "HE5_ZAgatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
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
|  FUNCTION: HE5_ZAinqgattrs (FORTRAN wrapper)                                |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list in "Data Fields" group                        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in "Data Fields"   |
|                                       group.                                |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in "Data Fields" group  |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqgattrs(int ZaID, char *attrnames, long *strbufsize)
{
  long            nattr   = FAIL;         /* Number of attributes (return)  */

  hid_t           zaID    = FAIL;         /* HDF5 type za ID                */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer       */

  zaID = (hid_t)ZaID;
  
  nattr = HE5_ZAinqgrpattrs(zaID, attrnames, strbufsize);
  if ( nattr < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the attributes. \n");
      H5Epush(__FILE__, "HE5_ZAinqgattrs", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  return(nattr);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqattrsF     (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION: Retrieves information about attributes defined in ZA          |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in za struct       |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in za struct            |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long HE5_ZAinqattrsF(int ZaID, char *attrnames, long *strbufsize)
{
  long            nattr    = FAIL;  /* Number of attributes (return)  */

  hid_t           zaID     = FAIL;  /* HDF5 type za ID                */

  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer */


  zaID = (hid_t)ZaID;

  nattr = HE5_ZAinqattrs(zaID, attrnames, strbufsize);
  if ( nattr < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the attributes. \n");
      H5Epush(__FILE__, "HE5_ZAinqattrsF", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
  
  return(nattr);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAlatinfo                                                    |
|                                                                             |
|  DESCRIPTION:  FORTRAN wrapper for ZAlocattrinfo                            |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char*               field name name                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrname       char*               attribute name                          |
|  numbertype     int                 attribute HDF numbertype                |
|  fortcount      long                Number of attribute elements            |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
|                                                                             |
-----------------------------------------------------------------------------*/
int
HE5_ZAlatinfo(int ZaID, char *fieldname, char *attrname, int *numbertype, long *fortcount)
{
  int	   	  ret     = FAIL;	        /* (int) return status variable   */

  herr_t   	  status  = FAIL;           /* routine return status variable */

  hid_t       *ntype  = (hid_t *)NULL;  /* Data type ID                */

  hid_t       zaID    = FAIL;           /* HDF5 type za ID             */

  hsize_t     *count  = (hsize_t *)NULL;/* Pointer to count  (C order)    */
  
  char        *errbuf = (char *)NULL;   /* error message buffer           */

  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAlatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  count = (hsize_t *)calloc(1, sizeof(hsize_t));
  if( count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for count.\n");
      H5Epush(__FILE__, "HE5_ZAlatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  ntype = (hid_t *)calloc(1, sizeof(hid_t));
  if( ntype == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for ntype.\n");
      H5Epush(__FILE__, "HE5_ZAlatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(count);
      return(FAIL);
    }

  zaID = (hid_t)ZaID;
  
  status = HE5_ZAlocattrinfo(zaID, fieldname, attrname, ntype, count);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about local attribute.\n");
      H5Epush(__FILE__, "HE5_ZAlatinfo", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
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
|  FUNCTION: HE5_ZAinqlattrs     (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list associated with a specified field in the      |
|                "Data Fields" group.                                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                Number of attributes in "Data Fields"   |
|                                       group.                                |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char                field name                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  attrnames      char                Attribute names in "Data Fields" group  |
|                                     (Comma-separated list)                  |
|  strbufsize     long                Attributes name list string length      |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqlattrs(int ZaID, char *fieldname, char *attrnames, long *strbufsize)
{
  long            nattr       = FAIL;     /* Number of attributes   */

  hid_t           zaID        = FAIL;     /* HDF5 type za ID        */
    
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer   */  

  zaID = (hid_t)ZaID;

  nattr = HE5_ZAinqlocattrs(zaID, fieldname, attrnames, strbufsize);
  if ( nattr < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the attributes. \n");
      H5Epush(__FILE__, "HE5_ZAinqlattrs", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
 
  return (nattr);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAidtype                                                     |
|                                                                             |
|  DESCRIPTION: Inquires about data fields in za (FORTRAN Wrapper)            |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                  return status (0) SUCCEED, (-1) FAIL   |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                  ZA structure ID                        |
|  fieldname      char*                Field name                             |
|  attrname       char*                Attribute name                         |
|  fieldgroup     int                  Field group id                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  Type           int*                                                        |
|  Class          int*                                                        |
|  Order          int*                                                        |
|  size           long*                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_ZAidtype(int ZaID, char *fieldname, char *attrname, int fieldgroup, int *Type, int *Class, int *Order, long *size)
{
  int		   ret      = FAIL;               /* (int) status variable  */

  herr_t	   status   = FAIL;               /* return status variable */
  
  size_t	   *tsize   = (size_t *)NULL;     /* data size variable     */ 

  hid_t        *typeID  = (hid_t *)NULL;      /* HDF5 Data type ID      */
  hid_t        zaID     = FAIL;               /* HDF5 type za ID        */
 
  H5T_class_t  *classID = (H5T_class_t *)NULL;/*  Data type class ID    */
  H5T_order_t  *order   = (H5T_order_t *)NULL;/* Byte order of datatype */

  char         *errbuf  = (char *)NULL;       /* error message buffer   */

  
  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAidtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }
  
  typeID = (hid_t *)calloc(1, sizeof(hid_t));
  if(typeID == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for typeID.\n");
      H5Epush(__FILE__, "HE5_ZAidtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  tsize = (size_t *)calloc(1, sizeof(size_t));
  if(tsize == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for tsize.\n");
      H5Epush(__FILE__, "HE5_ZAidtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(typeID);
      return(FAIL);
    }

  classID = (H5T_class_t *)calloc(1, sizeof(H5T_class_t));
  if(classID == (H5T_class_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for classID.\n");
      H5Epush(__FILE__, "HE5_ZAidtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
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
      H5Epush(__FILE__, "HE5_ZAidtype", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(typeID);
      free(tsize);
      free(classID);
      return(FAIL);
    }


  zaID = (hid_t)ZaID;

  /* Get data type information */
  /* ------------------------- */
  status = HE5_ZAinqdatatype(zaID, fieldname, attrname, fieldgroup, typeID, classID, order, tsize);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot get information about data type.\n");
      H5Epush(__FILE__, "HE5_ZAidtype", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
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
|  FUNCTION: HE5_ZAinqzaF   (FORTRAN wrapper)                                 |
|                                                                             |
|  DESCRIPTION: Returns number and names of za structures in file             |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nza            long                Number of ZA structures in file         |
|                                                                             |
|  INPUTS:                                                                    |
|  filename       char                HDF-EOS filename                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  zalist         char                List of za names (comma-separated)      |
|  strbufsize     long                Length of zalist                        |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqzaF(char *filename, char *zalist, long *strbufsize)
{
  long       nza     = FAIL;/* Number of za structures in file */

  char       errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer     */  

  nza = HE5_ZAinqza(filename, zalist, strbufsize);
  if ( nza < 0 )
    {
      sprintf(errbuf,"Cannot get information about za.");
      H5Epush(__FILE__, "HE5_ZAinqzaF", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }								 

  return(nza);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAnentriesF    (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION: Returns number of entries and string buffer size              |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nEntries       long                Number of entries                       |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  entrycode      int                 Entry code                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  strbufsize     long                Length of comma-separated list          |
|                                     (Does not include null-terminator       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAnentriesF(int ZaID, int entrycode, long *strbufsize)
{
  long            nEntries   = FAIL;       /* Number of entries         */

  hid_t           zaID       = FAIL;       /* HDF5 type za ID           */

  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* Buffer for error message  */
  

  zaID = (hid_t)ZaID;

  nEntries = HE5_ZAnentries(zaID, entrycode, strbufsize);
  if (nEntries < 0)
    {
      sprintf(errbuf,"Failed to get number of entries in a za.");       
      H5Epush(__FILE__, "HE5_ZAnentriesF", __LINE__, H5E_ARGS, H5E_BADVALUE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  return(nEntries);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAsetaliasF    (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION:  Defines alias for a specified field name                     |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  ret            int                      Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|   ZaID          int                      ZA structure ID                    |
|   fieldname     char                     Original field name                |
|                                                                             |
|  OUTPUTS:                                                                   |
|   fortaliaslist char                     Comma separated list of aliases for|
|                                            the field name                   |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZAsetaliasF(int ZaID, char *fieldname, char *fortaliaslist)
{
  int      ret        = FAIL;/* (int)Return status variable */

  herr_t   status     = FAIL;/* return status variable      */

  hid_t    zaID       = FAIL;/* HDF5 type za ID             */

  char     *aliaslist = NULL;/* C-order list of aliases     */
  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */

  zaID = (hid_t)ZaID;

  /* Allocate space for C order alias list */
  /* ------------------------------------- */
  aliaslist = (char *)calloc(strlen(fortaliaslist) + 1, sizeof(char));
  if(aliaslist == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for aliaslist.\n");
      H5Epush(__FILE__, "HE5_ZAsetaliasF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
  
  /* Reverse entries in alias list (FORTRAN -> C) */
  /* -------------------------------------------- */
  status = HE5_EHrevflds(fortaliaslist, aliaslist);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot reverse entries in alias list.\n");
      H5Epush(__FILE__, "HE5_ZAsetaliasF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(aliaslist);
      return(FAIL);
    }



  /* Call ZAsetalias() */
  /* ----------------- */
  status = HE5_ZAsetalias(zaID, fieldname, aliaslist);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Error calling ZAsetalias() from FORTRAN wrapper. \n");
      H5Epush(__FILE__, "HE5_ZAsetaliasF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(aliaslist);
      return(FAIL);
    }

  free(aliaslist);

  ret = (int)status;
  return(ret);
}

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdropaliasF    (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION:  Removes the alias for a specified field name                 |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  ret            int                      Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    ZaID         int                      ZA structure ID                    |
|    aliasname    char                     Alias name to remove               |
|    fldgroup     int                      Field group flag                   |
|                                                                             |
|  OUTPUTS:                                                                   |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZAdropaliasF(int ZaID, int fldgroup, char *aliasname)
{  
  int      ret        = FAIL;/* (int)Return status variable */

  herr_t   status     = FAIL;/* return status variable      */

  hid_t    zaID       = FAIL;/* HDF5 type za ID             */

  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */

  zaID = (hid_t)ZaID;
  
  status = HE5_ZAdropalias(zaID, fldgroup, aliasname);
  if(status == FAIL)
    {
      sprintf(errbuf,"Error calling ZAdropalias() from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAdropaliasF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  ret = (int)status;

  return(ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqfldaliasF (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of fields & aliases and string length of|
|                fields & aliases list in "Data Fields" group                 |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nfldalias      long                Number of fields & aliases in "Data     |
|                                     Fields"  group.                         |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 za structure ID                         |
|                                                                             |
|  OUTPUTS:                                                                   |
|  fldalias       char*               Fields & aliases names in "Data Fields" |
|                                     group (Comma-separated list)            |
|  strbufsize     long*               Fields & aliases name list string length|
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Mar 03    S.Zhao       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAinqfldaliasF(int ZaID, char *fldalias, long *strbufsize)
{
  long            nfldalias   = FAIL;     /* Number of fields & aliases (return)  */
 
  hid_t           zaID = FAIL;            /* HDF5 type za ID                */
 
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer       */
 
  zaID = (hid_t)ZaID;
 
  nfldalias = HE5_ZAinqfldalias(zaID, fldalias, strbufsize);
  if ( nfldalias < 0 )
    {
      sprintf(errbuf, "Cannot retrieve the fields & aliases. \n");
      H5Epush(__FILE__, "HE5_ZAinqfldaliasF", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
 
  return(nfldalias);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAaliasinfoF    (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION:  Retrieves information about field aliases                    |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  ret            int                      Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    ZaID         int                      ZA structure ID                    |
|    aliasname    char                     alias name                         |
|    fldgroup     int                      Field group flag                   |
|                                                                             |
|  OUTPUTS:                                                                   |
|    length       int                      Buffer size                        |
|    buffer       char                     Buffer with original field name    |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZAaliasinfoF(int ZaID, int fldgroup, char *aliasname, int *length, char *buffer)
{
  int           ret        = FAIL;/* (int) return status variable          */
      
  herr_t        status     = FAIL;/* return status variable                */

  hid_t         zaID       = FAIL;/* HDF5 type za ID                       */

  char          errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer        */

  zaID = (hid_t)ZaID;

  status = HE5_ZAaliasinfo(zaID, fldgroup, aliasname, length, buffer);
  if(status == FAIL)
    {
      sprintf(errbuf,"Error calling ZAaliasinfo() from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAaliasinfoF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  ret = (int)status;

  return(ret);  

}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAfldrenameF   (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION:  Changes the field name                                       |
|                                                                             |
|                                                                             |
|  Return Value    Type        Units       Description                        |
|  ============   ===========  =========   =================================  |
|  ret            int                      Return status                      |
|                                                                             |
|  INPUTS:                                                                    |
|    ZaID         int                      ZA structure ID                    |
|    oldfieldname char                     Original field name                |
|                                                                             |
|  OUTPUTS:                                                                   |
|    newfieldname char                     New field name                     |
|                                                                             |
|  NOTES:                                                                     |
|    None                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  =======  ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZAfldrenameF(int ZaID, char *oldfieldname, char *newfieldname)
{
  int      ret        = FAIL;/* (int)Return status variable */

  herr_t   status     = FAIL;/* return status variable      */

  hid_t    zaID       = FAIL;/* HDF5 type za ID             */

  char     errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer  */

  zaID = (hid_t)ZaID;
  
  status = HE5_ZAfldrename(zaID, oldfieldname, newfieldname);
  if(status == FAIL)
    {
      sprintf(errbuf,"Error calling ZAfldrename() from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAfldrenameF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  ret = (int)status;

  return(ret);  
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefcomchunkF    (FORTRAN wrapper)                          |
|                                                                             |
|  DESCRIPTION: Defines compression type and parameters and sets chunking     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int     None        return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  compcode       int                 compression code                        |
|  compparm       int                 compression parameters                  |
|  rank           int                 rank of a dataset                       |
|  *dim           long                pointer to the array containing sizes   |
|                                     of each dimension of a chunk            |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int   
HE5_ZAdefcomchunkF(int ZaID, int compcode, int *compparm, int rank, long *dim)
{
  int		ret       = FAIL;           /* return status        */
  int		j         = 0;              /* Loop index           */

  herr_t	status    = FAIL;           /* return status        */

  hsize_t	*tempdims = (hsize_t *)NULL;/* Temporary variable   */

  hid_t         zaID      = FAIL;           /* HDF5 type za ID      */
  
  char		*errbuf   = (char *)NULL;   /* error message buffer */
    
  /* Allocate memory for error message buffer */
  /* ---------------------------------------- */
  errbuf  = (char * )calloc( HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAdefcomchunkF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate space for C order dimension list */
  /* ----------------------------------------- */
  tempdims = (hsize_t *)calloc(rank, sizeof(hsize_t));
  if(tempdims == (hsize_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for tempdims.\n");
      H5Epush(__FILE__, "HE5_ZAdefcomchunkF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }
  
  /* Change from FORTRAN to C dimension order */
  /* ---------------------------------------- */
  for (j = 0; j < rank; j++)
    tempdims[j] = (hsize_t)dim[ rank - 1 - j ];

  zaID = (hid_t)ZaID;

  /* Call HE5_ZAdefcomchunk() */
  /* ------------------------ */
  status = HE5_ZAdefcomchunk(zaID, compcode, compparm, rank, tempdims);
  if (status == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_ZAdefcomchunk from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAdefcomchunkF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(tempdims);
      return(FAIL);
    }

  free(tempdims);
  free(errbuf);
    
  ret = (int)status;
  return(ret);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAsetextdataF      (FORTRAN wrapper)                         |
|                                                                             |
|  DESCRIPTION: Sets external data files.                                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  ret            int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fortfilelist   char*               list of external files (FORTRAN order)  |
|  offset         long                array of offsets (in bytes) from the    |
|                                     beginning of the file to the location   |
|                                     in the file where the data starts       |
|  size           long                array of sizes (in bytes) reserved in   |
|                                     the file for the data.                  |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Aug 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAsetextdataF(int ZaID, char *fortfilelist, long offset[], long size[])
{
  int             ret          = FAIL;           /* return status variable */
 
  herr_t          status       = FAIL;           /* Return status variable */
  hid_t           zaID         = FAIL;           /* ZA HDF5 type ID        */
 
  off_t           *off         = (off_t *)NULL;  /* Array of data offsets  */
 
  long            nentries     = 0;              /* Number of list entries */
  long            i;                             /* Loop index             */
 
  hsize_t         *sz          = (hsize_t *)NULL;/* Array of data sizes    */
 
  char            *filelist    = (char *)NULL;   /* Files list (C order)   */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];   /* Error message buffer   */
 
  /* Allocate space for C order file list */
  /* ------------------------------------ */
  filelist = (char *)calloc(HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if(filelist == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for input list.\n");
      H5Epush(__FILE__, "HE5_ZAsetextdataF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Reverse entries in input list */
  /* ----------------------------- */
  status = HE5_EHrevflds(fortfilelist, filelist);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot reverse entries in file list.\n");
      H5Epush(__FILE__, "HE5_ZAsetextdataF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      return(FAIL);
    }
 
 
  /* Parse the list of entries to get number of files */
  /* ------------------------------------------------ */
  nentries = HE5_EHparsestr(filelist, ',', NULL, NULL);
  if(nentries <= 0)
    {
      sprintf(errbuf,"Cannot parse the input list/input list is empty.\n");
      H5Epush(__FILE__, "HE5_ZAsetextdataF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      return(FAIL);
    }
 
 
  /* Allocate space for offset array */
  /* ------------------------------- */
  off = (off_t *)calloc(nentries, sizeof(off_t));
  if(off == (off_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for input offset array.\n");
      H5Epush(__FILE__, "HE5_ZAsetextdataF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      return(FAIL);
    }
 
  /* Allocate space for size array */
  /* ----------------------------- */
  sz = (hsize_t *)calloc(nentries, sizeof(hsize_t));
  if(sz == (hsize_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for input size array.\n");
      H5Epush(__FILE__, "HE5_ZAsetextdataF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      free(off);
      return(FAIL);
    }
 
  /* Copy the elements of input arrays */
  /* --------------------------------- */
  for (i = 0; i < nentries; i++)
    {
      off[ nentries - 1 - i ] = (off_t)offset[ i ];
      sz[  nentries - 1 - i ]  = (hsize_t)size[ i ];
    }
 
 
  zaID = (hid_t)ZaID;
 
  /* Call the C function */
  /* ------------------- */
  status = HE5_ZAsetextdata(zaID, filelist, off, sz);
  if (status == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_ZAsetextdata from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAsetextdataF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      free(off);
      free(sz);
      return(FAIL);
    }
 
  if (filelist != NULL) free(filelist);
  if (off != NULL) free(off);
  if (sz != NULL) free(sz);
 
  ret = (int)status;
  return(ret);
}
 
 
 
/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAgetextdataF       (FORTRAN wrapper)                        |
|                                                                             |
|  DESCRIPTION: Gets external data files information.                         |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nfiles         int                 number of external files  SUCCEED,      |
|                                               (-1) FAIL                     |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char*               External field name                     |
|                                                                             |
|  OUTPUTS:                                                                   |
|  namelength     long                Length of each name entry               |
|  fortfilelist   char*               List of file names (FORTRAN order)      |
|  offset         long                array of offsets (in bytes) from the    |
|                                     beginning of the file to the location   |
|                                     in the file where the data starts       |
|  size           long                array of sizes (in bytes) reserved in   |
|                                     the file for the data.                  |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Aug 02    S.ZHAO       Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAgetextdataF(int ZaID, char *fieldname, long namelength, char *fortfilelist, long offset[], long size[])
{
  int             nfiles       = FAIL;           /* number of files (return) */
  int             i;                             /* Loop index               */
 
  herr_t          status       = FAIL;           /* Return status variable   */
  hid_t           zaID         = FAIL;           /* ZA HDF5 type ID          */
 
  off_t           *off         = (off_t *)NULL;  /* Array of data offsets    */
 
  hsize_t         *sz          = (hsize_t *)NULL;/* Array of sizes           */
 
  size_t          nmlen        = 0;              /* Each entry string length */
 
  char            *filelist    = (char *)NULL;   /* Files list (C order)     */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];   /* Error message buffer     */
 
  /* Allocate space for C order output file list */
  /* ------------------------------------------- */
  filelist = (char *)calloc(HE5_HDFE_UTLBUFSIZE, sizeof(char));
  if(filelist == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for output list.\n");
      H5Epush(__FILE__, "HE5_ZAgetextdataF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }
 
  /* Allocate space for offset array */
  /* ------------------------------- */
  off = (off_t *)calloc(HE5_FLDNUMBERMAX , sizeof(off_t));
  if(off == (off_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for input offset array.\n");
      H5Epush(__FILE__, "HE5_ZAgetextdataF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      return(FAIL);
    }
 
  /* Allocate space for size array */
  /* ----------------------------- */
  sz = (hsize_t *)calloc(HE5_FLDNUMBERMAX, sizeof(hsize_t));
  if(sz == (hsize_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for input size array.\n");
      H5Epush(__FILE__, "HE5_ZAgetextdataF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      free(off);
      return(FAIL);
    }
 
  zaID = (hid_t)ZaID;
 
  nmlen = (size_t)namelength;
 
  /* Call the C function */
  /* ------------------- */
  nfiles = HE5_ZAgetextdata(zaID, fieldname, nmlen, filelist, off, sz);
  if (nfiles == FAIL)
    {
      sprintf(errbuf, "Error calling HE5_ZAgetextdata from FORTRAN wrapper.\n");
      H5Epush(__FILE__, "HE5_ZAgetextdataF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      free(off);
      free(sz);
      return(FAIL);
    }
 
  /* Reverse entries in output list */
  /* ----------------------------- */
  status = HE5_EHrevflds(filelist, fortfilelist);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot reverse entries in output file list.\n");
      H5Epush(__FILE__, "HE5_ZAgetextdataF", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(filelist);
      free(off);
      free(sz);
      return(FAIL);
    }
 
  /* Populate the output arrays */
  /* -------------------------- */
  for (i = 0; i < nfiles; i++)
    {
      offset[ nfiles - 1 - i ] = (long)off[ i ];
      size[ nfiles - 1 - i ]   = (long)sz[ i ];
    }
 
  if (filelist != NULL) free(filelist);
  if (off != NULL) free(off);
  if (sz != NULL) free(sz);
 
 
  return(nfiles);
}
 

/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAwrdmeta                                                    |
|                                                                             |
|  DESCRIPTION: Defines structural metadata for pre-existing data             |
|               field within ZA structure (FORTRAN)                           |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 ZA structure ID                         |
|  fieldname      char                fieldname                               |
|  fortdimlist    char                Dimension list (comma-separated list)   |
|                                         FORTRAN dimesion order              |
|  mvalue         int                 field type                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jun 05    S. Zhao      Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAwrdmeta(int ZaID, char *fieldname, char *fortdimlist, int mvalue)
{
  int       ret        = FAIL;   /* (int) return status variable   */
  herr_t    status     = FAIL;   /* routine return status variable */

  hid_t     zaID       = FAIL;   /* HDF5 type za ID                */
  hid_t     numbertype = FAIL;   /* HDF5 type data type ID         */

  char      *dimlist = (char *)NULL;   /* Dimension list (C order) */
  char      *errbuf  = (char *)NULL;   /* error message buffer     */

  zaID = (hid_t)ZaID;

  numbertype = HE5_EHconvdatatype(mvalue);
  if(numbertype == FAIL)
    {
      H5Epush(__FILE__, "HE5_ZAwrdmeta", __LINE__, H5E_DATATYPE, H5E_BADVALUE, "Cannot convert datatype for FORTRAN wrapper.");
      HE5_EHprint("Error: Cannot convert datatype for FORTRAN wrapper, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate memory for error message buffers */
  /* ----------------------------------------- */
  errbuf  = (char * )calloc(HE5_HDFE_ERRBUFSIZE, sizeof(char));
  if(errbuf == NULL)
    {
      H5Epush(__FILE__, "HE5_ZAwrdmeta", __LINE__, H5E_RESOURCE, H5E_NOSPACE, "Cannot allocate memory for error buffer.");
      HE5_EHprint("Error: Cannot allocate memory for error buffer, occured", __FILE__, __LINE__);
      return(FAIL);
    }

  /* Allocate space for C order dimension list */
  /* ----------------------------------------- */
  dimlist = (char *) calloc(strlen(fortdimlist) + 1, sizeof(char));
  if(dimlist == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for dimlist.\n");
      H5Epush(__FILE__, "HE5_ZAwrdmeta", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      return(FAIL);
    }

  /* Reverse entries in dimension list (FORTRAN -> C) */
  /* ------------------------------------------------ */
  status = HE5_EHrevflds(fortdimlist, dimlist);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot reverse entries in dimension list.\n");
      H5Epush(__FILE__, "HE5_ZAwrdmeta", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(dimlist);
      return(FAIL);
    }

  /* Call Write Metadata routine */
  /* --------------------------- */
  status = HE5_ZAwritedatameta(zaID, fieldname, dimlist, numbertype);
  if(status == FAIL)
    {
      sprintf(errbuf,"Cannot write \"%s\" field metadata.\n", fieldname);
      H5Epush(__FILE__, "HE5_ZAwrdmeta", __LINE__, H5E_FUNC, H5E_CANTINIT, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(errbuf);
      free(dimlist);
      return(FAIL);
    }

  free(dimlist);
  free(errbuf);

  ret = (int)status;
  return (ret);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAgetaliaslistF (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of aliases and aliases list in "Data    |
|                Fields" group                                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nalias         long                number of aliases in "Data Fields"      |
|                                     group                                   |
|                                                                             |
|  INPUTS:                                                                    |
|  ZaID           int                 za structure ID                         |
|  fldgroup       int                 field group flag                        |
|                                                                             |
|  OUTPUTS:                                                                   |
|  aliaslist      char*               list of aliases  in "Data Fields" group |
|                                     (comma-separated list)                  |
|  strbufsize     long*               length of aliases list                  |
|                                                                             |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jul 05    S.Zhao       Original development                                |
|  Dec 05    T.Roegner    NCR 44092 - Added CYGWIN capability                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAgetaliaslistF(int ZaID, int fldgroup, char *aliaslist, long *strbufsize)
{
  long            nalias  = FAIL;         /* Number of aliases (return)     */
  hid_t           zaID    = FAIL;         /* HDF5 type za ID                */
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer       */


  zaID = (hid_t)ZaID;

  nalias = HE5_ZAgetaliaslist(zaID, fldgroup, aliaslist, strbufsize);
  if (nalias < 0)
    {
      sprintf(errbuf, "Cannot retrieve the aliases. \n");
      H5Epush(__FILE__, "HE5_ZAgetaliaslistF", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  return(nalias);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAsetdimscaleF (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION: Defines a dimension scale for a fields dimension in a ZA      |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           int                 ZA structure ID                         |
|  fieldname      char                field name                              |
|  dimname        char                dim name                                |
|  dim            long                Dimemsion value                         |
|  numbertype_in  int                 dim scale data type such as DFNT_INT32, |
|                                     DFNT_FLOAT64, DFNT_FLOAT32, etc.        |
|  data           void                data buffer for write                   |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|                None                                                         |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 10    Abe Taaheri  Original Programmer                                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAsetdimscaleF(int ZaID, char *fieldname, char *dimname,
		  long dim, int numbertype_in, void * data)
{
  herr_t          status;
  hid_t           zaID;
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer       */


  zaID = (hid_t)ZaID;

  status = HE5_ZAsetdimscale(zaID, fieldname, dimname,
			     (hsize_t) dim, (hid_t) numbertype_in, data);

  if (status < 0)
    {
      sprintf(errbuf, "Cannot set dimension scale %s for field %s. \n", dimname, fieldname);
      H5Epush(__FILE__, "HE5_ZAsetdimscaleF", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  return((int)status);

}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdefdimscaleF (FORTRAN wrapper)                             |
|                                                                             |
|  DESCRIPTION: Defines dimension scale for a dimension for all fields in a ZA|
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           int                 ZA structure ID                         |
|  dimname        char                dim name                                |
|  dim            long                Dimemsion value                         |
|  numbertype_in  int                 dim scale data type such as DFNT_INT32, |
|                                     DFNT_FLOAT64, DFNT_FLOAT32, etc.        |
|  data           void                data buffer for write                   |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|                None                                                         |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  Jan 14    Abe Taaheri  Original Programmer                                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAdefdimscaleF(int ZaID, char *dimname,
		  long dim, int numbertype_in, void * data)
{
  herr_t          status;
  hid_t           zaID;
  char            errbuf[HE5_HDFE_ERRBUFSIZE];/* Error message buffer       */


  zaID = (hid_t)ZaID;

  status = HE5_ZAdefdimscale(zaID, dimname,
			     (hsize_t) dim, (hid_t) numbertype_in, data);

  if (status < 0)
    {
      sprintf(errbuf, "Cannot set dimension scale %s for all fields in ZA. \n", dimname);
      H5Epush(__FILE__, "HE5_ZAdefdimscaleF", __LINE__, H5E_DATASET, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  return((int)status);

}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAgetdimscaleF  (FORTRAN wrapper)                            |
|                                                                             |
|  DESCRIPTION: Get dimension scale for a dimension of a field in a ZA        |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return databuffsize  SUCCEED, (-1) FAIL |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           int                 ZA structure ID                         |
|  fieldname      char                field name                              |
|  dimname        char                dim name                                |
|                                                                             |
|                                                                             |
|  OUTPUTS:                                                                   |
|  dimsize        long                dimension size                          |
|  ntype          int                 number type                             |
|  databuff       void                data buffer for read                    |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 10    Abe Taaheri  Original Programmer                                 |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long
HE5_ZAgetdimscaleF(int ZaID, char *fieldname, char *dimname, long *dimsize, 
		   int *ntype, void * databuff)
{
  hsize_t         databuffsize = (hsize_t)FAIL;/* Size of buffer            */
  hid_t           zaID         = FAIL;         /* HDF5 type za ID           */
  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* Buffer for error message  */
  hsize_t         ds;
  hid_t           numtype;

  zaID = (hid_t)ZaID;

  databuffsize = HE5_ZAgetdimscale(zaID, fieldname, dimname, &ds, &numtype,
				   databuff);
  *dimsize = (long) ds;
  *ntype = (int) numtype;

  if ( databuffsize < 0 )
    {
      sprintf(errbuf,"Cannot get dimension scale %s for field %s. \n", dimname, fieldname);
      H5Epush(__FILE__, "HE5_ZAgetdimscaleF", __LINE__, H5E_OHDR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
						 
  return((long)databuffsize);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAreaddscaleattrF (FORTRAN wrapper)                          |
|                                                                             |
|  DESCRIPTION: Reads attribute associated with a dimension scale field       |
|               from a ZA.                                                    |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           int     None        HDF-EOS type ZA  ID                     |
|  fieldname      char                field name                              |
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
|  Jun 10   Abe Taaheri   Original development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int
HE5_ZAreaddscaleattrF(int ZaID, char *fieldname, char *attrname, void *datbuf)
{
  herr_t          status         = FAIL;       /* return status             */
  hid_t           zaID           = FAIL;       /* HDF5 type za ID           */
  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* Buffer for error message  */
  

  zaID = (hid_t)ZaID;

  status = HE5_ZAreaddscaleattr(zaID, fieldname, attrname, datbuf);
  if ( status < 0 )
    {
      sprintf(errbuf,"Cannot read attribute %s for dimension scale %s. \n", attrname, fieldname);
      H5Epush(__FILE__, "HE5_ZAreaddscaleattrF", __LINE__, H5E_IO, H5E_SEEKERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
						 
  return((int)status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION:  HE5_ZAwritedscaleattrF (FORTRAN wrapper)                        |
|                                                                             |
|  DESCRIPTION: Writes/updates attribute associated with a dimension scale    |
|               field in a ZA.                                                |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           int                 ZA structure ID                         |
|  fieldname      char*               dimension scale SDS name                |
|  attrname       char*               attribute name                          |
|  numtype        int                 attribute datatype ID                   |
|  count[]        long                Number of attribute elements            |
|  datbuf         void                I/O buffer                              |
|                                                                             |
|  OUTPUTS:                                                                   |
|  None                                                                       |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 10   Abe Taaheri   Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int    
HE5_ZAwritedscaleattrF(int ZaID, char *fieldname, char *attrname, int numtype, long count[], void *datbuf)
{
  herr_t          status         = FAIL;       /* return status             */
  hid_t           zaID           = FAIL;       /* HDF5 type za ID           */
  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* Buffer for error message  */
  hsize_t         fort_count[HE5_FLDNUMBERMAX];

  zaID = (hid_t)ZaID;

  fort_count[0] = (hsize_t)count[0];
  status = HE5_ZAwritedscaleattr(zaID, fieldname, attrname, (hid_t)numtype, fort_count, datbuf);
  if ( status < 0 )
    {
      sprintf(errbuf,"Cannot write attribute %s for dimension scale %s. \n", attrname, fieldname);
      H5Epush(__FILE__, "HE5_ZAwritedscaleattrF", __LINE__, H5E_ATTR, H5E_WRITEERROR, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }
						 
  return((int)status);
}



/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAdscaleattrinfoF (FORTRAN wrapper)                          |
|                                                                             |
|  DESCRIPTION: Retrieves information about dimension scale attribute         |
|               (attribute associated with a specified dimension scale field) |
|                in a ZA.                                                     |
|                                                                             |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  status         int                 return status (0) SUCCEED, (-1) FAIL    |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           int       None      HDF-EOS type ZA  ID                     |
|  fieldname      char*               field name                              |
|  attrname       char*               attribute name                          |
|                                                                             |
|  OUTPUTS:                                                                   |
|  ntype          int                 attribute data type ID                  |
|  fortcount      long                Number of attribute elements            |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  May 10   Abe Taaheri   Original Development                                |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
int 
HE5_ZAdscaleattrinfoF(int ZaID, char *fieldname, char *attrname, int *ntype, long *fortcount)
{
  herr_t          status         = FAIL;       /* return status             */
  hid_t           zaID           = FAIL;       /* HDF5 type za ID           */
  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* Buffer for error message  */
  hsize_t         *count = (hsize_t *)NULL;    /* C-order count array       */
  hid_t           *dtype  = (hid_t *)NULL;     /* Data type class ID        */

  zaID = (hid_t)ZaID;

  dtype = (hid_t *)calloc(1, sizeof(hid_t));
  if(dtype == (hid_t *)NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for \"dtype\".\n");
      H5Epush(__FILE__, "HE5_ZAdscaleattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      return(FAIL);
    }

  count = (hsize_t *)calloc(1, sizeof(hsize_t));
  if(count == NULL)
    {
      sprintf(errbuf,"Cannot allocate memory for \"count\".\n");
      H5Epush(__FILE__, "HE5_ZAdscaleattrinfoF", __LINE__, H5E_RESOURCE, H5E_NOSPACE, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(dtype);
      return(FAIL);
    }

  count[0] = 0;

  status = HE5_ZAdscaleattrinfo(zaID, fieldname, attrname, dtype, count);
  if ( status == FAIL)
    {
      sprintf(errbuf, "Cannot retrieve attribute info for the %s dimension scale. \n", fieldname);
      H5Epush(__FILE__, "HE5_ZAdscaleattrinfoF", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
      free(count);
      free(dtype);
      return(FAIL);
    }
  
  *fortcount = (long)(*count);
  *ntype     = (int)(*dtype);

  free(count);
  free(dtype);
  return((int)status);
}


/*----------------------------------------------------------------------------|
|  BEGIN_PROLOG                                                               |
|                                                                             |
|  FUNCTION: HE5_ZAinqdscaleattrsF (FORTRAN wrapper)                          |
|                                                                             |
|  DESCRIPTION:  Retrieves the number of attributes and string length of      |
|                attribute list associated with a specified dimension scale   |
|                in a ZA.                                                     |
|                                                                             |
|  Return Value    Type     Units     Description                             |
|  ============   ======  =========   =====================================   |
|  nattr          long                number of attributes (-1 if fails)      |
|                                                                             |
|  INPUTS:                                                                    |
|  zaID           int     None        HDF-EOS type ZA  ID                     |
|  fieldname      char                dimension scale name                    |
|  attrnames      char                attribute name(s)                       |
|                                                                             |
|  OUTPUTS:                                                                   |
|  strbufsize     long                String length of attribute (in bytes)   |
|                                                                             |
|  NOTES:                                                                     |
|                                                                             |
|  None                                                                       |
|                                                                             |
|   Date     Programmer   Description                                         |
|  ======   ============  =================================================   |
|  June 10   Abe Taaheri  Original development.                               |
|                                                                             |
|  END_PROLOG                                                                 |
-----------------------------------------------------------------------------*/
long 
HE5_ZAinqdscaleattrsF(int ZaID, char *fieldname, char *attrnames, long *strbufsize)
{
  long            status         = FAIL;       /* return status             */
  hid_t           zaID           = FAIL;       /* HDF5 type za ID           */
  char            errbuf[HE5_HDFE_ERRBUFSIZE]; /* Buffer for error message  */
  

  zaID = (hid_t)ZaID;

  status = HE5_ZAinqdscaleattrs(zaID, fieldname, attrnames, strbufsize);
  if ( status == FAIL )
    {
      sprintf(errbuf, "Cannot find the attribute %s for dimension scale %s. \n",attrnames,fieldname);
      H5Epush(__FILE__, "HE5_ZAinqdscaleattrsF", __LINE__, H5E_ATTR, H5E_NOTFOUND, errbuf);
      HE5_EHprint(errbuf, __FILE__, __LINE__);
    }

  return (status);
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



/* File/ZA access routines */

FCALLSCFUN2(INT, HE5_ZAopenF, HE5_ZAOPEN, he5_zaopen, STRING, INT)
FCALLSCFUN2(INT, HE5_ZAcreateF, HE5_ZACREATE, he5_zacreate, INT, STRING)
FCALLSCFUN2(INT, HE5_ZAattachF, HE5_ZAATTACH, he5_zaattach, INT, STRING)
FCALLSCFUN1(INT, HE5_ZAdetachF, HE5_ZADETACH, he5_zadetach, INT)
FCALLSCFUN1(INT, HE5_ZAcloseF, HE5_ZACLOSE, he5_zaclose, INT)

/* Definition routines */

FCALLSCFUN3(INT, HE5_ZAdefdimF, HE5_ZADEFDIM, he5_zadefdim, INT, STRING, LONG)
FCALLSCFUN5(INT, HE5_ZAdefineF, HE5_ZADEFINE, he5_zadefine, INT, STRING, STRING, STRING, INT)
FCALLSCFUN3(INT, HE5_ZAdefcompF, HE5_ZADEFCOMP, he5_zadefcomp, INT, INT, PINT)
FCALLSCFUN3(INT, HE5_ZAdefchunkF, HE5_ZADEFCHUNK, he5_zadefchunk, INT, INT, PLONG)
FCALLSCFUN5(INT, HE5_ZAdefcomchunkF, HE5_ZADEFCOMCH, he5_zadefcomch, INT, INT, PINT, INT, PLONG)
FCALLSCFUN3(INT, HE5_ZAfldrenameF, HE5_ZAFLDRNM, he5_zafldrnm, INT, STRING, STRING)

/* I/O routines */

FCALLSCFUN6(INT, HE5_ZAwriteF, HE5_ZAWRITE, he5_zawrite, INT, STRING, LONGV, LONGV, LONGV,PVOID)
FCALLSCFUN8(INT, HE5_ZAwritecharF, HE5_ZAWRITECHAR, he5_zawritechar, INT, STRING, INT, INT, LONGV, LONGV, LONGV,PVOID)
FCALLSCFUN6(INT, HE5_ZAreadF, HE5_ZAREAD, he5_zaread, INT, STRING, LONGV, LONGV, LONGV, PVOID)
FCALLSCFUN8(INT, HE5_ZAreadcharF, HE5_ZAREADCHAR, he5_zareadchar, INT, STRING, INT, INT, LONGV, LONGV, LONGV, PVOID)
FCALLSCFUN5(INT, HE5_ZAwrattr, HE5_ZAWRATTR, he5_zawrattr,INT,STRING,INT,LONGV,PVOID)
FCALLSCFUN5(INT, HE5_ZAwrgattr, HE5_ZAWRGATTR, he5_zawrgattr, INT, STRING, INT, LONGV,PVOID)
FCALLSCFUN6(INT, HE5_ZAwrlattr, HE5_ZAWRLATTR, he5_zawrlattr, INT, STRING,STRING,INT,LONGV,PVOID)
FCALLSCFUN3(INT, HE5_ZArdattr, HE5_ZARDATTR, he5_zardattr, INT, STRING, PVOID)
FCALLSCFUN3(INT, HE5_ZArdgattr, HE5_ZARDGATTR, he5_zardgattr,INT,STRING,PVOID)
FCALLSCFUN4(INT, HE5_ZArdlattr, HE5_ZARDLATTR, he5_zardlattr,INT,STRING,STRING,PVOID)
FCALLSCFUN4(INT, HE5_ZAsetfill, HE5_ZASETFILL, he5_zasetfill, INT, STRING, INT, PVOID)
FCALLSCFUN3(INT, HE5_ZAgetfill, HE5_ZAGETFILL, he5_zagetfill, INT, STRING, PVOID)
FCALLSCFUN3(INT, HE5_ZAsetaliasF, HE5_ZASETALIAS, he5_zasetalias, INT, STRING, STRING)
FCALLSCFUN3(INT, HE5_ZAdropaliasF, HE5_ZADRPALIAS, he5_zadrpalias, INT, INT, STRING)
FCALLSCFUN4(INT, HE5_ZAwrdmeta, HE5_ZAWRDMETA, he5_zawrdmeta, INT, STRING, STRING, INT)


/* Inquiry routines */

FCALLSCFUN3(LONG, HE5_ZAinqzaF, HE5_ZAINQZA, he5_zainqza, STRING, PSTRING,PLONG)
FCALLSCFUN3(LONG, HE5_ZAnentriesF, HE5_ZANENTRIES, he5_zanentries,INT,INT,PLONG)
FCALLSCFUN2(LONG, HE5_ZAdiminfoF, HE5_ZANDIMINFO, he5_zadiminfo, INT, STRING)
FCALLSCFUN7(INT, HE5_ZAinfoF, HE5_ZAINFO, he5_zainfo,INT,STRING,PINT,LONGV,PINT,PSTRING,PSTRING)
FCALLSCFUN3(LONG, HE5_ZAinqdimsF, HE5_ZAINQDIMS, he5_zainqdims,INT,PSTRING,LONGV)
FCALLSCFUN4(LONG, HE5_ZAinquireF, HE5_ZAINQUIRE, he5_zainquire, INT,PSTRING, INTV, INTV)
FCALLSCFUN4(INT, HE5_ZAatinfo, HE5_ZAATTRINFO, he5_zaattrinfo, INT, STRING, PINT, PLONG)
FCALLSCFUN4(INT, HE5_ZAgatinfo, HE5_ZAGATTRINFO, he5_zagattrinfo, INT, STRING, PINT, PLONG)
FCALLSCFUN5(INT, HE5_ZAlatinfo, HE5_ZALATTRINFO, he5_zalattrinfo, INT, STRING, STRING, PINT, PLONG)
FCALLSCFUN4(INT, HE5_ZAcompinfoF, HE5_ZACOMPINFO, he5_zacompinfo, INT, STRING,PINT, INTV)
FCALLSCFUN3(LONG, HE5_ZAinqattrsF, HE5_ZAINQATTRS, he5_zainqattrs,INT,PSTRING,PLONG)
FCALLSCFUN4(LONG, HE5_ZAinqlattrs, HE5_ZAINQLATTRS, he5_zainqlattrs,INT,STRING,PSTRING,PLONG)
FCALLSCFUN3(LONG, HE5_ZAinqgattrs, HE5_ZAINQGATTRS, he5_zainqgattrs,INT,PSTRING,PLONG)
FCALLSCFUN8(INT, HE5_ZAidtype, HE5_ZAIDYPEF, he5_zaidtype, INT,STRING,STRING,INT,PINT,PINT,PINT,PLONG)
FCALLSCFUN5(INT, HE5_ZAaliasinfoF, HE5_ZAALIASINFO, he5_zaaliasinfo, INT, INT, STRING, PINT, STRING)
FCALLSCFUN3(LONG, HE5_ZAinqfldaliasF, HE5_ZAINQFLDALIAS, he5_zainqfldalias,INT,PSTRING,PLONG)
FCALLSCFUN4(INT, HE5_ZAchunkinfoF, HE5_ZACHUNKINFO, he5_zachunkinfo,INT,STRING,PINT,LONGV)
FCALLSCFUN4(LONG, HE5_ZAgetaliaslistF, HE5_ZAGETALIASLIST, he5_zagetaliaslist,INT,INT,PSTRING,PLONG)

FCALLSCFUN5(INT, HE5_ZAdefdimscaleF, HE5_ZADEFDIMSCALE, he5_zadefdimscale,INT,STRING,LONG,INT,PVOID)
FCALLSCFUN6(INT, HE5_ZAsetdimscaleF, HE5_ZASETDIMSCALE, he5_zasetdimscale,INT,STRING,STRING,LONG,INT,PVOID)
FCALLSCFUN6(LONG, HE5_ZAgetdimscaleF, HE5_ZAGETDIMSCALE, he5_zagetdimscale,INT,STRING,STRING,PLONG,PINT,PVOID)
FCALLSCFUN4(INT, HE5_ZAreaddscaleattrF,HE5_ZAREADDSCALEATTR, he5_zareaddscaleattr,INT,STRING,STRING,PVOID)
FCALLSCFUN6(INT, HE5_ZAwritedscaleattrF, HE5_ZAWRITEDSCALEATTR, he5_zawritedscaleattr,INT,STRING,STRING,INT,LONGV,PVOID)
FCALLSCFUN5(INT, HE5_ZAdscaleattrinfoF, HE5_ZADSCALEATTRINFO, he5_zadscaleattrinfo,INT,STRING,STRING,PINT,PLONG)
FCALLSCFUN4(INT, HE5_ZAinqdscaleattrsF, HE5_ZAINQDSCALATTRS, he5_zainqdscaleattrs,INT,STRING,STRING,PLONG)


/* EXTERNAL DATA FILE INTERFACE */
 
FCALLSCFUN4(INT, HE5_ZAsetextdataF, HE5_ZASETXDAT, he5_zasetxdat,INT,STRING,LONGV,LONGV)
FCALLSCFUN6(INT, HE5_ZAgetextdataF, HE5_ZAGETXDAT, he5_zagetxdat,INT,STRING,LONG,PSTRING,LONGV,LONGV)

#endif








