#include "VCard.h"

#include <QTextStream>

VCard::VCard(const QString& content)
{
   if (!content.isNull())
   {
      QString contentString(content);
      QTextStream textStream(&contentString);
      QString wholeLine;
      forever
      {
          QString singleLine = textStream.readLine();
          if (singleLine.isNull())
          {
              break;
          }
          wholeLine.append(singleLine);
          if (!wholeLine.endsWith('='))
          {
             wholeLine = wholeLine.trimmed();
             if (!wholeLine.isEmpty())
             {
                m_contentList.append(wholeLine);
             }
             wholeLine.clear();
          }
          else
          {
             wholeLine.remove(wholeLine.length() - 1, 1);
          }
      }
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

QString VCard::getSummary() const
{
    QList<int> nameIndexList = getTagIndexList("N");
    QStringList nameStringList;
    foreach(int nameIndex, nameIndexList)
    {
        nameStringList.append(getTagContent(nameIndex));
    }

    QList<int> fullNameIndexList = getTagIndexList("FN");
    QStringList fullNameStringList;
    foreach(int nameIndex, fullNameIndexList)
    {
        fullNameStringList.append(getTagContent(nameIndex));
    }

    QString summary = QString("%1 (%2)").arg(fullNameStringList.join("/")).arg(nameStringList.join("/"));
    return summary;
}

int VCard::getTagCount() const
{
   return m_contentList.length();
}
QString VCard::getCompleteTag(int index) const
{
   if ((index >= 0) && (index < m_contentList.length()))
   {
      QString line = m_contentList.at(index);
      QString completeTag = line.section(':', 0, 0);
      return completeTag;
   }
   return QString();
}
QString VCard::getTag(int index) const
{
    QString completeTag = getCompleteTag(index);
    return completeTag.section(';', 0, 0);
}
QStringList VCard::getTagProperties(int index) const
{
    QString completeTag = getCompleteTag(index);
    return completeTag.section(';', 1).split(';');
}

QString VCard::getTagContent(int index) const
{
   if ((index >= 0) && (index < m_contentList.length()))
   {
      QString line = m_contentList.at(index);
      QString content = line.section(':', 1);

      if (getTagProperties(index).contains("CHARSET=UTF-8", Qt::CaseInsensitive))
      {
          QRegExp charRegExp("=([0-9A-Fa-f]{2})");
          int offset = charRegExp.indexIn(content);
          while(offset != -1)
          {
              QString charCodeString = charRegExp.cap(1);
              int charCode = charCodeString.toInt(0, 16);
              content.replace(offset, 3, QChar(charCode));
              offset = charRegExp.indexIn(content, offset);
          }          
      }
      QByteArray asciiByteArray = content.toAscii();
      content = QString::fromUtf8(asciiByteArray.data(), asciiByteArray.size()).trimmed();

      return content;
   }
   return QString();
}

QList<int> VCard::getCompleteTagIndexList(const QString& completeTag) const
{
    QList<int> indexList;
   for(int index = 0; index < getTagCount(); ++index)
   {
      if (getCompleteTag(index) == completeTag)
      {
         indexList.append(index);
      }
   }
   return indexList;
}

QList<int> VCard::getTagIndexList(const QString& tag) const
{
    QList<int> indexList;
   for(int index = 0; index < getTagCount(); ++index)
   {
      if (getTag(index) == tag)
      {
         indexList.append(index);
      }
   }
   return indexList;
}


void VCard::insertTag(int index)
{
    m_contentList.insert(index, "");
}

void VCard::updateTag(int index, const QString& completeTag, const QString& tagContent)
{
    QString line = QString("%1:%2").arg(completeTag).arg(tagContent);
    if ((index >= 0) && (index < m_contentList.length()))
    {
       m_contentList[index] = line;
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
