#include "Project.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>

Project::Project(const QString &folderName) :
      m_folderName(folderName),
      m_status(NOT_STARTED)
{
   m_timer.setInterval(1);
   m_timer.setSingleShot(false);
   connect(&m_timer, SIGNAL(timeout()), SLOT(doScan()));
}

QString Project::getFolderName() const
{
   return m_folderName;
}

Project::Status Project::getStatus() const
{
   return m_status;
}

void Project::scan()
{
   if (m_status == PAUSED)
   {
      m_status = SCANNING;
   }
   m_timer.start();
}

void Project::pause()
{
   m_status = PAUSED;
   m_timer.stop();
}

void Project::rescan()
{
   m_status = NOT_STARTED;
   scan();
}

int Project::getFileCount() const
{
   return m_fileMd4List.size();
}

QString Project::getFileName(int index) const
{
   return m_fileNameList.at(index);
}
QString Project::getFileFolderName(int index) const
{
   return m_fileFolderNameList.at(index);
}
int Project::getFileSize(int index) const
{
   return m_fileSizeList.at(index);
}
QByteArray Project::getFileMd4(int index) const
{
   return m_fileMd4List.at(index);
}

int Project::getFileGroup(int index) const
{
   return m_fileGroupList.at(index);
}

QList<int> Project::getFolderFileList(const QList<int>& fileIndexList) const
{
   QSet<QString> folderNameSet;
   foreach(int fileIndex, fileIndexList)
   {
      folderNameSet.insert(m_fileFolderNameList.at(fileIndex));
   }
   QList<int> folderFileList;
   for(int fileIndex = 0; fileIndex < m_fileFolderNameList.size(); ++fileIndex)
   {
      if (folderNameSet.contains(m_fileFolderNameList[fileIndex]))
      {
         folderFileList.append(fileIndex);
      }
   }
   return folderFileList;
}

QList<int> Project::getFolderFileList(int fileIndex) const
{
   QList<int> fileIndexList;
   fileIndexList.append(fileIndex);
   return getFolderFileList(fileIndexList);
}

int Project::getGroupCount() const
{
   return m_groupMd4List.size();
}
QSet<int> Project::getGroupFileIndexSet(int index) const
{
   return m_folderFileHash.values(index).toSet();
}

QList<int> Project::getGroupList(const QList<int> &fileIndexList) const
{
   QSet<int> groupIndexSet;
   foreach(int fileIndex, fileIndexList)
   {
      groupIndexSet.insert(m_fileGroupList.at(fileIndex));
   }

   QList<int> groupIndexList = groupIndexSet.toList();
   qSort(groupIndexList);
   return groupIndexList;
}


void Project::addFile(const QString& fileName,  const QString& fileFolder,
      int fileSize, int fileCrc)
{

}

void Project::doScan()
{
   if (m_status == NOT_STARTED)
   {
      qWarning("scan start");
      emit progressStatus(-1, "Starting...");
      addFolderFiles(m_folderName);
      m_status = SCANNING;
   }

   int index = m_fileMd4List.size();
   if (index == m_fileNameList.size())
   {
      m_status = FINISHED;
      emit progressStatus(100, "Finished");
      return;
   }
   int size = m_fileSizeList.at(index);
   if (m_fileSizeIndexHash.count(size) == 1)
   {
      //uninteressant
      m_fileMd4List.append(QByteArray());
      int groupIndex = m_groupMd4List.size();
      m_fileGroupList.append(groupIndex);
      m_groupMd4List.append(QByteArray());
      m_folderFileHash.insertMulti(groupIndex, index);

      emit groupUpdated(groupIndex);
      qWarning("%i ist kein duplikat", index);
   }
   else
   {
      qWarning("%i ist ein duplikat", index);
      QFileInfo fileInfo(getFileFolderName(index), getFileName(index));
      QFile file(fileInfo.absoluteFilePath());
      if (!file.open(QIODevice::ReadOnly))
      {
         qWarning("nicht lesbar");
         m_fileNameList.removeAt(index);
         m_fileFolderNameList.removeAt(index);
         m_fileSizeList.removeAt(index);
      }
      else
      {
         QByteArray fileContent = file.readAll();
         QByteArray md4 = QCryptographicHash::hash(fileContent, QCryptographicHash::Md4);
         qWarning("gelesen %i, hash %i", fileContent.size(), md4.size());
         m_fileMd4List.append(md4);
         int groupIndex = m_groupMd4List.indexOf(md4);
         if (groupIndex == -1)
         {
            groupIndex = m_groupMd4List.size();
            m_groupMd4List.append(md4);
         }
         m_folderFileHash.insertMulti(groupIndex, index);
         m_fileGroupList.append(groupIndex);
         emit groupUpdated(groupIndex);

         int progress = 100 * (index + 1);
         progress = progress / m_fileNameList.size();
         emit progressStatus(progress, QString("Parsing %1").arg(getFileName(index)));
      }
   }
}

void Project::addFolderFiles(const QString& folderName)
{
   emit progressStatus(-1, QString("Scanning %1").arg(folderName));

   QFileInfo folderInfo(folderName);
   if (!folderInfo.isDir())
   {
      return;
   }

   QStringList subfolderList;
   QFileInfoList fileInfoList = QDir(folderInfo.absoluteFilePath()).entryInfoList(
         QDir::Dirs | QDir::Files | QDir::NoDotAndDotDot);
   foreach(const QFileInfo& fileInfo, fileInfoList)
   {
      if (fileInfo.isDir())
      {
         subfolderList.append(fileInfo.absoluteFilePath());
      }
      else
      if (fileInfo.isFile())
      {
         qWarning("New file: %s", fileInfo.fileName().toAscii().data());
         int index = m_fileNameList.size();
         m_fileNameList.append(fileInfo.fileName());
         m_fileFolderNameList.append(folderName);
         m_fileSizeList.append(fileInfo.size());
         emit fileUpdated(m_fileNameList.size() - 1);
         m_fileSizeIndexHash.insertMulti(fileInfo.size(), index);
      }
      QCoreApplication::processEvents();
   }
   foreach(const QString& folderName, subfolderList)
   {
      qWarning("New folder: %s", folderName.toAscii().data());
      addFolderFiles(folderName);
   }
}
