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

#ifndef DSPCORE_H
#define DSPCORE_H

#include <Processor>
#include <QList>
#include <QSemaphore>

#include "equalizer.h"
#include "jnoise/jnoise.h"

#include "earfilter.h"
#include "semaphorelocker.h"

class DSPCore :
    public QObject,
    public QtJack::Processor {
    Q_OBJECT
public:
    DSPCore(QtJack::Client& client);
    QtJack::Client& client();

    void process(int samples);

    EARFilter *addEARFilter();

    QList<EARFilter*> earFilters();

private:
    QtJack::Client& _client;

    QList<EARFilter*> _earFilters;

    QSemaphore *_earFiltersSemaphore;
};

#endif // DSPCORE_H
