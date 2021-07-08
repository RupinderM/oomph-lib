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
// Header file for a relatively simple Quad Meshe
#ifndef OOMPH_RECTANGULAR_QUADMESH_HEADER
#define OOMPH_RECTANGULAR_QUADMESH_HEADER

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// OOMPH-LIB headers
#include "../generic/mesh.h"
#include "../generic/quad_mesh.h"
#include "../generic/refineable_quad_mesh.h"

namespace oomph
{
  //==========================================================================
  /// RectangularQuadMesh is a two-dimensional mesh of Quad elements with Nx
  /// elements in the "x" (horizonal) direction and Ny elements in the "y"
  /// (vertical) direction. Two Constructors are provided. The basic constructor
  /// assumes that the lower-left-hand corner of the mesh is (0,0) and
  /// takes only the arguments, Nx, Ny, Xmax and Ymax. The more complex
  /// constructor takes the additional arguments Xmin and Ymin.
  ///
  /// This class is designed to be used as a Base class for more complex
  /// two dimensional meshes. The virtual functions x_spacing_function()
  /// and y_spacing_function() may be overloaded to provide arbitrary node
  /// spacing. The default is uniformly spaced nodes in each direction.
  ///
  /// It is also possible to make the solution periodic in the x direction.
  //===========================================================================
  template<class ELEMENT>
  class RectangularQuadMesh : public virtual QuadMeshBase
  {
  protected:
    // Mesh variables
    /// Nx: number of elements in x-direction
    unsigned Nx;
    /// Ny: number of elements in y-direction
    unsigned Ny;
    /// Np: number of (linear) points in the element
    unsigned Np;

    /// Minimum value of x coordinate
    double Xmin;
    /// Maximum value of x coordinate
    double Xmax;

    /// Minimum value of y coordinate
    double Ymin;
    /// Maximum value of y coordinate
    double Ymax;

    /// \short Boolean variable used to determine whether the mesh
    /// is periodic in the x-direction
    bool Xperiodic;

    /// Generic mesh construction function: contains all the hard work
    void build_mesh(TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper);

    /// \short Constructor that allows the specification of minimum and maximum
    /// values of x and y coordinates and does not build the mesh
    /// This is intend to be used in derived classes that overload the
    /// spacing functions. THis is scheduled to be changed, however.
    /// The reason why this MUST be done is because the virtual spacing
    /// functions cannot be called in the base constructur, because they will
    /// not have been overloaded yet!!
    RectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& xmin,
      const double& xmax,
      const double& ymin,
      const double& ymax,
      const bool& periodic_in_x,
      const bool& build,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      Nx(nx),
      Ny(ny),
      Xmin(xmin),
      Xmax(xmax),
      Ymin(ymin),
      Ymax(ymax),
      Xperiodic(periodic_in_x)
    {
      if (build)
      {
        // Call the generic mesh constructor
        build_mesh(time_stepper_pt);
      }
    }

  public:
    /// \short Simple constructor: nx: number of elements in x direction;
    /// ny: number of elements in y direction; lx, length of domain in x
    /// direction (0,lx); ly, length of domain in y direction (0,ly)
    /// Also pass pointer to timestepper (defaults to Steady)
    RectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      Nx(nx), Ny(ny), Xmin(0.0), Xmax(lx), Ymin(0.0), Ymax(ly), Xperiodic(false)
    {
      // Mesh can only be built with 2D Qelements.
      MeshChecker::assert_geometric_element<QElementGeometricBase, ELEMENT>(2);

      // Call the generic mesh constructor
      build_mesh(time_stepper_pt);
    }

    /// \short Constructor that allows the specification of minimum and maximum
    /// values of x and y coordinates
    RectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& xmin,
      const double& xmax,
      const double& ymin,
      const double& ymax,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      Nx(nx),
      Ny(ny),
      Xmin(xmin),
      Xmax(xmax),
      Ymin(ymin),
      Ymax(ymax),
      Xperiodic(false)
    {
      // Mesh can only be built with 2D Qelements.
      MeshChecker::assert_geometric_element<QElementGeometricBase, ELEMENT>(2);

      // Call the generic mesh constructor
      build_mesh(time_stepper_pt);
    }

    /// \short Simple constructor: nx: number of elements in x direction;
    /// ny: number of elements in y direction; lx, length of domain in x
    /// direction (0,lx); ly, length of domain in y direction (0,ly)
    /// Boolean flag specifies if the mesh is periodic in the x-direction.
    /// Also pass pointer to timestepper (defaults to Steady)
    RectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      const bool& periodic_in_x,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      Nx(nx),
      Ny(ny),
      Xmin(0.0),
      Xmax(lx),
      Ymin(0.0),
      Ymax(ly),
      Xperiodic(periodic_in_x)
    {
      // Mesh can only be built with 2D Qelements.
      MeshChecker::assert_geometric_element<QElementGeometricBase, ELEMENT>(2);

      // Call the generic mesh constructor
      build_mesh(time_stepper_pt);
    }

    /// \short Constructor that allows the specification of minimum and maximum
    /// values of x and y coordinates.
    /// Boolean flag specifies if the mesh is periodic in the x-direction.
    RectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& xmin,
      const double& xmax,
      const double& ymin,
      const double& ymax,
      const bool& periodic_in_x,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      Nx(nx),
      Ny(ny),
      Xmin(xmin),
      Xmax(xmax),
      Ymin(ymin),
      Ymax(ymax),
      Xperiodic(periodic_in_x)
    {
      // Mesh can only be built with 2D Qelements.
      MeshChecker::assert_geometric_element<QElementGeometricBase, ELEMENT>(2);

      // Call the generic mesh constructor
      build_mesh(time_stepper_pt);
    }

    /// Return number of elements in x direction
    const unsigned& nx() const
    {
      // Return the value of Nx
      return Nx;
    }

    /// Return number of elements in y direction
    const unsigned& ny() const
    {
      // Return the value of Ny
      return Ny;
    }

    /// Return the minimum value of x coordinate
    const double x_min() const
    {
      // Return the value of Xmin
      return Xmin;
    }

    /// Return the maximum value of x coordinate
    const double x_max() const
    {
      // Return the value of Xmax
      return Xmax;
    }

    /// Return the minimum value of y coordinate
    const double y_min() const
    {
      // Return the value of Ymin
      return Ymin;
    }

    /// Return the maximum value of y coordinate
    const double y_max() const
    {
      // Return the value of Ymax
      return Ymax;
    }

    /// \short Reorder the elements: By default they are ordered
    /// in "horizontal" layers (increasing in x, then in y). This
    /// function changes this to an ordering in the vertical direction
    /// (y first, then x). This is more efficient if a frontal solver
    /// is used and the mesh has more elements in the x than the y direction.
    /// Can be overloaded in specific derived meshes.
    virtual void element_reorder();

    /// \short Return the value of the x-coordinate at the node given by the
    /// local node number (xnode, ynode) in the element (xelement,yelement).
    /// The description is in a "psudeo" two-dimensional coordinate system,
    /// so the range of xelement is [0,Nx-1], yelement is [0,Ny-1], and
    /// that of xnode and ynode is [0,Np-1]. The default is to return
    /// nodes that are equally spaced in the x coodinate.
    virtual double x_spacing_function(unsigned xelement,
                                      unsigned xnode,
                                      unsigned yelement,
                                      unsigned ynode)
    {
      // Calculate the values of equal increments in nodal values
      double xstep = (Xmax - Xmin) / ((Np - 1) * Nx);
      // Return the appropriate value
      return (Xmin + xstep * ((Np - 1) * xelement + xnode));
    }

    /// \short Return the value of the y-coordinate at the node given by the
    /// local node number (xnode, ynode) in the element (xelement,yelement).
    /// The description is in a "psudeo" two-dimensional coordinate system,
    /// so the range of xelement is [0,Nx-1], yelement is [0,Ny-1], and
    /// that of xnode and ynode is [0,Np-1]. The default it to return
    /// nodes that are equally spaced in the y coordinate.
    virtual double y_spacing_function(unsigned xelement,
                                      unsigned xnode,
                                      unsigned yelement,
                                      unsigned ynode)
    {
      double ystep = (Ymax - Ymin) / ((Np - 1) * Ny);
      // Return the appropriate value
      return (Ymin + ystep * ((Np - 1) * yelement + ynode));
    }
  };

  //==========================================================================
  /// Refineable version of the RectangularQuadMesh: A two-dimensional
  /// mesh of Quad elements with Nx elements in the "x" (horizonal)
  /// direction and Ny elements in the "y" (vertical) direction. Two
  /// Constructors are provided. The basic constructor
  /// assumes that the lower-left-hand corner of the mesh is (0,0) and
  /// takes only the arguments, Nx, Ny, Xmax and Ymax. The more complex
  /// constructor takes the additional arguments Xmin and Ymin.
  ///
  /// This class is designed to be used as a Base class for more complex
  /// two dimensional meshes. The virtual functions x_spacing_function()
  /// and y_spacing_function() may be overloaded to provide arbitrary node
  /// spacing. The default is uniformly spaced nodes in each direction.
  //===========================================================================
  template<class ELEMENT>
  class RefineableRectangularQuadMesh :
    public virtual RectangularQuadMesh<ELEMENT>,
    public RefineableQuadMesh<ELEMENT>
  {
  public:
    /// \short Simple constructor: nx: number of elements in x direction;
    /// ny: number of elements in y direction; lx, length of domain in x
    /// direction (0,lx); ly, length of domain in y direction (0,ly).
    /// Also pass pointer to timestepper (defaults to Steady)
    RefineableRectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt)
    {
      // Nodal positions etc. were created in constructor for
      // RectangularMesh<...>. Only need to setup quadtree forest
      this->setup_quadtree_forest();
    }

    /// \short Simple constructor: nx: number of elements in x direction;
    /// ny: number of elements in y direction; lx, length of domain in x
    /// direction (0,lx); ly, length of domain in y direction (0,ly);
    /// periodic_in_x, periodicity in x.
    /// Also pass pointer to timestepper (defaults to Steady)
    RefineableRectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      const bool& periodic_in_x,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(
        nx, ny, lx, ly, periodic_in_x, time_stepper_pt)
    {
      // Nodal positions etc. were created in constructor for
      // RectangularMesh<...>. Only need to setup quadtree forest
      this->setup_quadtree_forest();
    }

    /// \short Constructor that allows the specification of minimum and maximum
    /// values of x and y coordinates
    /// Also pass pointer to timestepper (defaults to Steady)
    RefineableRectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& xmin,
      const double& xmax,
      const double& ymin,
      const double& ymax,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(
        nx, ny, xmin, xmax, ymin, ymax, time_stepper_pt)
    {
      // Nodal positions etc. were created in constructor for
      // RectangularMesh<...>. Only need to setup quadtree forest
      this->setup_quadtree_forest();
    }

    /// \short Constructor that allows the specification of minimum and maximum
    /// values of x and y coordinates and periodicity
    /// Also pass pointer to timestepper (defaults to Steady)
    RefineableRectangularQuadMesh(
      const unsigned& nx,
      const unsigned& ny,
      const double& xmin,
      const double& xmax,
      const double& ymin,
      const double& ymax,
      const bool& periodic_in_x,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(
        nx, ny, xmin, xmax, ymin, ymax, periodic_in_x, time_stepper_pt)
    {
      // Nodal positions etc. were created in constructor for
      // RectangularMesh<...>. Only need to setup quadtree forest
      this->setup_quadtree_forest();
    }
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  //================================================================
  /// Elastic quad mesh with functionality to
  /// attach traction elements to the specified boundaries. We "upgrade"
  /// the RectangularQuadMesh to become an
  /// SolidMesh and equate the Eulerian and Lagrangian coordinates,
  /// thus making the domain represented by the mesh the stress-free
  /// configuration.
  //================================================================
  template<class ELEMENT>
  class ElasticRectangularQuadMesh :
    public virtual RectangularQuadMesh<ELEMENT>,
    public virtual SolidMesh
  {
  public:
    /// \short Constructor: Build mesh and copy Eulerian coords to Lagrangian
    /// ones so that the initial configuration is the stress-free one and
    /// assign boundary coordinates. Origin specifies
    /// an additional rigid-body displacement.
    ElasticRectangularQuadMesh<ELEMENT>(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      const Vector<double>& origin,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt)
    {
      // Translate the nodes
      unsigned nnod = nnode();
      for (unsigned j = 0; j < nnod; j++)
      {
        node_pt(j)->x(0) += origin[0];
        node_pt(j)->x(1) += origin[1];
      }

      /// Make the current configuration the undeformed one by
      /// setting the nodal Lagrangian coordinates to their current
      /// Eulerian ones
      set_lagrangian_nodal_coordinates();

      // Setup boundary coordinates
      set_boundary_coordinates(origin);
    }

    /// \short Constructor: Build mesh and copy Eulerian coords to Lagrangian
    /// ones so that the initial configuration is the stress-free one and
    /// assign boundary coordinates
    ElasticRectangularQuadMesh<ELEMENT>(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt)
    {
      // No shift
      Vector<double> origin(2, 0.0);

      /// Make the current configuration the undeformed one by
      /// setting the nodal Lagrangian coordinates to their current
      /// Eulerian ones
      set_lagrangian_nodal_coordinates();

      // Setup boundary coordinates
      set_boundary_coordinates(origin);
    }

    /// \short Constructor: Build mesh and copy Eulerian coords to Lagrangian
    /// ones so that the initial configuration is the stress-free one and
    /// assign boundary coordinates. This includes a boolean flag to specify
    /// if the mesh is periodic in the x-direction
    ElasticRectangularQuadMesh<ELEMENT>(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      const bool& periodic_in_x,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(
        nx, ny, lx, ly, periodic_in_x, time_stepper_pt)
    {
      // No shift
      Vector<double> origin(2, 0.0);

      /// Make the current configuration the undeformed one by
      /// setting the nodal Lagrangian coordinates to their current
      /// Eulerian ones
      set_lagrangian_nodal_coordinates();

      // Setup boundary coordinates
      set_boundary_coordinates(origin);
    }

  private:
    /// \short Setup the boundary coordinates. Vector
    /// origin specifies the coordinates of the lower left corner of
    /// the mesh.
    void set_boundary_coordinates(const Vector<double>& origin)
    {
      // 1D vector fo boundary coordinate
      Vector<double> zeta(1);

      // Loop over boundaries 0 and 2 where xi_0 is the varying
      // boundary coordinate
      for (unsigned b = 0; b < 3; b += 2)
      {
        // Number of nodes on those boundaries
        unsigned n_nod = nboundary_node(b);

        // Loop over the nodes
        for (unsigned i = 0; i < n_nod; i++)
        {
          // Boundary coordinate varies between 0 and L
          zeta[0] = boundary_node_pt(b, i)->xi(0) - origin[0];
          boundary_node_pt(b, i)->set_coordinates_on_boundary(b, zeta);
        }
        Boundary_coordinate_exists[b] = true;
      }

      // Loop over boundaries 1 and 3 where xi_1 is the varying
      // boundary coordinate
      for (unsigned b = 1; b < 4; b += 2)
      {
        // Number of nodes on those boundaries
        unsigned n_nod = nboundary_node(b);

        // Loop over the nodes
        for (unsigned i = 0; i < n_nod; i++)
        {
          // Boundary coordinate varies between +/- H/2
          zeta[0] = boundary_node_pt(b, i)->xi(1) - origin[1] -
                    0.5 * (this->Ymax - this->Ymin);
          boundary_node_pt(b, i)->set_coordinates_on_boundary(b, zeta);
        }
        Boundary_coordinate_exists[b] = true;
      }
    }
  };

  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////
  //////////////////////////////////////////////////////////////////////////

  //================================================================
  /// Elastic refineable quad mesh with functionality to
  /// attach traction elements to the specified boundaries. We "upgrade"
  /// the RefineableRectangularQuadMesh to become an
  /// SolidMesh and equate the Eulerian and Lagrangian coordinates,
  /// thus making the domain represented by the mesh the stress-free
  /// configuration. We also move the mesh "down" by half the
  /// the "height" so x=0 is located on the centreline -- appropriate
  /// for the beam-type problems for which this mesh was developed.
  //================================================================
  template<class ELEMENT>
  class ElasticRefineableRectangularQuadMesh :
    public virtual ElasticRectangularQuadMesh<ELEMENT>,
    public RefineableQuadMesh<ELEMENT>
  {
  public:
    /// \short Constructor: Build mesh and copy Eulerian coords to Lagrangian
    /// ones so that the initial configuration is the stress-free one and
    /// assign boundary coordinates (variable Lagrangian coordinates along
    /// the relevant boundaries).
    ElasticRefineableRectangularQuadMesh<ELEMENT>(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt),
      ElasticRectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt)

    {
      // Nodal positions etc. were created in base class.
      // Only need to setup quadtree forest
      this->setup_quadtree_forest();
    }

    /// \short Constructor: Build mesh and copy Eulerian coords to Lagrangian
    /// ones so that the initial configuration is the stress-free one and
    /// assign boundary coordinates. This includes a boolean flag to specify
    /// if the mesh is periodic in the x-direction
    ElasticRefineableRectangularQuadMesh<ELEMENT>(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      const bool& periodic_in_x,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(
        nx, ny, lx, ly, periodic_in_x, time_stepper_pt),
      ElasticRectangularQuadMesh<ELEMENT>(
        nx, ny, lx, ly, periodic_in_x, time_stepper_pt)
    {
      // Nodal positions etc. were created in base class.
      // Only need to setup quadtree forest
      this->setup_quadtree_forest();
    }

    /// \short Constructor: Build mesh and copy Eulerian coords to Lagrangian
    /// ones so that the initial configuration is the stress-free one and
    /// assign boundary coordinates (variable Lagrangian coordinates along
    /// the relevant boundaries). Origin specifies an additional rigid-body
    /// displacement.
    ElasticRefineableRectangularQuadMesh<ELEMENT>(
      const unsigned& nx,
      const unsigned& ny,
      const double& lx,
      const double& ly,
      const Vector<double>& origin,
      TimeStepper* time_stepper_pt = &Mesh::Default_TimeStepper) :
      RectangularQuadMesh<ELEMENT>(nx, ny, lx, ly, time_stepper_pt),
      ElasticRectangularQuadMesh<ELEMENT>(
        nx, ny, lx, ly, origin, time_stepper_pt)

    {
      // Nodal positions etc. were created in base class.
      // Only need to setup quadtree forest
      this->setup_quadtree_forest();
    }
  };

} // namespace oomph

#include "rectangular_quadmesh.tpp"
#endif
