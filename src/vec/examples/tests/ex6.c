/*$Id: ex6.c,v 1.44 2001/01/17 22:21:24 bsmith Exp bsmith $*/

static char help[] = "Demonstrates a scatter with a stride and general index set.\n\n";

#include "petscvec.h"
#include "petscsys.h"

#undef __FUNC__
#define __FUNC__ "main"
int main(int argc,char **argv)
{
  int           n = 6,ierr,idx1[3] = {0,1,2},loc[6] = {0,1,2,3,4,5};
  Scalar        two = 2.0,vals[6] = {10,11,12,13,14,15};
  Vec           x,y;
  IS            is1,is2;
  VecScatter    ctx = 0;

  ierr = PetscInitialize(&argc,&argv,(char*)0,help);CHKERRQ(ierr); 

  /* create two vector */
  ierr = VecCreateSeq(PETSC_COMM_SELF,n,&x);CHKERRQ(ierr);
  ierr = VecDuplicate(x,&y);CHKERRQ(ierr);

  /* create two index sets */
  ierr = ISCreateGeneral(PETSC_COMM_SELF,3,idx1,&is1);CHKERRQ(ierr);
  ierr = ISCreateStride(PETSC_COMM_SELF,3,0,2,&is2);CHKERRQ(ierr);

  ierr = VecSetValues(x,6,loc,vals,INSERT_VALUES);CHKERRQ(ierr);
  ierr = VecView(x,PETSC_VIEWER_STDOUT_SELF);CHKERRQ(ierr);
  ierr = PetscPrintf(PETSC_COMM_SELF,"----\n");CHKERRQ(ierr);
  ierr = VecSet(&two,y);CHKERRQ(ierr);
  ierr = VecScatterCreate(x,is1,y,is2,&ctx);CHKERRQ(ierr);
  ierr = VecScatterBegin(x,y,INSERT_VALUES,SCATTER_FORWARD,ctx);CHKERRQ(ierr);
  ierr = VecScatterEnd(x,y,INSERT_VALUES,SCATTER_FORWARD,ctx);CHKERRQ(ierr);
  ierr = VecScatterDestroy(ctx);CHKERRQ(ierr);
  
  ierr = VecView(y,PETSC_VIEWER_STDOUT_SELF);CHKERRQ(ierr);

  ierr = ISDestroy(is1);CHKERRQ(ierr);
  ierr = ISDestroy(is2);CHKERRQ(ierr);
  ierr = VecDestroy(x);CHKERRQ(ierr);
  ierr = VecDestroy(y);CHKERRQ(ierr);

  PetscFinalize();
  return 0;
}
 
