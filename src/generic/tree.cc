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
// Non-inline and non-templated functions for Tree and TreeForest
// classes

// Config header generated by autoconfig
#ifdef HAVE_CONFIG_H
#include <oomph-lib-config.h>
#endif

// oomph-lib headers
#include "tree.h"

// Need to include this so that we can use the member functions of
// RefineableElement
#include "refineable_elements.h"

namespace oomph
{
  //========================================================================
  /// Static value used to represent unassigned quantities.
  /// This has to remain consistent with the enumerations in
  /// the Octree and Quadtree namespaces!
  //========================================================================
  const int Tree::OMEGA = 26;

  //========================================================================
  /// Maximum tolerance for neighbour finding (distance between points
  /// when identified from the two neighbours)
  //========================================================================
  double Tree::Max_neighbour_finding_tolerance = 1.0e-14;

  //================================================================
  /// Constructor for empty (root) Tree: No father, no sons.
  /// Pass pointer to the object that this tree (root) contains.
  /// Protected because Trees can only be created internally,
  /// during the split operation.
  //=================================================================
  Tree::Tree(RefineableElement* const& object_pt) : Object_pt(object_pt)
  {
    // No father node:
    Father_pt = 0;

    // I'm not a son, so I don't have a son type either....
    Son_type = OMEGA;

    // I am the root
    Level = 0;

    // Root pointer must be set in the TreeRoot constructor
    Root_pt = 0;

    // No sons just yet:
    Son_pt.resize(0);

    // Tell the object which tree it's represented by
    object_pt->set_tree_pt(this);
  };

  //================================================================
  /// Constructor for Tree: This one has a father
  /// and is a son of a certain type, but has no sons
  /// of its own (just yet), so it's a leaf node.
  /// Protected because Trees can only be created internally,
  /// during the split operation.
  //=================================================================
  Tree::Tree(RefineableElement* const& object_pt,
             Tree* const& father_pt,
             const int& son_type) :
    Object_pt(object_pt)
  {
    // Set the father node
    Father_pt = father_pt;

    // Set the son type
    Son_type = son_type;

    // My level is one deeper than that of my father
    Level = father_pt->Level + 1;

    // I have no sons of my own just yet:
    Son_pt.resize(0);

    // Inherit root pointer from father
    Root_pt = father_pt->Root_pt;

    // Tell the object which tree it's represented by
    object_pt->set_tree_pt(this);
  }

  //================================================================
  /// Destructor for Tree: Recursively kill all sons and the
  /// associated objects of the non-leaf nodes. However, the objects
  /// of the leaf nodes are not destroyed. Their destruction is handled
  /// by the Mesh destructor.
  //=================================================================
  Tree::~Tree()
  {
    // Loop over all the sons and delete them
    unsigned nsons = Son_pt.size();
    for (unsigned i = 0; i < nsons; i++)
    {
      // This will call the son's destructor (a subtle recursion)
      delete Son_pt[i];
      Son_pt[i] = 0;
    }

    // Delete the object only if the Tree has sons (is not a leaf node)
    if (nsons > 0)
    {
      delete Object_pt;
      Object_pt = 0;
    }
  }

  //================================================================
  /// Preorder traverse the tree and execute void Tree member function
  /// at all nodes
  //=================================================================
  void Tree::traverse_all(Tree::VoidMemberFctPt member_function)
  {
    // Process the object contained in (well, pointed to) by current
    // Tree
    (this->*member_function)();

    // Now do the sons (if they exist)
    unsigned numsons = Son_pt.size();
    for (unsigned i = 0; i < numsons; i++)
    {
      Son_pt[i]->traverse_all(member_function);
    }
  }

  //================================================================
  /// Preorder traverse the tree and execute a void Tree member function
  /// that takes one argument at all nodes.
  //=================================================================
  void Tree::traverse_all(Tree::VoidMeshPtArgumentMemberFctPt member_function,
                          Mesh*& mesh_pt)
  {
    // Process the object contained in (well, pointed to) by current
    // Tree
    (this->*member_function)(mesh_pt);

    // Now do the sons (if they exist)
    unsigned numsons = Son_pt.size();
    for (unsigned i = 0; i < numsons; i++)
    {
      Son_pt[i]->traverse_all(member_function, mesh_pt);
    }
  }

  //==================================================================
  /// Preorder traverse the tree and execute a void Tree member function
  /// for all elements that are not leaf elements.
  //==================================================================
  void Tree::traverse_all_but_leaves(Tree::VoidMemberFctPt member_function)
  {
    // Find the number of sons
    unsigned n_sons = Son_pt.size();
    // If the Tree has sons,
    if (n_sons > 0)
    {
      // Execute the function
      (this->*member_function)();
      // Proceed to the sons
      for (unsigned i = 0; i < n_sons; i++)
      {
        Son_pt[i]->traverse_all_but_leaves(member_function);
      }
    }
    // If the tree has no sons, the function will not be executed
  }

  //================================================================
  /// Preorder traverse the tree and execute void Tree member function
  /// at the leaves only (ignore "grey" = non-leaf nodes)
  //=================================================================
  void Tree::traverse_leaves(Tree::VoidMemberFctPt member_function)
  {
    // If the Tree has sons
    unsigned numsons = Son_pt.size();
    if (numsons > 0)
    {
      // Proceed to the sons (if they exist)
      for (unsigned i = 0; i < numsons; i++)
      {
        Son_pt[i]->traverse_leaves(member_function);
      }
    }
    else
    {
      // Call the member function
      (this->*member_function)();
    }
  }

  //================================================================
  /// Preorder traverse the tree and execute void Tree member function
  /// that takes one argument at the leaves only
  /// (ignore "grey" = non-leaf nodes)
  //=================================================================
  void Tree::traverse_leaves(
    Tree::VoidMeshPtArgumentMemberFctPt member_function, Mesh*& mesh_pt)
  {
    // If the Tree has sons
    unsigned numsons = Son_pt.size();
    if (numsons > 0)
    {
      // Proceed to the sons (if they exist)
      for (unsigned i = 0; i < numsons; i++)
      {
        Son_pt[i]->traverse_leaves(member_function, mesh_pt);
      }
    }
    else
    {
      // Call the member function
      (this->*member_function)(mesh_pt);
    }
  }

  //================================================================
  /// Traverse Tree: Preorder traverse and stick pointers to leaf
  /// nodes (only) into Vector
  //=================================================================
  void Tree::stick_leaves_into_vector(Vector<Tree*>& tree_nodes)
  {
    // If the Tree has sons
    unsigned numsons = Son_pt.size();
    if (numsons > 0)
    {
      // Now do the sons (if they exist)
      for (unsigned i = 0; i < numsons; i++)
      {
        Son_pt[i]->stick_leaves_into_vector(tree_nodes);
      }
    }
    else
    {
      tree_nodes.push_back(this);
    }
  }

  //================================================================
  /// Traverse Tree: Preorder traverse and stick pointer to all
  /// nodes (incl. "grey"=non-leaf ones) into Vector
  //=================================================================
  void Tree::stick_all_tree_nodes_into_vector(Vector<Tree*>& all_tree_nodes)
  {
    all_tree_nodes.push_back(this);

    // If the Tree has sons
    unsigned numsons = Son_pt.size();
    if (numsons > 0)
    {
      // Now do the sons (if they exist)
      for (unsigned i = 0; i < numsons; i++)
      {
        Son_pt[i]->stick_all_tree_nodes_into_vector(all_tree_nodes);
      }
    }
  }

  //================================================================
  /// If required, kill the sons to perform unrefinement.
  ///
  /// Unrefinement is performed if
  ///
  ///      object_pt()->sons_to_be_unrefined()
  ///
  /// returns true.
  //=================================================================
  void Tree::merge_sons_if_required(Mesh*& mesh_pt)
  {
    // Check if unrefinement is required
    if (Object_pt->sons_to_be_unrefined())
    {
      // Rebuild the father from sons
      object_pt()->rebuild_from_sons(mesh_pt);

      // Find the number of sons
      unsigned n_sons = nsons();
      // Kill all the sons
      for (unsigned ison = 0; ison < n_sons; ison++)
      {
        // Unbuild the element by marking the nodes as obsolete
        son_pt(ison)->object_pt()->unbuild();

        // Delete the object. This must be done here, because the
        // destructor of a tree does not delete the leaf nodes
        // (the actual elements that are used in the mesh).
        // In general, the destruction of the leaf nodes is handled by the
        // mesh destructors.
        delete son_pt(ison)->object_pt();

        // Now delete the tree representation
        delete son_pt(ison);
      }

      Son_pt.resize(0);

      // Have merged my sons -- can't do it again...
      Object_pt->deselect_sons_for_unrefinement();
    }
  }

  //===================================================================
  /// Call the RefineableElement's deactivate_element() function that
  /// is used to perform any final changes to internal data storage
  /// of deactivated objects.
  //===================================================================
  void Tree::deactivate_object()
  {
    // Call the function
    object_pt()->deactivate_element();
  }

  //================================================================
  /// Constructor for TreeForest:
  ///
  /// Pass:
  ///  - trees_pt[], the Vector of pointers to the constituent trees
  ///    (TreeRoot objects)
  ///
  /// Note that the pointers to the neighbour's of each tree must have
  /// been allocated before the constructor is called, otherwise the
  /// relative rotation scheme will not be constructed correctly.
  //=================================================================
  TreeForest::TreeForest(Vector<TreeRoot*>& trees_pt) : Trees_pt(trees_pt) {}

  //================================================================
  /// Kill tree forest: Delete the constituent trees
  //=================================================================
  TreeForest::~TreeForest()
  {
    long int numtrees = Trees_pt.size();
    for (long int i = 0; i < numtrees; i++)
    {
      // Kill the trees
      delete Trees_pt[i];
      Trees_pt[i] = 0;
    }
  }

  //================================================================
  /// Traverse TreeForest: Preorder traverse and stick
  /// pointers to leaf nodes (only) into Vector
  //=================================================================
  void TreeForest::stick_leaves_into_vector(Vector<Tree*>& forest_nodes)
  {
    unsigned numtrees = ntree();
    for (unsigned itree = 0; itree < numtrees; itree++)
    {
      // Now do the sons (if they exist)
      unsigned numsons = tree_pt(itree)->nsons();
      if (numsons > 0)
      {
        for (unsigned i = 0; i < numsons; i++)
        {
          tree_pt(itree)->son_pt(i)->stick_leaves_into_vector(forest_nodes);
        }
      }
      else
      {
        forest_nodes.push_back(tree_pt(itree));
      }
    }
  }

  //================================================================
  /// Traverse TreeForest: Preorder traverse and stick
  /// pointers to all nodes into Vector
  //=================================================================
  void TreeForest::stick_all_tree_nodes_into_vector(
    Vector<Tree*>& all_forest_nodes)
  {
    unsigned numtrees = ntree();
    for (unsigned itree = 0; itree < numtrees; itree++)
    {
      all_forest_nodes.push_back(tree_pt(itree));

      // Now do the sons (if they exist)
      unsigned numsons = tree_pt(itree)->nsons();
      if (numsons > 0)
      {
        // Now do the sons (if they exist)
        for (unsigned i = 0; i < numsons; i++)
        {
          tree_pt(itree)->son_pt(i)->stick_all_tree_nodes_into_vector(
            all_forest_nodes);
        }
      }
    }
  }

  //====================================================================
  /// Close the hanging node output files and delete storage allocated at
  /// the pointers. This can be done generically at the base level
  //====================================================================
  void TreeForest::close_hanging_node_files(
    DocInfo& doc_info, Vector<std::ofstream*>& output_stream)
  {
    // Find the number of files
    unsigned n_file = output_stream.size();
    // If we opened the files, close them
    if (doc_info.is_doc_enabled())
    {
      for (unsigned n = 0; n < n_file; n++)
      {
        output_stream[n]->close();
      }
    }

    // We should have always created ofstreams, so delete them and null out
    for (unsigned n = n_file; n > 0; n--)
    {
      delete output_stream[n - 1];
      output_stream[n - 1] = 0;
    }

    // Finally clear out the vector
    output_stream.clear();
  }

} // namespace oomph
