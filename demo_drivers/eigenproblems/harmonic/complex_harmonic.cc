// LIC// ====================================================================
// LIC// This file forms part of oomph-lib, the object-oriented,
// LIC// multi-physics finite-element library, available
// LIC// at http://www.oomph-lib.org.
// LIC//
// LIC//    Version 1.0; svn revision $LastChangedRevision$
// LIC//
// LIC// $LastChangedDate$
// LIC//
// LIC// Copyright (C) 2006-2016 Matthias Heil and Andrew Hazel
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
// Driver to solve the harmonic equation as a quadratic eigenvalue problem,
// by solving two first-order systems with homogeneous Dirichlet boundary
// conditions. The sign is chosen to that all the eigenvalues are imaginary
// when Mu is small (Mu=0 actually leads to a singular mass matrix so should
// be avoided), but pairs merge to become real as Mu increases.

// Generic oomph-lib routines
#include "generic.h"

// Include the mesh
#include "meshes/one_d_mesh.h"

using namespace std;

using namespace oomph;

/// Namespace for the shift applied to the eigenproblem
namespace EigenproblemShift
{
  // Parameter chosen to that only one complex-conjugate pair has merged
  double Mu = 6.5; // Will lead to singular mass matrix if set to zero
} // namespace EigenproblemShift

//===================================================================
/// Function-type-object to perform comparison of complex data types
/// Needed to sort the complex eigenvalues into order based on the
/// size of the real part.
//==================================================================
template<class T>
class ComplexLess
{
public:
  /// Comparison in terms of magnitude of complex number
  bool operator()(const complex<T>& x, const complex<T>& y) const
  {
    // Return the order in terms of magnitude if they are not equal
    // Include a tolerance to avoid processor specific ordering
    if (std::abs(std::abs(x) - std::abs(y)) > 1.0e-10)
    {
      return std::abs(x) < std::abs(y);
    }
    // Otherwise sort on real part, again with a tolerance
    else if (std::abs(x.real() - y.real()) > 1.0e-10)
    {
      return x.real() < y.real();
    }
    // Otherwise sort on imaginary part
    else
    {
      return x.imag() < y.imag();
    }
  }
};

//=================================================================
/// A class for all elements that solve the eigenvalue problem
/// \f[
/// \frac{\partial w}{\partial x}  = \lambda u
/// \f]
/// \f[
/// \frac{\partial u}{\partial x} = (\lambda - \mu) w
/// \f]
/// This class  contains the generic maths. Shape functions, geometric
/// mapping etc. must get implemented in derived class.
//================================================================
class ComplexHarmonicEquations : public virtual FiniteElement
{
public:
  /// Empty Constructor
  ComplexHarmonicEquations() {}

  /// \short Access function: First eigenfunction value at local node n
  /// Note that solving the eigenproblem does not assign values
  /// to this storage space. It is used for output purposes only.
  virtual inline double u(const unsigned& n) const
  {
    return nodal_value(n, 0);
  }

  /// \short Second eigenfunction value at local node n
  virtual inline double w(const unsigned& n) const
  {
    return nodal_value(n, 1);
  }

  /// Output the eigenfunction with default number of plot points
  void output(ostream& outfile)
  {
    unsigned nplot = 5;
    output(outfile, nplot);
  }

  /// \short Output FE representation of soln: x,y,u or x,y,z,u at
  /// Nplot  plot points
  void output(ostream& outfile, const unsigned& nplot)
  {
    // Vector of local coordinates
    Vector<double> s(1);

    // Tecplot header info
    outfile << tecplot_zone_string(nplot);

    // Loop over plot points
    unsigned num_plot_points = nplot_points(nplot);
    for (unsigned iplot = 0; iplot < num_plot_points; iplot++)
    {
      // Get local coordinates of plot point
      get_s_plot(iplot, nplot, s);
      // Output the coordinate and the eigenfunction
      outfile << interpolated_x(s, 0) << " " << interpolated_u(s) << " "
              << interpolated_w(s) << std::endl;
    }
    // Write tecplot footer (e.g. FE connectivity lists)
    write_tecplot_zone_footer(outfile, nplot);
  }

  /// \short Assemble the contributions to the jacobian and mass
  /// matrices
  void fill_in_contribution_to_jacobian_and_mass_matrix(
    Vector<double>& residuals,
    DenseMatrix<double>& jacobian,
    DenseMatrix<double>& mass_matrix)
  {
    // Find out how many nodes there are
    unsigned n_node = nnode();

    // Set up memory for the shape functions and their derivatives
    Shape psi(n_node);
    DShape dpsidx(n_node, 1);

    // Set the number of integration points
    unsigned n_intpt = integral_pt()->nweight();

    // Integers to store the local equation and unknown numbers
    int local_eqn = 0, local_unknown = 0;

    // Loop over the integration points
    for (unsigned ipt = 0; ipt < n_intpt; ipt++)
    {
      // Get the integral weight
      double w = integral_pt()->weight(ipt);

      // Call the derivatives of the shape and test functions
      double J = dshape_eulerian_at_knot(ipt, psi, dpsidx);

      // Premultiply the weights and the Jacobian
      double W = w * J;

      // Assemble the contributions to the mass matrix
      // Loop over the test functions
      for (unsigned l = 0; l < n_node; l++)
      {
        // Get the local equation number
        local_eqn = u_local_eqn(l, 0);
        // If it's not a boundary condition
        if (local_eqn >= 0)
        {
          // Loop over the shape functions
          for (unsigned l2 = 0; l2 < n_node; l2++)
          {
            local_unknown = u_local_eqn(l2, 0);
            // If at a non-zero degree of freedom add in the entry
            if (local_unknown >= 0)
            {
              // This corresponds to the top left B block
              mass_matrix(local_eqn, local_unknown) += psi(l2) * psi(l) * W;
            }
            local_unknown = u_local_eqn(l2, 1);
            // If at a non-zero degree of freedom add in the entry
            if (local_unknown >= 0)
            {
              // This corresponds to the top right A block
              jacobian(local_eqn, local_unknown) += dpsidx(l2, 0) * psi(l) * W;
            }
          }
        }

        // Get the local equation number
        local_eqn = u_local_eqn(l, 1);
        // IF it's not a boundary condition
        if (local_eqn >= 0)
        {
          // Loop over the shape functions
          for (unsigned l2 = 0; l2 < n_node; l2++)
          {
            local_unknown = u_local_eqn(l2, 0);
            // If at a non-zero degree of freedom add in the entry
            if (local_unknown >= 0)
            {
              // This corresponds to the lower left A block
              jacobian(local_eqn, local_unknown) += dpsidx(l2, 0) * psi(l) * W;
            }
            local_unknown = u_local_eqn(l2, 1);
            // If at a non-zero degree of freedom add in the entry
            if (local_unknown >= 0)
            {
              // This corresponds to the lower right B block
              mass_matrix(local_eqn, local_unknown) += psi(l2) * psi(l) * W;
              // This corresponds to the lower right \mu B block
              jacobian(local_eqn, local_unknown) +=
                EigenproblemShift::Mu * psi(l2) * psi(l) * W;
            }
          }
        }
      }
    }
  } // end_of_fill_in_contribution_to_jacobian_and_mass_matrix

  /// Return FE representation of function value u(s) at local coordinate s
  inline double interpolated_u(const Vector<double>& s) const
  {
    unsigned n_node = nnode();

    // Local shape functions
    Shape psi(n_node);

    // Find values of basis function
    this->shape(s, psi);

    // Initialise value of u
    double interpolated_u = 0.0;

    // Loop over the local nodes and sum
    for (unsigned l = 0; l < n_node; l++)
    {
      interpolated_u += u(l) * psi[l];
    }

    // Return the interpolated value of the eigenfunction
    return (interpolated_u);
  }

  /// Return FE representation of function value w(s) at local coordinate s
  inline double interpolated_w(const Vector<double>& s) const
  {
    unsigned n_node = nnode();

    // Local shape functions
    Shape psi(n_node);

    // Find values of basis function
    this->shape(s, psi);

    // Initialise value of u
    double interpolated_u = 0.0;

    // Loop over the local nodes and sum
    for (unsigned l = 0; l < n_node; l++)
    {
      interpolated_u += w(l) * psi[l];
    }

    // Return the interpolated value of the eigenfunction
    return (interpolated_u);
  }

protected:
  /// \short Shape/test functions and derivs w.r.t. to global coords at
  /// local coord. s; return  Jacobian of mapping
  virtual double dshape_eulerian(const Vector<double>& s,
                                 Shape& psi,
                                 DShape& dpsidx) const = 0;

  /// \short Shape/test functions and derivs w.r.t. to global coords at
  /// integration point ipt; return  Jacobian of mapping
  virtual double dshape_eulerian_at_knot(const unsigned& ipt,
                                         Shape& psi,
                                         DShape& dpsidx) const = 0;

  /// \short Access function that returns the local equation number
  /// of the unknown in the problem. Default is to assume that it is the
  /// first (only) value stored at the node.
  virtual inline int u_local_eqn(const unsigned& n, const unsigned& i)
  {
    return nodal_local_eqn(n, i);
  }

private:
};

//======================================================================
/// QComplexHarmonicElement<NNODE_1D> elements are 1D  Elements with
/// NNODE_1D nodal points that are used to solve the ComplexHarmonic eigenvalue
/// Problem described by ComplexHarmonicEquations.
//======================================================================
template<unsigned NNODE_1D>
class QComplexHarmonicElement :
  public virtual QElement<1, NNODE_1D>,
  public ComplexHarmonicEquations
{
public:
  ///\short  Constructor: Call constructors for QElement and
  /// Poisson equations
  QComplexHarmonicElement() :
    QElement<1, NNODE_1D>(), ComplexHarmonicEquations()
  {
  }

  /// \short  Required  # of `values' (pinned or dofs)
  /// at node n. Here there are two (u and w)
  inline unsigned required_nvalue(const unsigned& n) const
  {
    return 2;
  }

  /// \short Output function overloaded from ComplexHarmonicEquations
  void output(ostream& outfile)
  {
    ComplexHarmonicEquations::output(outfile);
  }

  ///  \short Output function overloaded from ComplexHarmonicEquations
  void output(ostream& outfile, const unsigned& Nplot)
  {
    ComplexHarmonicEquations::output(outfile, Nplot);
  }

protected:
  /// Shape, test functions & derivs. w.r.t. to global coords. Return Jacobian.
  inline double dshape_eulerian(const Vector<double>& s,
                                Shape& psi,
                                DShape& dpsidx) const
  {
    return QElement<1, NNODE_1D>::dshape_eulerian(s, psi, dpsidx);
  }

  /// \short Shape, test functions & derivs. w.r.t. to global coords. at
  /// integration point ipt. Return Jacobian.
  inline double dshape_eulerian_at_knot(const unsigned& ipt,
                                        Shape& psi,
                                        DShape& dpsidx) const
  {
    return QElement<1, NNODE_1D>::dshape_eulerian_at_knot(ipt, psi, dpsidx);
  }

}; // end_of_QComplexHarmonic_class_definition

//==start_of_problem_class============================================
/// 1D ComplexHarmonic problem in unit interval.
//====================================================================
template<class ELEMENT, class EIGEN_SOLVER>
class ComplexHarmonicProblem : public Problem
{
public:
  /// Constructor: Pass number of elements and pointer to source function
  ComplexHarmonicProblem(const unsigned& n_element);

  /// Destructor. Clean up the mesh and solver
  ~ComplexHarmonicProblem()
  {
    delete this->mesh_pt();
    delete this->eigen_solver_pt();
  }

  /// Solve the problem
  void solve(const unsigned& label);

  /// \short Doc the solution, pass the number of the case considered,
  /// so that output files can be distinguished.
  void doc_solution(const unsigned& label);

}; // end of problem class

//=====start_of_constructor===============================================
/// \short Constructor for 1D ComplexHarmonic problem in unit interval.
/// Discretise the 1D domain with n_element elements of type ELEMENT.
/// Specify function pointer to source function.
//========================================================================
template<class ELEMENT, class EIGEN_SOLVER>
ComplexHarmonicProblem<ELEMENT, EIGEN_SOLVER>::ComplexHarmonicProblem(
  const unsigned& n_element)
{
  // Create the eigen solver
  this->eigen_solver_pt() = new EIGEN_SOLVER;

  // Get the positive eigenvalues, shift is zero by default
  static_cast<EIGEN_SOLVER*>(eigen_solver_pt())
    ->get_eigenvalues_right_of_shift();

  // Set domain length
  double L = 1.0;

  // Build mesh and store pointer in Problem
  Problem::mesh_pt() = new OneDMesh<ELEMENT>(n_element, L);

  // Set the boundary conditions for this problem: By default, all nodal
  // values are free -- we only need to pin the ones that have
  // Dirichlet conditions.

  // Pin the single nodal value at the single node on mesh
  // boundary 0 (= the left domain boundary at x=0)
  mesh_pt()->boundary_node_pt(0, 0)->pin(0);

  // Pin the single nodal value at the single node on mesh
  // boundary 1 (= the right domain boundary at x=1)
  mesh_pt()->boundary_node_pt(1, 0)->pin(0);

  // Setup equation numbering scheme
  assign_eqn_numbers();

} // end of constructor

//===start_of_doc=========================================================
/// Doc the solution in tecplot format. Label files with label.
//========================================================================
template<class ELEMENT, class EIGEN_SOLVER>
void ComplexHarmonicProblem<ELEMENT, EIGEN_SOLVER>::doc_solution(
  const unsigned& label)
{
  ofstream some_file;
  char filename[100];

  // Number of plot points
  unsigned npts;
  npts = 5;

  // Output solution with specified number of plot points per element
  sprintf(filename, "soln%i.dat", label);
  some_file.open(filename);
  mesh_pt()->output(some_file, npts);
  some_file.close();

} // end of doc

//=======================start_of_solve==============================
/// Solve the eigenproblem
//===================================================================
template<class ELEMENT, class EIGEN_SOLVER>
void ComplexHarmonicProblem<ELEMENT, EIGEN_SOLVER>::solve(const unsigned& label)
{
  // Set external storage for the eigenvalues
  Vector<complex<double>> eigenvalues;
  // Set external storage for the eigenvectors
  Vector<DoubleVector> eigenvectors;
  // Desired number eigenvalues enough to include the first two real
  // eigenvalues and then the complex conjugate pair. If the number is set
  // to four then ARPACK prefers the degenerate (real) eigenvalues Mu, Mu.
  unsigned n_eval = 7;

  // Solve the eigenproblem
  this->solve_eigenproblem(n_eval, eigenvalues, eigenvectors);

  // We now need to sort the output based on the size of the real part
  // of the eigenvalues.
  // This is because the solver does not necessarily sort the eigenvalues
  Vector<complex<double>> sorted_eigenvalues = eigenvalues;
  sort(sorted_eigenvalues.begin(),
       sorted_eigenvalues.end(),
       ComplexLess<double>());

  // Read out the smallest eigenvalue
  complex<double> temp_evalue = sorted_eigenvalues[0];
  unsigned smallest_index = 0;
  // Loop over the unsorted eigenvalues and find the entry that corresponds
  // to our second smallest eigenvalue.
  for (unsigned i = 0; i < eigenvalues.size(); i++)
  {
    // Note that equality tests for doubles are bad, but it was just
    // sorted data, so should be fine
    if (eigenvalues[i] == temp_evalue)
    {
      smallest_index = i;
      break;
    }
  }

  // Normalise the eigenvector
  {
    // Get the dimension of the eigenvector
    unsigned dim = eigenvectors[smallest_index].nrow();
    double length = 0.0;
    // Loop over all the entries
    for (unsigned i = 0; i < dim; i++)
    {
      // Add the contribution to the length
      length += std::pow(eigenvectors[smallest_index][i], 2.0);
    }
    // Now take the magnitude
    length = sqrt(length);
    // Fix the sign
    if (eigenvectors[smallest_index][0] < 0)
    {
      length *= -1.0;
    }
    // Finally normalise
    for (unsigned i = 0; i < dim; i++)
    {
      eigenvectors[smallest_index][i] /= length;
    }
  }

  // Now assign the second eigenvector to the dofs of the problem
  this->assign_eigenvector_to_dofs(eigenvectors[smallest_index]);
  // Output solution for this case (label output files with "1")
  this->doc_solution(label);

  char filename[100];
  sprintf(filename, "eigenvalues%i.dat", label);

  // Open an output file for the sorted eigenvalues
  ofstream evalues(filename);
  for (unsigned i = 0; i < n_eval; i++)
  {
    // Print to screen
    cout << sorted_eigenvalues[i].real() << " " << sorted_eigenvalues[i].imag()
         << std::endl;
    // Send to file
    evalues << sorted_eigenvalues[i].real() << " "
            << sorted_eigenvalues[i].imag() << std::endl;
  }

  evalues.close();
} // end_of_solve

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////

//======start_of_main==================================================
/// Driver for 1D Poisson problem
//=====================================================================
int main(int argc, char** argv)
{
// Want to test Trilinos if we have it, so we must initialise MPI
// if we have compiled with it
#ifdef OOMPH_HAS_MPI
  MPI_Helpers::init(argc, argv);
#endif

  // Set up the problem:
  unsigned n_element = 100; // Number of elements

  clock_t t_start1 = clock();
  // Solve with ARPACK
  {
    ComplexHarmonicProblem<QComplexHarmonicElement<3>, ARPACK> problem(
      n_element);

    std::cout << "Matrix size " << problem.ndof() << std::endl;

    problem.solve(1);
  }
  clock_t t_end1 = clock();

  clock_t t_start2 = clock();
  // Solve with LAPACK_QZ
  {
    ComplexHarmonicProblem<QComplexHarmonicElement<3>, LAPACK_QZ> problem(
      n_element);

    problem.solve(2);
  }
  clock_t t_end2 = clock();

#ifdef OOMPH_HAS_TRILINOS
  clock_t t_start3 = clock();
  // Solve with Anasazi
  {
    ComplexHarmonicProblem<QComplexHarmonicElement<3>, ANASAZI> problem(
      n_element);
    problem.solve(3);
  }
  clock_t t_end3 = clock();
#endif

  std::cout << "ARPACK TIME: " << (double)(t_end1 - t_start1) / CLOCKS_PER_SEC
            << std::endl;

  std::cout << "LAPACK TIME: " << (double)(t_end2 - t_start2) / CLOCKS_PER_SEC
            << std::endl;

#ifdef OOMPH_HAS_TRILINOS
  std::cout << "ANASAZI TIME: " << (double)(t_end3 - t_start3) / CLOCKS_PER_SEC
            << std::endl;
#endif

#ifdef OOMPH_HAS_MPI
  MPI_Helpers::finalize();
#endif

} // end of main
