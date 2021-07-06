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
// Header file for classes that define hp-refineable element objects

// Include guard to prevent multiple inclusions of the header
#ifndef OOMPH_HP_REFINEABLE_ELEMENTS_HEADER
#define OOMPH_HP_REFINEABLE_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#include "refineable_elements.h"
#include "refineable_line_element.h"
#include "refineable_quad_element.h"
#include "refineable_brick_element.h"
#include "mesh.h"

namespace oomph
{
  //======================================================================
  /// p-refineable version of RefineableQElement<1,INITIAL_NNODE_1D>.
  /// Generic class definitions
  //======================================================================
  template<unsigned INITIAL_NNODE_1D>
  class PRefineableQElement<1, INITIAL_NNODE_1D> :
    public RefineableQElement<1>,
    public virtual QElement<1, INITIAL_NNODE_1D>,
    public virtual PRefineableElement
  {
  public:
    /// Constructor
    PRefineableQElement() : PRefineableElement(), RefineableQElement<1>() {}

    /// Destructor
    virtual ~PRefineableQElement() {}

    /// \short Initial setup of element (set the correct p-order and
    /// integration scheme) If an adopted father is specified, information
    /// from this is used instead of using the father found from the tree.
    void initial_setup(Tree* const& adopted_father_pt = 0,
                       const unsigned& initial_p_order = 0);

    /// \short Pre-build (search father for required nodes which may already
    /// exist)
    void pre_build(Mesh*& mesh_pt, Vector<Node*>& new_node_pt);

    /// \short p-refine the element (refine if inc>0, unrefine if inc<0).
    void p_refine(const int& inc,
                  Mesh* const& mesh_pt,
                  GeneralisedElement* const& clone_pt);

    /// Overload the shape functions
    void shape(const Vector<double>& s, Shape& psi) const;

    void dshape_local(const Vector<double>& s, Shape& psi, DShape& dpsi) const;

    void d2shape_local(const Vector<double>& s,
                       Shape& psi,
                       DShape& dpsids,
                       DShape& d2psids) const;

    /// \short Perform additional hanging node procedures for variables
    /// that are not interpolated by all nodes (e.g. lower order interpolations
    /// for the pressure in Taylor Hood).
    void further_setup_hanging_nodes() {}

    /// \short Returns the number of nodes along each edge of the element.
    /// Overloaded to return the (variable) p-order rather than the template
    /// argument.
    unsigned nnode_1d() const
    {
      return this->p_order();
    }

    /// Get the initial P_order
    unsigned initial_p_order() const
    {
      return INITIAL_NNODE_1D;
    }

    // Overloaded from QElement<1,NNODE_1D> to use nnode_1d() instead of
    // template argument.
    Node* get_node_at_local_coordinate(const Vector<double>& s) const;

    Node* node_created_by_son_of_neighbour(const Vector<double>& s_fraction,
                                           bool& is_periodic);

    // Overload nodal positions -- these elements have GLL-spaced nodes.
    /// Get local coordinates of node j in the element; vector sets its own size
    void local_coordinate_of_node(const unsigned& n, Vector<double>& s) const;

    /// Get the local fractino of node j in the element
    void local_fraction_of_node(const unsigned& n, Vector<double>& s_fraction);

    /// The local one-d fraction is the same
    double local_one_d_fraction_of_node(const unsigned& n1d, const unsigned& i);

    /// Rebuild the element. This needs to find any nodes in the sons which
    /// are still required.
    void rebuild_from_sons(Mesh*& mesh_pt);

    /// \short Check the integrity of interpolated values across element
    /// boundaries.
    void check_integrity(double& max_error);

  protected:
    /// \short Set up hanging node information. Empty for 1D elements.
    void binary_hang_helper(const int& value_id,
                            const int& my_edge,
                            std::ofstream& output_hangfile);
  };

  //=======================================================================
  /// p-refineable version of RefineableQElement<2,INITIAL_NNODE_1D>.
  //=======================================================================
  template<unsigned INITIAL_NNODE_1D>
  class PRefineableQElement<2, INITIAL_NNODE_1D> :
    public RefineableQElement<2>,
    public virtual QElement<2, INITIAL_NNODE_1D>,
    public virtual PRefineableElement
  {
  public:
    /// Constructor
    PRefineableQElement() : PRefineableElement(), RefineableQElement<2>() {}

    /// Destructor
    virtual ~PRefineableQElement() {}

    /// \short Initial setup of element (set the correct p-order and
    /// integration scheme) If an adopted father is specified, information
    /// from this is used instead of using the father found from the tree.
    void initial_setup(Tree* const& adopted_father_pt = 0,
                       const unsigned& initial_p_order = 0);

    /// \short Pre-build (search father for required nodes which may already
    /// exist)
    void pre_build(Mesh*& mesh_pt, Vector<Node*>& new_node_pt);

    /// \short p-refine the element (refine if inc>0, unrefine if inc<0).
    void p_refine(const int& inc,
                  Mesh* const& mesh_pt,
                  GeneralisedElement* const& clone_pt);

    /// Overload the shape functions
    void shape(const Vector<double>& s, Shape& psi) const;

    void dshape_local(const Vector<double>& s, Shape& psi, DShape& dpsi) const;

    void d2shape_local(const Vector<double>& s,
                       Shape& psi,
                       DShape& dpsids,
                       DShape& d2psids) const;

    /// \short Perform additional hanging node procedures for variables
    /// that are not interpolated by all nodes (e.g. lower order interpolations
    /// for the pressure in Taylor Hood).
    void further_setup_hanging_nodes() {}

    /// \short Returns the number of nodes along each edge of the element.
    /// Overloaded to return the (variable) p-order rather than the template
    /// argument.
    unsigned nnode_1d() const
    {
      return this->p_order();
    }

    /// Get the initial P_order
    unsigned initial_p_order() const
    {
      return INITIAL_NNODE_1D;
    }

    // Overloaded from QElement<2,NNODE_1D> to use nnode_1d() instead of
    // template argument.
    Node* get_node_at_local_coordinate(const Vector<double>& s) const;

    Node* node_created_by_neighbour(const Vector<double>& s_fraction,
                                    bool& is_periodic);

    Node* node_created_by_son_of_neighbour(const Vector<double>& s_fraction,
                                           bool& is_periodic);

    // Overload nodal positions -- these elements have GLL-spaced nodes.
    /// Get local coordinates of node j in the element; vector sets its own size
    void local_coordinate_of_node(const unsigned& n, Vector<double>& s) const;

    /// Get the local fractino of node j in the element
    void local_fraction_of_node(const unsigned& n, Vector<double>& s_fraction);

    /// The local one-d fraction is the same
    double local_one_d_fraction_of_node(const unsigned& n1d, const unsigned& i);

    /// Rebuild the element. This needs to find any nodes in the sons which
    /// are still required.
    void rebuild_from_sons(Mesh*& mesh_pt);

    /// \short Check the integrity of interpolated values across element
    /// boundaries.
    /// Note: with the mortar method, continuity is enforced weakly across non-
    /// conforming element boundaries, so it makes no sense to check the
    /// continuity of interpolated values across these boundaries.
    void check_integrity(double& max_error);

  protected:
    /// \short Set up hanging node information.
    /// Overloaded to implement the mortar method rather than constrained
    /// approximation. This enforces continuity weakly via an integral matching
    /// condition at non-conforming element boundaries.
    void quad_hang_helper(const int& value_id,
                          const int& my_edge,
                          std::ofstream& output_hangfile);
  };

  //=======================================================================
  /// p-refineable version of RefineableQElement<3,INITIAL_NNODE_1D>.
  //=======================================================================
  template<unsigned INITIAL_NNODE_1D>
  class PRefineableQElement<3, INITIAL_NNODE_1D> :
    public RefineableQElement<3>,
    public virtual QElement<3, INITIAL_NNODE_1D>,
    public virtual PRefineableElement
  {
  public:
    /// Constructor
    PRefineableQElement() : PRefineableElement(), RefineableQElement<3>() {}

    /// Destructor
    virtual ~PRefineableQElement() {}

    /// \short Initial setup of element (set the correct p-order and
    /// integration scheme) If an adopted father is specified, information
    /// from this is used instead of using the father found from the tree.
    void initial_setup(Tree* const& adopted_father_pt = 0,
                       const unsigned& initial_p_order = 0);

    /// \short Pre-build (search father for required nodes which may already
    /// exist)
    void pre_build(Mesh*& mesh_pt, Vector<Node*>& new_node_pt);

    /// \short p-refine the element (refine if inc>0, unrefine if inc<0).
    void p_refine(const int& inc,
                  Mesh* const& mesh_pt,
                  GeneralisedElement* const& clone_pt);

    /// Overload the shape functions
    void shape(const Vector<double>& s, Shape& psi) const;

    void dshape_local(const Vector<double>& s, Shape& psi, DShape& dpsi) const;

    void d2shape_local(const Vector<double>& s,
                       Shape& psi,
                       DShape& dpsids,
                       DShape& d2psids) const;

    /// \short Perform additional hanging node procedures for variables
    /// that are not interpolated by all nodes (e.g. lower order interpolations
    /// for the pressure in Taylor Hood).
    void further_setup_hanging_nodes() {}

    /// \short Returns the number of nodes along each edge of the element.
    /// Overloaded to return the (variable) p-order rather than the template
    /// argument.
    unsigned nnode_1d() const
    {
      return this->p_order();
    }

    /// Get the initial P_order
    unsigned initial_p_order() const
    {
      return INITIAL_NNODE_1D;
    }

    // Overloaded from QElement<3,NNODE_1D> to use nnode_1d() instead of
    // template argument.
    Node* get_node_at_local_coordinate(const Vector<double>& s) const;

    Node* node_created_by_neighbour(const Vector<double>& s_fraction,
                                    bool& is_periodic);

    Node* node_created_by_son_of_neighbour(const Vector<double>& s_fraction,
                                           bool& is_periodic);

    // Overload nodal positions -- these elements have GLL-spaced nodes.
    /// Get local coordinates of node j in the element; vector sets its own size
    void local_coordinate_of_node(const unsigned& n, Vector<double>& s) const;

    /// Get the local fractino of node j in the element
    void local_fraction_of_node(const unsigned& n, Vector<double>& s_fraction);

    /// The local one-d fraction is the same
    double local_one_d_fraction_of_node(const unsigned& n1d, const unsigned& i);

    /// Rebuild the element. This needs to find any nodes in the sons which
    /// are still required.
    void rebuild_from_sons(Mesh*& mesh_pt);

    /// \short Check the integrity of interpolated values across element
    /// boundaries.
    /// Note: with the mortar method, continuity is enforced weakly across non-
    /// conforming element boundaries, so it makes no sense to check the
    /// continuity of interpolated values across these boundaries.
    void check_integrity(double& max_error);

  protected:
    /// \short Set up hanging node information.
    /// Overloaded to implement the mortar method rather than constrained
    /// approximation. This enforces continuity weakly via an integral matching
    /// condition at non-conforming element boundaries.
    void oc_hang_helper(const int& value_id,
                        const int& my_face,
                        std::ofstream& output_hangfile);
  };

} // namespace oomph

#endif
