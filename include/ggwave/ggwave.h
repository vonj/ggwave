#pragma once

#include <cstdint>
#include <functional>
#include <vector>

class GGWave {
public:
    static constexpr auto kBaseSampleRate = 48000.0;
    static constexpr auto kMaxSamplesPerFrame = 1024;
    static constexpr auto kMaxDataBits = 256;
    static constexpr auto kMaxDataSize = 256;
    static constexpr auto kMaxLength = 140;
    static constexpr auto kMaxSpectrumHistory = 4;
    static constexpr auto kMaxRecordedFrames = 1024;

    struct TxProtocol {
        const char * name;

        int freqStart;
        int framesPerTx;
        int bytesPerTx;

        int nDataBitsPerTx() const { return 8*bytesPerTx; }
    };

    using TxProtocols     = std::vector<TxProtocol>;

    static const TxProtocols & getTxProtocols() {
        static TxProtocols kTxProtocols {
            { "Normal",      40,  9, 3, },
            { "Fast",        40,  6, 3, },
            { "Fastest",     40,  3, 3, },
            { "[U] Normal",  320, 9, 3, },
            { "[U] Fast",    320, 6, 3, },
            { "[U] Fastest", 320, 3, 3, },
        };

        return kTxProtocols;
    }

    using AmplitudeData   = std::vector<float>;
    using AmplitudeData16 = std::vector<int16_t>;
    using SpectrumData    = std::vector<float>;
    using RecordedData    = std::vector<float>;
    using TxRxData        = std::vector<std::uint8_t>;

    using CBQueueAudio = std::function<void(const void * data, uint32_t nBytes)>;
    using CBDequeueAudio = std::function<uint32_t(void * data, uint32_t nMaxBytes)>;

    GGWave(
            int sampleRateIn,
            int sampleRateOut,
            int samplesPerFrame,
            int sampleSizeBytesIn,
            int sampleSizeBytesOut);
    ~GGWave();

    bool init(int textLength, const char * stext, const TxProtocol & aProtocol, const int volume);
    bool send(const CBQueueAudio & cbQueueAudio);
    void receive(const CBDequeueAudio & CBDequeueAudio);

    const bool & hasTxData() const { return m_hasNewTxData; }
    const bool & isReceiving() const { return m_receivingData; }
    const bool & isAnalyzing() const { return m_analyzingData; }

    const int & getFramesToRecord()         const { return m_framesToRecord; }
    const int & getFramesLeftToRecord()     const { return m_framesLeftToRecord; }
    const int & getFramesToAnalyze()        const { return m_framesToAnalyze; }
    const int & getFramesLeftToAnalyze()    const { return m_framesLeftToAnalyze; }
    const int & getSamplesPerFrame()        const { return m_samplesPerFrame; }
    const int & getSampleSizeBytesIn()      const { return m_sampleSizeBytesIn; }
    const int & getSampleSizeBytesOut()     const { return m_sampleSizeBytesOut; }

    const float & getSampleRateIn() const { return m_sampleRateIn; }
    const float & getSampleRateOut() const { return m_sampleRateOut; }

    const TxProtocol & getDefultTxProtocol() const { return getTxProtocols()[1]; }

    const TxRxData & getRxData() const { return m_rxData; }
    const TxProtocol & getRxProtocol() const { return m_rxProtocol; }
    const int & getRxProtocolId() const { return m_rxProtocolId; }

    int takeRxData(TxRxData & dst);
    int takeTxAmplitudeData16(AmplitudeData16 & dst);
    bool takeSpectrum(SpectrumData & dst);

private:
    int maxFramesPerTx() const;
    int minBytesPerTx() const;

    double bitFreq(const TxProtocol & p, int bit) const {
        return m_hzPerSample*p.freqStart + m_freqDelta_hz*bit;
    }

    const float m_sampleRateIn;
    const float m_sampleRateOut;
    const int m_samplesPerFrame;
    const float m_isamplesPerFrame;
    const int m_sampleSizeBytesIn;
    const int m_sampleSizeBytesOut;

    const float m_hzPerSample;
    const float m_ihzPerSample;

    const int m_freqDelta_bin;
    const float m_freqDelta_hz;

    const int m_nBitsInMarker;
    const int m_nMarkerFrames;
    const int m_nPostMarkerFrames;
    const int m_encodedDataOffset;

    // Rx
    bool m_receivingData;
    bool m_analyzingData;

    int m_markerFreqStart;
    int m_recvDuration_frames;

    int m_framesLeftToAnalyze;
    int m_framesLeftToRecord;
    int m_framesToAnalyze;
    int m_framesToRecord;

    std::vector<float> m_fftIn;  // real
    std::vector<float> m_fftOut; // complex

    bool m_hasNewSpectrum;
    SpectrumData m_sampleSpectrum;
    AmplitudeData m_sampleAmplitude;

    bool m_hasNewRxData;
    int m_lastRxDataLength;
    TxRxData m_rxData;
    TxProtocol m_rxProtocol;
    int m_rxProtocolId;

    int m_historyId = 0;
    AmplitudeData m_sampleAmplitudeAverage;
    std::vector<AmplitudeData> m_sampleAmplitudeHistory;

    RecordedData m_recordedAmplitude;

    // Tx
    bool m_hasNewTxData;
    int m_nECCBytesPerTx;
    int m_sendDataLength;
    float m_sendVolume;

    int m_txDataLength;
    TxRxData m_txData;
    TxRxData m_txDataEncoded;

    TxProtocol m_txProtocol;

    AmplitudeData m_outputBlock;
    AmplitudeData16 m_outputBlock16;
    AmplitudeData16 m_txAmplitudeData16;
};
