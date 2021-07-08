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
// Header file for refineable unsteady heat elements

#ifndef OOMPH_REFINEABLE_UNSTEADY_HEAT_ELEMENTS_HEADER
#define OOMPH_REFINEABLE_UNSTEADY_HEAT_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// oomph-lib headers
#include "../generic/refineable_quad_element.h"
#include "../generic/refineable_brick_element.h"
#include "../generic/error_estimator.h"
#include "unsteady_heat_elements.h"

namespace oomph
{
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  //======================================================================
  /// Refineable version of Unsteady HEat equations
  ///
  ///
  //======================================================================
  template<unsigned DIM>
  class RefineableUnsteadyHeatEquations :
    public virtual UnsteadyHeatEquations<DIM>,
    public virtual RefineableElement,
    public virtual ElementWithZ2ErrorEstimator
  {
  public:
    /// \short Constructor
    RefineableUnsteadyHeatEquations() :
      UnsteadyHeatEquations<DIM>(),
      RefineableElement(),
      ElementWithZ2ErrorEstimator()
    {
    }

    /// Broken copy constructor
    RefineableUnsteadyHeatEquations(
      const RefineableUnsteadyHeatEquations<DIM>& dummy)
    {
      BrokenCopy::broken_copy("RefineableUnsteadyHeatEquations");
    }

    /// Broken assignment operator
    // Commented out broken assignment operator because this can lead to a
    // conflict warning when used in the virtual inheritence hierarchy.
    // Essentially the compiler doesn't realise that two separate
    // implementations of the broken function are the same and so, quite
    // rightly, it shouts.
    /*void operator=(const RefineableUnsteadyHeatEquations<DIM>&)
     {
      BrokenCopy::broken_assign("RefineableUnsteadyHeatEquations");
      }*/

    /// Number of 'flux' terms for Z2 error estimation
    unsigned num_Z2_flux_terms()
    {
      return DIM;
    }

    /// \short Get 'flux' for Z2 error recovery:
    /// Standard flux.from UnsteadyHeat equations
    void get_Z2_flux(const Vector<double>& s, Vector<double>& flux)
    {
      this->get_flux(s, flux);
    }

    /// \short Get the function value u in Vector.
    /// Note: Given the generality of the interface (this function
    /// is usually called from black-box documentation or interpolation
    /// routines), the values Vector sets its own size in here.
    void get_interpolated_values(const Vector<double>& s,
                                 Vector<double>& values)
    {
      // Set size of Vector: u
      values.resize(1);

      // Find number of nodes
      unsigned n_node = nnode();

      // Find the nodal index at which the unknown is stored
      unsigned u_nodal_index = this->u_index_ust_heat();

      // Local shape function
      Shape psi(n_node);

      // Find values of shape function
      shape(s, psi);

      // Initialise value of u
      values[0] = 0.0;

      // Loop over the local nodes and sum
      for (unsigned l = 0; l < n_node; l++)
      {
        values[0] += this->nodal_value(l, u_nodal_index) * psi[l];
      }
    }

    /// \short Get the function value u in Vector.
    /// Note: Given the generality of the interface (this function
    /// is usually called from black-box documentation or interpolation
    /// routines), the values Vector sets its own size in here.
    void get_interpolated_values(const unsigned& t,
                                 const Vector<double>& s,
                                 Vector<double>& values)
    {
      // Set size of Vector:
      values.resize(1);

      // Initialise
      values[0] = 0.0;

      // Find out how many nodes there are
      unsigned n_node = nnode();

      // Find the nodal index at which the unknown is stored
      unsigned u_nodal_index = this->u_index_ust_heat();

      // Shape functions
      Shape psi(n_node);
      shape(s, psi);

      // Calculate value
      for (unsigned l = 0; l < n_node; l++)
      {
        values[0] += this->nodal_value(t, l, u_nodal_index) * psi[l];
      }
    }

    ///  Further build: Copy source function pointer from father element
    void further_build()
    {
      // Get pointer to the father
      RefineableUnsteadyHeatEquations<DIM>* cast_father_element_pt =
        dynamic_cast<RefineableUnsteadyHeatEquations<DIM>*>(
          this->father_element_pt());

      // Set the source function from the parent
      this->Source_fct_pt = cast_father_element_pt->source_fct_pt();

      // Set the ALE status from the father
      this->ALE_is_disabled = cast_father_element_pt->ALE_is_disabled;
    }

  private:
    /// \short Add element's contribution to elemental residual vector and/or
    /// Jacobian matrix
    /// flag=1: compute both
    /// flag=0: compute only residual vector
    void fill_in_generic_residual_contribution_ust_heat(
      Vector<double>& residuals, DenseMatrix<double>& jacobian, unsigned flag);
  };

  //======================================================================
  /// Refineable version of 2D QUnsteadyHeatElement elements
  ///
  ///
  //======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class RefineableQUnsteadyHeatElement :
    public QUnsteadyHeatElement<DIM, NNODE_1D>,
    public virtual RefineableUnsteadyHeatEquations<DIM>,
    public virtual RefineableQElement<DIM>
  {
  public:
    /// \short Constructor
    RefineableQUnsteadyHeatElement() :
      RefineableElement(),
      RefineableUnsteadyHeatEquations<DIM>(),
      RefineableQElement<DIM>(),
      QUnsteadyHeatElement<DIM, NNODE_1D>()
    {
    }

    /// Broken copy constructor
    RefineableQUnsteadyHeatElement(
      const RefineableQUnsteadyHeatElement<DIM, NNODE_1D>& dummy)
    {
      BrokenCopy::broken_copy("RefineableQuadUnsteadyHeatElement");
    }

    /// Broken assignment operator
    /*void operator=(const RefineableQUnsteadyHeatElement<DIM,NNODE_1D>&)
     {
      BrokenCopy::broken_assign("RefineableQuadUnsteadyHeatElement");
      }*/

    /// Number of continuously interpolated values: 1
    unsigned ncont_interpolated_values() const
    {
      return 1;
    }

    /// \short Number of vertex nodes in the element
    unsigned nvertex_node() const
    {
      return QUnsteadyHeatElement<DIM, NNODE_1D>::nvertex_node();
    }

    /// \short Pointer to the j-th vertex node in the element
    Node* vertex_node_pt(const unsigned& j) const
    {
      return QUnsteadyHeatElement<DIM, NNODE_1D>::vertex_node_pt(j);
    }

    /// Rebuild from sons: empty
    void rebuild_from_sons(Mesh*& mesh_pt) {}

    /// \short Order of recovery shape functions for Z2 error estimation:
    /// Same order as shape functions.
    unsigned nrecovery_order()
    {
      return (NNODE_1D - 1);
    }

    ///  \short Perform additional hanging node procedures for variables
    /// that are not interpolated by all nodes. Empty.
    void further_setup_hanging_nodes() {}
  };
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  //=======================================================================
  /// Face geometry for the RefineableQuadUnsteadyHeatElement elements:
  /// The spatial
  /// dimension of the face elements is one lower than that of the
  /// bulk element but they have the same number of points
  /// along their 1D edges.
  //=======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class FaceGeometry<RefineableQUnsteadyHeatElement<DIM, NNODE_1D>> :
    public virtual QElement<DIM - 1, NNODE_1D>
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional QElement
    FaceGeometry() : QElement<DIM - 1, NNODE_1D>() {}
  };

} // namespace oomph

#endif
