/*
 ----------------------------------------------------------------------------
 |    Copyright (C) 2001 Emergent IT Inc. and Raytheon Systems Company      |
 |                                                                          | 
 |  Permission to use, modify, and distribute this software and its         |
 |  documentation for any purpose without fee is hereby granted, provided   |
 |  that the above copyright notice appear in all copies and that both that |
 |  copyright notice and this permission notice appear in supporting        |
 |                          documentation.                                  |
 ----------------------------------------------------------------------------
 */

#include <HE5_HdfEosDef.h>

#ifdef  _HDFEOS5_THREADSAFE

/* Global Variable definition */
/* -------------------------- */
pthread_once_t    HE5_HDFE_TS_FirstInit = PTHREAD_ONCE_INIT;

#endif

/*----------------------------------------------------------------------------
|  BEGIN_PROLOG                                                              | 
|                                                                            |
|  FUNCTION: HE5_TSinitfirst                                                 | 
|                                                                            | 
|  DESCRIPTION : This function initializes the mutex, once per run.          |
|                                                                            |
|                                                                            |
|  Return Value    Type     Units     Description                            | 
|  ============   ======  =========   =====================================  | 
|                                                                            |
|  INPUTS:   None                                                            | 
|                                                                            |
|                                                                            |
|  OUTPUTS:                                                                  | 
|                                                                            |
|             None                                                           | 
|                                                                            |
|  NOTES:                                                                    | 
|                                                                            |
|   Date      Programmer    Description                                      | 
|  ========   ============  ===============================================  | 
|  08/21/01   A.Muslimov    Original development.                            |
|  08/27/01   A.Muslimov    Added appropriate error handlings.               |
|                                                                            |
|  END_PROLOG                                                                | 
----------------------------------------------------------------------------*/
void    
HE5_TSinitfirst(void)
{
#ifdef  _HDFEOS5_THREADSAFE

  herr_t   status = FAIL;
  char     errbuf[HE5_HDFE_ERRBUFSIZE];

  /* Initialize Global Mutex Variable */
  /* -------------------------------- */
  GlobalMutex.MasterThread = (pthread_t *)NULL;

  /* Initialize mutex with default value */
  /* ----------------------------------- */
  status = pthread_mutex_init(&GlobalMutex.Lock, NULL);
  if (status != SUCCEED)
	{
	  status = FAIL;
	  sprintf(errbuf, "Failed to initialize the mutex.\n");
	  H5Epush(__FILE__, "HE5_TSinitfirst", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
	}

  /* Initialize condition variable with default attribute */
  /* ---------------------------------------------------- */
  status = pthread_cond_init(&GlobalMutex.CondVar, NULL);
  if (status != SUCCEED)
	{
	  status = FAIL;
	  sprintf(errbuf, "Failed to initialize the condition variable.\n");
	  H5Epush(__FILE__, "HE5_TSinitfirst", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
	}

  GlobalMutex.LockCount = 0;

#endif
}


/*----------------------------------------------------------------------------
|  BEGIN_PROLOG                                                              | 
|                                                                            |
|  FUNCTION: HE5_TSmutexlock                                                 | 
|                                                                            | 
|  DESCRIPTION : This function locks the mutex.                              |
|                                                                            |
|                                                                            |
|  Return Value    Type     Units     Description                            | 
|  ============   ======  =========   =====================================  | 
|  status         herr_t  none        Return status variable                 |
|                                                                            |
|  INPUTS:                                                                   | 
|  Mutex          struct              Input data structure                   |
|                                                                            |
|                                                                            |
|  OUTPUTS:                                                                  | 
|                                                                            |
|             None                                                           | 
|                                                                            |
|  NOTES:                                                                    | 
|                                                                            |
|   Date      Programmer    Description                                      | 
|  ========   ============  ===============================================  | 
|  08/21/01   A.Muslimov    Original development.                            |
|  08/27/01   A.Muslimov    Added appropriate error handlings.               |
|                                                                            |
|  END_PROLOG                                                                | 
----------------------------------------------------------------------------*/
herr_t   
HE5_TSmutexlock(HE5_HDFE_MutexStruct *Mutex)
{
  herr_t   status = SUCCEED;
#ifdef  _HDFEOS5_THREADSAFE
  char     errbuf[HE5_HDFE_ERRBUFSIZE];
#ifdef  _HDFEOS5_THREADSAFE_DEBUG
  time_t   lockTime;
#endif

  status = FAIL;

  /* Initialize the mutex */
  /* -------------------- */
  HE5_FIRST_THREAD_INIT

  /* Check the initialization */
  /* ------------------------ */
  if (status != SUCCEED)
	{
	  status = FAIL;
	  sprintf(errbuf, "Failed to initialize the mutex.\n");
	  H5Epush(__FILE__, "HE5_TSmutexlock", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
	  return(status);
	}	

#ifdef  _HDFEOS5_THREADSAFE_DEBUG
  time(&lockTime);
  printf("Attempt to lock mutex ... Time = %lf \n", (double)lockTime);
#endif

  /* Lock the mutex */
  /* -------------- */
  status = pthread_mutex_lock(&Mutex->Lock);
  if (status != SUCCEED)
	{
	  status = FAIL;
	  sprintf(errbuf, "Failed to lock the mutex.\n");
	  H5Epush(__FILE__, "HE5_TSmutexlock", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
	  return(status);
	}
  
  /* Find out if the mutex is locked and who owns the lock */
  /* ----------------------------------------------------- */
  if (Mutex->MasterThread && pthread_equal(pthread_self(), *Mutex->MasterThread)){
	/* already owned by self -- increment count */
	/* ---------------------------------------- */
	Mutex->LockCount++;
  } else if (!Mutex->MasterThread) {
	/* No one else has locked it -- set owner and get lock */
	/* --------------------------------------------------- */
	Mutex->MasterThread = (pthread_t *)malloc(sizeof(pthread_t));
	if (Mutex->MasterThread == (pthread_t *)NULL)
	  {
		status = FAIL;
		sprintf(errbuf, "Cannot allocate memory for master thread ID.\n");
		H5Epush(__FILE__, "HE5_TSmutexlock", __LINE__, H5E_RESOURCE, H5E_BADRANGE, errbuf);
		return(status);
	  }

	*Mutex->MasterThread = pthread_self();
	Mutex->LockCount = 1;
  } else {
	/* if already locked by someone else */
	/* --------------------------------- */
	for (;;) {
	  status = pthread_cond_wait(&Mutex->CondVar, &Mutex->Lock);
	  if (status != SUCCEED)
		{
		  status = FAIL;
		  sprintf(errbuf, "Failed to block the thread on condition variable.\n");
		  H5Epush(__FILE__, "HE5_TSmutexlock", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
		  return(status);
		}

	  if (!Mutex->MasterThread) {
		Mutex->MasterThread = (pthread_t *)malloc(sizeof(pthread_t));
		if (Mutex->MasterThread == (pthread_t *)NULL)
		  {
			status = FAIL;
			sprintf(errbuf, "Cannot allocate memory for master thread ID.\n");
			H5Epush(__FILE__, "HE5_TSmutexlock", __LINE__, H5E_RESOURCE, H5E_BADRANGE, errbuf);
			return(status);
		  }		
#ifdef  _HDFEOS5_THREADSAFE_DEBUG
		time(&lockTime);
		printf("Locking mutex ... Time = %lf \n", (double)lockTime);
#endif
		*Mutex->MasterThread = pthread_self();
		Mutex->LockCount = 1;
		break;
	  }
	}
  }
  

  /* Unlock the mutex */
  /* ---------------- */
  status = pthread_mutex_unlock(&Mutex->Lock);
  if (status != SUCCEED)
	{
	  status = FAIL;
	  sprintf(errbuf, "Failed to unlock the mutex.\n");
	  H5Epush(__FILE__, "HE5_TSmutexlock", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
	}
#ifdef  _HDFEOS5_THREADSAFE_DEBUG
  time(&lockTime);
  printf("Locking mutex ... Time = %lf \n", (double)lockTime);
#endif
#endif
  return(status);
}



/*----------------------------------------------------------------------------
|  BEGIN_PROLOG                                                              | 
|                                                                            |
|  FUNCTION: HE5_TSmutexunlock                                               | 
|                                                                            | 
|  DESCRIPTION : This function unlocks the mutex.                            |
|                                                                            |
|                                                                            |
|  Return Value    Type     Units     Description                            | 
|  ============   ======  =========   =====================================  |
|  status         herr_t              Return status variable                 |
|                                                                            |
|  INPUTS:                                                                   | 
|  Mutex          struct              data structure                         |
|                                                                            |
|                                                                            |
|  OUTPUTS:                                                                  | 
|                                                                            |
|             None                                                           | 
|                                                                            |
|  NOTES:                                                                    | 
|                                                                            |
|   Date      Programmer    Description                                      | 
|  ========   ============  ===============================================  | 
|  08/21/01   A.Muslimov    Original development.                            |
|  08/27/01   A.Muslimov    Added appropriate error handlings.               |
|                                                                            |
|  END_PROLOG                                                                | 
----------------------------------------------------------------------------*/
herr_t   
HE5_TSmutexunlock(HE5_HDFE_MutexStruct *Mutex)
{
  herr_t   status = SUCCEED;
#ifdef  _HDFEOS5_THREADSAFE
  char     errbuf[HE5_HDFE_ERRBUFSIZE];
#ifdef  _HDFEOS5_THREADSAFE_DEBUG
  time_t   unlockTime;
#endif  

  status = FAIL;

  /* Lock the mutex */
  /* -------------- */
  status = pthread_mutex_lock(&Mutex->Lock);
  if (status != SUCCEED)
	{
	  status = FAIL;
	  sprintf(errbuf, "Failed to unlock the mutex.\n");
	  H5Epush(__FILE__, "HE5_TSmutexunlock", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
	  return(status);
	}
  Mutex->LockCount--;
  
  if (Mutex->LockCount == 0)
	{
	  free(Mutex->MasterThread);
	  Mutex->MasterThread = NULL;
	  status = pthread_cond_signal(&Mutex->CondVar);
	  if (status != SUCCEED)
		{
		  status = FAIL;
		  sprintf(errbuf, "Failed to unblock the thread blocked on a condition variable.\n");
		  H5Epush(__FILE__, "HE5_TSmutexunlock", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
		  return(status);
		}
	}

  /* Unlock the mutex */
  /* ---------------- */
  status = pthread_mutex_unlock(&Mutex->Lock);
  if (status != SUCCEED)
	{
	  status = FAIL;
	  sprintf(errbuf, "Failed to unlock the mutex.\n");
	  H5Epush(__FILE__, "HE5_TSmutexunlock", __LINE__, H5E_FUNC, H5E_BADVALUE, errbuf);
	}
#ifdef  _HDFEOS5_THREADSAFE_DEBUG
  time(&unlockTime);
  printf("Unlocking mutex ... Time = %lf \n", (double)unlockTime);
#endif
#endif
  return(status);
}


























