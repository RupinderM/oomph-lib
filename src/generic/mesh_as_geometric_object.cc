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
// Templated functions for MeshAsGeomObject
// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// oomph-lib headers
#include "mesh.h"
#include "mesh_as_geometric_object.h"
#include "multi_domain.h"

#include <cstdio>
namespace oomph
{
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  //========================================================================
  /// Helper namespace for MeshAsGeomObject -- its only function creates
  /// SamplePointContainerParameters of the right type for the default
  /// sample point container
  //========================================================================
  namespace MeshAsGeomObject_Helper
  {
    /// \short Default sample point container type. Must currently be one of
    /// UseCGALSamplePointContainer, UseRefineableBinArray or
    /// UseNonRefineableBinArray
#ifdef OOMPH_HAS_CGAL
    unsigned Default_sample_point_container_version =
      UseCGALSamplePointContainer;
#else
    unsigned Default_sample_point_container_version = UseRefineableBinArray;
#endif

    /// \short "Factory" for SamplePointContainerParameters of the
    /// right type as selected
    /// by Default_sample_point_container_version
    void create_sample_point_container_parameters(
      Mesh* mesh_pt,
      SamplePointContainerParameters*& sample_point_container_parameters_pt)
    {
      switch (Default_sample_point_container_version)
      {
        case UseRefineableBinArray:
          sample_point_container_parameters_pt =
            new RefineableBinArrayParameters(mesh_pt);

          break;

        case UseNonRefineableBinArray:
          sample_point_container_parameters_pt =
            new NonRefineableBinArrayParameters(mesh_pt);

          break;

#ifdef OOMPH_HAS_CGAL

        case UseCGALSamplePointContainer:
          sample_point_container_parameters_pt =
            new CGALSamplePointContainerParameters(mesh_pt);

          break;

#endif

        default:

          throw OomphLibError("Wrong sample_point_container_parameters_pt",
                              OOMPH_CURRENT_FUNCTION,
                              OOMPH_EXCEPTION_LOCATION);
      }
    }

  } // namespace MeshAsGeomObject_Helper

  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

} // namespace oomph
