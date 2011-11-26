#ifndef __AUDIO_INFO_HPP__
#define __AUDIO_INFO_HPP__

#include <QIODevice>
#include <QAudioFormat>

/*!
 * \brief
 * �^���p��QIODevice�̋�̃N���X
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
	 * �o�b�t�@�S�f�[�^��Ԃ��D
	 *
	 */
	double* buffer();

	/**
	 * \brief
	 * �o�b�t�@�T�C�Y��Ԃ�
	 *
	 *
	 */
	unsigned int bufsize(){ return m_bufsize; }

	/**
	 * \brief
	 * �����O�o�b�t�@���̏������݃t�����g�ʒu��Ԃ�
	 *
	 */
	unsigned int front() { return m_front; }

	/**
	 * \brief
	 * �����O�o�b�t�@�o�b�t�@�̃t�����g�ʒu�̘^���J�n����̒ʎZ�ʒu��Ԃ��B
	 *
	 */
	unsigned long long offset() { return m_offset; }

	/**
	 * \brief
	 * �ʎZ�ʒu�̃~���b��Ԃ�
	 *
	 */
	double offset_msec() { return m_offset*1000/m_format.sampleRate(); }
	
	/**
	 * \brief
	 * �����t�H�[�}�b�g��Ԃ�
	 *
	 */
	const QAudioFormat& format(){ return m_format; }

	/**
	 * \brief
	 * �t�H�[�}�b�g��̍ő�U����Ԃ�
	 *
	 */
	quint16 maxAmplitude(){ return m_maxAmplitude; }

    qint64 readData(char *data, qint64 maxlen);
    qint64 writeData(const char *data, qint64 len);

private:
	//�t�H�[�}�b�g
    const QAudioFormat m_format;
	//�t�H�[�}�b�g��̍ő�l
    quint16 m_maxAmplitude;
	//��ԍő�l�i���g�p�j
    double m_level;   //[0,1]
	//��ԕ��ϒl�i���g�p�j
	double m_average; //[0,1]
	//�����O�o�b�t�@�{��
	double *m_buffer;
	//�o�b�t�@�T�C�Y
	unsigned int m_bufsize;
	//�t�����g�ʒu
	unsigned int m_front;

	//�ʎZ�o�b�t�@�ʒu�i�ő�13���Ԃ܂ŃJ�E���g�A�b�v�ł���j
	/*
	bit	sampling rate	bps		byte per sec	ULONG_MAX	�ő�b��	�ő啪��	�ő厞�Ԑ�
	16	44100			705600	88200			4294967295	48695.77432	811.5962387	13.52660398
	*/
	unsigned long long m_offset;

	bool init;

signals:
    void update();
};

#endif