//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Andrew Manson <g.real.ate@gmail.com>
//

#include "KmlDocumentTagWriter.h"

#include "GeoDocument.h"
#include "GeoDataDocument.h"
#include "GeoWriter.h"
#include "KmlElementDictionary.h"
#include "GeoDataObject.h"

#include <QtCore/QDebug>

namespace Marble
{

static GeoTagWriterRegistrar s_writerDocument( GeoTagWriter::QualifiedName(GeoDataTypes::GeoDataDocumentType,
                                                                            kml::kmlTag_nameSpace22),
                                               new KmlDocumentTagWriter() );

bool KmlDocumentTagWriter::write( const GeoDataObject &node, GeoWriter& writer ) const
{
    const GeoDataDocument &document = static_cast<const GeoDataDocument&>(node);

    writer.writeStartElement( kml::kmlTag_Document );

    //Write the actual important stuff!
    writer.writeEndElement();
    return true;
}

}
