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
   int offset = 0;
   while(cardRegExp.indexIn(content, offset, QRegExp::CaretAtOffset) != -1)
   {
      QString cardString = cardRegExp.cap();
      m_vCardList.append(cardString);
      offset += cardString.length();
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

int VCardProject::getVCardCount() const
{
   return m_vCardList.length();
}
VCard VCardProject::getVCard(int index) const
{
   QString vCardContent(m_vCardList.at(index));
   VCard vcard(vCardContent);
   return vcard;
}

void VCardProject::addVCard(const VCard& vCard)
{
   m_vCardList.append(vCard.getContent());
}

void VCardProject::updateVCard(int index, const VCard& vCard)
{
   m_vCardList[index] = vCard.getContent();
}

void VCardProject::removeVCard(int index)
{
   m_vCardList.removeAt(index);
}
