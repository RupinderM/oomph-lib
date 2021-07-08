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
// Common base class for all BrickMeshes

#ifndef OOMPH_BRICKMESH_HEADER
#define OOMPH_BRICKMESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
// mpi headers
#include "mpi.h"
#endif

// oomphlib includes
#include "Vector.h"
#include "nodes.h"
#include "matrices.h"
#include "mesh.h"
#include "Qelements.h"

namespace oomph
{
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////

  //====================================================================
  /// Helper namespace for generation of brick from tet mesh
  //====================================================================
  namespace BrickFromTetMeshHelper
  {
    /// Tolerance for mismatch during setup of boundary coordinates
    extern double Face_position_tolerance;

  } // namespace BrickFromTetMeshHelper

  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////

  //======================================================================
  /// Dummy QElement to interpolate local coordinates -- used in
  /// construction of brickified tet mesh.
  //======================================================================
  class DummyBrickElement : public virtual QElement<3, 2>
  {
  public:
    ///\short  Constructor:
    DummyBrickElement() : QElement<3, 2>() {}

    /// Broken copy constructor
    DummyBrickElement(const DummyBrickElement& dummy)
    {
      BrokenCopy::broken_copy("DummyElement");
    }

    /// Broken assignment operator
    // Commented out broken assignment operator because this can lead to a
    // conflict warning when used in the virtual inheritence hierarchy.
    // Essentially the compiler doesn't realise that two separate
    // implementations of the broken function are the same and so, quite
    // rightly, it shouts.
    /*void operator=(const DummyBrickElement&)
     {
      BrokenCopy::broken_assign("DummyBrickElement");
      }*/

    /// \short  Required  # of `values' (pinned or dofs)
    /// at node n
    inline unsigned required_nvalue(const unsigned& n) const
    {
      return 3;
    }

    /// \short Compute vector of FE interpolated local coordinate in tet,
    /// s_tet, evaluated at local coordinate s in current element.
    void interpolated_s_tet(const Vector<double>& s,
                            Vector<double>& s_tet) const
    {
      // Find number of nodes
      unsigned n_node = nnode();

      // Local shape function
      Shape psi(n_node);

      // Find values of shape function
      shape(s, psi);

      for (unsigned i = 0; i < 3; i++)
      {
        // Initialise value of u
        s_tet[i] = 0.0;

        // Loop over the local nodes and sum
        for (unsigned l = 0; l < n_node; l++)
        {
          s_tet[i] += nodal_value(l, i) * psi[l];
        }
      }
    }

    /// Output interpolated tet local coordinates
    void output(std::ostream& outfile, const unsigned& nplot = 5)
    {
      // Vector of local coordinates
      Vector<double> s(3);
      Vector<double> s_tet(3);

      // Tecplot header info
      outfile << tecplot_zone_string(nplot);

      // Loop over plot points
      unsigned num_plot_points = nplot_points(nplot);
      for (unsigned iplot = 0; iplot < num_plot_points; iplot++)
      {
        // Get local coordinates of plot point
        get_s_plot(iplot, nplot, s);

        // Local coordinates in tet
        interpolated_s_tet(s, s_tet);

        // Coordinates
        for (unsigned i = 0; i < 3; i++)
        {
          outfile << interpolated_x(s, i) << " ";
        }

        // Local coordinates in tet
        for (unsigned i = 0; i < 3; i++)
        {
          outfile << s_tet[i] << " ";
        }

        outfile << std::endl;
      }
      outfile << std::endl;

      // Write tecplot footer (e.g. FE connectivity lists)
      write_tecplot_zone_footer(outfile, nplot);
    }
  };

  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////
  ///////////////////////////////////////////////////////////////////////////

  //================================================================
  /// Base class for brick meshes (meshes made of 3D brick elements).
  //================================================================
  class BrickMeshBase : public virtual Mesh
  {
  public:
    /// Constructor (empty)
    BrickMeshBase() {}

    /// Broken copy constructor
    BrickMeshBase(const BrickMeshBase&)
    {
      BrokenCopy::broken_copy("BrickMeshBase");
    }

    /// Broken assignment operator
    /*void operator=(const BrickMeshBase&)
     {
      BrokenCopy::broken_assign("BrickMeshBase");
      }*/

    /// Destructor (empty)
    virtual ~BrickMeshBase() {}

    /// Setup lookup schemes which establish whic elements are located
    /// next to mesh's boundaries (wrapper to suppress doc).
    void setup_boundary_element_info()
    {
      std::ofstream outfile;
      setup_boundary_element_info(outfile);
    }

    /// \short Setup lookup schemes which establish whic elements are located
    /// next to mesh's boundaries. Doc in outfile (if it's open).
    void setup_boundary_element_info(std::ostream& outfile);
  };

} // namespace oomph

#endif
