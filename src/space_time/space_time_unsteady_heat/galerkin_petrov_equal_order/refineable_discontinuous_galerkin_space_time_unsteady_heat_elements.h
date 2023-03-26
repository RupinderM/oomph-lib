// LIC// ====================================================================
// LIC// This file forms part of oomph-lib, the object-oriented,
// LIC// multi-physics finite-element library, available
// LIC// at http://www.oomph-lib.org.
// LIC//
// LIC// Copyright (C) 2006-2023 Matthias Heil and Andrew Hazel
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
#ifndef OOMPH_REFINEABLE_DISCONTINUOUS_GALERKIN_SPACE_TIME_UNSTEADY_HEAT_ELEMENTS_HEADER
#define OOMPH_REFINEABLE_DISCONTINUOUS_GALERKIN_SPACE_TIME_UNSTEADY_HEAT_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// Oomph-lib headers
#include "generic/refineable_quad_element.h"
#include "generic/refineable_brick_element.h"
#include "generic/error_estimator.h"
#include "discontinuous_galerkin_space_time_unsteady_heat_elements.h"

/// //////////////////////////////////////////////////////////////////////
/// //////////////////////////////////////////////////////////////////////
/// //////////////////////////////////////////////////////////////////////

namespace oomph
{
  //======================================================================
  /// Refineable version of Unsteady Heat equations
  //======================================================================
  template<unsigned SPATIAL_DIM>
  class RefineableSpaceTimeUnsteadyHeatEquations
    : public virtual SpaceTimeUnsteadyHeatEquations<SPATIAL_DIM>,
      public virtual RefineableElement,
      public virtual ElementWithZ2ErrorEstimator
  {
  public:
    /// Constructor
    RefineableSpaceTimeUnsteadyHeatEquations()
      : SpaceTimeUnsteadyHeatEquations<SPATIAL_DIM>(),
        RefineableElement(),
        ElementWithZ2ErrorEstimator()
    {
    }


    /// Broken copy constructor
    RefineableSpaceTimeUnsteadyHeatEquations(
      const RefineableSpaceTimeUnsteadyHeatEquations<SPATIAL_DIM>& dummy) =
      delete;

    /// Number of 'flux' terms for Z2 error estimation
    unsigned num_Z2_flux_terms() override
    {
      // The flux terms are associated with spatial AND temporal derivatives
      return SPATIAL_DIM + 1;
    } // End of num_Z2_flux_terms


    /// Get 'flux' for Z2 error recovery:
    /// Different to the get_flux function in the base class as we also
    /// have to include du/dt as we're doing temporal adaptivity too
    void get_Z2_flux(const Vector<double>& s, Vector<double>& flux) override
    {
      // Find out how many nodes there are in the element
      unsigned n_node = nnode();

      // Find the index at which the variable is stored
      unsigned u_nodal_index = this->u_index_ust_heat();

      // Set up memory for the shape and test functions
      Shape psi(n_node);

      // Set up memory for the derivatives of the shape and test functions
      DShape dpsidx(n_node, SPATIAL_DIM + 1);

      // Call the derivatives of the shape and test functions
      dshape_eulerian(s, psi, dpsidx);

      // Loop over the entries of the flux vector
      for (unsigned j = 0; j < SPATIAL_DIM + 1; j++)
      {
        // Initialise j-th flux entry to zero
        flux[j] = 0.0;
      }

      // Loop over nodes
      for (unsigned l = 0; l < n_node; l++)
      {
        // Loop over derivative directions
        for (unsigned j = 0; j < SPATIAL_DIM + 1; j++)
        {
          // Update the flux value
          flux[j] += this->nodal_value(l, u_nodal_index) * dpsidx(l, j);
        }
      } // for (unsigned l=0;l<n_node;l++)
    } // End of get_Z2_flux


    /// Get the function value u in Vector.
    /// Note: Given the generality of the interface (this function is usually
    /// called from black-box documentation or interpolation routines), the
    /// values Vector sets its own size in here.
    void get_interpolated_values(const Vector<double>& s,
                                 Vector<double>& values) override
    {
      // Set the size of the vector u
      values.resize(1);

      // Find the number of nodes
      unsigned n_node = nnode();

      // Find the nodal index at which the unknown is stored
      unsigned u_nodal_index = this->u_index_ust_heat();

      // Local shape function
      Shape psi(n_node);

      // Find values of shape function
      shape(s, psi);

      // Initialise the value of u
      values[0] = 0.0;

      // Loop over the local nodes and sum
      for (unsigned l = 0; l < n_node; l++)
      {
        // Update the solution value
        values[0] += this->nodal_value(l, u_nodal_index) * psi[l];
      }
    } // End of get_interpolated_values


    /// Get the function value u in Vector.
    /// Note: Given the generality of the interface (this function is usually
    /// called from black-box documentation or interpolation routines), the
    /// values Vector sets its own size in here.
    void get_interpolated_values(const unsigned& t,
                                 const Vector<double>& s,
                                 Vector<double>& values) override
    {
      // Set the size of the vector u
      values.resize(1);

      // Find the number of nodes
      unsigned n_node = nnode();

      // Find the nodal index at which the unknown is stored
      unsigned u_nodal_index = this->u_index_ust_heat();

      // Local shape function
      Shape psi(n_node);

      // Find values of shape function
      shape(s, psi);

      // Initialise the value of u
      values[0] = 0.0;

      // Loop over the local nodes and sum
      for (unsigned l = 0; l < n_node; l++)
      {
        // Update the solution value
        values[0] += this->nodal_value(t, l, u_nodal_index) * psi[l];
      }
    } // End of get_interpolated_values


    /// Further build: Copy source function pointer from father element
    void further_build() override
    {
      // Get pointer to the father
      RefineableSpaceTimeUnsteadyHeatEquations<SPATIAL_DIM>*
        cast_father_element_pt =
          dynamic_cast<RefineableSpaceTimeUnsteadyHeatEquations<SPATIAL_DIM>*>(
            this->father_element_pt());

      // Get the source function from the parent and store it
      this->Source_fct_pt = cast_father_element_pt->source_fct_pt();

      // Set the ALE status from the father
      this->ALE_is_disabled = cast_father_element_pt->ALE_is_disabled;
    } // End of further_build

  private:
    /// Add element's contribution to elemental residual vector and/or
    /// Jacobian matrix
    /// flag=0: compute residual vector only
    /// flag=1: compute both
    void fill_in_generic_residual_contribution_ust_heat(
      Vector<double>& residuals,
      DenseMatrix<double>& jacobian,
      const unsigned& flag) override;
  }; // End of RefineableSpaceTimeUnsteadyHeatEquations class


  //======================================================================
  /// Refineable version of 2D QUnsteadyHeatSpaceTimeElement elements
  //======================================================================
  template<unsigned SPATIAL_DIM, unsigned NNODE_1D>
  class RefineableQUnsteadyHeatSpaceTimeElement
    : public QUnsteadyHeatSpaceTimeElement<SPATIAL_DIM, NNODE_1D>,
      public virtual RefineableSpaceTimeUnsteadyHeatEquations<SPATIAL_DIM>,
      public virtual RefineableQElement<SPATIAL_DIM + 1>
  {
  public:
    /// Constructor
    RefineableQUnsteadyHeatSpaceTimeElement()
      : RefineableElement(),
        RefineableSpaceTimeUnsteadyHeatEquations<SPATIAL_DIM>(),
        RefineableQElement<SPATIAL_DIM + 1>(),
        QUnsteadyHeatSpaceTimeElement<SPATIAL_DIM, NNODE_1D>()
    {
    }


    /// Broken copy constructor
    RefineableQUnsteadyHeatSpaceTimeElement(
      const RefineableQUnsteadyHeatSpaceTimeElement<SPATIAL_DIM, NNODE_1D>&
        dummy) = delete;

    /// Rebuild from sons (empty)
    void rebuild_from_sons(Mesh*& mesh_pt) override {}


    /// Perform additional hanging node procedures for variables
    /// that are not interpolated by all nodes (empty).
    void further_setup_hanging_nodes() override {}


    /// Number of continuously interpolated values: 1
    unsigned ncont_interpolated_values() const override
    {
      // Return the appropriate value
      return 1;
    } // End of ncont_interpolated_values


    /// Number of vertex nodes in the element
    unsigned nvertex_node() const override
    {
      // Call the base class function
      return QUnsteadyHeatSpaceTimeElement<SPATIAL_DIM,
                                           NNODE_1D>::nvertex_node();
    } // End of nvertex_node


    /// Pointer to the j-th vertex node in the element
    Node* vertex_node_pt(const unsigned& j) const override
    {
      // Call the base class function
      return QUnsteadyHeatSpaceTimeElement<SPATIAL_DIM,
                                           NNODE_1D>::vertex_node_pt(j);
    } // End of vertex_node_pt


    /// Order of recovery shape functions for Z2 error estimation:
    /// Same order as shape functions.
    unsigned nrecovery_order() override
    {
      // Return the approriate value
      return (NNODE_1D - 1);
    } // End of nrecovery_order
  }; // End of RefineableQUnsteadyHeatSpaceTimeElement class


  /// /////////////////////////////////////////////////////////////////////
  /// /////////////////////////////////////////////////////////////////////
  /// /////////////////////////////////////////////////////////////////////


  //=======================================================================
  /// Face geometry for the RefineableQuadUnsteadyHeatSpaceTimeElement elements:
  /// The spatial
  /// dimension of the face elements is one lower than that of the
  /// bulk element but they have the same number of points
  /// along their 1D edges.
  //=======================================================================
  template<unsigned SPATIAL_DIM, unsigned NNODE_1D>
  class FaceGeometry<
    RefineableQUnsteadyHeatSpaceTimeElement<SPATIAL_DIM, NNODE_1D>>
    : public virtual QElement<SPATIAL_DIM, NNODE_1D>
  {
  public:
    /// Constructor: Call the constructor for the
    /// appropriate lower-dimensional QElement
    FaceGeometry() : QElement<SPATIAL_DIM, NNODE_1D>() {}
  }; // End of FaceGeometry<RefineableQUnsteadyHeatSpaceTimeElement... class
} // End of namespace oomph

#endif
