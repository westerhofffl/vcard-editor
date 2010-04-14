#include "VCardProject.h"

#include "VCard.h"
#include <QFile>
#include <QString>
#include <QRegExp>

VCardProject::VCardProject(QFile& file)
{
   m_absoluteFilePath = file.fileName();
   file.open(QIODevice::ReadOnly);
   QString content = QString::fromLatin1(file.readAll());

   QRegExp cardRegExp("(BEGIN:VCARD.*END:VCARD)");
   cardRegExp.setCaseSensitivity(Qt::CaseInsensitive);
   cardRegExp.setMinimal(true);
   int offset = cardRegExp.indexIn(content, offset, QRegExp::CaretAtOffset);
   int id = 1;
   while(offset != -1)
   {
      QString cardString = cardRegExp.cap();
      m_vCardContentMap[id]= cardString;
      offset = cardRegExp.indexIn(content, offset + cardString.length(), QRegExp::CaretAtOffset);
      ++id;
   }
}

void VCardProject::saveTo(QFile& file) const
{
    if (file.open(QIODevice::WriteOnly))
    {
        QStringList vCardStringList(m_vCardContentMap.values());
        file.write(vCardStringList.join("\n").toUtf8());
    }
}

QString VCardProject::getAbsoluteFilePath() const
{
   return m_absoluteFilePath;
}
QString VCardProject::getFileName() const
{
   return m_absoluteFilePath.section('/', -1, -1);
}

QString VCardProject::getVersionAsString(Version version)
{
   switch(version)
   {
   case VER_2_1:
      return "2.1";
   case VER_3_0:
      return "3.0";
   default:
      return "unknown";
   }
}

QList<int> VCardProject::getVCardIdList() const
{
   QList<int> idList = m_vCardContentMap.keys();
   return idList;
}
VCard VCardProject::getVCard(int id) const
{
   QString vCardContent = m_vCardContentMap.value(id);
   VCard vcard(vCardContent);
   return vcard;
}

int VCardProject::addVCard(const VCard& vCard)
{
   int id = 1;
   QList<int> idList = getVCardIdList();
   if (!idList.isEmpty())
   {
       id = idList.last() + 1;
   }
   m_vCardContentMap.insert(id, vCard.getContent());
   return id;
}

void VCardProject::updateVCard(int id, const VCard& vCard)
{
   m_vCardContentMap[id] = vCard.getContent();
}

void VCardProject::removeVCard(int id)
{
   m_vCardContentMap.remove(id);
}
