#ifndef FILEUTILS_H
#define FILEUTILS_H

#include <QStringList>
#include <QList>

#include "prefdialog.h"

struct CSV
{
  QString target;
  QString sorce;
  int number;
  int width;
  int height;
};


class QProgressBar;

typedef QList<CSV> csv;

class mainUtils
{
public:
    mainUtils();
    ~mainUtils();

   static csv parseHtml(const QString& sfile );
   static csv parseDeb(const QString& sfile );
   static QString getTargetPath( const QString& source );
   static bool makePath( const QString& path, QString& message );
   static csv parseCSV( const QString& file, const DownloadOptions& options );
   void convertToJPG( const QString& file );
   void resizeImage( const QString& filename, int width, int height, const QString& method );

   //database functions
   bool connectDB( const QString& host, const QString& user, const QString& pass, const QString& dbdriver, int port = 3306  );
   QStringList getDataBasesList();
   QStringList getTablesList( const QString& dbname );
   QStringList getDBDrivers();
   csv getFilesQueueFromDB( const QString& db, const QString& table, QProgressBar* bar, bool exludeDublicates = false );

private:
   QString driverdb;

   bool contains(CSV item, const csv& list);

   //resize methods
   void methodPrestahop( const QString& filename, int width, int height, int method );
   void normalMethod( const QString& filename, int width, int height );
   QString detectImageFormat( const QString& filename );
};

#endif // FILEUTILS_H
