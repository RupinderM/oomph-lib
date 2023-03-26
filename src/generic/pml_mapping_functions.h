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
#ifndef OOMPH_PML_MAPPING_FUNCTIONS_HEADER
#define OOMPH_PML_MAPPING_FUNCTIONS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#include "complex_matrices.h"
#include "oomph_utilities.h"

namespace oomph
{
  //
  //=======================================================================
  /// Class to hold the mapping function (gamma) for the Pml which defines
  /// how the coordinates are transformed in the Pml. This class holds
  /// the one dimensional or uniaxial case which is the most common
  //=======================================================================
  class PMLMapping
  {
  public:
    /// Default constructor (empty)
    PMLMapping(){};

    /// Pure virtual to return Pml mapping gamma, where gamma is the
    /// \f$d\tilde x / d x\f$ as  function of \f$\nu\f$ where \f$\nu = x - h\f$
    /// where h is the vector from the origin to the start of the Pml
    virtual std::complex<double> gamma(const double& nu_i,
                                       const double& pml_width_i,
                                       const double& wavenumber_squared,
                                       const double& alpha_shift = 0.0) = 0;
  };

  //=======================================================================
  /// A mapping function propsed by Bermudez et al, appears to be the best
  /// for the Helmholtz equations and so this will be the default mapping
  /// (see definition of PmlHelmholtzEquations)
  //=======================================================================
  class BermudezPMLMapping : public PMLMapping
  {
  public:
    /// Default constructor (empty)
    BermudezPMLMapping(){};

    /// Overwrite the pure Pml mapping coefficient function to return the
    /// coeffcients proposed by Bermudez et al
    std::complex<double> gamma(const double& nu_i,
                               const double& pml_width_i,
                               const double& wavenumber_squared,
                               const double& alpha_shift = 0.0) override
    {
      /// return \f$\gamma=1 + (1/k)(i/|outer_boundary - x|)\f$
      return 1.0 + MathematicalConstants::I / sqrt(wavenumber_squared) *
                     (1.0 / std::fabs(pml_width_i - nu_i));
    }
  };

  //=======================================================================
  /// A mapping function proposed by Bermudez et al, similar to the one above
  /// but is continuous across the inner Pml boundary
  /// appears to be the best for TimeHarmonicLinearElasticity
  /// and so this will be the default mapping
  //=======================================================================
  class ContinuousBermudezPMLMapping : public PMLMapping
  {
  public:
    /// Default constructor (empty)
    ContinuousBermudezPMLMapping(){};

    /// Overwrite the pure Pml mapping coefficient function to return the
    /// coeffcients proposed by Bermudez et al
    std::complex<double> gamma(const double& nu_i,
                               const double& pml_width_i,
                               const double& wavenumber_squared,
                               const double& alpha_shift = 0.0) override
    {
      /// return \f$\gamma=1 + (i/k)(1/|outer_boundary - x|-1/|pml width|)\f$
      return 1.0 + MathematicalConstants::I / sqrt(wavenumber_squared) *
                     (1.0 / std::fabs(pml_width_i - nu_i) -
                      1.0 / std::fabs(pml_width_i));
    }
  };

} // namespace oomph

#endif
