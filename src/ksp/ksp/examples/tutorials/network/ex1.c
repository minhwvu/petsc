static char help[] = "This example demostrates the use of DMNetwork interface for solving a simple  electric circuit. \n\
                      The example can be found in p.150 of 'Strang, Gilbert. Computational Science and Engineering. Wellesley, MA'.\n\n";


/* T
   Concepts: DMNetwork
   Concepts: KSP
*/

#include <petscdmnetwork.h>
#include <petscksp.h>


/* 
   Structures containing physical data of circuit.
   Note that no topology is defined 
*/

typedef struct {
    PetscInt    id; /* node id */
    PetscScalar inj; /* current injection (A) */
    PetscBool   gr; /* grounded ? */
} Node;

typedef struct {
    PetscInt    id;  /* branch id */
    PetscScalar r;   /* resistance (ohms) */
    PetscScalar bat; /* battery (V) */
} Branch;

/* 
  read_data: this routine fills data structures with problem data.
  This can be substituted by an external parser.
*/

#undef __FUNCT__
#define __FUNCT__ "read_data"
void read_data(int *pnnode,int *pnbranch,Node **pnode,Branch **pbranch,int **pedgelist)
{
  PetscInt    nnode, nbranch, i;
  Branch      *branch;
  Node        *node;
  int         *edgelist;

  nnode   = 4;
  nbranch = 6;

  node   = calloc(nnode, sizeof(Node));
  branch = calloc(nbranch, sizeof(Branch));

  for (i = 0; i < nnode; i++) {
      node[i].id  = i;
      node[i].inj = 0;
      node[i].gr = PETSC_FALSE;
  }

  for (i = 0; i < nbranch; i++) {
      branch[i].id  = i;
      branch[i].r   = 1.0;
      branch[i].bat = 0;
  }

  /*
    Branch 1 contains a voltage source of 12.0 V
    From node 0 to 1 there exists a current source of 4.0 A
    Node 3 is grounded, hence the voltage is 0.
  */

  branch[1].bat = 12.0;
  node[0].inj   = -4.0;
  node[1].inj   =  4.0;
  node[3].gr    =  PETSC_TRUE;

  
  /* 
    edgelist is an array of len = 2*nbranch. that defines the
    topology of the network. For branch i we would have that:
      edgelist[2*i] = from node
      edgelist[2*i + 1] = to node
  */

  edgelist = calloc(2*nbranch, sizeof(int));

  for (i = 0; i < nbranch; i++) {
      switch (i) {
          case 0:
            edgelist[2*i]     = 0;
            edgelist[2*i + 1] = 1;
            break;
          case 1:
            edgelist[2*i]     = 0;
            edgelist[2*i + 1] = 2;
            break;
          case 2:
            edgelist[2*i]     = 1;
            edgelist[2*i + 1] = 2;
            break;
          case 3:
            edgelist[2*i]     = 0;
            edgelist[2*i + 1] = 3;
            break;
          case 4:
            edgelist[2*i]     = 1;
            edgelist[2*i + 1] = 3;
            break;
          case 5:
            edgelist[2*i]     = 2;
            edgelist[2*i + 1] = 3;
            break;
          default:
            break;
      }
  }

  /* assign pointers */
  *pnnode    = nnode;
  *pnbranch  = nbranch;
  *pedgelist = edgelist;
  *pbranch   = branch;
  *pnode     = node;
}


#undef __FUNCT__
#define __FUNCT__ "FormOperator"
PetscErrorCode FormOperator(DM networkdm,Mat A,Vec b)
{
  PetscErrorCode    ierr;
  Vec               localb;
  Branch            *branch;
  Node              *node;
  PetscInt          e;
  PetscInt          v,vStart,vEnd;
  PetscInt          eStart, eEnd;
  PetscBool         ghost;
  const PetscInt    *cone;
  PetscScalar       *barr;
  PetscInt          lofst, lofst_to, lofst_fr;
  PetscInt          gofst, gofst_to, gofst_fr;
  PetscInt          compoffset, key;
  PetscInt          row[2],col[6];
  PetscScalar       val[6];
  
  DMNetworkComponentGenericDataType *arr;

  PetscFunctionBegin;
  ierr = DMGetLocalVector(networkdm,&localb);CHKERRQ(ierr);
  ierr = VecSet(b,0.0);CHKERRQ(ierr);
  ierr = VecSet(localb,0.0);CHKERRQ(ierr);
  ierr = MatZeroEntries(A);CHKERRQ(ierr);

  ierr = VecGetArray(localb,&barr);CHKERRQ(ierr);
  
  /* 
    The component data array stores the information which we had in the
    node and branch data structures. We access the correct element  with 
    a variable offset that the DMNetwork provides.
  */
  
  ierr = DMNetworkGetComponentDataArray(networkdm,&arr);CHKERRQ(ierr);


  /* 
    We can define the current as a "edge characteristic" and the voltage
    and the voltage as a "vertex characteristic". With that, we can iterate
    the list of edges and vertices, query the associated voltages and currents
    and use them to write the Kirchoff equations.
  */


  /* Branch equations: i/r + uj - ui = battery */
  
  ierr = DMNetworkGetEdgeRange(networkdm,&eStart,&eEnd);CHKERRQ(ierr);
  for (e = 0; e < eEnd; e++) {
    ierr = DMNetworkGetComponentTypeOffset(networkdm,e,0,&key,&compoffset);CHKERRQ(ierr);
    ierr = DMNetworkGetVariableOffset(networkdm,e,&lofst);CHKERRQ(ierr);
    ierr = DMNetworkGetVariableGlobalOffset(networkdm,e,&gofst);CHKERRQ(ierr);
    
    ierr = DMNetworkGetConnectedNodes(networkdm,e,&cone);CHKERRQ(ierr);
    ierr = DMNetworkGetVariableGlobalOffset(networkdm,cone[0],&gofst_fr);CHKERRQ(ierr);
    ierr = DMNetworkGetVariableGlobalOffset(networkdm,cone[1],&gofst_to);CHKERRQ(ierr);
    ierr = DMNetworkGetVariableOffset(networkdm,cone[0],&lofst_fr);CHKERRQ(ierr);
    ierr = DMNetworkGetVariableOffset(networkdm,cone[1],&lofst_to);CHKERRQ(ierr);

    branch = (Branch*)(arr + compoffset);

    barr[lofst] = branch->bat;

    row[0] = gofst;
    col[0] = gofst;     val[0] =  1;
    col[1] = gofst_to;  val[1] =  1;
    col[2] = gofst_fr;  val[2] = -1;
    ierr = MatSetValues(A,1,row,3,col,val,ADD_VALUES);CHKERRQ(ierr);

    /* from node */
    ierr = DMNetworkGetComponentTypeOffset(networkdm,lofst_fr,0,&key,&compoffset);CHKERRQ(ierr);
    node = (Node*)(arr + compoffset);

    if (!node->gr) {
      row[0] = gofst_fr;
      col[0] = gofst;   val[0] =  1;
      ierr = MatSetValues(A,1,row,1,col,val,ADD_VALUES);CHKERRQ(ierr);
    }

    /* to node */
    ierr = DMNetworkGetComponentTypeOffset(networkdm,lofst_to,0,&key,&compoffset);CHKERRQ(ierr);
    node = (Node*)(arr + compoffset);
    
    if (!node->gr) {
      row[0] = gofst_to;
      col[0] = gofst;   val[0] =  -1;
      ierr = MatSetValues(A,1,row,1,col,val,ADD_VALUES);CHKERRQ(ierr);
    }
  }

  ierr = DMNetworkGetVertexRange(networkdm,&vStart,&vEnd);CHKERRQ(ierr);
  for (v = vStart; v < vEnd; v++) {
    ierr = DMNetworkIsGhostVertex(networkdm,v,&ghost);CHKERRQ(ierr);
    if (!ghost) {
      ierr = DMNetworkGetComponentTypeOffset(networkdm,v,0,&key,&compoffset);CHKERRQ(ierr);
      ierr = DMNetworkGetVariableOffset(networkdm,v,&lofst);CHKERRQ(ierr);
      ierr = DMNetworkGetVariableGlobalOffset(networkdm,v,&gofst);CHKERRQ(ierr);
      node = (Node*)(arr + compoffset);

      if (node->gr) {
        row[0] = gofst;
        col[0] = gofst;   val[0] =  1;
        ierr = MatSetValues(A,1,row,1,col,val,ADD_VALUES);CHKERRQ(ierr);
      } else {
        barr[lofst] -= node->inj;
      }
    }
  }

  ierr = VecRestoreArray(localb,&barr);CHKERRQ(ierr);

  ierr = DMLocalToGlobalBegin(networkdm,localb,ADD_VALUES,b);CHKERRQ(ierr);
  ierr = DMLocalToGlobalEnd(networkdm,localb,ADD_VALUES,b);CHKERRQ(ierr);
  ierr = DMRestoreLocalVector(networkdm,&localb);CHKERRQ(ierr);
  
  ierr = MatAssemblyBegin(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);
  ierr = MatAssemblyEnd(A,MAT_FINAL_ASSEMBLY);CHKERRQ(ierr);

  
  PetscFunctionReturn(0);
}

#undef __FUNCT__
#define __FUNCT__ "main"
int main(int argc,char ** argv)
{
  PetscErrorCode    ierr;
  PetscInt          i, nnode = 0, nbranch = 0;
  PetscInt          eStart, eEnd, vStart, vEnd;
  PetscMPIInt       size, rank;
  DM                networkdm;
  Vec               x, b;
  Mat               A;
  KSP               ksp;
  int               *edgelist;
  int               componentkey[2];
  Node              *node;
  Branch            *branch;

  ierr = PetscInitialize(&argc,&argv,(char*)0,help);if (ierr) return ierr;
  ierr = MPI_Comm_rank(PETSC_COMM_WORLD,&rank);CHKERRQ(ierr);
  ierr = MPI_Comm_size(PETSC_COMM_WORLD,&size);CHKERRQ(ierr);

  /* "read" data only for processor 0 */
  if (!rank) {
    read_data(&nnode, &nbranch, &node, &branch, &edgelist);
  }

  ierr = DMNetworkCreate(PETSC_COMM_WORLD,&networkdm);CHKERRQ(ierr);

  ierr = DMNetworkRegisterComponent(networkdm,"nstr",sizeof(Node),&componentkey[0]);CHKERRQ(ierr);
  ierr = DMNetworkRegisterComponent(networkdm,"bsrt",sizeof(Branch),&componentkey[1]);CHKERRQ(ierr);


  /* Set number of nodes/edges */
  ierr = DMNetworkSetSizes(networkdm,nnode,nbranch,PETSC_DETERMINE,PETSC_DETERMINE);CHKERRQ(ierr);
  /* Add edge connectivity */
  ierr = DMNetworkSetEdgeList(networkdm,edgelist);CHKERRQ(ierr);
  /* Set up the network layout */
  ierr = DMNetworkLayoutSetUp(networkdm);CHKERRQ(ierr);
    
  /* Add network components: physical parameters of nodes and branches*/
  if (!rank) {
    ierr = DMNetworkGetEdgeRange(networkdm,&eStart,&eEnd);CHKERRQ(ierr);
    for (i = eStart; i < eEnd; i++) {
      ierr = DMNetworkAddComponent(networkdm,i,componentkey[0],&branch[i-eStart]);CHKERRQ(ierr);
      ierr = DMNetworkAddNumVariables(networkdm,i,1);CHKERRQ(ierr);
    }

    ierr = DMNetworkGetVertexRange(networkdm,&vStart,&vEnd);CHKERRQ(ierr);
    for (i = vStart; i < vEnd; i++) {
      ierr = DMNetworkAddComponent(networkdm,i,componentkey[1],&node[i-vStart]);CHKERRQ(ierr);
      /* Add number of variables */
      ierr = DMNetworkAddNumVariables(networkdm,i,1);CHKERRQ(ierr);
    }
  }

  /* Set up DM for use */
  ierr = DMSetUp(networkdm);CHKERRQ(ierr);

  if (size > 1) {
    DM distnetworkdm;
    /* Network partitioning and distribution of data */
    ierr = DMNetworkDistribute(networkdm,0,&distnetworkdm);CHKERRQ(ierr);
    ierr = DMDestroy(&networkdm);CHKERRQ(ierr);
    networkdm = distnetworkdm;
  }

  /* We don't use these data structures anymore since they have been copied to networkdm */
  if (!rank) {
    free(edgelist);
    free(node);
    free(branch);
  }


  ierr = DMCreateGlobalVector(networkdm,&x);CHKERRQ(ierr);
  ierr = VecDuplicate(x,&b);CHKERRQ(ierr);

  ierr = DMCreateMatrix(networkdm,&A);CHKERRQ(ierr);
  
  /* Assembly system of equations */
  ierr = FormOperator(networkdm,A,b);CHKERRQ(ierr);

  ierr = KSPCreate(PETSC_COMM_WORLD, &ksp);CHKERRQ(ierr);
  ierr = KSPSetOperators(ksp, A, A);CHKERRQ(ierr);
  ierr = KSPSetFromOptions(ksp);CHKERRQ(ierr);
  ierr = KSPSolve(ksp, b, x);CHKERRQ(ierr);

  ierr = VecDestroy(&x);CHKERRQ(ierr);
  ierr = VecDestroy(&b);CHKERRQ(ierr);
  ierr = MatDestroy(&A);CHKERRQ(ierr);
  ierr = KSPDestroy(&ksp);CHKERRQ(ierr);
  ierr = DMDestroy(&networkdm);CHKERRQ(ierr);
  ierr = PetscFinalize();
  return 0;
}
