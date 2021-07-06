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
#ifndef OOMPH_CIRCULAR_SHELL_MESH_HEADER
#define OOMPH_CIRCULAR_SHELL_MESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// OOMPH-LIB headers
#include "../generic/mesh.h"
#include "../generic/matrices.h"
#include "../generic/quadtree.h"
#include "../generic/quad_mesh.h"
#include "rectangular_quadmesh.template.h"

namespace oomph
{
  //========================================================================
  /// A 2D solid mesh for (topologically) circular cylindrical shells.
  /// The shell is represented by two Lagrangian coordinates that correspond
  /// to z and theta in cylindrical polars. The required mesh is therefore a
  /// 2D mesh and is therefore inherited from the generic RectangularQuadMesh
  //=======================================================================
  template<class ELEMENT>
  class CircularCylindricalShellMesh :
    public virtual RectangularQuadMesh<ELEMENT>,
    public virtual SolidMesh
  {
  public:
    /// Typedef for fct that defines the axial stretching fct
    typedef double (*AxialBLStretchingFctPt)(const double& x);

    /// Constructor for the mesh -- uniformly spaced elements
    CircularCylindricalShellMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt)
    {
      // Use default stretching fct
      Axial_bl_stretching_fct_pt = 0;

      // Assign dummy data consistent with uniform spacing
      Nx_bl = 1;
      Delta_bl = lx / double(nx);

      // Build mesh
      this->build_mesh(nx, ny, lx, ly);
    }

    /// \short Constructor for the mesh -- specify fct that maps axial
    ///  Lagr. coordinates to new positions to allow for better resolution of
    /// bending boundary layer
    CircularCylindricalShellMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      AxialBLStretchingFctPt axial_bl_stretching_fct_pt,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt)
    {
      // Apply stretching fct
      Axial_bl_stretching_fct_pt = axial_bl_stretching_fct_pt;

      // Assign dummy data consistent with uniform spacing
      Nx_bl = 1;
      Delta_bl = lx / double(nx);

      // Build mesh
      this->build_mesh(nx, ny, lx, ly);
    }

    /// \short Constructor for the mesh. nx_bl azimuthal layers of
    /// elements near the ends are squashed to that axial extent
    /// of the elements changes from lx/nx to delta_bl.
    CircularCylindricalShellMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      const unsigned& nx_bl,
      const double& delta_bl,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt)
    {
      // Use default stretching fct
      Axial_bl_stretching_fct_pt = 0;

      // Store bl data
      Nx_bl = nx_bl;
      Delta_bl = delta_bl;

      // Build mesh
      this->build_mesh(nx, ny, lx, ly);
    }

    /// \short In all elastic problems, the nodes must be assigned an
    /// undeformed, or reference, position, corresponding to the
    /// stress-free state of the elastic body. This function assigns
    /// the undeformed position for the nodes on the elastic tube
    void assign_undeformed_positions(GeomObject* const& undeformed_midplane_pt);

    /// Access to fct pointer to fct that defines the axial stretching fct
    AxialBLStretchingFctPt axial_bl_stretching_fct_pt() const
    {
      return Axial_bl_stretching_fct_pt;
    }

  private:
    /// Mesh build helper fct
    void build_mesh(const unsigned& nx,
                    const unsigned& ny,
                    const double& lx,
                    const double& ly);

    /// \short Fct that defines the axial stretching to accomodate
    /// bending boundary layers
    double scaled_x(const double& x)
    {
      if (Axial_bl_stretching_fct_pt == 0)
      {
        return piecewise_linear_axial_bl_stretching_fct(x);
      }
      else
      {
        return (*Axial_bl_stretching_fct_pt)(x);
      }
    }

    /// Default axial scaling fct
    double piecewise_linear_axial_bl_stretching_fct(const double& xi)
    {
      // Length of shell
      double lx = this->Xmax - this->Xmin;

      // Old axial extent of the elements spanning the boundary layer
      double old_delta_bl = double(Nx_bl) * lx / double(this->Nx);

      double tmp_xi = xi;
      if (xi < old_delta_bl)
      {
        tmp_xi = xi * Delta_bl / old_delta_bl;
      }
      else if (xi < (lx - old_delta_bl))
      {
        tmp_xi = Delta_bl + (xi - old_delta_bl) / (lx - 2.0 * old_delta_bl) *
                              (lx - 2.0 * Delta_bl);
      }
      else
      {
        double end_x = lx - Delta_bl;
        tmp_xi = end_x + (xi - (lx - old_delta_bl)) / old_delta_bl * Delta_bl;
      }
      return tmp_xi;
    }

    /// Fct pointer to fct that defines the axial stretching fct
    AxialBLStretchingFctPt Axial_bl_stretching_fct_pt;

    /// \short Number of azimuthal element layers that get squashed into
    /// each of the the two boundary layers at the ends of the tube
    unsigned Nx_bl;

    /// \short Axial extent of the squashed boundary layer part of the mesh
    /// occupied by Nx_bl elements (at each end of the tube)
    double Delta_bl;
  };

} // namespace oomph

#endif
