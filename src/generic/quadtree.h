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
// Header file for quadtree and quadtree forest classes
#ifndef OOMPH_QUADTREE_HEADER
#define OOMPH_QUADTREE_HEADER


// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

#ifdef OOMPH_HAS_MPI
#include "mpi.h"
#endif

// OOMPH-LIB headers
#include "tree.h"
#include "matrices.h"


namespace oomph
{
  //====================================================================
  /// Namespace for QuadTree directions
  //====================================================================
  namespace QuadTreeNames
  {
    /// Directions. OMEGA is used if a direction is undefined
    /// in a certain context
    enum
    {
      SW,
      SE,
      NW,
      NE,
      N,
      E,
      S,
      W,
      OMEGA = 26
    };
  }; // namespace QuadTreeNames

  // Forward class definition for class representing the root of a QuadTree
  class QuadTreeRoot;

  //================================================================
  /// QuadTree class: Recursively defined, generalised quadtree.
  ///
  /// A QuadTree has:
  /// - a pointer to the object (of type RefineableQElement<2>) that it
  ///   represents in a mesh refinement context.
  /// - Vector of pointers to its four (SW/SE/NW/NE) sons (which are
  ///   themselves quadtrees).
  ///   If the Vector of pointers to the sons has zero length,
  ///   the QuadTree is a "leaf node" in the overall quadtree.
  /// - a pointer to its father. If this pointer is NULL, the QuadTree is the
  ///   the root node of the overall quadtree.
  /// This data is stored in the Tree base class.
  ///
  /// The tree can also be part of a forest. If that is the case, the root
  /// will have pointers to the roots of neighbouring quadtrees.
  ///
  /// The objects contained in the quadtree are assumed to be
  /// (topologically) rectangular elements whose geometry is
  /// parametrised by local coordinates \f$ {\bf s} \in [-1,1]^2 \f$.
  ///
  /// The tree can be traversed and actions performed
  /// at all its "nodes" or only at the leaf "nodes" ("nodes" without sons).
  ///
  /// Finally, the leaf "nodes" can be split depending on
  /// a criteria defined by the object.
  ///
  /// Note that QuadTrees are only generated by splitting existing
  /// QuadTrees. Therefore, the constructors are protected. The
  /// only QuadTree that "Joe User" can create is
  /// the (derived) class QuadTreeRoot.
  //=================================================================
  class QuadTree : public virtual Tree
  {
  public:
    /// Destructor. Note: Deleting a quadtree also deletes the
    /// objects associated with all non-leaf nodes!
    ~QuadTree() override {}

    /// Broken copy constructor
    QuadTree(const QuadTree& dummy) = delete;

    /// Broken assignment operator
    void operator=(const QuadTree&) = delete;

    /// Overload the function construct_son to ensure that the son
    /// is a specific QuadTree and not a general Tree.
    Tree* construct_son(RefineableElement* const& object_pt,
                        Tree* const& father_pt,
                        const int& son_type) override
    {
      QuadTree* temp_quad_pt = new QuadTree(object_pt, father_pt, son_type);
      return temp_quad_pt;
    }

    /// Return pointer to greater or equal-sized edge neighbour
    /// in specified \c direction; also provide info regarding the relative
    /// size and orientation of neighbour:
    /// - The vector translate_s turns the index of the local coordinate
    ///   in the present quadtree into that of the neighbour. If there are no
    ///   rotations then translate_s[i] = i, but if, for example, the
    ///   neighbour's eastern face is adjacent to our northern face
    ///   translate_s[0] = 1 and translate_s[1] = 0. Of course, this could be
    ///   deduced after the fact, but it's easier to do it here.
    /// - In the present quadtree, the lower left (south west) vertex is
    ///   located at local coordinates (-1,-1). This point is located
    ///   at the local coordinates (\c s_lo[0], \c s_lo[1]) in the neighbouring
    ///   quadtree.
    /// - ditto with s_hi: In the present quadtree, the upper right (north east)
    ///   vertex is located at local coordinates (1,1). This point is located
    ///   at the local coordinates (\c s_hi[0], \c s_hi[1]) in the neighbouring
    ///   quadtree.
    /// - We're looking for a neighbour in the specified \c direction. When
    ///   viewed from the neighbouring quadtree, the edge that separates
    ///   the present quadtree from its neighbour is the neighbour's \c edge
    ///   edge. If there's no rotation between the two quadtrees, this is a
    ///   simple reflection: For instance, if we're looking
    ///   for a neighhbour in the \c N [orthern] \c direction, \c edge will
    ///   be \c S [outh]
    /// - \c diff_level <= 0 indicates the difference in refinement levels
    /// between
    ///   the two neighbours. If \c diff_level==0, the neighbour has the
    ///   same size as the current quadtree.
    /// - \c in_neighbouring_tree indicates whether the neighbour is actually
    ///   in another tree in the forest. The introduction of this flag
    ///   was necessitated by periodic problems where a TreeRoot can be its
    ///   own neighbour.
    QuadTree* gteq_edge_neighbour(const int& direction,
                                  Vector<unsigned>& translate_s,
                                  Vector<double>& s_lo,
                                  Vector<double>& s_hi,
                                  int& edge,
                                  int& diff_level,
                                  bool& in_neighbouring_tree) const;

    /// Traverse Tree: Preorder traverse and stick pointers to
    /// neighbouring leaf nodes (only) into Vector
    void stick_neighbouring_leaves_into_vector(
      Vector<const QuadTree*>& tree_neighbouring_nodes,
      Vector<Vector<double>>& tree_neighbouring_s_lo,
      Vector<Vector<double>>& tree_neighbouring_s_hi,
      Vector<int>& tree_neighbouring_diff_level,
      const QuadTree* my_neigh_pt,
      const int& direction) const;

    /// Self-test: Check all neighbours. Return success (0)
    /// if the max. distance between corresponding points in the
    /// neighbours is less than the tolerance specified in the
    /// static value QuadTree::Max_neighbour_finding_tolerance.
    unsigned self_test();

    /// Setup the static data, rotation and reflection schemes, etc
    static void setup_static_data();

    /// Doc/check all neighbours of quadtree (nodes) contained in the
    /// Vector forest_node_pt. Output into neighbours_file which can
    /// be viewed from tecplot with QuadTreeNeighbours.mcr
    /// Neighbour info and errors are displayed on
    /// neighbours_txt_file.  Finally, compute the max. error between
    /// vertices when viewed from neighhbouring element.
    /// If the two filestreams are closed, output is suppressed.
    static void doc_neighbours(Vector<Tree*> forest_nodes_pt,
                               std::ofstream& neighbours_file,
                               std::ofstream& neighbours_txt_file,
                               double& max_error);


    /// Translate (enumerated) directions into strings
    static Vector<std::string> Direct_string;

  protected:
    /// Default constructor (empty and broken)
    QuadTree()
    {
      throw OomphLibError(
        "Don't call an empty constructor for a QuadTree object",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Default constructor for empty (root) tree:
    /// no father, no sons; just pass a pointer to its object
    /// Protected because QuadTrees can only be created internally,
    /// during the split operation. Only QuadTreeRoots can be
    /// created externally.
    QuadTree(RefineableElement* const& object_pt) : Tree(object_pt) {}

    /// Constructor for tree that has a father: Pass it the pointer
    /// to its object, the pointer to its father and tell it what type
    /// of son (SE/SW/NE/NW) it is.
    /// Protected because QuadTrees can only be created internally,
    /// during the split operation.  Only QuadTreeRoots can be
    /// created externally.
    QuadTree(RefineableElement* const& object_pt,
             Tree* const& father_pt,
             const int& son_type)
      : Tree(object_pt, father_pt, son_type)
    {
    }

    /// Bool indicating that static member data has been setup
    static bool Static_data_has_been_setup;


  private:
    /// Find greater or equal-sized edge neighbour in direction.
    /// Auxiliary internal routine which passes additional information around.
    QuadTree* gteq_edge_neighbour(const int& direction,
                                  double& s_diff,
                                  int& diff_level,
                                  bool& in_neighbouring_tree,
                                  int max_level,
                                  QuadTreeRoot* const& orig_root_pt) const;

    /// Colours for neighbours in various directions
    static Vector<std::string> Colour;

    /// S_base(i,direction):  Initial value for coordinate s[i] on
    /// the edge indicated by direction (S/E/N/W)
    static DenseMatrix<double> S_base;

    /// S_step(i,direction) Increments for coordinate s[i] when
    /// progressing along the edge indicated by direction (S/E/N/W);
    /// Left/lower vertex: S_base; Right/upper vertex: S_base + S_step
    static DenseMatrix<double> S_step;

    /// Get opposite edge, e.g. Reflect_edge[N]=S
    static Vector<int> Reflect_edge;

    /// Array of direction/quadrant adjacency scheme:
    /// Is_adjacent(i_vertex_or_edge,j_quadrant): Is edge/vertex
    /// adjacent to quadrant?
    static DenseMatrix<bool> Is_adjacent;

    /// Reflection scheme: Reflect(direction,quadrant): Get mirror
    /// of quadrant in specified direction. E.g. Reflect(S,NE)=SE
    static DenseMatrix<int> Reflect;

    /// Rotate coordinates: If North becomes NorthIs then direction
    /// becomes Rotate(NorthIs,direction). E.g.  Rotate(E,NW)=NE;
    static DenseMatrix<int> Rotate;

    /// Angle betwen rotated coordinates: If old_direction becomes
    /// new_direction then the angle between the axes (in anti-clockwise
    /// direction is Rotate_angle(old_direction,new_direction); E.g.
    /// Rotate_angle(E,N)=90;
    static DenseMatrix<int> Rotate_angle;

    /// S_direct(direction,son_quadrant): The lower left corner
    /// of son_quadrant has an offset of h/2 S_direct(direction,son_quadrant)
    /// in the specified direction. E.g. S_direct(S,NE)=1 and  S_direct(S,NW)=0
    static DenseMatrix<int> S_direct;
  };


  //===================================================================
  /// QuadTreeRoot is a QuadTree that forms the root of a (recursive)
  /// quadtree. The "root node" is special as it holds additional
  /// information about its neighbours and their relative
  /// rotation (inside a QuadTreeForest).
  //==================================================================
  class QuadTreeRoot : public virtual QuadTree, public virtual TreeRoot
  {
  private:
    /// Vector giving the north equivalent of the neighbours:
    /// When viewed from the current quadtree's \c neighbour neighbour,
    /// our northern direction is the neighbour's North_equivalent[neighbour]
    /// direction. If there's no rotation, this map contains the identify
    /// so that, e.g. \c North_equivalent[W]=N (read as: "in my Western
    /// neighbour, my North is its North"). If the western neighbour is rotated
    /// by 180 degrees relative to the current quadtree, say, we have
    /// \c North_equivalent[W]=S (read as: "in my Western
    /// neighbour, my North is its South"); etc.
    Vector<int> North_equivalent;

  public:
    /// Constructor for the (empty) root quadtree: Pass pointer to
    /// associated object, a RefineableQElement<2>.
    QuadTreeRoot(RefineableElement* const& object_pt)
      : Tree(object_pt), QuadTree(object_pt), TreeRoot(object_pt)
    {
#ifdef PARANOID
      // Check that static member data has been setup
      if (!Static_data_has_been_setup)
      {
        std::string error_message =
          "Static member data hasn't been setup yet.\n";
        error_message += "Call QuadTree::setup_static_data() before creating\n";
        error_message += "any QuadTreeRoots\n";

        throw OomphLibError(
          error_message, OOMPH_CURRENT_FUNCTION, OOMPH_EXCEPTION_LOCATION);
      }
#endif

      using namespace QuadTreeNames;

      // Initialise the North equivalents of the neighbouring QuadTreeRoots
      North_equivalent.resize(27);

      North_equivalent[N] = N;
      North_equivalent[E] = N;
      North_equivalent[W] = N;
      North_equivalent[S] = N;
    }


    /// Broken copy constructor
    QuadTreeRoot(const QuadTreeRoot& dummy) = delete;

    /// Broken assignment operator
    void operator=(const QuadTreeRoot&) = delete;


    /// Return north equivalent of the neighbours in specified
    /// direction: When viewed from the current quadtree's \c neighbour
    /// neighbour, our northern direction is the neighbour's
    /// north_equivalent(neighbour) direction. If there's no rotation, this map
    /// contains the identify so that, e.g. \c north_equivalent(W)=N (read as:
    /// "in my Western neighbour, my North is its North"). If the western
    /// neighbour is rotated by 180 degrees relative to the current quadtree,
    /// say, we have \c north_equivalent(W)=S (read as: "in my Western
    /// neighbour, my North is its South"); etc.
    int& north_equivalent(const int& neighbour)
    {
#ifdef PARANOID
      using namespace QuadTreeNames;
      // Neighbour can only be located in N/S/E/W direction
      if ((neighbour != S) && (neighbour != N) && (neighbour != W) &&
          (neighbour != E))
      {
        std::ostringstream error_message;
        error_message << "The neighbour can only be N,S,E,W, not"
                      << Direct_string[neighbour] << std::endl;

        throw OomphLibError(error_message.str(),
                            OOMPH_CURRENT_FUNCTION,
                            OOMPH_EXCEPTION_LOCATION);
      }
#endif
      return North_equivalent[neighbour];
    }


    /// If quadtree_root_pt is a neighbour, return the direction
    /// [N/S/E/W] in which it is found, otherwise return OMEGA
    int direction_of_neighbour(QuadTreeRoot* quadtree_root_pt)
    {
      using namespace QuadTreeNames;
      if (Neighbour_pt[N] == quadtree_root_pt)
      {
        return N;
      }
      if (Neighbour_pt[E] == quadtree_root_pt)
      {
        return E;
      }
      if (Neighbour_pt[S] == quadtree_root_pt)
      {
        return S;
      }
      if (Neighbour_pt[W] == quadtree_root_pt)
      {
        return W;
      }
      // If we get here, it's not a neighbour
      return OMEGA;
    }
  };


  //================================================================
  /// A QuadTreeForest consists of a collection of QuadTreeRoots.
  /// Each member tree can have neighbours to its S/W/N/E
  /// and the orientation of their compasses can differ, allowing
  /// for complex, unstructured meshes.
  //=================================================================
  class QuadTreeForest : public TreeForest
  {
  public:
    /// Default constructor (empty and broken)
    QuadTreeForest()
    {
      // Throw an error
      throw OomphLibError(
        "Don't call an empty constructor for a QuadTreeForest object",
        OOMPH_CURRENT_FUNCTION,
        OOMPH_EXCEPTION_LOCATION);
    }

    /// Constructor: Pass vector of pointers to the roots of the
    /// constituent QuadTrees
    QuadTreeForest(Vector<TreeRoot*>& trees_pt);

    /// Broken copy constructor
    QuadTreeForest(const QuadTreeForest& dummy) = delete;

    /// Broken assignment operator
    void operator=(const QuadTreeForest&) = delete;

    /// Destructor: Delete the constituent quadtrees (and thus
    /// the objects associated with its non-leaf nodes!)
    ~QuadTreeForest() override {}

    /// Document and check all the neighbours of all the nodes
    /// in the forest. DocInfo object specifies the output directory
    /// and file numbers for the various files. If \c doc_info.disable_doc()
    /// has been called no output is created.
    void check_all_neighbours(DocInfo& doc_info) override;

    /// Open output files that will store any hanging nodes in
    /// the forest and return a vector of the streams.
    void open_hanging_node_files(
      DocInfo& doc_info, Vector<std::ofstream*>& output_stream) override;

    /// Self-test: Check all neighbours. Return success (0)
    /// if the max. distance between corresponding points in the
    /// neighbours is less than the tolerance specified in the
    /// static value QuadTree::Max_neighbour_finding_tolerance.
    unsigned self_test();

  private:
    /// Construct the rotation schemes
    void construct_north_equivalents();

    /// Construct the neighbour lookup scheme
    void find_neighbours();

    /// Return pointer to i-th root quadtree in this forest.
    /// (Performs a dynamic cast from the TreeRoot to a
    /// QuadTreeRoot).
    QuadTreeRoot* quadtree_pt(const unsigned& i)
    {
      return dynamic_cast<QuadTreeRoot*>(Trees_pt[i]);
    }

    /// Given the number i of the root quadtree in this forest, return
    /// pointer to its neighbour in the specified direction. NULL
    /// if neighbour doesn't exist. (This does the dynamic cast
    /// from a TreeRoot to a QuadTreeRoot internally).
    QuadTreeRoot* quad_neigh_pt(const unsigned& i, const int& direction)
    {
      return dynamic_cast<QuadTreeRoot*>(Trees_pt[i]->neighbour_pt(direction));
    }
  };

} // namespace oomph

#endif
