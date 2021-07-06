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
// Header file for Helmholtz elements
#ifndef OOMPH_HELMHOLTZ_ELEMENTS_HEADER
#define OOMPH_HELMHOLTZ_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// OOMPH-LIB headers
#include "../generic/projection.h"
#include "../generic/nodes.h"
#include "../generic/Qelements.h"
#include "../generic/oomph_utilities.h"
#include "math.h"
#include <complex>

namespace oomph
{
  //=============================================================
  /// A class for all isoparametric elements that solve the
  /// Helmholtz equations.
  /// \f[
  /// \frac{\partial^2 u}{\partial x_i^2} + k^2 u = f(x_j)
  /// \f]
  /// This contains the generic maths. Shape functions, geometric
  /// mapping etc. must get implemented in derived class.
  //=============================================================
  template<unsigned DIM>
  class HelmholtzEquations : public virtual FiniteElement
  {
  public:
    /// \short Function pointer to source function fct(x,f(x)) --
    /// x is a Vector!
    typedef void (*HelmholtzSourceFctPt)(const Vector<double>& x,
                                         std::complex<double>& f);

    /// Constructor (must initialise the Source_fct_pt to null)
    HelmholtzEquations() : Source_fct_pt(0), K_squared_pt(0) {}

    /// Broken copy constructor
    HelmholtzEquations(const HelmholtzEquations& dummy)
    {
      BrokenCopy::broken_copy("HelmholtzEquations");
    }

    /// Broken assignment operator
    // Commented out broken assignment operator because this can lead to a
    // conflict warning when used in the virtual inheritence hierarchy.
    // Essentially the compiler doesn't realise that two separate
    // implementations of the broken function are the same and so, quite
    // rightly, it shouts.
    /*void operator=(const HelmholtzEquations&)
     {
      BrokenCopy::broken_assign("HelmholtzEquations");
      }*/

    /// \short Return the index at which the unknown value
    /// is stored.
    virtual inline std::complex<unsigned> u_index_helmholtz() const
    {
      return std::complex<unsigned>(0, 1);
    }

    /// Get pointer to square of wavenumber
    double*& k_squared_pt()
    {
      return K_squared_pt;
    }

    /// Get the square of wavenumber
    double k_squared()
    {
#ifdef PARANOID
      if (K_squared_pt == 0)
      {
        throw OomphLibError(
          "Please set pointer to k_squared using access fct to pointer!",
          OOMPH_CURRENT_FUNCTION,
          OOMPH_EXCEPTION_LOCATION);
      }
#endif
      return *K_squared_pt;
    }

    /// \short Number of scalars/fields output by this element. Reimplements
    /// broken virtual function in base class.
    unsigned nscalar_paraview() const
    {
      return 2;
    }

    /// \short Write values of the i-th scalar field at the plot points. Needs
    /// to be implemented for each new specific element type.
    void scalar_value_paraview(std::ofstream& file_out,
                               const unsigned& i,
                               const unsigned& nplot) const
    {
      // Vector of local coordinates
      Vector<double> s(DIM);

      // Loop over plot points
      unsigned num_plot_points = nplot_points_paraview(nplot);
      for (unsigned iplot = 0; iplot < num_plot_points; iplot++)
      {
        // Get local coordinates of plot point
        get_s_plot(iplot, nplot, s);
        std::complex<double> u(interpolated_u_helmholtz(s));

        // Paraview need to ouput the fileds seperatly so it loops through all
        // the elements twice
        switch (i)
        {
            // Real part first
          case 0:
            file_out << u.real() << std::endl;
            break;

            // Imaginary part second
          case 1:
            file_out << u.imag() << std::endl;
            break;

            // Never get here
          default:
            std::stringstream error_stream;
            error_stream
              << "Helmholtz elements only store 2 fields so i must be 0 or 1"
              << std::endl;
            throw OomphLibError(error_stream.str(),
                                OOMPH_CURRENT_FUNCTION,
                                OOMPH_EXCEPTION_LOCATION);
            break;
        }
      } // end of plotpoint loop
    } // end scalar_value_paraview

    /// \short Name of the i-th scalar field. Default implementation
    /// returns V1 for the first one, V2 for the second etc. Can (should!) be
    /// overloaded with more meaningful names in specific elements.
    std::string scalar_name_paraview(const unsigned& i) const
    {
      switch (i)
      {
        case 0:
          return "Real part";
          break;

        case 1:
          return "Imaginary part";
          break;

          // Never get here
        default:
          std::stringstream error_stream;
          error_stream
            << "Helmholtz elements only store 2 fields so i must be 0 or 1"
            << std::endl;
          throw OomphLibError(error_stream.str(),
                              OOMPH_CURRENT_FUNCTION,
                              OOMPH_EXCEPTION_LOCATION);

          // Dummy return for the default statement
          return " ";
          break;
      }
    }

    /// Output with default number of plot points
    void output(std::ostream& outfile)
    {
      const unsigned n_plot = 5;
      output(outfile, n_plot);
    }

    /// \short Output FE representation of soln: x,y,u_re,u_im or
    /// x,y,z,u_re,u_im at  n_plot^DIM plot points
    void output(std::ostream& outfile, const unsigned& n_plot);

    /// \short Output function for real part of full time-dependent solution
    /// u = Re( (u_r +i u_i) exp(-i omega t)
    /// at phase angle omega t = phi.
    /// x,y,u   or    x,y,z,u at n_plot plot points in each coordinate
    /// direction
    void output_real(std::ostream& outfile,
                     const double& phi,
                     const unsigned& n_plot);

    /// C_style output with default number of plot points
    void output(FILE* file_pt)
    {
      const unsigned n_plot = 5;
      output(file_pt, n_plot);
    }

    /// \short C-style output FE representation of soln: x,y,u_re,u_im or
    /// x,y,z,u_re,u_im at  n_plot^DIM plot points
    void output(FILE* file_pt, const unsigned& n_plot);

    /// Output exact soln: x,y,u_re_exact,u_im_exact
    /// or x,y,z,u_re_exact,u_im_exact at n_plot^DIM plot points
    void output_fct(std::ostream& outfile,
                    const unsigned& n_plot,
                    FiniteElement::SteadyExactSolutionFctPt exact_soln_pt);

    /// \short Output exact soln: (dummy time-dependent version to
    /// keep intel compiler happy)
    virtual void output_fct(
      std::ostream& outfile,
      const unsigned& n_plot,
      const double& time,
      FiniteElement::UnsteadyExactSolutionFctPt exact_soln_pt)
    {
      throw OomphLibError(
        "There is no time-dependent output_fct() for Helmholtz elements ",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }

    /// \short Output function for real part of full time-dependent fct
    /// u = Re( (u_r +i u_i) exp(-i omega t)
    /// at phase angle omega t = phi.
    /// x,y,u   or    x,y,z,u at n_plot plot points in each coordinate
    /// direction
    void output_real_fct(std::ostream& outfile,
                         const double& phi,
                         const unsigned& n_plot,
                         FiniteElement::SteadyExactSolutionFctPt exact_soln_pt);

    /// Get error against and norm of exact solution
    void compute_error(std::ostream& outfile,
                       FiniteElement::SteadyExactSolutionFctPt exact_soln_pt,
                       double& error,
                       double& norm);

    /// Dummy, time dependent error checker
    void compute_error(std::ostream& outfile,
                       FiniteElement::UnsteadyExactSolutionFctPt exact_soln_pt,
                       const double& time,
                       double& error,
                       double& norm)
    {
      throw OomphLibError(
        "There is no time-dependent compute_error() for Helmholtz elements",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Access function: Pointer to source function
    HelmholtzSourceFctPt& source_fct_pt()
    {
      return Source_fct_pt;
    }

    /// Access function: Pointer to source function. Const version
    HelmholtzSourceFctPt source_fct_pt() const
    {
      return Source_fct_pt;
    }

    /// Get source term at (Eulerian) position x. This function is
    /// virtual to allow overloading in multi-physics problems where
    /// the strength of the source function might be determined by
    /// another system of equations.
    inline virtual void get_source_helmholtz(const unsigned& ipt,
                                             const Vector<double>& x,
                                             std::complex<double>& source) const
    {
      // If no source function has been set, return zero
      if (Source_fct_pt == 0)
      {
        source = std::complex<double>(0.0, 0.0);
      }
      else
      {
        // Get source strength
        (*Source_fct_pt)(x, source);
      }
    }

    /// Get flux: flux[i] = du/dx_i for real and imag part
    void get_flux(const Vector<double>& s,
                  Vector<std::complex<double>>& flux) const
    {
      // Find out how many nodes there are in the element
      const unsigned n_node = nnode();

      // Set up memory for the shape and test functions
      Shape psi(n_node);
      DShape dpsidx(n_node, DIM);

      // Call the derivatives of the shape and test functions
      dshape_eulerian(s, psi, dpsidx);

      // Initialise to zero
      const std::complex<double> zero(0.0, 0.0);
      for (unsigned j = 0; j < DIM; j++)
      {
        flux[j] = zero;
      }

      // Loop over nodes
      for (unsigned l = 0; l < n_node; l++)
      {
        // Cache the complex value of the unknown
        const std::complex<double> u_value(
          this->nodal_value(l, u_index_helmholtz().real()),
          this->nodal_value(l, u_index_helmholtz().imag()));
        // Loop over derivative directions
        for (unsigned j = 0; j < DIM; j++)
        {
          flux[j] += u_value * dpsidx(l, j);
        }
      }
    }

    /// Add the element's contribution to its residual vector (wrapper)
    void fill_in_contribution_to_residuals(Vector<double>& residuals)
    {
      // Call the generic residuals function with flag set to 0
      // using a dummy matrix argument
      fill_in_generic_residual_contribution_helmholtz(
        residuals, GeneralisedElement::Dummy_matrix, 0);
    }

    /// \short Add the element's contribution to its residual vector and
    /// element Jacobian matrix (wrapper)
    void fill_in_contribution_to_jacobian(Vector<double>& residuals,
                                          DenseMatrix<double>& jacobian)
    {
      // Call the generic routine with the flag set to 1
      fill_in_generic_residual_contribution_helmholtz(residuals, jacobian, 1);
    }

    /// \short Return FE representation of function value u_helmholtz(s)
    /// at local coordinate s
    inline std::complex<double> interpolated_u_helmholtz(
      const Vector<double>& s) const
    {
      // Find number of nodes
      const unsigned n_node = nnode();

      // Local shape function
      Shape psi(n_node);

      // Find values of shape function
      shape(s, psi);

      // Initialise value of u
      std::complex<double> interpolated_u(0.0, 0.0);

      // Get the index at which the helmholtz unknown is stored
      const unsigned u_nodal_index_real = u_index_helmholtz().real();
      const unsigned u_nodal_index_imag = u_index_helmholtz().imag();

      // Loop over the local nodes and sum
      for (unsigned l = 0; l < n_node; l++)
      {
        // Make a temporary complex number from the stored data
        const std::complex<double> u_value(
          this->nodal_value(l, u_nodal_index_real),
          this->nodal_value(l, u_nodal_index_imag));
        // Add to the interpolated value
        interpolated_u += u_value * psi[l];
      }
      return interpolated_u;
    }

    /// \short Self-test: Return 0 for OK
    unsigned self_test();

  protected:
    /// \short Shape/test functions and derivs w.r.t. to global coords at
    /// local coord. s; return  Jacobian of mapping
    virtual double dshape_and_dtest_eulerian_helmholtz(
      const Vector<double>& s,
      Shape& psi,
      DShape& dpsidx,
      Shape& test,
      DShape& dtestdx) const = 0;

    /// \short Shape/test functions and derivs w.r.t. to global coords at
    /// integration point ipt; return  Jacobian of mapping
    virtual double dshape_and_dtest_eulerian_at_knot_helmholtz(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      Shape& test,
      DShape& dtestdx) const = 0;

    /// \short Compute element residual Vector only (if flag=and/or element
    /// Jacobian matrix
    virtual void fill_in_generic_residual_contribution_helmholtz(
      Vector<double>& residuals,
      DenseMatrix<double>& jacobian,
      const unsigned& flag);

    /// Pointer to source function:
    HelmholtzSourceFctPt Source_fct_pt;

    /// Pointer to square of wavenumber
    double* K_squared_pt;
  };

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  //======================================================================
  /// QHelmholtzElement elements are linear/quadrilateral/brick-shaped
  /// Helmholtz elements with isoparametric interpolation for the function.
  //======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class QHelmholtzElement :
    public virtual QElement<DIM, NNODE_1D>,
    public virtual HelmholtzEquations<DIM>
  {
  private:
    /// \short Static int that holds the number of variables at
    /// nodes: always the same
    static const unsigned Initial_Nvalue;

  public:
    ///\short  Constructor: Call constructors for QElement and
    /// Helmholtz equations
    QHelmholtzElement() : QElement<DIM, NNODE_1D>(), HelmholtzEquations<DIM>()
    {
    }

    /// Broken copy constructor
    QHelmholtzElement(const QHelmholtzElement<DIM, NNODE_1D>& dummy)
    {
      BrokenCopy::broken_copy("QHelmholtzElement");
    }

    /// Broken assignment operator
    /*void operator=(const QHelmholtzElement<DIM,NNODE_1D>&)
     {
      BrokenCopy::broken_assign("QHelmholtzElement");
      }*/

    /// \short  Required  # of `values' (pinned or dofs)
    /// at node n
    inline unsigned required_nvalue(const unsigned& n) const
    {
      return Initial_Nvalue;
    }

    /// \short Output function:
    ///  x,y,u   or    x,y,z,u
    void output(std::ostream& outfile)
    {
      HelmholtzEquations<DIM>::output(outfile);
    }

    ///  \short Output function:
    ///   x,y,u   or    x,y,z,u at n_plot^DIM plot points
    void output(std::ostream& outfile, const unsigned& n_plot)
    {
      HelmholtzEquations<DIM>::output(outfile, n_plot);
    }

    /// \short Output function for real part of full time-dependent solution
    /// u = Re( (u_r +i u_i) exp(-i omega t)
    /// at phase angle omega t = phi.
    /// x,y,u   or    x,y,z,u at n_plot plot points in each coordinate
    /// direction
    void output_real(std::ostream& outfile,
                     const double& phi,
                     const unsigned& n_plot)
    {
      HelmholtzEquations<DIM>::output_real(outfile, phi, n_plot);
    }

    /// \short C-style output function:
    ///  x,y,u   or    x,y,z,u
    void output(FILE* file_pt)
    {
      HelmholtzEquations<DIM>::output(file_pt);
    }

    ///  \short C-style output function:
    ///   x,y,u   or    x,y,z,u at n_plot^DIM plot points
    void output(FILE* file_pt, const unsigned& n_plot)
    {
      HelmholtzEquations<DIM>::output(file_pt, n_plot);
    }

    /// \short Output function for an exact solution:
    ///  x,y,u_exact   or    x,y,z,u_exact at n_plot^DIM plot points
    void output_fct(std::ostream& outfile,
                    const unsigned& n_plot,
                    FiniteElement::SteadyExactSolutionFctPt exact_soln_pt)
    {
      HelmholtzEquations<DIM>::output_fct(outfile, n_plot, exact_soln_pt);
    }

    /// \short Output function for real part of full time-dependent fct
    /// u = Re( (u_r +i u_i) exp(-i omega t)
    /// at phase angle omega t = phi.
    /// x,y,u   or    x,y,z,u at n_plot plot points in each coordinate
    /// direction
    void output_real_fct(std::ostream& outfile,
                         const double& phi,
                         const unsigned& n_plot,
                         FiniteElement::SteadyExactSolutionFctPt exact_soln_pt)
    {
      HelmholtzEquations<DIM>::output_real_fct(
        outfile, phi, n_plot, exact_soln_pt);
    }

    /// \short Output function for a time-dependent exact solution.
    ///  x,y,u_exact   or    x,y,z,u_exact at n_plot^DIM plot points
    /// (Calls the steady version)
    void output_fct(std::ostream& outfile,
                    const unsigned& n_plot,
                    const double& time,
                    FiniteElement::UnsteadyExactSolutionFctPt exact_soln_pt)
    {
      HelmholtzEquations<DIM>::output_fct(outfile, n_plot, time, exact_soln_pt);
    }

  protected:
    /// Shape, test functions & derivs. w.r.t. to global coords. Return
    /// Jacobian.
    inline double dshape_and_dtest_eulerian_helmholtz(const Vector<double>& s,
                                                      Shape& psi,
                                                      DShape& dpsidx,
                                                      Shape& test,
                                                      DShape& dtestdx) const;

    /// \short Shape, test functions & derivs. w.r.t. to global coords. at
    /// integration point ipt. Return Jacobian.
    inline double dshape_and_dtest_eulerian_at_knot_helmholtz(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      Shape& test,
      DShape& dtestdx) const;
  };

  // Inline functions:

  //======================================================================
  /// Define the shape functions and test functions and derivatives
  /// w.r.t. global coordinates and return Jacobian of mapping.
  ///
  /// Galerkin: Test functions = shape functions
  //======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  double QHelmholtzElement<DIM, NNODE_1D>::dshape_and_dtest_eulerian_helmholtz(
    const Vector<double>& s,
    Shape& psi,
    DShape& dpsidx,
    Shape& test,
    DShape& dtestdx) const
  {
    // Call the geometrical shape functions and derivatives
    const double J = this->dshape_eulerian(s, psi, dpsidx);

    // Set the test functions equal to the shape functions
    test = psi;
    dtestdx = dpsidx;

    // Return the jacobian
    return J;
  }

  //======================================================================
  /// Define the shape functions and test functions and derivatives
  /// w.r.t. global coordinates and return Jacobian of mapping.
  ///
  /// Galerkin: Test functions = shape functions
  //======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  double QHelmholtzElement<DIM, NNODE_1D>::
    dshape_and_dtest_eulerian_at_knot_helmholtz(const unsigned& ipt,
                                                Shape& psi,
                                                DShape& dpsidx,
                                                Shape& test,
                                                DShape& dtestdx) const
  {
    // Call the geometrical shape functions and derivatives
    const double J = this->dshape_eulerian_at_knot(ipt, psi, dpsidx);

    // Set the pointers of the test functions
    test = psi;
    dtestdx = dpsidx;

    // Return the jacobian
    return J;
  }

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  //=======================================================================
  /// Face geometry for the QHelmholtzElement elements: The spatial
  /// dimension of the face elements is one lower than that of the
  /// bulk element but they have the same number of points
  /// along their 1D edges.
  //=======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class FaceGeometry<QHelmholtzElement<DIM, NNODE_1D>> :
    public virtual QElement<DIM - 1, NNODE_1D>
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional QElement
    FaceGeometry() : QElement<DIM - 1, NNODE_1D>() {}
  };

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  //=======================================================================
  /// Face geometry for the 1D QHelmholtzElement elements: Point elements
  //=======================================================================
  template<unsigned NNODE_1D>
  class FaceGeometry<QHelmholtzElement<1, NNODE_1D>> :
    public virtual PointElement
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional QElement
    FaceGeometry() : PointElement() {}
  };

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  //==========================================================
  /// Helmholtz upgraded to become projectable
  //==========================================================
  template<class HELMHOLTZ_ELEMENT>
  class ProjectableHelmholtzElement :
    public virtual ProjectableElement<HELMHOLTZ_ELEMENT>
  {
  public:
    /// \short Constructor [this was only required explicitly
    /// from gcc 4.5.2 onwards...]
    ProjectableHelmholtzElement() {}

    /// \short Specify the values associated with field fld.
    /// The information is returned in a vector of pairs which comprise
    /// the Data object and the value within it, that correspond to field fld.
    Vector<std::pair<Data*, unsigned>> data_values_of_field(const unsigned& fld)
    {
#ifdef PARANOID
      if (fld > 1)
      {
        std::stringstream error_stream;
        error_stream << "Helmholtz elements only store 2 fields so fld = "
                     << fld << " is illegal \n";
        throw OomphLibError(
          error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
      }
#endif

      // Create the vector
      unsigned nnod = this->nnode();
      Vector<std::pair<Data*, unsigned>> data_values(nnod);

      // Loop over all nodes
      for (unsigned j = 0; j < nnod; j++)
      {
        // Add the data value associated field: The node itself
        data_values[j] = std::make_pair(this->node_pt(j), fld);
      }

      // Return the vector
      return data_values;
    }

    /// \short Number of fields to be projected: 2 (real and imag part)
    unsigned nfields_for_projection()
    {
      return 2;
    }

    /// \short Number of history values to be stored for fld-th field.
    /// (includes current value!)
    unsigned nhistory_values_for_projection(const unsigned& fld)
    {
#ifdef PARANOID
      if (fld > 1)
      {
        std::stringstream error_stream;
        error_stream << "Helmholtz elements only store two fields so fld = "
                     << fld << " is illegal\n";
        throw OomphLibError(
          error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
      }
#endif
      return this->node_pt(0)->ntstorage();
    }

    ///\short Number of positional history values
    /// (includes current value!)
    unsigned nhistory_values_for_coordinate_projection()
    {
      return this->node_pt(0)->position_time_stepper_pt()->ntstorage();
    }

    /// \short Return Jacobian of mapping and shape functions of field fld
    /// at local coordinate s
    double jacobian_and_shape_of_field(const unsigned& fld,
                                       const Vector<double>& s,
                                       Shape& psi)
    {
#ifdef PARANOID
      if (fld > 1)
      {
        std::stringstream error_stream;
        error_stream << "Helmholtz elements only store two fields so fld = "
                     << fld << " is illegal.\n";
        throw OomphLibError(
          error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
      }
#endif
      unsigned n_dim = this->dim();
      unsigned n_node = this->nnode();
      Shape test(n_node);
      DShape dpsidx(n_node, n_dim), dtestdx(n_node, n_dim);
      double J = this->dshape_and_dtest_eulerian_helmholtz(
        s, psi, dpsidx, test, dtestdx);
      return J;
    }

    /// \short Return interpolated field fld at local coordinate s, at time
    /// level t (t=0: present; t>0: history values)
    double get_field(const unsigned& t,
                     const unsigned& fld,
                     const Vector<double>& s)
    {
#ifdef PARANOID
      if (fld > 1)
      {
        std::stringstream error_stream;
        error_stream << "Helmholtz elements only store two fields so fld = "
                     << fld << " is illegal\n";
        throw OomphLibError(
          error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
      }
#endif
      // Find the index at which the variable is stored
      std::complex<unsigned> complex_u_nodal_index = this->u_index_helmholtz();
      unsigned u_nodal_index = 0;
      if (fld == 0)
      {
        u_nodal_index = complex_u_nodal_index.real();
      }
      else
      {
        u_nodal_index = complex_u_nodal_index.imag();
      }

      // Local shape function
      unsigned n_node = this->nnode();
      Shape psi(n_node);

      // Find values of shape function
      this->shape(s, psi);

      // Initialise value of u
      double interpolated_u = 0.0;

      // Sum over the local nodes
      for (unsigned l = 0; l < n_node; l++)
      {
        interpolated_u += this->nodal_value(t, l, u_nodal_index) * psi[l];
      }
      return interpolated_u;
    }

    /// Return number of values in field fld: One per node
    unsigned nvalue_of_field(const unsigned& fld)
    {
#ifdef PARANOID
      if (fld > 1)
      {
        std::stringstream error_stream;
        error_stream << "Helmholtz elements only store two fields so fld = "
                     << fld << " is illegal\n";
        throw OomphLibError(
          error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
      }
#endif
      return this->nnode();
    }

    /// Return local equation number of value j in field fld.
    int local_equation(const unsigned& fld, const unsigned& j)
    {
#ifdef PARANOID
      if (fld > 1)
      {
        std::stringstream error_stream;
        error_stream << "Helmholtz elements only store two fields so fld = "
                     << fld << " is illegal\n";
        throw OomphLibError(
          error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
      }
#endif
      std::complex<unsigned> complex_u_nodal_index = this->u_index_helmholtz();
      unsigned u_nodal_index = 0;
      if (fld == 0)
      {
        u_nodal_index = complex_u_nodal_index.real();
      }
      else
      {
        u_nodal_index = complex_u_nodal_index.imag();
      }
      return this->nodal_local_eqn(j, u_nodal_index);
    }

    /// \short Output FE representation of soln: x,y,u or x,y,z,u at
    /// n_plot^DIM plot points
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      HELMHOLTZ_ELEMENT::output(outfile, nplot);
    }
  };

  //=======================================================================
  /// Face geometry for element is the same as that for the underlying
  /// wrapped element
  //=======================================================================
  template<class ELEMENT>
  class FaceGeometry<ProjectableHelmholtzElement<ELEMENT>> :
    public virtual FaceGeometry<ELEMENT>
  {
  public:
    FaceGeometry() : FaceGeometry<ELEMENT>() {}
  };

  //=======================================================================
  /// Face geometry of the Face Geometry for element is the same as
  /// that for the underlying wrapped element
  //=======================================================================
  template<class ELEMENT>
  class FaceGeometry<FaceGeometry<ProjectableHelmholtzElement<ELEMENT>>> :
    public virtual FaceGeometry<FaceGeometry<ELEMENT>>
  {
  public:
    FaceGeometry() : FaceGeometry<FaceGeometry<ELEMENT>>() {}
  };

} // namespace oomph

#endif
