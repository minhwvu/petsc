/*
  Code for time stepping with the multi-rate Runge-Kutta method

  Notes:
  1) The general system is written as
     Udot = F(t,U) for the nonsplit version of multi-rate RK,
     user should give the indexes for both slow and fast components;
  2) The general system is written as
     Usdot = Fs(t,Us,Uf)
     Ufdot = Ff(t,Us,Uf) for multi-rate RK with RHS splits,
     user should partioned RHS by themselves and also provide the indexes for both slow and fast components.
*/

#include <petsc/private/tsimpl.h>
#include <petscdm.h>
#include <../src/ts/impls/explicit/rk/rk.h>

static TSRKType TSMRKDefault = TSRK2A;

static PetscErrorCode TSSetUp_MRKSPLIT(TS ts)
{
  TS_RK          *rk = (TS_RK*)ts->data;
  RKTableau      tab = rk->tableau;
  DM             dm,subdm,newdm;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = TSRHSSplitGetIS(ts,"slow",&rk->is_slow);CHKERRQ(ierr);
  ierr = TSRHSSplitGetIS(ts,"fast",&rk->is_fast);CHKERRQ(ierr);
  if (!rk->is_slow || !rk->is_fast) SETERRQ(PetscObjectComm((PetscObject)ts),PETSC_ERR_USER,"Must set up RHSSplits with TSRHSSplitSetIS() using split names 'slow' and 'fast' respectively in order to use -ts_type bsi");
  ierr = TSRHSSplitGetSubTS(ts,"slow",&rk->subts_slow);CHKERRQ(ierr);
  ierr = TSRHSSplitGetSubTS(ts,"fast",&rk->subts_fast);CHKERRQ(ierr);
  if (!rk->subts_slow || !rk->subts_fast) SETERRQ(PetscObjectComm((PetscObject)ts),PETSC_ERR_USER,"Must set up the RHSFunctions for 'slow' and 'fast' components using TSRHSSplitSetRHSFunction() or calling TSSetRHSFunction() for each sub-TS");

  /* Only copy */
  ierr = TSGetDM(ts,&dm);CHKERRQ(ierr);
  ierr = DMClone(dm,&newdm);CHKERRQ(ierr);
  ierr = TSGetDM(rk->subts_fast,&subdm);CHKERRQ(ierr);
  ierr = DMCopyDMTS(subdm,newdm);CHKERRQ(ierr);
  ierr = DMCopyDMSNES(subdm,newdm);CHKERRQ(ierr);
  ierr = TSSetDM(rk->subts_fast,newdm);CHKERRQ(ierr);
  ierr = DMDestroy(&newdm);CHKERRQ(ierr);
  ierr = DMClone(dm,&newdm);CHKERRQ(ierr);
  ierr = TSGetDM(rk->subts_slow,&subdm);CHKERRQ(ierr);
  ierr = DMCopyDMTS(subdm,newdm);CHKERRQ(ierr);
  ierr = DMCopyDMSNES(subdm,newdm);CHKERRQ(ierr);
  ierr = TSSetDM(rk->subts_slow,newdm);CHKERRQ(ierr);
  ierr = DMDestroy(&newdm);CHKERRQ(ierr);

  ierr = PetscMalloc1(tab->s,&rk->YdotRHS_fast);CHKERRQ(ierr);
  ierr = PetscMalloc1(tab->s,&rk->YdotRHS_slow);CHKERRQ(ierr);
  ierr = VecDuplicate(ts->vec_sol,&rk->X0);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

static PetscErrorCode TSReset_MRKSPLIT(TS ts)
{
  TS_RK          *rk = (TS_RK*)ts->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = PetscFree(rk->YdotRHS_fast);CHKERRQ(ierr);
  ierr = PetscFree(rk->YdotRHS_slow);CHKERRQ(ierr);
  ierr = VecDestroy(&rk->X0);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

static PetscErrorCode TSSetUp_MRKNONSPLIT(TS ts)
{
  TS_RK          *rk = (TS_RK*)ts->data;
  RKTableau       tab  = rk->tableau;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = TSRHSSplitGetIS(ts,"slow",&rk->is_slow);CHKERRQ(ierr);
  ierr = TSRHSSplitGetIS(ts,"fast",&rk->is_fast);CHKERRQ(ierr);
  if (!rk->is_slow || !rk->is_fast) SETERRQ(PetscObjectComm((PetscObject)ts),PETSC_ERR_USER,"Must set up RHSSplits with TSRHSSplitSetIS() using split names 'slow' and 'fast' respectively in order to use -ts_type bsi");
  ierr = VecDuplicate(ts->vec_sol,&rk->X0);CHKERRQ(ierr);
  ierr = VecDuplicateVecs(ts->vec_sol,tab->s,&rk->YdotRHS_slow);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

static PetscErrorCode TSReset_MRKNONSPLIT(TS ts)
{
  TS_RK          *rk = (TS_RK*)ts->data;
  RKTableau      tab  = rk->tableau;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  ierr = VecDestroy(&rk->X0);CHKERRQ(ierr);
  ierr = VecDestroyVecs(tab->s,&rk->YdotRHS_slow);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

static PetscErrorCode TSInterpolate_MRKNONSPLIT(TS ts,PetscReal itime,Vec X)
{
  TS_RK            *rk = (TS_RK*)ts->data;
  PetscInt         s  = rk->tableau->s,p = rk->tableau->p,i,j;
  PetscReal        h;
  PetscReal        tt,t;
  PetscScalar      *b;
  const PetscReal  *B = rk->tableau->binterp;
  PetscErrorCode   ierr;

  PetscFunctionBegin;
  if (!B) SETERRQ1(PetscObjectComm((PetscObject)ts),PETSC_ERR_SUP,"TSRK %s does not have an interpolation formula",rk->tableau->name);

  switch (rk->status) {
    case TS_STEP_INCOMPLETE:
    case TS_STEP_PENDING:
      h = ts->time_step;
      t = (itime - ts->ptime)/h;
      break;
    case TS_STEP_COMPLETE:
      h = ts->ptime - ts->ptime_prev;
      t = (itime - ts->ptime)/h + 1; /* In the interval [0,1] */
      break;
    default: SETERRQ(PetscObjectComm((PetscObject)ts),PETSC_ERR_PLIB,"Invalid TSStepStatus");
  }
  ierr = PetscMalloc1(s,&b);CHKERRQ(ierr);
  for (i=0; i<s; i++) b[i] = 0;
  for (j=0,tt=t; j<p; j++,tt*=t) {
    for (i=0; i<s; i++) {
      b[i]  += h * B[i*p+j] * tt;
    }
  }
  ierr = VecCopy(rk->X0,X);CHKERRQ(ierr);
  ierr = VecMAXPY(X,s,b,rk->YdotRHS_slow);CHKERRQ(ierr);
  ierr = PetscFree(b);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

static PetscErrorCode TSInterpolate_MRKSPLIT(TS ts,PetscReal itime,Vec X)
{
  TS_RK            *rk = (TS_RK*)ts->data;
  PetscInt         s  = rk->tableau->s,p = rk->tableau->p,i,j;
  Vec              Yslow;    /* vector holds the slow components in Y[0] */
  PetscReal        h;
  PetscReal        tt,t;
  PetscScalar      *b;
  const PetscReal  *B = rk->tableau->binterp;
  PetscErrorCode   ierr;

  PetscFunctionBegin;
  if (!B) SETERRQ1(PetscObjectComm((PetscObject)ts),PETSC_ERR_SUP,"TSRK %s does not have an interpolation formula",rk->tableau->name);

  switch (rk->status) {
    case TS_STEP_INCOMPLETE:
    case TS_STEP_PENDING:
      h = ts->time_step;
      t = (itime - ts->ptime)/h;
      break;
    case TS_STEP_COMPLETE:
      h = ts->ptime - ts->ptime_prev;
      t = (itime - ts->ptime)/h + 1; /* In the interval [0,1] */
      break;
    default: SETERRQ(PetscObjectComm((PetscObject)ts),PETSC_ERR_PLIB,"Invalid TSStepStatus");
  }
  ierr = PetscMalloc1(s,&b);CHKERRQ(ierr);
  for (i=0; i<s; i++) b[i] = 0;
  for (j=0,tt=t; j<p; j++,tt*=t) {
    for (i=0; i<s; i++) {
      b[i]  += h * B[i*p+j] * tt;
    }
  }
  ierr = VecGetSubVector(rk->X0,rk->is_slow,&Yslow);CHKERRQ(ierr);
  ierr = VecCopy(Yslow,X);CHKERRQ(ierr);
  ierr = VecMAXPY(X,s,b,rk->YdotRHS_slow);CHKERRQ(ierr);
  ierr = VecRestoreSubVector(rk->X0,rk->is_slow,&Yslow);CHKERRQ(ierr);
  ierr = PetscFree(b);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

static PetscErrorCode TSStep_MRKNONSPLIT(TS ts)
{
  TS_RK             *rk = (TS_RK*)ts->data;
  RKTableau         tab  = rk->tableau;
  Vec               *Y = rk->Y,*YdotRHS = rk->YdotRHS,*YdotRHS_slow = rk->YdotRHS_slow;
  Vec               stage_slow,sol_slow;   /* vectors store the slow components */
  Vec               subvec_slow;           /* sub vector to store the slow components */
  const PetscInt    s = tab->s;
  const PetscReal   *A = tab->A,*c = tab->c;
  PetscScalar       *w = rk->work;
  PetscInt          i,j,k,dtratio = rk->dtratio;
  PetscReal         next_time_step = ts->time_step,t = ts->ptime,h = ts->time_step;
  PetscErrorCode    ierr;

  PetscFunctionBegin;
  rk->status = TS_STEP_INCOMPLETE;
  ierr = VecDuplicate(ts->vec_sol,&stage_slow);CHKERRQ(ierr);
  ierr = VecDuplicate(ts->vec_sol,&sol_slow);CHKERRQ(ierr);
  ierr = VecCopy(ts->vec_sol,rk->X0);CHKERRQ(ierr);
  for (i=0; i<s; i++) {
    rk->stage_time = t + h*c[i];
    ierr = TSPreStage(ts,rk->stage_time);CHKERRQ(ierr);
    ierr = VecCopy(ts->vec_sol,Y[i]);CHKERRQ(ierr);
    for (j=0; j<i; j++) w[j] = h*A[i*s+j];
    ierr = VecMAXPY(Y[i],i,w,YdotRHS_slow);CHKERRQ(ierr);
    ierr = TSPostStage(ts,rk->stage_time,i,Y); CHKERRQ(ierr);
    /* compute the stage RHS */
    ierr = TSComputeRHSFunction(ts,t+h*c[i],Y[i],YdotRHS_slow[i]);CHKERRQ(ierr);
  }
  /* update the slow components in the solution */
  rk->YdotRHS = YdotRHS_slow;
  rk->dtratio = 1;
  ierr = TSEvaluateStep(ts,tab->order,sol_slow,NULL);CHKERRQ(ierr);
  rk->dtratio = dtratio;
  rk->YdotRHS = YdotRHS;
  for (k=0; k<rk->dtratio; k++) {
    for (i=0; i<s; i++) {
      rk->stage_time = t + h/rk->dtratio*c[i];
      ierr = TSPreStage(ts,rk->stage_time);CHKERRQ(ierr);
      /* update the fast components in the stage value, the slow components will be overwritten, so it is ok to have garbage in the slow components */
      ierr = VecCopy(ts->vec_sol,Y[i]);CHKERRQ(ierr);
      for (j=0; j<i; j++) w[j] = h/rk->dtratio*A[i*s+j];
      ierr = VecMAXPY(Y[i],i,w,YdotRHS);CHKERRQ(ierr);
      ierr = TSInterpolate_MRKNONSPLIT(ts,t+k*h/rk->dtratio+h/rk->dtratio*c[i],stage_slow);CHKERRQ(ierr);
      /* update the slow components in the stage value */
      ierr = VecGetSubVector(stage_slow,rk->is_slow,&subvec_slow);CHKERRQ(ierr);
      ierr = VecISCopy(Y[i],rk->is_slow,SCATTER_FORWARD,subvec_slow);CHKERRQ(ierr);
      ierr = VecRestoreSubVector(stage_slow,rk->is_slow,&subvec_slow);CHKERRQ(ierr);
      ierr = TSPostStage(ts,rk->stage_time,i,Y);CHKERRQ(ierr);
      /* compute the stage RHS */
      ierr = TSComputeRHSFunction(ts,t+k*h/rk->dtratio+h/rk->dtratio*c[i],Y[i],YdotRHS[i]);CHKERRQ(ierr);
    }
    ierr = TSEvaluateStep(ts,tab->order,ts->vec_sol,NULL);CHKERRQ(ierr);
  }
  /* update the slow components in the solution */
  ierr = VecGetSubVector(sol_slow,rk->is_slow,&subvec_slow);CHKERRQ(ierr);
  ierr = VecISCopy(ts->vec_sol,rk->is_slow,SCATTER_FORWARD,subvec_slow);CHKERRQ(ierr);
  ierr = VecRestoreSubVector(sol_slow,rk->is_slow,&subvec_slow);CHKERRQ(ierr);

  ts->ptime += ts->time_step;
  ts->time_step = next_time_step;
  rk->status = TS_STEP_COMPLETE;
  /* free memory of work vectors */
  ierr = VecDestroy(&stage_slow);CHKERRQ(ierr);
  ierr = VecDestroy(&sol_slow);CHKERRQ(ierr);
  PetscFunctionReturn(0);
}

/*
 This is for partitioned RHS multirate RK method
 The step completion formula is

 x1 = x0 + h b^T YdotRHS

*/
static PetscErrorCode TSEvaluateStep_MRKSPLIT(TS ts,PetscInt order,Vec X,PetscBool *done)
{
  TS_RK           *rk = (TS_RK*)ts->data;
  RKTableau       tab  = rk->tableau;
  Vec             Xslow,Xfast;                  /* subvectors of X which store slow components and fast components respectively */
  PetscScalar     *w = rk->work;
  PetscReal       h = ts->time_step;
  PetscInt        s = tab->s,j;
  PetscErrorCode  ierr;

  PetscFunctionBegin;
  ierr = VecCopy(ts->vec_sol,X);CHKERRQ(ierr);
  if (rk->slow) {
    for (j=0; j<s; j++) w[j] = h*tab->b[j];
    ierr = VecGetSubVector(ts->vec_sol,rk->is_slow,&Xslow);CHKERRQ(ierr);
    ierr = VecMAXPY(Xslow,s,w,rk->YdotRHS_slow);CHKERRQ(ierr);
    ierr = VecRestoreSubVector(ts->vec_sol,rk->is_slow,&Xslow);CHKERRQ(ierr);;
  } else {
    for (j=0; j<s; j++) w[j] = h/rk->dtratio*tab->b[j];
    ierr = VecGetSubVector(X,rk->is_fast,&Xfast);CHKERRQ(ierr);
    ierr = VecMAXPY(Xfast,s,w,rk->YdotRHS_fast);CHKERRQ(ierr);
    ierr = VecRestoreSubVector(X,rk->is_fast,&Xfast);CHKERRQ(ierr);
  }
  PetscFunctionReturn(0);
}

static PetscErrorCode TSStep_MRKSPLIT(TS ts)
{
  TS_RK             *rk = (TS_RK*)ts->data;
  RKTableau         tab = rk->tableau;
  Vec               *Y = rk->Y,*YdotRHS = rk->YdotRHS;
  Vec               *YdotRHS_fast = rk->YdotRHS_fast,*YdotRHS_slow = rk->YdotRHS_slow;
  Vec               Yslow,Yfast;                         /* subvectors store the stges of slow components and fast components respectively                           */
  const PetscInt    s = tab->s;
  const PetscReal   *A = tab->A,*c = tab->c;
  PetscScalar       *w = rk->work;
  PetscInt          i,j,k;
  PetscReal         next_time_step = ts->time_step,t = ts->ptime,h = ts->time_step;
  PetscErrorCode    ierr;

  PetscFunctionBegin;
  rk->status = TS_STEP_INCOMPLETE;
  for (i=0; i<s; i++) {
    ierr = VecGetSubVector(YdotRHS[i],rk->is_slow,&YdotRHS_slow[i]);CHKERRQ(ierr);
    ierr = VecGetSubVector(YdotRHS[i],rk->is_fast,&YdotRHS_fast[i]);CHKERRQ(ierr);
  }
  ierr = VecCopy(ts->vec_sol,rk->X0);CHKERRQ(ierr);
  /* propogate both slow and fast components using large time steps */
  for (i=0; i<s; i++) {
    rk->stage_time = t + h*c[i];
    ierr = TSPreStage(ts,rk->stage_time);CHKERRQ(ierr);
    ierr = VecCopy(ts->vec_sol,Y[i]);CHKERRQ(ierr);
    /*ierr = VecCopy(ts->vec_sol,Yc[i]);CHKERRQ(ierr);*/
    ierr = VecGetSubVector(Y[i],rk->is_slow,&Yslow);CHKERRQ(ierr);
    ierr = VecGetSubVector(Y[i],rk->is_fast,&Yfast);CHKERRQ(ierr);
    for (j=0; j<i; j++) w[j] = h*A[i*s+j];
    ierr = VecMAXPY(Yslow,i,w,YdotRHS_slow);CHKERRQ(ierr);
    ierr = VecMAXPY(Yfast,i,w,YdotRHS_fast);CHKERRQ(ierr);
    ierr = VecRestoreSubVector(Y[i],rk->is_slow,&Yslow);CHKERRQ(ierr);
    ierr = VecRestoreSubVector(Y[i],rk->is_fast,&Yfast);CHKERRQ(ierr);
    ierr = TSPostStage(ts,rk->stage_time,i,Y); CHKERRQ(ierr);
    ierr = TSComputeRHSFunction(rk->subts_slow,t+h*c[i],Y[i],YdotRHS_slow[i]);CHKERRQ(ierr);
    ierr = TSComputeRHSFunction(rk->subts_fast,t+h*c[i],Y[i],YdotRHS_fast[i]);CHKERRQ(ierr);
  }
  rk->slow = PETSC_TRUE;
  ierr = TSEvaluateStep_MRKSPLIT(ts,tab->order,ts->vec_sol,NULL);CHKERRQ(ierr);
  for (k=0; k<rk->dtratio; k++) {
    /* propogate fast component using small time steps */
    for (i=0; i<s; i++) {
      rk->stage_time = t + h/rk->dtratio*c[i];
      ierr = TSPreStage(ts,rk->stage_time);CHKERRQ(ierr);
      ierr = VecCopy(ts->vec_sol,Y[i]);CHKERRQ(ierr);
      /* stage value for fast components */
      for (j=0; j<i; j++) w[j] = h/rk->dtratio*A[i*s+j];
      ierr = VecGetSubVector(Y[i],rk->is_fast,&Yfast);CHKERRQ(ierr);
      ierr = VecMAXPY(Yfast,i,w,YdotRHS_fast);CHKERRQ(ierr);
      ierr = VecRestoreSubVector(Y[i],rk->is_fast,&Yfast);CHKERRQ(ierr);
      /* stage value for slow components */
      ierr = VecGetSubVector(Y[i],rk->is_slow,&Yslow);CHKERRQ(ierr);
      ierr = TSInterpolate_MRKSPLIT(ts,t+k*h/rk->dtratio+h/rk->dtratio*c[i],Yslow);CHKERRQ(ierr);
      ierr = VecRestoreSubVector(Y[i],rk->is_slow,&Yslow);CHKERRQ(ierr);
      ierr = TSPostStage(ts,rk->stage_time,i,Y); CHKERRQ(ierr);
      /* compute the stage RHS for fast components */
      ierr = TSComputeRHSFunction(rk->subts_fast,t+k*h/rk->dtratio+h/rk->dtratio*c[i],Y[i],YdotRHS_fast[i]);CHKERRQ(ierr);
    }
    /* update the value of fast components when using fast time step */
    rk->slow = PETSC_FALSE;
    ierr = TSEvaluateStep_MRKSPLIT(ts,tab->order,ts->vec_sol,NULL);CHKERRQ(ierr);
  }
  for (i=0; i<s; i++) {
    ierr = VecRestoreSubVector(YdotRHS[i],rk->is_slow,&YdotRHS_slow[i]);CHKERRQ(ierr);
    ierr = VecRestoreSubVector(YdotRHS[i],rk->is_fast,&YdotRHS_fast[i]);CHKERRQ(ierr);
  }
  ts->ptime += ts->time_step;
  ts->time_step = next_time_step;
  rk->status = TS_STEP_COMPLETE;
  PetscFunctionReturn(0);
}

/*@C
  TSRKSetMultirateType - Set the type of RK Multirate scheme

  Logically collective

  Input Parameter:
+  ts - timestepping context
-  mrktype - type of MRK-scheme

  Options Database:
.   -ts_rk_multiarte_type - <none,nonsplit,split>

  Level: intermediate
@*/
PetscErrorCode TSRKSetMultirateType(TS ts, TSMRKType mrktype)
{
  TS_RK          *rk = (TS_RK*)ts->data;
  PetscErrorCode ierr;

  PetscFunctionBegin;
  PetscValidHeaderSpecific(ts,TS_CLASSID,1);
  switch(mrktype){
    case TSMRKNONE:
      break;
    case TSMRKNONSPLIT:
      ts->ops->step           = TSStep_MRKNONSPLIT;
      ts->ops->interpolate    = TSInterpolate_MRKNONSPLIT;
      rk->dtratio             = 2;
      ierr = TSRKSetType(ts,TSMRKDefault);CHKERRQ(ierr);
      ierr = PetscObjectComposeFunction((PetscObject)ts,"TSSetUp_MRKNONSPLIT_C",TSSetUp_MRKNONSPLIT);CHKERRQ(ierr);
      ierr = PetscObjectComposeFunction((PetscObject)ts,"TSReset_MRKNONSPLIT_C",TSReset_MRKNONSPLIT);CHKERRQ(ierr);
      break;
    case TSMRKSPLIT:
      ts->ops->step           = TSStep_MRKSPLIT;
      ts->ops->evaluatestep   = TSEvaluateStep_MRKSPLIT;
      ts->ops->interpolate    = TSInterpolate_MRKSPLIT;
      rk->dtratio             = 2;
      ierr = TSRKSetType(ts,TSMRKDefault);CHKERRQ(ierr);
      ierr = PetscObjectComposeFunction((PetscObject)ts,"TSSetUp_MRKSPLIT_C",TSSetUp_MRKSPLIT);CHKERRQ(ierr);
      ierr = PetscObjectComposeFunction((PetscObject)ts,"TSReset_MRKSPLIT_C",TSReset_MRKSPLIT);CHKERRQ(ierr);
      break;
    default :
      SETERRQ1(PetscObjectComm((PetscObject)ts),PETSC_ERR_ARG_UNKNOWN_TYPE,"Unknown type '%s'",mrktype);
  }
  PetscFunctionReturn(0);
}

