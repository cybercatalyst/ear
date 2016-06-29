#ifndef EARFILTER_H
#define EARFILTER_H

#include "equalizer.h"
#include "fftwadapter.h"
#include "jnoise/jnoise.h"

#include <Processor>
#include <AudioPort>

#include <QMap>

class EARFilter :
    public QObject,
    public QtJack::Processor {
    Q_OBJECT

public:
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

    EARFilter(QString name, QtJack::AudioPort in, QtJack::AudioPort ref, QtJack::AudioPort out);

    void process(int samples);

    void setSignalSource(SignalSource signalSource);
    EARFilter::SignalSource signalSource();

    bool automaticAdaptionActive();
    bool bypassActive();

    /** Resets the calibration. */
    void startCalibration();

    /** Set mode to "Rectification". */
    void setModeToRectification();

    /** Activates/deactivates automatic adaption. */
    void setAutomaticAdaptionActive(bool on);

    /** Activates/deactivates bypassing. */
    void setBypassActive(bool on);

    /**
     * Provides the latency for the left channel.
     *
     * @return Latency measured in number of samples.
     * @see EarProcessor::processCalibration(int samples)
     */
    int latency() { return _calibration.m_latency; }

    Equalizer *equalizer();

    QString name();

signals:
    void calibrationStarted();
    void calibrationFinished();

    void referenceSignalLevelChanged(int level);
    void measuredSignalLevelChanged(int level);
    void outputSignalLevelChanged(int level);

private:
    QString _name;
    QtJack::AudioPort _in, _ref, _out;

    Equalizer _digitalEqualizer;
    JNoise _noiseGenerator;

    OperationMode _operationMode;
    SignalSource m_signalSource;

    QSemaphore *m_signalSourceSemaphore;
    QSemaphore *m_automaticAdaptionSemaphore;
    QSemaphore *m_bypassSemaphore;

    /** Automatic adaption state. */
    bool _adaptionActive;
    /** Bypass state. */
    bool _bypassActive;

    int _measuredSignalLevel;
    int _referenceSignalLevel;
    int _outputSignalLevel;

    /** Latency buffer for the reference input. */
    QList<double> _latencyBuffer;

    /** This struct contains attributes that refer
      * to the calibration process. */
    struct Calibration {
        /** This is true if we are waiting for a click. */
        bool m_waitingForClick;
        /** Latency for the left and right channel. */
        int m_latency;
        /** Offset in case we are waiting for more than
          * a sample buffer period. */
        int m_offset;
        /** All previous latency measures. */
        QVector<int> m_latencyMeasures;
        /** Weighted map that correlates the number of occurences
          * of a measure to the measure itself. */
        QMap<int, int> m_weightedMeasures;
    } _calibration;

    fftw_complex _measuredSignalBuffer[4096];
    fftw_complex _referenceSignalBuffer[4096];
    fftw_complex _outputSignalBuffer[4096];

    fftw_complex m_microphoneFrequencyDomain[4096];
    fftw_complex m_signalSourceFrequencyDomain[4096];
    fftw_complex m_delayedSignalSource[4096];

    jack_default_audio_sample_t _noiseBuffer[4096];

    /** This method will fetch all input buffers to be ready for processing. */
    void fetchInputBuffers(int samples);
    void writeOutputBuffers(int samples);

    void updateInputPeaks(int samples);
    void updateOutputPeaks(int samples);

    /** Processes audio. */
    void processRectification(int samples);
    /** Processes calibration. */
    void processCalibration(int samples);
};

#endif // EARFILTER_H
