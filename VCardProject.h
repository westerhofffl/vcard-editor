#ifndef VCARDPROJECT_H
#define VCARDPROJECT_H

class VCard;
class QFile;
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

    int getVCardCount() const;
    VCard getVCard(int index) const;

    void addVCard(const VCard& vCard);
    void updateVCard(int index, const VCard& vCard);
    void removeVCard(int index);

private:
    QString m_absoluteFilePath;
    QStringList m_vCardList;
};

#endif // VCARDPROJECT_H
