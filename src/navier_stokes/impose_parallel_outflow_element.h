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
#ifndef OOMPH_IMPOSE_PARALL_ELEMENTS_HEADER
#define OOMPH_IMPOSE_PARALL_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

namespace oomph
{
  //========================================================================
  /// \short  ImposeParallelOutflowElement
  /// are elements that coincide with the faces of
  /// higher-dimensional "bulk" elements. They are used on
  /// boundaries where we would like to impose parallel outflow and
  /// impose the pressure.
  //========================================================================
  template<class ELEMENT>
  class ImposeParallelOutflowElement :
    public virtual FaceGeometry<ELEMENT>,
    public virtual FaceElement
  {
  private:
    /// pointer to imposed pressure -- if null then no pressure imposed.
    double* Pressure_pt;

    /// Lagrange Id
    unsigned Id;

  public:
    /// \short Constructor takes a "bulk" element, the
    /// index that identifies which face the
    /// ImposeParallelOutflowElement is supposed
    /// to be attached to, and the face element ID
    ImposeParallelOutflowElement(FiniteElement* const& element_pt,
                                 const int& face_index,
                                 const unsigned& id = 0) :
      FaceGeometry<ELEMENT>(), FaceElement()
    {
      //  set the Id
      Id = id;

      // Build the face element
      element_pt->build_face_element(face_index, this);

      // dimension of the bulk element
      unsigned dim = element_pt->dim();

      // we need dim-1 additional values for each FaceElement node
      Vector<unsigned> n_additional_values(nnode(), dim - 1);

      // add storage for lagrange multipliers and set the map containing
      // the position of the first entry of this face element's
      // additional values.
      add_additional_values(n_additional_values, id);

      // set the pressure pointer to zero
      Pressure_pt = 0;
    }

    /// Fill in the residuals
    void fill_in_contribution_to_residuals(Vector<double>& residuals)
    {
      // Call the generic routine with the flag set to 0
      fill_in_generic_contribution_to_residuals_parall_lagr_multiplier(
        residuals, GeneralisedElement::Dummy_matrix, 0);
    }

    /// Fill in contribution from Jacobian
    void fill_in_contribution_to_jacobian(Vector<double>& residuals,
                                          DenseMatrix<double>& jacobian)
    {
      // Call the generic routine with the flag set to 1
      fill_in_generic_contribution_to_residuals_parall_lagr_multiplier(
        residuals, jacobian, 1);
    }

    /// Overload the output function
    void output(std::ostream& outfile)
    {
      FiniteElement::output(outfile);
    }

    /// Output function: x,y,[z],u,v,[w],p in tecplot format
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      FiniteElement::output(outfile, nplot);
    }

    /// \short The "global" intrinsic coordinate of the element when
    /// viewed as part of a geometric object should be given by
    /// the FaceElement representation, by default
    /// This final over-ride is required because both SolidFiniteElements
    /// and FaceElements overload zeta_nodal
    double zeta_nodal(const unsigned& n,
                      const unsigned& k,
                      const unsigned& i) const
    {
      return FaceElement::zeta_nodal(n, k, i);
    }

    ///  Access function for the pressure
    double*& pressure_pt()
    {
      return Pressure_pt;
    }

  protected:
    /// \short Helper function to compute the residuals and, if flag==1, the
    /// Jacobian
    void fill_in_generic_contribution_to_residuals_parall_lagr_multiplier(
      Vector<double>& residuals,
      DenseMatrix<double>& jacobian,
      const unsigned& flag)
    {
      // Find out how many nodes there are
      unsigned n_node = nnode();

      // Dimension of element
      unsigned dim_el = dim();

      // Set up memory for the shape functions
      Shape psi(n_node);

      // Set the value of n_intpt
      unsigned n_intpt = integral_pt()->nweight();

      // to store local equation number
      int local_eqn = 0;
      int local_unknown = 0;

      // to store normal vector
      Vector<double> norm_vec(dim_el + 1);

      // to store tangantial vectors
      Vector<Vector<double>> tang_vec(dim_el, Vector<double>(dim_el + 1));

      // get the value at which the velocities are stored
      Vector<unsigned> u_index(dim_el + 1);
      ELEMENT* el_pt = dynamic_cast<ELEMENT*>(this->bulk_element_pt());
      for (unsigned i = 0; i < dim_el + 1; i++)
      {
        u_index[i] = el_pt->u_index_nst(i);
      }

      // Loop over the integration points
      for (unsigned ipt = 0; ipt < n_intpt; ipt++)
      {
        // Get the integral weight
        double w = integral_pt()->weight(ipt);

        // Jacobian of mapping
        double J = J_eulerian_at_knot(ipt);

        // Premultiply the weights and the Jacobian
        double W = w * J;

        // Calculate the shape functions
        shape_at_knot(ipt, psi);

        // compute  the velocity and the Lagrange multiplier
        Vector<double> interpolated_u(dim_el + 1, 0.0);
        Vector<double> lambda(dim_el, 0.0);
        // Loop over nodes
        for (unsigned j = 0; j < n_node; j++)
        {
          // Assemble the velocity component
          for (unsigned i = 0; i < dim_el + 1; i++)
          {
            interpolated_u[i] += nodal_value(j, u_index[i]) * psi(j);
          }

          // Cast to a boundary node
          BoundaryNodeBase* bnod_pt =
            dynamic_cast<BoundaryNodeBase*>(node_pt(j));

          // get the node
          Node* nod_pt = node_pt(j);

          // Get the index of the first nodal value associated with
          // this FaceElement
          unsigned first_index =
            bnod_pt->index_of_first_value_assigned_by_face_element(Id);

          // Assemble the Lagrange multiplier
          for (unsigned l = 0; l < dim_el; l++)
          {
            lambda[l] += nod_pt->value(first_index + l) * psi(j);
          }
        }

        this->continuous_tangent_and_outer_unit_normal(ipt, tang_vec, norm_vec);

        // Assemble residuals and jacobian

        // Loop over the nodes
        for (unsigned j = 0; j < n_node; j++)
        {
          // Cast to a boundary node
          BoundaryNodeBase* bnod_pt =
            dynamic_cast<BoundaryNodeBase*>(node_pt(j));

          // Get the index of the first nodal value associated with
          // this FaceElement
          unsigned first_index =
            bnod_pt->index_of_first_value_assigned_by_face_element(Id);

          // loop over the lagrange multiplier components
          for (unsigned l = 0; l < dim_el; l++)
          {
            // Local eqn number for the l-th component of lamdba
            // in the j-th element
            local_eqn = nodal_local_eqn(j, first_index + l);

            if (local_eqn >= 0)
            {
              for (unsigned i = 0; i < dim_el + 1; i++)
              {
                // Assemble residual for lagrange multiplier
                residuals[local_eqn] +=
                  interpolated_u[i] * tang_vec[l][i] * psi(j) * W;

                // Assemble Jacobian for Lagrange multiplier:
                if (flag == 1)
                {
                  // Loop over the nodes again for unknowns
                  for (unsigned jj = 0; jj < n_node; jj++)
                  {
                    // Local eqn number for the i-th component
                    // of the velocity in the jj-th element
                    local_unknown = nodal_local_eqn(jj, u_index[i]);
                    if (local_unknown >= 0)
                    {
                      jacobian(local_eqn, local_unknown) +=
                        tang_vec[l][i] * psi(jj) * psi(j) * W;
                    }
                  }
                }
              }
            }
          }

          // Loop over the directions
          for (unsigned i = 0; i < dim_el + 1; i++)
          {
            // Local eqn number for the i-th component of the
            // velocity in the j-th element
            local_eqn = nodal_local_eqn(j, u_index[i]);

            if (local_eqn >= 0)
            {
              // Add the contribution of the imposed pressure
              if (Pressure_pt != 0)
              {
                residuals[local_eqn] -=
                  (*Pressure_pt) * norm_vec[i] * psi(j) * W;
              }

              // Add lagrange multiplier contribution to the bulk equation
              for (unsigned l = 0; l < dim_el; l++)
              {
                // Add to residual
                residuals[local_eqn] += tang_vec[l][i] * psi(j) * lambda[l] * W;

                // Do Jacobian too?
                if (flag == 1)
                {
                  // Loop over the nodes again for unknowns
                  for (unsigned jj = 0; jj < n_node; jj++)
                  {
                    // Cast to a boundary node
                    BoundaryNodeBase* bnode_pt =
                      dynamic_cast<BoundaryNodeBase*>(node_pt(jj));

                    // Local eqn number for the l-th component of lamdba
                    // in the jj-th element
                    local_unknown = nodal_local_eqn(
                      jj,
                      bnode_pt->index_of_first_value_assigned_by_face_element(
                        Id) +
                        l);
                    if (local_unknown >= 0)
                    {
                      jacobian(local_eqn, local_unknown) +=
                        tang_vec[l][i] * psi(jj) * psi(j) * W;
                    }
                  }
                }
              }
            }
          }
        }
      }
    }

    /// \short The number of degrees of freedom types that this element is
    /// divided into: The ImposeParallelOutflowElement is responsible for 1(2)
    /// of it's own degrees of freedom. In addition to this, it also classifies
    /// the 2(3) constrained velocity bulk degrees of freedom which this face
    /// element is attached to.
    unsigned ndof_types() const
    {
      // The the dof types in this element is 1 in 2D, 2 in 3D, so we use the
      // elemental dimension function this->dim().
      // In addition, we have the number of constrained velocity dof types, this
      // is this->dim()+1 dof types.
      // In total we have 2*(this->dim()) + 1 dof types.
      return 2 * (this->dim()) + 1;
    }

    /// \short Create a list of pairs for all unknowns in this element,
    /// so that the first entry in each pair contains the global equation
    /// number of the unknown, while the second one contains the LOCAL number
    /// of the "DOF types" that this unknown is associated with.
    /// (Function can obviously only be called if the equation numbering
    /// scheme has been set up.)
    ///
    /// For the ImposeParallelOutflowElement, we need to classify both the
    /// constrained velocity from the bulk element and the dof types from this
    /// element. In 3D, let up, vp, wp be the constrained velocity dof types
    /// associated with the ImposeParallelOutflowElement. Let Lp1 and Lp2 be the
    /// two Lagrange multiplier dof types associated with the
    /// ImposeParallelOutflowElement. Then we have:
    ///
    /// 0  1  2  3  4
    /// up vp wp L1 L2
    void get_dof_numbers_for_unknowns(
      std::list<std::pair<unsigned long, unsigned>>& dof_lookup_list) const
    {
      // Temporary pair (used to store dof lookup prior to being added to list)
      std::pair<unsigned, unsigned> dof_lookup;

      // Number of nodes in this element.
      const unsigned n_node = this->nnode();

      // The elemental dimension.
      const unsigned dim_el = this->dim();

      // The spatial dimension, since this is a face element, as assume that the
      // spatial dimension is one higher than the elemental dimension.
      const unsigned dim_bulk = dim_el + 1;

      // The classification of the constrained bulk velocity dof types is
      // i, i = 0, ..., dim_bulk.
      //
      // The classification of the lagrange multiplier dof types is
      // i + dim_bulk, i = 0, ..., dim_el
      // as it is offset by the spatial dimension.

      // Loop over the nodes
      for (unsigned node_i = 0; node_i < n_node; node_i++)
      {
        // First we classify the constrained velocity dof types from the bulk
        // element. NOTE: This is a RE-CLASSIFICATION, since the dof types we
        // are classifying should already be classified at least once by the
        // bulk element. Furthermore, we assume that the velocity dof types
        // comes first, then the pressure dof types. This is indeed true with
        // OOMPH-LIB's implementation of Navier-Stokes elements.
        for (unsigned velocity_i = 0; velocity_i < dim_bulk; velocity_i++)
        {
          // We only concern ourselves with the nodes of the "bulk" element
          // that are associated with this "face" element. Bulk_node_number
          // gives us the mapping from the face element's node to the bulk
          // element's node. The local equation number is required to check if
          // the value is pinned, if it is not pinned, the local equation number
          // is required to get the global equation number.
          int local_eqn = Bulk_element_pt->nodal_local_eqn(
            Bulk_node_number[node_i], velocity_i);

          // Ignore pinned values.
          if (local_eqn >= 0)
          {
            // Store the dof lookup in temporary pair: First entry in pair
            // is the global equation number; second entry is the LOCAL dof
            // type. It is assumed that the velocity dof types comes first. We
            // do not re-classify the pressure dof types as they are not
            // constrained.
            dof_lookup.first = Bulk_element_pt->eqn_number(local_eqn);
            dof_lookup.second = velocity_i;
            dof_lookup_list.push_front(dof_lookup);
          } // ignore pinned nodes "if(local-eqn>=0)"
        } // for loop over the velocity components

        // Now we classify the dof types of this face element.
        // Cast to a boundary node
        BoundaryNodeBase* bnod_pt =
          dynamic_cast<BoundaryNodeBase*>(node_pt(node_i));

        // Loop over directions in this Face(!)Element
        for (unsigned dim_i = 0; dim_i < dim_el; dim_i++)
        {
          // Local eqn number:
          int local_eqn = nodal_local_eqn(
            node_i,
            bnod_pt->index_of_first_value_assigned_by_face_element(Id) + dim_i);

          if (local_eqn >= 0)
          {
            // Store dof lookup in temporary pair: First entry in pair
            // is global equation number; second entry is dof type.
            // We assume that the first 2(3) types are fluid dof types in
            // 2(3) spatial dimensions (classified above):
            //
            // 0  1  2  3  4
            // up vp wp L1 L2
            //
            // Thus the offset is the dim_bulk.
            dof_lookup.first = this->eqn_number(local_eqn);
            dof_lookup.second = dim_i + dim_bulk;

            // Add to list
            dof_lookup_list.push_front(dof_lookup);
          } // if local_eqn > 0
        } // for: loop over the directions in the face element.
      } // for: loop over the nodes in the face element.

    } // get_dof_numbers_for_unknowns

  }; // End of ImposeParallelOutflowElement class

} // namespace oomph

#endif
