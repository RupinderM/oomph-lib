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
// Header file for TAdvectionDiffusionReaction elements
#ifndef OOMPH_TADVECTION_DIFFUSION_REACTION_ELEMENTS_HEADER
#define OOMPH_TADVECTION_DIFFUSION_REACTION_ELEMENTS_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif


// OOMPH-LIB headers
#include "../generic/nodes.h"
#include "../generic/oomph_utilities.h"
#include "../generic/Telements.h"
#include "../generic/error_estimator.h"
#include "advection_diffusion_reaction_elements.h"

namespace oomph
{
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  // TAdvectionDiffusionReactionElement
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////


  //======================================================================
  /// TAdvectionDiffusionReactionElement<NREAGENT,DIM,NNODE_1D>
  /// elements are isoparametric triangular
  /// DIM-dimensional AdvectionDiffusionReaction elements
  /// with  NNODE_1D nodal points along each
  /// element edge. Inherits from TElement
  /// and AdvectionDiffusionReactionEquations
  //======================================================================
  template<unsigned NREAGENT, unsigned DIM, unsigned NNODE_1D>
  class TAdvectionDiffusionReactionElement
    : public virtual TElement<DIM, NNODE_1D>,
      public virtual AdvectionDiffusionReactionEquations<NREAGENT, DIM>,
      public virtual ElementWithZ2ErrorEstimator
  {
  public:
    ///\short  Constructor: Call constructors for TElement and
    /// AdvectionDiffusionReaction equations
    TAdvectionDiffusionReactionElement()
      : TElement<DIM, NNODE_1D>(),
        AdvectionDiffusionReactionEquations<NREAGENT, DIM>()
    {
    }


    /// Broken copy constructor
    TAdvectionDiffusionReactionElement(
      const TAdvectionDiffusionReactionElement<NREAGENT, DIM, NNODE_1D>&
        dummy) = delete;

    /// Broken assignment operator
    void operator=(
      const TAdvectionDiffusionReactionElement<NREAGENT, DIM, NNODE_1D>&) =
      delete;

    /// \short  Access function for Nvalue: # of `values' (pinned or dofs)
    /// at node n (always returns the same value at every node, 1)
    inline unsigned required_nvalue(const unsigned& n) const
    {
      return NREAGENT;
    }

    /// \short Output function:
    ///  x,y,u   or    x,y,z,u
    void output(std::ostream& outfile)
    {
      AdvectionDiffusionReactionEquations<NREAGENT, DIM>::output(outfile);
    }

    ///  \short Output function:
    ///   x,y,u   or    x,y,z,u at n_plot^DIM plot points
    void output(std::ostream& outfile, const unsigned& n_plot)
    {
      AdvectionDiffusionReactionEquations<NREAGENT, DIM>::output(outfile,
                                                                 n_plot);
    }


    /// \short C-style output function:
    ///  x,y,u   or    x,y,z,u
    void output(FILE* file_pt)
    {
      AdvectionDiffusionReactionEquations<NREAGENT, DIM>::output(file_pt);
    }


    ///  \short C-style output function:
    ///   x,y,u   or    x,y,z,u at n_plot^DIM plot points
    void output(FILE* file_pt, const unsigned& n_plot)
    {
      AdvectionDiffusionReactionEquations<NREAGENT, DIM>::output(file_pt,
                                                                 n_plot);
    }


    /// \short Output function for an exact solution:
    ///  x,y,u_exact
    void output_fct(std::ostream& outfile,
                    const unsigned& n_plot,
                    FiniteElement::SteadyExactSolutionFctPt exact_soln_pt)
    {
      AdvectionDiffusionReactionEquations<NREAGENT, DIM>::output_fct(
        outfile, n_plot, exact_soln_pt);
    }


    /// \short Output function for a time-dependent exact solution.
    ///  x,y,u_exact (calls the steady version)
    void output_fct(std::ostream& outfile,
                    const unsigned& n_plot,
                    const double& time,
                    FiniteElement::UnsteadyExactSolutionFctPt exact_soln_pt)
    {
      AdvectionDiffusionReactionEquations<NREAGENT, DIM>::output_fct(
        outfile, n_plot, time, exact_soln_pt);
    }

  protected:
    /// Shape, test functions & derivs. w.r.t. to global coords. Return
    /// Jacobian.
    inline double dshape_and_dtest_eulerian_adv_diff_react(
      const Vector<double>& s,
      Shape& psi,
      DShape& dpsidx,
      Shape& test,
      DShape& dtestdx) const;


    /// Shape, test functions & derivs. w.r.t. to global coords. Return
    /// Jacobian.
    inline double dshape_and_dtest_eulerian_at_knot_adv_diff_react(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      Shape& test,
      DShape& dtestdx) const;

    /// \short Shape/test functions and derivs w.r.t. to global coords at
    /// integration point ipt; return Jacobian of mapping (J). Also compute
    /// derivatives of dpsidx, dtestdx and J w.r.t. nodal coordinates.
    inline double dshape_and_dtest_eulerian_at_knot_adv_diff_react(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      RankFourTensor<double>& d_dpsidx_dX,
      Shape& test,
      DShape& dtestdx,
      RankFourTensor<double>& d_dtestdx_dX,
      DenseMatrix<double>& djacobian_dX) const;

    /// \short Order of recovery shape functions for Z2 error estimation:
    /// Same order as shape functions.
    unsigned nrecovery_order()
    {
      return (NNODE_1D - 1);
    }

    /// Number of 'flux' terms for Z2 error estimation
    unsigned num_Z2_flux_terms()
    {
      return NREAGENT * DIM;
    }

    /// Get 'flux' for Z2 error recovery:  Standard flux.from
    /// AdvectionDiffusionReaction equations
    void get_Z2_flux(const Vector<double>& s, Vector<double>& flux)
    {
      this->get_flux(s, flux);
    }

    /// \short Number of vertex nodes in the element
    unsigned nvertex_node() const
    {
      return TElement<DIM, NNODE_1D>::nvertex_node();
    }

    /// \short Pointer to the j-th vertex node in the element
    Node* vertex_node_pt(const unsigned& j) const
    {
      return TElement<DIM, NNODE_1D>::vertex_node_pt(j);
    }
  };


  // Inline functions:


  //======================================================================
  /// Define the shape functions and test functions and derivatives
  /// w.r.t. global coordinates and return Jacobian of mapping.
  ///
  /// Galerkin: Test functions = shape functions
  //======================================================================
  template<unsigned NREAGENT, unsigned DIM, unsigned NNODE_1D>
  double TAdvectionDiffusionReactionElement<NREAGENT, DIM, NNODE_1D>::
    dshape_and_dtest_eulerian_adv_diff_react(const Vector<double>& s,
                                             Shape& psi,
                                             DShape& dpsidx,
                                             Shape& test,
                                             DShape& dtestdx) const
  {
    unsigned n_node = this->nnode();

    // Call the geometrical shape functions and derivatives
    double J = this->dshape_eulerian(s, psi, dpsidx);

    // Loop over the test functions and derivatives and set them equal to the
    // shape functions
    for (unsigned i = 0; i < n_node; i++)
    {
      test[i] = psi[i];
      dtestdx(i, 0) = dpsidx(i, 0);
      dtestdx(i, 1) = dpsidx(i, 1);
    }

    // Return the jacobian
    return J;
  }


  //======================================================================
  /// Define the shape functions and test functions and derivatives
  /// w.r.t. global coordinates and return Jacobian of mapping.
  ///
  /// Galerkin: Test functions = shape functions
  //======================================================================
  template<unsigned NREAGENT, unsigned DIM, unsigned NNODE_1D>
  double TAdvectionDiffusionReactionElement<NREAGENT, DIM, NNODE_1D>::
    dshape_and_dtest_eulerian_at_knot_adv_diff_react(const unsigned& ipt,
                                                     Shape& psi,
                                                     DShape& dpsidx,
                                                     Shape& test,
                                                     DShape& dtestdx) const
  {
    // Call the geometrical shape functions and derivatives
    double J = this->dshape_eulerian_at_knot(ipt, psi, dpsidx);

    // Set the pointers of the test functions
    test = psi;
    dtestdx = dpsidx;

    // Return the jacobian
    return J;
  }


  //======================================================================
  /// Define the shape functions (psi) and test functions (test) and
  /// their derivatives w.r.t. global coordinates (dpsidx and dtestdx)
  /// and return Jacobian of mapping (J). Additionally compute the
  /// derivatives of dpsidx, dtestdx and J w.r.t. nodal coordinates.
  ///
  /// Galerkin: Test functions = shape functions
  //======================================================================
  template<unsigned NREAGENT, unsigned DIM, unsigned NNODE_1D>
  double TAdvectionDiffusionReactionElement<NREAGENT, DIM, NNODE_1D>::
    dshape_and_dtest_eulerian_at_knot_adv_diff_react(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      RankFourTensor<double>& d_dpsidx_dX,
      Shape& test,
      DShape& dtestdx,
      RankFourTensor<double>& d_dtestdx_dX,
      DenseMatrix<double>& djacobian_dX) const
  {
    // Call the geometrical shape functions and derivatives
    const double J = this->dshape_eulerian_at_knot(
      ipt, psi, dpsidx, djacobian_dX, d_dpsidx_dX);

    // Set the pointers of the test functions
    test = psi;
    dtestdx = dpsidx;
    d_dtestdx_dX = d_dpsidx_dX;

    // Return the jacobian
    return J;
  }


  //=======================================================================
  /// Face geometry for the TAdvectionDiffusionReactionElement elements: The
  /// spatial dimension of the face elements is one lower than that of the bulk
  /// element but they have the same number of points along their 1D edges.
  //=======================================================================
  template<unsigned NREAGENT, unsigned DIM, unsigned NNODE_1D>
  class FaceGeometry<
    TAdvectionDiffusionReactionElement<NREAGENT, DIM, NNODE_1D>>
    : public virtual TElement<DIM - 1, NNODE_1D>
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional TElement
    FaceGeometry() : TElement<DIM - 1, NNODE_1D>() {}
  };

  //=======================================================================
  /// Face geometry for the 1D TAdvectionDiffusionReactionElement elements:
  /// Point elements
  //=======================================================================
  template<unsigned NREAGENT, unsigned NNODE_1D>
  class FaceGeometry<TAdvectionDiffusionReactionElement<NREAGENT, 1, NNODE_1D>>
    : public virtual PointElement
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional TElement
    FaceGeometry() : PointElement() {}
  };


} // namespace oomph

#endif
