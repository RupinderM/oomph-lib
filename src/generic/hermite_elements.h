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
//Header functions for classes that define Hermite elements

//Include guards to prevent multiple inclusions of the header
#ifndef OOMPH_HERMITE_ELEMENT_HEADER
#define OOMPH_HERMITE_ELEMENT_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

//oomph-lib headers
#include "Vector.h"
#include "shape.h"
#include "integral.h"
#include "elements.h"
#include "Qelements.h"


namespace oomph
{

//========================================================================
/// Empty base class for QHermiteElements (created so that 
/// we can use dynamic_cast<>() to figure out if a an element
/// is a QHermiteElement).
//========================================================================
 class QHermiteElementBase : public virtual QElementGeometricBase
{

  public:

 /// Empty default constructor
 QHermiteElementBase(){} 

 /// Broken copy constructor
 QHermiteElementBase(const QHermiteElementBase&) 
  { 
   BrokenCopy::broken_copy("QHermiteElementBase");
  } 
 
 /// Broken assignment operator
 void operator=(const QHermiteElementBase&) 
  {
   BrokenCopy::broken_assign("QHermiteElementBase");
  }
};


//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////


//=======================================================================
/// General QHermiteElement class. Local coordinates are not assumed
/// to be aligned with the global coordinates so the Jacobian
/// of the mapping between local and global coordinates is
/// a full matrix. For cases where the coordinates are aligned,
/// you should use the derived class, DiagQHermiteElement, which
/// uses a simplified mapping that  makes the evaluation of 
/// derivatives of the shape functions much cheaper.
//=======================================================================
template<unsigned DIM>
class QHermiteElement : public virtual QHermiteElementBase
{
  private:

 /// \short Default integration rule: Gaussian integration of same 'order' 
 /// as the element
 // This is sort of optimal, because it means that the integration is exact
 // for the shape functions. Can overwrite this in specific element definition.
 //static Gauss_Rescaled<DIM,3> Default_integration_scheme;
 static Gauss<DIM,3> Default_integration_scheme;

public:

 /// Constructor
 QHermiteElement() 
  {
   //Calculate the number of nodes
   unsigned n_node = 
    static_cast<unsigned>(pow(2.0,static_cast<int>(DIM)));
   //Set the number of nodes
   this->set_n_node(n_node);
   //Set the elemental and nodal dimensions
   this->set_dimension(DIM);
   //Set the number of interpolated position types (always n_node)
   this->set_nnodal_position_type(n_node);
   //Assign pointer to default integration scheme
   this->set_integration_scheme(&Default_integration_scheme);
  }


 /// Broken copy constructor
 QHermiteElement(const QHermiteElement& dummy) 
  { 
   BrokenCopy::broken_copy("QHermiteElement");
  } 
 
 /// Broken assignment operator
 void operator=(const QHermiteElement&) 
  {
   BrokenCopy::broken_assign("QHermiteElement");
  }
 

 ///Check whether the local coordinate are valid or not 
 bool local_coord_is_valid(const Vector<double> &s) 
 {
  unsigned ncoord = dim();
  for(unsigned i=0;i<ncoord;i++)
   {
    // We're outside 
    if((s[i] - s_max() >  0.0) ||
       (s_min() - s[i] >  0.0))
     {
      return false;
     }
   }
  return true;
 }
 
 /// \short Check whether the local coordinate are valid or not, allowing for
 /// rounding tolerance. Nodal coordinate is adjusted to move the
 /// point back into the element if it's outside the element
 /// to within that tolerance
 bool local_coord_is_valid(Vector<double> &s, 
                           const double & rounding_tolerance)
 {
  unsigned ncoord = dim();
  for(unsigned i=0;i<ncoord;i++)
   {
    // We're outside
    if((s[i] - s_max() >  rounding_tolerance) ||
       (s_min() - s[i] >  rounding_tolerance))
     {
      return false;
     }
    else
     {
      // Adjust to move it onto the boundary
      if (s[i] > s_max() ) s[i] = s_max();
      if (s[i] < s_min() ) s[i] = s_min();
     }
   }
  return true;
 }
 
 /// Function to calculate the geometric shape functions at local coordinate s
 void shape(const Vector<double> &s, Shape &psi) const;

 /// \short Function to compute the  geometric shape functions and 
 /// derivatives w.r.t. local coordinates at local coordinate s
 void dshape_local(const Vector<double> &s, Shape &psi, DShape &dpsids) 
  const;

 /// \short Function to compute the geometric shape functions and 
 /// also first and second derivatives wrt local coordinates at 
 /// local coordinate s.
 ///  Numbering:
 ///  \b 1D: 
 /// d2psids(i,0) = \f$ d^2 \psi_j / d s^2 \f$
 ///  \b 2D: 
 /// d2psids(i,0) = \f$ \partial^2 \psi_j / \partial s_0^2 \f$ 
 /// d2psids(i,1) = \f$ \partial^2 \psi_j / \partial s_1^2 \f$ 
 /// d2psids(i,2) = \f$ \partial^2 \psi_j / \partial s_0 \partial s_1 \f$ 
 ///  \b 3D: 
 /// d2psids(i,0) = \f$ \partial^2 \psi_j / \partial s_0^2 \f$ 
 /// d2psids(i,1) = \f$ \partial^2 \psi_j / \partial s_1^2 \f$ 
 /// d2psids(i,2) = \f$ \partial^2 \psi_j / \partial s_2^2 \f$ 
 /// d2psids(i,3) = \f$ \partial^2 \psi_j / \partial s_0 \partial s_1 \f$ 
 /// d2psids(i,4) = \f$ \partial^2 \psi_j / \partial s_0 \partial s_2 \f$ 
 /// d2psids(i,5) = \f$ \partial^2 \psi_j / \partial s_1 \partial s_2 \f$ 
 void d2shape_local(const Vector<double> &s, Shape &psi, DShape &dpsids,
                    DShape &d2psids) const;


 /// \short Overload the template-free interface for the calculation of
 /// the inverse jacobian. The element dimension must be passed to 
 /// the function
 double invert_jacobian_mapping(const DenseMatrix<double> &jacobian,
                                DenseMatrix<double> &inverse_jacobian) const
  {return invert_jacobian<DIM>(jacobian,inverse_jacobian);}

 /// \short Overload the template-free interface for the calculation of
 /// transformation of second derivatives. The element dimension should be
 /// passed as a template paremeter, for "optimum" efficiency.
 void transform_second_derivatives(const DenseMatrix<double>
                                   &jacobian,
                                   const DenseMatrix<double> 
                                   &inverse_jacobian,
                                   const DenseMatrix<double> 
                                   &jacobian2,
                                   DShape &dbasis,
                                   DShape &d2basis) const
  {
   transform_second_derivatives_template<DIM>(jacobian,inverse_jacobian,
                                              jacobian2,dbasis,d2basis);
  }
                                             
 /// Min. value of local coordinate
 double s_min() const {return -1.0;}

 /// Max. value of local coordinate
 double s_max() const {return 1.0;}

 
 /// Get local coordinates of node j in the element; vector sets its own size
 void local_coordinate_of_node(const unsigned& j, Vector<double>& s)
  {
   s.resize(DIM);
   Vector<unsigned> j_sub(DIM);
   unsigned j_copy = j;
   unsigned NNODE_1D=2;
   const double S_min = this->s_min();
   const double S_range = this->s_max() - S_min;
   for(unsigned i=0;i<DIM;i++)
    {
     j_sub[i] = j_copy%NNODE_1D;
     j_copy = (j_copy - j_sub[i])/NNODE_1D;
     s[i]= S_min + double(j_sub[i])/(double)(NNODE_1D-1)*S_range;
    }
  }

 /// Get local fraction of node j in the element; vector sets its own size
 void local_fraction_of_node(const unsigned& j, Vector<double>& s_fraction)
  {
   s_fraction.resize(DIM);
   Vector<unsigned> j_sub(DIM);
   unsigned j_copy = j;
   unsigned NNODE_1D=2;
   for(unsigned i=0;i<DIM;i++)
    {
     j_sub[i] = j_copy%NNODE_1D;
     j_copy = (j_copy - j_sub[i])/NNODE_1D;
     s_fraction[i]= j_sub[i];
    }
  }

 /// \short Get the local fraction of any node in the n-th position
 /// in a one dimensional expansion along the i-th local coordinate
 double local_one_d_fraction_of_node(const unsigned &n1d, 
                                     const unsigned &i)
  {
   //The spacing is just the node number because there are only two
   //nodes
   return n1d;
  }
  
 /// Return number of nodes along each element edge
 unsigned nnode_1d() const {return 2;}

 /// Output 
 void output(std::ostream &outfile);

 /// Output at n_plot points
 void output(std::ostream &outfile, const unsigned &n_plot);

 /// C-style output 
 void output(FILE* file_pt);

 /// C_style output at n_plot points
 void output(FILE* file_pt, const unsigned &n_plot);

 /// \short  Get cector of local coordinates of plot point i (when plotting 
 /// nplot points in each "coordinate direction).
 void get_s_plot(const unsigned& i, const unsigned& nplot,
                 Vector<double>& s) const;

 /// \short Return string for tecplot zone header (when plotting 
 /// nplot points in each "coordinate direction)
 std::string tecplot_zone_string(const unsigned& nplot);

 /// Return total number of plot points (when plotting 
 /// nplot points in each "coordinate direction)
 unsigned nplot_points(const unsigned& nplot);

 /// \short Build the lower-dimensional FaceElement of the type
 /// QHermiteElement<DIM-1>. The face index takes a value that 
 /// correponds to the possible faces:
 /// 
 /// In 1D:
 /// -1 (Left)  s[0] = -1.0
 /// +1 (Right) s[0] =  1.0
 /// 
 /// In 2D:
 /// -1 (West)  s[0] = -1.0
 /// +1 (East)  s[0] =  1.0
 /// -2 (South) s[1] = -1.0
 /// +2 (North) s[1] =  1.0
 /// 
 /// In 3D:
 /// -1 (Left)   s[0] = -1.0
 /// +1 (Right)  s[0] =  1.0
 /// -2 (Down)   s[1] = -1.0
 /// +2 (Up)     s[1] =  1.0
 /// -3 (Back)   s[2] = -1.0
 /// +3 (Front)  s[2] =  1.0
 void build_face_element(const int &face_index,
                         FaceElement* face_element_pt);

};

//Inline functions:
//=======================================================================
/// Get cector of local coordinates of plot point i (when plotting nplot 
/// points in each coordinate direction).
//=======================================================================
template<> 
inline void QHermiteElement<1>::get_s_plot(const unsigned& i, const 
                                           unsigned& nplot,
                                           Vector<double>& s) const
 {
  if (nplot>1)
   {
    s[0]=-1.0+2.0*double(i)/double(nplot-1);
   }
  else
   {
    s[0]=0.0;
   }
 }
 
//=======================================================================
/// Return string for tecplot zone header (when plotting nplot points in 
/// each coordinate direction)
//=======================================================================
template<> 
inline std::string QHermiteElement<1>::
tecplot_zone_string(const unsigned& nplot)
  {
   std::ostringstream header;
   header << "ZONE I=" << nplot << "\n";
   return header.str();
  }

//========================================================================
/// Return total number of plot points (when plotting nplot points in each
/// coordinate direction)
//========================================================================
template<> 
inline unsigned QHermiteElement<1>::nplot_points(const unsigned& nplot) 
{return nplot;}


//=======================================================================
/// Get cector of local coordinates of plot point i (when plotting nplot 
/// points in each "coordinate direction).
//=======================================================================
template<> 
inline void QHermiteElement<2>::get_s_plot(const unsigned& i, const 
                                           unsigned& nplot,
                                           Vector<double>& s) const
 {
  if (nplot>1)
   {
    unsigned i0=i%nplot;
    unsigned i1=(i-i0)/nplot;
    
    s[0]=-1.0+2.0*double(i0)/double(nplot-1);
    s[1]=-1.0+2.0*double(i1)/double(nplot-1);
   }
  else
   {
    s[0]=0.0;
    s[1]=0.0;
   }
 }

//=======================================================================
/// Return string for tecplot zone header (when plotting nplot points in 
/// each coordinate direction)
//=======================================================================
template<> 
inline std::string QHermiteElement<2>::tecplot_zone_string(
 const unsigned& nplot)
{
 std::ostringstream header;
 header << "ZONE I=" << nplot << ", J=" << nplot << "\n";
 return header.str();
}

//=======================================================================
/// Return total number of plot points (when plotting 
/// nplot points in each coordinate direction)
//=======================================================================
template<> 
inline unsigned QHermiteElement<2>::nplot_points(const unsigned& nplot) 
{return nplot*nplot;}

//=====================================================================
///These elements are exactly the same as QHermiteElements, but they
///employ the simplifying assumption that the local and global
///coordinates are aligned. This makes the evaluation of the
///derivatives of the shape functions much cheaper.
//=====================================================================
template<unsigned DIM>
class DiagQHermiteElement : public virtual QHermiteElement<DIM>
{
  protected:

 /// \short Overload the template-free interface for the calculation of
 /// the inverse jacobian. Pass the dimension of the element to the 
 /// invert_jacobian function.
 double invert_jacobian_mapping(const DenseMatrix<double> &jacobian,
                                DenseMatrix<double> &inverse_jacobian) const
  {return FiniteElement::invert_jacobian<DIM>(jacobian,inverse_jacobian);}

 /// \short Overload the local to eulerian mapping so that it uses diagonal
 /// terms only.
 double local_to_eulerian_mapping(const DShape &dpsids,
                                  DenseMatrix<double> &jacobian,
                                  DenseMatrix<double> &inverse_jacobian) const
  {
   return this->local_to_eulerian_mapping_diagonal(dpsids,jacobian,
                                                   inverse_jacobian);
  }

 /// \short Overload the template-free interface for the transformation
 /// of derivatives, so that the diagonal version is used.
 void transform_derivatives(const DenseMatrix<double>
                            &inverse_jacobian, DShape &dbasis) const
  {
   FiniteElement::
    transform_derivatives_diagonal(inverse_jacobian,dbasis);
  }

 /// \short Overload the template-free interface for the calculation of
 /// transformation of second derivatives.
 void transform_second_derivatives(const DenseMatrix<double>
                                   &jacobian,
                                   const DenseMatrix<double> 
                                   &inverse_jacobian,
                                   const DenseMatrix<double> 
                                   &jacobian2,
                                   DShape &dbasis,
                                   DShape &d2basis) const
  {
   FiniteElement::transform_second_derivatives_diagonal<DIM>(
    jacobian,inverse_jacobian,jacobian2,dbasis,d2basis);
  }


  public:

 /// Constructor 
 DiagQHermiteElement() : QHermiteElement<DIM>() {}

 /// Broken copy constructor
 DiagQHermiteElement(const DiagQHermiteElement& dummy) 
  { 
   BrokenCopy::broken_copy("DiagQHermiteElement");
  } 
 
 /// Broken assignment operator
 void operator=(const DiagQHermiteElement&) 
  {
   BrokenCopy::broken_assign("DiagQHermiteElement");
  }

};

///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


//=======================================================================
/// SolidQHermiteElement elements are Hermite elements whose Jacobian 
/// matrices include derivatives w.r.t. the Eulerian positions 
/// of their nodes. They are the basis for elasticity elements.
/// No assumptions are made about alignment of local and global
/// coordinates.
//=======================================================================
template <unsigned DIM>
class SolidQHermiteElement : public virtual QHermiteElement<DIM>, 
 public virtual SolidFiniteElement 
{
  public:

 /// Constructor
 SolidQHermiteElement() : QHermiteElement<DIM>(), SolidFiniteElement() 
  {
   //Get the number of nodes (alloactaed in the QHermiteElement<DIM> class)
   unsigned n_node = nnode();
   //Set the lagrangian dimension
   this->set_lagrangian_dimension(DIM);
   //Set the number of interpolated lagrangian types (always n_node)
   this->set_nnodal_lagrangian_type(n_node);
  }

 /// Broken copy constructor
 SolidQHermiteElement(const SolidQHermiteElement& dummy) 
  { 
   BrokenCopy::broken_copy("SolidQHermiteElement");
  } 
 
 /// Broken assignment operator
 void operator=(const SolidQHermiteElement&) 
  {
   BrokenCopy::broken_assign("SolidQHermiteElement");
  }

 /// Overload the output function
 void output(std::ostream &outfile);

 /// Output at n_plot points
 void output(std::ostream &outfile, const unsigned &n_plot);

 /// C-style output
 void output(FILE* file_pt);

 /// C_style output at n_plot points
 void output(FILE* file_pt, const unsigned &n_plot);

 /// \short Build the lower-dimensional FaceElement of the type
 /// SolidQHermiteElement<DIM-1>. The face index takes a value that 
 /// correponds to the possible faces:
 /// 
 /// In 1D:
 /// -1 (Left)  s[0] = -1.0
 /// +1 (Right) s[0] =  1.0
 /// 
 /// In 2D:
 /// -1 (West)  s[0] = -1.0
 /// +1 (East)  s[0] =  1.0
 /// -2 (South) s[1] = -1.0
 /// +2 (North) s[1] =  1.0
 /// 
 /// In 3D:
 /// -1 (Left)   s[0] = -1.0
 /// +1 (Right)  s[0] =  1.0
 /// -2 (Down)   s[1] = -1.0
 /// +2 (Up)     s[1] =  1.0
 /// -3 (Back)   s[2] = -1.0
 /// +3 (Front)  s[2] =  1.0
 void build_face_element(const int &face_index,
                         FaceElement* face_element_pt);


};

//============================================================================
/// SolidQHermiteElements in which we assume the local and global
/// coordinates to be aligned so that the Jacobian of the mapping
/// betwteen local and global coordinates is diagonal. This makes
/// the evaluation of the derivatives of the shape functions
/// much cheaper.
//============================================================================
template <unsigned DIM>
class SolidDiagQHermiteElement : 
 public virtual DiagQHermiteElement<DIM>, 
 public virtual SolidQHermiteElement<DIM>
{
  public:

 /// Constructor
 SolidDiagQHermiteElement() : DiagQHermiteElement<DIM>(),
  SolidQHermiteElement<DIM>() {}

 /// Broken copy constructor
 SolidDiagQHermiteElement(const SolidDiagQHermiteElement& dummy) 
  { 
   BrokenCopy::broken_copy("SolidDiagQHermiteElement");
  } 
 
 /// Broken assignment operator
 void operator=(const SolidDiagQHermiteElement&) 
  {
   BrokenCopy::broken_assign("SolidDiagQHermiteElement");
  }

 /// \short Overload the local to lagrangian mapping so that it uses diagonal
 /// terms only.
 double local_to_lagrangian_mapping(const DShape &dpsids,
                                    DenseMatrix<double> &jacobian,
                                    DenseMatrix<double> &inverse_jacobian) const
  {
   return this->local_to_lagrangian_mapping_diagonal(dpsids,jacobian,
                                                     inverse_jacobian);
  }
 
};

}

#endif







