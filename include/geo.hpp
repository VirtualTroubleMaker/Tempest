/*!
 * \file geo.hpp
 * \brief Header file for geometry class
 *
 * \author - Jacob Crabill
 *           Aerospace Computing Laboratory (ACL)
 *           Aero/Astro Department. Stanford University
 *
 * \version 0.0.1
 *
 * Flux Reconstruction in C++ (Flurry++) Code
 * Copyright (C) 2014 Jacob Crabill.
 *
 */
#pragma once

#include <array>
#include <string>
#include <vector>

#include "global.hpp"

//class tioga;

#include "input.hpp"
#include "solver.hpp"
//#include "tioga.h"

class geo
{
public:
  geo();

  ~geo();

  /* === Primay setup routines === */

  //! Setup the geomery using input parameters
  void setup(input* params);

  //! Take the basic connectivity data and generate the rest
  void processConnectivity();

  /* === Helper Routines === */

  //! Read essential connectivity from a Gmsh mesh file
  void readGmsh(string fileName);

  //! Create a simple Cartesian mesh from input parameters
  void createMesh();

  //! Get the reference-domain location of the solution points for the given element & polynomial order
  vector<point> getLocSpts(int eType, int order);

  //! Get the reference-domain location of the flux points for the given element & polynomial order
  vector<point> getLocFpts(int eType, int order);

  //! Get the point locations of the requested type (i.e. Gauss, Lobatto) for the given order
  vector<double> getPts1D(string ptsType, int order);

  //! Get the Gauss quadrature weights for the Gauss points of the given order [2D]
  vector<double> getQptWeights(int order);

  //! Get the Gauss quadrature weights for the Gauss points of the given order [1D]
  vector<double> getQptWeights1D(int order);

  //! Update connectivity / node-blanking for overset grids
  void registerGridDataTIOGA();

  /*!
   * \brief Call TIOGA to re-process overset connectivity
   *
   * Called once during pre-processing by default; re-call each iteration
   * for moving-mesh cases
   */
  void updateOversetConnectivity();

  //! Have TIOGA output the mesh along with nodal IBLANK values
  void writeOversetConnectivity();

  int nDims, nFields;
  int nEles, nVerts, nEdges, nFaces, nIntFaces, nBndFaces, nMpiFaces;
  int nBounds;  //! Number of boundaries
  int meshType;

  // Basic [essential] Connectivity Data
  matrix<int> c2v;
  matrix<double> xv;

  // Additional Connectivity Data
  matrix<int> c2e, c2b, e2c, e2v, v2e, v2v, v2c;
  matrix<int> c2f, f2v, f2c, c2c;
  vector<int> v2nv, v2nc, c2nv, c2nf, c2ne, c2nc, e2nc, f2nv, ctype;
  vector<int> v2b;               //! Does vertex lie on a boundary? (0 or 1)
  vector<double> e2A;            //! Dual-mesh face area for each edge
  vector<double> v2vol;          //! Dual-mesh element volumes for each vertex
  vector<int> intFaces, bndFaces, mpiFaces, mpiCells;
  vector<int> nBcFaces;          //! Number of faces on each boundary
  vector<vector<int>> bcFaceList; //! Global face IDs of faces on each boundary
  Array<Vec3,2> bndNorm;         //! Outward unit normal for each boundary point
  matrix<double> bndArea;        //! Face area for each boundary point
  vector<point> c2xc;            //! Centroid of each cell in mesh (for creation of dual mesh)
  vector<int> bcList;            //! List of boundary conditions for each boundary
  vector<int> bcTypeF;           //! Boundary condition for each boundary face
  vector<int> bcTypeE;           //! Boundary condition for each boundary edge
  matrix<int> bndPts;            //! List of node IDs on each boundary
  vector<int> nBndPts;           //! Number of points on each boudary
  vector<matrix<int> > bcFaces;  //! List of nodes on each face (edge) for each boundary condition
  vector<int> nFacesPerBnd;      //! List of # of faces on each boundary
  vector<int> procR;             //! What processor lies to the 'right' of this face
  vector<int> locF_R;            //! The local mpiFace ID of each mpiFace on the opposite processor
  vector<int> gIC_R;             //! The global cell ID of the right cell on the opposite processor
  vector<int> mpiLocF;           //! Element-local face ID of MPI Face in left cell
  vector<int> mpiLocF_R;         //! Element-local face ID of MPI Face in right cell
  vector<bool> isBnd; // might want to change this to "int" and have it store WHICH boundary the face is on (-1 for internal)
  vector<int> isBndEdge;         //! For each edge, flag for "normal" or "boundary"

  /* --- Overset-Related Variables --- */
  int nprocPerGrid;       //! Number of MPI processes assigned to each (overset) grid block
  int gridID;             //! Which (overset) grid block is this process handling
  int gridRank;           //! MPI rank of process *within* the grid block [0 to nprocPerGrid-1]
  vector<int> iblank;     //! Output of TIOGA: flag for whether vertex is normal, blanked, or receptor
  vector<int> iblankCell; //! Output? of TIOGA: flag for whether cell is normal, blanked, or receptor
  vector<int> iwall;      //! List of nodes on wall boundaries
  vector<int> iover;      //! List of nodes on overset boundaries

  //tioga* tg;           //! Pointer to Tioga object for processing overset grids
  int* nodesPerCell;   //! Pointer for Tioga to know # of nodes for each element type
  array<int*,1> conn;  //! Pointer to c2v for each element type [but only 1, so will be size(1)]

  Vec3 getFaceNormalTri(int faceID);
  Vec3 getFaceNormalQuad(int faceID);
private:

  input *params;

  /* --- MPI-Related Varialbes (global vs. local data) --- */
  matrix<int> c2v_g;    //! Global element connectivity
  matrix<double> xv_g;  //! Global mesh node locations
  vector<int> ic2icg;   //! Local cell to global cell index
  vector<int> iv2ivg;   //! Local vertex to global vertex index
  vector<int> ctype_g, c2ne_g, c2nv_g; //! Global element info
  matrix<int> bndPts_g;  //! Global lists of points on boundaries
  vector<int> nBndPts_g; //! Global number of points on each boundary
  map<int,int> bcIdMap;  //! Map from Gmsh boundary ID to Flurry BC ID
  int nEles_g, nVerts_g;

#ifndef _NO_MPI
  MPI_Comm gridComm;
#endif

  void processConnEdges(void);
  void processConnFaces(void);
  void processConnDual(void);

  //! Match up pairs of periodic boundary faces
  void processPeriodicBoundaries(void);

  //! Check if two given periodic edges match up
  bool checkPeriodicFaces(int *edge1, int *edge2);
  bool checkPeriodicFaces3D(vector<int> &face1, vector<int> &face2);

  //! Compare the orientation (rotation in ref. space) betwen the local faces of 2 elements
  int compareOrientation(int ic1, int ic2, int f1, int f2);

  //! Compare the orientation (rotation in ref. space) betwen the local faces of 2 elements across MPI boundary
  int compareOrientationMPI(int ic1, int ic2, int f1, int f2);

  //! For MPI runs, partition the mesh across all processors
  void partitionMesh(void);

  //! For MPI runs, match internal faces across MPI boundaries
  void matchMPIFaces();

  //! Compare two faces [lists of nodes] to see if they match [used for MPI]
  bool compareFaces(vector<int> &face1, vector<int> &face2);
};
