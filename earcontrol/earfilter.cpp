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

#include <QDebug>

#include "earfilter.h"

#include "semaphorelocker.h"

#include <cmath>

EARFilter::EARFilter(
    QString name,
    QtJack::AudioPort in,
    QtJack::AudioPort ref,
    QtJack::AudioPort out) :
    QtJack::Processor(),
    _name(name),
    _in(in), _ref(ref), _out(out) {

    _adaptionActive = false;
    _operationMode = ProcessingAudio;
    m_signalSource = ExternalSource;


    _bypassActive = true;

    while(_latencyBuffer.size() < 44100)
        _latencyBuffer.append(0.0);

    _calibration.m_latency = 12000;

    m_signalSourceSemaphore = new QSemaphore(1);
    m_automaticAdaptionSemaphore = new QSemaphore(1);
    m_bypassSemaphore = new QSemaphore(1);
}

void EARFilter::process(int samples) {
    switch(_operationMode) {
    case CalibratingLatency:
        processCalibration(samples);
        break;
    case ProcessingAudio:
        processRectification(samples);
        break;
    };
}

Equalizer *EARFilter::equalizer() {
    return &_digitalEqualizer;
}

QString EARFilter::name() {
    return _name;
}

void EARFilter::setSignalSource(SignalSource signalSource) {
    SemaphoreLocker locker(m_signalSourceSemaphore);
    m_signalSource = signalSource;
}

EARFilter::SignalSource EARFilter::signalSource() {
    SemaphoreLocker locker(m_signalSourceSemaphore);
    return m_signalSource;
}

bool EARFilter::automaticAdaptionActive() {
    SemaphoreLocker locker(m_automaticAdaptionSemaphore);
    return _adaptionActive;
}

bool EARFilter::bypassActive() {
    SemaphoreLocker locker(m_bypassSemaphore);
    return _bypassActive;
}

void EARFilter::startCalibration() {
    _operationMode = CalibratingLatency;
    _calibration.m_waitingForClick = false;
    _calibration.m_latencyMeasures.clear();
    _calibration.m_weightedMeasures.clear();
    emit calibrationStarted();
}

void EARFilter::setModeToRectification() {
    _operationMode = ProcessingAudio;
    _calibration.m_waitingForClick = false;
    _calibration.m_latencyMeasures.clear();
    _calibration.m_weightedMeasures.clear();
}

void EARFilter::setAutomaticAdaptionActive(bool on) {
    SemaphoreLocker locker(m_automaticAdaptionSemaphore);
    _adaptionActive = on;
}

void EARFilter::setBypassActive(bool on) {
    SemaphoreLocker locker(m_bypassSemaphore);
    _bypassActive = on;
}

void EARFilter::fetchInputBuffers(int samples) {
    FFTWAdapter::blit(
        (jack_default_audio_sample_t*)_in.buffer(samples).internalMemory(),
        _measuredSignalBuffer,
        samples);

    switch(signalSource()) {
    case ExternalSource: {
        // When transferring music, read directly from JACK buffers.
        FFTWAdapter::blit(
            (jack_default_audio_sample_t*)_ref.buffer(samples).internalMemory(),
            _referenceSignalBuffer,
            samples
        );
        break;
        }
    case WhiteNoise: {
        _noiseGenerator.process(samples, _noiseBuffer, 0, 0, 0);
        FFTWAdapter::blit(_noiseBuffer, _referenceSignalBuffer, samples);
        break;
        }
    case PinkNoise: {
        _noiseGenerator.process(samples, 0, 0, _noiseBuffer, 0);
        FFTWAdapter::blit(_noiseBuffer, _referenceSignalBuffer, samples);
        break;
        }
    }

    updateInputPeaks(samples);
}

void EARFilter::writeOutputBuffers(int samples) {
    updateOutputPeaks(samples);

    // Write result into the output buffers.
    FFTWAdapter::blit(
        _outputSignalBuffer,
        (jack_default_audio_sample_t*)_out.buffer(samples).internalMemory(),
        samples
    );
}

void EARFilter::updateInputPeaks(int samples) {
    int measuredSignalLevel = 0, referenceSignalLevel = 0;
    double previous, current;

    for(int i = 0; i < samples; i++) {
        previous = measuredSignalLevel, current = abs(100 * _measuredSignalBuffer[i][0]);
        measuredSignalLevel = previous > current ? previous : current;
        previous = referenceSignalLevel, current = abs(100 * _referenceSignalBuffer[i][0]);
        referenceSignalLevel = previous > current ? previous : current;
    }

    double slow = 0.9;

    _measuredSignalLevel = slow * _measuredSignalLevel + (1 - slow) * measuredSignalLevel;
    _referenceSignalLevel = slow * _referenceSignalLevel + (1 - slow) * referenceSignalLevel;

    emit measuredSignalLevelChanged(_measuredSignalLevel);
    emit referenceSignalLevelChanged(_referenceSignalLevel);
}

void EARFilter::updateOutputPeaks(int samples) {
    int outputSignalLevel = 0;
    double previous, current;

    for(int i = 0; i < samples; i++) {
        previous = outputSignalLevel, current = abs(100 * _outputSignalBuffer[i][0]);
        outputSignalLevel = previous > current ? previous : current;
    }

    double slow = 0.9;

    _outputSignalLevel = slow * _outputSignalLevel + (1 - slow) * outputSignalLevel;
    emit outputSignalLevelChanged(_outputSignalLevel);
}

void EARFilter::processRectification(int samples) {
    fetchInputBuffers(samples);

    // Shift samples of the reference signals into the latency buffers.
    for(int i = 0; i < samples; i++) {
        _latencyBuffer.append(_referenceSignalBuffer[i][0]);
    }

    // Drops samples that are older than 44100 samples.
    while(_latencyBuffer.size() > 44100)
        _latencyBuffer.removeFirst();

    if(automaticAdaptionActive()) {
        // As soon as we have enogh samples accumulated, we can start to compare.
        if(_latencyBuffer.size() > (latency() - samples)) {
            // Extract delayed samples ready for a comparison.
            for(int i = 0; i < samples; i++) {
                m_delayedSignalSource[i][0]
                    = _latencyBuffer.at(_latencyBuffer.size() - latency() - samples + i);
                m_delayedSignalSource[i][1] = 0;
            }

            // Get the spectrum by performing the dft.
            FFTWAdapter::performFFT(_measuredSignalBuffer, m_microphoneFrequencyDomain, samples);
            FFTWAdapter::performFFT(m_delayedSignalSource, m_signalSourceFrequencyDomain, samples);

            // Gain exclusive access to equalizer controls.
            _digitalEqualizer.acquireControls();
            double *equalizerControls = _digitalEqualizer.controls();

            // Compare microphone signal and signal source (ie. music signal).
            for(int i = 0; i < 2048; i++) {
                double microphoneAmplitude
                    = sqrt(m_microphoneFrequencyDomain[i][0]
                         * m_microphoneFrequencyDomain[i][0]
                         + m_microphoneFrequencyDomain[i][1]
                         * m_microphoneFrequencyDomain[i][1]);
                double signalAmplitude
                    = sqrt(m_signalSourceFrequencyDomain[i][0]
                         * m_signalSourceFrequencyDomain[i][0]
                         + m_signalSourceFrequencyDomain[i][1]
                         * m_signalSourceFrequencyDomain[i][1]);

                equalizerControls[i]
                    += (signalAmplitude - microphoneAmplitude) / 2
                        / (double)((2049 - i));
            }

            // Average filter for smoothing the controls.
            for(int i = 1; i < 2047; i++) {
                equalizerControls[i] = (equalizerControls[i-1]
                                          + equalizerControls[i]
                                          + equalizerControls[i+1])
                                            / 3.0;
            }

            // Limit controls to 0.01 .. 1.0.
            for(int i = 0; i < 2048; i++) {
                if(equalizerControls[i] > 1.0)
                    equalizerControls[i] = 1.0;
                if(equalizerControls[i] < 0.01)
                    equalizerControls[i] = 0.01;
            }

            // We're done manipulating the controls, release them.
            _digitalEqualizer.releaseControls();
        }

        // We're done updating the controls, now generate a new filter.
        _digitalEqualizer.generateFilter();
    }

    if(!bypassActive()) {
        // Run signals through equalizers into the output buffers.
        _digitalEqualizer.process(_referenceSignalBuffer, _outputSignalBuffer, samples);
    } else {
        // Bypass equalizers and copy the signal source in the output buffers.
        FFTWAdapter::blit(_referenceSignalBuffer, _outputSignalBuffer, samples);
    }

    writeOutputBuffers(samples);
}

void EARFilter::processCalibration(int samples) {
    // The calibration process basically consists of two states:
    // 1.) Sending a signal
    // 2.) Waiting to receive the signal
    // The signal will be sent at the end of a buffer period, so that it could
    // be received - theoretically - in the next sample of the next period.

    // We are not waiting for a click, so we can prepare to send a new one.
    if(!_calibration.m_waitingForClick) {
        // Reset the offset. We need the offset in case that it takes more
        // samples to receive the click than we are processing right now.
        _calibration.m_offset = 0;

        // Generate new click. A click containes a single high sample at the
        // very end of the buffer period.
        for(int i = 0; i < samples; i++) {
            _outputSignalBuffer[i][0] = (i < (samples - 1)) ? 0 : 1.0;
            _outputSignalBuffer[i][1] = 0;
        }

        // Now let's indicate we are waiting for the click to return.
        _calibration.m_waitingForClick = true;
    } else {
        fetchInputBuffers(samples);

        // Find the first occurence of the maximum value.
        int maxLeft = 0;
        for(int i = 0; i < samples; i++) {
            if(_measuredSignalBuffer[i][0] > _measuredSignalBuffer[maxLeft][0])
                maxLeft = i;
        }

        // With maxLeft and maxRight we have two concrete candidates that may
        // be the incoming click. In order to not recognize noise as incoming
        // clicks, we use a threshold. Also, we suppose to have a minimum
        // latency to avoid echoing effects.
        double threshold = 0.8;
        int minLatency = 512;

        if(_measuredSignalBuffer[maxLeft][0] > threshold
        && (maxLeft + _calibration.m_offset > minLatency)) {
            // This is an incoming click. Add the offsets to get the absolute
            // latency, because maxLeft and maxRight only contain the samples
            // in this period.
            maxLeft  += _calibration.m_offset;

            // Append the result to the list of measures.
            _calibration.m_latencyMeasures.append(maxLeft);

            // Check if we have enough measures to determine the latency.
            if(_calibration.m_latencyMeasures.size() > 20) {
                // Set the mode to Running, we are done with measuring.
                _operationMode = ProcessingAudio;

                // Assort all measures in a weighted map. That way, we can
                // see how often a measured value appeared.
                foreach(int value, _calibration.m_latencyMeasures)
                    _calibration.m_weightedMeasures[value]++;

                // Now pick the measured value that appeared most. This is
                // more precise than just calculating the intermediate value,
                // because in practice the correct value appears multiple
                // times.
                int candidate = 0;
                QList<int> keysLeft = _calibration.m_weightedMeasures.keys();
                foreach(int key, keysLeft) {
                    if(_calibration.m_weightedMeasures[key]
                        > _calibration.m_weightedMeasures[candidate])
                        candidate = key;
                }

                // Pick the best hit and take it as the latency.
                _calibration.m_latency = candidate + 50;

                emit calibrationFinished();
            }

            // We are not waiting for the click anymore,
            // send out the next one.
            _calibration.m_waitingForClick = false;
        } else {
            // If we could not detect the click,
            // it may not be in this buffer period.
            _calibration.m_offset += samples;
        }

        // Generate silence, we don't want to send anything out right now.
        for(int i = 0; i < samples; i++) {
            _outputSignalBuffer[i][0] = 0;
            _outputSignalBuffer[i][1] = 0;
        }
    }

    writeOutputBuffers(samples);
}
