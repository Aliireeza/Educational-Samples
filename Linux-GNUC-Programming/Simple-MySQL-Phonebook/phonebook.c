//These are the essential header files for this simplest posssible
//MYSQL-C-API sample
#include <my_global.h>
#include <my_sys.h>
#include <mysql.h>
#include <my_getopt.h>
#include <string.h>


//These are our pre-defined information which will come in handy
//when user wants to print the credits information
#define SIMPLE_PHONEBOOK_AUTHUR "Aliireeza Teymoorian"
#define SIMPLE_PHONEBOOK_VERSION "1.0.2 Summer 2017"
#define SIMPLE_PHONEBOOK_COMPILER "GNU/GCC 7.1.1"
#define SIMPLE_PHONEBOOK_LICENSE "GNU/GPL v3"


//Server Host Name, default = localhost
static char *opt_host_name = NULL;
//Username, default = your login name
static char *opt_user_name = NULL;
//Password, default = none
static char *opt_password = NULL;
//Port Number, default use built-in value
static unsigned int opt_port_number = 0;
//Socket Name, default use built-in value
static char *opt_socket_name = NULL;
//Database Name, default = none
static char *opt_db_name = NULL;
//connection flags (none)
static unsigned int opt_flags = 0;


//Whether to colicite password
static int ask_password = 0;
//pointer to our connection handler
static MYSQL *conn;
//User groups
static const char *client_groups[] = {"client", NULL};


//Option information structures
static struct my_option my_opts[] = {
	{"help", '?', "Display this help and exit", NULL, NULL, NULL, GET_NO_ARG, NO_ARG, 0,0,0,0,0,0},
	{"host", 'h', "Host to connect to", &opt_host_name, NULL, NULL, GET_STR_ALLOC, REQUIRED_ARG, 0,0,0,0,0,0},
	{"password", 'p', "Password", &opt_password, NULL, NULL, GET_STR_ALLOC, REQUIRED_ARG, 0,0,0,0,0,0},
	{"port", 'P', "Port Number", &opt_port_number, NULL, NULL, GET_UINT, REQUIRED_ARG, 0,0,0,0,0,0},
	{"socket", 'S', "Socket Path", &opt_socket_name, NULL, NULL, GET_STR_ALLOC, REQUIRED_ARG, 0,0,0,0,0,0},
	{"user", 'u', "Username", &opt_user_name, NULL, NULL, GET_STR_ALLOC, REQUIRED_ARG, 0,0,0,0,0,0},
	{NULL, 0, NULL, NULL, NULL, NULL, GET_NO_ARG, NO_ARG, 0,0,0,0,0,0}
	};


//This is our universal error  printing function
static void print_error(MYSQL *conn, char *message){
	//First it print programmer-defined error messages
	fprintf(stderr,"%s\n", message);
	if(conn){
		//Then due to the version of your mysql it uses below functions
		//mysql_error will return a string comment about the occured error
		//mysql_errno will return an negative integer number of the error
		//mysql_sqlstate will also return a number, but it defines only in mysql states and returned actually as a string
		#if MYSQL_VERSION_ID >= 40101
			fprintf(stderr, "ERROR %u (%s): %s\n", mysql_errno(conn), mysql_sqlstate(conn), mysql_error(conn));
		#else
			fprintf(stderr, "ERROR %u: %s\n", mysql_errno(conn), mysql_error(coon));
		#endif
	}
}


//this function will analyze options one by one and decide that which
//to print help or ask for a password in a secure mannar
static my_bool get_one_option(int optid, const struct my_option *opt, char *argument){
	switch(optid){
		case '?':
			my_print_help(my_opts);
			exit(0);
		case 'p':
			if(!argument)
				ask_password = 1;
			else{
				opt_password = strdup(argument);
				if(!opt_password){
					print_error(NULL, "could not allocate password buffer");
					exit(1);
				}
				while(*argument)
					*argument++ = 'x';
				ask_password = 0;
			}
			break;
		}
	return 0;
}

//This is the section in which we will process the SQL queries and
//retrive the answers from database, then show the results to the user
//Three below functions will do that and they could be reused almost
//without any change in any similar applications

//This function will process and show the result set retrived from executing a correct query
static void process_result_set(MYSQL *conn, MYSQL_RES *res_set){
	MYSQL_ROW row;
	unsigned int i;
	//For each record of the result set, we can obtain a row with
	//mysql_fetch_row function and check it wether it would be null
	while((row = mysql_fetch_row(res_set)) != NULL){
		//For showing information of each row, we have to seperate them
		//and the mysql_num_fields will give us the number of columns
		for(i=0; i<mysql_num_fields(res_set); i++){
			if(i>0)
				fputc('\t', stdout);
			printf("%s", row[i] != NULL ? row[i] : "NULL");
		}
		fputc('\n', stdout);
	}

	//after printing each line, there might be one of these two options:
	//First we have experienced an error which leads us to print error function
	//Second we did all things correctlly and we could print the number of lines that printed
	if(mysql_errno(conn) != 0)
		print_error(conn, "mysql_fetch_row() has failed");
	else
		printf("\nQuery handled: %lu rows returned\n", (unsigned long) mysql_num_rows(res_set));
}


//This function will send a SQL query to the database and check wether it
//could returna valid result set or might lead to an error
static void process_statement(MYSQL *conn, char *statement){
	MYSQL_RES *res_set;
	//This function will send our SQL statement to the database and await the result
	if(mysql_query(conn, statement)){
		print_error(conn, "Query Failed: Could not execute the statement");
		return;
	}
	printf("\"%s\"\n\n", statement);

	//If the SQL statement executed in the database, it might return a
	//result set and store them in user memory with meysql_store_result
	res_set = mysql_store_result(conn);
	//If this result set was not empty then we could process the results
	//and afterwards we could release the memory
	if(res_set){
		process_result_set(conn, res_set);
		mysql_free_result(res_set);
	}
	//Otherwise, the result set is null and there will be only two possibilities
	//Either our query handled correctly but there were no printable results or we have experienced an error
	else{
		//If the reult set was empty due to the nature of SQL statment and not an error, then we print out the number of rows which have been effected by that statement
		if(mysql_field_count(conn) == 0)
			printf("\nQuery handled: %lu rows affected\n", (unsigned long) mysql_affected_rows(conn));
		else
			print_error(conn, "\nQuery Failed: Could not retrive the result set");
	}
}


//This function does exactly what the previous did, except with different set of mysql APIs, which could be prefferable in some situations
static void process_real_statement(MYSQL *conn, char *statement, unsigned int length){
	MYSQL_RES *res_set;
	//This function will send our SQL statement to the database and await the result
	if(mysql_real_query(conn, statement, length)){
		print_error(conn, "Could not execute the statement");
		return;
	}

	//If the SQL statement executed in the database, it might return a
	//result set which could be obtained on demand by mysql_use_result
	res_set = mysql_use_result(conn);
	//If this result set was not empty then we could process the results
	//and afterwards we could release the memory
	if(res_set){
		process_result_set(conn, res_set);
		mysql_free_result(res_set);
	}
	//Otherwise, the result set is null and there will be only two possibilities
	//Either our query handled correctly but there were no printable results or we have experienced an error
	else{
		if(mysql_errno(conn) == 0)
			//If the reult set was empty due to the nature of SQL statment and not an error, then we print out the number of rows which have been effected by that statement
			printf("\nQuery handled: %lu rows affected\n", (unsigned long) mysql_affected_rows(conn));
		else
			print_error(conn, "\nQuery Failed: Could not retrive the result set");
	}
}


//This function creates a simple information about the program
static void show_credits(){
	printf("Simple Phonebook Version %s\n", SIMPLE_PHONEBOOK_VERSION);
	printf("Created By %s\n", SIMPLE_PHONEBOOK_AUTHUR);
	printf("USING %s COMPILER, ", SIMPLE_PHONEBOOK_COMPILER);
	printf("DATABASE %s\n", mysql_get_client_info());
	printf("Published Under %s License\n", SIMPLE_PHONEBOOK_LICENSE);
}


//This function creates the correct SQL query to search for a specific person in the phonebook
static void search_contact(){
	char name[40], statement[80];
	printf("Enter Contact Name: ");
	scanf("%s", name);
	printf("\n---------------------------------\n\n");

	sprintf(statement, "SELECT * FROM book WHERE name LIKE \'%s\'", name);
	process_statement(conn, statement);
}


//This function creates the correct SQL query to list whole contacts and all their information
static void list_contacts(){
	char statement[30];
	sprintf(statement, "SELECT * FROM book");
	process_statement(conn, statement);
}


//This function creates the correct SQL query to insert a new contact into our phonebook
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


//This function creates the correct SQL query to update information for a record of our phonebook
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


//This function creates the correct SQL query to remove a specific record from the phonebook
static void delete_contact(){
	char name[40], statement[80];
	printf("Enter Contact Name: ");
	scanf("%s", name);
	printf("\n---------------------------------\n\n");

	sprintf(statement, "DELETE FROM book WHERE name LIKE \"%s\"", name);
	process_statement(conn, statement);
}


//This function creates the correct SQL query to remove all contacts from our phonebook
static void drop_contacts(){
	char statement[30];
	sprintf(statement, "DELETE FROM book");
	process_real_statement(conn, statement, strlen(statement));
}


//This function creates the correct SQL query to obtain the number of contacts in the phonebook
static void contacts_information(){
	char statement[30];
	sprintf(statement, "SELECT COUNT(name) FROM book");
	process_real_statement(conn, statement, strlen(statement));
}


//Depend on the user choice in face of the menu, this function will choose the write action to do next, simply means that it select which other function should run, each of those function should create the correct SQL query
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


//For user convenient this fuction will create a simple menu and request an option from user
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


//This function analyze the command line argument which passed to the main function
static void process_options(int argc, char *argv[]){
	int opt_err;

	MY_INIT(argv[0]);
	load_defaults("my", client_groups, &argc, &argv);
	if((opt_err = handle_options(&argc, &argv, my_opts, get_one_option)))
		exit(opt_err);

	//Solicit password if necessary
	if(ask_password)
		opt_password = get_tty_password(NULL);

	//get database name if present on command line
	if(argc > 0){
		opt_db_name = argv[0];
		--argc;
		++argv;
	}
}


//This function will connect to mysql or mariadb RDBMS
//then create a database, connect to it and create a table in it
//
//use mysql_int for initializing a MYSQL connection data structure
//use mysql_real_connect to connect to a RDBMS with a specific username and password and other options
//use mysql_query to send our SQL-style queries such as creating a database and table
//use mysql_select_db after creating a database to conect to it and create a table in it
static void create_and_connect(){
	printf("INITIALIZING our Database ...");
	//First we need to initialize our connection data structure
	conn = mysql_init(NULL);
	if(!conn){
		printf(" Failed\n");
		print_error(NULL, "mysql_init() has failed, Probably out of memory");
		exit(1);
	}

	//Second, we will connect to the MySQL or MariaDB RDBMS
	if(!mysql_real_connect(conn, opt_host_name, opt_user_name, opt_password, opt_db_name, opt_port_number, opt_socket_name, opt_flags)){
		printf(" Failed\n");
		print_error(conn, "mysql_real_connect() has failed");
		mysql_close(conn);
		exit(1);
	}

	//Third, if everything has gone well, we want to create our database and table, if they were not created yet
	if(mysql_query(conn, "CREATE DATABASE IF NOT EXISTS phonebook")){
		printf(" Failed\n");
		print_error(conn, "Creation of phonebook database has failed");
		mysql_close(conn);
		exit(1);
	}

	//Fourth, we have to use that database
	if(mysql_select_db(conn, "phonebook")){
		printf(" Failed\n");
		print_error(conn, "Using phonebook database has failed");
		mysql_close(conn);
		exit(1);
	}

	//Fifth, at the last stage we want to create a table for our phone book if that was not exists
	if(mysql_query(conn, "CREATE TABLE IF NOT EXISTS book( name VARCHAR(40) NOT NULL, home DECIMAL(14, 0) default NULL, work DECIMAL(14, 0) default NULL, mobile DECIMAL(14, 0) default NULL, email VARCHAR(40) default NULL, PRIMARY KEY (name));")){
		printf(" Failed\n");
		print_error(conn, "Creation of phonebook table has failed");
		mysql_close(conn);
		exit(1);
	}

	printf(" OK\n");
}


//This is the mail part of our program
int main(int argc, char *argv[]){
	int choice;

	//First of all we need to process mysql options given by the user
	process_options(argc, argv);

	//Second, we need to create a database and a proper table for it
	create_and_connect();

	//Now the databse and table has created, so we have to show the user menu
	while((choice = show_user_menu()) >= 0 && choice <= 8)
		process_user_choice(choice);

	//At the end, we have to terminate the connection to the database and exit
	printf("You have entered wrong option, program terminated ...\n");
	mysql_close(conn);
	exit(0);
}
