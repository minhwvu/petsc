#ifndef lint
static char vcid[] = "$Id: index.c,v 1.27 1996/04/20 04:18:30 bsmith Exp curfman $";
#endif
/*  
   Defines the abstract operations on index sets 
*/
#include "isimpl.h"      /*I "is.h" I*/

/*@
   ISIdentity - Determines if index set is the identity mapping.

   Input Parmeters:
.  is - the index set

   Output Parameters:
.  ident - PETSC_TRUE if an identity

.keywords: IS, index set, identity

.seealso: ISSetIdentity()
@*/
int ISIdentity(IS is,PetscTruth *ident)
{
  if (!is) SETERRQ(-1,"ISIdentity:Null IS");
  if (is->cookie != IS_COOKIE) SETERRQ(-1,"ISIdentity:Bad IS");
  *ident = (PetscTruth) is->isidentity;
  return 0;
}

/*@
   ISSetIdentity - Informs the index set that it is an identity.

   Input Parmeters:
.  is - the index set

.keywords: IS, index set, identity

.seealso: ISIdentity()
@*/
int ISSetIdentity(IS is)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  is->isidentity = 1;
  return 0;
}


/*@
   ISPermutation - PETSC_TRUE or PETSC_FALSE depending on if the 
      index set has been declared to be a permutation.

   Input Parmeters:
.  is - the index set

   Output Parameters:
.  perm - PETSC_TRUE if a permutation

.keywords: IS, index set, permutation

.seealso: ISSetPermutation()
@*/
int ISPermutation(IS is,PetscTruth *perm)
{
  if (!is) SETERRQ(-1,"ISPermutation:Null IS");
  if (is->cookie != IS_COOKIE) SETERRQ(-1,"ISPermutation:Bad IS");
  *perm = (PetscTruth) is->isperm;
  return 0;
}

/*@
   ISSetPermutation - Informs the index set that it is a permutation.

   Input Parmeters:
.  is - the index set

.keywords: IS, index set, permutation

.seealso: ISPermutation()
@*/
int ISSetPermutation(IS is)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  is->isperm = 1;
  return 0;
}

/*@C
   ISDestroy - Destroys an index set.

   Input Parameters:
.  is - the index set

.keywords: IS, index set, destroy

.seealso: ISCreateSeq(), ISCreateStrideSeq()
@*/
int ISDestroy(IS is)
{
  if (!is) return 0;
  PetscValidHeaderSpecific(is,IS_COOKIE);
  return (*is->destroy)((PetscObject) is);
}

/*@
   ISInvertPermutation - Creates a new permutation that is the inverse of 
                         a given permutation.

   Input Parameter:
.  is - the index set

   Output Parameter:
.  isout - the inverse permutation

.keywords: IS, index set, invert, inverse, permutation
@*/
int ISInvertPermutation(IS is,IS *isout)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  if (!is->isperm) SETERRQ(1,"ISInvertPermutation:not permutation");
  return (*is->ops.invertpermutation)(is,isout);
}

/*@
   ISGetSize - Returns the global length of an index set. 

   Input Parameter:
.  is - the index set

   Output Parameter:
.  size - the global size

.keywords: IS, index set, get, global, size

@*/
int ISGetSize(IS is,int *size)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  return (*is->ops.getsize)(is,size);
}

/*@C
   ISGetIndices - Returns a pointer to the indices.  The user should call 
   ISRestoreIndices() after having looked at the indices.  The user should 
   NOT change the indices.

   Input Parameter:
.  is - the index set

   Output Parameter:
.  ptr - the location to put the pointer to the indices

   Notes:
   In a parallel enviroment this probably points to only the indices that 
   are local to a particular processor.

   Fortran Note:
   The Fortran interface is slightly different from that given below.
   See the Fortran chapter of the users manual and 
   petsc/src/is/examples for details.

.keywords: IS, index set, get, indices

.seealso: ISRestoreIndices()
@*/
int ISGetIndices(IS is,int **ptr)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  return (*is->ops.getindices)(is,ptr);
} 

/*@C
   ISRestoreIndices - Restores an index set to a usable state after a call 
   to ISGetIndices().

   Input Parameters:
.  is - the index set
.  ptr - the pointer obtained by ISGetIndices()

   Fortran Note:
   The Fortran interface is slightly different from that given below.
   See the users manual and petsc/src/is/examples for details.

.keywords: IS, index set, restore, indices

.seealso: ISGetIndices()
@*/
int ISRestoreIndices(IS is,int **ptr)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  if (is->ops.restoreindices) return (*is->ops.restoreindices)(is,ptr);
  else return 0;
}

/*@
   ISView - Displays an index set.

   Input Parameters:
.  is - the index set
.  viewer - viewer used to display the set, for example STDOUT_VIEWER_SELF.

.keywords: IS, index set, restore, indices

.seealso: ViewerFileOpenASCII()
@*/
int ISView(IS is, Viewer viewer)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  return (*is->view)((PetscObject)is,viewer);
}

/*@
   ISSort - Sort the indices of an index set.

   Input Parameters:
.  is - the index set

.keywords: IS, index set, sort, indices

.seealso: ISSorted()
@*/
int ISSort(IS is)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  return (*is->ops.sortindices)(is);
}

/*@
   ISSorted - Checks the indices to determine if they have been sorted.

   Input Parameters:
.  is - the index set

   Output Parameters:
.  flg - output flag, either
$     1 if the index set is sorted;
$     0 otherwise.

.keywords: IS, index set, sort, indices

.seealso: ISSort()
@*/
int ISSorted(IS is, PetscTruth *flg)
{
  PetscValidHeaderSpecific(is,IS_COOKIE);
  return (*is->ops.sorted)(is, flg);
}
