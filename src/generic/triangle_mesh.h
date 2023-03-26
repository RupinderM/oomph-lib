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
// Common base class for all Triangle Meshes
#ifndef OOMPH_GENERIC_TRIANGLE_MESH_HEADER
#define OOMPH_GENERIC_TRIANGLE_MESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// Oomph-lib includes
#include "Vector.h"
#include "nodes.h"
#include "matrices.h"
#include "mesh.h"
#include "unstructured_two_d_mesh_geometry_base.h"

namespace oomph
{
  //================================================================
  /// Base class for triangle meshes (meshes made of 2D triangle elements).
  /// Note: we choose to template TriangleMeshBase here because certain
  /// functions in UnstructuredTwoDMeshGeometryBase need template parameters
  /// and it's much cleaner simply to template the entire class
  //================================================================
  class TriangleMeshBase : public virtual UnstructuredTwoDMeshGeometryBase,
                           public virtual Mesh
  {
  public:
    /// Constructor
    TriangleMeshBase()
    {
#ifdef OOMPH_HAS_TRIANGLE_LIB
      // Initialise the TriangulateIO Data structure
      TriangleHelper::initialise_triangulateio(Triangulateio);

      // Enable triangulateio specific parts for dump/restart by default.
      Use_triangulateio_restart = true;
#endif
    }

    /// Broken copy constructor
    TriangleMeshBase(const TriangleMeshBase& node) = delete;

    /// Broken assignment operator
    // Commented out broken assignment operator because this can lead to a
    // conflict warning when used in the virtual inheritence hierarchy.
    // Essentially the compiler doesn't realise that two separate
    // implementations of the broken function are the same and so, quite
    // rightly, it shouts.
    /*void operator=(const TriangleMeshBase&) = delete;*/

    /// Destructor (empty)
    ~TriangleMeshBase() override
    {
#ifdef OOMPH_HAS_TRIANGLE_LIB
      // Clear the triangulate data structure
      TriangleHelper::clear_triangulateio(Triangulateio);
#endif
    }

    /// Setup lookup schemes which establish whic elements are located
    /// next to mesh's boundaries (wrapper to suppress doc).
    void setup_boundary_element_info() override
    {
      std::ofstream outfile;
      setup_boundary_element_info(outfile);
    }

    /// Setup lookup schemes which establish which elements are located
    /// next to mesh's boundaries. Doc in outfile (if it's open).
    void setup_boundary_element_info(std::ostream& outfile) override;

#ifdef OOMPH_HAS_TRIANGLE_LIB
    /// const access for Use_triangulateio_restart.
    bool use_triangulateio_restart() const
    {
      return Use_triangulateio_restart;
    }

    /// write access for Use_triangulateio_restart.
    void enable_triangulateio_restart()
    {
      Use_triangulateio_restart = true;
    }

    /// write access for Use_triangulateio_restart.
    void disable_triangulateio_restart()
    {
      Use_triangulateio_restart = false;
    }

    /// Access to the triangulateio representation of the mesh
    TriangulateIO& triangulateio_representation()
    {
      return Triangulateio;
    }

    /// Helper function. Write a TriangulateIO object file with all the
    /// triangulateio fields. String s is add to assign a different value for
    /// the input and/or output structure
    void write_triangulateio(TriangulateIO& triangulate_io, std::string& s);

    /// Helper function. Clean up the memory associated with the
    /// TriangulateIO object. This should really only be used to save
    /// memory in extremely tight situations.
    void clear_triangulateio()
    {
      TriangleHelper::clear_triangulateio(Triangulateio);
    }

    /// Dump the triangulateio structure to a dump file and
    /// record boundary coordinates of boundary nodes
    void dump_triangulateio(std::ostream& dump_file);

#ifdef OOMPH_HAS_MPI
    /// Virtual function that is used to dump info. related with
    /// distributed triangle meshes
    virtual void dump_distributed_info_for_restart(std::ostream& dump_file)
    {
      std::ostringstream error_stream;
      error_stream << "Empty default dump disributed info. method called.\n";
      error_stream << "This should be overloaded in a specific TriangleMesh\n";
      throw OomphLibError(
        error_stream.str(),
        "TriangleMeshBase::dump_distributed_info_for_restart()",
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Virtual function that is used to dump info. related with
    /// distributed triangle meshes
    virtual void dump_info_to_reset_halo_haloed_scheme(std::ostream& dump_file)
    {
      std::ostringstream error_stream;
      error_stream << "Empty default dump info. to reset halo haloed scheme.\n";
      error_stream << "This should be overloaded in a specific TriangleMesh\n";
      throw OomphLibError(
        error_stream.str(),
        "TriangleMeshBase::dump_info_to_reset_halo_haloed_scheme()",
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Virtual function that is used to read info. related with
    /// distributed triangle meshes
    virtual void read_distributed_info_for_restart(std::istream& restart_file)
    {
      std::ostringstream error_stream;
      error_stream << "Empty default read disributed info. method called.\n";
      error_stream << "This should be overloaded in a specific TriangleMesh\n";
      throw OomphLibError(
        error_stream.str(),
        "TriangleMeshBase::read_distributed_info_for_restart()",
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Virtual function used to re-establish any additional info. related with
    /// the distribution after a re-starting for triangle meshes
    virtual void reestablish_distribution_info_for_restart(
      OomphCommunicator* comm_pt, std::istream& restart_file)
    {
      std::ostringstream error_stream;
      error_stream << "Empty default reestablish disributed info method "
                   << "called.\n";
      error_stream << "This should be overloaded in a specific "
                   << "RefineableTriangleMesh\n";
      throw OomphLibError(
        error_stream.str(),
        "TriangleMeshBase::reestablish_distribution_info_for_restart()",
        OOMPH_EXCEPTION_LOCATION);
    }
#endif

    /// Virtual function used to update the polylines representation after
    /// restart
    virtual void update_polyline_representation_from_restart()
    {
      std::ostringstream error_stream;
      error_stream << "Empty default update polylines representation from "
                   << "restart method called.\n";
      error_stream << "This should be overloaded in a specific "
                   << "RefineableTriangleMesh\n";
      throw OomphLibError(
        error_stream.str(),
        "TriangleMeshBase::update_polyline_representation_from_restart()",
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Regenerate the mesh from a dumped triangulateio file
    /// and dumped boundary coordinates of boundary nodes
    void remesh_from_triangulateio(std::istream& restart_file);

    /// Virtual function that is used for specific remeshing from the
    /// triangulateio
    virtual void remesh_from_internal_triangulateio()
    {
      std::ostringstream error_stream;
      error_stream << "Empty default remesh function called.\n";
      error_stream << "This should be overloaded in a specific TriangleMesh\n";
      throw OomphLibError(
        error_stream.str(), OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
    }

#endif // #ifdef OOMPH_HAS_TRIANGLE_LIB

    /// Virtual function to perform the load balance rutines
    virtual void load_balance(
      const Vector<unsigned>& target_domain_for_local_non_halo_element)
    {
      std::ostringstream error_stream;
      error_stream << "Empty default load balancing function called.\n";
      error_stream << "This should be overloaded in a specific TriangleMesh\n";
      throw OomphLibError(error_stream.str(),
                          "TriangleMeshBase::load_balance()",
                          OOMPH_EXCEPTION_LOCATION);
    }

    /// Virtual function to perform the reset boundary elements info rutines
    void reset_boundary_element_info(
      Vector<unsigned>& ntmp_boundary_elements,
      Vector<Vector<unsigned>>& ntmp_boundary_elements_in_region,
      Vector<FiniteElement*>& deleted_elements) override
    {
      std::ostringstream error_stream;
      error_stream << "Empty default reset boundary element info function"
                   << "called.\n";
      error_stream << "This should be overloaded in a specific "
                   << "TriangleMesh\n";
      throw OomphLibError(error_stream.str(),
                          "TriangleMeshBase::reset_boundary_element_info()",
                          OOMPH_EXCEPTION_LOCATION);
    }

  protected:
#ifdef OOMPH_HAS_TRIANGLE_LIB

    /// TriangulateIO representation of the mesh
    TriangulateIO Triangulateio;

    /// Should we use triangulateio specific parts for dump/restart? (Doesn't
    /// work with some elements and isn't needed if not using adaptivity).
    bool Use_triangulateio_restart;

#endif // OOMPH_HAS_TRIANGLE
  };

} // namespace oomph

#endif
