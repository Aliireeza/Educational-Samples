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
#include <QListView>
#include <QMouseEvent>
#include <QFileDialog>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QDateTime>
#include <QStringListModel>


//*****************************************************
//*****************************************************
//*****************************************************
//Application: Simple Image Viewer
//By: Aliireeza Teymoorian
//Version: 1.0.2
//License: GPL v3
//Created By: QT 7.9.0
//November 2017


//TODO: Few image filters
//TODO: Actions to List Image dialogbox
//TODO: Thumbnail View
//TODO: Print DialogBox
//TODO: Save Filtered Images
//TODO: Create Image Palette
//*****************************************************
//*****************************************************
//*****************************************************


//Here is the class defenition
class SimpleImageViewer : public QWidget {
	public:
		SimpleImageViewer(QWidget *parent = 0);
        
	private slots:
		void openimage();
		
		void imageinfo();
		void listimages();
		
		void nextimage();
		void pervimage();
		void firstimage();
		void lastimage();
		void randomimage();
		
		void slideshow();
		void fullscreen();
		
		void showabout();
		void showhelp();
		
		void showeachimage();
		void resizeimage();
		
	private:
		int counter = 0;
		int current;
		QLabel *image;
		QString address;
		QFileInfoList imagelist;

		//These are our context menu items
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
		QAction *about;
		QAction *help;
		
		QTimer *timer;
		QMenu *contextmenu;
		
		QPalette defaultpalette;
		QPalette blackpalette;
		//Some globally used fuctions
		void showimage();
		void openaddress();
		QString file_permissions(QFile::Permissions permission);
		
		//Event handlers for mouse, keyboard and drag and drop options
		void contextMenuEvent(QContextMenuEvent *event) override;	
		void mouseReleaseEvent(QMouseEvent * event) override;
		void keyPressEvent(QKeyEvent *event);
		void dropEvent(QDropEvent *event) override;
		void dragEnterEvent(QDragEnterEvent *event) override;
		void resizeEvent(QResizeEvent *event);
};

//Class Constructor
SimpleImageViewer::SimpleImageViewer(QWidget *parent): QWidget(parent){
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
	
	full = new QAction("Toggle Full &Screen", this);
	full->setShortcut(tr("CTRL+M"));
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

	about = new QAction("&About", this);
	about->setShortcut(tr("CTRL+A"));
	
	help = new QAction("&Help", this);
	help->setShortcut(tr("CTRL+H"));
	
	quit = new QAction("&Quit", this);
	quit->setShortcut(tr("CTRL+Q"));
	

    //This is our image pallete using a QLabel
	image = new QLabel("Simple Image Viewer, Right Click to Open an Image Folder ...", this);
	image->setToolTip("This is our output plate");
	image->setBackgroundRole(QPalette::Base);
	image->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	//image->setScaledContents(true);
	image->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
   
	address = "./images/";
	QVBoxLayout *vertical = new QVBoxLayout(this);
	vertical->addWidget(image);
	vertical->setSpacing(0);
	setLayout(vertical);
    setAcceptDrops(true);
    
    defaultpalette = this->palette();
    blackpalette.setColor(QPalette::Window, Qt::black);
    //Then connecting signals from menu to some slots
	connect(open, &QAction::triggered, this, &SimpleImageViewer::openimage);
	connect(info, &QAction::triggered, this, &SimpleImageViewer::imageinfo);
	connect(list, &QAction::triggered, this, &SimpleImageViewer::listimages);
	connect(next, &QAction::triggered, this, &SimpleImageViewer::nextimage);
	connect(perv, &QAction::triggered, this, &SimpleImageViewer::pervimage);
	connect(first, &QAction::triggered, this, &SimpleImageViewer::firstimage);
	connect(last, &QAction::triggered, this, &SimpleImageViewer::lastimage);
	connect(rand, &QAction::triggered, this, &SimpleImageViewer::randomimage);
	connect(slider, &QAction::triggered, this, &SimpleImageViewer::slideshow);
	connect(full, &QAction::triggered, this, &SimpleImageViewer::fullscreen);
	connect(about, &QAction::triggered, this, &SimpleImageViewer::showabout);
	connect(help, &QAction::triggered, this, &SimpleImageViewer::showhelp);
	connect(quit, &QAction::triggered, this, &QApplication::quit);
	connect(timer, &QTimer::timeout, this, &SimpleImageViewer::showeachimage);
}


//This function, capture the url of the folder or single image you have dropped in
void SimpleImageViewer::dropEvent(QDropEvent *e){
	const QMimeData *mimeData = e->mimeData();
	if(mimeData->hasUrls()){
        QList<QUrl> urlList = mimeData->urls();
		QStringList pathList;
		if(urlList.size() == 1)
       		address = urlList.at(0).toLocalFile();
     	this->openaddress();
		e->acceptProposedAction();
   }
}


//This fuction is responsible for accepting drag and drop actions
void SimpleImageViewer::dragEnterEvent(QDragEnterEvent *e){
	this->setBackgroundRole(QPalette::Highlight);
    e->acceptProposedAction();
}


//This is our keyboard handler
void SimpleImageViewer::keyPressEvent(QKeyEvent *e){
	int key = e->key();
	if(counter == 0)
		return;
		
	switch(key){
		case Qt::Key_0:
		case Qt::Key_1:
		case Qt::Key_2:
		case Qt::Key_3:
		case Qt::Key_4:
		case Qt::Key_5:
		case Qt::Key_6:
		case Qt::Key_7:
		case Qt::Key_8:
		case Qt::Key_9:
			break;
		case Qt::Key_F:
			this->fullscreen();
			break;
		case Qt::Key_R:
			this->randomimage();
			break;
		case Qt::Key_I:
			this->imageinfo();
			break;
		case Qt::Key_Right:
			this->nextimage();
			break;
		case Qt::Key_Left:
			this->pervimage();
			break;
		case Qt::Key_Up:
			this->firstimage();
			break;
		case Qt::Key_Down:
			this->lastimage();
			break;
	default:
		break;
	}
}


//This function, decide on what to do with mouse buttons
void SimpleImageViewer::mouseReleaseEvent(QMouseEvent *event){
	if(counter != 0){
		if(event->button() == Qt::LeftButton)
			this->nextimage();
		if(event->button() == Qt::MiddleButton)
			this->pervimage();
	}
}


//This fuction actually create the context menu of our application
void SimpleImageViewer::contextMenuEvent(QContextMenuEvent *event){ 
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
	contextmenu->addAction(about);	
	contextmenu->addAction(help);	
	contextmenu->addSeparator();
	contextmenu->addAction(quit);
	
	contextmenu->exec(event->globalPos());
}


//This fuction show an image from our image list at the current variable
void SimpleImageViewer::showimage(){
	QFileInfo imageinfo = imagelist.at(current);
	QPixmap picture(imageinfo.absoluteFilePath());
	
	image->setPixmap(picture.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
	
	QString windowtitle("Simple Image Viewer - ");
	windowtitle.append(QString::number(current));
	windowtitle.append(" - ");
	windowtitle.append(imageinfo.fileName());
	this->setWindowTitle(windowtitle);
}


//Now we have decided on what faunctionalities should be available and that you opened a valid file/directory or not
void SimpleImageViewer::openaddress(){
	QDir directory(address);
	QFile file(address);
	if(directory.exists()){
		directory.setFilter(QDir::Files);
		imagelist = directory.entryInfoList();
		counter = imagelist.size();
		current = 0;
		next->setEnabled(true);
		perv->setEnabled(true);
		first->setEnabled(true);
		last->setEnabled(true);
		rand->setEnabled(true);
		full->setEnabled(true);
		info->setEnabled(true);
		list->setEnabled(true);
		slider->setEnabled(true);
		this->showimage();
	}
	else if(!directory.exists() && file.exists()){
		imagelist.clear();
		imagelist.append(address);
		counter = 1;
		current = 0;
		next->setEnabled(false);
		perv->setEnabled(false);
		first->setEnabled(false);
		last->setEnabled(false);
		full->setEnabled(true);
		info->setEnabled(true);
		list->setEnabled(false);
		rand->setEnabled(false);
		slider->setEnabled(false);
		this->showimage();
	}
	else{
		image->setText("The directory does not exist");
		next->setEnabled(false);
		perv->setEnabled(false);
		first->setEnabled(false);
		last->setEnabled(false);
		rand->setEnabled(false);
		full->setEnabled(false);
		info->setEnabled(false);
		list->setEnabled(false);
		slider->setEnabled(false);
	}
}


//This function get the file/directory path from a file dialogbox
void SimpleImageViewer::openimage(){
	address = QFileDialog::getExistingDirectory(this, "Open Directory", QDir::homePath(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
	this->openaddress();
}


//This function set the current pointer to show the next image
void SimpleImageViewer::nextimage(){
	if(++current==counter)
		current = 0;
	this->showimage();
}


//This function set the current pointer to show the previous image
void SimpleImageViewer::pervimage(){
	if(--current==-1)
		current = counter - 1;
	this->showimage();
}


//This function set the current pointer to show the last image
void SimpleImageViewer::lastimage(){
	current = counter - 1;
	this->showimage();
}


//This function set the current pointer to show the first image
void SimpleImageViewer::firstimage(){
	current = 0;
	this->showimage();
}


//This function actually just set a timer to call next image automatically
void SimpleImageViewer::slideshow(){
	if(slider->isChecked()){
		open->setEnabled(false);
		next->setEnabled(false);
		perv->setEnabled(false);
		first->setEnabled(false);
		last->setEnabled(false);
		rand->setEnabled(false);
		info->setEnabled(false);
		list->setEnabled(false);
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
		info->setEnabled(true);
		list->setEnabled(true);
		slider->setEnabled(true);
		 
		timer->stop();
	}
}


//Toggle the view andbackground between fullscreen and normal mode
void SimpleImageViewer::fullscreen(){
		if(this->isFullScreen()){
			this->showNormal();
			this->layout()->setContentsMargins(10, 10, 10, 10);
			this->setPalette(defaultpalette);
			}
		else{
			this->showFullScreen();
			this->layout()->setContentsMargins(0, 0, 0, 0);
			this->setPalette(blackpalette);
			}
}


//Resize the image by calling open image function in time of window resize
void SimpleImageViewer::resizeEvent(QResizeEvent *e){
	Q_UNUSED(e); 
	if(counter !=0)
		this->showimage();
}


//This function select a random number for current in the correct range
void SimpleImageViewer::randomimage(){
	current = qrand() % (counter - 1) + 1;
	this->nextimage();
}


//This is the callback fuction for timer, in each timeout it goes for the next image
void SimpleImageViewer::showeachimage(){
	this->nextimage();
}


//This function decide on what permission the image file has and return a ls -l like string
QString SimpleImageViewer::file_permissions(QFile::Permissions permission){
  QString filepermission;
  
  if (permission & QFile::ReadOwner) {
      filepermission.append('r');
  } else {
      filepermission.append('-');
  }
  
  if (permission & QFile::WriteOwner) {
      filepermission.append('w');
  } else {
      filepermission.append('-');
  }  
  
  if (permission & QFile::ExeOwner) {
      filepermission.append('x');
  } else {
      filepermission.append('-');
  }    
  
  if (permission & QFile::ReadGroup) {
      filepermission.append('r');
  } else {
      filepermission.append('-');
  }
  
  if (permission & QFile::WriteGroup) {
      filepermission.append('w');
  } else {
      filepermission.append('-');
  }  
  
  if (permission & QFile::ExeGroup) {
      filepermission.append('x');
  } else {
      filepermission.append('-');
  }    
  
  if (permission & QFile::ReadOther) {
      filepermission.append('r');
  } else {
      filepermission.append('-');
  }
  
  if (permission & QFile::WriteOther) {
      filepermission.append('w');
  } else {
      filepermission.append('-');
  }  
  
  if (permission & QFile::ExeOther) {
      filepermission.append('x');
  } else {
      filepermission.append('-');
  }      
  
  return filepermission;
}


//This function extract the image file information and sho the relevant dialogbox
void SimpleImageViewer::imageinfo(){
	QString output;

	QFileInfo fileinfo = imagelist.at(current);
	QString name = fileinfo.fileName();
	int size = fileinfo.size();
	QString absolutepath = fileinfo.absoluteFilePath();
	QFile::Permissions permission = QFile::permissions(absolutepath);
	QString basename = fileinfo.baseName();
	QString completebasename = fileinfo.completeBaseName();
	QString suffix = fileinfo.suffix();
	QString completesuffix = fileinfo.completeSuffix();
	QDateTime lastread = fileinfo.lastRead();
	QDateTime lastmodified = fileinfo.lastModified();
	QString group = fileinfo.group();
	QString owner = fileinfo.owner();
	QImage picture(absolutepath);

	output.append("File: " + name + " [" + QString::number(size /1024) + " KB, " + QString::number(picture.width()) + "x" + QString::number(picture.height()) + "]<br>");
	output.append("Suffix: " + suffix + " [" + completesuffix + "]<br>");
	output.append("Owner: " + owner + "&nbsp;&nbsp;Group: " + group + "&nbsp;&nbsp;Permissions: " + this->file_permissions(permission));
	output.append("<br>Basename: " + basename + " [" + completebasename + "]<br>");
	output.append("Last Access Time: " + lastread.toString() + "<br>Last Modiefied Time: " + lastmodified.toString());

	
	QDialog dialog(this);
	QFormLayout form(&dialog);
	dialog.setWindowTitle("Image Info");
	dialog.resize(300, 180);
	
	QLabel *label = new QLabel(output, this);
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	label->setAlignment(Qt::AlignLeft);
	label->setTextFormat(Qt::RichText);
	label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	
	form.addRow(label);
	
	QDialogButtonBox buttonbox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
	buttonbox.setCenterButtons(true);
	form.addRow(&buttonbox);
	
	connect(&buttonbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	dialog.exec();
}


//This function showes a dialogbox to list all opened images in a folder
void SimpleImageViewer::listimages(){
	QDialog dialog(this);
	QFormLayout form(&dialog);
	dialog.setWindowTitle("Images");
	dialog.resize(300, 200);

	QListView *listview = new QListView(this);
	QStringListModel *model = new QStringListModel(this);
	QStringList filepaths;
	for(int i=0; i<imagelist.size(); filepaths.append(imagelist.at(i++).fileName()));
	model->setStringList(filepaths);
	listview->setModel(model);
	listview->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	form.addRow(listview);
	
	QDialogButtonBox buttonbox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
	buttonbox.setCenterButtons(true);
	form.addRow(&buttonbox);
	
	connect(&buttonbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	dialog.exec();
}


//This fucntion show About dialogbox
void SimpleImageViewer::showabout(){
	QDialog dialog(this);
	QFormLayout form(&dialog);
	dialog.setWindowTitle("About");
	dialog.resize(300, 200);
	
	QLabel *label = new QLabel("Simple Image Viewer, 1.0.2", this);
	label->setText(label->text() + "<br>By: Aliireeza Teymoorian");
	label->setText(label->text() + "<br><br><a href=\"mailto:teymoorian@gmail.com\">teymoorian@gmail.com</a>");
	label->setText(label->text() + "<br><a href=\"https://github.com/Aliireeza/Educational-Samples/tree/master/Linux-QT-Programming/Simple-Image-Viewer\">Github Repository ;)</a>");
	label->setText(label->text() + "<br><br>November 2017");
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	label->setAlignment(Qt::AlignCenter);
	label->setTextFormat(Qt::RichText);
	label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	label->setOpenExternalLinks(true);
	
	form.addRow(label);
	
	QDialogButtonBox buttonbox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
	buttonbox.setCenterButtons(true);
	form.addRow(&buttonbox);
	
	connect(&buttonbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	dialog.exec();
}


//This fuction shoes a simple help dialogbox
void SimpleImageViewer::showhelp(){
	QDialog dialog(this);
	QFormLayout form(&dialog);
	dialog.setWindowTitle("Help");
	dialog.resize(300, 200);
	
	QLabel *label = new QLabel("Simple Image Viewer, 1.0.2", this);
	label->setText(label->text() + "<br><br>Right Click: Main/Navigation Menu");
	label->setText(label->text() + "<br>Left Click/ Left Arrow: Next Image");
	label->setText(label->text() + "<br>Middle Click/ Right Arrow: Previous Image");
	label->setText(label->text() + "<br>Up Arrow: First Image");
	label->setText(label->text() + "<br>Down Arrow: Last Image");
	label->setText(label->text() + "<br>0-9: Image Filters");
	label->setText(label->text() + "<br>R: Random Image");
	label->setText(label->text() + "<br>F: Toggle Fulscreen View");
	label->setText(label->text() + "<br>I: Image Infromation");
	label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	label->setAlignment(Qt::AlignLeft);
	label->setTextFormat(Qt::RichText);
	label->setTextInteractionFlags(Qt::TextBrowserInteraction);
	label->setOpenExternalLinks(true);
	
	form.addRow(label);
	
	QDialogButtonBox buttonbox(QDialogButtonBox::Ok, Qt::Horizontal, &dialog);
	buttonbox.setCenterButtons(true);
	form.addRow(&buttonbox);
	
	connect(&buttonbox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	dialog.exec();
}


//The main entry od the application as any othe c/c++ programs
int main(int argc, char *argv[]){
	QApplication app(argc, argv);

	SimpleImageViewer window;
	window.resize(640, 400);
	window.setWindowTitle("Simple Image Viewer");
	window.move(QApplication::desktop()->screen()->rect().center() - window.rect().center());
	window.setWindowIcon(QIcon("qt-logo.png")); 
	window.show();

	return app.exec();
}
