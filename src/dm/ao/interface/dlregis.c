#ifdef PETSC_RCS_HEADER
static char vcid[] = "$Id: dlregis.c,v 1.1 2000/01/10 06:34:46 knepley Exp $";
#endif

#include "petscao.h"
#include "petscda.h"

#undef __FUNCT__  
#define __FUNCT__ "DMInitializePackage"
/*@C
  DMInitializePackage - This function initializes everything in the DM package. It is called
  from PetscDLLibraryRegister() when using dynamic libraries, and on the first call to AOCreate()
  or DACreate() when using static libraries.

  Input Parameter:
  path - The dynamic library path, or PETSC_NULL

  Level: developer

.keywords: AO, DA, initialize, package
.seealso: PetscInitialize()
@*/
int DMInitializePackage(char *path) {
  static int initialized = 0;
  char       logList[256];
  char      *className;
  PetscTruth opt;
  int        ierr;

  PetscFunctionBegin;
  if (initialized) PetscFunctionReturn(0);
  initialized = 1;
  /* Register Classes */
  ierr = PetscLogClassRegister(&AO_COOKIE,     "Application Order");                                      CHKERRQ(ierr);
  ierr = PetscLogClassRegister(&AODATA_COOKIE, "Application Data");                                       CHKERRQ(ierr);
  ierr = PetscLogClassRegister(&DA_COOKIE,     "Distributed array");                                      CHKERRQ(ierr);
  /* Register Constructors and Serializers */
  ierr = AOSerializeRegisterAll(path);                                                                    CHKERRQ(ierr);
  /* Register Events */
  ierr = PetscLogEventRegister(&AOEvents[AO_PetscToApplication], "AOPetscToApplication", PETSC_NULL, AO_COOKIE);CHKERRQ(ierr);
  ierr = PetscLogEventRegister(&AOEvents[AO_ApplicationToPetsc], "AOApplicationToPetsc", PETSC_NULL, AO_COOKIE);CHKERRQ(ierr);
  ierr = PetscLogEventRegister(&DAEvents[DA_GlobalToLocal],      "DAGlobalToLocal",      PETSC_NULL, DA_COOKIE);CHKERRQ(ierr);
  ierr = PetscLogEventRegister(&DAEvents[DA_LocalToGlobal],      "DALocalToGlobal",      PETSC_NULL, DA_COOKIE);CHKERRQ(ierr);
  /* Process info exclusions */
  ierr = PetscOptionsGetString(PETSC_NULL, "-log_info_exclude", logList, 256, &opt);                      CHKERRQ(ierr);
  if (opt == PETSC_TRUE) {
    ierr = PetscStrstr(logList, "ao", &className);                                                        CHKERRQ(ierr);
    if (className) {
      ierr = PetscLogInfoDeactivateClass(AO_COOKIE);                                                      CHKERRQ(ierr);
    }
    ierr = PetscStrstr(logList, "da", &className);                                                        CHKERRQ(ierr);
    if (className) {
      ierr = PetscLogInfoDeactivateClass(DA_COOKIE);                                                      CHKERRQ(ierr);
    }
  }
  /* Process summary exclusions */
  ierr = PetscOptionsGetString(PETSC_NULL, "-log_summary_exclude", logList, 256, &opt);                   CHKERRQ(ierr);
  if (opt == PETSC_TRUE) {
    ierr = PetscStrstr(logList, "ao", &className);                                                        CHKERRQ(ierr);
    if (className) {
      ierr = PetscLogEventDeactivateClass(AO_COOKIE);                                                     CHKERRQ(ierr);
    }
    ierr = PetscStrstr(logList, "da", &className);                                                        CHKERRQ(ierr);
    if (className) {
      ierr = PetscLogEventDeactivateClass(DA_COOKIE);                                                     CHKERRQ(ierr);
    }
  }
  PetscFunctionReturn(0);
}

#ifdef PETSC_USE_DYNAMIC_LIBRARIES
EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "PetscDLLibraryRegister"
/*
  PetscDLLibraryRegister - This function is called when the dynamic library it is in is opened.

  This one registers all the mesh generators and partitioners that are in
  the basic DM library.

  Input Parameter:
  path - library path
*/
int PetscDLLibraryRegister(char *path)
{
  int ierr;

  ierr = PetscInitializeNoArguments();
  if (ierr) return(1);

  /*
      If we got here then PETSc was properly loaded
  */
  ierr = DMInitializePackage(path);                                                                       CHKERRQ(ierr);
  return(0);
}
EXTERN_C_END

/* --------------------------------------------------------------------------*/
static char *contents = "PETSc Distributed Structures library, includes\n\
Application Orderings, Application Data, and Distributed Arrays";

static char *authors = PETSC_AUTHOR_INFO;
static char *version = PETSC_VERSION_NUMBER;

/* --------------------------------------------------------------------------*/
EXTERN_C_BEGIN
#undef __FUNCT__  
#define __FUNCT__ "PetscDLLibraryInfo"
int PetscDLLibraryInfo(char *path,char *type,char **mess) 
{ 
  PetscTruth iscontents, isauthors, isversion;
  int        ierr;

  ierr = PetscStrcmp(type, "Contents", &iscontents);                                                      CHKERRQ(ierr);
  ierr = PetscStrcmp(type, "Authors",  &isauthors);                                                       CHKERRQ(ierr);
  ierr = PetscStrcmp(type, "Version",  &isversion);                                                       CHKERRQ(ierr);
  if      (iscontents == PETSC_TRUE) *mess = contents;
  else if (isauthors  == PETSC_TRUE) *mess = authors;
  else if (isversion  == PETSC_TRUE) *mess = version;
  else                               *mess = PETSC_NULL;

  return(0);
}
EXTERN_C_END

#endif /* PETSC_USE_DYNAMIC_LIBRARIES */
