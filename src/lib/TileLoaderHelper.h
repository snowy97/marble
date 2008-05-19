/**
 * This file is part of the Marble Desktop Globe.
 *
 * Copyright 2005-2007 Torsten Rahn <tackat@kde.org>"
 * Copyright 2007      Inge Wallin  <ingwa@kde.org>"
 * Copyright 2008      Patrick Spendrin <ps_ml@gmx.de>"
 * Copyright 2008      Jens-Michael Hoffmann <jensmh@gmx.de>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __MARBLE__TILELOADERHELPER_H
#define __MARBLE__TILELOADERHELPER_H

#include <QtCore/QString>
#include <QtCore/QUrl>

class GeoSceneTexture;

namespace TileLoaderHelper {
    /**
     * @brief Get the maximum number of tile rows for a given tile level.
     * @param levelZeroRows  the number of rows in level zero
     * @param level  the tile level
     * @return       the maximum number of rows that a map level was tiled into. 
     *               If the tile level number is invalid then "-1" gets 
     *               returned so this case of wrong input data can get caught 
     *               by the code which makes use of it.
     */
    int levelToRow( const int levelZeroRows, int level );

    /**
     * @brief Get the maximum number of tile columns for a given tile level.
     * @param levelZeroColumns  the number of columns in level zero
     * @param level  the tile level
     * @return       the maximum number of columns that a map level was tiled into. 
     *               If the tile level number is invalid then "-1" gets 
     *               returned so this case of wrong input data can get caught 
     *               by the code which makes use of it.
     */
    int levelToColumn( const int levelZeroColumns, int level );

    /**
     * @brief Get the tile level for the given maximum number of tile columns.
     * @param levelZeroRows  the number of rows in level zero
     * @param row    the maximum number of rows that a map level was tiled into.
     * @return       the corresponding tile level.
     *               If the number of rows is invalid then "-1" gets 
     *               returned so this case of wrong input data can get caught 
     *               by the code which makes use of it.
     */
    int rowToLevel( const int levelZeroRows, int row );

    /**
     * @brief Get the tile level for the given maximum number of tile columns.
     * @param levelZeroColumns  the number of columns in level zero
     * @param column the maximum number of columns that a map level was tiled into.
     * @return       the corresponding tile level.
     *               If the number of columns is invalid then "-1" gets 
     *               returned so this case of wrong input data can get caught 
     *               by the code which makes use of it.
     */
    int columnToLevel( const int levelZeroColumns, int column );

    /**
     * @brief Get the url for download of a tile.
     */
    QUrl downloadUrl( GeoSceneTexture *textureLayer, int zoomLevel, int x, int y );

    /**
     * @brief Get the relative file name of a tile.
     */
    QString relativeTileFileName( GeoSceneTexture *textureLayer, int zoomLevel, int x, int y );

    /**
     * @brief Get the theme string
     * @param theme points to the GeoSceneDocument which represents a .dgml file.
     * @return the old style theme string which used to be the argument for many methods,
     *         for example "maps/earth/srtm".
     */
    QString themeStr( GeoSceneTexture *textureLayer );
}

#endif // __MARBLE__TILELOADERHELPER_H
