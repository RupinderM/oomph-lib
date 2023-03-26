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
#ifndef OOMPH_EXTRUDED_MACRO_ELEMENT_HEADER
#define OOMPH_EXTRUDED_MACRO_ELEMENT_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

// oomph-lib headers
#include "Vector.h"
#include "macro_element.h"
#include "oomph_utilities.h"
#include "quadtree.h"
#include "octree.h"

namespace oomph
{
  class ExtrudedDomain;

  //===================================================================
  ///
  ///
  ///
  /// DRAIG: FILL IN COMPLETE DESCRIPTION ONCE FINISHED...
  ///
  ///
  ///
  //===================================================================
  class ExtrudedMacroElement : public virtual MacroElement
  {
  public:
    /// Constructor: Pass pointer to ExtrudedDomain and the ID number
    /// of the ExtrudedMacroElement within that ExtrudedDomain.
    /// NOTE: The base class MacroElement requires a pointer to a Domain
    /// object. However, the ExtrudedDomain class has not yet been defined
    /// which means the compiler cannot know yet that its derives from Domain.
    /// To avoid a compiler error a null pointer is passed to the base class
    /// MacroElement.
    ExtrudedMacroElement(ExtrudedDomain* extruded_domain_pt,
                         unsigned macro_element_number)
      : MacroElement(0, macro_element_number),
        Extruded_domain_pt(extruded_domain_pt)
    {
#ifdef LEAK_CHECK
      LeakCheckNames::ExtrudedMacroElement_build += 1;
#endif
    } // End of ExtrudedMacroElement


    /// Default constructor (empty and broken)
    ExtrudedMacroElement()
    {
      // Throw an error!
      throw OomphLibError(
        "Don't call empty constructor for ExtrudedMacroElement!",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    } // End of ExtrudedMacroElement


    /// Broken copy constructor
    ExtrudedMacroElement(const ExtrudedMacroElement& dummy) = delete;

    /// Broken assignment operator
    void operator=(const ExtrudedMacroElement&) = delete;

    /// Empty destructor
    ~ExtrudedMacroElement() override
    {
#ifdef LEAK_CHECK
      LeakCheckNames::ExtrudedMacroElement_build -= 1;
#endif
    } // End of ~ExtrudedMacroElement


    /// Access function to the ExtrudedDomain
    ExtrudedDomain*& extruded_domain_pt()
    {
      // Return a pointer to the extruded domain within which this extruded
      // macro element lies
      return Extruded_domain_pt;
    } // End of extruded_domain_pt

  protected:
    /// Pointer to the extruded domain
    ExtrudedDomain* Extruded_domain_pt;
  };

  /// ////////////////////////////////////////////////////////////////////////
  /// ////////////////////////////////////////////////////////////////////////
  /// ////////////////////////////////////////////////////////////////////////

  //===================================================================
  /// QExtrudedMacroElement
  ///
  /// QExtrudedMacroElements are ExtrudedMacroElements with
  /// linear/quadrilateral/hexahedral shape. This class is empty and
  /// simply establishes the dimension as the template parameter.
  //===================================================================
  template<int DIM>
  class QExtrudedMacroElement : public ExtrudedMacroElement
  {
  };

  //===================================================================
  ///
  ///
  ///
  /// DRAIG: FILL IN COMPLETE DESCRIPTION ONCE FINISHED...
  ///
  ///
  ///
  //===================================================================
  template<>
  class QExtrudedMacroElement<3> : public ExtrudedMacroElement
  {
  public:
    /// Constructor: Pass the pointer to the domain and the ID
    /// number of this extruded macro element
    QExtrudedMacroElement(ExtrudedDomain* domain_pt,
                          const unsigned& macro_element_number)
      : MacroElement(0, macro_element_number),
        ExtrudedMacroElement(domain_pt, macro_element_number)
    {
    }

    /// Default constructor (empty and broken)
    QExtrudedMacroElement()
    {
      // Create an error message
      std::string error =
        "Don't call empty constructor for QExtrudedMacroElement!";

      // Throw the error message
      throw OomphLibError(
        error, OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
    } // End of QExtrudedMacroElement


    /// Broken copy constructor
    QExtrudedMacroElement(const QExtrudedMacroElement& dummy) = delete;

    /// Broken assignment operator
    void operator=(const QExtrudedMacroElement&) = delete;

    /// Empty destructor
    ~QExtrudedMacroElement() override {}

    /// Plot: x,y,t in tecplot format
    void output(const unsigned& t, std::ostream& outfile, const unsigned& nplot) override
    {
      // Make sure that t=0 otherwise this doesn't make sense
      if (t != 0)
      {
        // Create an output stream
        std::ostringstream error_message_stream;

        // Create an error message
        error_message_stream << "This output function outputs a space-time\n"
                             << "representation of the solution. As such, it\n"
                             << "does not make sense to output the solution\n"
                             << "at a previous time level!" << std::endl;

        // Throw an error
        throw OomphLibError(error_message_stream.str(),
                            OOMPH_CURRENT_FUNCTION,
                            OOMPH_EXCEPTION_LOCATION);
      }

      // Storage for the local (space-time) coordinates
      Vector<double> s(3, 0.0);

      // Storage for the global (space-time) coordinates
      Vector<double> x(3, 0.0);

      // Output the header
      outfile << "ZONE I=" << nplot << ", J=" << nplot << ", K=" << nplot
              << std::endl;

      // Loop over the plot points in the t-direction
      for (unsigned i = 0; i < nplot; i++)
      {
        // Calculate the time value
        // s[2]=-1.0+2.0*double(i)/double(nplot-1);
        s[2] = -1.0 + 2.0 * double(i) / double(nplot - 1);

        // Loop over the plot points in the y-direction
        for (unsigned j = 0; j < nplot; j++)
        {
          // Calculate the y value
          s[1] = -1.0 + 2.0 * double(j) / double(nplot - 1);

          // Loop over the plot points in the x-direction
          for (unsigned k = 0; k < nplot; k++)
          {
            // Calculate the x value
            s[0] = -1.0 + 2.0 * double(k) / double(nplot - 1);

            // Get the corresponding global space-time coordinates.
            // NOTE: Calling the macro_map function from the base class requires
            // the time level. To make the call function normally, we simply
            // pass the argument t=0 and the appropriate function will be
            // called.
            macro_map(t, s, x);

            // Output the global coordinates
            outfile << x[0] << " " << x[1] << " " << x[2] << " " << 0.0
                    << std::endl;
          }
        } // for (unsigned j=0;j<nplot;j++)
      } // for (unsigned i=0;i<nplot;i++)
    } // End of output


    /// Get the global position r(s) at the continuous time, t
    void macro_map(const unsigned& t,
                   const Vector<double>& s,
                   Vector<double>& r) override;


    /// Output all macro element boundaries as tecplot zones
    void output_macro_element_boundaries(std::ostream& outfile,
                                         const unsigned& nplot) override;
  };
} // End of namespace oomph

#endif
