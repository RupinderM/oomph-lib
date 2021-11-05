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
// This header file contains classes and inline function definitions for
// matrices of complex numbers and their derived types

// Include guards to prevent multiple inclusion of the header
#ifndef OOMPH_COMPLEX_MATRICES_HEADER
#define OOMPH_COMPLEX_MATRICES_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

// Of course we need the complex type
#include <complex>

// Also need the standard matrices header
#include "matrices.h"

namespace oomph
{
  //===================================================================
  /// Abstract base class for matrices of complex doubles -- adds
  /// abstract interfaces for solving, LU decomposition and
  /// multiplication by vectors.
  //===================================================================
  class ComplexMatrixBase
  {
  public:
    /// (Empty) constructor.
    ComplexMatrixBase() {}

    /// Broken copy constructor
    ComplexMatrixBase(const ComplexMatrixBase& matrix) = delete;

    /// Broken assignment operator
    void operator=(const ComplexMatrixBase&) = delete;

    /// Return the number of rows of the matrix
    virtual unsigned long nrow() const = 0;

    /// Return the number of columns of the matrix
    virtual unsigned long ncol() const = 0;

    /// virtual (empty) destructor
    virtual ~ComplexMatrixBase() {}

    /// Round brackets to give access as a(i,j) for read only
    /// (we're not providing a general interface for component-wise write
    /// access since not all matrix formats allow efficient direct access!)
    virtual std::complex<double> operator()(const unsigned long& i,
                                            const unsigned long& j) const = 0;

    /// LU decomposition of the matrix using the appropriate
    /// linear solver. Return the sign of the determinant
    virtual int ludecompose()
    {
      throw OomphLibError(
        "ludecompose() has not been written for this matrix class\n",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);

      /// Dummy return
      return 1;
    }

    /// LU backsubstitue a previously LU-decomposed matrix;
    /// The passed rhs will be over-written with the solution vector
    virtual void lubksub(Vector<std::complex<double>>& rhs)
    {
      throw OomphLibError(
        "lubksub() has not been written for this matrix class\n",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }


    /// Complete LU solve (replaces matrix by its LU decomposition
    /// and overwrites RHS with solution). The default should not need
    /// to be over-written
    virtual void solve(Vector<std::complex<double>>& rhs);

    /// Complete LU solve (Nothing gets overwritten!). The default should
    /// not need to be overwritten
    virtual void solve(const Vector<std::complex<double>>& rhs,
                       Vector<std::complex<double>>& soln);

    /// Find the residual, i.e. r=b-Ax the residual
    virtual void residual(const Vector<std::complex<double>>& x,
                          const Vector<std::complex<double>>& b,
                          Vector<std::complex<double>>& residual) = 0;

    /// Find the maximum residual r=b-Ax -- generic version, can be
    /// overloaded for specific derived classes where the
    /// max. can be determined "on the fly"
    virtual double max_residual(const Vector<std::complex<double>>& x,
                                const Vector<std::complex<double>>& rhs)
    {
      unsigned long n = rhs.size();
      Vector<std::complex<double>> res(n);
      residual(x, rhs, res);
      double ans = 0.0;
      for (unsigned long i = 0; i < n; i++)
      {
        ans = std::max(ans, std::abs(res[i]));
      }
      return ans;
    }

    /// Multiply the matrix by the vector x: soln=Ax.
    virtual void multiply(const Vector<std::complex<double>>& x,
                          Vector<std::complex<double>>& soln) = 0;

    /// Multiply the  transposed matrix by the vector x: soln=A^T x
    virtual void multiply_transpose(const Vector<std::complex<double>>& x,
                                    Vector<std::complex<double>>& soln) = 0;
  };


  //=================================================================
  /// Class of matrices containing double complex, and stored as a
  /// DenseMatrix<complex<double> >, but with solving functionality inherited
  /// from the abstract ComplexMatrix class.
  //=================================================================
  class DenseComplexMatrix : public ComplexMatrixBase,
                             public DenseMatrix<std::complex<double>>
  {
  private:
    /// Pointer to storage for the index of permutations in the LU solve
    Vector<long>* Index;

    /// Pointer to storage for the LU decomposition
    std::complex<double>* LU_factors;

    /// Boolean flag used to decided whether the LU decomposition is stored
    /// separately, or not
    bool Overwrite_matrix_storage;

    /// Function that deletes the storage for the LU_factors, if it is
    /// not the same as the matrix storage
    void delete_lu_factors();

  public:
    /// Empty constructor, simply assign the lengths N and M to 0
    DenseComplexMatrix()
      : DenseMatrix<std::complex<double>>(),
        Index(0),
        LU_factors(0),
        Overwrite_matrix_storage(false)
    {
    }

    /// Constructor to build a square n by n matrix.
    DenseComplexMatrix(const unsigned long& n)
      : DenseMatrix<std::complex<double>>(n),
        Index(0),
        LU_factors(0),
        Overwrite_matrix_storage(false)
    {
    }

    /// Constructor to build a matrix with n rows and m columns.
    DenseComplexMatrix(const unsigned long& n, const unsigned long& m)
      : DenseMatrix<std::complex<double>>(n, m),
        Index(0),
        LU_factors(0),
        Overwrite_matrix_storage(false)
    {
    }

    /// Constructor to build a matrix with n rows and m columns,
    /// with initial value initial_val
    DenseComplexMatrix(const unsigned long& n,
                       const unsigned long& m,
                       const std::complex<double>& initial_val)
      : DenseMatrix<std::complex<double>>(n, m, initial_val),
        Index(0),
        LU_factors(0),
        Overwrite_matrix_storage(false)
    {
    }


    /// Broken copy constructor
    DenseComplexMatrix(const DenseComplexMatrix& matrix) = delete;

    /// Broken assignment operator
    void operator=(const DenseComplexMatrix&) = delete;

    /// Return the number of rows of the matrix
    inline unsigned long nrow() const
    {
      return DenseMatrix<std::complex<double>>::nrow();
    }

    /// Return the number of columns of the matrix
    inline unsigned long ncol() const
    {
      return DenseMatrix<std::complex<double>>::ncol();
    }

    /// Overload the const version of the round-bracket access operator
    /// for read-only access.
    inline std::complex<double> operator()(const unsigned long& i,
                                           const unsigned long& j) const
    {
      return DenseMatrix<std::complex<double>>::get_entry(i, j);
    }

    /// Overload the non-const version of the round-bracket access
    /// operator for read-write access
    inline std::complex<double>& operator()(const unsigned long& i,
                                            const unsigned long& j)
    {
      return DenseMatrix<std::complex<double>>::entry(i, j);
    }

    /// Destructor, delete the storage for Index vector and LU storage (if any)
    virtual ~DenseComplexMatrix();

    /// Overload the LU decomposition.
    /// For this storage scheme the matrix storage will be over-writeen
    /// by its LU decomposition
    int ludecompose();

    /// Overload the LU back substitution. Note that the rhs will be
    /// overwritten with the solution vector
    void lubksub(Vector<std::complex<double>>& rhs);

    /// Find the residual of Ax=b, ie r=b-Ax for the
    /// "solution" x.
    void residual(const Vector<std::complex<double>>& x,
                  const Vector<std::complex<double>>& rhs,
                  Vector<std::complex<double>>& residual);

    /// Multiply the matrix by the vector x: soln=Ax
    void multiply(const Vector<std::complex<double>>& x,
                  Vector<std::complex<double>>& soln);

    /// Multiply the  transposed matrix by the vector x: soln=A^T x
    void multiply_transpose(const Vector<std::complex<double>>& x,
                            Vector<std::complex<double>>& soln);
  };


  //=================================================================
  /// A class for compressed row matrices
  //=================================================================
  class CRComplexMatrix : public CRMatrix<std::complex<double>>,
                          public ComplexMatrixBase
  {
  private:
    /// Storage for the LU factors as required by SuperLU
    void* F_factors;

    ///  Info flag for the SuperLU solver
    int Info;

  public:
    /// Default constructor
    CRComplexMatrix() : CRMatrix<std::complex<double>>(), F_factors(0), Info(0)
    {
      // By default SuperLU solves linear systems quietly
      Doc_stats_during_solve = false;
    }

    /// Constructor: Pass vector of values, vector of column indices,
    /// vector of row starts and number of columns (can be suppressed
    /// for square matrices)
    CRComplexMatrix(const Vector<std::complex<double>>& value,
                    const Vector<int>& column_index,
                    const Vector<int>& row_start,
                    const unsigned long& n,
                    const unsigned long& m)
      : CRMatrix<std::complex<double>>(value, column_index, row_start, n, m),
        F_factors(0),
        Info(0)
    {
      // By default SuperLU solves linear systems quietly
      Doc_stats_during_solve = false;
    }


    /// Broken copy constructor
    CRComplexMatrix(const CRComplexMatrix& matrix) = delete;

    /// Broken assignment operator
    void operator=(const CRComplexMatrix&) = delete;

    /// Destructor: Kill the LU decomposition if it has been computed
    virtual ~CRComplexMatrix()
    {
      clean_up_memory();
    }

    /// Set flag to indicate that stats are to be displayed during
    /// solution of linear system with SuperLU
    void enable_doc_stats()
    {
      Doc_stats_during_solve = true;
    }

    // Set flag to indicate that stats are not to be displayed during
    /// the solve
    void disable_doc_stats()
    {
      Doc_stats_during_solve = false;
    }

    /// Return the number of rows of the matrix
    inline unsigned long nrow() const
    {
      return CRMatrix<std::complex<double>>::nrow();
    }

    /// Return the number of columns of the matrix
    inline unsigned long ncol() const
    {
      return CRMatrix<std::complex<double>>::ncol();
    }

    /// Overload the round-bracket access operator for read-only access
    inline std::complex<double> operator()(const unsigned long& i,
                                           const unsigned long& j) const
    {
      return CRMatrix<std::complex<double>>::get_entry(i, j);
    }

    /// LU decomposition using SuperLU
    int ludecompose();

    /// LU back solve for given RHS
    void lubksub(Vector<std::complex<double>>& rhs);

    /// LU clean up (perhaps this should happen in the destructor)
    void clean_up_memory();

    /// Find the residual to x of Ax=b, i.e. r=b-Ax
    void residual(const Vector<std::complex<double>>& x,
                  const Vector<std::complex<double>>& b,
                  Vector<std::complex<double>>& residual);

    /// Multiply the matrix by the vector x: soln=Ax
    void multiply(const Vector<std::complex<double>>& x,
                  Vector<std::complex<double>>& soln);


    /// Multiply the  transposed matrix by the vector x: soln=A^T x
    void multiply_transpose(const Vector<std::complex<double>>& x,
                            Vector<std::complex<double>>& soln);

  protected:
    /// Flag to indicate if stats are to be displayed during
    /// solution of linear system with SuperLU
    bool Doc_stats_during_solve;
  };


  //=================================================================
  /// A class for compressed column matrices that store doubles
  //=================================================================
  class CCComplexMatrix : public ComplexMatrixBase,
                          public CCMatrix<std::complex<double>>
  {
  private:
    /// Storage for the LU factors as required by SuperLU
    void* F_factors;

    /// Info flag for the SuperLU solver
    int Info;

  protected:
    /// Flag to indicate if stats are to be displayed during
    /// solution of linear system with SuperLU
    bool Doc_stats_during_solve;

  public:
    /// Default constructor
    CCComplexMatrix() : CCMatrix<std::complex<double>>(), F_factors(0), Info(0)
    {
      // By default SuperLU solves linear systems quietly
      Doc_stats_during_solve = false;
    }

    /// Constructor: Pass vector of values, vector of row indices,
    /// vector of column starts and number of rows (can be suppressed
    /// for square matrices). Number of nonzero entries is read
    /// off from value, so make sure the vector has been shrunk
    /// to its correct length.
    CCComplexMatrix(const Vector<std::complex<double>>& value,
                    const Vector<int>& row_index,
                    const Vector<int>& column_start,
                    const unsigned long& n,
                    const unsigned long& m)
      : CCMatrix<std::complex<double>>(value, row_index, column_start, n, m),
        F_factors(0),
        Info(0)
    {
      // By default SuperLU solves linear systems quietly
      Doc_stats_during_solve = false;
    }

    /// Broken copy constructor
    CCComplexMatrix(const CCComplexMatrix& matrix) = delete;

    /// Broken assignment operator
    void operator=(const CCComplexMatrix&) = delete;

    /// Destructor: Kill the LU factors if they have been setup.
    virtual ~CCComplexMatrix()
    {
      clean_up_memory();
    }

    /// Set flag to indicate that stats are to be displayed during
    /// solution of linear system with SuperLU
    void enable_doc_stats()
    {
      Doc_stats_during_solve = true;
    }

    // Set flag to indicate that stats are not to be displayed during
    /// the solve
    void disable_doc_stats()
    {
      Doc_stats_during_solve = false;
    }

    /// Return the number of rows of the matrix
    inline unsigned long nrow() const
    {
      return CCMatrix<std::complex<double>>::nrow();
    }

    /// Return the number of columns of the matrix
    inline unsigned long ncol() const
    {
      return CCMatrix<std::complex<double>>::ncol();
    }

    /// Overload the round-bracket access operator to provide
    /// read-only (const) access to the data
    inline std::complex<double> operator()(const unsigned long& i,
                                           const unsigned long& j) const
    {
      return CCMatrix<std::complex<double>>::get_entry(i, j);
    }

    /// LU decomposition using SuperLU
    int ludecompose();

    /// LU back solve for given RHS
    void lubksub(Vector<std::complex<double>>& rhs);

    /// LU clean up (perhaps this should happen in the destructor)
    void clean_up_memory();

    /// Find the residulal to x of Ax=b, ie r=b-Ax
    void residual(const Vector<std::complex<double>>& x,
                  const Vector<std::complex<double>>& b,
                  Vector<std::complex<double>>& residual);


    /// Multiply the matrix by the vector x: soln=Ax
    void multiply(const Vector<std::complex<double>>& x,
                  Vector<std::complex<double>>& soln);


    /// Multiply the  transposed matrix by the vector x: soln=A^T x
    void multiply_transpose(const Vector<std::complex<double>>& x,
                            Vector<std::complex<double>>& soln);
  };
} // namespace oomph

#endif
