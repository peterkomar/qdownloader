#include "fileutils.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QDir>
#include <QSqlDatabase>
#include <QMessageBox>
#include <QDebug>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlError>
#include <QProgressBar>
#include <QSqlRecord>
#include <QImage>
#include <QPainter>

mainUtils::mainUtils()
{
  driverdb = "";
}

mainUtils::~mainUtils()
{
  if( driverdb != "" ) {
    QSqlDatabase::database("_db").close();
    QSqlDatabase::removeDatabase("_db");
  }
}

csv mainUtils::parseHtml(const QString& sfile )
{
  csv listUrl;

   QFile file( sfile );

   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return listUrl;
   }

   QTextStream in(&file);
   int i = 1;
   while (!in.atEnd()) {
     QString line = in.readLine();

     int pos2, pos1 = line.indexOf("<a href=\"");

     if( pos1 != -1 ) {
       pos2 = line.indexOf( ".rpm\">", pos1 );

       if( pos2 != -1 ) {
         CSV data;
         data.number = i++;
         data.sorce  = line.mid( pos1+9, pos2-pos1-5 );
         QFileInfo fifo(data.sorce);
         data.target = fifo.fileName();
         data.width = 0;
         data.height = 0;
         listUrl.append( data );
       }
     }
   }

  file.close();
  return listUrl;
}

csv mainUtils::parseDeb(const QString& sfile )
{
  csv listUrl;

   QFile file( sfile );

   if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return listUrl;
   }

   QTextStream in(&file);
   int i = 1;
   while (!in.atEnd()) {
     QString line = in.readLine();

     int pos = line.indexOf("Filename:");

     if( pos == 0 ) {

         QString package = line.mid(pos+9).trimmed();

         if( package == "" ) {
             continue;
         }

         CSV data;
         data.number = i++;
         data.sorce = package;
         data.width = 0;
         data.height = 0;
         data.target = package;

         listUrl.append(data);
     }
   }

  file.close();
  return listUrl;
}

QString mainUtils::getTargetPath( const QString& source )
{
  QFileInfo fifo(source);

  return fifo.fileName();
}

bool mainUtils::makePath( const QString& spath, QString& message )
{
  message = "";
  QFileInfo fifo( spath );
  QString path =  fifo.absolutePath();
  QDir tdir( path );
  if( !tdir.exists() ){
    if(!tdir.mkpath(path)) {
      message =QString("Problem make path '%s' "). arg(path);
      return false;
    }
  }

  return true;
}

csv mainUtils::parseCSV( const QString& fileName, const DownloadOptions& options )
{
  csv data_file;

  QFile file( fileName );

  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return data_file;
  }

  int pos = 1;
  QTextStream in(&file);
  while (!in.atEnd()) {
    QString line = in.readLine();

    CSV data;

    data.number = pos++;
    data.sorce  = line.section( options.s_delimiter, options.sourcePosition, options.sourcePosition);
    data.target = line.section( options.s_delimiter, options.targetPosition, options.targetPosition);

    data.width  = line.section( options.s_delimiter, options.widthPosition,options.widthPosition).toInt();
    data.height = line.section( options.s_delimiter, options.heightPosition, options.heightPosition).toInt();

    QString type = line.section( options.s_delimiter, options.statusPosition, options.statusPosition);

    if( options.bOpenOnlyFailed ) {
      if( type.indexOf("failed") != -1 ) {
        data_file.append(data);
      }
    } else {
       data_file.append(data);
    }
  }
   file.close();

   return data_file;
}

bool mainUtils::connectDB( const QString& host, const QString& user, const QString& pass, const QString& dbdriver, int port  )
{
  if( QSqlDatabase::database().databaseName() != "" )
  {

    QSqlDatabase::database("_db").close();
  }

  driverdb = dbdriver;

  QSqlDatabase::database("_db", false).close();
  QSqlDatabase::removeDatabase("_db");

  QSqlDatabase database = QSqlDatabase::addDatabase(driverdb, "_db");
  database.setHostName( host );
  database.setDatabaseName("");
  database.setPort( port );
  //
  if (!database.open(user, pass  )) {
    QMessageBox::critical(0, "SQL db",
              QObject::tr("Unable to establish a database connection.")+"\n"+
                  QObject::tr("QDownloader needs DB support. Please read "
                  "the Qt SQL driver documentation for information how "
                  "to build it."), QMessageBox::Cancel,
      QMessageBox::NoButton);

    return false;
  }

  return true;
}

QStringList mainUtils::getDataBasesList()
{
  QStringList list;

//  if( QSqlDatabase::database().databaseName() == "" )
//  {
//    return list;
//  }

  QSqlQuery query("SHOW DATABASES", QSqlDatabase::database("_db"));

  if(!query.exec())
  {
      qDebug() << "Failed to get databases list";
      return list;
  }

  while (query.next())
  {
    list += query.value(0).toString();
  }

  return list;
}

QStringList mainUtils::getTablesList( const QString& dbname )
{
  QSqlQuery query( QSqlDatabase::database("_db") );
  query.prepare("USE "+dbname );
  QStringList list;

  if(!query.exec())
  {
    qDebug() << "Query ( USE "+dbname +" ) error:";
    qDebug() << query.lastError().driverText();
    qDebug() << query.lastError().databaseText();
    return list;
  }

  query.prepare( "SHOW TABLES" );
  if(!query.exec())
  {
    qDebug() << "Query ( SHOW TABLES ) error:" + query.lastError().text();
    return list;
  }

  while (query.next())
  {
    list += query.value(0).toString();
  }

  return list;
}

QStringList mainUtils::getDBDrivers()
{
  QStringList drivers = QSqlDatabase::drivers();
  drivers.removeAll("QMYSQL3");
  drivers.removeAll("QOCI8");
  drivers.removeAll("QODBC3");
  drivers.removeAll("QPSQL7");
  drivers.removeAll("QTDS7");

  return drivers;
}

csv mainUtils::getFilesQueueFromDB( const QString& db, const QString& table, QProgressBar *bar, bool bExlude )
{
   csv data_file;
   QSqlQuery query( QSqlDatabase::database("_db") );

   query.prepare( "USE " + db );
   query.exec();

   QString distinct = "";
   if( bExlude ){
     distinct = " DISTINCT ";
   }

   QString countQuery = "SELECT count(*) FROM "+table;
   if( bExlude ){
     countQuery = "SELECT count( DISTINCT source, target) FROM "+table;
   }

   query.prepare(countQuery);

   if(!query.exec())
   {
     QMessageBox::critical(0,"Validator", "This is not valid cart2cart table or database");
     qDebug() << "Query error:" + query.lastError().text();
     return data_file;
   }

   bar->setVisible(true);
   query.next();
   bar->setMaximum(query.value(0).toInt());

   query.prepare( "SELECT "+ distinct +" source, target FROM " + table  +" WHERE status = 'failed'");
   if(!query.exec())
   {
     QMessageBox::critical(0,"Validator", "This is not valid cart2cart table or database");
     qDebug() << "Query error:" + query.lastError().text();
     return data_file;
   }

   int i = 0;
   while (query.next()) {
     bar->setValue( i );
     CSV data;
     data.number = i+1;
     data.sorce  = query.value(0).toString();
     data.target = query.value(1).toString();
     data_file.append(data);

     i++;
   }

   return data_file;
}

bool mainUtils::contains(CSV item, const csv& list)
{
  for( int i = 0; i< list.count(); i++ ) {
    CSV data = list.at(i);
    if( (data.sorce == item.sorce)
      && ( data.target == item.target) ) {
      return true;
    }
  }
  return false;
}

QString mainUtils::detectImageFormat( const QString& filename )
{
  QString format = "";

  if( filename.endsWith(".png", Qt::CaseInsensitive) ) {
      format = "PNG";
  } else if( filename.endsWith(".gif", Qt::CaseInsensitive) ){
      format = "GIF";
  } else if( filename.endsWith(".bmp", Qt::CaseInsensitive) ) {
      format = "BMP";
  } else if( filename.endsWith(".jpeg", Qt::CaseInsensitive)
    || filename.endsWith(".jpg", Qt::CaseInsensitive) ) {
      format = "JPG";
  }

  return format;
}

void mainUtils::convertToJPG( const QString& file )
{

   if( file.endsWith(".jpg" ) ) {
     return;
   }

   QString format = detectImageFormat( file );

   if( format == "" ) return;

   QImage image( file, format.toLatin1().data() );

   QFileInfo fifo( file );

   QString path = fifo.absolutePath ();
   path += "/"+fifo.baseName() + ".jpg";

   image.save( path, "JPG", 100 );

   QFile::remove( fifo.absoluteFilePath() );
}

void mainUtils::resizeImage( const QString& filename, int width, int height, const QString& method )
{
  if( width == 0 || height == 0 ) {
    return;
  }

  if( method == "normal" ) {
    normalMethod( filename, width, height );
    return;
  }

  if( method.indexOf("prestashop-") != -1 ) {
    QStringList meth = method.split("-");
    methodPrestahop(filename, width, height, meth.at(1).toInt());
  }
}

void mainUtils::normalMethod(const QString &filename, int width, int height)
{
  QString format = detectImageFormat( filename );
  if( format == "" ) return;

  QImage image( filename, format.toLatin1().data() );
  QImage rzImage = image.scaled( width,
                                 height,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation );

  //QFile::remove( filename );
  rzImage.save( filename, format.toLatin1().data(), 100 );
}

void mainUtils::methodPrestahop(const QString &filename, int destWidth, int destHeight, int method)
{
  QString format = detectImageFormat( filename );
  if( format == "" ) return;

  QImage sourceImage;

  if( !sourceImage.load(filename) ) {
     qDebug() << "Can't load image " + filename;
     return;
  }

  int sourceWidth  = sourceImage.width();
  int sourceHeight = sourceImage.height();

  if( !sourceWidth || !sourceHeight ) {
    qDebug() << "whith or height image is 0";
    return;
  }

  double widthDiff  = (double)(destWidth/sourceWidth);
  double heightDiff = (double)(destHeight/sourceHeight);

  int nextWidth, nextHeight;


  if ( widthDiff > 1.0 && heightDiff > 1.0 )
  {
    nextWidth = sourceWidth;
    nextHeight = sourceHeight;
  } else {
    if ( (method == 2) || ( (method == 0) && (widthDiff > heightDiff))) {
      nextHeight = destHeight;
      nextWidth  = (int)((sourceWidth * nextHeight) / sourceHeight);
      destWidth = ((method == 0 )? destWidth : nextWidth);
    } else {
      nextWidth = destWidth;
      nextHeight = (int)(sourceHeight * destWidth / sourceWidth);
      destHeight = (int)((method == 0)? destHeight : nextHeight);
    }
  }

  int borderWidth = (int)((destWidth - nextWidth) / 2);
  int borderHeight = (int)((destHeight - nextHeight) / 2);

  QImage *img = new QImage( destWidth, destHeight, QImage::Format_ARGB32_Premultiplied);
  img->fill(QColor(255, 255, 255, 0).rgba());
  QPainter p(img);
  p.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  QImage rzImage = sourceImage.scaled( nextWidth,
                                 nextHeight,
                                 Qt::IgnoreAspectRatio,
                                 Qt::SmoothTransformation );

  p.drawImage( borderWidth, borderHeight, rzImage, 0,0, nextWidth,nextHeight );
  p.end();

  //QFile::remove( filename );
  img->save( filename, format.toLatin1().data(), 100 );

  delete img;
}
