#ifndef PROJECT_H
#define PROJECT_H

#include <QPixmap>
#include <QSet>
#include <QStringList>
#include <QTimer>

#include <QObject>

class Project : public QObject
{
    Q_OBJECT

public:
    Project(const QString& folderName,
            const QString& duplicatesFolderName);

    QString getFolderName() const;
    QString getDuplicatesFolderName() const;

    enum Status
    {
       NOT_STARTED,
       SCANNING,
       PAUSED,
       FINISHED
    };
    Status getStatus() const;
    void scan();
    void pause();
    void rescan();

    int getFileCount() const;
    QString getFileName(int index) const;
    QString getFileFolderName(int index) const;
    QString getFullFileFolderName(int index) const;
    int getFileSize(int index) const;
    QByteArray getFileCheckSum(int index) const;
    int getFileGroup(int index) const;
    bool isFileMoved(int index) const;
    void setFileMoved(int index, bool isMoved);
    QString getAbsoluteFilePath(int index) const;
    QPixmap getFilePixmap(int index) const;

    QList<int> getFolderFileList(const QList<int>& fileIndexList) const;
    QList<int> getFolderFileList(int fileIndex) const;

    int getGroupCount() const;
    QSet<int> getGroupFileIndexSet(int index) const;

    QList<int> getGroupList(const QList<int>& index) const;

private slots:
    void doScan();

signals:
   void progressStatus(int progress, QString text);
   void fileUpdated(int index);
   void groupUpdated(int index);

private:
   void addFolderFiles(const QString& folderName,
                       bool isDuplicatesFolder);

private:
    const QString m_folderName;
    const QString m_duplicatesFolderName;

    Status m_status;

    class FileDetails
    {
    public:
        FileDetails(const QString& fileName,
                    const QString& relativePath,
                    int size,
                    const QByteArray& md4,
                    bool isInBackupDir) :
            m_fileName(fileName),
            m_relativePath(relativePath),
            m_size(size),
            m_md4(md4),
            m_isInBackupDir(isInBackupDir)
        {
        }

        inline const QString& getFileName() const { return m_fileName; }
        inline const QString& getRelativePath() const { return m_relativePath; }
        inline int getSize() const { return m_size; }
        inline QByteArray getCheckSum() const { return m_md4; }
        inline bool isInBackupDir() const { return m_isInBackupDir; }
        inline void setInBackupDir(bool isInBackupDir) { m_isInBackupDir = isInBackupDir; }
    private:
        const QString m_fileName;
        const QString m_relativePath;
        const int m_size;
        const QByteArray m_md4;
        bool m_isInBackupDir;
    };

    QList<FileDetails*> m_fileDetailsList;

    QList<QByteArray> m_checkSumList;
    QHash<QByteArray, int> m_checkSumIndexHash;

    QMultiHash<int, int> m_checkSumIndexFileIndexHash;

    QStringList m_notParsedFilePathList;
    /*
    QStringList m_fileNameList;
    QStringList m_fileFolderNameList;
    QList<int> m_fileSizeList;
    QList<QByteArray> m_fileMd4List;
    QList<int> m_fileGroupList;
    QSet<int> m_movedFileIndexSet;

    QMultiHash<int, int> m_fileSizeIndexHash;
    QList<QByteArray> m_groupMd4List;
    QMultiHash<int, int> m_folderFileHash;
    */
    QTimer m_timer;
};

#endif // PROJECT_H
