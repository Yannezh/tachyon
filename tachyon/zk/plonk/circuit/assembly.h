// Copyright 2020-2022 The Electric Coin Company
// Copyright 2022 The Halo2 developers
// Use of this source code is governed by a MIT/Apache-2.0 style license that
// can be found in the LICENSE-MIT.halo2 and the LICENCE-APACHE.halo2
// file.

#ifndef TACHYON_ZK_PLONK_CIRCUIT_ASSEMBLY_H_
#define TACHYON_ZK_PLONK_CIRCUIT_ASSEMBLY_H_

#include <utility>
#include <vector>

#include "tachyon/base/range.h"
#include "tachyon/math/base/rational_field.h"
#include "tachyon/math/polynomials/univariate/univariate_polynomial.h"
#include "tachyon/zk/plonk/circuit/assignment.h"
#include "tachyon/zk/plonk/permutation/permutation_assembly.h"

namespace tachyon::zk {

template <typename PCSTy>
class Assembly : public Assignment<typename PCSTy::Field> {
 public:
  using F = typename PCSTy::Field;
  using DensePoly =
      math::UnivariateDensePolynomial<math::RationalField<F>, PCSTy::kMaxSize>;

  // Assignment methods
  Error EnableSelector(const Selector& selector, size_t row) override {
    if (!usable_rows_.Contains(row)) {
      return Error::kNotEnoughRowsAvailable;
    }
    selectors_[selector.index()][row] = true;
    return Error::kNone;
  }

  Error QueryInstance(const InstanceColumn& column, size_t row,
                      Value<F>* instance) override {
    if (!usable_rows_.Contains(row)) {
      return Error::kNotEnoughRowsAvailable;
    }
    *instance = Value<F>::Unknown();
    return Error::kNone;
  }

  Error AssignFixed(const FixedColumn& column, size_t row,
                    base::OnceCallback<math::RationalField<F>>() >
                        to) override {
    if (!usable_rows_.Contains(row)) {
      return Error::kNotEnoughRowsAvailable;
    }
    fixeds_[column.index()] = std::move(to).Run();
    return Error::kNone;
  }

  Error Copy(const AnyColumn& left_column, size_t left_row,
             const AnyColumn& right_column, size_t right_row) override {
    if (!(usable_rows_.Contains(left_row) &&
          usable_rows_.Contains(right_row))) {
      return Error::kNotEnoughRowsAvailable;
    }
    return permutation_.Copy(left_column, left_row, right_column, right_row);
  }

  Error FillFromRow(const FixedColumn& column, size_t from_row,
                    base::OnceCallback<math::RationalField<F>>() >
                        to) override {
    if (!usable_rows_.Contains(from_row)) {
      return Error::kNotEnoughRowsAvailable;
    }
    math::RationalField<F> value = std::move(to).Run();
    for (size_t i = usable_rows_.start + from_row; i < usable_rows_.end; ++i) {
      fixeds_[column.index()] = value;
    }
    return Error::kNone;
  }

 private:
  uint32_t k_;
  std::vector<DensePoly> fixeds_;
  PermutationAssembly<PCSTy> permutation_;
  std::vector<std::vector<bool>> selectors_;
  // A range of available rows for assignment and copies.
  base::Range<size_t> usable_rows_;
};

}  // namespace tachyon::zk

#endif  // TACHYON_ZK_PLONK_CIRCUIT_ASSEMBLY_H_
