//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2010      Dennis Nienhüser <earthwings@gentoo.org>
//


#ifndef OSM_NOMINATIM_RUNNER_H
#define OSM_NOMINATIM_RUNNER_H

#include "MarbleAbstractRunner.h"

#include <QtCore/QString>
#include <QtNetwork/QHostInfo>

class QNetworkReply;
class QNetworkAccessManager;

namespace Marble
{

class OsmNominatimRunner : public MarbleAbstractRunner
{
    Q_OBJECT
public:
    explicit OsmNominatimRunner(QObject *parent = 0);

    ~OsmNominatimRunner();

    // Overriding MarbleAbstractRunner
    GeoDataFeature::GeoDataVisualCategory category() const;

    // Overriding QThread
    virtual void run();
    
private Q_SLOTS:
    // Http request with nominatim.openstreetmap.org done
    void handleHttpReply( QNetworkReply* );

    // No results (or an error)
    void returnNoResults();

private:
    QNetworkAccessManager* m_manager;
};

}

#endif // OSM_NOMINATIM_RUNNER_H