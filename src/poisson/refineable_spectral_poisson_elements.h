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
// Header file for refineable QSpectralPoissonElement elements

#ifndef OOMPH_REFINEABLE_SPECTRAL_POISSON_ELEMENTS_HEADER
#define OOMPH_REFINEABLE_SPECTRAL_POISSON_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif


// oomph-lib headers
#include "refineable_poisson_elements.h"
#include "generic/refineable_line_spectral_element.h"
#include "generic/refineable_quad_spectral_element.h"
#include "generic/refineable_brick_spectral_element.h"

namespace oomph
{
  //======================================================================
  /// Refineable version of 2D QSpectralPoissonElement elements
  ///
  ///
  //======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class RefineableQSpectralPoissonElement
    : public QSpectralPoissonElement<DIM, NNODE_1D>,
      public virtual RefineablePoissonEquations<DIM>,
      public virtual RefineableQSpectralElement<DIM>
  {
  public:
    /// \short Constructor: Pass refinement level to refineable quad element
    /// (default 0 = root)
    RefineableQSpectralPoissonElement()
      : RefineableElement(),
        RefineablePoissonEquations<DIM>(),
        RefineableQElement<DIM>(),
        QSpectralPoissonElement<DIM, NNODE_1D>()
    {
    }


    /// Broken copy constructor
    RefineableQSpectralPoissonElement(
      const RefineableQSpectralPoissonElement<DIM, NNODE_1D>& dummy) = delete;

    /// Broken assignment operator
    // Commented out broken assignment operator because this can lead to a
    // conflict warning when used in the virtual inheritence hierarchy.
    // Essentially the compiler doesn't realise that two separate
    // implementations of the broken function are the same and so, quite
    // rightly, it shouts.
    /*void operator=(const RefineableQSpectralPoissonElement<DIM,NNODE_1D>&) =
     * delete;*/

    /// Number of continuously interpolated values: 1
    unsigned ncont_interpolated_values() const
    {
      return 1;
    }

    /// \short Number of vertex nodes in the element
    unsigned nvertex_node() const
    {
      return QSpectralPoissonElement<DIM, NNODE_1D>::nvertex_node();
    }

    /// \short Pointer to the j-th vertex node in the element
    Node* vertex_node_pt(const unsigned& j) const
    {
      return QSpectralPoissonElement<DIM, NNODE_1D>::vertex_node_pt(j);
    }

    void assign_all_generic_local_eqn_numbers(const bool& store_local_dof_pt)
    {
      RefineableElement::assign_all_generic_local_eqn_numbers(
        store_local_dof_pt);
    }

    /// \short Function to describe the local dofs of the element. The ostream
    /// specifies the output stream to which the description
    /// is written; the string stores the currently
    /// assembled output that is ultimately written to the
    /// output stream by Data::describe_dofs(...); it is typically
    /// built up incrementally as we descend through the
    /// call hierarchy of this function when called from
    /// Problem::describe_dofs(...)
    void describe_local_dofs(std::ostream& out,
                             const std::string& current_string) const
    {
      RefineableElement::describe_local_dofs(out, current_string);
    }

    /// \short Order of recovery shape functions for Z2 error estimation:
    /// Same order as shape functions.
    unsigned nrecovery_order()
    {
      if (NNODE_1D < 4)
      {
        return (NNODE_1D - 1);
      }
      else
      {
        return 3;
      }
    }

    ///  \short Perform additional hanging node procedures for variables
    /// that are not interpolated by all nodes. Empty.
    void further_setup_hanging_nodes() {}
  };


  //=======================================================================
  /// Face geometry for the RefineableQuadPoissonElement elements: The spatial
  /// dimension of the face elements is one lower than that of the
  /// bulk element but they have the same number of points
  /// along their 1D edges.
  //=======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class FaceGeometry<RefineableQSpectralPoissonElement<DIM, NNODE_1D>>
    : public virtual QSpectralElement<DIM - 1, NNODE_1D>
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional QElement
    FaceGeometry() : QSpectralElement<DIM - 1, NNODE_1D>() {}
  };

} // namespace oomph


#endif
