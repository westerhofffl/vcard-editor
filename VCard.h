#ifndef VCARD_H
#define VCARD_H

#include <QString>
#include <QStringList>

class VCard
{
public:
    VCard(const QString& content = QString());

    QString getContent() const;

    int getTagCount() const;
    QString getTag(int index) const;
    QString getTagContent(int index) const;
    QString getTagContent(const QString& tag) const;
    int getTagIndex(const QString& tag) const;

    void updateTag(const QString& tag, const QString& tagContent);
    void removeTag(int tagIndex);
    void removeTag(const QString& tag);

private:
   QStringList m_contentList;
};

#endif // VCARD_H
