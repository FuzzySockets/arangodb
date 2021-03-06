////////////////////////////////////////////////////////////////////////////////
/// @brief path-based index
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2014 ArangoDB GmbH, Cologne, Germany
/// Copyright 2004-2014 triAGENS GmbH, Cologne, Germany
///
/// Licensed under the Apache License, Version 2.0 (the "License");
/// you may not use this file except in compliance with the License.
/// You may obtain a copy of the License at
///
///     http://www.apache.org/licenses/LICENSE-2.0
///
/// Unless required by applicable law or agreed to in writing, software
/// distributed under the License is distributed on an "AS IS" BASIS,
/// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
/// See the License for the specific language governing permissions and
/// limitations under the License.
///
/// Copyright holder is ArangoDB GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2014, ArangoDB GmbH, Cologne, Germany
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef ARANGODB_INDEXES_PATH_BASED_INDEX_H
#define ARANGODB_INDEXES_PATH_BASED_INDEX_H 1

#include "Basics/Common.h"
#include "Indexes/Index.h"
#include "VocBase/shaped-json.h"
#include "VocBase/vocbase.h"
#include "VocBase/voc-types.h"
#include "VocBase/document-collection.h"

struct TRI_json_t;
class VocShaper;

// -----------------------------------------------------------------------------
// --SECTION--                                              class PathBasedIndex
// -----------------------------------------------------------------------------

namespace triagens {
  namespace aql {
    enum AstNodeType : uint32_t;
  }

  namespace arango {

    class PathBasedIndex : public Index {

      protected:

        struct PermutationState {
          PermutationState (triagens::aql::AstNodeType type, triagens::aql::AstNode const* value, size_t attributePosition, size_t n)
            : type(type),
              value(value),
              attributePosition(attributePosition),
              current(0),
              n(n) {
            TRI_ASSERT(n > 0);
          }
            
          triagens::aql::AstNode const* getValue () const; 

          triagens::aql::AstNodeType    type;
          triagens::aql::AstNode const* value;
          size_t const                  attributePosition;
          size_t                        current;
          size_t const                  n;
        };

// -----------------------------------------------------------------------------
// --SECTION--                                          shared Permuation struct
// -----------------------------------------------------------------------------

// -----------------------------------------------------------------------------
// --SECTION--                                        constructors / destructors
// -----------------------------------------------------------------------------

      public:

        PathBasedIndex () = delete;

        PathBasedIndex (TRI_idx_iid_t,
                        struct TRI_document_collection_t*,
                        std::vector<std::vector<triagens::basics::AttributeName>> const&,
                        bool unique,
                        bool sparse,
                        bool allowPartialIndex);
        
        explicit PathBasedIndex (struct TRI_json_t const*, bool);

        ~PathBasedIndex ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not the index should reveal its fields
////////////////////////////////////////////////////////////////////////////////
        
        bool dumpFields () const override final {
          return true;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the attribute paths
////////////////////////////////////////////////////////////////////////////////
        
        std::vector<std::vector<std::pair<TRI_shape_pid_t, bool>>> const& paths () const {
          return _paths;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the memory needed for an index key entry
////////////////////////////////////////////////////////////////////////////////

        inline size_t elementSize () const {
          return TRI_index_element_t::memoryUsage(_paths.size());
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                 protected methods
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief helper function to insert a document into any index type
////////////////////////////////////////////////////////////////////////////////

        int fillElement (std::vector<TRI_index_element_t*>& elements,
                         TRI_doc_mptr_t const* document);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the number of paths
////////////////////////////////////////////////////////////////////////////////

        inline size_t numPaths () const {
          return _paths.size();
        }

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief helper function to transform AttributeNames into pid lists
/// This will create PIDs for all indexed Attributes
////////////////////////////////////////////////////////////////////////////////

        std::vector<std::vector<std::pair<TRI_shape_pid_t, bool>>> fillPidPaths ();

////////////////////////////////////////////////////////////////////////////////
/// @brief helper function to create a set of index combinations to insert
////////////////////////////////////////////////////////////////////////////////

        void buildIndexValue (TRI_shaped_json_t const*, 
                              std::vector<TRI_shaped_json_t>&);

////////////////////////////////////////////////////////////////////////////////
/// @brief helper function to create a set of index combinations to insert
////////////////////////////////////////////////////////////////////////////////

        void buildIndexValues (TRI_shaped_json_t const*, 
                               TRI_shaped_json_t const*,
                               size_t,
                               size_t, 
                               std::unordered_set<std::vector<TRI_shaped_json_t>>&,
                               std::vector<TRI_shaped_json_t>&,
                               bool);

// -----------------------------------------------------------------------------
// --SECTION--                                               protected variables
// -----------------------------------------------------------------------------

      protected:

////////////////////////////////////////////////////////////////////////////////
/// @brief the shaper for the collection
////////////////////////////////////////////////////////////////////////////////

        VocShaper* _shaper;

////////////////////////////////////////////////////////////////////////////////
/// @brief the attribute paths
////////////////////////////////////////////////////////////////////////////////
        
        std::vector<std::vector<std::pair<TRI_shape_pid_t, bool>>> const  _paths;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not at least one attribute is expanded
////////////////////////////////////////////////////////////////////////////////
        
        bool _useExpansion;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not partial indexing is allowed
////////////////////////////////////////////////////////////////////////////////
        
        bool _allowPartialIndex;
    };

  }
}

#endif

// -----------------------------------------------------------------------------
// --SECTION--                                                       END-OF-FILE
// -----------------------------------------------------------------------------

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
