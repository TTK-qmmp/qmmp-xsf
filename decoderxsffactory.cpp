#include "decoderxsffactory.h"
#include "xsfhelper.h"
#include "decoder_xsf.h"

#include <QMessageBox>

bool DecoderXSFFactory::canDecode(QIODevice *input) const
{
    QFile *file = static_cast<QFile*>(input);
    if(!file)
    {
        return false;
    }

    XSFHelper helper(file->fileName());
    return helper.initialize();
}

DecoderProperties DecoderXSFFactory::properties() const
{
    DecoderProperties properties;
    properties.name = tr("XSF Plugin");
    properties.shortName = "xsf";
    properties.filters << "*.2sf" << "*.mini2sf";
    properties.filters << "*.gsf" << "*.minigsf";
    properties.filters << "*.usf" << "*.miniusf";
    properties.filters << "*.ncsf" << "*.minincsf";
    properties.filters << "*.snsf" << "*.minisnsf";
    properties.filters << "*.pcm" << "*.msu";
    properties.description = "Overload Audio File";
    properties.protocols << "file";
    properties.noInput = true;
    return properties;
}

Decoder *DecoderXSFFactory::create(const QString &path, QIODevice *input)
{
    Q_UNUSED(input);
    return new DecoderXSF(path);
}

QList<TrackInfo*> DecoderXSFFactory::createPlayList(const QString &path, TrackInfo::Parts parts, QStringList *)
{
    TrackInfo *info = new TrackInfo(path);
    if(parts == TrackInfo::Parts())
    {
        return QList<TrackInfo*>() << info;
    }

    XSFHelper helper(path);
    helper.metaMode(true);
    if(!helper.initialize())
    {
        delete info;
        return QList<TrackInfo*>();
    }

    if((parts & TrackInfo::MetaData) && helper.hasTags())
    {
        info->setValue(Qmmp::TITLE, helper.tag("title"));
        info->setValue(Qmmp::ARTIST, helper.tag("artist"));
        info->setValue(Qmmp::ALBUM, helper.tag("album"));
        info->setValue(Qmmp::GENRE, helper.tag("genre"));
        info->setValue(Qmmp::YEAR, helper.tag("year"));
    }

    if(parts & TrackInfo::Properties)
    {
        info->setValue(Qmmp::BITRATE, helper.bitrate());
        info->setValue(Qmmp::SAMPLERATE, helper.sampleRate());
        info->setValue(Qmmp::CHANNELS, helper.channels());
        info->setValue(Qmmp::BITS_PER_SAMPLE, helper.depth());
        info->setValue(Qmmp::FORMAT_NAME, "Overload XSF");
        info->setDuration(helper.totalTime());
    }
    return QList<TrackInfo*>() << info;
}

MetaDataModel* DecoderXSFFactory::createMetaDataModel(const QString &path, bool readOnly)
{
    Q_UNUSED(path);
    Q_UNUSED(readOnly);
    return nullptr;
}

void DecoderXSFFactory::showSettings(QWidget *parent)
{
    Q_UNUSED(parent);
}

void DecoderXSFFactory::showAbout(QWidget *parent)
{
    QMessageBox::about (parent, tr("About XSF Reader Plugin"),
                        tr("Qmmp XSF Reader Plugin")+"\n"+
                        tr("Written by: Greedysky <greedysky@163.com>"));
}

QString DecoderXSFFactory::translation() const
{
    return QString();
}

