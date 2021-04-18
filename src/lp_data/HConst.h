/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                       */
/*    This file is part of the HiGHS linear optimization suite           */
/*                                                                       */
/*    Written and engineered 2008-2021 at the University of Edinburgh    */
/*                                                                       */
/*    Available as open-source under the MIT License                     */
/*                                                                       */
/*    Authors: Julian Hall, Ivet Galabova, Qi Huangfu, Leona Gottwald    */
/*    and Michael Feldmeier                                              */
/*                                                                       */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/**@file lp_data/HConst.h
 * @brief Constants for HiGHS
 */
#ifndef LP_DATA_HCONST_H_
#define LP_DATA_HCONST_H_

#include <limits>
#include <string>

#include "util/HighsInt.h"

const HighsInt kHighsIInf = std::numeric_limits<HighsInt>::max();
const double kHighsInf = std::numeric_limits<double>::infinity();
const double kHighsTiny = 1e-14;
const double kHighsZero = 1e-50;
const std::string kHighsOffString = "off";
const std::string kHighsChooseString = "choose";
const std::string kHighsOnString = "on";
const HighsInt kHighsThreadLimit = 8;  // 32;
const double kRunningAverageMultiplier = 0.05;

enum HighsDebugLevel {
  kHighsDebugLevelMin = 0,
  kHighsDebugLevelNone = kHighsDebugLevelMin,  // 0
  kHighsDebugLevelCheap,                       // 1
  kHighsDebugLevelCostly,                      // 2
  kHighsDebugLevelExpensive,                   // 3
  kHighsDebugLevelMax = kHighsDebugLevelExpensive
};

enum class HighsDebugStatus {
  kNotChecked = -1,
  kOk,
  kSmallError,
  kWarning,
  kLargeError,
  kError,
  kExcessiveError,
  kLogicalError,
};

enum HighsAnalysisLevel {
  kHighsAnalysisLevelMin = 0,
  kHighsAnalysisLevelNone = kHighsAnalysisLevelMin,
  kHighsAnalysisLevelModelData = 1,
  kHighsAnalysisLevelSolverData = 2,
  kHighsAnalysisLevelSolverTime = 4,
  kHighsAnalysisLevelNlaData = 8,
  kHighsAnalysisLevelNlaTime = 16,
  kHighsAnalysisLevelMax =
      kHighsAnalysisLevelModelData + kHighsAnalysisLevelSolverData +
      kHighsAnalysisLevelSolverTime + kHighsAnalysisLevelNlaData +
      kHighsAnalysisLevelNlaTime
};

enum class HighsVarType : uint8_t {
  kContinuous = 0,
  kInteger = 1,
  kImplicitInteger = 2,
};

enum class HighsOptionType { kBool = 0, kInt, kDouble, kString };

enum class HighsInfoType { kInt = 1, kDouble };

enum OptionOffChooseOn {
  kHighsOptionOff = -1,
  kHighsOptionChoose,
  kHighsOptionOn
};

/** SCIP/HiGHS Objective sense */
enum class ObjSense { kMinimize = 1, kMaximize = -1 };

enum class MatrixOrientation { kNone = 0, kColwise, kRowwise };

enum PrimalDualStatus {
  kHighsPrimalDualStatusNotset = -1,
  kHighsPrimalDualStatusMin = kHighsPrimalDualStatusNotset,
  kHighsPrimalDualStatusNoSolution,
  kHighsPrimalDualStatusUnknown,
  kHighsPrimalDualStatusInfeasiblePoint,
  kHighsPrimalDualStatusFeasiblePoint,
  kHighsPrimalDualStatusMax = kHighsPrimalDualStatusFeasiblePoint
};

const std::string kHighsFilenameDefault = "";

// Need to allow infinite costs to pass SCIP LPI unit tests
const bool kHighsAllowInfiniteCosts = true;

// Primal/dual statuses and corresponding HighsModelStatus
// values. Note that if dual infeasibility is identified, then the
// prototype primal code is used to distinguish PRIMAL_DUAL_INFEASIBLE
// from PRIMAL_UNBOUNDED. If this fails, then HiGHS may just return
// DUAL_INFEASIBLE
//
//           | Du Infeas    | Du Feas   | Du UnBd
// Pr Infeas | PR_DU_INFEAS | PR_INFEAS | PR_INFEAS
// Pr Feas   | PR_UNBD      | OPTIMAL   |   N/A
// Pr Unbd   | PR_UNBD      |     N/A   |   N/A
//
// Dual infeasibility is recognised by infeasibility at dual phase 1 optimality
// (and implied by primal unboundedness)
//
// Dual feasibility is recognised by feasibility at dual phase 1 optimality or
// primal phase 2 optimality
//
// Dual unboundedness is recognised by unboundedness in dual phase 2
//
// Primal infeasibility is recognised by infeasibility at primal phase 1
// optimality (and implied by dual unboundedness)
//
// Primal feasibility is recognised by feasibility at primal phase 1 optimality
// or dual phase 2 optimality
//
// Primal unboundedness is recognised by unboundedness in primal phase 2
//

enum class HighsModelStatus {
  // NB Add new status values to the end so that int cast of status
  // values is unchanged, since enums are not preserved in some
  // interfaces
  kNotset = 0,
  kMin = kNotset,
  kLoadError,
  kModelError,
  kPresolveError,
  kSolveError,
  kPostsolveError,
  kModelEmpty,
  kOptimal,
  kInfeasible,
  kUnboundedOrInfeasible,
  kUnbounded,
  kReachedDualObjectiveValueUpperBound,
  kReachedTimeLimit,
  kReachedIterationLimit,
  kPrimalDualInfeasible,
  kDualInfeasible,
  kMax = kDualInfeasible
};

/** SCIP/CPLEX-like HiGHS basis status for columns and rows. */
enum class HighsBasisStatus {
  kLower =
      0,   // (slack) variable is at its lower bound [including fixed variables]
  kBasic,  // (slack) variable is basic
  kUpper,  // (slack) variable is at its upper bound
  kZero,   // free variable is non-basic and set to zero
  kNonbasic  // nonbasic with no specific bound information - useful for users
             // and postsolve
};

// Illegal values of num/max/sum infeasibility - used to indicate that true
// values aren't known
const HighsInt kHighsIllegalInfeasibilityCount = -1;
const double kHighsIllegalInfeasibilityMeasure = -1;

#endif /* LP_DATA_HCONST_H_ */
