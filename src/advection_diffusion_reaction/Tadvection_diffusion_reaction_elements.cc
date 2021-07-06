// LIC// ====================================================================
// LIC// This file forms part of oomph-lib, the object-oriented,
// LIC// multi-physics finite-element library, available
// LIC// at http://www.oomph-lib.org.
// LIC//
// LIC//    Version 1.0; svn revision $LastChangedRevision: 1097 $
// LIC//
// LIC// $LastChangedDate: 2015-12-17 11:53:17 +0000 (Thu, 17 Dec 2015) $
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
// Non-inline functions for Poisson elements
#include "Tadvection_diffusion_reaction_elements.h"

namespace oomph
{
  /////////////////////////////////////////////////////////////////////////
  // TAdvectionDiffusionReactionElement
  /////////////////////////////////////////////////////////////////////////

  //====================================================================
  // Force build of templates
  //====================================================================
  template class TAdvectionDiffusionReactionElement<1, 1, 2>;
  template class TAdvectionDiffusionReactionElement<1, 1, 3>;
  template class TAdvectionDiffusionReactionElement<1, 1, 4>;

  template class TAdvectionDiffusionReactionElement<1, 2, 2>;
  template class TAdvectionDiffusionReactionElement<1, 2, 3>;
  template class TAdvectionDiffusionReactionElement<1, 2, 4>;

  template class TAdvectionDiffusionReactionElement<1, 3, 2>;
  template class TAdvectionDiffusionReactionElement<1, 3, 3>;

  template class TAdvectionDiffusionReactionElement<2, 1, 2>;
  template class TAdvectionDiffusionReactionElement<2, 1, 3>;
  template class TAdvectionDiffusionReactionElement<2, 1, 4>;

  template class TAdvectionDiffusionReactionElement<2, 2, 2>;
  template class TAdvectionDiffusionReactionElement<2, 2, 3>;
  template class TAdvectionDiffusionReactionElement<2, 2, 4>;

  template class TAdvectionDiffusionReactionElement<2, 3, 2>;
  template class TAdvectionDiffusionReactionElement<2, 3, 3>;

} // namespace oomph
