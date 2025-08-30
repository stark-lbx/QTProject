#ifndef SOUNDRECORDER_H
#define SOUNDRECORDER_H

#include <QAudioSink>
#include <QAudioSource>
#include <QFile>
#include <QMediaDevices>
#include <QObject>
#include <QStandardPaths>
// 音频录制、音频播放
class SoundRecorder : public QObject
{
    Q_OBJECT

    ///////////////////////////////////////////////
    /// 录制语音
    ///////////////////////////////////////////////
public:
    void startRecord();// 开始录制
    void stopRecord();// 停止录制
private:
    QAudioSource* audioSource;
    QFile soundFile;

    ///////////////////////////////////////////////
    /// 录制语音
    ///////////////////////////////////////////////
public:
    void startPlay(const QByteArray& content);//开始播放
    void stopPlay();//停止播放
private:
    QMediaDevices *outputDevices;
    QAudioDevice outputDevice;
    QAudioSink *audioSink;          //AudioSink: 播放pcm文件的关键类
    QFile inputFile;

public:
    ~SoundRecorder();
    static SoundRecorder* getInstance();
    static int getPcmContentDuration(const QByteArray& pcmContent);
private:
    static SoundRecorder* instance;
    explicit SoundRecorder(QObject *parent = nullptr);

    // 通过录制代码, 会把录制结果, 直接保存到指定目录的文件中(api需要)
    const QString AppDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString RECORD_PATH = AppDataPath + "/sound/tmpRecord.pcm";
    const QString PLAY_PATH = AppDataPath + "/sound/tmpPlay.pcm";

    // 设置采样率、采样位数、声道数
    static const int m_sampleRate = 16000;
    static const int m_channelCount = 1;
    static const  QAudioFormat::SampleFormat m_sampleFormat = QAudioFormat::Int16;
signals:
    void soundRecordDone(const QString& path);// 录制完成后发送信号
    void soundPlayDone();   //语音播放完成后发送信号
};

#endif // SOUNDRECORDER_H
