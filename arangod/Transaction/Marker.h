////////////////////////////////////////////////////////////////////////////////
/// @brief transaction markers
///
/// @file
///
/// DISCLAIMER
///
/// Copyright 2004-2013 triAGENS GmbH, Cologne, Germany
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
/// Copyright holder is triAGENS GmbH, Cologne, Germany
///
/// @author Jan Steemann
/// @author Copyright 2011-2013, triAGENS GmbH, Cologne, Germany
////////////////////////////////////////////////////////////////////////////////

#ifndef TRIAGENS_TRANSACTION_MARKER_H
#define TRIAGENS_TRANSACTION_MARKER_H 1

#include "Basics/Common.h"
#include "Basics/BsonHelper.h"
#include "VocBase/datafile.h"

namespace triagens {
  namespace transaction {

    struct Marker {
      Marker (TRI_df_marker_type_e type,
              size_t size) 
        : buffer(new char[sizeof(TRI_df_marker_t) + size]),
          size(sizeof(TRI_df_marker_t) + size) {

        // initialise the marker header
        auto h = header();
        h->_type = type;
        h->_size = static_cast<TRI_voc_size_t>(size);
        h->_crc  = 0;
        h->_tick = 0;
      }

      virtual ~Marker () {
        if (buffer != nullptr) {
          delete buffer;
        }
      }

      inline TRI_df_marker_t* header () const {
        return (TRI_df_marker_t*) buffer;
      }
      
      inline char* data () const {
        return (char*) buffer + sizeof(TRI_df_marker_t);
      }

      template <typename T> void storeValue (char*& ptr, T value) {
        *((T*) ptr) = value;
        ptr += sizeof(T);
      }
      
      char*          buffer;
      uint32_t const size;
    };

    struct DocumentMarker : public Marker {
      DocumentMarker (TRI_voc_tick_t databaseId,
                      TRI_voc_cid_t collectionId,
                      triagens::basics::Bson const& document)
        : Marker(TRI_WAL_MARKER_DOCUMENT_STANDALONE, sizeof(TRI_voc_tick_t) + sizeof(TRI_voc_cid_t) + document.getSize()) {

        char* p = data();
        storeValue<TRI_voc_tick_t>(p, databaseId);
        storeValue<TRI_voc_cid_t>(p, collectionId);

        memcpy(p, document.getBuffer(), document.getSize());
      }

      ~DocumentMarker () {
      }

    };

  }
}

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
