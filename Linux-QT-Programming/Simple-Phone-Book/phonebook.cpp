#include <QApplication>
#include <QtCore>
#include <QWidget>
#include <QDesktopWidget>
#include <QIcon>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>

#include <QtSql>

#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <iostream>
#include <QList>


//This is just the definition of the class that is responsible for all
class Phonebook : public QWidget {
    public:
        Phonebook(QWidget *parent = 0);

    private:
		QPushButton *connectbutton;
		QPushButton *listcontact;
		QPushButton *addcontact;
		QPushButton *delcontact;
		QPushButton *updatecontact;
		QPushButton *removecontact;
		QPushButton *searchcontact;
		QPushButton *quitbutton;

		QSqlDatabase db;
		QMessageBox messagebox;

		QString password;
		QString username;
		QString dbname;

		QString contactname = "NULL";
		QString contacthome = "NULL";
		QString contactwork = "NULL";
		QString contactmobile = "NULL";
		QString contactemail = "NULL";

		void GetContact();

    private slots:
    	void ConnectDatabase();
    	void ListContacts();
    	void RemoveContacts();
    	void SearchName();
    	void AddContact();
    	void DeleteContact();
    	void UpdateContact();
};

//This is the constructor of our phonebook class
Phonebook::Phonebook(QWidget *parent): QWidget(parent) {
	//First, create the UI, begin with buttons
	connectbutton = new QPushButton("Connect Database", this);
	listcontact = new QPushButton("List Contacts", this);
	addcontact = new QPushButton("Add Contact", this);
	delcontact = new QPushButton("Delete Contact", this);
	updatecontact = new QPushButton("Update Contact", this);
	removecontact = new QPushButton("Remove Contacts", this);
	searchcontact = new QPushButton("Search Name", this);
	quitbutton = new QPushButton("Quit Program", this);

	//All buttons except connect and quit should be disable at first
	listcontact->setEnabled(false);
	addcontact->setEnabled(false);
	delcontact->setEnabled(false);
	updatecontact->setEnabled(false);
	removecontact->setEnabled(false);
	searchcontact->setEnabled(false);

	//After creating buttons, we should placed them correctly
	QVBoxLayout *vertical = new QVBoxLayout(this);
	vertical->addWidget(connectbutton);
	vertical->addSpacing(10);
	vertical->addWidget(listcontact);
	vertical->addWidget(removecontact);
	vertical->addWidget(searchcontact);
	vertical->addSpacing(10);
	vertical->addWidget(addcontact);
	vertical->addWidget(updatecontact);
	vertical->addWidget(delcontact);
	vertical->addSpacing(10);
	vertical->addWidget(quitbutton);
	setLayout(vertical);


	//Second, it is important to connect clicked signal of each button to correct slot
	connect(quitbutton, &QPushButton::clicked, qApp, &QApplication::quit);
	connect(connectbutton, &QPushButton::clicked, this, &Phonebook::ConnectDatabase);
	connect(listcontact, &QPushButton::clicked, this, &Phonebook::ListContacts);
	connect(removecontact, &QPushButton::clicked, this, &Phonebook::RemoveContacts);
	connect(addcontact, &QPushButton::clicked, this, &Phonebook::AddContact);
	connect(delcontact, &QPushButton::clicked, this, &Phonebook::DeleteContact);
	connect(updatecontact, &QPushButton::clicked, this, &Phonebook::UpdateContact);
	connect(searchcontact, &QPushButton::clicked, this, &Phonebook::SearchName);
}

//Connecting to database should create a connection, database and table if neccessary
void Phonebook::ConnectDatabase() {
	bool ok;
	//We want to use command line for now, after this simple sample we will use
	//query and table model, but for now we need a stream to stdout
	QTextStream output(stdout);
	//First things first, in QT we should determine which database driver
	//we want to use, here just QMYSQL
	db = QSqlDatabase::addDatabase("QMYSQL");
	//then initialize database parameters, host, username and password
	db.setHostName("localhost");

	//We want to get these information from user by dialog boxes
	username = QInputDialog::getText(this, tr("User Name"), tr("Enter User Name:"), QLineEdit::Normal, "root", &ok);
	if(ok)
		db.setUserName(username);

	password = QInputDialog::getText(this, tr("User Name"), tr("Enter User Name:"), QLineEdit::Password, "MariaDBRoot", &ok);
	if(ok)
		db.setPassword(password);

	//If everythings went well till this point, we can create a connection and
	//send SQL commands for creating database and table
	if (!db.isValid()){
    	messagebox.setText("Connection Failure: " + db.lastError().text());
		messagebox.setWindowTitle("Error :(");
		messagebox.exec();
		return;
		}


	//Now, it is time for database and table creation
	output << "Initializing Database ... ";
	if(db.open()){
		dbname = QInputDialog::getText(this, tr("Database Name"), tr("Enter Database Name:"), QLineEdit::Normal, "phonebook", &ok);
		if(ok){
			//After getting database name from user, we create it
			db.setDatabaseName(dbname);
			output << "ok" << endl;
			QString querysyntax = "CREATE DATABASE IF NOT EXISTS " + dbname;
			output << querysyntax << endl;
			db.exec(querysyntax);
			//Then we should use this newly created database
			querysyntax = "USE " + dbname;
			output << querysyntax << endl;
			db.exec(querysyntax);
			QSqlQuery query;

			//From now on, we can just send query to the database and evaluate
			//the result, print out the result set and report errors
			querysyntax = "CREATE TABLE IF NOT EXISTS book( name VARCHAR(40) NOT NULL, home DECIMAL(14, 0) default NULL, work DECIMAL(14, 0) default NULL, mobile DECIMAL(14, 0) default NULL, email VARCHAR(40) default NULL, PRIMARY KEY (name));";
			output << querysyntax << endl;
			if(!query.exec(querysyntax)){
				messagebox.setText("CREATE TABLE FAILURE: " + query.lastError().text());
				messagebox.setWindowTitle("Query ERROR");
				messagebox.exec();
				QApplication::quit();
			}

			//If everything were ok, we can enable other buttons so their
			//functionality would be available for users
			connectbutton->setEnabled(false);
			listcontact->setEnabled(true);
			addcontact->setEnabled(true);
			delcontact->setEnabled(true);
			updatecontact->setEnabled(true);
			removecontact->setEnabled(true);
			searchcontact->setEnabled(true);

			messagebox.setText("You have connected to database");
			messagebox.setWindowTitle("Conguratulations ;)");
			messagebox.exec();
			}
		}
	else{
		//Otherwise, we have to report errors
		messagebox.setText("Connection Failure: " + db.lastError().text());
		messagebox.setWindowTitle("Error :(");
		messagebox.exec();
		output << "failed!" << endl;
		return;
		}
}


//For listing contacts, we have to create a query of SELECT syntax and
//send it to the database, then print the result set one row at a time
void Phonebook::ListContacts() {
	db.transaction();
		QSqlQuery query;
		QTextStream output(stdout);
		//This is the essential SQL query
		QString querysyntax = "SELECT * FROM book";
		output << querysyntax << endl;
		if(!query.exec(querysyntax)){
			//In case of error
			messagebox.setText("LIST ALL CONTACTS FAILURE: " + query.lastError().text());
			messagebox.setWindowTitle("Query ERROR");
			messagebox.exec();
			}
		else{
			//Otherwise, it is possible to print out every row now
			output << query.size() << " records are as follows: " << endl;

			while(query.next()){
				QString personinfo = "   " + query.value(0).toString();
				personinfo.append("  |  " + query.value(1).toString());
				personinfo.append("  |  " + query.value(2).toString());
				personinfo.append("  |  " + query.value(3).toString());
				personinfo.append("  |  " + query.value(4).toString());
				output << personinfo << endl;
			}
		}
	db.commit();
}


//For remove all contacts, again we can create a simple SQL command and send it
void Phonebook::RemoveContacts() {
	QSqlQuery query;
	//This is serous, so we have to be sure wether user really intended to do so
	messagebox.setText("Are You Sure You Want To Delete All Contacts?");
	messagebox.setStandardButtons(QMessageBox::Yes);
	messagebox.addButton(QMessageBox::No);
	messagebox.setDefaultButton(QMessageBox::No);
	messagebox.setWindowTitle("Question");
	//if ok, we send the query and check for errors
	if(messagebox.exec() == QMessageBox::Yes){
		QTextStream output(stdout);

		QString querysyntax = "DELETE FROM book";
		output << querysyntax << endl;
		if(!query.exec(querysyntax)){
			messagebox.setText("REMOVE ALL CONTACTS FAILURE: " + query.lastError().text());
			messagebox.setWindowTitle("Query ERROR");
			messagebox.exec();
		}
		else
			//otherwise, everything has been deleted and there is nothing left
			output << "All Contacts Deleted (" << query.size() << ")" << endl;
	}
}


//For searching a contact, we have to ask for a name (primary key), then send
//a proper query and report possible result set
void Phonebook::SearchName() {
	QTextStream output(stdout);
	bool ok;
	//Asking for a name to search
	QString person = QInputDialog::getText(this, tr("Search Name"), tr("Enter a Name:"), QLineEdit::Normal, "", &ok);
	if(ok){
		QSqlQuery query;
		//If we have a name, then this would be the query fo search it
		QString querysyntax = "SELECT * FROM book WHERE name LIKE \'" + person + "\'";
		output << querysyntax << endl;
		if(!query.exec(querysyntax)){
			messagebox.setText("SEARCH CONTACT FAILURE: " + query.lastError().text());
			messagebox.setWindowTitle("Query ERROR");
			messagebox.exec();
			}
		else{
			//again, it there was no error, we can print out results line by
			//line till the result set become empty
			while(query.next()){
				QString personinfo = "   " + query.value(0).toString();
				personinfo.append("  |  " + query.value(1).toString());
				personinfo.append("  |  " + query.value(2).toString());
				personinfo.append("  |  " + query.value(3).toString());
				personinfo.append("  |  " + query.value(4).toString());
				output << personinfo << endl;
				messagebox.setText("Founded Record: " + personinfo);
				messagebox.setWindowTitle(query.size() + " results for " + person );
				messagebox.exec();
				}
			//In case of no error and not found anything we have to create a
			//message to indicate everything went ok, size method will be useful
			if(query.size() == 0){
				messagebox.setText("Not Result Has Found");
				messagebox.setWindowTitle("Query Handled");
				messagebox.exec();
				}
			}
		}
}


//This function create a satisfactory dialog box with five inputs, one for each
//fields of a contact record, then fill the variables which will be used in
//both add and update methods
void Phonebook::GetContact(){
	QDialog dialog(this);
	QFormLayout form(&dialog);
	form.addRow(new QLabel("Enter Contact Information"));


	QLineEdit *contactnameedit = new QLineEdit(&dialog);
	contactnameedit->setFixedWidth(240);
	QLabel *contactnamelable = new QLabel("Contact Name: ");
	form.addRow(contactnamelable, contactnameedit);

	QLineEdit *contacthomeedit = new QLineEdit(&dialog);
	contacthomeedit->setFixedWidth(240);
	QLabel *contacthomelable = new QLabel("Contact Home Phone: ");
	form.addRow(contacthomelable, contacthomeedit);

	QLineEdit *contactworkedit = new QLineEdit(&dialog);
	contactworkedit->setFixedWidth(240);
	QLabel *contactworklable = new QLabel("Contact Work Phone: ");
	form.addRow(contactworklable, contactworkedit);

	QLineEdit *contactmobileedit = new QLineEdit(&dialog);
	contactmobileedit->setFixedWidth(240);
	QLabel *contactmobilelable = new QLabel("Contact Mobile Phone: ");
	form.addRow(contactmobilelable, contactmobileedit);

	QLineEdit *contactemailedit = new QLineEdit(&dialog);
	contactemailedit->setFixedWidth(240);
	QLabel *contactemaillable = new QLabel("Contact Email: ");
	form.addRow(contactemaillable, contactemailedit);

	QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
        form.addRow(&buttonBox);

	connect(&buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
	connect(&buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

	startagain:
	if (dialog.exec() == QDialog::Accepted){
		contactname = contactnameedit->text();
		contacthome = contacthomeedit->text();
		contactwork = contactworkedit->text();
		contactmobile = contactmobileedit->text();
		contactemail = contactemailedit->text();
		}
	else{
		messagebox.setText("Are You Sure You Want To USE NULL INformation for Contacts?");
		messagebox.setStandardButtons(QMessageBox::Yes);
		messagebox.addButton(QMessageBox::No);
		messagebox.setDefaultButton(QMessageBox::No);
		messagebox.setWindowTitle("Question");
		if(messagebox.exec() == QMessageBox::No)
			goto startagain;
	}
}


//For adding a new contact, we will get the information by using above function
//then, it is all the same as any other queries
void Phonebook::AddContact() {
	GetContact();
	QTextStream output(stdout);
	QSqlQuery query;

	//This is important to create a correct SQL query syntax from variables
	QString querysyntax = "INSERT INTO book VALUES (\'" + contactname;
	querysyntax.append("\', \'" + contacthome + "\', \'" + contactwork);
	querysyntax.append("\', \'" + contactmobile + "\', \'" + contactemail + "\')");
	output << querysyntax << endl;
	if(!query.exec(querysyntax)){
		messagebox.setText("ADD CONTACT FAILURE: " + query.lastError().text());
		messagebox.setWindowTitle("Query ERROR");
		messagebox.exec();
		}
	else
		output << "Contact " << contactname << " Added Successfuly" << endl;
}

//For updating an existing contact, we will get the information by using above function
//then, it is all the same as any other queries
void Phonebook::UpdateContact() {
	GetContact();
	QTextStream output(stdout);
	QSqlQuery query;

	//This is important to create a correct SQL query syntax from variables
	QString querysyntax = "REPLACE INTO book VALUES (\'" + contactname;
	querysyntax.append("\', \'" + contacthome + "\', \'" + contactwork);
	querysyntax.append("\', \'" + contactmobile + "\', \'" + contactemail + "\')");
	output << querysyntax << endl;
	if(!query.exec(querysyntax)){
		messagebox.setText("UPDATE CONTACT FAILURE: " + query.lastError().text());
		messagebox.setWindowTitle("Query ERROR");
		messagebox.exec();
		}
	else
		output << "Contact " << contactname << " Updated Successfuly" << endl;
}


//For deleting a contact, fist we have to get its name (primary key) then
//send a query to database and check for errors
void Phonebook::DeleteContact() {
	QTextStream output(stdout);
	bool ok;
	//Asking for a name
	QString person = QInputDialog::getText(this, tr("Delete Name"), tr("Enter a Name:"), QLineEdit::Normal, "", &ok);
	if(ok){
		QSqlQuery query;
		//Now we create our query using the inserted name
		QString querysyntax = "DELETE FROM book WHERE name LIKE \'" + person + "\'";
		output << querysyntax << endl;
		//Send the delete query and lookout for errors
		if(!query.exec(querysyntax)){
			messagebox.setText("DELETE CONTACT FAILURE: " + query.lastError().text());
			messagebox.setWindowTitle("Query ERROR");
			messagebox.exec();
		}
		else{
			messagebox.setWindowTitle("Contact Deleted");
			messagebox.setText("Contacts Deleted for " + person);
			messagebox.exec();
		}
	}
}


//This is where everything begins, the migthy main function
int main(int argc, char *argv[]) {
	//First, pass command line arguments to our application
	QApplication app(argc, argv);
	//Second, create an instant of our class
	Phonebook window;

	//Then, set some attributes of the window of our class
	window.resize(300, 400);
	window.setWindowTitle("Simple Phonebook");
	window.move(QApplication::desktop()->screen()->rect().center() - window.rect().center());
	window.setWindowIcon(QIcon("qt-logo.png"));
	//Now show the window
	window.show();
	//And finally execute the application
	return app.exec();
}
