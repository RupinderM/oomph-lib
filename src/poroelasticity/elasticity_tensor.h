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
// Header file for objects representing the fourth-rank elasticity tensor
// for linear elasticity problems

// Include guards to prevent multiple inclusion of the header
#ifndef OOMPH_POROELASTICITY_TENSOR_HEADER
#define OOMPH_POROELASTICITY_TENSOR_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#include "../generic/oomph_utilities.h"

namespace oomph
{
  //=====================================================================
  /// A base class that represents the fourth-rank elasticity tensor
  /// \f$E_{ijkl}\f$ defined such that
  /// \f[\tau_{ij} = E_{ijkl} e_{kl},\f]
  /// where \f$e_{ij}\f$ is the infinitessimal (Cauchy) strain tensor
  /// and \f$\tau_{ij}\f$ is the stress tensor. The symmetries of the
  /// tensor are such that
  /// \f[E_{ijkl} = E_{jikl} = E_{ijlk} = E_{klij}\f]
  /// and thus there are relatively few independent components. These
  /// symmetries are included in the definition of the object so that
  /// non-physical symmetries cannot be accidentally imposed.
  //=====================================================================
  class ElasticityTensor
  {
    ///\short Translation table from the four indices to the corresponding
    /// independent component
    static const unsigned Index[3][3][3][3];

  protected:
    /// Member function that returns the i-th independent component of the
    /// elasticity tensor
    virtual inline double independent_component(const unsigned& i) const
    {
      return 0.0;
    }

    ///\short Helper range checking function
    /// (Note that this only captures over-runs in 3D but
    /// errors are likely to be caught in evaluation of the
    /// stress and strain tensors anyway...)
    void range_check(const unsigned& i,
                     const unsigned& j,
                     const unsigned& k,
                     const unsigned& l) const
    {
      if ((i > 2) || (j > 2) || (k > 2) || (l > 2))
      {
        std::ostringstream error_message;
        if (i > 2)
        {
          error_message << "Range Error : Index 1 " << i
                        << " is not in the range (0,2)";
        }
        if (j > 2)
        {
          error_message << "Range Error : Index 2 " << j
                        << " is not in the range (0,2)";
        }

        if (k > 2)
        {
          error_message << "Range Error : Index 2 " << k
                        << " is not in the range (0,2)";
        }

        if (l > 2)
        {
          error_message << "Range Error : Index 4 " << l
                        << " is not in the range (0,2)";
        }

        // Throw the error
        throw OomphLibError(error_message.str(),
                            OOMPH_CURRENT_FUNCTION,
                            OOMPH_EXCEPTION_LOCATION);
      }
    }

    /// Empty Constructor
    ElasticityTensor() {}

  public:
    /// Empty virtual Destructor
    virtual ~ElasticityTensor() {}

  public:
    ///\short Return the appropriate independent component
    /// via the index translation scheme (const version).
    double operator()(const unsigned& i,
                      const unsigned& j,
                      const unsigned& k,
                      const unsigned& l) const
    {
      // Range check
#ifdef PARANOID
      range_check(i, j, k, l);
#endif
      return independent_component(Index[i][j][k][l]);
    }
  };

  //===================================================================
  /// An isotropic elasticity tensor defined in terms of Young's modulus
  /// and Poisson's ratio. The elasticity tensor is assumed to be
  /// non-dimensionalised on some reference value for Young's modulus
  /// so the value provided to the constructor (if any) is to be
  /// interpreted as the ratio of the actual Young's modulus to the
  /// Young's modulus used to non-dimensionalise the stresses/tractions
  /// in the governing equations.
  //===================================================================
  class IsotropicElasticityTensor : public ElasticityTensor
  {
    // Storage for the independent components of the elasticity tensor
    double C[4];

    // Translation scheme between the 21 independent components of the general
    // elasticity tensor and the isotropic case
    static const unsigned StaticIndex[21];

  public:
    /// \short Constructor. Passing in the values of the Poisson's ratio
    /// and Young's modulus (interpreted as the ratio of the actual
    /// Young's modulus to the Young's modulus (or other reference stiffness)
    /// used to non-dimensionalise stresses and tractions in the governing
    /// equations).
    IsotropicElasticityTensor(const double& nu, const double& E) :
      ElasticityTensor()
    {
      // Set the three independent components
      C[0] = 0.0;
      double lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
      double mu = E / (2.0 * (1.0 + nu));
      this->set_lame_coefficients(lambda, mu);
    }

    /// \short Constructor. Passing in the value of the Poisson's ratio.
    /// Stresses and tractions in the governing equations are assumed
    /// to have been non-dimensionalised on Young's modulus.
    IsotropicElasticityTensor(const double& nu) : ElasticityTensor()
    {
      // Set the three independent components
      C[0] = 0.0;

      // reference value
      double E = 1.0;
      double lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
      double mu = E / (2.0 * (1.0 + nu));
      this->set_lame_coefficients(lambda, mu);
    }

    /// \short Constructur. Passing in the values of the two lame
    /// coefficients directly (interpreted as the ratios of these
    /// quantities to a reference stiffness used to non-dimensionalised
    IsotropicElasticityTensor(const Vector<double>& lame)
    {
      // Set the three independent components
      C[0] = 0.0;
      this->set_lame_coefficients(lame[0], lame[1]);
    }

    /// Overload the independent coefficient function
    inline double independent_component(const unsigned& i) const
    {
      return C[StaticIndex[i]];
    }

  private:
    // Set the values of the lame coefficients
    void set_lame_coefficients(const double& lambda, const double& mu)
    {
      C[1] = lambda + 2.0 * mu;
      C[2] = lambda;
      C[3] = mu;
    }
  };

  //===================================================================
  /// An isotropic elasticity tensor defined in terms of Young's modulus
  /// and Poisson's ratio. The elasticity tensor is assumed to be
  /// non-dimensionalised on some reference value for Young's modulus
  /// so the value provided to the constructor (if any) is to be
  /// interpreted as the ratio of the actual Young's modulus to the
  /// Young's modulus used to non-dimensionalise the stresses/tractions
  /// in the governing equations.
  //===================================================================
  class DeviatoricIsotropicElasticityTensor : public ElasticityTensor
  {
    // Storage for the independent components of the elasticity tensor
    double C[3];

    // Storage for the first Lame parameter
    double Lambda;

    // Storage for the second Lame parameter
    double Mu;

    // Translation scheme between the 21 independent components of the general
    // elasticity tensor and the isotropic case
    static const unsigned StaticIndex[21];

  public:
    /// \short Constructor. For use with incompressibility. Requires no
    /// parameters since Poisson's ratio is fixed at 0.5 and lambda is set to a
    /// dummy value of 0 (since it would be infinite)
    DeviatoricIsotropicElasticityTensor() : ElasticityTensor()
    {
      C[0] = 0.0;
      double E = 1.0;
      double nu = 0.5;
      double lambda = 0.0;
      double mu = E / (2.0 * (1.0 + nu));
      this->set_lame_coefficients(lambda, mu);
    }

    /// \short Constructor. Passing in the values of the Poisson's ratio
    /// and Young's modulus (interpreted as the ratio of the actual
    /// Young's modulus to the Young's modulus (or other reference stiffness)
    /// used to non-dimensionalise stresses and tractions in the governing
    /// equations).
    DeviatoricIsotropicElasticityTensor(const double& nu, const double& E) :
      ElasticityTensor()
    {
      // Set the three independent components
      C[0] = 0.0;
      double lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
      double mu = E / (2.0 * (1.0 + nu));
      this->set_lame_coefficients(lambda, mu);
    }

    /// \short Constructor. Passing in the value of the Poisson's ratio.
    /// Stresses and tractions in the governing equations are assumed
    /// to have been non-dimensionalised on Young's modulus.
    DeviatoricIsotropicElasticityTensor(const double& nu) : ElasticityTensor()
    {
      // Set the three independent components
      C[0] = 0.0;

      // reference value
      double E = 1.0;
      double lambda = E * nu / ((1.0 + nu) * (1.0 - 2.0 * nu));
      double mu = E / (2.0 * (1.0 + nu));
      this->set_lame_coefficients(lambda, mu);
    }

    /// \short Constructur. Passing in the values of the two lame
    /// coefficients directly (interpreted as the ratios of these
    /// quantities to a reference stiffness used to non-dimensionalised
    DeviatoricIsotropicElasticityTensor(const Vector<double>& lame)
    {
      // Set the three independent components
      C[0] = 0.0;
      this->set_lame_coefficients(lame[0], lame[1]);
    }

    /// Overload the independent coefficient function
    inline double independent_component(const unsigned& i) const
    {
      return C[StaticIndex[i]];
    }

    /// Accessor function for the first lame parameter
    const double& lambda() const
    {
      return Lambda;
    }

    /// Accessor function for the second lame parameter
    const double& mu() const
    {
      return Mu;
    }

  private:
    // Set the values of the lame coefficients
    void set_lame_coefficients(const double& lambda, const double& mu)
    {
      C[1] = 2.0 * mu;
      C[2] = mu;

      Lambda = lambda;
      Mu = mu;
    }
  };

} // namespace oomph
#endif
