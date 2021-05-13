/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2020 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file simplex/QFactor.h
 * @brief Basis matrix factorization, update and solves for HiGHS
 * @author Julian Hall, Ivet Galabova, Qi Huangfu and Michael Feldmeier
 */
#ifndef __SRC_LIB_QFactor_H__
#define __SRC_LIB_QFactor_H__

#include <algorithm>
#include <cmath>
#include <vector>

#include "vector.hpp"

using std::max;
using std::min;
using std::vector;

enum UPDATE_METHOD_ {
  UPDATE_METHOD__FT = 1,
  UPDATE_METHOD__PF = 2,
  UPDATE_METHOD__MPF = 3,
  UPDATE_METHOD__APF = 4
};
/**
 * Necessary threshholds for historical density to trigger
 * hyper-sparse TRANs,
 */
const double hyperFTRANL_ = 0.15;
const double hyperFTRANU_ = 0.10;
const double hyperBTRANL_ = 0.10;
const double hyperBTRANU_ = 0.15;
/**
 * Necessary threshhold for RHS density to trigger hyper-sparse TRANs,
 */
const double hyperCANCEL_ = 0.05;
/**
 * Threshhold for result density for it to be considered as
 * hyper-sparse - only for reporting
 */
const double hyperRESULT_ = 0.10;
/**
 * @brief Basis matrix factorization, update and solves for HiGHS
 *
 * Class for the following
 *
 * Basis matrix factorization \f$PBQ=LU\f$
 *
 * Update according to \f$B'=B+(\mathbf{a}_q-B\mathbf{e}_p)\mathbf{e}_p^T\f$
 *
 * Solves \f$B\mathbf{x}=\mathbf{b}\f$ (FTRAN) and
 * \f$B^T\mathbf{x}=\mathbf{b}\f$ (BTRAN)
 *
 * QFactor is initialised using QFactor::setup, which takes copies of
 * the pointers to the constraint matrix starts, indices, values and
 * basic column indices.
 *
 * Forming \f$PBQ=LU\f$ (INVERT) is performed using QFactor::build
 *
 * Solving \f$B\mathbf{x}=\mathbf{b}\f$ (FTRAN) is performed using
 * QFactor::ftran
 *
 * Solving \f$B^T\mathbf{x}=\mathbf{b}\f$ (BTRAN) is performed using
 * QFactor::btran
 *
 * Updating the invertible representation of the basis matrix
 * according to \f$B'=B+(\mathbf{a}_q-B\mathbf{e}_p)\mathbf{e}_p^T\f$
 * is performed by QFactor::update. UPDATE requires vectors
 * \f$B^{-1}\mathbf{a}_q\f$ and \f$B^{-T}\mathbf{e}_q\f$, together
 * with the index of the pivotal row.
 *
 * QFactor assumes that the basic column indices are kept up-to-date
 * externally as basis changes take place. INVERT permutes the basic
 * column indices, since these define the order of the solution values
 * after FTRAN, and the assumed order of the RHS before BTRAN
 *
 */
class QFactor {
 public:
  /**
   * @brief Copy problem size and pointers of constraint matrix, and set
   * up space for INVERT
   *
   * Copy problem size and pointers to coefficient matrix, allocate
   * working buffer for INVERT, allocate space for basis matrix, L, U
   * factor and Update buffer, allocated space for Markowitz matrices,
   * count-link-list, L factor and U factor
   */
  void setup(int numCol,            //!< Number of columns
             int numRow,            //!< Number of rows
             const int* Astart,     //!< Column starts of constraint matrix
             const int* Aindex,     //!< Row indices of constraint matrix
             const double* Avalue,  //!< Row values of constraint matrix
             int* baseIndex,        //!< Indices of basic variables
             const bool use_original_QFactor_logic = true,
             int updateMethod =
                 UPDATE_METHOD__FT  //!< Default update method is Forrest Tomlin
  );

#ifdef HiGHSDEV
  /**
   * @brief Change the update method
   *
   * Only called in HighsSimplexInterface::change_UPDATE_METHOD_, which
   * is only called in HTester.cpp Should only be compiled when
   * HiGHSDEV=on
   */
  void change(int updateMethod  //!< New update method
  );
#endif

  /**
   * @brief Form \f$PBQ=LU\f$ for basis matrix \f$B\f$ or report degree of rank
   * deficiency.
   *
   * @return 0 if successful, otherwise rankDeficiency>0
   *
   */
  int build();

  /**
   * @brief Solve \f$B\mathbf{x}=\mathbf{b}\f$ (FTRAN)
   */
  void ftran(Vector& vector,            //!< RHS vector \f$\mathbf{b}\f$
             double historical_density  //!< Historical density of the result
             ) const;

  /**
   * @brief Solve \f$B^T\mathbf{x}=\mathbf{b}\f$ (BTRAN)
   */
  void btran(Vector& vector,            //!< RHS vector \f$\mathbf{b}\f$
             double historical_density  //!< Historical density of the result
             ) const;

  /**
   * @brief Update according to
   * \f$B'=B+(\mathbf{a}_q-B\mathbf{e}_p)\mathbf{e}_p^T\f$
   */
  void update(Vector* aq,  //!< Vector \f$B^{-1}\mathbf{a}_q\f$
              Vector* ep,  //!< Vector \f$B^{-T}\mathbf{e}_p\f$
              int* iRow,    //!< Index of pivotal row
              int* hint     //!< Reinversion status
  );

#ifdef HiGHSDEV
  /**
   * @brief Data used for reporting in HTester.cpp. Should only be
   * compiled when HiGHSDEV=on
   */
  int BtotalX;

  /**
   * @brief Data used for reporting in HTester.cpp. Should only be
   * compiled when HiGHSDEV=on
   */
  int FtotalX;
#endif

  /**
   * @brief Wall clock time for INVERT
   */
  double build_realTick;

  /**
   * @brief The synthetic clock for INVERT
   */
  double build_syntheticTick;

  // Rank deficiency information

  /**
   * @brief Degree of rank deficiency in \f$B\f$
   */
  int rank_deficiency;

  /**
   * @brief Rows not pivoted on
   */
  vector<int> noPvR;

  /**
   * @brief Columns not pivoted on
   */
  vector<int> noPvC;

  /**
   * @brief Gets noPvR when QFactor.h cannot be included
   */
  vector<int>& getNoPvR() { return noPvR; }

  /**
   * @brief Gets noPvC when QFactor.h cannot be included
   */
  const int* getNoPvC() const { return &noPvC[0]; }

  // TODO Understand why handling noPvC and noPvR in what seem to be
  // different ways ends up equivalent.
  //  vector<int>& getNoPvC() {return noPvC;}

#ifdef HiGHSDEV
  /**
   * @brief Checks \f$B^{-1}\mathbf{a}_i=\mathbf{e}_i\f$ for each column \f$i\f$
   *
   * Should only be compiled when HiGHSDEV=on
   */
  void checkInvert();
#endif

  // Properties of data held in QFactor.h. To "have" them means that
  // they are assigned.
  int haveArrays;
  // The representation of B^{-1} corresponds to the current basis
  int haveInvert;
  // The representation of B^{-1} corresponds to the current basis and is fresh
  int haveFreshInvert;
  int basis_matrix_num_el = 0;
  int invert_num_el = 0;
  int kernel_dim = 0;
  int kernel_num_el = 0;

  /**
   * Data of the factor
   */

  // private:
  // Problem size, coefficient matrix and update method
  int numRow;
  int numCol;

 private:
  const int* Astart;
  const int* Aindex;
  const double* Avalue;
  int* baseIndex;
  int updateMethod;
  bool use_original_QFactor_logic;

  // Working buffer
  int nwork;
  vector<int> iwork;
  vector<double> dwork;

  // Basis matrix
  vector<int> Bstart;
  vector<int> Bindex;
  vector<double> Bvalue;

  // Permutation
  vector<int> permute;

  // Kernel matrix
  vector<int> MCstart;
  vector<int> MCcountA;
  vector<int> MCcountN;
  vector<int> MCspace;
  vector<int> MCindex;
  vector<double> MCvalue;
  vector<double> MCminpivot;

  // Row wise kernel matrix
  vector<int> MRstart;
  vector<int> MRcount;
  vector<int> MRspace;
  vector<int> MRcountb4;
  vector<int> MRindex;

  // Kernel column buffer
  vector<int> McolumnIndex;
  vector<char> McolumnMark;
  vector<double> McolumnArray;

  // Count link list
  vector<int> clinkFirst;
  vector<int> clinkNext;
  vector<int> clinkLast;

  vector<int> rlinkFirst;
  vector<int> rlinkNext;
  vector<int> rlinkLast;

  // Factor L
  vector<int> LpivotLookup;
  vector<int> LpivotIndex;

  vector<int> Lstart;
  vector<int> Lindex;
  vector<double> Lvalue;
  vector<int> LRstart;
  vector<int> LRindex;
  vector<double> LRvalue;

  // Factor U
  vector<int> UpivotLookup;
  vector<int> UpivotIndex;
  vector<double> UpivotValue;

  int UmeritX;
  int UtotalX;
  vector<int> Ustart;
  vector<int> Ulastp;
  vector<int> Uindex;
  vector<double> Uvalue;
  vector<int> URstart;
  vector<int> URlastp;
  vector<int> URspace;
  vector<int> URindex;
  vector<double> URvalue;

  // Update buffer
  vector<double> PFpivotValue;
  vector<int> PFpivotIndex;
  vector<int> PFstart;
  vector<int> PFindex;
  vector<double> PFvalue;

  // Implementation
  void buildSimple();
  //    void buildKernel();
  int buildKernel();
  void buildHandleRankDeficiency();
  void buildRpRankDeficiency();
  void buildMarkSingC();
  void buildFinish();

  void ftranL(Vector& vector, double historical_density
              ) const;
  void btranL(Vector& vector, double historical_density
              ) const;
  void ftranU(Vector& vector, double historical_density
              ) const;
  void btranU(Vector& vector, double historical_density
              ) const;

  void ftranFT(Vector& vector) const;
  void btranFT(Vector& vector) const;
  void ftranPF(Vector& vector) const;
  void btranPF(Vector& vector) const;
  void ftranMPF(Vector& vector) const;
  void btranMPF(Vector& vector) const;
  void ftranAPF(Vector& vector) const;
  void btranAPF(Vector& vector) const;

  // void updateCFT(Vector* aq, Vector* ep, int* iRow);  //, int* hint); // TODO
  void updateFT(Vector* aq, Vector* ep, int iRow);    //, int* hint);
  void updatePF(Vector* aq, int iRow, int* hint);
  void updateMPF(Vector* aq, Vector* ep, int iRow, int* hint);
  void updateAPF(Vector* aq, Vector* ep, int iRow);  //, int* hint);

  /**
   * Local in-line functions
   */
  void colInsert(const int iCol, const int iRow, const double value) {
    const int iput = MCstart[iCol] + MCcountA[iCol]++;
    MCindex[iput] = iRow;
    MCvalue[iput] = value;
  }
  void colStoreN(const int iCol, const int iRow, const double value) {
    const int iput = MCstart[iCol] + MCspace[iCol] - (++MCcountN[iCol]);
    MCindex[iput] = iRow;
    MCvalue[iput] = value;
  }
  void colFixMax(const int iCol) {
    double maxValue = 0;
    for (int k = MCstart[iCol]; k < MCstart[iCol] + MCcountA[iCol]; k++)
      maxValue = max(maxValue, fabs(MCvalue[k]));
    MCminpivot[iCol] = maxValue * 0.1;
  }

  double colDelete(const int iCol, const int iRow) {
    int idel = MCstart[iCol];
    int imov = idel + (--MCcountA[iCol]);
    while (MCindex[idel] != iRow) idel++;
    double pivotX = MCvalue[idel];
    MCindex[idel] = MCindex[imov];
    MCvalue[idel] = MCvalue[imov];
    return pivotX;
  }

  void rowInsert(const int iCol, const int iRow) {
    int iput = MRstart[iRow] + MRcount[iRow]++;
    MRindex[iput] = iCol;
  }

  void rowDelete(const int iCol, const int iRow) {
    int idel = MRstart[iRow];
    int imov = idel + (--MRcount[iRow]);
    while (MRindex[idel] != iCol) idel++;
    MRindex[idel] = MRindex[imov];
  }

  void clinkAdd(const int index, const int count) {
    const int mover = clinkFirst[count];
    clinkLast[index] = -2 - count;
    clinkNext[index] = mover;
    clinkFirst[count] = index;
    if (mover >= 0) clinkLast[mover] = index;
  }

  void clinkDel(const int index) {
    const int xlast = clinkLast[index];
    const int xnext = clinkNext[index];
    if (xlast >= 0)
      clinkNext[xlast] = xnext;
    else
      clinkFirst[-xlast - 2] = xnext;
    if (xnext >= 0) clinkLast[xnext] = xlast;
  }

  void rlinkAdd(const int index, const int count) {
    const int mover = rlinkFirst[count];
    rlinkLast[index] = -2 - count;
    rlinkNext[index] = mover;
    rlinkFirst[count] = index;
    if (mover >= 0) rlinkLast[mover] = index;
  }

  void rlinkDel(const int index) {
    const int xlast = rlinkLast[index];
    const int xnext = rlinkNext[index];
    if (xlast >= 0)
      rlinkNext[xlast] = xnext;
    else
      rlinkFirst[-xlast - 2] = xnext;
    if (xnext >= 0) rlinkLast[xnext] = xlast;
  }
};

#endif /* QFactor_H_ */