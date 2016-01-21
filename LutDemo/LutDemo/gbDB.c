//
//  gbDB.c
//  Safecast
//
//  Created by Nicholas Dolezal on 2/3/14.
//  Copyright (c) 2014 The Momoko Ito Foundation. All rights reserved.
//

#include "gbDB.h"



void gbDB_CreateIfNeededDB(const char* dbPath)
{
    sqlite3* db = NULL;
    
    if (sqlite3_open_v2(dbPath, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) == SQLITE_OK)
    {
        sqlite3_close(db);
    }//if
}//gbDB_CreateIfNeededDB


int gbDB_ExecSQL_Scalar_ReusingStmt(sqlite3_stmt* scalar_stmt)
{
    int rowCount = 0;
    
    while(sqlite3_step(scalar_stmt) == SQLITE_ROW)
    {
        rowCount = sqlite3_column_int(scalar_stmt, 0);
    }//while
    
    sqlite3_clear_bindings(scalar_stmt);
    sqlite3_reset(scalar_stmt);
    
    return rowCount;
}//gbDB_ExecSQL_Scalar_ReusingStmt


int gbDB_ExecSQL_Scalar_ReusingDB(sqlite3* database,
                                  const char* query)
{
    int           rowCount          = 0;
    sqlite3_stmt *compiledStatement;
    
    if (sqlite3_prepare_v2(database, query, -1, &compiledStatement, NULL) == SQLITE_OK)
    {
        while(sqlite3_step(compiledStatement) == SQLITE_ROW)
        {
            rowCount = sqlite3_column_int(compiledStatement, 0);
        }//while
    }//if
    
    sqlite3_finalize(compiledStatement);
    
    return rowCount;
}//gbDB_ExecSQL_Scalar_ReusingDB


int gbDB_ExecSQL_Scalar(const char* dbPath,
                        const char* query)
{
    int retVal = -1;
    
    sqlite3 *database;
    
    //if (sqlite3_open(dbPath, &database) != SQLITE_OK)
    if (sqlite3_open_v2(dbPath, &database, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK)
    {
        printf("gbDB_ExecSQL_Scalar: abort: db not OK!\n");
    }//if
    else
    {
        retVal = gbDB_ExecSQL_Scalar_ReusingDB(database, query);
    }//else
    
    sqlite3_close(database);
    
    return retVal;
}//gbDB_ExecSQL_Scalar

// ===================================
// gbDB_GetReusableInsertQueryCString:
// ===================================
//
// Obsolete / unused.
//
/*
char* gbDB_GetReusableInsertQueryCString()
{
    return "INSERT INTO Tiles(TileX, TileY, TileLevel, DataCount, TileData) VALUES (?, ?, ?, ?, ?);";
}//gbDB_GetReusableInsertQueryCString()
*/

// =================================
// gbDB_BuildQuadClustering_SingleZ:
// =================================
//
// Used by gbDB_BuildQuadClustering_MultiZ.
//
// For a recursive quadclustering ORDER BY query, build a fragment of the query
// for a single zoom level as such:
//
// ((Y >> ZS) << Z) + (X >> ZS)
//
// Where:
//  Y  = y_col, name of the column containing the tile y coordinate
//  X  = x_col, name of the column containing the tile x coordinate
//  ZS = zoom level shift, equal to z_native minus z_dest
//  Z  = z_dest, the destination zoom level
//
// This must be executed for all zoom levels from z_native to 1.  The fragments
// should be assembled in the ORDER BY clause as comma separated components in
// ascending order per zoom level, such that the native zoom level is the final
// component in the clause.
//
void gbDB_BuildQuadClustering_SingleZ(const int   z_native,
                                      const int   z_dest,
                                      const char* x_col,
                                      const char* y_col,
                                      char**      dest)
{
    char temp[4096];
    
    const int z_shift = z_native - z_dest;
    
    if (z_shift > 0)
    {
        sprintf(temp, "((%s >> %d) << %d) + (%s >> %d)", y_col, z_shift, z_dest, x_col, z_shift);
    }//if
    else
    {
        sprintf(temp, "(%s << %d) + %s", y_col, z_dest, x_col);
    }//else
    
    size_t temp_bytes = strlen(temp);
    
    char* _dest = malloc(temp_bytes + 1UL);
    
    memcpy(_dest, temp, temp_bytes + 1UL);
    
    *dest = _dest;
}//gbDB_BuildQuadClustering_SingleZ


// ================================
// gbDB_BuildQuadClustering_MultiZ:
// ================================
//
// Used by gbDB_BuildQuadClustering_ORDERBY_MultiZ.
//
// This function returns the core of the ORDER BY clause, without the "ORDER BY"
// prefix or semicolon suffix.
//
void gbDB_BuildQuadClustering_MultiZ(const int   z_native,
                                     const char* x_col,
                                     const char* y_col,
                                     char**      dest)
{
    char   temp[4096];
    char*  _tempz = NULL;
    size_t idx    = 0;
    size_t _tempz_len;
    

    for (int i = 1; i <= z_native; i++)
    {
        gbDB_BuildQuadClustering_SingleZ(z_native, i, x_col, y_col, &_tempz);
        
        _tempz_len = strlen(_tempz);
        
        memcpy(temp + idx, _tempz, _tempz_len);
        
        idx += _tempz_len;
        
        if (i < z_native)
        {
            _tempz[idx++] = ',';
        }//if
        
        free(_tempz);
        _tempz = NULL;
    }//for
    
    char* _dest = malloc(idx + 1UL); // add null terminator to string at end
    
    memcpy(_dest, temp, idx);
    
    _dest[idx] = '\0';
    
    *dest = _dest;
}//gbDB_BuildQuadClustering_MultiZ


// ========================================
// gbDB_BuildQuadClustering_ORDERBY_MultiZ:
// ========================================
//
// Convienience function for updating Safecast data.  This returns a heap-
// allocated C string which can be used as the ORDER BY clause of a SQL query.
//
// Ordeirng is a recursive multidimensional spatial clustering similar to
// Microsoft's QuadKey, but with the caveat it will only work for a single
// zoom level.
//
// The clustering allows for novel high-performance downsampling of the tiles
// to the next raster pyramid zoom level by detecting when the tile's x or y
// coordinates change after being divided by two.  This is several orders of
// magnitude faster than using a brute force query method for the data's
// extent.
//
void gbDB_BuildQuadClustering_ORDERBY_MultiZ(const int   z,
                                             const char* x_col,
                                             const char* y_col,
                                             char**      dest)
{
    char  temp[4096];
    char* comps = NULL;
    
    gbDB_BuildQuadClustering_MultiZ(z, x_col, y_col, &comps);

    sprintf(temp, "ORDER BY %s;", comps);
    
    free(comps);
    comps = NULL;
    
    size_t temp_bytes = strlen(temp);
    
    char* _dest = malloc(temp_bytes + 1UL); // add null terminator to string at end
    
    memcpy(_dest, temp, temp_bytes + 1UL);
    
    *dest = _dest;
}//gbDB_BuildQuadClustering_ORDERBY_MultiZ



void gbDB_BuildQuadClustering_ORDERBY_SingleZ(const int   z,
                                              const char* x_col,
                                              const char* y_col,
                                              char**      dest)
{
    char  temp[4096];
    char* comps = NULL;
    char* _dest = NULL;
    
    gbDB_BuildQuadClustering_SingleZ(z, z, x_col, y_col, &comps);
    
    sprintf(temp, "ORDER BY %s;", comps);
    
    free(comps);
    comps = NULL;
    
    size_t temp_bytes = strlen(temp);
    
    _dest = malloc(temp_bytes + 1UL); // add null terminator to string at end
    
    memcpy(_dest, temp, temp_bytes + 1UL);
    
    *dest = _dest;
}//gbDB_BuildQuadClustering_ORDERBY_SingleZ




void gbDB_GetRecursiveQuadClustering_ORDERBY_forZoomLevel_CString(const int z, char** dest)
{
    gbDB_BuildQuadClustering_ORDERBY_MultiZ(z, "TileX", "TileY", dest);
    
    /*
    char retVal[4096];
    
    switch (z)
    {
        case 13:
        {
            const char* temp = "ORDER BY TileY/4096*2+TileX/4096,TileY/2048*4+TileX/2048,TileY/1024*8+TileX/1024,TileY/512*16+TileX/512,TileY/256*32+TileX/256,TileY/128*64+TileX/128,TileY/64*128+TileX/64,TileY/32*256+TileX/32,TileY/16*512+TileX/16,TileY/8*1024+TileX/8,TileY/4*2048+TileX/4,TileY/2*4096+TileX/2,TileY*8192+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 12:
        {
            const char* temp = "ORDER BY TileY/2048*2+TileX/2048,TileY/1024*4+TileX/1024,TileY/512*8+TileX/512,TileY/256*16+TileX/256,TileY/128*32+TileX/128,TileY/64*64+TileX/64,TileY/32*128+TileX/32,TileY/16*256+TileX/16,TileY/8*512+TileX/8,TileY/4*1024+TileX/4,TileY/2*2048+TileX/2,TileY*4096+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 11:
        {
            const char* temp = "ORDER BY TileY/1024*2+TileX/1024,TileY/512*4+TileX/512,TileY/256*8+TileX/256,TileY/128*16+TileX/128,TileY/64*32+ TileX/64,TileY/32*64+TileX/32,TileY/16*128+TileX/16,TileY/8*256+TileX/8,TileY/4*512+TileX/4,TileY/2*1024+TileX/2,TileY*2048+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 10:
        {
            const char* temp = "ORDER BY TileY/512*2+TileX/512,TileY/256*4+TileX/256,TileY/128*8+TileX/128,TileY/64*16+TileX/64,TileY/32*32+TileX/32,TileY/16*64+TileX/16,TileY/8*128+TileX/8,TileY/4*256+TileX/4,TileY/2*512+TileX/2,TileY*1024+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 9:
        {
            const char* temp = "ORDER BY TileY/256*2+TileX/256,TileY/128*4+TileX/128,TileY/64*8+TileX/64,TileY/32*16+TileX/32,TileY/16*32+TileX/16,TileY/8*64+TileX/8,TileY/4*128+TileX/4,TileY/2*256+TileX/2,TileY*512+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 8:
        {
            const char* temp = "ORDER BY TileY/128*2+TileX/128,TileY/64*4+TileX/64,TileY/32*8+TileX/32,TileY/16*16+TileX/16,TileY/8*32+TileX/8,TileY/4*64+TileX/4,TileY/2*128+TileX/2,TileY*256+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 7:
        {
            const char* temp = "ORDER BY TileY/64*2+TileX/64,TileY/32*4+TileX/32,TileY/16*8+TileX/16,TileY/8*16+TileX/8,TileY/4*32+TileX/4,TileY/2*64+TileX/2,TileY*128+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 6:
        {
            const char* temp = "ORDER BY TileY/32*2+TileX/32,TileY/16*4+TileX/16,TileY/8*8+TileX/8,TileY/4*16+TileX/4,TileY/2*32+TileX/2,TileY*64+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 5:
        {
            const char* temp = "ORDER BY TileY/16*2+TileX/16,TileY/8*4+TileX/8,TileY/4*8+TileX/4,TileY/2*16+TileX/2,TileY*32+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 4:
        {
            const char* temp = "ORDER BY TileY/8*2+TileX/8,TileY/4*4+TileX/4,TileY/2*8+TileX/2,TileY*16+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 3:
        {
            const char* temp = "ORDER BY TileY/4*2+TileX/4,TileY/2*4+TileX/2,TileY*8+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 2:
        {
            const char* temp = "ORDER BY TileY/2*2+TileX/2,TileY*4+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        case 1:
        {
            const char* temp = "ORDER BY TileY+TileX;";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
        default:
        {
            const char* temp = ";";
            
            sprintf(retVal, "%s", temp);
            
            break;
        }
    }//switch
    
    size_t src_bytes = strlen(retVal);
    
    char* _dest = malloc(src_bytes + 1UL);
    
    memcpy(_dest, retVal, src_bytes + 1UL);
    
    *dest = _dest;
     */
}//gbDB_GetRecursiveQuadClustering_ORDERBY_forTileLevel_CString



char* gbDB_GetSelectAllTilesForZ_CString(const int  z,
                                         const bool useQuadClusterOrderBy)
{
    char*  orderBy = NULL;
    char*  dest    = NULL;
    size_t len;
    
    if (useQuadClusterOrderBy)
    {
        // 2014-10-25 ND: *** TEST ***
        //                Disabling the "recursive" part, not sure if it helps
        //                because of the variable multithreaded write order.
        
        //gbDB_GetRecursiveQuadClustering_ORDERBY_forZoomLevel_CString(z, &orderBy);
        // ORDER BY Y/2 * (1 << (Z-1)) + X/2, Y * (1 << Z) + X;
        gbDB_BuildQuadClustering_ORDERBY_SingleZ(z-1, "(TileX>>1)", "(TileY>>1)", &orderBy);
    }//if
    else
    {
        orderBy    = malloc(16);
        orderBy[0] = ';';
        orderBy[1] = '\0';
    }//else

    len  = snprintf(NULL, 0, "SELECT TileX, TileY, DataCount, TileData FROM Tiles WHERE TileLevel = %d %s", z, orderBy);
    dest = malloc(1UL + len);
    
    sprintf(dest, "SELECT TileX, TileY, DataCount, TileData FROM Tiles WHERE TileLevel = %d %s", z, orderBy);
    
    free(orderBy);
    orderBy = NULL;
    
    return dest;
}//gbDB_GetSelectAllTilesForZ_CString




void gbDB_CloseDBConnAndQueryStmt(sqlite3*      database,
                                  sqlite3_stmt* queryStmt)
{
    sqlite3_finalize(queryStmt);
    sqlite3_close(database);
}//gbDB_CloseDBConnAndQueryStmt





bool gbDB_PrepConn_DBPath_CString(const char*    dbPath,
                                  const char*    selectSQL,
                                  sqlite3**      db,
                                  sqlite3_stmt** stmt)
{
    bool          retVal = false;
    sqlite3*      _db;
    sqlite3_stmt* _stmt = NULL;
    
    if (sqlite3_open_v2(dbPath, &_db, SQLITE_OPEN_READWRITE, NULL) == SQLITE_OK)
    {
        //printf("PrepConn_DBPath_CString: Opened DB: %s\n", dbPath);
        
        if (sqlite3_prepare_v2(_db, selectSQL , -1, &_stmt, NULL) == SQLITE_OK)
        {
            //printf("PrepConn_DBPath_CString: Prepared query: %s\n", selectSQL);
            retVal = true;
        }//if
        else
        {
            printf("gbDB.c: gbDB_PrepConn_DBPath_CString: Error preparing statement: %s\n", sqlite3_errmsg(_db));
            printf("gbDB.c: gbDB_PrepConn_DBPath_CString: SQL: %s\n", selectSQL);
            printf("gbDB.c: gbDB_PrepConn_DBPath_CString: dbPath: %s\n", dbPath);
            sqlite3_finalize(_stmt);
            sqlite3_close(_db);
        }//else
    }//if
    else
    {
        printf("gbDB.c: gbDB_PrepConn_DBPath_CString: Error opening DB: %s\n", sqlite3_errmsg(_db));
        printf("gbDB.c: gbDB_PrepConn_DBPath_CString: dbPath: %s\n", dbPath);
        sqlite3_close(_db);
    }//else
    
    *db   = _db;
    *stmt = _stmt;
    
    return retVal;
}//gbDB_PrepConn_DBPath_CString





bool gbDB_BeginOrCommitTransactionReusingDB(sqlite3*   database,
                                            const bool isBegin)
{
    bool          retVal    = true;
    char*         sqlTrans  = isBegin ? "BEGIN TRANSACTION;" : "COMMIT TRANSACTION;";
    sqlite3_stmt* transStmt;
    
    if (sqlite3_prepare_v2(database, sqlTrans, -1, &transStmt, NULL) == SQLITE_OK)
    {
        if (sqlite3_step(transStmt) != SQLITE_DONE)
        {
            printf("BeginOrCommitTransactionReusingDB: DB error executing %s: %s\n", sqlTrans, sqlite3_errmsg(database));
            retVal = false;
        }//if
        
        sqlite3_finalize(transStmt);
    }//if
    else
    {
        printf("BeginOrCommitTransactionReusingDB: DB error compiling %s: %s\n", sqlTrans, sqlite3_errmsg(database));
        retVal = false;
    }//else
    
    return retVal;
}//gbDB_BeginOrCommitTransactionReusingDB


// ===============================
// gbDB_ExecSQL_Generic_ReusingDB:
// ===============================
//
// Executes a SQL query via sqlite3_exec for a database with an already open
// connection.  Returns true if no errors were thrown by SQLite.
//
// If the database is busy, it will block the thread and sleep for up to 60
// seconds before timing out.
//
bool gbDB_ExecSQL_Generic_ReusingDB(sqlite3*    database,
                                    const char* query)
{
    bool retVal = true;
    int  status;
    
    status = sqlite3_exec(database, query, NULL, NULL, NULL);
    
    if (status != SQLITE_OK)
    {
        printf("gbDB_ExecSQL_Generic_ReusingDB: [ERR] [FIRST] Exec SQL failed, status: %d, msg: %s (query: %s)\n", status, sqlite3_errmsg(database), query);
        
        uint32_t        currentSleepMS  = 0;
        const uint32_t kSleepIntervalMS = 100;                      // 100ms
        const uint32_t kSleepIntervalUS = kSleepIntervalMS * 1000;  // 100ms -> us
        const uint32_t kMaxSleepMS      = 60 * 1000;                // 60s   -> ms
        
        while (status != SQLITE_OK)
        {
            if (          (status != SQLITE_BUSY
                        && status != SQLITE_LOCKED)
                || currentSleepMS >= kMaxSleepMS)
            {
                break;
            }//if
            
            status = sqlite3_exec(database, query, NULL, NULL, NULL);
            
            usleep(kSleepIntervalUS);
            currentSleepMS += kSleepIntervalMS;
        }//while
        
        if (currentSleepMS >= kMaxSleepMS)
        {
            printf("gbDB_ExecSQL_Generic_ReusingDB: [ERR] Timeout while DB was busy.\n");
        }//if
        
        if (status != SQLITE_OK)
        {
            printf("gbDB_ExecSQL_Generic_ReusingDB: [ERR] Exec SQL failed, status: %d, msg: %s (query: %s)\n", status, sqlite3_errmsg(database), query);
            retVal = false;;
        }//if
    }//if
    
    /*
    if (sqlite3_exec(database, query, NULL, NULL, NULL) != SQLITE_OK)
    {
        printf("gbDB_ExecSQL_Generic_ReusingDB: exec sql failed, msg: %s\n", sqlite3_errmsg(database));
        retVal = false;;
    }//if
    */
    
    return retVal;
}//gbDB_ExecSQL_Generic_ReusingDB



bool gbDB_ExecSQL_Generic(const char* dbPath,
                          const char* query)
{
    bool retVal = true;
    
    sqlite3 *database;
    
    //if (sqlite3_open(dbPath, &database) != SQLITE_OK)
    if (sqlite3_open_v2(dbPath, &database, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK)
    {
        printf("gbDB_ExecSQL_Generic: abort: db not OK!\n");
        retVal = false;
    }//if
    else
    {
        retVal = gbDB_ExecSQL_Generic_ReusingDB(database, query);
    }//else
    
    sqlite3_close(database);
    
    return retVal;
}//gbDB_ExecSQL_Generic










int gbDB_IncDecrementWithSemaphore(int*                 x,
                                   dispatch_semaphore_t sema,
                                   const bool           isInc,
                                   const bool           isDec)
{
    int retVal = -1;
    
    dispatch_semaphore_wait(sema, DISPATCH_TIME_FOREVER);
    
    if (isInc)
    {
        *x = *x + 1;
    }//if
    else if (isDec)
    {
        *x = *x - 1;
    }//else if
    
    retVal = *x;
    
    dispatch_semaphore_signal(sema);
    
    return retVal;
}//gbDB_IncDecrementWithSemaphore



void gbDB_WaitUntilQueueDoneDB(int* queue_n, dispatch_semaphore_t sema_idx)
{
    int polled_n = gbDB_IncDecrementWithSemaphore(queue_n, sema_idx, false, false);
    
    if (polled_n > 0)
    {
        printf("Waiting on queue: %d\n", (int)polled_n);
        
        uint32_t        currentSleepMS  = 0;
        const uint32_t kSleepIntervalMS = 100;                      // 100ms
        const uint32_t kSleepIntervalUS = kSleepIntervalMS * 1000;  // 100ms -> us
        const uint32_t kMaxSleepMS      = 60 * 1000;                // 60s   -> ms
        
        while (polled_n > 0)
        {
            if (polled_n <= 0 || currentSleepMS > kMaxSleepMS)
            {
                break;
            }//if
            else if (currentSleepMS % 10000 == 0)
            {
                printf("Waiting on queue: %d\n", (int)polled_n);
            }//else if
            
            polled_n = gbDB_IncDecrementWithSemaphore(queue_n, sema_idx, false, false);
            
            usleep(kSleepIntervalUS);
            currentSleepMS += kSleepIntervalMS;
        }//while
        
        if (currentSleepMS > kMaxSleepMS)
        {
            printf("[WARN] Timeout after %d ms while waiting on queue: %d\n", (int)kMaxSleepMS, polled_n);
        }//if
    }//if
}//gbDB_WaitUntilQueueDoneDB








bool gbDB_WAL_Checkpoint_ReusingDB(sqlite3* database)
{
    bool retVal = true;
    int  status;
    
    const int kSQLiteCheckpointMode = SQLITE_CHECKPOINT_PASSIVE;
    
    status = sqlite3_wal_checkpoint_v2(database, NULL, kSQLiteCheckpointMode, NULL, NULL);
    
    if (status != SQLITE_OK)
    {
        printf("gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB: [ERR] Checkpoint failed, msg: %s ... status: %d\n", sqlite3_errmsg(database), status);
        
        uint32_t        currentSleepMS  = 0;
        const uint32_t kSleepIntervalMS = 100;                      // 100ms
        const uint32_t kSleepIntervalUS = kSleepIntervalMS * 1000;  // 100ms -> us
        const uint32_t kMaxSleepMS      = 60 * 1000;                // 60s   -> ms

        
        while (status != SQLITE_OK)
        {
            if (          (status != SQLITE_BUSY
                        && status != SQLITE_LOCKED)
                || currentSleepMS >= kMaxSleepMS)
            {
                break;
            }//if
            
            status = sqlite3_wal_checkpoint_v2(database, NULL, kSQLiteCheckpointMode, NULL, NULL);
            
            usleep(kSleepIntervalUS);
            currentSleepMS += kSleepIntervalMS;
        }//while
        
        if (currentSleepMS >= kMaxSleepMS)
        {
            printf("gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB: [ERR] Timeout while DB was busy.\n");
        }//if
        
        if (status != SQLITE_OK)
        {
            printf("gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB: [ERR] Checkpoint failed, msg: %s ... status: %d\n", sqlite3_errmsg(database), status);
            retVal = false;;
        }//if
    }//if
    
    return retVal;
}//gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB



bool gbDB_WAL_Checkpoint(const char* dbPath)
{
    bool retVal = true;
    
    sqlite3 *database;
    
    //if (sqlite3_open(dbPath, &database) != SQLITE_OK)
    if (sqlite3_open_v2(dbPath, &database, SQLITE_OPEN_READWRITE, NULL) != SQLITE_OK)
    {
        printf("gbDB_WAL_Checkpoint: abort: db not OK!\n");
        retVal = false;
    }//if
    else
    {
        retVal = gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB(database);
    }//else
    
    sqlite3_close(database);
    
    return retVal;
}//gbDB_WAL_Checkpoint



bool gbDB_Meta_PRAGMA_mmap_size_ReusingDB(sqlite3* database)
{
    return gbDB_ExecSQL_Generic_ReusingDB(database, "PRAGMA mmap_size = 268435456;");
}//gbDB_Meta_PRAGMA_mmap_size_ReusingDB


// ===========================
// gbDB_Meta_PRAGMA_mmap_size:
// ===========================
//
// Likely a no-op, as the mmap size must be set every time a connection is
// opened to the database.
//
bool gbDB_Meta_PRAGMA_mmap_size(const char* dbPath)
{
    return gbDB_ExecSQL_Generic(dbPath, "PRAGMA mmap_size = 268435456;");
}//gbDB_Meta_PRAGMA_mmap_size



bool gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB(sqlite3* database)
{
    return gbDB_ExecSQL_Generic_ReusingDB(database, "PRAGMA journal_mode = WAL;");
}//gbDB_Meta_PRAGMA_Journal_WAL_ReusingDB



bool gbDB_Meta_PRAGMA_Journal_WAL(const char* dbPath)
{
    return gbDB_ExecSQL_Generic(dbPath, "PRAGMA journal_mode = WAL;");
}//gbDB_Meta_PRAGMA_Journal_WAL


bool gbDB_Meta_PRAGMA_Synchronous_OFF(const char* dbPath)
{
    return gbDB_ExecSQL_Generic(dbPath, "PRAGMA synchronous = OFF;");
}

bool gbDB_Meta_PRAGMA_Synchronous_NORMAL(const char* dbPath)
{
    return gbDB_ExecSQL_Generic(dbPath, "PRAGMA synchronous = NORMAL;");
}

bool gbDB_Meta_PRAGMA_Synchronous_FULL(const char* dbPath)
{
    return gbDB_ExecSQL_Generic(dbPath, "PRAGMA synchronous = FULL;");
}


bool gbDB_Meta_PRAGMA_Journal_DELETE_ReusingDB(sqlite3* database)
{
    return gbDB_ExecSQL_Generic_ReusingDB(database, "PRAGMA journal_mode = DELETE;");
}//gbDB_Meta_PRAGMA_Journal_DELETE_ReusingDB



bool gbDB_Meta_PRAGMA_Journal_DELETE(const char* dbPath)
{
    return gbDB_ExecSQL_Generic(dbPath, "PRAGMA journal_mode = DELETE;");
}//gbDB_Meta_PRAGMA_Journal_DELETE



bool gbDB_Meta_VACUUM_ReusingDB(sqlite3* database)
{
    return gbDB_ExecSQL_Generic_ReusingDB(database, "VACUUM;");
}//gbDB_Meta_VACUUM_ReusingDB


bool gbDB_Meta_VACUUM(const char* dbPath)
{
    return gbDB_ExecSQL_Generic(dbPath, "VACUUM;");
}//gbDB_Meta_VACUUM





// ================================
// gbDB_GetNextRowFromMassTileRead:
// ================================
//
// Unused / deprecated.
//
int gbDB_GetNextRowFromMassTileRead(sqlite3*      database,
                                    sqlite3_stmt* selStmt,
                                    void**        tileData,
                                    size_t*       tileDataBytes,
                                    int*          dataCount,
                                    uint32_t*     x,
                                    uint32_t*     y)
{
    int      sqliteStepResult = sqlite3_step(selStmt);
    uint32_t _x               = -1;
    uint32_t _y               = -1;
    int      _dataCount       = 0;
    void*    _tileData        = NULL;
    size_t   _tileDataBytes   = 0;
    
    // -- 1. read row with tile, store in temp vector --
    if (sqliteStepResult == SQLITE_ROW)
    {
        _x                    = (uint32_t)sqlite3_column_int(selStmt, 0);
        _y                    = (uint32_t)sqlite3_column_int(selStmt, 1);
        _dataCount            =           sqlite3_column_int(selStmt, 2);
        const void *blobBytes =          sqlite3_column_blob(selStmt, 3);
        int blobLen           =         sqlite3_column_bytes(selStmt, 3);
        
        if (blobLen > 0)
        {
            _tileDataBytes = blobLen;
            _tileData      = malloc(_tileDataBytes);
            
            memcpy(_tileData, blobBytes, _tileDataBytes);
        }//if
    }//if
    
    *tileData      = _tileData;
    *tileDataBytes = _tileDataBytes;
    *x             = _x;
    *y             = _y;
    *dataCount     = _dataCount;
    
    return sqliteStepResult;
}//gbDB_GetNextRowFromMassTileRead
