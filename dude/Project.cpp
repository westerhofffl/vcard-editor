#include "Project.h"

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>

Project::Project(const QString &folderName, const QString &duplicatesFolderName) :
        m_folderName(folderName),
        m_duplicatesFolderName(duplicatesFolderName),
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

QString Project::getDuplicatesFolderName() const
{
    return m_duplicatesFolderName;
}

Project::Status Project::getStatus() const
{
    return m_status;
}

void Project::scan()
{
    if (m_status == PAUSED)
    {
        qWarning("start scan");
        m_status = SCANNING;
    }
    m_timer.start();
}

void Project::pause()
{
    qWarning("pause scan");
    m_status = PAUSED;
    m_timer.stop();
}

void Project::rescan()
{
    qWarning("rescan");
    m_status = NOT_STARTED;
    scan();
}

int Project::getFileCount() const
{
    return m_fileDetailsList.size();
}

QString Project::getFileName(int index) const
{
    FileDetails* fileDetails = m_fileDetailsList[index];
    return fileDetails->getFileName();
}
QString Project::getFileFolderName(int index) const
{
    FileDetails* fileDetails = m_fileDetailsList[index];
    return fileDetails->getRelativePath();
}
QString Project::getFullFileFolderName(int index) const
{
    QString prefix = m_folderName;
    if (isFileMoved(index))
    {
        prefix = m_duplicatesFolderName;
    }
    return prefix + getFileFolderName(index);
}

int Project::getFileSize(int index) const
{
    FileDetails* fileDetails = m_fileDetailsList[index];
    return fileDetails->getSize();
}
QByteArray Project::getFileCheckSum(int index) const
{
    FileDetails* fileDetails = m_fileDetailsList[index];
    return fileDetails->getCheckSum();
}

int Project::getFileGroup(int index) const
{
    QByteArray checkSum = getFileCheckSum(index);
    return m_checkSumIndexHash.value(checkSum);
}

bool Project::isFileMoved(int index) const
{
    FileDetails* fileDetails = m_fileDetailsList[index];
    return fileDetails->isInBackupDir();
}
void Project::setFileMoved(int index, bool isMoved)
{
    FileDetails* fileDetails = m_fileDetailsList[index];
    bool wasMoved = fileDetails->isInBackupDir();
    if (isMoved == wasMoved)
    {
        qWarning("moved status for %s not changed", getFileName(index).toAscii().data());
        return;
    }

    QFileInfo sourceFileInfo(getFullFileFolderName(index), fileDetails->getFileName());
    fileDetails->setInBackupDir(isMoved);
    QFileInfo targetFileInfo(getFullFileFolderName(index), fileDetails->getFileName());
    QDir targetDir = targetFileInfo.dir();
    QStringList dirPathList = targetDir.absolutePath().split("/");
    for(int pathStep = 1; pathStep <= dirPathList.length(); ++pathStep)
    {
        QString dirPath = QStringList(dirPathList.mid(0, pathStep)).join("/");
        if (dirPath.isEmpty())
        {
            dirPath.append(QDir::rootPath());
        }
        QDir stepDir(dirPath);
        if (!stepDir.exists())
        {
            stepDir.cdUp();
            stepDir.mkdir(dirPathList[pathStep - 1]);
        }
    }
    QFile::rename(sourceFileInfo.absoluteFilePath(), targetFileInfo.absoluteFilePath());
    qWarning("moved status for %s changed to %i", getFileName(index).toAscii().data(), isMoved);
    emit groupUpdated(getFileGroup(index));
}

QString Project::getAbsoluteFilePath(int index) const
{
    QString folderName = getFullFileFolderName(index);
    QFileInfo fileInfo(folderName, getFileName(index));
    return fileInfo.absoluteFilePath();
}

QPixmap Project::getFilePixmap(int index) const
{
    QImage image(getAbsoluteFilePath(index));
    return QPixmap::fromImage(image);
}

QList<int> Project::getFolderFileList(const QList<int>& fileIndexList) const
{
    QSet<QString> folderNameSet;
    foreach(int fileIndex, fileIndexList)
    {
        FileDetails* fileDetails = m_fileDetailsList[fileIndex];
        folderNameSet.insert(fileDetails->getRelativePath());
    }
    QList<int> folderFileList;
    for(int fileIndex = 0; fileIndex < m_fileDetailsList.size(); ++fileIndex)
    {
        FileDetails* fileDetails = m_fileDetailsList[fileIndex];
        if (folderNameSet.contains(fileDetails->getRelativePath()))
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
    return m_checkSumList.size();
}
QSet<int> Project::getGroupFileIndexSet(int index) const
{
    return m_checkSumIndexFileIndexHash.values(index).toSet();
}

QSet<int> Project::getGroupFileIndexSet(const QSet<int>& groupIndexSet) const
{
    QSet<int> fileIndexSet;
    foreach(int groupIndex, groupIndexSet)
    {
        fileIndexSet += getGroupFileIndexSet(groupIndex);
    }
    return fileIndexSet;
}

QList<int> Project::getGroupList(const QList<int> &fileIndexList) const
{
    QSet<int> groupIndexSet;
    foreach(int fileIndex, fileIndexList)
    {
        FileDetails* fileDetails = m_fileDetailsList[fileIndex];
        groupIndexSet.insert(m_checkSumIndexHash.value(fileDetails->getCheckSum()));
    }

    QList<int> groupIndexList = groupIndexSet.toList();
    qSort(groupIndexList);
    return groupIndexList;
}

void Project::doScan()
{
    if (m_status == NOT_STARTED)
    {
        qWarning("scan init");
        emit progressStatus(-1, "Init...");
        addFolderFiles(m_folderName, false);
        addFolderFiles(m_duplicatesFolderName, true);
        m_status = SCANNING;
    }

    if (m_notParsedFilePathList.isEmpty())
    {
        qWarning("scan finished");
        m_status = FINISHED;
        emit progressStatus(100, "Finished");
        m_timer.stop();
        return;
    }

    QFileInfo fileInfo(m_notParsedFilePathList.takeFirst());
    QFile file(fileInfo.absoluteFilePath());
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning("%s nicht lesbar", fileInfo.absoluteFilePath().toAscii().data());
    }
    else
    {
        int fileDetailsCount = m_fileDetailsList.size();
        int progress = 100 * (fileDetailsCount + 1) /
                       (fileDetailsCount + m_notParsedFilePathList.size());
        emit progressStatus(progress, QString("Parsing %1").arg(fileInfo.fileName()));

        QByteArray fileContent = file.readAll();
        QByteArray checkSum = QCryptographicHash::hash(fileContent, QCryptographicHash::Md4);
        qWarning("%s read, size: %i", fileInfo.absoluteFilePath().toAscii().data(), fileContent.size());
        QString absolutePath = fileInfo.absolutePath();
        bool isInBackupDir = absolutePath.startsWith(m_duplicatesFolderName);
        QString relativePath;
        if (isInBackupDir)
        {
            relativePath = absolutePath.mid(m_duplicatesFolderName.length());
        }
        else
        {
            relativePath = absolutePath.mid(m_folderName.length());
        }
        FileDetails* fileDetails =
                new FileDetails(fileInfo.fileName(), relativePath,
                                fileInfo.size(), checkSum, isInBackupDir);
        int fileIndex = m_fileDetailsList.size();
        m_fileDetailsList.append(fileDetails);

        int checkSumIndex = m_checkSumIndexHash.value(checkSum, -1);
        if (checkSumIndex == -1)
        {
            checkSumIndex = m_checkSumList.size();
            m_checkSumList.append(checkSum);
            m_checkSumIndexHash.insert(checkSum, checkSumIndex);
        }
        m_checkSumIndexFileIndexHash.insertMulti(checkSumIndex, fileIndex);
        emit groupUpdated(checkSumIndex);
    }
}

void Project::addFolderFiles(const QString& folderName, bool isDuplicatesFolder)
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
            m_notParsedFilePathList.append(fileInfo.absoluteFilePath());
        }
        QCoreApplication::processEvents();
    }
    foreach(const QString& folderName, subfolderList)
    {
        qWarning("New folder: %s", folderName.toAscii().data());
        addFolderFiles(folderName, isDuplicatesFolder);
    }
}
