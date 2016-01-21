//
//  gbDB.h
//  Safecast
//
//  Created by Nicholas Dolezal on 2/3/14.
//  Copyright (c) 2014 The Momoko Ito Foundation. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <Accelerate/Accelerate.h>
#include "sqlite3.h"
#include "unistd.h"

#ifndef Geiger_Bot_gbDB_h
#define Geiger_Bot_gbDB_h

/*
#ifndef GB_SPRINTF
#define GB_SPRINTF(src, ...) ({                                     \
    char* dest = NULL;                                              \
    if (src != NULL)                                                \
    {                                                               \
        dest = alloca(1UL + snprintf(NULL, 0, src, __VA_ARGS__));   \
        snprintf(dest, 9000, src, __VA_ARGS__);                     \
    }                                                               \
    dest;                                                           \
})
#endif
*/

#if defined (__cplusplus)
extern "C" {
#endif


//                     ────┬──────────────────────────┬────
//              ───────────┤   COMMON - CREATE/OPEN   ├───────────
//                     ────┴──────────────────────────┴────

// ======================
// gbDB_CreateIfNeededDB:
// ======================
//
// For a database at dbPath, this function will create a new empty SQLite3
// database if it is able to write to the path and a file does not already
// exist.  Note this does not test if an already existing file is a valid
// database or not.
//
void gbDB_CreateIfNeededDB(const char* dbPath);


// =============================
// gbDB_PrepConn_DBPath_CString:
// =============================
//
// Convienience function to both open a SQLite3 database located at the filesystem
// path dbPath, and additionally compile a SQL query whose text is specified by
// selectSQL.
//
// A pointer to the newly-opened database connection is returned in "db", and
// a pointer to the compiled statement is returned in "stmt".
//
// It is the caller's responsibility to finalize the statement, and then close
// the DB connection when they are done using it to prevent resource leaks.
//
// Note that the database connection being opened is read-only.
//
// Returns true if no errors were thrown by SQLite.
//
bool gbDB_PrepConn_DBPath_CString(const char*    dbPath,
                                  const char*    selectSQL,
                                  sqlite3**      db,
                                  sqlite3_stmt** stmt);


// =============================
// gbDB_CloseDBConnAndQueryStmt:
// =============================
//
// Convienience function to both finalize a compiled SQLite3 statement and close
// an open connection to a database.
//
void gbDB_CloseDBConnAndQueryStmt(sqlite3*      database,
                                  sqlite3_stmt* queryStmt);





//                     ────┬──────────────────────────┬────
//              ───────────┤     COMMON - EXEC SQL    ├───────────
//                     ────┴──────────────────────────┴────

// =====================
// gbDB_ExecSQL_Generic:
// =====================
//
// Executes a SQL query via sqlite3_exec for a database located at the filesystem
// path dbPath.  Returns true if no errors were thrown by SQLite.
//
// If the database is busy, it will block the thread and sleep for up to 60
// seconds before timing out.
//
bool gbDB_ExecSQL_Generic(const char* dbPath,
                          const char* query);
    
bool gbDB_ExecSQL_Generic_ReusingDB(sqlite3*    database,
                                    const char* query);

// ====================
// gbDB_ExecSQL_Scalar:
// ====================
//
// Executes a SQL query via sqlite3_step for a database located at the filesystem
// path dbPath.
//
// Returns a 32-bit signed integer that is the first column of the last row
// returned by the query.  This is suitable for simple aggregate queries such as
// "SELECT COUNT(*) FROM...".
//
int gbDB_ExecSQL_Scalar_ReusingStmt(sqlite3_stmt* scalar_stmt);


// ==============================
// gbDB_ExecSQL_Scalar_ReusingDB:
// ==============================
//
// Executes a SQL query via sqlite3_step for an already open connection to the
// SQLite3 database specifies.
//
// Returns a 32-bit signed integer that is the first column of the last row
// returned by the query.  This is suitable for simple aggregate queries such as
// "SELECT COUNT(*) FROM...".
//
int gbDB_ExecSQL_Scalar_ReusingDB(sqlite3*    database,
                                  const char* query);

// ====================
// gbDB_ExecSQL_Scalar:
// ====================
//
// Executes a SQL query via sqlite3_step for a database located at the filesystem
// path dbPath.
//
// Returns a 32-bit signed integer that is the first column of the last row
// returned by the query.  This is suitable for simple aggregate queries such as
// "SELECT COUNT(*) FROM...".
//
int gbDB_ExecSQL_Scalar(const char* dbPath,
                        const char* query);


// =======================================
// gbDB_BeginOrCommitTransactionReusingDB:
// =======================================
//
// Convienience function to execute the query "BEGIN TRANSACTION;" or
// "COMMIT TRANSACTION;" for an existing open and valid connection to a
// database.  Using these transaction-block commands improves write performance.
//
// Returns true if no errors were thrown by SQLite.
//
bool gbDB_BeginOrCommitTransactionReusingDB(sqlite3*   database,
                                            const bool isBegin);







//                     ────┬──────────────────────────┬────
//              ───────────┤  METACOMMANDS / PRAGMAS  ├───────────
//                     ────┴──────────────────────────┴────

// =====================================
// gbDB_Meta_PRAGMA_mmap_size_ReusingDB:
// =====================================
//
// Convienience function to enable memory-mapped IO for an already open SQLite3
// database connection.
//
// Note: this must be done every time the DB is opened, provides a very minor
// increase in performance, but increases appearent memory usage.  For best
// results the DB should have a page size larger than the default; perhaps 32k.
//
bool gbDB_Meta_PRAGMA_mmap_size_ReusingDB(sqlite3* database);



// =======================================
// gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB:
// =======================================
//
// Convienience function to enable write-ahead log mode for an already open
// SQLite3 database connection.
//
bool gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB(sqlite3* database);


// =============================
// gbDB_Meta_PRAGMA_Journal_WAL:
// =============================
//
// Convienience function to enable write-ahead log mode for an SQLite3 database
// located at the file system path dbPath.
//
bool gbDB_Meta_PRAGMA_Journal_WAL(const char* dbPath);
    
bool gbDB_Meta_PRAGMA_Synchronous_OFF(const char* dbPath);
bool gbDB_Meta_PRAGMA_Synchronous_NORMAL(const char* dbPath);
bool gbDB_Meta_PRAGMA_Synchronous_FULL(const char* dbPath);


// ==========================================
// gbDB_Meta_PRAGMA_Journal_DELETE_ReusingDB:
// ==========================================
//
// Convienience function to enable the default delete log mode for an already
// open SQLite3 database connection.
//
bool gbDB_Meta_PRAGMA_Journal_DELETE_ReusingDB(sqlite3* database);


// ================================
// gbDB_Meta_PRAGMA_Journal_DELETE:
// ================================
//
// Convienience function to enable the default delete log mode for an SQLite3
// database located at the file system path dbPath.
//
bool gbDB_Meta_PRAGMA_Journal_DELETE(const char* dbPath);


// ===========================
// gbDB_Meta_VACUUM_ReusingDB:
// ===========================
//
// Convienience function to perform a VACUUM (defragment) for an already
// open SQLite3 database connection.
//
bool gbDB_Meta_VACUUM_ReusingDB(sqlite3* database);


// =================
// gbDB_Meta_VACUUM:
// =================
//
// Convienience function to perform a VACUUM (defragment) for an SQLite3
// database located at the file system path dbPath.
//
bool gbDB_Meta_VACUUM(const char* dbPath);

















//                     ────┬──────────────────────────┬────
//              ───────────┤  MULTITHREADING HELPERS  ├───────────
//                     ────┴──────────────────────────┴────

// ===============================
// gbDB_IncDecrementWithSemaphore:
// ===============================
//
// Terrible mutex boilerplate code for wrapping read, increment and decrement ops for x.
//
int gbDB_IncDecrementWithSemaphore(int*                 x,
                                   dispatch_semaphore_t sema,
                                   const bool           isInc,
                                   const bool           isDec);


// ==========================
// gbDB_WaitUntilQueueDoneDB:
// ==========================
//
// For a queue counter queue_n, protected by a single-unit semaphore sema_idx,
// this uses polling with a failsafe timeout to wait until the queue is finished
// executing.
//
// This is necessary if any aspect of the DB processing or writes is
// multithreaded, even with a serial queue, as the last work unit in the queue
// cannot be predicted.
//
void gbDB_WaitUntilQueueDoneDB(int* queue_n, dispatch_semaphore_t sema_idx);






//                     ────┬──────────────────────────┬────
//              ───────────┤  WRITE-AHEAD LOG HELPERS ├───────────
//                     ────┴──────────────────────────┴────

// ==============================
// gbDB_WAL_Checkpoint_ReusingDB:
// ==============================
//
// Convienience function to manually perform a Write-Ahead Log checkpoint
// for an open SQLite3 connection to a database in WAL mode.
//
// If the database is busy, this will block the current thread and sleep for
// up to 60 seconds.
//
// ATTENTION: All WAL-mode queries, either read or write, MUST be executed on
// the same thread.
//
bool gbDB_WAL_Checkpoint_ReusingDB(sqlite3* database);


// ====================
// gbDB_WAL_Checkpoint:
// ====================
//
// Convienience function to manually perform a Write-Ahead Log checkpoint
// for an SQLite3 database in WAL mode located at the file system path dbPath.
//
// If the database is busy, this will block the current thread and sleep for
// up to 60 seconds.
//
// ATTENTION: All WAL-mode queries, either read or write, MUST be executed on
// the same thread.
//
bool gbDB_WAL_Checkpoint(const char* dbPath);






//                     ────┬──────────────────────────┬────
//              ───────────┤  IMPLEMENTATION-SPECIFIC ├───────────
//                     ────┴──────────────────────────┴────

// =========================================
// gbDB_BuildQuadClustering_ORDERBY_SingleZ:
// =========================================
//
// A "shallow" variant of gbDB_BuildQuadClustering_ORDERBY_MultiZ, this function
// differs by doing the absolute minimum amount of data ordering useful only
// to generate a 50% scaled raster pyramid level from another.
//
// This may be useful over the MultiZ function because it executes more quickly,
// and in an environment with multithreaded processing either some or all of the
// data clustering will be lost anyway.
//
void gbDB_BuildQuadClustering_ORDERBY_SingleZ(const int   z,
                                              const char* x_col,
                                              const char* y_col,
                                              char**      dest);

// ===================================
// gbDB_GetSelectAllTilesForZ_CString:
// ===================================
//
// Convienience function for updating Safecast data.  This returns a heap-
// allocated C string which may be compiled to a SQLite 3 select statement.
//
// The query itself will select all tiles for zoom level z, and optionally
// order them by a recursive multidimensional spatial clustering similar to
// Microsoft's QuadKey, but with the caveat it will only work for a single
// zoom level.
//
// The clustering allows for novel high-performance downsampling of the tiles
// to the next raster pyramid zoom level by detecting when the tile's x or y
// coordinates change after being divided by two.  This is several orders of
// magnitude faster than using a brute force query method for the data's
// extent.
//
char* gbDB_GetSelectAllTilesForZ_CString(const int  z,
                                         const bool useQuadClusterOrderBy);


#if defined (__cplusplus)
}
#endif

#endif
