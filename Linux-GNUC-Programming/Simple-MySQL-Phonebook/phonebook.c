#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>

#include <string.h>

#define SIMPLE_PHONEBOOK_AUTHUR "Aliireeza Teymoorian"
#define SIMPLE_PHONEBOOK_VERSION "1.0.1"
#define SIMPLE_PHONEBOOK_COMPILER "GNU/GCC 7.1.1"
#define SIMPLE_PHONEBOOK_LICENSE "GNU/GPL v3"

static char *opt_host_name = "localhost";
static char *opt_user_name = "root";
static char *opt_password = "bs#72us9@5grt^4!";
static char *opt_socket_name = NULL;
static char *opt_db_name = NULL;
static unsigned int opt_port_number = 0;
static unsigned int opt_flags = 0;

static MYSQL *conn;

static void print_error(MYSQL *conn, char *message){
	fprintf(stderr,"%s\n", message);
	if(conn){
		#if MYSQL_VERSION_ID >= 40101
			fprintf(stderr, "ERROR %u (%s): %s\n", mysql_errno(conn), mysql_sqlstate(conn), mysql_error(conn));
		#else
			fprintf(stderr, "ERROR %u: %s\n", mysql_errno(conn), mysql_error(coon));
		#endif
	}
}


static void process_result_set(MYSQL *conn, MYSQL_RES *res_set){
	MYSQL_ROW row;
	unsigned int i;
	while((row = mysql_fetch_row(res_set)) != NULL){
		for(i=0; i<mysql_num_fields(res_set); i++){
			if(i>0)
				fputc('\t', stdout);
			printf("%s", row[i] != NULL ? row[i] : "NULL");
		}
		fputc('\n', stdout);
	}

	if(mysql_errno(conn) != 0)
		print_error(conn, "mysql_fetch_row() has failed");
	else
		printf("\nQuery handled: %lu rows returned\n", (unsigned long) mysql_num_rows(res_set));
}


static void process_statement(MYSQL *conn, char *statement){
	MYSQL_RES *res_set;
	if(mysql_query(conn, statement)){
		print_error(conn, "Query Failed: Could not execute the statement");
		return;
	}
	printf("\"%s\"\n\n", statement);
	res_set = mysql_store_result(conn);
	if(res_set){
		process_result_set(conn, res_set);
		mysql_free_result(res_set);
	}
	else{
		if(mysql_field_count(conn) == 0)
			printf("\nQuery handled: %lu rows affected\n", (unsigned long) mysql_affected_rows(conn));
		else
			print_error(conn, "\nQuery Failed: Could not retrive the result set");
	}

}



static void show_credits(){
	printf("Simple Phonebook Version %s\n", SIMPLE_PHONEBOOK_VERSION);
	printf("Created By %s\n", SIMPLE_PHONEBOOK_AUTHUR);
	printf("USING %s COMPILER, ", SIMPLE_PHONEBOOK_COMPILER);
	printf("DATABASE %s\n", mysql_get_client_info());
	printf("Published Under %s License\n", SIMPLE_PHONEBOOK_LICENSE);
}



static void search_contact(){
	char name[40], statement[80];
	printf("Enter Contact Name: ");
	scanf("%s", name);
	printf("\n---------------------------------\n\n");

	sprintf(statement, "SELECT * FROM book WHERE name LIKE \'%s\'", name);
	process_statement(conn, statement);
}



static void list_contacts(){
	char statement[30];
	sprintf(statement, "SELECT * FROM book");
	process_statement(conn, statement);
}



static void insert_contact(){
	char statement[200];

	char name[41], email[41];
	char homephone[15], workphone[15], mobilephone[15];

	printf("Enter Contact Name: ");
	scanf("%s", name);
	printf("Enter Home Phone Number: ");
	scanf("%s", homephone);
	printf("Enter Work Phone Number: ");
	scanf("%s", workphone);
	printf("Enter Mobile Phone Number: ");
	scanf("%s", mobilephone);
	printf("Enter Contact Email: ");
	scanf("%s", email);
	printf("\n---------------------------------\n\n");

	sprintf(statement, "INSERT INTO book VALUES (\'%s\', \'%s\', \'%s\', \'%s\', \'%s\')", name, homephone, workphone, mobilephone, email);
	process_statement(conn, statement);
}



static void update_contact(){
	char statement[200];

	char name[41], email[41];
	char homephone[15], workphone[15], mobilephone[15];

	printf("Enter Contact Name: ");
	scanf("%s", name);
	printf("Enter Home Phone Number: ");
	scanf("%s", homephone);
	printf("Enter Work Phone Number: ");
	scanf("%s", workphone);
	printf("Enter Mobile Phone Number: ");
	scanf("%s", mobilephone);
	printf("Enter Contact Email: ");
	scanf("%s", email);
	printf("\n---------------------------------\n\n");

	sprintf(statement, "REPLACE INTO book VALUES (\'%s\', \'%s\', \'%s\', \'%s\', \'%s\')", name, homephone, workphone, mobilephone, email);
	process_statement(conn, statement);
}



static void delete_contact(){
	char name[40], statement[80];
	printf("Enter Contact Name: ");
	scanf("%s", name);
	printf("\n---------------------------------\n\n");

	sprintf(statement, "DELETE FROM book WHERE name LIKE \"%s\"", name);
	process_statement(conn, statement);
}



static void drop_contacts(){
	char statement[30];
	sprintf(statement, "DELETE FROM book");
	process_statement(conn, statement);
}



static void contacts_information(){
	char statement[30];
	sprintf(statement, "SELECT COUNT(name) FROM book");
	process_statement(conn, statement);
}



static void process_user_choice(int choice){
	switch(choice){
		case 0:
			show_credits();
			break;
		case 1:
			search_contact();
			break;
		case 2:
			list_contacts();
			break;
		case 3:
			insert_contact();
			break;
		case 4:
			update_contact();
			break;
		case 5:
			delete_contact();
			break;
		case 6:
			drop_contacts();
			break;
		case 7:
			contacts_information();
			break;
		default:
			printf("Goodbye ;)\n");
			mysql_close(conn);
			exit(0);
	}
}



static int show_user_menu(){
	int choice;
	//After our simple prompt we will show these Eight options
	printf("\n----------  MAIN MENU  ----------\n");
	printf("\n1. Search Contact\t2.List Contacts\n");
	printf("3. Insert Contact\t4.Update Contact\n");
	printf("5. Delete Contact\t6. Drop Contacts\n");
	printf("7. Contacts Information\t8. Quit Phonebook\n\n");
	//However, Option 0 will show credits Information

	printf("Select Your Choice: ");
	scanf("%d", &choice);
	printf("\n---------------------------------\n\n");
	return choice;
}



static void create_and_connect(){
	printf("INITIALIZING our Database ...");
	//First we need to initialize our connection data structure
	conn = mysql_init(NULL);
	if(!conn){
		print_error(NULL, "mysql_init() has failed, Probably out of memory");
		exit(1);
	}

	//Second, we will connect to the MySQL or MariaDB RDBMS
	if(!mysql_real_connect(conn, opt_host_name, opt_user_name, opt_password, opt_db_name, opt_port_number, opt_socket_name, opt_flags)){
		print_error(conn, "mysql_real_connect() has failed");
		mysql_close(conn);
		exit(1);
	}

	//Third, if everything has gone well, we want to create our database and table, if they were not created yet
	if(mysql_query(conn, "CREATE DATABASE IF NOT EXISTS phonebook")){
		print_error(conn, "Creation of phonebook database has failed");
		mysql_close(conn);
		exit(1);
	}

	//Fourth, we have to use that database
	if(mysql_select_db(conn, "phonebook")){
		print_error(conn, "Using phonebook database has failed");
		mysql_close(conn);
		exit(1);
	}

	//Fifth, at the last stage we want to create a table for our phone book if that was not exists
	if(mysql_query(conn, "CREATE TABLE IF NOT EXISTS book( name VARCHAR(40) NOT NULL, home DECIMAL(14, 0) default NULL, work DECIMAL(14, 0) default NULL, mobile DECIMAL(14, 0) default NULL, email VARCHAR(40) default NULL, PRIMARY KEY (name));")){
		print_error(conn, "Creation of phonebook table has failed");
		mysql_close(conn);
		exit(1);
	}

	printf(" OK\n");
}


//This is the mail part of our program
int main(int argc, char *argv[]){
	int choice;

	//First, we need to create a database and a proper table for it
	create_and_connect();

	//Now the databse and table has created, so we have to show the user menu
	while((choice = show_user_menu()) >= 0 && choice <= 8)
		process_user_choice(choice);

	//At the end, we have to terminate the connection to the database and exit
	printf("You have entered wrong option, program terminated ...\n");
	mysql_close(conn);
	exit(0);
}
