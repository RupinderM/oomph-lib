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
// Header file for triangular/tetrahedaral Axisymmetric Navier Stokes elements

#ifndef OOMPH_TAXISYM_NAVIER_STOKES_ELEMENTS_HEADER
#define OOMPH_TAXISYM_NAVIER_STOKES_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif


// OOMPH-LIB headers
#include "generic/Telements.h"
#include "axisym_navier_stokes_elements.h"
#include "generic/error_estimator.h"

namespace oomph
{
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////
  // NOTE: TRI/TET CROZIER RAVIARTS REQUIRE BUBBLE FUNCTIONS! THEY'RE NOT
  // STRAIGHTFORWARD GENERALISATIONS OF THE Q-EQUIVALENTS (WHICH ARE
  // LBB UNSTABLE!)
  //////////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////////


  //==========================================================================
  /// AxisymmetricTCrouzeix_Raviart elements are Navier--Stokes elements with
  /// quadratic interpolation for velocities and positions enriched by a single
  /// cubic bubble function, but a discontinuous linear pressure interpolation
  //==========================================================================
  class AxisymmetricTCrouzeixRaviartElement
    : public virtual TBubbleEnrichedElement<2, 3>,
      public virtual AxisymmetricNavierStokesEquations,
      public virtual ElementWithZ2ErrorEstimator
  {
  protected:
    /// Internal index that indicates at which internal datum the pressure is
    /// stored
    unsigned P_axi_nst_internal_index;


    /// \short Velocity shape and test functions and their derivs
    /// w.r.t. to global coords  at local coordinate s (taken from geometry)
    /// Return Jacobian of mapping between local and global coordinates.
    inline double dshape_and_dtest_eulerian_axi_nst(const Vector<double>& s,
                                                    Shape& psi,
                                                    DShape& dpsidx,
                                                    Shape& test,
                                                    DShape& dtestdx) const;

    /// \short Velocity shape and test functions and their derivs
    /// w.r.t. to global coords at ipt-th integation point (taken from geometry)
    /// Return Jacobian of mapping between local and global coordinates.
    inline double dshape_and_dtest_eulerian_at_knot_axi_nst(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      Shape& test,
      DShape& dtestdx) const;

    /// \short Shape/test functions and derivs w.r.t. to global coords at
    /// integration point ipt; return Jacobian of mapping (J). Also compute
    /// derivatives of dpsidx, dtestdx and J w.r.t. nodal coordinates.
    inline double dshape_and_dtest_eulerian_at_knot_axi_nst(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      RankFourTensor<double>& d_dpsidx_dX,
      Shape& test,
      DShape& dtestdx,
      RankFourTensor<double>& d_dtestdx_dX,
      DenseMatrix<double>& djacobian_dX) const;

    /// \short Pressure shape and test functions and their derivs
    /// w.r.t. to global coords  at local coordinate s (taken from geometry)
    /// Return Jacobian of mapping between local and global coordinates.
    inline double dpshape_and_dptest_eulerian_axi_nst(const Vector<double>& s,
                                                      Shape& ppsi,
                                                      DShape& dppsidx,
                                                      Shape& ptest,
                                                      DShape& dptestdx) const;

  public:
    /// Pressure shape functions at local coordinate s
    inline void pshape_axi_nst(const Vector<double>& s, Shape& psi) const;

    /// Pressure shape and test functions at local coordinte s
    inline void pshape_axi_nst(const Vector<double>& s,
                               Shape& psi,
                               Shape& test) const;

    /// Unpin all internal pressure dofs
    void unpin_all_internal_pressure_dofs();

    /// Return the local equation numbers for the pressure values.
    inline int p_local_eqn(const unsigned& n) const
    {
      return this->internal_local_eqn(P_axi_nst_internal_index, n);
    }

  public:
    /// Constructor, there are 3 internal values (for the pressure)
    AxisymmetricTCrouzeixRaviartElement()
      : TBubbleEnrichedElement<2, 3>(), AxisymmetricNavierStokesEquations()
    {
      // Allocate and a single internal datum with 3 entries for the
      // pressure
      P_axi_nst_internal_index = this->add_internal_data(new Data(3));
    }

    /// Broken copy constructor
    AxisymmetricTCrouzeixRaviartElement(
      const AxisymmetricTCrouzeixRaviartElement& dummy) = delete;

    /// Broken assignment operator
    // Commented out broken assignment operator because this can lead to a
    // conflict warning when used in the virtual inheritence hierarchy.
    // Essentially the compiler doesn't realise that two separate
    // implementations of the broken function are the same and so, quite
    // rightly, it shouts.
    /*void operator=(const AxisymmetricTCrouzeixRaviartElement&) =
      delete;*/


    /// \short Number of values (pinned or dofs) required at local node n.
    inline virtual unsigned required_nvalue(const unsigned& n) const
    {
      return 3;
    }


    /// \short Return the pressure values at internal dof i_internal
    /// (Discontinous pressure interpolation -- no need to cater for hanging
    /// nodes).
    double p_axi_nst(const unsigned& i) const
    {
      return this->internal_data_pt(P_axi_nst_internal_index)->value(i);
    }

    /// Return number of pressure values
    unsigned npres_axi_nst() const
    {
      return 3;
    }

    /// Pin p_dof-th pressure dof and set it to value specified by p_value.
    void fix_pressure(const unsigned& p_dof, const double& p_value)
    {
      this->internal_data_pt(P_axi_nst_internal_index)->pin(p_dof);
      this->internal_data_pt(P_axi_nst_internal_index)
        ->set_value(p_dof, p_value);
    }

    /// \short Build FaceElements that apply the Robin boundary condition
    /// to the pressure advection diffusion problem required by
    /// Fp preconditioner
    // void build_fp_press_adv_diff_robin_bc_element(const unsigned&
    //                                              face_index)
    // {
    // this->Pressure_advection_diffusion_robin_element_pt.push_back(
    //  new
    //  FpPressureAdvDiffRobinBCElement<AxisymmetricTCrouzeixRaviartElement<DIM>
    //  >(
    //   this, face_index));
    // }

    /// \short Add to the set paired_load_data
    /// pairs of pointers to data objects and unsignedegers that
    /// index the values in the data object that affect the load (traction),
    /// as specified in the get_load() function.
    void identify_load_data(
      std::set<std::pair<Data*, unsigned>>& paired_load_data);

    /// \short  Add to the set \c paired_pressure_data pairs
    /// containing
    /// - the pointer to a Data object
    /// and
    /// - the index of the value in that Data object
    /// .
    /// for all pressure values that affect the
    /// load computed in the \c get_load(...) function.
    void identify_pressure_data(
      std::set<std::pair<Data*, unsigned>>& paired_pressure_data);

    /// Redirect output to NavierStokesEquations output
    void output(std::ostream& outfile)
    {
      AxisymmetricNavierStokesEquations::output(outfile);
    }

    /// Redirect output to NavierStokesEquations output
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      AxisymmetricNavierStokesEquations::output(outfile, nplot);
    }

    /// Redirect output to NavierStokesEquations output
    void output(FILE* file_pt)
    {
      AxisymmetricNavierStokesEquations::output(file_pt);
    }

    /// Redirect output to NavierStokesEquations output
    void output(FILE* file_pt, const unsigned& n_plot)
    {
      AxisymmetricNavierStokesEquations::output(file_pt, n_plot);
    }


    /// \short Order of recovery shape functions for Z2 error estimation:
    /// Same order as unenriched shape functions.
    unsigned nrecovery_order()
    {
      return 2;
    }

    /// \short Number of vertex nodes in the element
    unsigned nvertex_node() const
    {
      return 3;
    }

    /// \short Pointer to the j-th vertex node in the element
    Node* vertex_node_pt(const unsigned& j) const
    {
      return node_pt(j);
    }

    /// Number of 'flux' terms for Z2 error estimation
    unsigned num_Z2_flux_terms()
    {
      // 3 diagonal strain rates, 3 off diagonal
      return 6;
    }

    /// \short Get 'flux' for Z2 error recovery:   Upper triangular entries
    /// in strain rate tensor.
    void get_Z2_flux(const Vector<double>& s, Vector<double>& flux)
    {
#ifdef PARANOID
      unsigned num_entries = 6;
      if (flux.size() < num_entries)
      {
        std::ostringstream error_message;
        error_message << "The flux vector has the wrong number of entries, "
                      << flux.size() << ", whereas it should be at least "
                      << num_entries << std::endl;
        throw OomphLibError(error_message.str(),
                            OOMPH_CURRENT_FUNCTION,
                            OOMPH_EXCEPTION_LOCATION);
      }
#endif

      // Get strain rate matrix
      DenseMatrix<double> strainrate(3);
      this->strain_rate(s, strainrate);

      // Pack into flux Vector
      unsigned icount = 0;

      // Start with diagonal terms
      for (unsigned i = 0; i < 3; i++)
      {
        flux[icount] = strainrate(i, i);
        icount++;
      }

      // Off diagonals row by row
      for (unsigned i = 0; i < 3; i++)
      {
        for (unsigned j = i + 1; j < 3; j++)
        {
          flux[icount] = strainrate(i, j);
          icount++;
        }
      }
    }


    /// \short The number of "DOF types" that degrees of freedom in this element
    /// are sub-divided into: Velocity (3 components) and pressure.
    unsigned ndof_types() const
    {
      return 4;
    }

    /// \short Create a list of pairs for all unknowns in this element,
    /// so that the first entry in each pair contains the global equation
    /// number of the unknown, while the second one contains the number
    /// of the "DOF type" that this unknown is associated with.
    /// (Function can obviously only be called if the equation numbering
    /// scheme has been set up.)
    void get_dof_numbers_for_unknowns(
      std::list<std::pair<unsigned long, unsigned>>& dof_lookup_list) const
    {
      // number of nodes
      unsigned n_node = this->nnode();

      // number of pressure values
      unsigned n_press = this->npres_axi_nst();

      // temporary pair (used to store dof lookup prior to being added to list)
      std::pair<unsigned, unsigned> dof_lookup;

      // pressure dof number
      unsigned pressure_dof_number = 3;

      // loop over the pressure values
      for (unsigned n = 0; n < n_press; n++)
      {
        // determine local eqn number
        int local_eqn_number = this->p_local_eqn(n);

        // ignore pinned values - far away degrees of freedom resulting
        // from hanging nodes can be ignored since these are be dealt
        // with by the element containing their master nodes
        if (local_eqn_number >= 0)
        {
          // store dof lookup in temporary pair: First entry in pair
          // is global equation number; second entry is dof type
          dof_lookup.first = this->eqn_number(local_eqn_number);
          dof_lookup.second = pressure_dof_number;

          // add to list
          dof_lookup_list.push_front(dof_lookup);
        }
      }

      // loop over the nodes
      for (unsigned n = 0; n < n_node; n++)
      {
        // find the number of values at this node
        unsigned nv = this->node_pt(n)->nvalue();

        // loop over these values
        for (unsigned v = 0; v < nv; v++)
        {
          // determine local eqn number
          int local_eqn_number = this->nodal_local_eqn(n, v);

          // ignore pinned values
          if (local_eqn_number >= 0)
          {
            // store dof lookup in temporary pair: First entry in pair
            // is global equation number; second entry is dof type
            dof_lookup.first = this->eqn_number(local_eqn_number);
            dof_lookup.second = v;

            // add to list
            dof_lookup_list.push_front(dof_lookup);
          }
        }
      }
    }
  };

  // Inline functions

  //=======================================================================
  /// Derivatives of the shape functions and test functions w.r.t. to global
  /// (Eulerian) coordinates. Return Jacobian of mapping between
  /// local and global coordinates.
  //=======================================================================
  inline double AxisymmetricTCrouzeixRaviartElement::
    dshape_and_dtest_eulerian_axi_nst(const Vector<double>& s,
                                      Shape& psi,
                                      DShape& dpsidx,
                                      Shape& test,
                                      DShape& dtestdx) const
  {
    // Call the geometrical shape functions and derivatives
    double J = this->dshape_eulerian(s, psi, dpsidx);
    // The test functions are equal to the shape functions
    test = psi;
    dtestdx = dpsidx;
    // Return the jacobian
    return J;
  }


  //=======================================================================
  /// Derivatives of the shape functions and test functions w.r.t. to global
  /// (Eulerian) coordinates. Return Jacobian of mapping between
  /// local and global coordinates.
  //=======================================================================
  inline double AxisymmetricTCrouzeixRaviartElement::
    dshape_and_dtest_eulerian_at_knot_axi_nst(const unsigned& ipt,
                                              Shape& psi,
                                              DShape& dpsidx,
                                              Shape& test,
                                              DShape& dtestdx) const
  {
    // Call the geometrical shape functions and derivatives
    double J = this->dshape_eulerian_at_knot(ipt, psi, dpsidx);
    // The test functions are the shape functions
    test = psi;
    dtestdx = dpsidx;
    // Return the jacobian
    return J;
  }


  //=======================================================================
  /// Define the shape functions (psi) and test functions (test) and
  /// their derivatives w.r.t. global coordinates (dpsidx and dtestdx)
  /// and return Jacobian of mapping (J). Additionally compute the
  /// derivatives of dpsidx, dtestdx and J w.r.t. nodal coordinates.
  ///
  /// Galerkin: Test functions = shape functions
  //=======================================================================
  inline double AxisymmetricTCrouzeixRaviartElement::
    dshape_and_dtest_eulerian_at_knot_axi_nst(
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

    // Set the test functions equal to the shape functions
    test = psi;
    dtestdx = dpsidx;
    d_dtestdx_dX = d_dpsidx_dX;

    // Return the jacobian
    return J;
  }


  //=======================================================================
  /// Pressure shape functions
  //=======================================================================
  inline void AxisymmetricTCrouzeixRaviartElement::pshape_axi_nst(
    const Vector<double>& s, Shape& psi) const
  {
    psi[0] = 1.0;
    psi[1] = s[0];
    psi[2] = s[1];
  }

  //=======================================================================
  /// Pressure shape and test functions
  //=======================================================================
  inline void AxisymmetricTCrouzeixRaviartElement::pshape_axi_nst(
    const Vector<double>& s, Shape& psi, Shape& test) const
  {
    // Call the pressure shape functions
    this->pshape_axi_nst(s, psi);
    // The test functions are the shape functions
    test = psi;
  }

  //==========================================================================
  /// 2D :
  /// Pressure shape and test functions and derivs w.r.t. to Eulerian coords.
  /// Return Jacobian of mapping between local and global coordinates.
  //==========================================================================
  inline double AxisymmetricTCrouzeixRaviartElement::
    dpshape_and_dptest_eulerian_axi_nst(const Vector<double>& s,
                                        Shape& ppsi,
                                        DShape& dppsidx,
                                        Shape& ptest,
                                        DShape& dptestdx) const
  {
    // Initalise with shape fcts and derivs. w.r.t. to local coordinates
    ppsi[0] = 1.0;
    ppsi[1] = s[0];
    ppsi[2] = s[1];

    dppsidx(0, 0) = 0.0;
    dppsidx(1, 0) = 1.0;
    dppsidx(2, 0) = 0.0;

    dppsidx(0, 1) = 0.0;
    dppsidx(1, 1) = 0.0;
    dppsidx(2, 1) = 1.0;


    // Get the values of the shape functions and their local derivatives
    Shape psi(7);
    DShape dpsi(7, 2);
    dshape_local(s, psi, dpsi);

    // Allocate memory for the inverse 2x2 jacobian
    DenseMatrix<double> inverse_jacobian(2);

    // Now calculate the inverse jacobian
    const double det = local_to_eulerian_mapping(dpsi, inverse_jacobian);

    // Now set the values of the derivatives to be derivs w.r.t. to the
    // Eulerian coordinates
    transform_derivatives(inverse_jacobian, dppsidx);

    // The test functions are equal to the shape functions
    ptest = ppsi;
    dptestdx = dppsidx;

    // Return the determinant of the jacobian
    return det;
  }


  //=======================================================================
  /// Face geometry of the 2D Crouzeix_Raviart elements
  //=======================================================================
  template<>
  class FaceGeometry<AxisymmetricTCrouzeixRaviartElement>
    : public virtual TElement<1, 3>
  {
  public:
    FaceGeometry() : TElement<1, 3>() {}
  };


  //=======================================================================
  /// Face geometry of the FaceGeometry of the 2D CrouzeixRaviart elements
  //=======================================================================
  template<>
  class FaceGeometry<FaceGeometry<AxisymmetricTCrouzeixRaviartElement>>
    : public virtual PointElement
  {
  public:
    FaceGeometry() : PointElement() {}
  };


  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////////


  //=======================================================================
  /// Taylor--Hood elements are Navier--Stokes elements
  /// with quadratic interpolation for velocities and positions and
  /// continous linear pressure interpolation
  //=======================================================================
  class AxisymmetricTTaylorHoodElement
    : public virtual TElement<2, 3>,
      public virtual AxisymmetricNavierStokesEquations,
      public virtual ElementWithZ2ErrorEstimator

  {
  private:
    /// Static array of ints to hold number of variables at node
    static const unsigned Initial_Nvalue[];

  protected:
    /// \short Static array of ints to hold conversion from pressure
    /// node numbers to actual node numbers
    static const unsigned Pconv[];

    /// \short Velocity shape and test functions and their derivs
    /// w.r.t. to global coords  at local coordinate s (taken from geometry)
    /// Return Jacobian of mapping between local and global coordinates.
    inline double dshape_and_dtest_eulerian_axi_nst(const Vector<double>& s,
                                                    Shape& psi,
                                                    DShape& dpsidx,
                                                    Shape& test,
                                                    DShape& dtestdx) const;

    /// \short Velocity shape and test functions and their derivs
    /// w.r.t. to global coords  at local coordinate s (taken from geometry)
    /// Return Jacobian of mapping between local and global coordinates.
    inline double dshape_and_dtest_eulerian_at_knot_axi_nst(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      Shape& test,
      DShape& dtestdx) const;

    /// \short Shape/test functions and derivs w.r.t. to global coords at
    /// integration point ipt; return Jacobian of mapping (J). Also compute
    /// derivatives of dpsidx, dtestdx and J w.r.t. nodal coordinates.
    inline double dshape_and_dtest_eulerian_at_knot_axi_nst(
      const unsigned& ipt,
      Shape& psi,
      DShape& dpsidx,
      RankFourTensor<double>& d_dpsidx_dX,
      Shape& test,
      DShape& dtestdx,
      RankFourTensor<double>& d_dtestdx_dX,
      DenseMatrix<double>& djacobian_dX) const;

    /// \short Compute the pressure shape and test functions and derivatives
    /// w.r.t. global coords at local coordinate s.
    /// Return Jacobian of mapping between local and global coordinates.
    virtual double dpshape_and_dptest_eulerian_axi_nst(const Vector<double>& s,
                                                       Shape& ppsi,
                                                       DShape& dppsidx,
                                                       Shape& ptest,
                                                       DShape& dptestdx) const;

    /// Unpin all pressure dofs
    void unpin_all_nodal_pressure_dofs();

    ///  Pin all nodal pressure dofs
    void pin_all_nodal_pressure_dofs();

    ///  Unpin the proper nodal pressure dofs
    void unpin_proper_nodal_pressure_dofs();


  public:
    /// Constructor, no internal data points
    AxisymmetricTTaylorHoodElement()
      : TElement<2, 3>(), AxisymmetricNavierStokesEquations()
    {
    }


    /// Broken copy constructor
    AxisymmetricTTaylorHoodElement(
      const AxisymmetricTTaylorHoodElement& dummy) = delete;

    /// Broken assignment operator
    /*void operator=(const AxisymmetricTTaylorHoodElement&) =
      delete;*/

    /// \short Number of values (pinned or dofs) required at node n. Can
    /// be overwritten for hanging node version
    inline virtual unsigned required_nvalue(const unsigned& n) const
    {
      return Initial_Nvalue[n];
    }

    /// Test whether the pressure dof p_dof hanging or not?
    // bool pressure_dof_is_hanging(const unsigned& p_dof)
    // {return this->node_pt(Pconv[p_dof])->is_hanging(DIM);}


    /// Pressure shape functions at local coordinate s
    inline void pshape_axi_nst(const Vector<double>& s, Shape& psi) const;

    /// Pressure shape and test functions at local coordinte s
    inline void pshape_axi_nst(const Vector<double>& s,
                               Shape& psi,
                               Shape& test) const;

    /// \short Which nodal value represents the pressure?
    unsigned p_index_axi_nst()
    {
      return 3;
    }

    /// \short Pointer to n_p-th pressure node
    // Node* pressure_node_pt(const unsigned &n_p)
    //{return this->Node_pt[Pconv[n_p]];}

    /// Return the local equation numbers for the pressure values.
    inline int p_local_eqn(const unsigned& n) const
    {
      return this->nodal_local_eqn(Pconv[n], 3);
    }

    /// \short Access function for the pressure values at local pressure
    /// node n_p (const version)
    double p_axi_nst(const unsigned& n_p) const
    {
      return this->nodal_value(Pconv[n_p], 3);
    }

    /// \short Set the value at which the pressure is stored in the nodes
    int p_nodal_index_axi_nst() const
    {
      return static_cast<int>(3);
    }

    /// Return number of pressure values
    unsigned npres_axi_nst() const
    {
      return 3;
    }

    /// Pin p_dof-th pressure dof and set it to value specified by p_value.
    void fix_pressure(const unsigned& p_dof, const double& p_value)
    {
      this->node_pt(Pconv[p_dof])->pin(3);
      this->node_pt(Pconv[p_dof])->set_value(3, p_value);
    }


    /// \short Build FaceElements that apply the Robin boundary condition
    /// to the pressure advection diffusion problem required by
    /// Fp preconditioner
    // void build_fp_press_adv_diff_robin_bc_element(const unsigned&
    //                                               face_index)
    // {
    //  this->Pressure_advection_diffusion_robin_element_pt.push_back(
    //   new FpPressureAdvDiffRobinBCElement<AxisymmetricTTaylorHoodElement<DIM>
    //   >(
    //    this, face_index));
    // }

    /// \short Add to the set \c paired_load_data pairs containing
    /// - the pointer to a Data object
    /// and
    /// - the index of the value in that Data object
    /// .
    /// for all values (pressures, velocities) that affect the
    /// load computed in the \c get_load(...) function.
    void identify_load_data(
      std::set<std::pair<Data*, unsigned>>& paired_load_data);

    /// \short  Add to the set \c paired_pressure_data pairs
    /// containing
    /// - the pointer to a Data object
    /// and
    /// - the index of the value in that Data object
    /// .
    /// for all pressure values that affect the
    /// load computed in the \c get_load(...) function.
    void identify_pressure_data(
      std::set<std::pair<Data*, unsigned>>& paired_pressure_data);

    /// Redirect output to NavierStokesEquations output
    void output(std::ostream& outfile)
    {
      AxisymmetricNavierStokesEquations::output(outfile);
    }

    /// Redirect output to NavierStokesEquations output
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      AxisymmetricNavierStokesEquations::output(outfile, nplot);
    }

    /// Redirect output to NavierStokesEquations output
    void output(FILE* file_pt)
    {
      AxisymmetricNavierStokesEquations::output(file_pt);
    }

    /// Redirect output to NavierStokesEquations output
    void output(FILE* file_pt, const unsigned& n_plot)
    {
      AxisymmetricNavierStokesEquations::output(file_pt, n_plot);
    }

    /// \short Order of recovery shape functions for Z2 error estimation:
    /// Same order as shape functions.
    unsigned nrecovery_order()
    {
      return 2;
    }

    /// \short Number of vertex nodes in the element
    unsigned nvertex_node() const
    {
      return 3;
    }

    /// \short Pointer to the j-th vertex node in the element
    Node* vertex_node_pt(const unsigned& j) const
    {
      return node_pt(j);
    }


    /// Number of 'flux' terms for Z2 error estimation
    unsigned num_Z2_flux_terms()
    {
      // 3 diagonal strain rates, 3 off diagonal rates
      return 6;
    }

    /// \short Get 'flux' for Z2 error recovery:   Upper triangular entries
    /// in strain rate tensor.
    void get_Z2_flux(const Vector<double>& s, Vector<double>& flux)
    {
#ifdef PARANOID
      unsigned num_entries = 6;
      if (flux.size() < num_entries)
      {
        std::ostringstream error_message;
        error_message << "The flux vector has the wrong number of entries, "
                      << flux.size() << ", whereas it should be at least "
                      << num_entries << std::endl;
        throw OomphLibError(error_message.str(),
                            OOMPH_CURRENT_FUNCTION,
                            OOMPH_EXCEPTION_LOCATION);
      }
#endif

      // Get strain rate matrix
      DenseMatrix<double> strainrate(3);
      this->strain_rate(s, strainrate);

      // Pack into flux Vector
      unsigned icount = 0;

      // Start with diagonal terms
      for (unsigned i = 0; i < 3; i++)
      {
        flux[icount] = strainrate(i, i);
        icount++;
      }

      // Off diagonals row by row
      for (unsigned i = 0; i < 3; i++)
      {
        for (unsigned j = i + 1; j < 3; j++)
        {
          flux[icount] = strainrate(i, j);
          icount++;
        }
      }
    }

    /// \short The number of "DOF types" that degrees of freedom in this element
    /// are sub-divided into: Velocities (3  components) and pressure.
    unsigned ndof_types() const
    {
      return 4;
    }

    /// \short Create a list of pairs for all unknowns in this element,
    /// so that the first entry in each pair contains the global equation
    /// number of the unknown, while the second one contains the number
    /// of the "DOF type" that this unknown is associated with.
    /// (Function can obviously only be called if the equation numbering
    /// scheme has been set up.)
    void get_dof_numbers_for_unknowns(
      std::list<std::pair<unsigned long, unsigned>>& dof_lookup_list) const
    {
      // number of nodes
      unsigned n_node = this->nnode();

      // temporary pair (used to store dof lookup prior to being added to list)
      std::pair<unsigned, unsigned> dof_lookup;

      // loop over the nodes
      for (unsigned n = 0; n < n_node; n++)
      {
        // find the number of Navier Stokes values at this node
        unsigned nv = this->required_nvalue(n);

        // loop over these values
        for (unsigned v = 0; v < nv; v++)
        {
          // determine local eqn number
          int local_eqn_number = this->nodal_local_eqn(n, v);

          // ignore pinned values - far away degrees of freedom resulting
          // from hanging nodes can be ignored since these are be dealt
          // with by the element containing their master nodes
          if (local_eqn_number >= 0)
          {
            // store dof lookup in temporary pair: Global equation number
            // is the first entry in pair
            dof_lookup.first = this->eqn_number(local_eqn_number);

            // set dof numbers: Dof number is the second entry in pair
            dof_lookup.second = v;

            // add to list
            dof_lookup_list.push_front(dof_lookup);
          }
        }
      }
    }
  };


  // Inline functions

  //==========================================================================
  /// Derivatives of the shape functions and test functions w.r.t to
  /// global (Eulerian) coordinates. Return Jacobian of mapping between
  /// local and global coordinates.
  //==========================================================================
  inline double AxisymmetricTTaylorHoodElement::
    dshape_and_dtest_eulerian_axi_nst(const Vector<double>& s,
                                      Shape& psi,
                                      DShape& dpsidx,
                                      Shape& test,
                                      DShape& dtestdx) const
  {
    // Call the geometrical shape functions and derivatives
    double J = this->dshape_eulerian(s, psi, dpsidx);
    // Test functions are the shape functions
    test = psi;
    dtestdx = dpsidx;
    // Return the jacobian
    return J;
  }


  //==========================================================================
  /// Derivatives of the shape functions and test functions w.r.t to
  /// global (Eulerian) coordinates. Return Jacobian of mapping between
  /// local and global coordinates.
  //==========================================================================
  inline double AxisymmetricTTaylorHoodElement::
    dshape_and_dtest_eulerian_at_knot_axi_nst(const unsigned& ipt,
                                              Shape& psi,
                                              DShape& dpsidx,
                                              Shape& test,
                                              DShape& dtestdx) const
  {
    // Call the geometrical shape functions and derivatives
    double J = this->dshape_eulerian_at_knot(ipt, psi, dpsidx);
    // Test functions are the shape functions
    test = psi;
    dtestdx = dpsidx;
    // Return the jacobian
    return J;
  }

  //==========================================================================
  /// Define the shape functions (psi) and test functions (test) and
  /// their derivatives w.r.t. global coordinates (dpsidx and dtestdx)
  /// and return Jacobian of mapping (J). Additionally compute the
  /// derivatives of dpsidx, dtestdx and J w.r.t. nodal coordinates.
  ///
  /// Galerkin: Test functions = shape functions
  //==========================================================================
  inline double AxisymmetricTTaylorHoodElement::
    dshape_and_dtest_eulerian_at_knot_axi_nst(
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

    // Set the test functions equal to the shape functions
    test = psi;
    dtestdx = dpsidx;
    d_dtestdx_dX = d_dpsidx_dX;

    // Return the jacobian
    return J;
  }

  //==========================================================================
  /// Pressure shape and test functions and derivs w.r.t. to Eulerian coords.
  /// Return Jacobian of mapping between local and global coordinates.
  //==========================================================================
  inline double AxisymmetricTTaylorHoodElement::
    dpshape_and_dptest_eulerian_axi_nst(const Vector<double>& s,
                                        Shape& ppsi,
                                        DShape& dppsidx,
                                        Shape& ptest,
                                        DShape& dptestdx) const
  {
    ppsi[0] = s[0];
    ppsi[1] = s[1];
    ppsi[2] = 1.0 - s[0] - s[1];

    dppsidx(0, 0) = 1.0;
    dppsidx(0, 1) = 0.0;

    dppsidx(1, 0) = 0.0;
    dppsidx(1, 1) = 1.0;

    dppsidx(2, 0) = -1.0;
    dppsidx(2, 1) = -1.0;

    // Allocate memory for the inverse 2x2 jacobian
    DenseMatrix<double> inverse_jacobian(2);


    // Get the values of the shape functions and their local derivatives
    Shape psi(6);
    DShape dpsi(6, 2);
    dshape_local(s, psi, dpsi);

    // Now calculate the inverse jacobian
    const double det = local_to_eulerian_mapping(dpsi, inverse_jacobian);

    // Now set the values of the derivatives to be derivs w.r.t. to the
    // Eulerian coordinates
    transform_derivatives(inverse_jacobian, dppsidx);

    // Test functions are shape functions
    ptest = ppsi;
    dptestdx = dppsidx;

    // Return the determinant of the jacobian
    return det;
  }


  //==========================================================================
  /// Pressure shape functions
  //==========================================================================
  inline void AxisymmetricTTaylorHoodElement::pshape_axi_nst(
    const Vector<double>& s, Shape& psi) const
  {
    psi[0] = s[0];
    psi[1] = s[1];
    psi[2] = 1.0 - s[0] - s[1];
  }

  //==========================================================================
  /// Pressure shape and test functions
  //==========================================================================
  inline void AxisymmetricTTaylorHoodElement::pshape_axi_nst(
    const Vector<double>& s, Shape& psi, Shape& test) const
  {
    // Call the pressure shape functions
    this->pshape_axi_nst(s, psi);
    // Test functions are shape functions
    test = psi;
  }


  //=======================================================================
  /// Face geometry of the Axisymmetric Taylor_Hood elements
  //=======================================================================
  template<>
  class FaceGeometry<AxisymmetricTTaylorHoodElement>
    : public virtual TElement<1, 3>
  {
  public:
    /// Constructor: Call constructor of base
    FaceGeometry() : TElement<1, 3>() {}
  };


  //=======================================================================
  /// Face geometry of the FaceGeometry of the Axisymmetric TaylorHood elements
  //=======================================================================
  template<>
  class FaceGeometry<FaceGeometry<AxisymmetricTTaylorHoodElement>>
    : public virtual PointElement
  {
  public:
    FaceGeometry() : PointElement() {}
  };


} // namespace oomph

#endif
