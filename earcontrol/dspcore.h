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
#include <QMap>

#include "digitalequalizer.h"
#include "jnoise/jnoise.h"

class DSPCore :
    public QObject,
    public QtJack::Processor {
    Q_OBJECT
public:
    struct StereoAudioPort {
        QtJack::AudioPort left;
        QtJack::AudioPort right;
    };

    /** Operating modes. */
    enum OperationMode {
        CalibratingLatency,
        ProcessingAudio
    };

    /** Different kinds of signal sources. */
    enum SignalSource {
        ExternalSource,
        WhiteNoise,
        PinkNoise
    };

    /** Signal channels. */
    enum Channel {
        LeftChannel = 0,
        RightChannel = 1
    };

    DSPCore(QtJack::Client& client);
    QtJack::Client& client();

    StereoAudioPort registerStereoAudioInPort(QString name);
    StereoAudioPort registerStereoAudioOutPort(QString name);

    void process(int samples);

    StereoAudioPort portByName(QString name);

    /** Returns a pointer to the equalizer object for the left channel. */
    DigitalEqualizer *leftEqualizer();
    /** Returns a pointer to the equalizer object for the right channel. */
    DigitalEqualizer *rightEqualizer();

    /** Immediately sets the signal source to use. */
    void setSignalSource(SignalSource signalSource);
    /** Returns the the signal source that is used for processing. */
    SignalSource signalSource();

    /** Returns true, when the automatic adaption is active. */
    bool automaticAdaptionActive();
    /** Returns true, when the signal is being bypassed. */
    bool bypassActive();
    /** Returns the average microphone amplitude for the left channel. */
    int microphoneLevelLeft();
    /** Returns the average microphone amplitude for the right channel. */
    int microphoneLevelRight();
    /** Returns the average signal source amplitude for the left channel. */
    int signalSourceLevelLeft();
    /** Returns the average signal source amplitude for the right channel. */
    int signalSourceLevelRight();

    /**
     * Provides the latency for the left channel.
     *
     * @return Latency measured in number of samples.
     * @see EarProcessor::processCalibration(int samples)
     */
    int leftLatency() { return m_calibration.m_latency[0]; }

    /**
     * Provides the latency for the right channel.
     * @return Latency measured in number of samples.
     * @see EarProcessor::processCalibration(int samples)
     */
    int rightLatency() { return m_calibration.m_latency[1]; }

public slots:
    /** Resets the calibration. */
    void calibrate();

    /** Set mode to "Rectification". */
    void setModeToRectification();

    /** Activates/deactivates automatic adaption. */
    void setAutomaticAdaptionActive(bool on);

    /** Activates/deactivates bypassing. */
    void setBypassActive(bool on);

signals:
    /** This signal is emitted when the calibration finished. */
    void calibrationFinished();


private:
    DigitalEqualizer _leftEq, _rightEq;
    JNoise _noiseGenerator;

    QtJack::Client& _client;
    QMap<QString, StereoAudioPort> _stereoPorts;

    // ..
    /** Contains the current operation mode for this unit. */
    OperationMode m_mode;
    /** The current signal source. */
    SignalSource m_signalSource;

    /** Ensures thread-safety. */
    QSemaphore *m_levelSemaphore;
    /** Semaphore for multithreaded access to the signal source. */
    QSemaphore *m_signalSourceSemaphore;
    /** Semaphore for multithreaded access to the automatic adaption state member. */
    QSemaphore *m_automaticAdaptionSemaphore;
    /** Semaphore for multithreaded access to the bypass state member. */
    QSemaphore *m_bypassSemaphore;

    /** Automatic adaption state. */
    bool m_automaticAdaptionActive;
    /** Bypass state. */
    bool m_bypassActive;

    /** Latency buffer for the left channel reference input. */
    QList<double> m_leftLatencyBuffer;
    /** Latency buffer for the right channel reference input. */
    QList<double> m_rightLatencyBuffer;

    /** This struct contains attributes that refer
      * to the calibration process. */
    struct Calibration {
        /** This is true if we are waiting for a click. */
        bool m_waitingForClick;
        /** Latency for the left and right channel. */
        int m_latency[2];
        /** Offset in case we are waiting for more than
          * a sample buffer period. */
        int m_offset;
        /** All previous latency measures. */
        QVector<int> m_latencyMeasures[2];
        /** Weighted map that correlates the number of occurences
          * of a measure to the measure itself. */
        QMap<int, int> m_weightedMeasures[2];
    } m_calibration;

    /** Buffer for microphone input. This will be updated by fetchInputBuffers(). */
    fftw_complex m_microphoneBuffer[2][4096];

    /** Buffer for signal source input. This will be updated by fetchInputBuffers(). */
    fftw_complex m_signalSourceBuffer[2][4096];

    /** Buffer for signal output. The contents of this buffer will be written on
      * a call for writeOutputBuffers(). */
    fftw_complex m_outputBuffer[2][4096];

    /** Buffer for the left channel from microphone
      * in the frequency domain after dft. */
    fftw_complex m_leftMicrophoneFrequencyDomain[4096];

    /** Buffer for the right channel from microphone
      * in the frequency domain after dft. */
    fftw_complex m_rightMicrophoneFrequencyDomain[4096];

    /** Buffer for the left channel from delay buffer transformed
      * in the frequency domain after dft. */
    fftw_complex m_leftSignalSourceFrequencyDomain[4096];

    /** Buffer for the right channel from delay buffer transformed
      * in the frequency domain after dft. */
    fftw_complex m_rightSignalSourceFrequencyDomain[4096];

    /** Buffer for the left channel to compare
      * with the microphone feedback. */
    fftw_complex m_delayedLeftSignalSource[4096];

    /** Buffer for the right channel to compare
      * with the microphone feedback. */
    fftw_complex m_delayedRightSignalSource[4096];

    /** The average absolute amplitude for microphone input. */
    int m_microphoneLevel[2];

    /** The average absolute amplitude for the signal source input. */
    int m_signalSourceLevel[2];

    /** Fake left sample buffer for generating noise. */
    jack_default_audio_sample_t m_fakeSampleBufferLeft[4096];

    /** Fake right sample buffer for generating noise. */
    jack_default_audio_sample_t m_fakeSampleBufferRight[4096];

    /** This method will fetch all input buffers to be ready for processing. */
    void fetchInputBuffers(int samples);

    /** Usually called at the end of audio processing, copies buffers back. */
    void writeOutputBuffers(int samples);

    /** Processes audio. */
    void processRectification(int samples);

    /** Processes calibration. */
    void processCalibration(int samples);
};

#endif // DSPCORE_H
