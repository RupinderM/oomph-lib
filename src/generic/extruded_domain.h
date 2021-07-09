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
#ifndef OOMPH_EXTRUDED_DOMAIN_HEADER
#define OOMPH_EXTRUDED_DOMAIN_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// oomph-lib headers
#include "generic/domain.h"
// #include "extruded_macro_element.h"

namespace oomph
{
  // Forward declaration of the extruded macro element class
  class ExtrudedMacroElement;

  //=================================================================
  /// \short Base class for ExtrudedDomains with curvilinear and/or
  /// time-dependent boundaries. ExtrudedDomain boundaries are
  /// typically represented by GeomObjects and the ExtrudedDomain
  /// itself is decomposed into a number of ExtrudedMacroElements.
  /// Any instantiation of a specific ExtrudedDomain needs to
  /// implement the pure virtual member function
  /// \code ExtrudedDomain::macro_element_boundary(...) \endcode
  /// which returns a Vector representation of each of the
  /// ExtrudedMacroElements' boundaries, parametrised by the
  /// coordinate(s) along this boundary. The ExtrudedMacroElements
  /// obtain their boundary positions via member function pointers to
  /// \c ExtrudedDomain::macro_element_boundary(...).
  //=================================================================
  class ExtrudedDomain : public virtual Domain
  {
  public:
    /// Constructor
    ExtrudedDomain(Domain* domain_pt,
                   const unsigned& n_extruded_element,
                   const double& extrusion_length);

    /// Constructor
    ExtrudedDomain(Domain* domain_pt,
                   const unsigned& n_extruded_element,
                   const double& t_min,
                   const double& t_max);

    /// Destructor: Empty; the extruded macro elements will be deleted in Domain
    ~ExtrudedDomain() {}

    /// Broken copy constructor
    ExtrudedDomain(const ExtrudedDomain&)
    {
      // Return a broken copy message
      BrokenCopy::broken_copy("ExtrudedDomain");
    } // End of ExtrudedDomain

    /// Broken assignment operator
    void operator=(const ExtrudedDomain&)
    {
      // Return a broken assign message
      BrokenCopy::broken_assign("ExtrudedDomain");
    } // End of ExtrudedDomain

    /// \short Access to i-th extruded macro element
    ExtrudedMacroElement* macro_element_pt(const unsigned& i);

    /// Number of macro elements in domain
    unsigned nmacro_element();

    /// \short Vector representation of the i_macro-th macro element
    /// boundary i_direct (e.g. N/S/W/E in 2D spatial = 3D space-time).
    /// NOTE: Some extra care has to be taken here to translate the
    /// OcTree enumeration to the QuadTree enumeration (in the
    /// appropriate manner) so that the original Domain object can be
    /// used to calculate the global coordinate associated with the
    /// provided local coordinates.
    void macro_element_boundary(const unsigned& time,
                                const unsigned& i_macro,
                                const unsigned& i_direct,
                                const Vector<double>& s,
                                Vector<double>& x);

  private:
    /// Pointer to the Domain
    Domain* Domain_pt;

    // The number of elements to create in the extrusion direction
    unsigned N_extruded_element;

    /// The minimum t-value (defaults to 0.0 if not specified)
    double T_min;

    /// The maximum t-value
    double T_max;
  };
} // End of namespace oomph

#endif
