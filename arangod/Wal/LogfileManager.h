////////////////////////////////////////////////////////////////////////////////
/// @brief Write-ahead log file manager
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

#ifndef TRIAGENS_WAL_LOGFILE_MANAGER_H
#define TRIAGENS_WAL_LOGFILE_MANAGER_H 1

#include "Basics/Common.h"
#include "Basics/Mutex.h"
#include "Basics/ReadWriteLock.h"
#include "ApplicationServer/ApplicationFeature.h"
#include "Wal/Logfile.h"
#include "Wal/Slots.h"

#include <regex.h>

namespace triagens {
  namespace wal {
    
    class AllocatorThread;
    class CollectorThread;
    class Slot;
    class SynchroniserThread;

// -----------------------------------------------------------------------------
// --SECTION--                                              class LogfileManager
// -----------------------------------------------------------------------------

    class LogfileManager : public rest::ApplicationFeature {

      friend class AllocatorThread;
      friend class CollectorThread;

////////////////////////////////////////////////////////////////////////////////
/// @brief LogfileManager
////////////////////////////////////////////////////////////////////////////////

      private:
        LogfileManager (LogfileManager const&);
        LogfileManager& operator= (LogfileManager const&);

// -----------------------------------------------------------------------------
// --SECTION--                                      constructors and destructors
// -----------------------------------------------------------------------------

      public:

        LogfileManager (std::string*);

////////////////////////////////////////////////////////////////////////////////
/// @brief destroy the logfile manager
////////////////////////////////////////////////////////////////////////////////

        ~LogfileManager ();

////////////////////////////////////////////////////////////////////////////////
/// @brief get the logfile manager instance
////////////////////////////////////////////////////////////////////////////////

        static LogfileManager* instance ();

////////////////////////////////////////////////////////////////////////////////
/// @brief initialise the logfile manager instance
////////////////////////////////////////////////////////////////////////////////

        static void initialise (std::string*);

// -----------------------------------------------------------------------------
// --SECTION--                                        ApplicationFeature methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void setupOptions (std::map<string, triagens::basics::ProgramOptionsDescription>&);

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool prepare ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool open ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        bool start ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void close ();

////////////////////////////////////////////////////////////////////////////////
/// {@inheritDoc}
////////////////////////////////////////////////////////////////////////////////

        void stop ();

// -----------------------------------------------------------------------------
// --SECTION--                                                    public methods
// -----------------------------------------------------------------------------

      public:

////////////////////////////////////////////////////////////////////////////////
/// @brief get the maximum size of a logfile entry
////////////////////////////////////////////////////////////////////////////////

        inline uint32_t maxEntrySize () const {
          return 2 << 30; // 2 GB
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the logfile directory
////////////////////////////////////////////////////////////////////////////////

        inline std::string directory () const {
          return _directory;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the logfile size
////////////////////////////////////////////////////////////////////////////////

        inline uint32_t filesize () const {
          return _filesize;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the number of reserve logfiles
////////////////////////////////////////////////////////////////////////////////

        inline uint32_t reserveLogfiles () const {
          return _reserveLogfiles;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief get the number of historic logfiles to keep
////////////////////////////////////////////////////////////////////////////////

        inline uint32_t historicLogfiles () const {
          return _historicLogfiles;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief return the slots manager
////////////////////////////////////////////////////////////////////////////////

        Slots* slots () {
          return _slots;
        }

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not it is currently allowed to create an additional 
/// logfile
////////////////////////////////////////////////////////////////////////////////

        bool logfileCreationAllowed (uint32_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not there are reserve logfiles
////////////////////////////////////////////////////////////////////////////////

        bool hasReserveLogfiles ();

////////////////////////////////////////////////////////////////////////////////
/// @brief signal that a sync operation is required
////////////////////////////////////////////////////////////////////////////////
        
        void signalSync ();

////////////////////////////////////////////////////////////////////////////////
/// @brief reserve space in a logfile
////////////////////////////////////////////////////////////////////////////////

        SlotInfo allocate (uint32_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief finalise a log entry
////////////////////////////////////////////////////////////////////////////////

        void finalise (SlotInfo&, bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief write data into the logfile
/// this is a convenience function that combines allocate, memcpy and finalise
////////////////////////////////////////////////////////////////////////////////

        int allocateAndWrite (void*,
                              uint32_t,
                              bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief re-inserts a logfile back into the inventory only
////////////////////////////////////////////////////////////////////////////////

        void relinkLogfile (Logfile*);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a logfile from the inventory only
////////////////////////////////////////////////////////////////////////////////

        bool unlinkLogfile (Logfile*);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a logfile from the inventory only
////////////////////////////////////////////////////////////////////////////////

        Logfile* unlinkLogfile (Logfile::IdType);

////////////////////////////////////////////////////////////////////////////////
/// @brief remove a logfile from the inventory and in the file system
////////////////////////////////////////////////////////////////////////////////

        void removeLogfile (Logfile*,
                            bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the status of a logfile to open
////////////////////////////////////////////////////////////////////////////////

        void setLogfileOpen (Logfile*);

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the status of a logfile to seal-requested
////////////////////////////////////////////////////////////////////////////////

        void setLogfileSealRequested (Logfile*);

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the status of a logfile to sealed
////////////////////////////////////////////////////////////////////////////////

        void setLogfileSealed (Logfile*);

////////////////////////////////////////////////////////////////////////////////
/// @brief sets the status of a logfile to sealed
////////////////////////////////////////////////////////////////////////////////

        void setLogfileSealed (Logfile::IdType);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the status of a logfile
////////////////////////////////////////////////////////////////////////////////

        Logfile::StatusType getLogfileStatus (Logfile::IdType);

////////////////////////////////////////////////////////////////////////////////
/// @brief return the file descriptor of a logfile
////////////////////////////////////////////////////////////////////////////////

        int getLogfileDescriptor (Logfile::IdType);

////////////////////////////////////////////////////////////////////////////////
/// @brief get a logfile for writing. this may return nullptr
////////////////////////////////////////////////////////////////////////////////

        Logfile* getWriteableLogfile (uint32_t,  
                                      Logfile::StatusType&);

////////////////////////////////////////////////////////////////////////////////
/// @brief get a logfile to collect. this may return nullptr
////////////////////////////////////////////////////////////////////////////////

        Logfile* getCollectableLogfile ();

////////////////////////////////////////////////////////////////////////////////
/// @brief get a logfile to remove. this may return nullptr
////////////////////////////////////////////////////////////////////////////////

        Logfile* getRemovableLogfile ();

////////////////////////////////////////////////////////////////////////////////
/// @brief mark a file as being requested for collection
////////////////////////////////////////////////////////////////////////////////

        void setCollectionRequested (Logfile*);

////////////////////////////////////////////////////////////////////////////////
/// @brief mark a file as being done with collection
////////////////////////////////////////////////////////////////////////////////
        
        void setCollectionDone (Logfile*);

// -----------------------------------------------------------------------------
// --SECTION--                                                   private methods
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief closes all logfiles
////////////////////////////////////////////////////////////////////////////////
  
        void closeLogfiles ();

////////////////////////////////////////////////////////////////////////////////
/// @brief returns the id of the last fully collected logfile
/// returns 0 if no logfile was yet collected or no information about the
/// collection is present
////////////////////////////////////////////////////////////////////////////////

        Logfile::IdType lastCollected ();

////////////////////////////////////////////////////////////////////////////////
/// @brief reads the shutdown information
////////////////////////////////////////////////////////////////////////////////

        int readShutdownInfo ();

////////////////////////////////////////////////////////////////////////////////
/// @brief writes the shutdown information
////////////////////////////////////////////////////////////////////////////////

        int writeShutdownInfo ();

////////////////////////////////////////////////////////////////////////////////
/// @brief start the synchroniser thread
////////////////////////////////////////////////////////////////////////////////

        int startSynchroniserThread ();

////////////////////////////////////////////////////////////////////////////////
/// @brief stop the synchroniser thread
////////////////////////////////////////////////////////////////////////////////

        void stopSynchroniserThread ();

////////////////////////////////////////////////////////////////////////////////
/// @brief start the allocator thread
////////////////////////////////////////////////////////////////////////////////

        int startAllocatorThread ();

////////////////////////////////////////////////////////////////////////////////
/// @brief stop the allocator thread
////////////////////////////////////////////////////////////////////////////////

        void stopAllocatorThread ();

////////////////////////////////////////////////////////////////////////////////
/// @brief start the collector thread
////////////////////////////////////////////////////////////////////////////////

        int startCollectorThread ();

////////////////////////////////////////////////////////////////////////////////
/// @brief stop the collector thread
////////////////////////////////////////////////////////////////////////////////

        void stopCollectorThread ();

////////////////////////////////////////////////////////////////////////////////
/// @brief check which logfiles are present in the log directory
////////////////////////////////////////////////////////////////////////////////

        int inventory ();

////////////////////////////////////////////////////////////////////////////////
/// @brief open the logfiles in the log directory
////////////////////////////////////////////////////////////////////////////////
        
        int openLogfiles (bool);

////////////////////////////////////////////////////////////////////////////////
/// @brief allocate a new reserve logfile
////////////////////////////////////////////////////////////////////////////////

        int createReserveLogfile (uint32_t);

////////////////////////////////////////////////////////////////////////////////
/// @brief get an id for the next logfile
////////////////////////////////////////////////////////////////////////////////
        
        Logfile::IdType nextId ();

////////////////////////////////////////////////////////////////////////////////
/// @brief ensure the wal logfiles directory is actually there
////////////////////////////////////////////////////////////////////////////////

        int ensureDirectory ();

////////////////////////////////////////////////////////////////////////////////
/// @brief return the absolute name of the shutdown file
////////////////////////////////////////////////////////////////////////////////

        std::string shutdownFilename () const;

////////////////////////////////////////////////////////////////////////////////
/// @brief return an absolute filename for a logfile id
////////////////////////////////////////////////////////////////////////////////

        std::string logfileName (Logfile::IdType) const;

// -----------------------------------------------------------------------------
// --SECTION--                                                 private variables
// -----------------------------------------------------------------------------

      private:

////////////////////////////////////////////////////////////////////////////////
/// @brief the arangod config variable containing the database path
////////////////////////////////////////////////////////////////////////////////

        std::string* _databasePath;

////////////////////////////////////////////////////////////////////////////////
/// @brief the logfile directory
////////////////////////////////////////////////////////////////////////////////

        std::string _directory;

////////////////////////////////////////////////////////////////////////////////
/// @brief the size of each logfile
////////////////////////////////////////////////////////////////////////////////

        uint32_t _filesize;

////////////////////////////////////////////////////////////////////////////////
/// @brief maximum number of reserve logfiles
////////////////////////////////////////////////////////////////////////////////

        uint32_t _reserveLogfiles;

////////////////////////////////////////////////////////////////////////////////
/// @brief maximum number of historic logfiles
////////////////////////////////////////////////////////////////////////////////

        uint32_t _historicLogfiles;

////////////////////////////////////////////////////////////////////////////////
/// @brief maximum number of parallel open logfiles
////////////////////////////////////////////////////////////////////////////////

        uint32_t _maxOpenLogfiles;

////////////////////////////////////////////////////////////////////////////////
/// @brief allow entries that are bigger than a single logfile
////////////////////////////////////////////////////////////////////////////////

        bool _allowOversizeEntries;

////////////////////////////////////////////////////////////////////////////////
/// @brief the slots manager
////////////////////////////////////////////////////////////////////////////////

        Slots* _slots;

////////////////////////////////////////////////////////////////////////////////
/// @brief the synchroniser thread
////////////////////////////////////////////////////////////////////////////////

        SynchroniserThread* _synchroniserThread;

////////////////////////////////////////////////////////////////////////////////
/// @brief the allocator thread
////////////////////////////////////////////////////////////////////////////////

        AllocatorThread* _allocatorThread;

////////////////////////////////////////////////////////////////////////////////
/// @brief the collector thread
////////////////////////////////////////////////////////////////////////////////

        CollectorThread* _collectorThread;

////////////////////////////////////////////////////////////////////////////////
/// @brief a lock protecting the _logfiles map, _lastCollectedId
////////////////////////////////////////////////////////////////////////////////

        basics::ReadWriteLock _logfilesLock;

////////////////////////////////////////////////////////////////////////////////
/// @brief last fully collected logfile id
////////////////////////////////////////////////////////////////////////////////

        Logfile::IdType _lastCollectedId;

////////////////////////////////////////////////////////////////////////////////
/// @brief the logfiles
////////////////////////////////////////////////////////////////////////////////

        std::map<Logfile::IdType, Logfile*> _logfiles;

////////////////////////////////////////////////////////////////////////////////
/// @brief regex to match logfiles
////////////////////////////////////////////////////////////////////////////////
  
        regex_t _regex;

////////////////////////////////////////////////////////////////////////////////
/// @brief whether or not we have been shut down already
////////////////////////////////////////////////////////////////////////////////

        volatile sig_atomic_t _shutdown;

////////////////////////////////////////////////////////////////////////////////
/// @brief minimum logfile size
////////////////////////////////////////////////////////////////////////////////

        static const uint32_t MinFilesize;
    };

  }
}

#endif

// Local Variables:
// mode: outline-minor
// outline-regexp: "/// @brief\\|/// {@inheritDoc}\\|/// @addtogroup\\|/// @page\\|// --SECTION--\\|/// @\\}"
// End:
