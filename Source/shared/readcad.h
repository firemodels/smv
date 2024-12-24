#ifndef READCAD_H_DEFINED
#define READCAD_H_DEFINED

#include "shared_structures.h"

/**
 * @brief Create and initialise an @ref cadgeom_collection.
 *
 * @param capacity The maximum capacity of this collection.
 *
 * @returns A @ref cadgeom_collection which has been properly initialized.
 */
cadgeom_collection *CreateCADGeomCollection(int capacity);

/**
 * @brief Initialise an already allocated cadgeom_collection. This is useful
 * when the collection is allocated as part of a larger data structure.
 *
 * @param coll
 * @param capacity
 * @return int
 */
int InitCADGeomCollection(cadgeom_collection *coll, int capacity);

/**
 * @brief Clear an @ref cadgeom_collection. This does not free the data
 * structure itself, but simply empties it.
 *
 * @param[inout] coll The @ref cadgeom_collection to clear.
 */
void ClearCADGeomCollection(cadgeom_collection *coll);

/**
 * @brief Free a @ref cadgeom_collection that was previously allocated with
 * NewMemory or created by @ref CreateCADGeomCollection.
 *
 * @param[inout] coll The @ref cadgeom_collection to free.
 */
void FreeCADGeomCollection(cadgeom_collection *coll);

/**
 * @brief Read CAD geometry data and add it to a collection.
 *
 * @param coll A pointer to the cadgeom_collection, this must have been created
 * with CreateCADGeomCollection.
 * @param file A path to the file to read.
 * @param block_shininess The block shininess to apply.
 */
int ReadCADGeomToCollection(cadgeom_collection *coll, const char *file,
                            GLfloat block_shininess);

/**
 * @brief Return the number of CAD object definitions.
 *
 * @param coll The collection.
 * @return  Number of defined objects. If coll is NULL, return 0.
 */
int NCADGeom(cadgeom_collection *coll);

#endif
