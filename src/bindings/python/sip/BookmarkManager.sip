// Copyright 2011 Simon Edwards <simon@simonzone.com>

//                 Generated by twine2

// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU Library General Public License as
// published by the Free Software Foundation; either version 2, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details

// You should have received a copy of the GNU Library General Public
// License along with this program; if not, write to the
// Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

namespace Marble
{
class BookmarkManager : QObject
{
%TypeHeaderCode
#include <BookmarkManager.h>
%End

public:
    explicit                BookmarkManager (QObject* parent /TransferThis/ = 0);
    ~BookmarkManager ();
    bool                    loadFile (const QString& relativeFilePath);
    QString                 bookmarkFile () const;
    void                    addBookmark (const Marble::GeoDataPlacemark& bookmark, const QString& folderName);
//FIXME
//ig    QVector<Marble::GeoDataFolder*>  folders () const;
    void                    addNewBookmarkFolder (const QString& folder);
    void                    removeAllBookmarks ();
signals:
    void                    bookmarksChanged ();
};
};
