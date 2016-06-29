/* This file is part of EAR, an audio processing tool.
 *
 * Copyright (C) 2011-2016 Otto Ritter, Jacob Dawid
 * otto.ritter.or@googlemail.com
 * jacob@omg-it.works
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the Affero GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the Affero GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dspcore.h"
#include "semaphorelocker.h"

#include <cmath>

#include <QDebug>

DSPCore::DSPCore(QtJack::Client& client)
    : Processor(client),
      _client(client) {
    _earFiltersSemaphore = new QSemaphore(1);
}

QtJack::Client& DSPCore::client() {
    return _client;
}

void DSPCore::process(int samples) {
    SemaphoreLocker locker(_earFiltersSemaphore);
    foreach(EARFilter *earFilter, _earFilters) {
        earFilter->process(samples);
    }
}

EARFilter *DSPCore::addEARFilter() {
    QtJack::AudioPort in, out, ref;
    int num = _earFilters.count() + 1;
    EARFilter *filter = new EARFilter(
        QString("Channel %1").arg(num),
        in = _client.registerAudioInPort(QString("in_%1").arg(num)),
        ref = _client.registerAudioInPort(QString("ref_%1").arg(num)),
        out = _client.registerAudioOutPort(QString("out_%1").arg(num))
    );

    SemaphoreLocker locker(_earFiltersSemaphore);
    _earFilters.append(filter);

    _client.connect(_client.portByName(QString("system:capture_%1").arg(num)), in);
    _client.connect(out, _client.portByName(QString("system:playback_%1").arg(num)));
    return filter;
}

QList<EARFilter*> DSPCore::earFilters() {
    SemaphoreLocker locker(_earFiltersSemaphore);
    return _earFilters;
}

