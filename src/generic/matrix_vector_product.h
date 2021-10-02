// LIC// ====================================================================
// LIC// This file forms part of oomph-lib, the object-oriented,
// LIC// multi-physics finite-element library, available
// LIC// at http://www.oomph-lib.org.
// LIC//
// LIC// Copyright (C) 2006-2021 Matthias Heil and Andrew Hazel
// LIC//
// LIC// This library is free software; you can redistribute it and/or
// LIC// modify it under the terms of the GNU Lesser General Public
// LIC// License as published by the Free Software Foundation; either
// LIC// version 2.1 of the License, or (at your option) any later version.
// LIC//
// LIC// This library is distributed in the hope that it will be useful,
// LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
// LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// LIC// Lesser General Public License for more details.
// LIC//
// LIC// You should have received a copy of the GNU Lesser General Public
// LIC// License along with this library; if not, write to the Free Software
// LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
// LIC// 02110-1301  USA.
// LIC//
// LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
// LIC//
// LIC//====================================================================
// Include guards
#ifndef OOMPH_MATRIX_VECTOR_PRODUCT_HEADER
#define OOMPH_MATRIX_VECTOR_PRODUCT_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#include "matrices.h"
#include "linear_algebra_distribution.h"
#ifdef OOMPH_HAS_TRILINOS
#include "trilinos_helpers.h"
#endif

namespace oomph
{
  //=============================================================================
  /// \short Matrix vector product helper class - primarily a wrapper to
  /// Trilinos's Epetra matrix vector product methods. This allows the
  /// epetra matrix to be assembled once and the matrix vector product to be
  /// performed many times.
  //=============================================================================
  class MatrixVectorProduct : public DistributableLinearAlgebraObject
  {
  public:
    /// \short Constructor
    MatrixVectorProduct()
    {
      // null pointers
#ifdef OOMPH_HAS_TRILINOS
      Epetra_matrix_pt = 0;
#endif
      Oomph_matrix_pt = 0;
      Column_distribution_pt = 0;
    }

    /// Broken copy constructor
    MatrixVectorProduct(const MatrixVectorProduct&) = delete;

    /// Broken assignment operator
    void operator=(const MatrixVectorProduct&) = delete;

    /// \short Destructor
    ~MatrixVectorProduct()
    {
      this->clean_up_memory();
    }

    /// \short clear the memory
    void clean_up_memory()
    {
#ifdef OOMPH_HAS_TRILINOS
      delete Epetra_matrix_pt;
      Epetra_matrix_pt = 0;
#endif
      delete Oomph_matrix_pt;
      Oomph_matrix_pt = 0;
      delete Column_distribution_pt;
      Column_distribution_pt = 0;
    }

    /// \short Setup the matrix vector product operator.
    /// WARNING: This class is wrapper to Trilinos Epetra matrix vector
    /// multiply methods, if Trilinos is not installed then this class will
    /// function as expected, but there will be no computational speed gain.
    /// By default the Epetra_CrsMatrix::multiply(...) are employed.
    /// The optional argument col_dist_pt is the distribution of:
    /// x if using multiply(...) or y if using multiply_transpose(...)
    /// where this is A x = y. By default, this is assumed to the uniformly
    /// distributed based on matrix_pt->ncol().
    void setup(CRDoubleMatrix* matrix_pt,
               const LinearAlgebraDistribution* col_dist_pt = 0);

    /// \short Apply the operator to the vector x and return the result in
    /// the vector y
    void multiply(const DoubleVector& x, DoubleVector& y) const;

    /// \short Apply the transpose of the operator to the vector x and return
    /// the result in the vector y
    void multiply_transpose(const DoubleVector& x, DoubleVector& y) const;

    /// Access function to the number of columns.
    const unsigned& ncol() const
    {
      return Ncol;
    }

  private:
#ifdef OOMPH_HAS_TRILINOS
    /// Helper function for multiply(...)
    void trilinos_multiply_helper(const DoubleVector& x, DoubleVector& y) const;

    /// Helper function for multiply_transpose(...)
    void trilinos_multiply_transpose_helper(const DoubleVector& x,
                                            DoubleVector& y) const;

    /// \short The Epetra version of the matrix
    Epetra_CrsMatrix* Epetra_matrix_pt;
#endif

    /// \short boolean indicating whether we are using trilinos to perform
    /// matvec
    bool Using_trilinos;

    /// \short an oomph-lib matrix
    CRDoubleMatrix* Oomph_matrix_pt;

    /// \short The distribution of: x if using multiply(...) or y
    /// if using multiply_transpose(...) where this is A x = y.
    LinearAlgebraDistribution* Column_distribution_pt;

    /// number of columns of the matrix
    unsigned Ncol;
  };
} // namespace oomph
#endif
