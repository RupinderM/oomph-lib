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
// Header file for elements that are used to apply prescribed flux
// boundary conditions to the Advection Diffusion equations
#ifndef OOMPH_ADV_DIFF_FLUX_ELEMENTS_HEADER
#define OOMPH_ADV_DIFF_FLUX_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// oomph-lib ncludes
#include "generic/Qelements.h"

namespace oomph
{
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  //======================================================================
  /// \short A class for elements that allow the imposition of an
  /// applied flux on the boundaries of Advection Diffusion elements.
  /// The element geometry is obtained from the  FaceGeometry<ELEMENT>
  /// policy class.
  //======================================================================
  template<class ELEMENT>
  class AdvectionDiffusionFluxElement :
    public virtual FaceGeometry<ELEMENT>,
    public virtual FaceElement
  {
  public:
    /// \short Function pointer to the prescribed-flux function fct(x,f(x)) --
    /// x is a Vector!
    typedef void (*AdvectionDiffusionPrescribedFluxFctPt)(
      const Vector<double>& x, double& flux);

    /// \short Constructor, takes the pointer to the "bulk" element
    /// and the index of the face to be created
    AdvectionDiffusionFluxElement(FiniteElement* const& bulk_el_pt,
                                  const int& face_index);

    ///\short  Broken empty constructor
    AdvectionDiffusionFluxElement()
    {
      throw OomphLibError(
        "Don't call empty constructor for AdvectionDiffusionFluxElement",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Broken copy constructor
    AdvectionDiffusionFluxElement(const AdvectionDiffusionFluxElement& dummy)
    {
      BrokenCopy::broken_copy("AdvectionDiffusionFluxElement");
    }

    /// Broken assignment operator
    void operator=(const AdvectionDiffusionFluxElement&)
    {
      BrokenCopy::broken_assign("AdvectionDiffusionFluxElement");
    }

    /// Access function for the prescribed-flux function pointer
    AdvectionDiffusionPrescribedFluxFctPt& flux_fct_pt()
    {
      return Flux_fct_pt;
    }

    /// Add the element's contribution to its residual vector
    inline void fill_in_contribution_to_residuals(Vector<double>& residuals)
    {
      // Call the generic residuals function with flag set to 0
      // using a dummy matrix
      fill_in_generic_residual_contribution_adv_diff_flux(
        residuals, GeneralisedElement::Dummy_matrix, 0);
    }

    /// \short Add the element's contribution to its residual vector and
    /// its Jacobian matrix
    inline void fill_in_contribution_to_jacobian(Vector<double>& residuals,
                                                 DenseMatrix<double>& jacobian)
    {
      // Call the generic routine with the flag set to 1
      fill_in_generic_residual_contribution_adv_diff_flux(
        residuals, jacobian, 1);
    }

    /// Specify the value of nodal zeta from the face geometry
    /// \short The "global" intrinsic coordinate of the element when
    /// viewed as part of a geometric object should be given by
    /// the FaceElement representation, by default (needed to break
    /// indeterminacy if bulk element is SolidElement)
    double zeta_nodal(const unsigned& n,
                      const unsigned& k,
                      const unsigned& i) const
    {
      return FaceElement::zeta_nodal(n, k, i);
    }

    /// \short Output function -- forward to broken version in FiniteElement
    /// until somebody decides what exactly they want to plot here...
    void output(std::ostream& outfile)
    {
      FiniteElement::output(outfile);
    }

    /// \short Output function -- forward to broken version in FiniteElement
    /// until somebody decides what exactly they want to plot here...
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      FiniteElement::output(outfile, nplot);
    }

  protected:
    /// \short Function to compute the shape and test functions and to return
    /// the Jacobian of mapping between local and global (Eulerian)
    /// coordinates
    inline double shape_and_test(const Vector<double>& s,
                                 Shape& psi,
                                 Shape& test) const
    {
      // Find number of nodes
      unsigned n_node = nnode();

      // Get the shape functions
      shape(s, psi);

      // Set the test functions to be the same as the shape functions
      for (unsigned i = 0; i < n_node; i++)
      {
        test[i] = psi[i];
      }

      // Return the value of the jacobian
      return J_eulerian(s);
    }

    /// \short Function to compute the shape and test functions and to return
    /// the Jacobian of mapping between local and global (Eulerian)
    /// coordinates
    inline double shape_and_test_at_knot(const unsigned& ipt,
                                         Shape& psi,
                                         Shape& test) const
    {
      // Find number of nodes
      unsigned n_node = nnode();

      // Get the shape functions
      shape_at_knot(ipt, psi);

      // Set the test functions to be the same as the shape functions
      for (unsigned i = 0; i < n_node; i++)
      {
        test[i] = psi[i];
      }

      // Return the value of the jacobian
      return J_eulerian_at_knot(ipt);
    }

    /// \short Function to calculate the prescribed flux at a given spatial
    /// position
    void get_flux(const Vector<double>& x, double& flux)
    {
      // If the function pointer is zero return zero
      if (Flux_fct_pt == 0)
      {
        flux = 0.0;
      }
      // Otherwise call the function
      else
      {
        (*Flux_fct_pt)(x, flux);
      }
    }

  private:
    /// \short Add the element's contribution to its residual vector.
    /// flag=1(or 0): do (or don't) compute the Jacobian as well.
    void fill_in_generic_residual_contribution_adv_diff_flux(
      Vector<double>& residuals, DenseMatrix<double>& jacobian, unsigned flag);

    /// Function pointer to the (global) prescribed-flux function
    AdvectionDiffusionPrescribedFluxFctPt Flux_fct_pt;

    /// The spatial dimension of the problem
    unsigned Dim;

    /// The index at which the unknown is stored at the nodes
    unsigned U_index_adv_diff;
  };

  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

  //===========================================================================
  /// \short Constructor, takes the pointer to the "bulk" element and the index
  /// of the face to be created
  //===========================================================================
  template<class ELEMENT>
  AdvectionDiffusionFluxElement<ELEMENT>::AdvectionDiffusionFluxElement(
    FiniteElement* const& bulk_el_pt, const int& face_index) :
    FaceGeometry<ELEMENT>(), FaceElement()
  {
    // Let the bulk element build the FaceElement, i.e. setup the pointers
    // to its nodes (by referring to the appropriate nodes in the bulk
    // element), etc.
    bulk_el_pt->build_face_element(face_index, this);

#ifdef PARANOID
    {
      // Check that the element is not a refineable 3d element
      ELEMENT* elem_pt = dynamic_cast<ELEMENT*>(bulk_el_pt);
      // If it's three-d
      if (elem_pt->dim() == 3)
      {
        // Is it refineable
        RefineableElement* ref_el_pt =
          dynamic_cast<RefineableElement*>(elem_pt);
        if (ref_el_pt != 0)
        {
          if (this->has_hanging_nodes())
          {
            throw OomphLibError("This flux element will not work correctly if "
                                "nodes are hanging\n",
                                OOMPH_CURRENT_FUNCTION,
                                OOMPH_EXCEPTION_LOCATION);
          }
        }
      }
    }
#endif

    // Initialise the prescribed-flux function pointer to zero
    Flux_fct_pt = 0;

    // Extract the dimension of the problem from the dimension of
    // the first node
    Dim = this->node_pt(0)->ndim();

    // Set up U_index_adv_diff. Initialise to zero, which probably won't change
    // in most cases, oh well, the price we pay for generality
    U_index_adv_diff = 0;

    // Cast to the appropriate AdvectionDiffusionEquation so that we can
    // find the index at which the variable is stored
    // We assume that the dimension of the full problem is the same
    // as the dimension of the node, if this is not the case you will have
    // to write custom elements, sorry
    switch (Dim)
    {
        // One dimensional problem
      case 1:
      {
        AdvectionDiffusionEquations<1>* eqn_pt =
          dynamic_cast<AdvectionDiffusionEquations<1>*>(bulk_el_pt);
        // If the cast has failed die
        if (eqn_pt == 0)
        {
          std::string error_string =
            "Bulk element must inherit from AdvectionDiffusionEquations.";
          error_string +=
            "Nodes are one dimensional, but cannot cast the bulk element to\n";
          error_string += "AdvectionDiffusionEquations<1>\n.";
          error_string += "If you desire this functionality, you must "
                          "implement it yourself\n";

          throw OomphLibError(
            error_string, OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
        }
        // Otherwise read out the value
        else
        {
          // Read the index from the (cast) bulk element
          U_index_adv_diff = eqn_pt->u_index_adv_diff();
        }
      }
      break;

      // Two dimensional problem
      case 2:
      {
        AdvectionDiffusionEquations<2>* eqn_pt =
          dynamic_cast<AdvectionDiffusionEquations<2>*>(bulk_el_pt);
        // If the cast has failed die
        if (eqn_pt == 0)
        {
          std::string error_string =
            "Bulk element must inherit from AdvectionDiffusionEquations.";
          error_string +=
            "Nodes are two dimensional, but cannot cast the bulk element to\n";
          error_string += "AdvectionDiffusionEquations<2>\n.";
          error_string += "If you desire this functionality, you must "
                          "implement it yourself\n";

          throw OomphLibError(
            error_string, OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
        }
        else
        {
          // Read the index from the (cast) bulk element.
          U_index_adv_diff = eqn_pt->u_index_adv_diff();
        }
      }
      break;

      // Three dimensional problem
      case 3:
      {
        AdvectionDiffusionEquations<3>* eqn_pt =
          dynamic_cast<AdvectionDiffusionEquations<3>*>(bulk_el_pt);
        // If the cast has failed die
        if (eqn_pt == 0)
        {
          std::string error_string =
            "Bulk element must inherit from AdvectionDiffusionEquations.";
          error_string += "Nodes are three dimensional, but cannot cast the "
                          "bulk element to\n";
          error_string += "AdvectionDiffusionEquations<3>\n.";
          error_string += "If you desire this functionality, you must "
                          "implement it yourself\n";

          throw OomphLibError(
            error_string, OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
        }
        else
        {
          // Read the index from the (cast) bulk element.
          U_index_adv_diff = eqn_pt->u_index_adv_diff();
        }
      }
      break;

      // Any other case is an error
      default:
        std::ostringstream error_stream;
        error_stream << "Dimension of node is " << Dim
                     << ". It should be 1,2, or 3!" << std::endl;

        throw OomphLibError(
          error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
        break;
    }
  }

  //===========================================================================
  /// Compute the element's residual vector and the (zero) Jacobian matrix.
  //===========================================================================
  template<class ELEMENT>
  void AdvectionDiffusionFluxElement<ELEMENT>::
    fill_in_generic_residual_contribution_adv_diff_flux(
      Vector<double>& residuals, DenseMatrix<double>& jacobian, unsigned flag)
  {
    // Find out how many nodes there are
    const unsigned n_node = nnode();

    // Set up memory for the shape and test functions
    Shape psif(n_node), testf(n_node);

    // Set the value of n_intpt
    const unsigned n_intpt = integral_pt()->nweight();

    // Set the Vector to hold local coordinates
    Vector<double> s(Dim - 1);

    // Integers used to store the local equation number and local unknown
    // indices for the residuals and jacobians
    int local_eqn = 0;

    // Locally cache the index at which the variable is stored
    const unsigned u_index_adv_diff = U_index_adv_diff;

    // Loop over the integration points
    //--------------------------------
    for (unsigned ipt = 0; ipt < n_intpt; ipt++)
    {
      // Assign values of s
      for (unsigned i = 0; i < (Dim - 1); i++)
      {
        s[i] = integral_pt()->knot(ipt, i);
      }

      // Get the integral weight
      double w = integral_pt()->weight(ipt);

      // Find the shape and test functions and return the Jacobian
      // of the mapping
      double J = shape_and_test(s, psif, testf);

      // Premultiply the weights and the Jacobian
      double W = w * J;

      // Need to find position to feed into flux function
      Vector<double> interpolated_x(Dim, 0.0);

      // Calculate position
      for (unsigned l = 0; l < n_node; l++)
      {
        // Loop over coordinate directions
        for (unsigned i = 0; i < Dim; i++)
        {
          interpolated_x[i] += nodal_position(l, i) * psif[l];
        }
      }

      // Get the imposed flux
      double flux;
      get_flux(interpolated_x, flux);

      // Now add to the appropriate equations

      // Loop over the test functions
      for (unsigned l = 0; l < n_node; l++)
      {
        // Set the local equation number
        local_eqn = nodal_local_eqn(l, u_index_adv_diff);
        /*IF it's not a boundary condition*/
        if (local_eqn >= 0)
        {
          // Add the prescribed flux terms
          residuals[local_eqn] += flux * testf[l] * W;

          // Imposed traction doesn't depend upon the solution,
          // --> the Jacobian is always zero, so no Jacobian
          // terms are required
        }
      }
    }
  }

} // namespace oomph

#endif
