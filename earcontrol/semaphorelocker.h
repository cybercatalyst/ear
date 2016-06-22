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

#ifndef SEMAPHORELOCKER_H
#define SEMAPHORELOCKER_H

#include <QSemaphore>

/**
  * @class SemaphoreLocker
  * @author Jacob Dawid ( jacob@omg-it.works )
  * Helper class for locking semaphores in scopes.
  */
class SemaphoreLocker {
public:
    /** Constructs a locker, locks the given semaphore on construction. */
    SemaphoreLocker(QSemaphore *semaphore) {
        Q_ASSERT(semaphore != 0);
        m_semaphore = semaphore;
        if(m_semaphore)
            m_semaphore->acquire();
    }

    /** Destructor. Unlocks the semaphore. */
    ~SemaphoreLocker() {
        if(m_semaphore)
            m_semaphore->release();
    }
private:
    /** Semaphore that has been locked. */
    QSemaphore *m_semaphore;
};

#endif // SEMAPHORELOCKER_H

