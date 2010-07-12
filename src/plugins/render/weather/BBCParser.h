//
// This file is part of the Marble Desktop Globe.
//
// This program is free software licensed under the GNU LGPL. You can
// find a copy of this license in LICENSE.txt in the top directory of
// the source code.
//
// Copyright 2009      Bastian Holst <bastianholst@gmx.de>
//

#ifndef BBCPARSER_H
#define BBCPARSER_H

// Marble
#include "WeatherData.h"

// Qt
#include <QtCore/QHash>
#include <QtCore/QList>
#include <QtCore/QXmlStreamReader>

class QObject;

namespace Marble
{

class BBCWeatherItem;

class BBCParser : public QXmlStreamReader
{
public:
    BBCParser( QList<WeatherData> &list );
    ~BBCParser();

    bool read( const QByteArray &data );

private:

    void readUnknownElement();
    void readBBC();
    void readChannel();
    void readItem();
    void readDescription( WeatherData *data );
    void readTitle( WeatherData *data );
    void readPubDate( WeatherData *data );

    QList<WeatherData> *const m_list;

    static QHash<QString, WeatherData::WeatherCondition> setupDayConditions();
    static QHash<QString, WeatherData::WeatherCondition> setupNightConditions();
    static QHash<QString, WeatherData::WindDirection> setupWindDirections();
    static QHash<QString, WeatherData::PressureDevelopment> setupPressureDevelopments();
    static QHash<QString, WeatherData::Visibility> setupVisibilityStates();
    static QHash<QString, int> setupMonthNames();

    static const QHash<QString, WeatherData::WeatherCondition> dayConditions;
    static const QHash<QString, WeatherData::WeatherCondition> nightConditions;
    static const QHash<QString, WeatherData::WindDirection> windDirections;
    static const QHash<QString, WeatherData::PressureDevelopment> pressureDevelopments;
    static const QHash<QString, WeatherData::Visibility> visibilityStates;
    static const QHash<QString, int> monthNames;
};

} // Marble namespace

#endif // BBCPARSER_H
