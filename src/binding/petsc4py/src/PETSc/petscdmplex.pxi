# --------------------------------------------------------------------

cdef extern from * nogil:

    int DMPlexCreate(MPI_Comm,PetscDM*)
    int DMPlexCreateCohesiveSubmesh(PetscDM,PetscBool,const char[],PetscInt,PetscDM*)
    int DMPlexCreateFromCellListPetsc(MPI_Comm,PetscInt,PetscInt,PetscInt,PetscInt,PetscBool,PetscInt[],PetscInt,PetscReal[],PetscDM*)
    #int DMPlexCreateFromDAG(PetscDM,PetscInt,const PetscInt[],const PetscInt[],const PetscInt[],const PetscInt[],const PetscScalar[])

    int DMPlexGetChart(PetscDM,PetscInt*,PetscInt*)
    int DMPlexSetChart(PetscDM,PetscInt,PetscInt)
    int DMPlexGetConeSize(PetscDM,PetscInt,PetscInt*)
    int DMPlexSetConeSize(PetscDM,PetscInt,PetscInt)
    int DMPlexGetCone(PetscDM,PetscInt,const PetscInt*[])
    int DMPlexSetCone(PetscDM,PetscInt,const PetscInt[])
    int DMPlexInsertCone(PetscDM,PetscInt,PetscInt,PetscInt)
    int DMPlexInsertConeOrientation(PetscDM,PetscInt,PetscInt,PetscInt)
    int DMPlexGetConeOrientation(PetscDM,PetscInt,const PetscInt*[])
    int DMPlexSetConeOrientation(PetscDM,PetscInt,const PetscInt[])
    int DMPlexGetSupportSize(PetscDM,PetscInt,PetscInt*)
    int DMPlexSetSupportSize(PetscDM,PetscInt,PetscInt)
    int DMPlexGetSupport(PetscDM,PetscInt,const PetscInt*[])
    int DMPlexSetSupport(PetscDM,PetscInt,const PetscInt[])
    #int DMPlexInsertSupport(PetscDM,PetscInt,PetscInt,PetscInt)
    #int DMPlexGetConeSection(PetscDM,PetscSection*)
    #int DMPlexGetSupportSection(PetscDM,PetscSection*)
    #int DMPlexGetCones(PetscDM,PetscInt*[])
    #int DMPlexGetConeOrientations(PetscDM,PetscInt*[])
    int DMPlexGetMaxSizes(PetscDM,PetscInt*,PetscInt*)
    int DMPlexSymmetrize(PetscDM)
    int DMPlexStratify(PetscDM)
    #int DMPlexEqual(PetscDM,PetscDM,PetscBool*)
    int DMPlexOrient(PetscDM)
    int DMPlexInterpolate(PetscDM,PetscDM*)
    int DMPlexUninterpolate(PetscDM,PetscDM*)
    #int DMPlexLoad(PetscViewer,PetscDM)
    #int DMPlexSetPreallocationCenterDimension(PetscDM,PetscInt)
    #int DMPlexGetPreallocationCenterDimension(PetscDM,PetscInt*)
    #int DMPlexPreallocateOperator(PetscDM,PetscInt,PetscSection,PetscSection,PetscInt[],PetscInt[],PetscInt[],PetscInt[],Mat,PetscBool)
    int DMPlexGetPointLocal(PetscDM,PetscInt,PetscInt*,PetscInt*)
    #int DMPlexPointLocalRef(PetscDM,PetscInt,PetscScalar*,void*)
    #int DMPlexPointLocalRead(PetscDM,PetscInt,const PetscScalar*,const void*)
    int DMPlexGetPointGlobal(PetscDM,PetscInt,PetscInt*,PetscInt*)
    #int DMPlexPointGlobalRef(PetscDM,PetscInt,PetscScalar*,void*)
    #int DMPlexPointGlobalRead(PetscDM,PetscInt,const PetscScalar*,const void*)
    int DMPlexGetPointLocalField(PetscDM,PetscInt,PetscInt,PetscInt*,PetscInt*)
    int DMPlexGetPointGlobalField(PetscDM,PetscInt,PetscInt,PetscInt*,PetscInt*)
    int DMPlexCreateClosureIndex(PetscDM,PetscSection)
    #int PetscSectionCreateGlobalSectionLabel(PetscSection,PetscSF,PetscBool,PetscDMLabel,PetscInt,PetscSection*)

    int DMPlexGetCellNumbering(PetscDM,PetscIS*)
    int DMPlexGetVertexNumbering(PetscDM,PetscIS*)
    int DMPlexCreatePointNumbering(PetscDM,PetscIS*)

    int DMPlexGetDepth(PetscDM,PetscInt*)
    #int DMPlexGetDepthLabel(PetscDM,PetscDMLabel*)
    int DMPlexGetDepthStratum(PetscDM,PetscInt,PetscInt*,PetscInt*)
    int DMPlexGetHeightStratum(PetscDM,PetscInt,PetscInt*,PetscInt*)

    int DMPlexGetMeet(PetscDM,PetscInt,const PetscInt[],PetscInt*,const PetscInt**)
    #int DMPlexGetFullMeet(PetscDM,PetscInt,const PetscInt[],PetscInt*,const PetscInt**)
    int DMPlexRestoreMeet(PetscDM,PetscInt,const PetscInt[],PetscInt*,const PetscInt**)
    int DMPlexGetJoin(PetscDM,PetscInt,const PetscInt[],PetscInt*,const PetscInt**)
    #int DMPlexGetFullJoin(PetscDM,PetscInt,const PetscInt[],PetscInt*,const PetscInt**)
    int DMPlexRestoreJoin(PetscDM,PetscInt,const PetscInt[],PetscInt*,const PetscInt**)
    int DMPlexGetTransitiveClosure(PetscDM,PetscInt,PetscBool,PetscInt*,PetscInt*[])
    int DMPlexRestoreTransitiveClosure(PetscDM,PetscInt,PetscBool,PetscInt*,PetscInt*[])
    int DMPlexVecGetClosure(PetscDM,PetscSection,PetscVec,PetscInt,PetscInt*,PetscScalar*[])
    int DMPlexVecRestoreClosure(PetscDM,PetscSection,PetscVec,PetscInt,PetscInt*,PetscScalar*[])
    int DMPlexVecSetClosure(PetscDM,PetscSection,PetscVec,PetscInt,PetscScalar[],PetscInsertMode)
    int DMPlexMatSetClosure(PetscDM,PetscSection,PetscSection,PetscMat,PetscInt,PetscScalar[],PetscInsertMode)

    int DMPlexGenerate(PetscDM,const char[],PetscBool ,PetscDM*)
    int DMPlexTriangleSetOptions(PetscDM,const char*)
    int DMPlexTetgenSetOptions(PetscDM,const char*)
    #int DMPlexCopyCoordinates(PetscDM,PetscDM)
    #int DMPlexCreateDoublet(MPI_Comm,PetscInt,PetscBool,PetscBool,PetscBool,PetscReal,PetscDM*)
    int DMPlexCreateBoxMesh(MPI_Comm,PetscInt,PetscBool,PetscInt[],PetscReal[],PetscReal[],PetscDMBoundaryType[],PetscBool,PetscDM*)
    int DMPlexCreateBoxSurfaceMesh(MPI_Comm,PetscInt,PetscInt[],PetscReal[],PetscReal[],PetscBool,PetscDM*)
    int DMPlexCreateFromFile(MPI_Comm,const char[],PetscBool,PetscDM*)
    int DMPlexCreateCGNS(MPI_Comm,PetscInt,PetscBool,PetscDM*)
    int DMPlexCreateCGNSFromFile(MPI_Comm,const char[],PetscBool,PetscDM*)
    int DMPlexCreateExodus(MPI_Comm,PetscInt,PetscBool,PetscDM*)
    int DMPlexCreateExodusFromFile(MPI_Comm,const char[],PetscBool,PetscDM*)
    int DMPlexCreateGmsh(MPI_Comm,PetscViewer,PetscBool,PetscDM*)

    #int DMPlexCreateConeSection(PetscDM,PetscSection*)
    #int DMPlexInvertCell(PetscInt,PetscInt,int[])
    #int DMPlexCheckSymmetry(PetscDM)
    #int DMPlexCheckSkeleton(PetscDM,PetscBool,PetscInt)
    #int DMPlexCheckFaces(PetscDM,PetscBool,PetscInt)

    int DMPlexSetAdjacencyUseAnchors(PetscDM,PetscBool)
    int DMPlexGetAdjacencyUseAnchors(PetscDM,PetscBool*)
    int DMPlexGetAdjacency(PetscDM,PetscInt,PetscInt*,PetscInt*[])
    #int DMPlexCreateNeighborCSR(PetscDM,PetscInt,PetscInt*,PetscInt**,PetscInt**)
    int DMPlexRebalanceSharedPoints(PetscDM,PetscInt,PetscBool,PetscBool,PetscBool*)
    int DMPlexDistribute(PetscDM,PetscInt,PetscSF*,PetscDM*)
    int DMPlexDistributeOverlap(PetscDM,PetscInt,PetscSF*,PetscDM*)
    int DMPlexSetPartitioner(PetscDM,PetscPartitioner)
    int DMPlexGetPartitioner(PetscDM,PetscPartitioner*)
    int DMPlexDistributeField(PetscDM,PetscSF,PetscSection,PetscVec,PetscSection,PetscVec)
    #int DMPlexDistributeData(PetscDM,PetscSF,PetscSection,MPI_Datatype,void*,PetscSection,void**)
    int DMPlexIsDistributed(PetscDM,PetscBool*)

    int DMPlexGetOrdering(PetscDM,PetscMatOrderingType,PetscDMLabel,PetscIS*)
    int DMPlexPermute(PetscDM,PetscIS,PetscDM*)

    #int DMPlexCreateSubmesh(PetscDM,PetscDMLabel,PetscInt,PetscDM*)
    #int DMPlexCreateHybridMesh(PetscDM,PetscDMLabel,PetscDMLabel*,PetscDM*)
    #int DMPlexGetSubpointMap(PetscDM,PetscDMLabel*)
    #int DMPlexSetSubpointMap(PetscDM,PetscDMLabel)
    #int DMPlexCreateSubpointIS(PetscDM,PetscIS*)

    int DMPlexCreateCoarsePointIS(PetscDM,PetscIS*)
    int DMPlexMarkBoundaryFaces(PetscDM,PetscInt,PetscDMLabel)
    #int DMPlexLabelComplete(PetscDM,PetscDMLabel)
    #int DMPlexLabelCohesiveComplete(PetscDM,PetscDMLabel,PetscBool,PetscDM)

    int DMPlexGetRefinementLimit(PetscDM,PetscReal*)
    int DMPlexSetRefinementLimit(PetscDM,PetscReal)
    int DMPlexGetRefinementUniform(PetscDM,PetscBool*)
    int DMPlexSetRefinementUniform(PetscDM,PetscBool)

    #int DMPlexGetNumFaceVertices(PetscDM,PetscInt,PetscInt,PetscInt*)
    #int DMPlexGetOrientedFace(PetscDM,PetscInt,PetscInt,const PetscInt[],PetscInt,PetscInt[],PetscInt[],PetscInt[],PetscBool*)

    int DMPlexCreateSection(PetscDM,PetscDMLabel[],const PetscInt[],const PetscInt[],PetscInt,const PetscInt[],const PetscIS[],const PetscIS[],PetscIS,PetscSection*)

    int DMPlexComputeCellGeometryFVM(PetscDM,PetscInt,PetscReal*,PetscReal[],PetscReal[])
    int DMPlexConstructGhostCells(PetscDM,const char[],PetscInt*,PetscDM*)

    int DMPlexTopologyView(PetscDM,PetscViewer)

    int DMPlexTopologyLoad(PetscDM,PetscViewer,PetscSF*)
    int DMPlexCoordinatesLoad(PetscDM,PetscViewer)
    int DMPlexLabelsLoad(PetscDM,PetscViewer)
