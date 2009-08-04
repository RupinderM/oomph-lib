//LIC// ====================================================================
//LIC// This file forms part of oomph-lib, the object-oriented, 
//LIC// multi-physics finite-element library, available 
//LIC// at http://www.oomph-lib.org.
//LIC// 
//LIC//           Version 0.90. August 3, 2009.
//LIC// 
//LIC// Copyright (C) 2006-2009 Matthias Heil and Andrew Hazel
//LIC// 
//LIC// This library is free software; you can redistribute it and/or
//LIC// modify it under the terms of the GNU Lesser General Public
//LIC// License as published by the Free Software Foundation; either
//LIC// version 2.1 of the License, or (at your option) any later version.
//LIC// 
//LIC// This library is distributed in the hope that it will be useful,
//LIC// but WITHOUT ANY WARRANTY; without even the implied warranty of
//LIC// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//LIC// Lesser General Public License for more details.
//LIC// 
//LIC// You should have received a copy of the GNU Lesser General Public
//LIC// License along with this library; if not, write to the Free Software
//LIC// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
//LIC// 02110-1301  USA.
//LIC// 
//LIC// The authors may be contacted at oomph-lib@maths.man.ac.uk.
//LIC// 
//LIC//====================================================================
//A header containing useful utility classes, functions and constants

//Include guard to prevent multiple inclusions of the header
#ifndef OOMPH_UTILITIES_HEADER
#define OOMPH_UTILITIES_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

//Standard libray headers
#include<string>
#include<sstream>
#include<fstream>
#include<cmath>


//oomph-lib headers
#include "Vector.h"
#include "oomph_definitions.h"
#include "communicator.h"


#ifdef OOMPH_HAS_MPI
//mpi headers
#include "mpi.h"
#endif


namespace oomph
{

//==============================================
/// Namespace for error messages for broken
/// copy constructors and assignment operators
//==============================================
namespace BrokenCopy
{

 /// Issue error message and terminate execution
 extern void broken_assign(const std::string& class_name);
 
 /// Issue error message and terminate execution
 extern void broken_copy(const std::string& class_name);
 
}

//========================================
/// Namespace for mathematical constants
///
//=======================================
namespace MathematicalConstants
{
 extern double Pi;
}


//================================================================
/// Function-type-object to perform absolute comparison of objects.
/// Apparently this inlines better
//================================================================
template <class T>
class AbsCmp
{
public:

 /// Comparison. Are the values identical or not?
 bool operator()(const T &x, const T &y) const
  {
   return std::abs(x) < std::abs(y);
  }
};





//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////



//====================================================================
/// Namespace for global (cumulative) timings
//====================================================================
namespace CumulativeTimings
{

 /// (Re-)start i-th timer
 extern void start(const unsigned& i);

 /// Halt i-th timer
 extern void halt(const unsigned& i);

 /// Reset i-th timer
 extern void reset(const unsigned& i);

 /// Reset all timers
 extern void reset();

 /// Report time accumulated by i-th timer
 extern double cumulative_time(const unsigned& i);

 /// Set number of timings that can be recorded in parallel
 extern void set_ntimers(const unsigned& ntimers);

 /// Cumulative timings
 extern Vector<clock_t> Timing;

 /// Start times of active timers
 extern Vector<clock_t> Start_time;

}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

//====================================================================
/// Timer 
//====================================================================
class Timer
{

  public:

 /// Constructor: Specify number of timers
 Timer(const unsigned& n_timer)
  {
   set_ntimers(n_timer);
  }
 
 /// (Re-)start i-th timer
 void start(const unsigned& i)
  {
   Start_time[i]=clock();
  }
 
 /// Halt i-th timer
 void halt(const unsigned& i)
  {
   Timing[i]+=clock()-Start_time[i];
  }
 
 /// Report time accumulated by i-th timer
 double cumulative_time(const unsigned& i)
  {
   return double(Timing[i])/CLOCKS_PER_SEC;
  }
 
 /// Reset i-th timer
 void reset(const unsigned& i)
  {
   Timing[i]=clock_t(0.0);
  }
 
 /// Reset all timers
 void reset()
  {
   unsigned n=Timing.size();
   for (unsigned i=0;i<n;i++)
    {
     Timing[i]=clock_t(0.0);
    }
  }
 
 /// Set number of timings that can be recorded in parallel
 void set_ntimers(const unsigned& ntimers)
  {
   Timing.resize(ntimers,clock_t(0.0));
   Start_time.resize(ntimers,clock_t(0.0));
  }
 
  private:
 
 /// Cumulative timings
 Vector<clock_t> Timing;
 
 /// Start times of active timers
 Vector<clock_t> Start_time;

};



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////



//=====================================================================
/// Base class for functions whose parameters can be fitted
/// by Levenberg Marquardt technique.
//=====================================================================
class LevenbergMarquardtFittingFunctionObject
{
 
public:


 /// Constructor: Specify number of fitting parameters
 LevenbergMarquardtFittingFunctionObject(const unsigned& n_param)
  {
   Parameter.resize(n_param);
   for (unsigned i=0;i<n_param;i++)
    {
     Parameter[i]=0.0;
    }
  }

 /// Empty destructor
 virtual ~LevenbergMarquardtFittingFunctionObject() {}

 /// \short Evaluate the fitting function for the current set
 /// of parameters: Pure virtual, must be implemented.
 virtual double fitting_function(const double& x)=0;

 /// \short Evaluate the fitting function and its derivatives
 /// w.r.t. fitting parameters (done by FD by default; can be
 /// overloaded)
 virtual double fitting_function(const double& x,
                                 Vector<double>& dfit_dparam);
                   
 /// \short Number of parameters in fitting function. Pure virtual, must be
 /// implemented.
 virtual unsigned nparameter()=0;

 /// Access to i-th fitting parameter
 double& parameter(const unsigned& i)
  {
   return Parameter[i];
  }

 /// Access to vector of fitting parameters
 Vector<double>& parameter()
  {
   return Parameter;
  }


protected:

 /// Vector of fitting parameters
 Vector<double> Parameter;

};





//=====================================================================
/// Damped oscillatory function whose parameters can be
/// fitted with Levenberg Marquardt.
//=====================================================================
class DampedOscillatoryFittingFunctionObject : 
 virtual public LevenbergMarquardtFittingFunctionObject
{
 
public:
 
 /// Constructor: Number of fitting parameters is five. 
 DampedOscillatoryFittingFunctionObject() :  
  LevenbergMarquardtFittingFunctionObject(5)
  {}


 /// \short Evaluate the fitting function for the current set
 /// of parameters
 double fitting_function(const double& x)
  {
   return Parameter[0]+
    exp(Parameter[1]*x)*Parameter[2]*sin(Parameter[3]*x+Parameter[4]);
  }
                                 
 /// \short Overload all interfaces of the fitting function, call the default
 /// finite difference version
 double fitting_function(const double &x,
                         Vector<double> &dfit_dparam)
  {
   return 
    LevenbergMarquardtFittingFunctionObject::fitting_function(x,dfit_dparam);
  }

 /// Number of parameters in fitting function
 virtual unsigned nparameter()
  {
   return 5;
  }


};




// forward declaration
class DenseDoubleMatrix;

//=====================================================================
/// Class that allows fitting of free parameters in function
/// (represented by a LevenbergMarquardtFittingFunctionObject)
/// to given (x,y) data.
//=====================================================================
class LevenbergMarquardtFitter
{
 
public:

 /// Empty constructor 
 LevenbergMarquardtFitter(): Fitting_function_object_pt(0)
  {}
 
 /// \short Access to pointer to LevenbergMarquardtFittingFunctionObject
 /// whose parameters we want to determine by fit to data.
 LevenbergMarquardtFittingFunctionObject*& fitting_function_object_pt()
  {
   return Fitting_function_object_pt;
  }

 /// \short Fit the parameters to the pairs of (x,y) data specified, 
 /// using max_iter Levenberg Marquardt iterations
 void fit_it(const Vector<std::pair<double,double> >& fitting_data,
             const unsigned& max_iter,
             const bool& quiet=true);

private:

 /// Pointer to LevenbergMarquardtFittingFunctionObject
 LevenbergMarquardtFittingFunctionObject* Fitting_function_object_pt;


 /// Private helper function -- don't look into it...
 void mrqcof(Vector<double>& x, 
             Vector<double>& y, 
             Vector<double>& sig, 
             Vector<double>& a,
             std::vector<bool>& ia, 
             DenseDoubleMatrix& alpha, 
             Vector<double>& beta, 
             double& chisq);

 
};



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////



//======================================================================
// Namespace for black-box FD Newton solver.
//======================================================================
namespace BlackBoxFDNewtonSolver
{
// Function pointer for function that specifies residuals: The arguments 
// are: Parameters, unknowns, residuals
typedef void (*ResidualFctPt)(const Vector<double>& parameters,
                              const Vector<double>& unknowns,
                              Vector<double>& residuals);

// Maximum number of Newton iterations
extern unsigned Max_iter;

// Flag to indicate if progress of Newton iteration is to be documented
// (defaults to false) 
 extern bool Doc_Progress;

// Size of increment used in finite-difference calculations 
extern double FD_step;

// Tolerance (maximum allowed value of an single residual at convergence) 
extern double Tol;

// Black-box FD Newton solver:
// Calling sequence for residual function is
// \code residual_fct(parameters,unknowns,residuals) \endcode
// where all arguments are double Vectors.
// unknowns.size() = residuals.size() 
extern void black_box_fd_newton_solve(ResidualFctPt residual_fct,
                                      const Vector<double>& params, 
                                      Vector<double>& unknowns);
}


//====================================================================
/// \short Information for documentation of results: 
/// Directory and file number to enable output
/// in the form RESLT/filename11.dat, say.
/// Documentation can be switched on and off.
//====================================================================
class DocInfo
{
 
public:

 /// \short Constructor. Default settings: Current directory, step `0',
 /// label="", full documentation enabled and output directory is not required
 /// to exist when set_directory() is called.
 DocInfo()
  {
   Directory=".";
   Number=0;
   Label="";
   Doc_flag=true;
   Directory_must_exist=false;
  }


 /// Output directory
 std::string directory() const {return Directory;}

 /// \short Set output directory (we try to open a file in it
 /// to see if the directory exists -- if it doesn't we'll
 /// issue a warning -- or, if directory_must_exist()==true,
 /// throw and OomphLibError
 void set_directory(const std::string& directory);

 /// Doc or no doc?
 bool& doc_flag() {return Doc_flag;}

 /// Number used (e.g.) for labeling output files
 unsigned& number() {return Number;}


 /// String used (e.g.) for labeling output files
 std::string& label() {return Label;}

 /// \short What do to do if an output directory doesn't exist:
 /// If true, we terminate code execution by throwing an OomphLibError 
 /// rather than  just issuing a warning. Default: false.
 bool& directory_must_exist()
  {return Directory_must_exist;}
 
private:

 /// Directory name
 std::string Directory;

 /// Doc or don't?
 bool Doc_flag;

 /// Number to label output file, say
 unsigned Number;

 /// String to label output file, say
 std::string Label;
               
 
 /// Boolean flag to decide response if an output 
 /// directory doesn't exist: If true, we terminate
 /// code execution by throwing an OomphLibError rather than 
 /// just issuing a warning.
 bool Directory_must_exist;
};


//====================================================================
// Namespace for command line arguments
//====================================================================
namespace CommandLineArgs
{

 // Number of arguments + 1
 extern int Argc;

 // Arguments themselves
 extern char** Argv;

 // Set values
 extern void setup(int argc, char** argv);

 // Doc the command line arguments
 extern void output();

}

// forward declaration of OomphCommunicator class
class OomphCommunicator;

#ifdef OOMPH_HAS_MPI
//========================================================================
/// MPI output modifier: Preceeds every output by 
/// specification of the processor ID. Output can be restricted
/// to a single processor.
//========================================================================
class MPIOutputModifier : public OutputModifier
{

private:

 /// \short Rank of single processor that produces output (only used
 /// if  Output_from_single_processor=true
 unsigned Output_rank; 

 /// Boolean to control if output is performed only on a single
 /// processor (default: false)
 bool Output_from_single_processor;

 /// Communicator
 OomphCommunicator* Communicator_pt;

public:


 /// Constructor -- initialise flags for output from all processors
 MPIOutputModifier() : Output_rank(0), 
  Output_from_single_processor(false)
  {}

 OomphCommunicator*& communicator_pt() { return Communicator_pt; }

 /// Precede the output by the processor ID but output everything
 virtual bool operator()(std::ostream &stream);

 /// Switch to ensure output is only produced from a single
 /// processor (default: Master node, i.e. rank 0)
 void restrict_output_to_single_processor(const unsigned& output_rank=0)
  {
   Output_from_single_processor=true;
   Output_rank=output_rank;
  }


 /// Switch to ensure output is only produced from a single
 /// processor (default: Master node, i.e. rank 0)
 void allow_output_from_all_processors()
  {
   Output_from_single_processor=false;
  }


};


//========================================================================
/// Single (global) instantiation of the mpi output modifier
//========================================================================
extern MPIOutputModifier oomph_mpi_output;

//=== Forward DenseMatrix class
template <class T>
class DenseMatrix;

// forward declaration of oomph-communicator class
//class OomphCommunicator;

#endif



//======================================================================
/// Basic namespace for MPI helper data and functions; this
/// just basic version, containing default assignments for
/// My_rank and Nproc (simulating the run a single processor
/// which is appropriate for the serial execution).
/// This namespace is extended by mpi_helpers.h in mpi/mpi_src/mpi_generic
//=====================================================================
namespace MPI_Helpers
{
 /// Processor rank
 extern int My_rank;
 
 /// Total number of processors
 extern int Nproc;

 /// Bool set to true if MPI has been initialised
 extern bool MPI_has_been_initialised;

#ifdef OOMPH_HAS_MPI

 /// the global communicator
 extern OomphCommunicator* Communicator_pt;

 /// initialise mpi
 void init(int argc, char **argv);

 /// finalize mpi
 void finalize();

 /// LEGACY - keep this for now?
 /// Setup MPI helpers
 void setup();
#endif
}


//====================================================================
// Namespace for flagging up obsolete parts of the code
//====================================================================
namespace ObsoleteCode
{
 
 // Flag up obsolete parts of the code
 extern bool FlagObsoleteCode;

 // Output warning message
 extern void obsolete();

 extern void obsolete(const std::string &message);

}

//====================================================================
// Namespace for tecplot stuff
//====================================================================
namespace TecplotNames
{

 // Tecplot colours 
 extern Vector<std::string> colour;

 // Setup tecplot colours namespace
 extern void setup();

}


#ifdef LEAK_CHECK

//====================================================================
// Namespace for leak check: Keep a running count of all instantiated
// objects -- add your own if you want to...
//====================================================================
namespace LeakCheckNames
{

 extern long QuadTree_build;
 extern long OcTree_build;
 extern long QuadTreeForest_build;
 extern long OcTreeForest_build;
 extern long RefineableQElement<2>_build;
 extern long RefineableQElement<3>_build;
 extern long MacroElement_build;
 extern long HangInfo_build;
 extern long Node_build;
 extern long GeomReference_build;
 extern long AlgebraicNodeNodeUpdateInfo_build;
 extern long AlgebraicNode_build;

 // Reset counters
 extern void reset();


 // Doc counters
 extern void doc();
}

#endif

//====================================================================
// Namespace for pause() command
//====================================================================
namespace PauseFlags
{
 
 // Flag to enable pausing code
 extern bool PauseFlag;

}

/// Pause and dump out message
void pause(std::string message);

/// Doc memory usage (in % of available memory) -- write to file 
void doc_memory_usage();


/// Initialise doc memory usage (in % of available memory)
void init_doc_memory_usage();


//=============================================================================
/// Helper for recordning execution time.
//=============================================================================
namespace TimingHelpers
{

 /// returns the time in seconds after some point in past
 double timer();

}//end of namespace TimingHelpers
}
#endif
