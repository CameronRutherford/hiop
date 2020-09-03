// Copyright (c) 2017, Lawrence Livermore National Security, LLC.
// Produced at the Lawrence Livermore National Laboratory (LLNL).
// Written by Cosmin G. Petra, petra1@llnl.gov.
// LLNL-CODE-742473. All rights reserved.
//
// This file is part of HiOp. For details, see https://github.com/LLNL/hiop. HiOp 
// is released under the BSD 3-clause license (https://opensource.org/licenses/BSD-3-Clause). 
// Please also read “Additional BSD Notice” below.
//
// Redistribution and use in source and binary forms, with or without modification, 
// are permitted provided that the following conditions are met:
// i. Redistributions of source code must retain the above copyright notice, this list 
// of conditions and the disclaimer below.
// ii. Redistributions in binary form must reproduce the above copyright notice, 
// this list of conditions and the disclaimer (as noted below) in the documentation and/or 
// other materials provided with the distribution.
// iii. Neither the name of the LLNS/LLNL nor the names of its contributors may be used to 
// endorse or promote products derived from this software without specific prior written 
// permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
// EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
// SHALL LAWRENCE LIVERMORE NATIONAL SECURITY, LLC, THE U.S. DEPARTMENT OF ENERGY OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS 
// OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED 
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, 
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Additional BSD Notice
// 1. This notice is required to be provided under our contract with the U.S. Department 
// of Energy (DOE). This work was produced at Lawrence Livermore National Laboratory under 
// Contract No. DE-AC52-07NA27344 with the DOE.
// 2. Neither the United States Government nor Lawrence Livermore National Security, LLC 
// nor any of their employees, makes any warranty, express or implied, or assumes any 
// liability or responsibility for the accuracy, completeness, or usefulness of any 
// information, apparatus, product, or process disclosed, or represents that its use would
// not infringe privately-owned rights.
// 3. Also, reference herein to any specific commercial products, process, or services by 
// trade name, trademark, manufacturer or otherwise does not necessarily constitute or 
// imply its endorsement, recommendation, or favoring by the United States Government or 
// Lawrence Livermore National Security, LLC. The views and opinions of authors expressed 
// herein do not necessarily state or reflect those of the United States Government or 
// Lawrence Livermore National Security, LLC, and shall not be used for advertising or 
// product endorsement purposes.

/**
 * @file hiopMatrixSparse.cpp
 *
 */
#pragma once

#include "hiopVector.hpp"
#include "hiopMatrixDense.hpp"

#include <cassert>

namespace hiop
{

/**
 * @brief Sparse matrix of doubles in triplet format - it is not distributed
 * @note for now (i,j) are expected ordered: first on rows 'i' and then on cols 'j'
 */
class hiopMatrixSparse : public hiopMatrix
{
public:
  hiopMatrixSparse(int rows, int cols, int nnz)
      : nrows(rows)
      , ncols(cols)
      , nnz(nnz)
  {
  }
  virtual ~hiopMatrixSparse()
  {
  }

  virtual void setToZero() = 0;
  virtual void setToConstant(double c) = 0;
  virtual void copyFrom(const hiopMatrixSparse& dm) = 0;

  virtual void copyRowsFrom(const hiopMatrix& src, const long long* rows_idxs, long long n_rows) = 0;

  virtual void timesVec(double beta, hiopVector& y, double alpha, const hiopVector& x) const = 0;
  virtual void timesVec(double beta, double* y, double alpha, const double* x) const = 0;

  virtual void transTimesVec(double beta, hiopVector& y, double alpha, const hiopVector& x) const = 0;
  virtual void transTimesVec(double beta, double* y, double alpha, const double* x) const = 0;

  virtual void timesMat(double beta, hiopMatrix& W, double alpha, const hiopMatrix& X) const = 0;

  virtual void transTimesMat(double beta, hiopMatrix& W, double alpha, const hiopMatrix& X) const = 0;

  virtual void timesMatTrans(double beta, hiopMatrix& W, double alpha, const hiopMatrix& X) const = 0;

  virtual void addDiagonal(const double& alpha, const hiopVector& d_) = 0;
  virtual void addDiagonal(const double& value) = 0;
  virtual void addSubDiagonal(const double& alpha, long long start, const hiopVector& d_) = 0;
  /* add to the diagonal of 'this' (destination) starting at 'start_on_dest_diag' elements of
   * 'd_' (source) starting at index 'start_on_src_vec'. The number of elements added is 'num_elems'
   * when num_elems>=0, or the remaining elems on 'd_' starting at 'start_on_src_vec'. */
  virtual void addSubDiagonal(int start_on_dest_diag, const double& alpha, const hiopVector& d_,
    int start_on_src_vec, int num_elems = -1)
  {
    assert(false && "not needed / implemented");
  }
  virtual void addSubDiagonal(int start_on_dest_diag, int num_elems, const double& c)
  {
    assert(false && "not needed / implemented");
  }

  virtual void addMatrix(double alpha, const hiopMatrix& X) = 0;

  /* block of W += alpha*this */
  virtual void addToSymDenseMatrixUpperTriangle(
    int row_dest_start, int col_dest_start, double alpha, hiopMatrixDense& W) const = 0;
  /* block of W += alpha*transpose(this) */
  virtual void transAddToSymDenseMatrixUpperTriangle(
    int row_dest_start, int col_dest_start, double alpha, hiopMatrixDense& W) const = 0;
  virtual void addUpperTriangleToSymDenseMatrixUpperTriangle(
    int diag_start, double alpha, hiopMatrixDense& W) const = 0;

  /* diag block of W += alpha * M * D^{-1} * transpose(M), where M=this
   *
   * Only the upper triangular entries of W are updated.
   */
  virtual void addMDinvMtransToDiagBlockOfSymDeMatUTri(
    int rowCol_dest_start, const double& alpha, const hiopVector& D, hiopMatrixDense& W) const = 0;

  /* block of W += alpha * M * D^{-1} * transpose(N), where M=this
   *
   * Warning: The product matrix M * D^{-1} * transpose(N) with start offsets 'row_dest_start' and
   * 'col_dest_start' needs to fit completely in the upper triangle of W. If this is NOT the
   * case, the method will assert(false) in debug; in release, the method will issue a
   * warning with HIOP_DEEPCHECKS (otherwise NO warning will be issue) and will silently update
   * the (strictly) lower triangular  elements (these are ignored later on since only the upper
   * triangular part of W will be accessed)
   */
  virtual void addMDinvNtransToSymDeMatUTri(int row_dest_start, int col_dest_start,
    const double& alpha, const hiopVector& D, const hiopMatrixSparse& N, hiopMatrixDense& W) const = 0;

  virtual double max_abs_value() = 0;

  virtual bool isfinite() const = 0;

  // virtual void print(int maxRows=-1, int maxCols=-1, int rank=-1) const;
  virtual void print(FILE* f = NULL, const char* msg = NULL, int maxRows = -1, int maxCols = -1,
    int rank = -1) const = 0;

  /* extract subdiagonal from 'this' (source) and adds the entries to 'vec_dest' starting at
   * index 'vec_start'. If num_elems>=0, 'num_elems' are copied; otherwise copies as many as
   * are available in 'vec_dest' starting at 'vec_start'
   */
  virtual void startingAtAddSubDiagonalToStartingAt(int diag_src_start, const double& alpha, 
					    hiopVector& vec_dest, int vec_start, int num_elems=-1) const = 0;
					    

  virtual hiopMatrixSparse* alloc_clone() const = 0;
  virtual hiopMatrixSparse* new_copy() const = 0;

  virtual int* i_row() = 0;
  virtual int* j_col() = 0;
  virtual double* M()  = 0;
  virtual const int* i_row() const = 0;
  virtual const int* j_col() const = 0;
  virtual const double* M()  const = 0;

  inline long long m() const
  {
    return nrows;
  }
  inline long long n() const
  {
    return ncols;
  }
  inline long long numberOfNonzeros() const
  {
    return nnz;
  }

#ifdef HIOP_DEEPCHECKS
  virtual bool assertSymmetry(double tol = 1e-16) const
  {
    return false;
  }
  virtual bool checkIndexesAreOrdered() const = 0;
#endif
protected:
  int nrows;   ///< number of rows
  int ncols;   ///< number of columns
  int nnz;     ///< number of nonzero entries
};

}   // namespace hiop
