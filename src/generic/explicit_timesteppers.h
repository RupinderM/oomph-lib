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
// Header functions for classes that represent explicit time-stepping schemes

// Include guards to prevent multiple inclusion of this header
#ifndef OOMPH_EXPLICIT_TIMESTEPPERS
#define OOMPH_EXPLICIT_TIMESTEPPERS

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

#include "Vector.h"
#include "double_vector.h"
#include "oomph_utilities.h"

namespace oomph
{
  class Time;

  //===============================================================
  /// Class for objects than can be advanced in time by an Explicit
  /// Timestepper.
  /// WARNING: For explicit time stepping to work the object's residual
  /// function (as used by get_inverse_mass_matrix_times_residuals(..)) MUST
  /// be in the form r = f(t, u) - [timestepper approximation to dudt]!
  /// Standard implicit time stepping will work with plenty of residuals that
  /// don't fit into this form. Some examples where implicit time stepping
  /// will work fine but explicit will fail:
  /// 1) The negation of the above formula, this implementation will end up
  /// using dudt = - f(u,t).
  /// 2) A residual which is implicit or non-linear in dudt, such as r = dudt
  /// - u x dudt.
  //===============================================================
  class ExplicitTimeSteppableObject
  {
    // Dummy double value for time
    static double Dummy_time_value;

  public:
    /// Empty constructor
    ExplicitTimeSteppableObject() {}

    /// Broken copy constructor
    ExplicitTimeSteppableObject(const ExplicitTimeSteppableObject&)
    {
      BrokenCopy::broken_copy("ExplicitTimeSteppableObject");
    }

    /// Broken assignment operator
    void operator=(const ExplicitTimeSteppableObject&)
    {
      BrokenCopy::broken_assign("ExplicitTimeSteppableObject");
    }

    /// Empty destructor
    virtual ~ExplicitTimeSteppableObject() {}

    ///\short A single virtual function that returns the residuals
    /// vector multiplied by the inverse mass matrix
    virtual void get_dvaluesdt(DoubleVector& minv_res);

    /// Function that gets the values of the dofs in the object
    virtual void get_dofs(DoubleVector& dofs) const;

    /// Function that gets the history values of the dofs in the object
    virtual void get_dofs(const unsigned& t, DoubleVector& dofs) const;

    /// Function that sets the values of the dofs in the object
    virtual void set_dofs(const DoubleVector& dofs);

    /// Function that adds the values to the dofs
    virtual void add_to_dofs(const double& lambda,
                             const DoubleVector& increment_dofs);

    /// \short Empty virtual function to do anything needed before a stage of
    /// an explicit time step (Runge-Kutta steps contain multiple stages per
    /// time step, most others only contain one).
    virtual void actions_before_explicit_stage() {}

    /// \short Empty virtual function that should be overloaded to update and
    /// slaved data or boundary conditions that should be advanced after each
    /// stage of an explicit time step (Runge-Kutta steps contain multiple
    /// stages per time step, most others only contain one).
    virtual void actions_after_explicit_stage() {}

    /// \short Empty virtual function that can be overloaded to do anything
    /// needed before an explicit step.
    virtual void actions_before_explicit_timestep() {}

    /// \short Empty virtual function that can be overloaded to do anything
    /// needed after an explicit step.
    virtual void actions_after_explicit_timestep() {}

    ///\short Broken virtual function that should be overloaded to
    /// return access to the local time in the object
    virtual double& time();

    /// \short Virtual function that should be overloaded to return a pointer to
    /// a Time object.
    virtual Time* time_pt() const;
  };

  //=====================================================================
  /// A Base class for explicit timesteppers
  //=====================================================================
  class ExplicitTimeStepper
  {
  protected:
    /// \short String that indicates the type of the timestepper
    ///(e.g. "RungeKutta", etc.)
    std::string Type;

  public:
    /// \short Empty Constructor.
    ExplicitTimeStepper() {}

    /// Broken copy constructor
    ExplicitTimeStepper(const ExplicitTimeStepper&)
    {
      BrokenCopy::broken_copy("ExplicitTimeStepper");
    }

    /// Broken assignment operator
    void operator=(const ExplicitTimeStepper&)
    {
      BrokenCopy::broken_assign("ExplicitTimeStepper");
    }

    /// Empty virtual destructor --- no memory is allocated in this class
    virtual ~ExplicitTimeStepper() {}

    /// Pure virtual function that is used to advance time in the object
    // referenced by object_pt by an amount dt
    virtual void timestep(ExplicitTimeSteppableObject* const& object_pt,
                          const double& dt) = 0;
  };

  ///===========================================================
  /// Simple first-order Euler Timestepping
  //============================================================
  class Euler : public ExplicitTimeStepper
  {
  public:
    /// Constructor, set the type
    Euler()
    {
      Type = "Euler";
    }

    /// Broken copy constructor
    Euler(const Euler&)
    {
      BrokenCopy::broken_copy("Euler");
    }

    /// Broken assignment operator
    void operator=(const Euler&)
    {
      BrokenCopy::broken_assign("Euler");
    }

    /// \short Overload function that is used to advance time in the object
    /// reference by object_pt by an amount dt
    void timestep(ExplicitTimeSteppableObject* const& object_pt,
                  const double& dt);
  };

  ///===========================================================
  /// Standard Runge Kutta Timestepping
  //============================================================
  template<unsigned ORDER>
  class RungeKutta : public ExplicitTimeStepper
  {
  public:
    /// Constructor, set the type
    RungeKutta()
    {
      Type = "RungeKutta";
    }

    /// Broken copy constructor
    RungeKutta(const RungeKutta&)
    {
      BrokenCopy::broken_copy("RungeKutta");
    }

    /// Broken assignment operator
    void operator=(const RungeKutta&)
    {
      BrokenCopy::broken_assign("RungeKutta");
    }

    /// Function that is used to advance time in the object
    // reference by object_pt by an amount dt
    void timestep(ExplicitTimeSteppableObject* const& object_pt,
                  const double& dt);
  };

  ///===========================================================
  /// Runge Kutta Timestepping that uses low storage
  //============================================================
  template<unsigned ORDER>
  class LowStorageRungeKutta : public ExplicitTimeStepper
  {
    // Storage for the coefficients
    Vector<double> A, B, C;

  public:
    /// Constructor, set the type
    LowStorageRungeKutta();

    /// Broken copy constructor
    LowStorageRungeKutta(const LowStorageRungeKutta&)
    {
      BrokenCopy::broken_copy("LowStorageRungeKutta");
    }

    /// Broken assignment operator
    void operator=(const LowStorageRungeKutta&)
    {
      BrokenCopy::broken_assign("LowStorageRungeKutta");
    }

    /// Function that is used to advance the solution by time dt
    void timestep(ExplicitTimeSteppableObject* const& object_pt,
                  const double& dt);
  };

  ///===========================================================
  /// An explicit version of BDF3 (i.e. uses derivative evaluation at y_n
  /// instead of y_{n+1}). Useful as a predictor because it is third order
  /// accurate but requires only one function evaluation (i.e. only one mass
  /// matrix inversion + residual calculation).
  //============================================================
  class EBDF3 : public ExplicitTimeStepper
  {
    double Yn_weight;
    double Ynm1_weight;
    double Ynm2_weight;
    double Fn_weight;

  public:
    /// Constructor, set the type
    EBDF3() {}

    /// Broken copy constructor
    EBDF3(const EBDF3&)
    {
      BrokenCopy::broken_copy("EBDF3");
    }

    /// Broken assignment operator
    void operator=(const EBDF3&)
    {
      BrokenCopy::broken_assign("EBDF3");
    }

    void set_weights(const double& dtn,
                     const double& dtnm1,
                     const double& dtnm2);

    /// Function that is used to advance the solution by time dt
    void timestep(ExplicitTimeSteppableObject* const& object_pt,
                  const double& dt);
  };

} // namespace oomph

#endif
