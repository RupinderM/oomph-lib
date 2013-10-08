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
//Header file for classes that define element objects

//Include guard to prevent multiple inclusions of the header
#ifndef OOMPH_DISPLACEMENT_CONTROL_ELEMENT_HEADER
#define OOMPH_DISPLACEMENT_CONTROL_ELEMENT_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
  #include <oomph-lib-config.h>
#endif

#include <deque>

//oomph-lib includes
#include "elements.h"

namespace oomph
{

//======================================================================
/// \short Displacement control element: In the "normal" formulation
/// of solid mechanics problems, the external load is given and the 
/// displacement throughout the solid body is computed. 
/// For highly nonlinear problems it is sometimes helpful to 
/// re-formulate the problem by prescribing the position of a 
/// selected control point and treating the (scalar) load level 
/// required to achieve this deformation as an unknown. 
/// As an example consider the buckling of pressure-loaded,
/// thin-walled elastic shells.
/// The load-displacement characteristics of such structures tend to be highly
/// nonlinear and bifurcations from the structure's pre-buckling state
/// often occur via sub-critical bifurcations. If we have some
/// a-priori knowledge of the expected deformation (for example, during the
/// non-axisymmetric buckling of a circular cylindrical shell certain 
/// material points will be displaced radially inwards), it 
/// is advantageous to prescribe the radial displacement of a carefully
/// selected control point and treat the external pressure as an unknown. 
/// 
/// \c DisplacementControlElements facilitate the use of
/// such methods. They require the specification of
/// - the control point at which the displacement is prescribed. This is
///   done by specifying:
///   - a pointer, \c controlled_element_pt, to a \c SolidFiniteElement and
///   - the vector \c controlled_point which contains the local 
///     coordinates of the control point in that \c SolidFiniteElement. 
///   .
/// - the coordinate direction. \c controlled_direction. in which 
///   the displacement is controlled.
/// - a pointer to a double, \c  control_position_value_pt, 
///   that specifies the desired value of the prescribed
///   coordinate after the deformation (i.e. if \c controlled_direction=1
///   then \c *control_position_value_pt specifies 
///   the \f$ x_1 \f$ coordinate of the control point in the deformed 
///   configuration.)
/// .
/// The \c DisplacementControlElement has two constructors:
/// - In the first version, we pass the pointer to the \c Data object whose
///   one-and-only value contains the scalar load level that is "traded"
///   for the displacement constraint. This is appropriate if the
///   load \c Data has already been created (and is included in the
///   overall equation numbering procedure) by some other element. In that
///   case the  \c DisplacementControlElement treats the (already existing)
///   \c Data object external \c Data. 
/// - In the second version, a \c Data object (with a single value)
///   is created by the constructor of the \c DisplacementControlElement
///   and stored in its internal \c Data. Once the  
///   \c DisplacementControlElement has been included in one of the
///   \c Problem's meshes, it is therefore automatically included 
///   in the equation numbering procedure. The (pointer to) the 
///   newly created \c Data is accessible via the access function
///   \c displacement_control_load_pt(). It can be used to make
///   make the unknown load level accessible to the load function
///   that drives the deformation.
/// . 
/// \b Note: The element inherits from the BlockPreconditionableElementBase 
/// and can be used in the block-preconditioning context. The element
/// is "in charge" of the control load (if it's been created internally)
/// and classifies it as its one-and-only "block type" 
//======================================================================
class DisplacementControlElement :  
 public virtual GeneralisedElement
{

public:

 /// \short Constructor. Pass: 
 /// - Pointer to \c SolidFiniteElement that contains the control point
 /// - Vector that contains the local coordinates of the control point
 ///   in that element.
 /// - the coordinate direction in which the position of the control
 ///   point is prescribed
 /// - pointer to double that specifies the prescribed coordinate
 ///   of the control point
 /// - Pointer to Data item whose one-and-only  value
 ///   contains the load value that is being adjusted 
 ///   to allow displacement control.
 /// .
 /// The load \c Data is treated as external \c Data for this
 /// element. 
 DisplacementControlElement(SolidFiniteElement* controlled_element_pt,
                            const Vector<double>& controlled_point,
                            const unsigned& controlled_direction,
                            double* control_position_value_pt,
                            Data* displacement_control_load_pt
                            ) :
  Displacement_control_load_pt(displacement_control_load_pt),
  Control_position_value_pt(control_position_value_pt),
  Controlled_direction(controlled_direction),
  Controlled_element_pt(controlled_element_pt),
  Controlled_point(controlled_point)
  {
#ifdef PARANOID
   if (displacement_control_load_pt->nvalue()!=1)
    {
     throw OomphLibError(
      "Displacement control data must only contain a single value!\n",
      OOMPH_CURRENT_FUNCTION,
      OOMPH_EXCEPTION_LOCATION);
    }
#endif 

   // The displacement control load is external Data for this element;
   // add it to the container and and store its index in that container.
   Load_data_index=add_external_data(Displacement_control_load_pt);
   
   // Store flag
   Load_data_created_internally=false;

   // The positional degrees of freedom of all the nodes in the 
   // controlled SolidFiniteElement are external Data
   // for this element
   unsigned nnode = Controlled_element_pt->nnode();
   for (unsigned j=0;j<nnode;j++)
    {
     add_external_data(static_cast<SolidNode*>(
      Controlled_element_pt->node_pt(j))->variable_position_pt());
    }


  }

 /// \short Constructor. Pass: 
 /// - Pointer to \c SolidFiniteElement that contains the control point
 /// - Vector that contains the local coordinates of the control point
 ///   in that element.
 /// - the coordinate direction in which the position of the control
 ///   point is prescribed
 /// - pointer to double that specifies the prescribed coordinate
 ///   of the control point
 /// .
 /// The pointer to a Data item whose one-and-only  value
 /// contains the load value that is being adjusted 
 /// to allow displacement control is created internally (and stored
 /// in the element's internal \c Data. It is accessible (for use
 /// the load function) via the access function  
 /// \c displacement_control_load_pt()
 DisplacementControlElement(SolidFiniteElement* controlled_element_pt,
                            const Vector<double>& controlled_point,
                            const unsigned& controlled_direction,
                            double* control_position_value_pt) :
  Control_position_value_pt(control_position_value_pt),
  Controlled_direction(controlled_direction),
  Controlled_element_pt(controlled_element_pt),
  Controlled_point(controlled_point)
  {
   // Create displacement control load internally (below, we'll store it
   // in the element's internal data so it'll be killed automatically --
   // no need for a destructor)
   Displacement_control_load_pt = new Data(1);

   // The displacement control load is internal Data for this element
   Load_data_index = add_internal_data(Displacement_control_load_pt);

   // Store flag
   Load_data_created_internally=true;

   // The positional degrees of freedom of all the nodes in the 
   // controlled SolidFiniteElement are external Data
   // for this element
   unsigned nnode=Controlled_element_pt->nnode();
   for (unsigned j=0;j<nnode;j++)
    {
     add_external_data(static_cast<SolidNode*>(
      Controlled_element_pt->node_pt(j))->variable_position_pt());
    }
  }

 /// Broken copy constructor
 DisplacementControlElement(const DisplacementControlElement&) 
  { 
   BrokenCopy::broken_copy("DisplacementControlElement");
  } 
 

 /// Broken assignment operator
 void operator=(const DisplacementControlElement&) 
  {
   BrokenCopy::broken_assign("DisplacementControlElement");
  }


 /// \short Pointer to Data object whose one-and-only value represents the
 /// load that is adjusted to allow displacement control
 Data* displacement_control_load_pt() const
  {return Displacement_control_load_pt;}


 /// Store local equation number of displacement control equation
 void assign_additional_local_eqn_numbers()
  {
   if (Load_data_created_internally)
    {
     // Local equation number is the local equation number of the
     // one and only (i.e. the zero-th value stored in the
     // load data (which is stored in the internal data)
     Displ_ctrl_local_eqn = internal_local_eqn(Load_data_index,0);
    }
   else
    {
     // Local equation number is the local equation number of the
     // one and only (i.e. the zero-th value stored in the
     // load data (which is stored in the external data)
     Displ_ctrl_local_eqn = external_local_eqn(Load_data_index,0);
    }
  }


 /// \short Add the element's contribution to its residual vector:
 /// The displacement constraint. [Note: Jacobian is computed 
 /// automatically by finite-differencing]
 void fill_in_contribution_to_residuals(Vector<double> &residuals)
  {
   if (Displ_ctrl_local_eqn>=0)
    {
     residuals[Displ_ctrl_local_eqn]=Controlled_element_pt->
      interpolated_x(Controlled_point,Controlled_direction)-
      *Control_position_value_pt;
    }
  }

 /// \short The number of "blocks" that degrees of freedom in this element
 /// are sub-divided into: Just the control pressure.
 unsigned ndof_types()
  {
   return 1;
  }
 
 /// \short Create a list of pairs for all unknowns in this element,
 /// so that the first entry in each pair contains the global equation
 /// number of the unknown, while the second one contains the number
 /// of the "block" that this unknown is associated with.
 /// (Function can obviously only be called if the equation numbering
 /// scheme has been set up.) The only dof this element is in charge
 /// of is the control load, provided it's been created as
 /// internal Data. 
 void get_dof_numbers_for_unknowns(
  std::list<std::pair<unsigned long,unsigned> >& block_lookup_list)
  {
   if (Load_data_created_internally)
    {
     // temporary pair (used to store block lookup prior to being 
     // added to list)
     std::pair<unsigned long,unsigned> block_lookup;
 
     // determine local eqn number for displacement control eqn
     int local_eqn_number = Displ_ctrl_local_eqn;

     // Is it a dof or is it pinned?
     if (local_eqn_number>=0)
      {
       // store block lookup in temporary pair: First entry in pair
       // is global equation number; second entry is block type
       block_lookup.first = this->eqn_number(local_eqn_number);
       block_lookup.second = 0;

       // Add to list
       block_lookup_list.push_front(block_lookup);
      }
    }
  }
 
protected:

 /// \short Pointer to Data item whose one-and-only  value
 /// contains the load value that is being adjusted 
 /// to allow displacement control.
 Data* Displacement_control_load_pt;

 /// \short Pointer to the value that stores the prescribed coordinate 
 /// of the control point
 double* Control_position_value_pt;

 /// \short Coordinate direction in which the displacement of the 
 /// control point is controlled
 unsigned Controlled_direction;

 /// Pointer to SolidFiniteElement at which control displacement is applied
 SolidFiniteElement* Controlled_element_pt;

 /// \short Vector of local coordinates of point at which control displacement 
 /// is applied
 Vector<double> Controlled_point;

 /// \short Flag to indicate if load data was created internally or externally 
 /// (and is therefore stored in the element's internal or external Data)
 bool Load_data_created_internally;

 /// \short In which component (in the vector of the element's internal or 
 /// external Data) is the load stored?
 unsigned Load_data_index;

 /// Local equation number of the control-displacement equation
 int Displ_ctrl_local_eqn;

};

}

#endif








