#include <QApplication>
#include <QMainWindow>
#include <QWidget>
#include <QDesktopWidget>
#include <QIcon>
#include <QVBoxLayout>
#include <QLabel>
#include <QTextStream>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QImage>
#include <QMenu>
#include <QMenuBar>
#include <QTimer>
#include <QMouseEvent>
#include <QFileDialog>


//Application: Simple Image Viewer
//By: Aliireeza Teymoorian
//Version: 1.0.1
//License: GPL v3
//Created By: QT 7.9.0
//November 2017

//TODO: Impelemtation of Image info and list image as seperated dialogboxes
//TODO; Implementation of fullscreen view
//TODO: Comment the entire code ;)
//TODO: Adding an About Dialog and Help Dialog
//TODO: Implementation of few image filters


class ContextMenuSample : public QWidget {
	public:
		ContextMenuSample(QWidget *parent = 0);
        
	private slots:
		void openimage();
		void nextimage();
		void pervimage();
		void firstimage();
		void lastimage();
		void slideshow();
		void showeachimage();
		void randomimage();
			

	private:
		int counter;
		int current;
		QLabel *image;
		QString address;
		QFileInfoList imagelist;

		QAction *quit;
		QAction *open;
		QAction *list;
		QAction *rand;
		QAction *info;
		QAction *full;
		QAction *next;
		QAction *perv;
		QAction *first;
		QAction *last;
		QAction *slider;
		
		QTimer *timer;
		QMenu *contextmenu;
		void contextMenuEvent(QContextMenuEvent *event) override;	
		void mouseReleaseEvent(QMouseEvent * event) override;
};


ContextMenuSample::ContextMenuSample(QWidget *parent): QWidget(parent) {
	timer = new QTimer(this);

	open = new QAction("&Open", this);
	open->setShortcut(tr("CTRL+O"));

	next = new QAction("&Next Image", this);
	next->setShortcut(tr("CTRL+N"));
	next->setEnabled(false);
  
	perv = new QAction("&Perv Image", this);
	perv->setShortcut(tr("CTRL+P"));
	perv->setEnabled(false);
  
	first = new QAction("&First Image", this);
	first->setShortcut(tr("CTRL+F"));
	first->setEnabled(false);	
	
	last = new QAction("&Last Image", this);
	last->setShortcut(tr("CTRL+L"));
	last->setEnabled(false);
	
	full = new QAction("Full &Screen", this);
	full->setShortcut(tr("CTRL+M"));
	full->setCheckable(true);
	full->setChecked(false);
	full->setEnabled(false);
	
	list = new QAction("Lis&t Images", this);
	list->setShortcut(tr("CTRL+T"));
	list->setEnabled(false);
	
	info = new QAction("&Image Information", this);
	info->setShortcut(tr("CTRL+I"));
	info->setEnabled(false);
	
	rand = new QAction("&Random Image", this);
	rand->setShortcut(tr("CTRL+R"));
	rand->setEnabled(false);
	
	slider = new QAction("&Slide S&how", this);
	slider->setShortcut(tr("CTRL+W"));
	slider->setCheckable(true);
	slider->setChecked(false);
	slider->setEnabled(false);
  
	quit = new QAction("&Quit", this);
	quit->setShortcut(tr("CTRL+Q"));
	

    
	image = new QLabel("...", this);
	image->setToolTip("This is our output plate");
	image->setBackgroundRole(QPalette::Base);
	image->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	image->setScaledContents(true);
   
	address = "./images/";
	QVBoxLayout *vertical = new QVBoxLayout(this);
	vertical->addWidget(image);
	setLayout(vertical);
    
	connect(open, &QAction::triggered, this, &ContextMenuSample::openimage);
	connect(first, &QAction::triggered, this, &ContextMenuSample::firstimage);
	connect(next, &QAction::triggered, this, &ContextMenuSample::nextimage);
	connect(perv, &QAction::triggered, this, &ContextMenuSample::pervimage);
	connect(last, &QAction::triggered, this, &ContextMenuSample::lastimage);
	connect(quit, &QAction::triggered, this, &QApplication::quit);
	connect(slider, &QAction::triggered, this, &ContextMenuSample::slideshow);
	connect(rand, &QAction::triggered, this, &ContextMenuSample::randomimage);
	connect(timer, &QTimer::timeout, this, &ContextMenuSample::showeachimage);

	
}




void ContextMenuSample::mouseReleaseEvent(QMouseEvent *event){
	  if(event->button() == Qt::LeftButton)
	  	nextimage();
	  if(event->button() == Qt::MiddleButton)
	  	pervimage();
}


void ContextMenuSample::contextMenuEvent(QContextMenuEvent *event){ 
	contextmenu = new QMenu(this);
	contextmenu->addAction(open);
	contextmenu->addSeparator();
	contextmenu->addAction(info);
	contextmenu->addAction(list);
	contextmenu->addSeparator();
	contextmenu->addAction(next);
	contextmenu->addAction(perv);
	contextmenu->addSeparator();
	contextmenu->addAction(first);
	contextmenu->addAction(last);
	contextmenu->addAction(rand);
	contextmenu->addSeparator();
	contextmenu->addAction(slider);	
	contextmenu->addSeparator();
	contextmenu->addAction(full);	
	contextmenu->addSeparator();
	contextmenu->addAction(quit);
	
	contextmenu->exec(event->globalPos());
}

void ContextMenuSample::openimage(){
	address = QFileDialog::getExistingDirectory(this, "Open Directory", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	QDir directory(address);
  
	if (!directory.exists()){
		image->setText("The directory does not exist");
		next->setEnabled(false);
		perv->setEnabled(false);
		first->setEnabled(false);
		last->setEnabled(false);
		rand->setEnabled(false);
		slider->setEnabled(false);
	}
	else{
		directory.setFilter(QDir::Files);

		imagelist = directory.entryInfoList();
		counter = imagelist.size();
		current = 0;
		QFileInfo imageinfo = imagelist.at(current);
		QImage picture(imageinfo.absoluteFilePath());
		image->resize(picture.size());
		image->setPixmap(QPixmap::fromImage(picture));
		next->setEnabled(true);
		perv->setEnabled(true);
		first->setEnabled(true);
		last->setEnabled(true);
		rand->setEnabled(true);
		slider->setEnabled(true);
		
		QString windowtitle("Simple Image Viewer - ");
		windowtitle.append(QString::number(counter));
		windowtitle.append(" Files");
		this->setWindowTitle(windowtitle);
	}
}


void ContextMenuSample::nextimage(){
	if(++current==counter)
		current = 0;
	QFileInfo imageinfo = imagelist.at(current);
	QImage picture(imageinfo.absoluteFilePath());
	image->resize(picture.size());
	image->setPixmap(QPixmap::fromImage(picture));
	
	QString windowtitle("Simple Image Viewer - ");
	windowtitle.append(QString::number(current));
	windowtitle.append(" - ");
	windowtitle.append(imageinfo.fileName());
	this->setWindowTitle(windowtitle);
}

void ContextMenuSample::pervimage(){
	if(--current==-1)
		current = counter - 1;
	QFileInfo imageinfo = imagelist.at(current);
	QImage picture(imageinfo.absoluteFilePath());
	image->resize(picture.size());
	image->setPixmap(QPixmap::fromImage(picture));
	
	QString windowtitle("Simple Image Viewer - ");
	windowtitle.append(QString::number(current));
	windowtitle.append(" - ");
	windowtitle.append(imageinfo.fileName());
	this->setWindowTitle(windowtitle);
}

void ContextMenuSample::lastimage(){
	current = counter - 1;
	QFileInfo imageinfo = imagelist.at(current);
	QImage picture(imageinfo.absoluteFilePath());
	image->resize(picture.size());
	image->setPixmap(QPixmap::fromImage(picture));
	
	QString windowtitle("Simple Image Viewer - ");
	windowtitle.append(QString::number(current));
	windowtitle.append(" - ");
	windowtitle.append(imageinfo.fileName());
	this->setWindowTitle(windowtitle);
}


void ContextMenuSample::firstimage(){
	current = 0;
	QFileInfo imageinfo = imagelist.at(current);
	QImage picture(imageinfo.absoluteFilePath());
	image->resize(picture.size());
	image->setPixmap(QPixmap::fromImage(picture));
	
	QString windowtitle("Simple Image Viewer - ");
	windowtitle.append(QString::number(current));
	windowtitle.append(" - ");
	windowtitle.append(imageinfo.fileName());
	this->setWindowTitle(windowtitle);
}




void ContextMenuSample::slideshow(){
	if (slider->isChecked()) {
		open->setEnabled(false);
		next->setEnabled(false);
		perv->setEnabled(false);
		first->setEnabled(false);
		last->setEnabled(false);
		rand->setEnabled(false);
		slider->setEnabled(true);
		
		timer->start(1000);
	}
	else{
		open->setEnabled(true);
		next->setEnabled(true);
		perv->setEnabled(true);
		first->setEnabled(true);
		last->setEnabled(true);
		rand->setEnabled(true);
		slider->setEnabled(true);
		 
		timer->stop();
	}
}

void ContextMenuSample::randomimage(){
	current = qrand() % (counter - 1) + 1;
	this->nextimage();
}


void ContextMenuSample::showeachimage(){
	this->nextimage();
}


int main(int argc, char *argv[]){
	QApplication app(argc, argv);

	ContextMenuSample window;
	window.resize(640, 400);
	window.setWindowTitle("Simple Image Viewer");
	window.move(QApplication::desktop()->screen()->rect().center() - window.rect().center());
	window.setWindowIcon(QIcon("qt-logo.png")); 
	window.show();

	return app.exec();
}
