// LIC// ====================================================================
// LIC// This file forms part of oomph-lib, the object-oriented,
// LIC// multi-physics finite-element library, available
// LIC// at http://www.oomph-lib.org.
// LIC//
// LIC// Copyright (C) 2006-2022 Matthias Heil and Andrew Hazel
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
#ifndef OOMPH_DOMAIN_HEADER
#define OOMPH_DOMAIN_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// oomph-lib headers
#include "macro_element.h"

namespace oomph
{
  class MacroElement;
  class GeomObject;

  class GeomReference;


  //=================================================================
  /// Base class for  Domains with curvilinear and/or time-dependent
  /// boundaries. Domain boundaries are typically represented by GeomObject s
  /// and the Domain itself is decomposed into a number of MacroElement s
  /// as shown in this 2D example:
  /// \image html DomainWithMacroElementSketch.gif
  /// Any instantiation of a specific Domain needs to implement the pure
  /// virtual member function
  /// \code Domain::macro_element_boundary(...) \endcode
  /// which returns a Vector representation of each of the MacroElement s'
  /// boundaries, parametrised by the coordinate(s) along this
  /// boundary. For instance, in the above example,
  /// the eastern boundary of MacroElement 1 is given by
  /// the appropriate fraction of the curvilinear boundary;
  /// its northern boundary (which coincides with the southern
  /// boundary of MacroElement 2) is given by the straight line emanating
  /// from the curvilinear boundary, etc. The MacroElement s obtain
  /// their boundary positions via member function pointers to
  /// \c Domain::macro_element_boundary(...).
  //=================================================================
  class Domain
  {
  public:
    /// Constructor
    Domain()
    {
      // Make sure all containers are empty (a bit paranoid, I know...)
      Macro_element_pt.resize(0);
    }

    /// Broken copy constructor
    Domain(const Domain&) = delete;

    /// Broken assignment operator
    void operator=(const Domain&) = delete;

    /// Destructor: Strictly speaking, whoever creates an object
    /// dynamically should be responsible for the cleanup of said object
    /// but it makes sense here for the Domain to generically kill any
    /// MacroElements left over in the MacroElement container (if it hasn't
    /// already been done in the derived class) to avoid memory leaks.
    virtual ~Domain()
    {
      // How many macro elements are there?
      unsigned n_macro_element = Macro_element_pt.size();

      // Loop over the macro elements
      for (unsigned i = 0; i < n_macro_element; i++)
      {
        // They might have already deleted (some or all of) the macro elements
        // so skip them if they've already been made null pointers
        if (Macro_element_pt[i] != 0)
        {
          // Delete the i-th macro element
          delete Macro_element_pt[i];

          // Make it a null pointer
          Macro_element_pt[i] = 0;
        }
      } // for (unsigned i=0;i<n_macro_element;i++)

      // Now clear the storage. NOTE: We can't just call this function
      // as this would only delete the pointers to the macro elements,
      // not the macro elements themselves!
      Macro_element_pt.clear();

    } // End of ~Domain


    /// Access to i-th macro element
    MacroElement* macro_element_pt(const unsigned& i)
    {
      return Macro_element_pt[i];
    }


    /// Number of macro elements in domain
    unsigned nmacro_element()
    {
      return Macro_element_pt.size();
    }

    /// Output macro elements
    void output(const std::string& filename, const unsigned& nplot)
    {
      std::ofstream outfile;
      outfile.open(filename.c_str());
      output(outfile, nplot);
      outfile.close();
    }

    /// Output macro elements
    void output(std::ostream& outfile, const unsigned& nplot)
    {
      unsigned nmacro = Macro_element_pt.size();
      for (unsigned i_macro = 0; i_macro < nmacro; i_macro++)
      {
        Macro_element_pt[i_macro]->output(outfile, nplot);
      }
    }


    /// Vector representation of the  i_macro-th macro element
    /// boundary i_direct (e.g. N/S/W/E in 2D) at time level t
    /// (t=0: present; t>0: previous): f(s)
    virtual void macro_element_boundary(const unsigned& t,
                                        const unsigned& i_macro,
                                        const unsigned& i_direct,
                                        const Vector<double>& s,
                                        Vector<double>& f) = 0;


    /// Vector representation of the i_macro-th macro element
    /// boundary i_direct (e.g. N/S/W/E in 2D) at continuous time, t
    virtual void macro_element_boundary(const double& t,
                                        const unsigned& i_macro,
                                        const unsigned& i_direct,
                                        const Vector<double>& s,
                                        Vector<double>& f)
    {
      // Throw an error
      throw OomphLibError("Domain::macro_element_boundary() is broken virtual.",
                          OOMPH_CURRENT_FUNCTION,
                          OOMPH_EXCEPTION_LOCATION);
    } // End of macro_element_boundary


    /// Vector representation of the  i_macro-th macro element
    /// boundary i_direct (e.g. N/S/W/E in 2D) at current time: f(s).
    void macro_element_boundary(const unsigned& i_macro,
                                const unsigned& i_direct,
                                const Vector<double>& s,
                                Vector<double>& f)
    {
      // Call unsteady version for current time
      unsigned t = 0;
      macro_element_boundary(t, i_macro, i_direct, s, f);
    }


    /// Output all macro element boundaries as tecplot zones
    void output_macro_element_boundaries(const std::string& filename,
                                         const unsigned& nplot)
    {
      std::ofstream outfile;
      outfile.open(filename.c_str());
      output_macro_element_boundaries(outfile, nplot);
      outfile.close();
    }

    /// Output all macro element boundaries as tecplot zones
    void output_macro_element_boundaries(std::ostream& outfile,
                                         const unsigned& nplot)
    {
      // Loop over macro elements
      unsigned nmacro = nmacro_element();
      for (unsigned i = 0; i < nmacro; i++)
      {
        macro_element_pt(i)->output_macro_element_boundaries(outfile, nplot);
      }
    }


    /// Vector representation of the  i_macro-th macro element
    /// boundary derivatives i_direct (e.g. N/S/W/E in 2D) at time level t
    /// (t=0: present; t>0: previous): f(s). Broken virtual.
    virtual void dmacro_element_boundary(const unsigned& t,
                                         const unsigned& i_macro,
                                         const unsigned& i_direct,
                                         const Vector<double>& s,
                                         Vector<double>& f)
    {
      throw OomphLibError(
        "Domain::dmacro_element_boundary() is broken virtual.",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }


    /// Vector representation of the  i_macro-th macro element boundary
    /// derivatives i_direct (e.g. N/S/W/E in 2D) at continuous time level t.
    /// Broken virtual.
    virtual void dmacro_element_boundary(const double& t,
                                         const unsigned& i_macro,
                                         const unsigned& i_direct,
                                         const Vector<double>& s,
                                         Vector<double>& f)
    {
      throw OomphLibError(
        "Domain::dmacro_element_boundary() is broken virtual.",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }


    /// Vector representation of the  i_macro-th macro element
    /// boundary derivatives i_direct (e.g. N/S/W/E in 2D) at current time:
    /// f(s).
    void dmacro_element_boundary(const unsigned& i_macro,
                                 const unsigned& i_direct,
                                 const Vector<double>& s,
                                 Vector<double>& f)
    {
      // Call unsteady version for current time
      unsigned t = 0;
      dmacro_element_boundary(t, i_macro, i_direct, s, f);
    }


    /// Vector representation of the  i_macro-th macro element
    /// boundary second derivatives i_direct (e.g. N/S/W/E in 2D) at time level
    /// t (t=0: present; t>0: previous): f(s). Broken virtual.
    virtual void d2macro_element_boundary(const unsigned& t,
                                          const unsigned& i_macro,
                                          const unsigned& i_direct,
                                          const Vector<double>& s,
                                          Vector<double>& f)
    {
      throw OomphLibError(
        "Domain::d2macro_element_boundary() is broken virtual.",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Vector representation of the i_macro-th macro element boundary
    /// seocond derivatives i_direct (e.g. N/S/W/E in 2D) at continuous time
    /// level t. Broken virtual.
    virtual void d2macro_element_boundary(const double& t,
                                          const unsigned& i_macro,
                                          const unsigned& i_direct,
                                          const Vector<double>& s,
                                          Vector<double>& f)
    {
      throw OomphLibError(
        "Domain::d2macro_element_boundary() is broken virtual.",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Vector representation of the  i_macro-th macro element
    /// boundary second derivatives i_direct (e.g. N/S/W/E in 2D) at
    /// current time: f(s).
    void d2macro_element_boundary(const unsigned& i_macro,
                                  const unsigned& i_direct,
                                  const Vector<double>& s,
                                  Vector<double>& f)
    {
      // Call unsteady version for current time
      unsigned t = 0;
      d2macro_element_boundary(t, i_macro, i_direct, s, f);
    }


  protected:
    /// Vector of pointers to macro elements
    Vector<MacroElement*> Macro_element_pt;
  };


  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////
  // Warped cube domain
  /////////////////////////////////////////////////////////////////////////
  /////////////////////////////////////////////////////////////////////////


  //=================================================================
  /// Warped cube as domain which is parametrised by
  /// a single macro element
  //=================================================================
  class WarpedCubeDomain : public Domain
  {
  public:
    /// Constructor:
    WarpedCubeDomain()
    {
      // Resize
      Macro_element_pt.resize(1);

      // Create macro element
      Macro_element_pt[0] = new QMacroElement<3>(this, 0);
    }

    /// Broken copy constructor
    WarpedCubeDomain(const WarpedCubeDomain&) = delete;

    /// Broken assignment operator
    void operator=(const WarpedCubeDomain&) = delete;

    /// Destructor (empty; clean up handled in base class)
    ~WarpedCubeDomain() {}


    /// Warp the unit cube
    void warp_it(Vector<double>& f);


    /// Vector representation of the  i_macro-th macro element
    /// boundary i_direct (L/R/D/U/B/F) at time level t
    /// (t=0: present; t>0: previous):
    /// f(s).
    void macro_element_boundary(const unsigned& t,
                                const unsigned& i_macro,
                                const unsigned& i_direct,
                                const Vector<double>& s,
                                Vector<double>& f);


  private:
    /// Left boundary face
    /// zeta \f$ \in [-1,1]^2 \f$
    void r_L(const unsigned& t, const Vector<double>& zeta, Vector<double>& f);

    /// Right boundary face
    /// zeta \f$ \in [-1,1]^2 \f$
    void r_R(const unsigned& t, const Vector<double>& zeta, Vector<double>& f);


    /// Down boundary face
    /// zeta \f$ \in [-1,1]^2 \f$
    void r_D(const unsigned& t, const Vector<double>& zeta, Vector<double>& f);


    /// Up boundary face
    /// zeta \f$ \in [-1,1]^2 \f$
    void r_U(const unsigned& t, const Vector<double>& zeta, Vector<double>& f);


    /// Back boundary face
    /// zeta \f$ \in [-1,1]^2 \f$
    void r_B(const unsigned& t, const Vector<double>& zeta, Vector<double>& f);


    /// Front boundary face
    /// zeta \f$ \in [-1,1]^2 \f$
    void r_F(const unsigned& t, const Vector<double>& zeta, Vector<double>& f);
  };


} // namespace oomph

#endif
