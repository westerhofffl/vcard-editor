#include "VCard.h"

#include <QTextStream>

VCard::VCard(const QString& content)
{
   if (!content.isNull())
   {
      QString contentString(content);
      QTextStream textStream(&contentString);
      QString line;
      do {
          line = textStream.readLine();
          m_contentList.append(line);
      } while (!line.isNull());
   }
   else
   {
      m_contentList.append("BEGIN:VCARD");
      m_contentList.append("END:VCARD");
   }
}
QString VCard::getContent() const
{
   return m_contentList.join("\n");
}

int VCard::getTagCount() const
{
   return m_contentList.length();
}
QString VCard::getTag(int index) const
{
   if ((index >= 0) && (index < m_contentList.length()))
   {
      QString line = m_contentList.at(index);
      int colonIndex = line.indexOf(':');
      return line.left(colonIndex);
   }
   return QString();
}
QString VCard::getTagContent(int index) const
{
   if ((index >= 0) && (index < m_contentList.length()))
   {
      QString line = m_contentList.at(index);
      int colonIndex = line.indexOf(':');
      return line.mid(colonIndex + 1);
   }
   return QString();
}

QString VCard::getTagContent(const QString& tag) const
{
   return getTagContent(getTagIndex(tag));
}
int VCard::getTagIndex(const QString& tag) const
{
   for(int index = 0; index < getTagCount(); ++index)
   {
      if (getTag(index) == tag)
      {
         return index;
      }
   }
   return -1;
}

void VCard::updateTag(const QString& tag, const QString& tagContent)
{
   QString line = QString("%1:%2").arg(tag).arg(tagContent);
   int tagIndex = getTagIndex(tag);
   if ((tagIndex >= 0) && (tagIndex < m_contentList.length()))
   {
      m_contentList[tagIndex] = line;
   }
   else
   {
      m_contentList.append(line);
   }
}

void VCard::removeTag(int tagIndex)
{
   if ((tagIndex >= 0) && (tagIndex < m_contentList.length()))
   {
      m_contentList.removeAt(tagIndex);
   }
}

void VCard::removeTag(const QString& tag)
{
   int tagIndex = getTagIndex(tag);
   removeTag(tagIndex);
}
