#ifndef VCARDPROJECT_H
#define VCARDPROJECT_H

class VCard;
class QFile;
#include <QMap>
#include <QString>
#include <QStringList>

class VCardProject
{
public:
    enum Version
    {
        VER_2_1,
        VER_3_0
    };

    VCardProject(QFile& file);

    QString getAbsoluteFilePath() const;
    QString getFileName() const;

    Version getVersion() const;
    QString getVersionAsString() const;
    static QString getVersionAsString(Version version);

    QList<int> getVCardIdList() const;
    VCard getVCard(int id) const;

    int addVCard(const VCard& vCard);
    void updateVCard(int id, const VCard& vCard);
    void removeVCard(int id);

private:
    QString m_absoluteFilePath;
    QMap<int, QString> m_vCardContentMap;
};

#endif // VCARDPROJECT_H
