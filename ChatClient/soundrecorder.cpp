#include "soundrecorder.h"

#include "model/data.h"
#include <QDir>
#include <QApplication>
#include <QMediaDevices>


SoundRecorder::~SoundRecorder()
{
    // 清理播放数据源
    if(audioSource) audioSource->deleteLater();
    // 清理播放数据终端
    if(audioSink) audioSink->deleteLater();
}

SoundRecorder* SoundRecorder::instance = new SoundRecorder();
SoundRecorder *SoundRecorder::getInstance()
{
    return instance;
}

int SoundRecorder::getPcmContentDuration(const QByteArray &pcmContent)
{
    int sampleBit = 1;
    if(m_sampleFormat == QAudioFormat::UInt8){
        sampleBit = sizeof(uint8_t);
    } else if(m_sampleFormat == QAudioFormat::Int16){
        sampleBit = sizeof(int16_t);
    } else if(m_sampleFormat == QAudioFormat::Int32){
        sampleBit = sizeof(int32_t);
    }
    sampleBit *= 8;
    qDebug() << pcmContent.size() << "\t" << m_sampleRate <<"\t" << sampleBit/8 <<"\t" <<m_channelCount;
    return pcmContent.size() / ( m_sampleRate * (sampleBit/8) * m_channelCount );
}

void SoundRecorder::startRecord()
{
    // 开始录制
    soundFile.open(QIODevice::WriteOnly | QIODevice::Truncate);
    audioSource->start(&soundFile);
}

void SoundRecorder::stopRecord()
{
    // 停止录制
    audioSource->stop();
    soundFile.close();

    // 通知调用者, 录音完成
    emit this->soundRecordDone(RECORD_PATH);
}

void SoundRecorder::startPlay(const QByteArray &content)
{
    this->stopPlay();   //先关闭, 再播放
    //开始播放
    if(content.isEmpty()){
        // Toast::showMessage("数据加载中, 请稍后播放");
        LOG() << "数据加载中, 请稍后播放";
        return;
    }
    // 1.把数据写到临时文件中
    model::writeByteArrayToFile(PLAY_PATH, content);

    // 2.播放语音
    inputFile.setFileName(PLAY_PATH);
    inputFile.open(QIODevice::ReadOnly);
    audioSink->start(&inputFile);
}

void SoundRecorder::stopPlay()
{
    //停止播放
    audioSink->stop();
    inputFile.close();
    emit this->soundPlayDone();
}

SoundRecorder::SoundRecorder(QObject *parent)
    : QObject{parent}
{
    // 1.创建目录
    QDir soundRootPath(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    if(!soundRootPath.exists("sound")){
        if(!soundRootPath.mkdir("sound")){
            LOG() << "[错误] 创建音频存储目录失败, 程序崩溃";
            qApp->quit();
            return;
        }
    }

    // 2.初始化录制模块
    //      2.1设置录制结果存储路径
    soundFile.setFileName(RECORD_PATH);
    //      2.2设置录制的文件的格式
    QAudioFormat inputFormat;
    inputFormat.setSampleRate(m_sampleRate);   //设置采样率
    inputFormat.setChannelCount(m_channelCount);     //设置单通道
    inputFormat.setSampleFormat(m_sampleFormat);
    //      2.3设置录制音频的设备信息（麦克风）
    QAudioDevice info = QMediaDevices::defaultAudioInput();
    if(!info.isFormatSupported(inputFormat)){
        LOG() << "录制设备, 格式不支持!";
        return;
    }
    //      2.4设置音频源
    audioSource = new QAudioSource(inputFormat, this);
    connect(audioSource, &QAudioSource::stateChanged, this, [=](QtAudio::State state){
        if(state == QtAudio::StoppedState){
            // 录制完毕
            if(audioSource->error() != QAudio::NoError){
                LOG() << audioSource->error();
            }
        }
    });

    // 3.初始化播放模块
    //      3.1获取到默认的播放设备
    outputDevices = new QMediaDevices(this);
    outputDevice = outputDevices->defaultAudioOutput();
    //      3.2设置播放格式
    QAudioFormat outputFormat;  //确保和输入一致
    outputFormat.setSampleRate(m_sampleRate);
    outputFormat.setChannelCount(m_channelCount);
    outputFormat.setSampleFormat(m_sampleFormat);
    if(!outputDevice.isFormatSupported(outputFormat)){
        LOG() <<"播放设备, 格式不支持";
        return;
    }
    //      3.3设置下沉样本
    audioSink = new QAudioSink(outputDevice, outputFormat);
    connect(audioSink, &QAudioSink::stateChanged, this, [=](QtAudio::State state){
        if(state == QtAudio::IdleState){
            LOG() <<"IdleState";
            this->stopPlay();
            emit this->soundPlayDone();
        } else if(state == QAudio::ActiveState){
            LOG() << "ActiveState";
        } else if(state == QAudio::StoppedState){
            LOG() << "StoppedState";
            if(audioSink->error() != QtAudio::NoError){
                LOG() << audioSink->error();
            }
        }
    });
}
