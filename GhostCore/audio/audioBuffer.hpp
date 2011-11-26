#ifndef __AUDIO_INFO_HPP__
#define __AUDIO_INFO_HPP__

#include <QIODevice>
#include <QAudioFormat>

/*!
 * \brief
 * 録音用のQIODeviceの具体クラス
 * 
 */
class AudioBuffer : public QIODevice
{
	Q_OBJECT
public:
	AudioBuffer(const QAudioFormat &format, QObject *parent);
    ~AudioBuffer();

    void start();
    void stop();

	/**
	 * \brief
	 * バッファ全データを返す．
	 *
	 */
	double* buffer();

	/**
	 * \brief
	 * バッファサイズを返す
	 *
	 *
	 */
	unsigned int bufsize(){ return m_bufsize; }

	/**
	 * \brief
	 * リングバッファ内の書き込みフロント位置を返す
	 *
	 */
	unsigned int front() { return m_front; }

	/**
	 * \brief
	 * リングバッファバッファのフロント位置の録音開始からの通算位置を返す。
	 *
	 */
	unsigned long long offset() { return m_offset; }

	/**
	 * \brief
	 * 通算位置のミリ秒を返す
	 *
	 */
	double offset_msec() { return m_offset*1000/m_format.sampleRate(); }
	
	/**
	 * \brief
	 * 音声フォーマットを返す
	 *
	 */
	const QAudioFormat& format(){ return m_format; }

	/**
	 * \brief
	 * フォーマット上の最大振幅を返す
	 *
	 */
	quint16 maxAmplitude(){ return m_maxAmplitude; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
	//フォーマット
    const QAudioFormat m_format;
	//フォーマット上の最大値
    quint16 m_maxAmplitude;
	//区間最大値（未使用）
    double m_level;   //[0,1]
	//区間平均値（未使用）
	double m_average; //[0,1]
	//リングバッファ本体
	double *m_buffer;
	//バッファサイズ
	unsigned int m_bufsize;
	//フロント位置
	unsigned int m_front;

	//通算バッファ位置（最大13時間までカウントアップできる）
	/*
	bit	sampling rate	bps		byte per sec	ULONG_MAX	最大秒数	最大分数	最大時間数
	16	44100			705600	88200			4294967295	48695.77432	811.5962387	13.52660398
	*/
	unsigned long long m_offset;

	bool init;

signals:
    void update();
};

#endif