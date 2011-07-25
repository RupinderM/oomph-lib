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
//Header file for (one-dimensional) free surface elements
//Include guards, to prevent multiple includes
#ifndef FIXED_VOLUME_ELASTIC_LINE_INTERFACE_ELEMENTS_HEADER
#define FIXED_VOLUME_ELASTIC_LINE_INTERFACE_ELEMENTS_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

//OOMPH-LIB headers
#include "generic/Qelements.h"
#include "generic/spines.h"
#include "fluid_interface/line_interface_elements.h"

using namespace std;

namespace oomph
{

//==========================================================================
/// A class that is used to implement the constraint that the fluid volume
/// must take a specific value. This is required in steady interface
/// problems, where, in general, a family of solutions may exist.
/// Note that as written, the function assumes that the mesh contains
/// three-straight boundaries at (x(0) = 0, x(1) = 0, x[0] = L) and
/// one free surface. In order to implement the constraint in total generality,
/// one would need to introduce surface elements all round the mesh
/// and integrate x.n along them. This is not necessary in the special case
/// described above, because the integrals along the three straight edges
/// are easily evaluated. The first two are zero and the third is
/// included in the residuals calculated in this element.
//=========================================================================
template<class ELEMENT>
class ElasticVolumeConstraintPointElement : 
 public virtual ElasticPointFluidInterfaceBoundingElement<ELEMENT>
 {
  private:

 /// Pointer to the desired value of the volume
 double *Volume_pt;
 
 /// \short Pointer to the Data item that stores the pressure that has been
 /// "traded" for the volume constraint in its single value.
 Data* Ptraded_data_pt;
 
 /// \short The Data that contains the traded pressure is stored
 /// as external Data for the element. Which external Data item is it?
 unsigned External_data_number_of_traded_pressure;

 /// \short The local eqn number for the traded pressure, which is 
 /// the variable that corresponds to this equation
 inline int ptraded_local_eqn()
  {
   if(Ptraded_data_pt==0) {return -1;}
   else 
    {return 
      this->external_local_eqn(External_data_number_of_traded_pressure,0);}
  }

  public:

 ///Constructor
 ElasticVolumeConstraintPointElement() : 
  ElasticPointFluidInterfaceBoundingElement<ELEMENT>()
  {
   oomph_info << "hierher: Replace/move to "
              << "src/fluid_interface/constrained_volume_elements.h \n";


   //Initialise the stored values to zero
   Volume_pt = 0;
   Ptraded_data_pt = 0;
  } 

 /// \short Access function used to set the volume
 double* &volume_pt() {return Volume_pt;}

 ///Custom overload the additional volume constraint
 void add_additional_residual_contributions_interface_boundary(
   Vector<double> &residuals, 
   DenseMatrix<double> &jacobian,
   const unsigned &flag,
   const Shape &psif,
   const DShape &dpsifds,
   const Vector<double> &interpolated_n, 
   const double &W)
 // hierher
/* add_additional_residual_contributions( */
/*   Vector<double> &residuals, DenseMatrix<double> &jacobian, */
/*   const unsigned &flag)  */
  {
   //If we have an external pressure, add the final term
   //to the volumetric constraint equation
   int local_eqn = ptraded_local_eqn();
   if(local_eqn >= 0)
    {
     //The integral of x.n on the RHS of the boundary is just x(0)*x(1)
     //of the top right-hand corner, we divide by two because we are working
     //in two dimensions and then subtract the desired volume
     residuals[local_eqn] = 
      0.5*this->node_pt(0)->x(0)*this->node_pt(0)->x(1) - *Volume_pt;
    }
  }

 /// \short Set the Data that contains the single pressure value
 /// that is "traded" for the volume constraint.
 /// The Data item must only contain a single value!
 void set_traded_pressure_data(Data* traded_pressure_data_pt)
  {
#ifdef PARANOID
   if (traded_pressure_data_pt->nvalue()!=1)
    {
     std::ostringstream error_stream;
     error_stream 
      << "The `traded` pressure Data must only contain a single value\n"
      << "This one contains " << traded_pressure_data_pt->nvalue() 
      << std::endl;
     
     std::string function_name = 
      "ElasticVolumConstraintPointElement::\n";
     function_name += "set_traded_pressure_data()";
     
     throw OomphLibError(error_stream.str(),function_name,
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
     // Store pointer explicitly
     Ptraded_data_pt=traded_pressure_data_pt;
     // Add to the element's external data so it gets included
     // in the black-box local equation numbering scheme
     External_data_number_of_traded_pressure = 
      this->add_external_data(traded_pressure_data_pt);
  }

}; 


//=======================================================================
/// One-dimensional interface elements that are used when the mesh
/// deformation is handled by a set of equations that modify the nodal
/// positions. These elements are FaceElements of bulkFluid elements and
/// the fluid element is passed as a template parameter to the element.
/// Optionally an external pressure may be specified, which must be
/// passed to the element as external data. The default value of the external
/// pressure is zero.
//=======================================================================
template<class ELEMENT>
class FixedVolumeElasticLineFluidInterfaceElement : 
public ElasticLineFluidInterfaceElement<ELEMENT>
{
  private:

 /// \short The Data that contains the traded pressure is stored
 /// as external Data for the element. Which external Data item is it?
 unsigned External_data_number_of_traded_pressure;

 /// \short Pointer to the Data item that stores the pressure that has been
 /// "traded" for the volume constraint in its single value.
 Data* Ptraded_data_pt;

  /// \short The local eqn number for the traded pressure, which is 
 /// the variable that corresponds to this equation
 inline int ptraded_local_eqn()
  {
   if(Ptraded_data_pt==0) {return -1;}
   else 
    {return this->external_local_eqn(
     External_data_number_of_traded_pressure,0);}
  }


 /// \short Overload the Helper function to calculate the residuals and 
 /// and jacobian entries. This particular function ensures that the
 /// additional entries are calculated inside the integration loop
 void add_additional_residual_contributions_interface(
  Vector<double> &residuals, 
  DenseMatrix<double> &jacobian,
  const unsigned &flag,
  const Shape &psif,
  const DShape &dpsifds,
  const Vector<double> &interpolated_x, 
  const Vector<double> &interpolated_n, 
  const double &W, 
  const double &J)
 // hierher
/*   Vector<double> &residuals, DenseMatrix<double> &jacobian, */
/*   const unsigned &flag, */
/*   const Shape &psif, const DShape &dpsifds, */
/*   const Vector<double> &interpolated_n, const double &W, */
/*   const double &J) */
  {
   //Call the underlying additional residual terms
   ElasticLineFluidInterfaceElement<ELEMENT>::
    add_additional_residual_contributions_interface(
     residuals,jacobian,flag,psif,
     dpsifds,interpolated_x,
     interpolated_n,W,J);
   
   //Add in the volume constraint term if required -- HACKY!
   //The volume of the fluid is found by integrating x.n around the boundary
   int local_eqn= ptraded_local_eqn(); 
   if(local_eqn >=0)
    {
     //Find out how many nodes there are
     unsigned n_node = this->nnode();
     
     //Find the x position
     Vector<double> interpolated_x(2,0.0);
     //Fill in the x position
     for(unsigned l=0;l<n_node;l++)
      {
       const double psi_local = psif(l);
       for(unsigned i=0;i<2;i++)
        {
         interpolated_x[i] += this->nodal_position(l,i)*psi_local;
        }
      }
     
     //Find the dot product
     double dot = 0.0;
     for(unsigned k=0;k<2;k++) {dot += interpolated_x[k]*interpolated_n[k];}
     residuals[local_eqn] += 0.5*dot*W*J;
    }
  }

 
  public:
 /// Constructor, overload
 FixedVolumeElasticLineFluidInterfaceElement(
  FiniteElement* const &element_pt, const int &face_index) :
  ElasticLineFluidInterfaceElement<ELEMENT>(
   element_pt,face_index)
  {


   oomph_info << "hierher: Replace/move to "
              << "src/fluid_interface/constrained_volume_elements.h \n";


   //Initialise pointer to pressure data that has been traded
   Ptraded_data_pt = 0;
  }
   
 /// \short Set the Data that contains the single pressure value
 /// that is "traded" for the volume constraint.
 /// The Data item must only contain a single value!
 void set_traded_pressure_data(Data* traded_pressure_data_pt)
  {
#ifdef PARANOID
   if (traded_pressure_data_pt->nvalue()!=1)
    {
     std::ostringstream error_stream;
     error_stream 
      << "The `traded` pressure Data must only contain a single value\n"
      << "This one contains " << traded_pressure_data_pt->nvalue() 
      << std::endl;
     
     std::string function_name = 
      "FixedVolumeElasticLineFluidInterfaceElement::\n";
     function_name += "set_traded_pressure_data()";
     
     throw OomphLibError(error_stream.str(),function_name,
                         OOMPH_EXCEPTION_LOCATION);
    }
#endif
   // Store pointer explicitly
   Ptraded_data_pt=traded_pressure_data_pt;
   //Add the data to the external scheme so that it is numbered
   External_data_number_of_traded_pressure =
    this->add_external_data(traded_pressure_data_pt);
  }

 ///\short Overload the making of the edge element to create out
 ///volume constraint edge element.
 FluidInterfaceBoundingElement* make_bounding_element(const int &face_index)
  {
   //Create a temporary pointer to the appropriate FaceElement
   ElasticVolumeConstraintPointElement<ELEMENT> *Temp_pt = 
    new ElasticVolumeConstraintPointElement<ELEMENT>;

   //Attach the geometrical information to the new element
   this->build_face_element(face_index,Temp_pt);
 
   //Set the index at which the unknowns are stored from the element
   Temp_pt->u_index_interface_boundary() = this->U_index_interface;

   //Set the value of the nbulk_value, the node is not resized
   //in this problem, so it will just be the actual nvalue - 1
   Temp_pt->nbulk_value(0) = Temp_pt->node_pt(0)->nvalue() -1;
   
   //Find the nodes
   std::set<SolidNode*> set_of_solid_nodes;
   unsigned n_node = this->nnode();
   for(unsigned n=0;n<n_node;n++)
    {
     set_of_solid_nodes.insert(static_cast<SolidNode*>(this->node_pt(n)));
    }

   //Delete the nodes from the face
   n_node = Temp_pt->nnode();
   for(unsigned n=0;n<n_node;n++)
    {
     set_of_solid_nodes.erase(static_cast<SolidNode*>(Temp_pt->node_pt(n)));
    }

   //Now add these as external data
   for(std::set<SolidNode*>::iterator it=set_of_solid_nodes.begin();
       it!=set_of_solid_nodes.end();++it)
    {
     Temp_pt->add_external_data((*it)->variable_position_pt());
    }
           
   //Return the value of the pointer
   return Temp_pt;
  }

};

}

#endif






