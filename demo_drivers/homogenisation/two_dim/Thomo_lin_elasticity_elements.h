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
// Header file for TPoisson elements
#ifndef OOMPH_THOMOGENISED_LINEAR_ELASTICITY_ELEMENTS_HEADER
#define OOMPH_THOMOGENISED_LINEAR_ELASTICITY_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// OOMPH-LIB headers
#include "generic/nodes.h"
#include "generic/oomph_utilities.h"
#include "generic/Telements.h"
#include "homo_lin_elasticity_elements.h"

namespace oomph
{
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  // THomogenisedLinearElasticityElement
  ////////////////////////////////////////////////////////////////////////
  ////////////////////////////////////////////////////////////////////////

  //======================================================================
  /// THomogenisedLinearElasticityElement<DIM,NNODE_1D> elements are
  /// isoparametric triangular
  /// DIM-dimensional HomogenisedLinearElasticity elements with
  /// NNODE_1D nodal points along each
  /// element edge. Inherits from TElement and
  /// HomogenisedLinearElasticityEquations
  //======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class THomogenisedLinearElasticityElement :
    public TElement<DIM, NNODE_1D>,
    public HomogenisedLinearElasticityEquations<DIM>
  {
  public:
    ///\short  Constructor: Call constructors for TElement and
    /// HomogenisedLinearElasticity equations
    THomogenisedLinearElasticityElement() :
      TElement<DIM, NNODE_1D>(), HomogenisedLinearElasticityEquations<DIM>()
    {
    }

    /// Broken copy constructor
    THomogenisedLinearElasticityElement(
      const THomogenisedLinearElasticityElement<DIM, NNODE_1D>& dummy)
    {
      BrokenCopy::broken_copy("THomogenisedLinearElasticityElement");
    }

    /// \short Output function:
    ///  x,y,u   or    x,y,z,u
    void output(std::ostream& outfile)
    {
      HomogenisedLinearElasticityEquations<DIM>::output(outfile);
    }

    ///  \short Output function:
    ///   x,y,u   or    x,y,z,u at n_plot^DIM plot points
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      // Vector of local coordinates
      Vector<double> s(DIM);

      Vector<double> x(DIM);
      Vector<double> u(3);

      // Tecplot header info
      outfile << this->tecplot_zone_string(nplot);

      // Loop over plot points
      unsigned num_plot_points = this->nplot_points(nplot);
      for (unsigned iplot = 0; iplot < num_plot_points; iplot++)
      {
        // Get local coordinates of plot point
        this->get_s_plot(iplot, nplot, s);
        this->interpolated_x(s, x);
        this->interpolated_u_linear_elasticity(s, u);

        for (unsigned i = 0; i < DIM; i++)
        {
          outfile << x[i] << " ";
        }
        for (unsigned i = 0; i < 3; i++)
        {
          outfile << u[i] << " ";
        }
        outfile << "\n";
      }

      // Write tecplot footer (e.g. FE connectivity lists)
      this->write_tecplot_zone_footer(outfile, nplot);
    }

    /// \short C-style output function:
    ///  x,y,u   or    x,y,z,u
    void output(FILE* file_pt)
    {
      HomogenisedLinearElasticityEquations<DIM>::output(file_pt);
    }

    ///  \short C-style output function:
    ///   x,y,u   or    x,y,z,u at n_plot^DIM plot points
    void output(FILE* file_pt, const unsigned& n_plot)
    {
      HomogenisedLinearElasticityEquations<DIM>::output(file_pt, n_plot);
    }
  };

  //=======================================================================
  /// Face geometry for the THomogenisedLinearElasticityElement elements: The
  /// spatial dimension of the face elements is one lower than that of the bulk
  /// element but they have the same number of points along their 1D edges.
  //=======================================================================
  template<unsigned DIM, unsigned NNODE_1D>
  class FaceGeometry<THomogenisedLinearElasticityElement<DIM, NNODE_1D>> :
    public virtual TElement<DIM - 1, NNODE_1D>
  {
  public:
    /// \short Constructor: Call the constructor for the
    /// appropriate lower-dimensional QElement
    FaceGeometry() : TElement<DIM - 1, NNODE_1D>() {}
  };

} // namespace oomph

#endif
