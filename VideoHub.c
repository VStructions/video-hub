/*
 * ===========================================================================================================================
 *
 * Program: VideoHub
 * Author: VStructions
 * Date: January 15 2020
 *
 * ===========================================================================================================================
 */

/*  											Synopsis of VideoHub.db
 * ============================================================================================================================
 *  CREATE TABLE Videos(VidID integer primary key, 
 *	 				    VidName text not null, datetime not null default current_timestamp,
 *					    unique(VidID,VidName) );
 * 
 *  CREATE TABLE VideoCategAssos(VidID integer references Videos(VidID) on delete cascade deferrable initially deferred,
 *							     CatID integer references Categories(CatID) on delete restrict deferrable initially deferred,
 *							     unique(VidID, CatID) ); 
 *							   
 *	CREATE TABLE Categories(CatID integer primary key, CatName text not null, unique(CatID, CatName) );
 * ============================================================================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "sqlite3/sqlite3.h"	//or <>

#define ACTION1 1
#define ACTION2 2
#define ACTION3 4
#define ACTION4 8

#define SQLSLEN 201
int printingColumnNamesInSQLprintFunction=1;

typedef struct category
{
	int used;
	int num;		//not id
	char Name[100];
}CatStrct;

typedef struct categoryid	//........
{
	int used;
	int num;
	char id[10];		
	char CatName[100];
}CatidStrct;

static int SQLprint(void *emptyfornow, int columncount, char **data, char ** columns);
static int DBtableExtract(void *emptyfornow, int columncount, char **data, char **columns);
void credits();
int shouldistayorshouldigo(char*);
sqlite3* linkingtoDB();
int action(sqlite3* DBlink);
int getLine(char *);
void strCpy(char* _TARGET, char* _SOURCE);
int fendostr(char* source);
void zeroinnitstrCat(char* target, char* source);
int strCatTillComma(char* target, int* trgtPtr, char* source);
void ptrinnitstrCat(char *target, int *trgtptr, char* source);
void SQLstatementmode(sqlite3* DBlink);
void VideoHubmode(sqlite3* DBlink);
int searchDB(sqlite3* DBlink);
int insertDB(sqlite3* DBlink);
int modifyDB(sqlite3* DBlink);
int removeDB(sqlite3* DBlink);
void fcsv(char* formattedcat, char* unformatted);
void insertCategory(sqlite3* DBlink, char* sqlstatementname, char* name, char* userinput);
void getNumVideos(sqlite3* DBlink, CatStrct** videos, int* nrows);
void getNumCategories(sqlite3* DBlink, CatStrct** categories, int* nrows);
void getIDCategories(sqlite3* DBlink, CatidStrct** categories, int* nrows);
void translateNumCategories(char* formattedstr, char* catsInNum, CatStrct** categories, int* nrows);
void createSQLStatementlike(char* sqlstatement, char* condition, char* columnName, int usedBefore);
void createSQLStatement(char* sqlstatement, char* condition, char* columnName, int usedBefore);

int main()
{
    sqlite3 *DBlink;
    credits();

    DBlink = linkingtoDB();
    action(DBlink);

    sqlite3_close(DBlink);
    getchar();getchar();

    return 0;
}

void SQLstatementmode(sqlite3* DBlink)
{
    int i;
    char sqlstatement[SQLSLEN], *errormsg;

    printf("\nEnter SQL statement: ");

    Getline:
    if( getLine(sqlstatement) )
    {
        printf("\nYou actually have to type something (eg: select * from Videos): ");
        goto Getline;
    }
    //Disable column name printing with printingColumnNamesInSQLprintFunction = 0
    sqlite3_exec(DBlink, sqlstatement, SQLprint, NULL, &errormsg);
    printingColumnNamesInSQLprintFunction = 1;

    if( errormsg != SQLITE_OK )
    {
        fprintf(stderr, "\nSQL error: %s\n\n", errormsg);
        sqlite3_free(errormsg);
    }
}

void VideoHubmode(sqlite3* DBlink)
{
    int action;

    printf("\nOptions:\n  Search for videos {1}\n  Add to VideoHub {2}\n  Modify your VideoHub {3}\n  Remove from VideoHub {4}\nSelection: ");

    input:
    fflush(stdin);
    switch( action = getchar() )
    {
        case '1': getchar(); searchDB(DBlink); break;
        case '2': getchar(); insertDB(DBlink); break;
        case '3': getchar(); modifyDB(DBlink); break;
        case '4': getchar(); removeDB(DBlink); break;
        default: printf("Type 1-4: "); fflush(stdin); goto input;
    }
}

int inputfield = 0;
int searchDB(sqlite3* DBlink)
{
	char defaultstatement[] = "select VidName, C.CatName, datetime from Videos as V join VideoCategAssos as VCA on V.rowid=VCA.VidID join Categories as C on C.CatID=VCA.CatID ";
	CatStrct* categories = NULL;
	int nrows, SQLsStrPtr;
	int action, i;
    char sqlstatement[3*SQLSLEN] = {'\0'}, *errormsg, userinput[SQLSLEN], usedCreate = 0;
    char byname[SQLSLEN] = {'\0'}, bycat[SQLSLEN] = {'\0'}, bydate[SQLSLEN] = {'\0'}, catsInNum[SQLSLEN] = {'\0'};

    startofsearch:
    printf("\nSearch by (add parameters and select \"finish\" when done):\n  Name {1}\n  Category {2}\n  Show all {3}\n  Finish {4}\nSelection: ");
    inputerror:
    fflush(stdin);
    if( (action = getchar()) == '1' )
    {
        printf("\nType in the name or names, seperated by commas: ");
        Getline:
        fflush(stdin);
        if( getLine(userinput) )
        {
           printf("\nYou actually have to type something (eg: forest, sea): ");
           goto Getline;
        }
        fcsv( byname, userinput );
        inputfield |= ACTION1;
        goto startofsearch;
    }
    else if( action == '2' )
    {
		if ( categories == NULL )
			getNumCategories(DBlink, &categories, &nrows);
		else {
			for(i=0; catsInNum[i] != '\0'; i++)
				catsInNum[i] = '\0';
		}
		for( i=0; i < nrows; i++)
			printf("\n%s {%d}", categories[i].Name, categories[i].num);      
	
		printf("\n\nType in the numbers that correspond to the categories shown on screen, seperated by commas: ");
		Gitline:
		fflush(stdin);
		if( getLine(userinput) )
		{
			printf("\nYou actually have to type something (eg: 1, 2): ");
			goto Gitline;
		}
		fcsv( catsInNum, userinput );
		
		translateNumCategories(bycat, catsInNum, &categories, &nrows);
		
		inputfield |= ACTION2;
        goto startofsearch;
    }
    else if( action == '3' )
    {
        sqlite3_exec(DBlink, "select VidName, C.CatName, datetime from Videos as V join VideoCategAssos as VCA on V.rowid=VCA.VidID join Categories as C on C.CatID=VCA.CatID order by C.CatName;", SQLprint, NULL, NULL);
		printingColumnNamesInSQLprintFunction = 1;
		printf("\n");
        goto startofsearch;
    }
    else if ( action == '4' )
    {
        if ( inputfield & (ACTION1 | ACTION2 | ACTION3) )
        {
            zeroinnitstrCat( sqlstatement, defaultstatement);
			
			if ( inputfield & ACTION1 )
			{
				//printf("ACTION1!");
				createSQLStatementlike( sqlstatement, byname, "VidName", 0 );
				usedCreate = 1;
			}
			if ( inputfield & ACTION2 )
			{
				//printf("ACTION2!");
				createSQLStatement( sqlstatement, bycat, "C.CatName", usedCreate );
				usedCreate = 1;
			}
			SQLsStrPtr = fendostr(sqlstatement);
			if ( (inputfield & ACTION1) && (inputfield & ACTION2) )
			{
				ptrinnitstrCat( sqlstatement, &SQLsStrPtr, "order by VidName");
			}
			else if ( inputfield & ACTION1 )
				ptrinnitstrCat( sqlstatement, &SQLsStrPtr, "order by C.CatName");
			else
				ptrinnitstrCat( sqlstatement, &SQLsStrPtr, "order by VidName");
        }
		else
			;
        
        //printf("\n~~%s~~\n\n", sqlstatement);
        sqlite3_exec(DBlink, sqlstatement, SQLprint, NULL, &errormsg);
        printingColumnNamesInSQLprintFunction = 1;
		
		for(i=0; sqlstatement[i] != '\0'; i++)
			sqlstatement[i] = '\0';
		inputfield = 0;
		
        if( errormsg != SQLITE_OK )
        {
            fprintf(stderr, "\nHey there! Small problem  %s\n\n", errormsg);
            sqlite3_free(errormsg);
        }
		
		if ( categories != NULL )
			free ( categories );
    }
    else
    {
        printf("Type 1-4: "); fflush(stdin); goto inputerror;
    }

    return 0;
}

int insertDB(sqlite3* DBlink)
{
    CatidStrct* categories = NULL;
	int i, j, is, action1, action2, SQLsStrPtr, staticPtr, nrows=0, insertRowID, lastComma;
    char sqlstatementname[SQLSLEN] = {'\0'}, sqlstatementcat[SQLSLEN] = {'\0'}, insertRowIDs[10] ={'\0'}, *errormsg, userinput[SQLSLEN], name[100], catsInNum[SQLSLEN] = {'\0'};

    startofinsert:
    printf("\nAdd a (add parameters and select \"finish\" when done):\n  Video {1}\n  Category {2}\nSelection: ");
    inputerror:
    fflush(stdin);
    if( (action1 = getchar()) == '1' )
    {
		for(i=0; name[i] != '\0'; i++)
			name[i] = '\0';
		SQLsStrPtr=0;
		
		printf("\nType in the name: ");
        Getname:
        fflush(stdin);
        if( getLine(userinput) )
        {
           printf("\nYou actually have to type something (eg: forest): ");
           goto Getname;
        }
        fcsv( name, userinput );
		
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "insert into Videos (VidName) values('");
        strCatTillComma( sqlstatementname, &SQLsStrPtr, name);
        ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "');");
		
		sqlite3_exec(DBlink, "begin;", NULL, NULL, NULL);
		
		Catselect:
		printf("\nSelect from \n  existing categories {1}\n  add in a new one {2}\nSelection: ");
		Catinputerror:
		fflush(stdin);
		if( (action2 = getchar()) == '1' )
		{
			if ( categories == NULL )
				getIDCategories(DBlink, &categories, &nrows);
			
			for( j=0; j < nrows; j++)
				printf("\n%s {%d}", categories[j].CatName, categories[j].num);
		
			printf("\n\nType in the numbers that correspond to the categories shown on screen, seperated by commas: ");
			Gitline:
			fflush(stdin);
			if( getLine(userinput) )
			{
				printf("\nYou actually have to type something (eg: 1, 2): ");
				goto Gitline;
			}
			fcsv( catsInNum, userinput );    
		}
		else if ( action2 == '2' )
		{	
			for(i=0; name[i] != '\0'; i++)
				name[i] = '\0';
			for(i=0; sqlstatementcat[i] != '\0'; i++)
				sqlstatementcat[i] = '\0';
			
			insertCategory(DBlink, sqlstatementcat, name, userinput);
			
			insertRowID = sqlite3_last_insert_rowid(DBlink);
			sprintf(userinput, "%d\0", insertRowID);
			fcsv( catsInNum, userinput ); 
			
			free(categories);
			getIDCategories(DBlink, &categories, &nrows);
			
			for(i=0; name[i] != '\0'; i++)
				name[i] = '\0';
			for(i=0; sqlstatementcat[i] != '\0'; i++)
				sqlstatementcat[i] = '\0';
		}
		else
		{
			printf("\nYou actually have to type something (eg: 1): ");
			goto Catinputerror;
		}
		if (shouldistayorshouldigo("\nDo you want to select more categories? (Y/N): "))
			goto Catselect;

		
		if (!shouldistayorshouldigo("\nAre you sure that you want to make this addition? (Y/N): "))
		{
			fail:
			for(i=0; catsInNum[i] != '\0'; i++)
				catsInNum[i] = '\0';
			sqlite3_exec(DBlink, "rollback;", NULL, NULL, NULL);
			goto Addmore;
		}
		sqlite3_exec(DBlink, sqlstatementname, NULL, NULL, &errormsg);
		if( errormsg != SQLITE_OK )
        {
            fprintf(stderr, "\nHey there! Small problem %s\n\n", errormsg);
            sqlite3_free(errormsg);
			sqlite3_exec(DBlink, "rollback;", NULL, NULL, NULL);
			goto fail;
        }
		insertRowID = sqlite3_last_insert_rowid(DBlink);
		sqlite3_exec(DBlink, "commit;", NULL, NULL, NULL);
		
		SQLsStrPtr = 0;
		ptrinnitstrCat( sqlstatementcat, &SQLsStrPtr, "insert into VideoCategAssos(VidID,CatID) values("); 
		sprintf(insertRowIDs, "%d", insertRowID);
		ptrinnitstrCat( sqlstatementcat, &SQLsStrPtr, insertRowIDs );
		ptrinnitstrCat( sqlstatementcat, &SQLsStrPtr, ",");
		staticPtr = SQLsStrPtr;
		
		sqlite3_exec(DBlink, "begin;", NULL, NULL, NULL);
		
		for( i=lastComma=0; ; )
		{
			if( catsInNum[i] != '\0' )
			{
				while( catsInNum[i] != ',' )
					i++;
				catsInNum[i] = '\0';
				
				is=atoi(&catsInNum[lastComma]);
				if ( is < 1 && is >= nrows)
					goto skip;
				if ( categories[--is].used == 1 )
				{
					skip:
					catsInNum[i] = ',';
					lastComma = ++i;
				}
				else
				{
					SQLsStrPtr = staticPtr;
					ptrinnitstrCat( sqlstatementcat, &SQLsStrPtr, categories[is].id );
					ptrinnitstrCat( sqlstatementcat, &SQLsStrPtr, ");");
					
					sqlite3_exec(DBlink, sqlstatementcat, NULL, NULL, &errormsg);

					if( errormsg != SQLITE_OK )
					{
						fprintf(stderr, "\nHey there! Small problem %s\n\n", errormsg);
						sqlite3_free(errormsg);
						sqlite3_exec(DBlink, "rollback;", NULL, NULL, NULL);
						goto fail;
					}
					
					categories[is].used = 1;
					catsInNum[i] = ',';
					lastComma = ++i;
				}
			}
			else
				break;
		} 
		sqlite3_exec(DBlink, "commit;", NULL, NULL, NULL);
		
		for(i=0; name[i] != '\0'; i++)
			name[i] = '\0';
		for(i=0; sqlstatementname[i] != '\0'; i++)
			sqlstatementname[i] = '\0';
		for(i=0; sqlstatementcat[i] != '\0'; i++)
			sqlstatementcat[i] = '\0';
    }
    else if( action1 == '2' )
    {
		insertCategory(DBlink, sqlstatementname, name, userinput);
		for(i=0; name[i] != '\0'; i++)
			name[i] = '\0';
		for(i=0; sqlstatementname[i] != '\0'; i++)
			sqlstatementname[i] = '\0';
		free(categories);
    }
    else
    {
        printf("Type 1-2: "); fflush(stdin); goto inputerror;
    }
	Addmore:
	if (shouldistayorshouldigo("\nAdd more? (Y/N): "))
        goto startofinsert;
	
	free( categories );
	
	return 0;
}

int modifyDB(sqlite3* DBlink)
{
    CatStrct* videos = NULL;
	CatStrct* categories = NULL;
	int i, action, SQLsStrPtr, nrows=0, viNum;
    char sqlstatementname[SQLSLEN] = {'\0'}, *errormsg, userinput[SQLSLEN], name[100];

    startofmodify:
    printf("\nRename a (add parameters and select \"finish\" when done):\n  Video {1}\n  Category {2}\nSelection: ");
    inputerror:
    fflush(stdin);
    if( (action = getchar()) == '1' )
	{	
		for(i=0; name[i] != '\0'; i++)
			name[i] = '\0';
		SQLsStrPtr=0;
		
		if ( videos == NULL )
			getNumVideos(DBlink, &videos, &nrows);
		
		for( i=0; i < nrows; i++)
			printf("\n%s {%d}", videos[i].Name, videos[i].num);
	
		printf("\n\nType in the number that corresponds to a video shown on screen: ");
		Gitline:
		fflush(stdin);
		if( getLine(userinput) )
		{
			printf("\nYou actually have to type something (eg: 1): ");
			goto Gitline;
		}
		if ( (viNum = atoi(userinput)) < 0 && viNum >= nrows )
		{
			printf("\nThe number you typed does not correlate to a video in VideoHub. Type in a number: \n");
			goto Gitline;
		}
		printf("\nType in the new name: ");
        Getname:
        fflush(stdin);
        if( getLine(userinput) )
        {
           printf("\nYou actually have to type something (eg: forest): ");
           goto Getname;
        }
        fcsv( name, userinput );
		
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "update Videos set VidName='");
        strCatTillComma( sqlstatementname, &SQLsStrPtr, name);
        ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "' where VidName='");
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, videos[--viNum].Name);
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "';");
			
		if (!shouldistayorshouldigo("\nAre you sure you want to rename this video?: "))
			goto modmore;
			
		sqlite3_exec(DBlink, sqlstatementname, NULL, NULL, &errormsg);
		
		if( errormsg != SQLITE_OK )
		{
			fprintf(stderr, "\nHey there! Small problem %s\n\n", errormsg);
			sqlite3_free(errormsg);
			goto modmore;
		}		
		getNumVideos(DBlink, &videos, &nrows);
	}
	else if ( action == '2' )
	{
		for(i=0; name[i] != '\0'; i++)
			name[i] = '\0';
		SQLsStrPtr=0;
		
		if ( categories == NULL )
			getNumCategories(DBlink, &categories, &nrows);
		
		for( i=0; i < nrows; i++)
			printf("\n%s {%d}", categories[i].Name, categories[i].num);
	
		printf("\n\nType in the number that corresponds to a category shown on screen: ");
		Getline:
		fflush(stdin);
		if( getLine(userinput) )
		{
			printf("\nYou actually have to type something (eg: 1): ");
			goto Getline;
		}		
		if ( (viNum = atoi(userinput)) < 0 && viNum >= nrows )
		{
			printf("\nThe number you typed does not correlate to a category in VideoHub. Type in a number: \n");
			goto Getline;
		}
		
		printf("\nType in the new name: ");
        Gotname:
        fflush(stdin);
        if( getLine(userinput) )
        {
           printf("\nYou actually have to type something (eg: forest): ");
           goto Gotname;
        }
        fcsv( name, userinput );
		
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "update Categories set CatName='");
        strCatTillComma( sqlstatementname, &SQLsStrPtr, name);
        ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "' where CatName='");
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, categories[--viNum].Name);
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "';");
			
		if (!shouldistayorshouldigo("\nAre you sure you want to rename this category?: "))
			goto modmore;
			
		sqlite3_exec(DBlink, sqlstatementname, NULL, NULL, &errormsg);
		
		if( errormsg != SQLITE_OK )
		{
			fprintf(stderr, "\nHey there! Small problem %s\n\n", errormsg);
			sqlite3_free(errormsg);
			goto modmore;
		}		
		getNumCategories(DBlink, &categories, &nrows);
	}
	else
	{
        printf("Type 1-2: "); fflush(stdin); goto inputerror;
    }
	
	modmore:
	if (shouldistayorshouldigo("\nmodify more?: "))
        goto startofmodify;
	
	free( videos );
	free( categories );
	
	return 0;
}

int removeDB(sqlite3* DBlink)
{
    CatStrct* videos = NULL;
	CatStrct* categories = NULL;
	int i, action, SQLsStrPtr, nrows=0, viNum;
    char sqlstatementname[SQLSLEN], *errormsg, userinput[SQLSLEN];

    startofremove:
    printf("\nRemove a (add parameters and select \"finish\" when done):\n  Video {1}\n  Category {2}\nSelection: ");
    inputerror:
    fflush(stdin);
    if( (action = getchar()) == '1' ) //19 fk constr error
	{
		for( i=0; sqlstatementname[i] != '\0' ; i++)
			sqlstatementname[i] = '\0';
		SQLsStrPtr=0;
		
		if ( videos == NULL )
			getNumVideos(DBlink, &videos, &nrows);
		
		for( i=0; i < nrows; i++)
			printf("\n%s {%d}", videos[i].Name, videos[i].num);
	
		printf("\n\nType in the number that corresponds to a video shown on screen: ");
		Gitline:
		fflush(stdin);
		if( getLine(userinput) )
		{
			printf("\nYou actually have to type something (eg: 1): ");
			goto Gitline;
		}
		if ( (viNum = atoi(userinput)) < 0 && viNum >= nrows )
		{
			printf("\nThe number you typed does not correlate to a video in VideoHub. Type in a number: \n");
			goto Gitline;
		}
		
		if (!shouldistayorshouldigo("\nAre you sure you want to remove this video?: "))
			goto removemore;
		
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "delete from Videos where VidName='");
        strCatTillComma( sqlstatementname, &SQLsStrPtr, videos[--viNum].Name);
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "';");
			
		sqlite3_exec(DBlink, sqlstatementname, NULL, NULL, &errormsg);
		
		if( errormsg != SQLITE_OK )
		{
			fprintf(stderr, "\nHey there! Small problem %s\n\n", errormsg);
			sqlite3_free(errormsg);
			goto removemore;
		}		
		getNumVideos(DBlink, &videos, &nrows);
	}
	else if ( action == '2' )
	{
		for( i=0; sqlstatementname[i] != '\0' ; i++)
			sqlstatementname[i] = '\0';
		SQLsStrPtr=0;
		
		if ( categories == NULL )
			getNumCategories(DBlink, &categories, &nrows);
		
		for( i=0; i < nrows; i++)
			printf("\n%s {%d}", categories[i].Name, categories[i].num);
	
		printf("\n\nType in the number that corresponds to a category shown on screen: ");
		Getline:
		fflush(stdin);
		if( getLine(userinput) )
		{
			printf("\nYou actually have to type something (eg: 1): ");
			goto Getline;
		}		
		if ( (viNum = atoi(userinput)) < 0 && viNum >= nrows )
		{
			printf("\nThe number you typed does not correlate to a category in VideoHub. Type in a number: \n");
			goto Getline;
		}
		if (!shouldistayorshouldigo("\nAre you sure you want to remove this category?: "))
			goto removemore;
		
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "delete from Categories where CatName='");
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, categories[--viNum].Name);
		ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "';");
			
		sqlite3_exec(DBlink, sqlstatementname, NULL, NULL, &errormsg);
		
		if( errormsg != SQLITE_OK )
		{
			fprintf(stderr, "\nHey there! Small problem %s\n\n", errormsg);
			sqlite3_free(errormsg);
			goto removemore;
		}		
		getNumCategories(DBlink, &categories, &nrows);
	}
	else
	{
        printf("Type 1-2: "); fflush(stdin); goto inputerror;
    }
	
	removemore:
	if (shouldistayorshouldigo("\nRemove more?: "))
        goto startofremove;
	
	free( videos );
	free( categories );
	
	return 0;
}

void insertCategory(sqlite3* DBlink, char* sqlstatementname, char* name, char* userinput)
{
	int SQLsStrPtr = 0;
	char* errormsg;
	
	printf("\nType in the name: ");
    Getname:
    fflush(stdin);
    if( getLine(userinput) )
	{
        printf("\nYou actually have to type something (eg: forest category): ");
        goto Getname;
    }
    fcsv( name, userinput );
	
	ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "insert into Categories(CatName) values('");
    strCatTillComma( sqlstatementname, &SQLsStrPtr, name);
    ptrinnitstrCat( sqlstatementname, &SQLsStrPtr, "');");
	
	sqlite3_exec(DBlink, sqlstatementname, NULL, NULL, &errormsg);

    if( errormsg != SQLITE_OK )
    {
        fprintf(stderr, "\nHey there! Small problem %s\n\n", errormsg);
        sqlite3_free(errormsg);
    }
}

void getNumCategories(sqlite3* DBlink, CatStrct** categories, int* nrows)
{
	CatStrct* categ = NULL;
	char** returnName;
	int i;
	
	sqlite3_get_table( DBlink, "select CatName from Categories;", &returnName, nrows, NULL, NULL );
		
	categ = (CatStrct*) malloc ( *nrows * sizeof(CatStrct));
	for( i=0; i < *nrows; i++ )
	{
		categ[i].used = 0;
		categ[i].num = i+1;
		strCpy( categ[i].Name, returnName[i+1]);
	}
	categ[*nrows].num = -1;
	*categories = categ;
}

void getNumVideos(sqlite3* DBlink, CatStrct** videos, int* nrows)
{
	CatStrct* categ = NULL;
	char** returnName;
	int i;
	
	sqlite3_get_table( DBlink, "select VidName from Videos;", &returnName, nrows, NULL, NULL );
		
	categ = (CatStrct*) malloc ( *nrows * sizeof(CatStrct));
	for( i=0; i < *nrows; i++ )
	{
		categ[i].used = 0;
		categ[i].num = i+1;
		strCpy( categ[i].Name, returnName[i+1]);
	}
	categ[*nrows].num = -1;
	*videos = categ;
}

void getIDCategories(sqlite3* DBlink, CatidStrct** categories, int* nrows)
{
	CatidStrct* categ = NULL;
	char** returnColumn;
	int i, j=0, g=1, loop;
	
	sqlite3_get_table( DBlink, "select CatID, CatName from Categories;", &returnColumn, nrows, NULL, NULL );
	
	categ = (CatidStrct*) malloc ( *nrows * sizeof(CatidStrct));
	
	loop = 2*(*nrows);
	for( i=2; i <= loop; i+=2 )
	{
		categ[j].used = 0;
		categ[j].num = g++;
		strCpy( categ[j].id, returnColumn[i]);
		strCpy( categ[j++].CatName, returnColumn[i+1]);
	}
	categ[*nrows].num = -1;
	*categories = categ;
}

void translateNumCategories(char* formattedstr, char* catsInNum, CatStrct** categories, int* nrows)
{
	int i, is, ftdstrPtr = 0, lastComma=0;
	
	ftdstrPtr = fendostr(formattedstr);
	for( i=0; ; )
	{
		if( catsInNum[i] != '\0' )
		{
			while( catsInNum[i] != ',' )
				i++;
			catsInNum[i] = '\0';
			
			is=atoi(&catsInNum[lastComma]);
			if ( is < 1 && is >= *nrows)
				goto skip;
			if ( (*categories)[--is].used == 1 )
			{
				skip:
				catsInNum[i] = ',';
				lastComma = ++i;
			}
			else
			{
				ptrinnitstrCat( formattedstr, &ftdstrPtr, (*categories)[is].Name );
				ptrinnitstrCat( formattedstr, &ftdstrPtr, ",");
				
				(*categories)[is].used = 1;
				catsInNum[i] = ',';
				lastComma = ++i;
			}
		}
		else
			return;
	}
}

void strCpy(char* _TARGET, char* _SOURCE)               
{                                                           
    while(*_TARGET++ = *_SOURCE++)
        ;
	*_TARGET = '\0';
}

void fcsv(char* formattedcat, char* unformatted)  //Formats the comma seperated user input (unform); and concatenates it to formatted
{
    int i, j, ftdStrPtr, spacecounter, charBeforeComma = 0;      //2nd var is formatted string pointer

    ftdStrPtr = fendostr(formattedcat);

    for(i=0; ; i++)
    {
        if(unformatted[i] != ' ' && unformatted[i] != '\t' && unformatted[i] != ',' && unformatted[i] != '\0')
        {
            //printf("~~%d~~char\n",i);//debugging
            formattedcat[ftdStrPtr++] = unformatted[i] ;
            formattedcat[ftdStrPtr] = '\0';
            charBeforeComma = 1;
        }
        else if( unformatted[i] == ' ' || unformatted[i] == '\t' )
        {
            for(spacecounter=i; ; spacecounter++)
            {
                //printf("~~%d~~space\n",spacecounter);//debugging
                if(unformatted[spacecounter] != ' ' && unformatted[spacecounter] != '\t' && unformatted[spacecounter] != ',' && unformatted[spacecounter] != '\0')
                {
                    if ( charBeforeComma == 1 )
                    {
                        for(; i < spacecounter; i++)
                            formattedcat[ftdStrPtr++] = unformatted[i] ;
                        formattedcat[ftdStrPtr] = '\0';
                        --i;

                        charBeforeComma = 1;
                    }
                    else
                        i = spacecounter-1;

                    spacecounter = 0;
                    break;

                }
                else if ( unformatted[spacecounter] == ',' || unformatted[spacecounter] == '\0' )
                {
                    i = spacecounter-1;
                    //printf("%d\n",i);//debugging
                    spacecounter = 0;
                    break;
                }
            }
        }
        else if( unformatted[i] == ',' || unformatted[i] == '\0' )
        {
            //printf("~~%d~~%d~~comma/end\n",i,charBeforeComma);//debugging
            if ( charBeforeComma == 1)
            {
                formattedcat[ftdStrPtr++] = ',' ;
                formattedcat[ftdStrPtr] = '\0';
                charBeforeComma = 0;
            }
            if ( unformatted[i] == '\0' )
                break;
        }
    }
}

void createSQLStatementlike(char* sqlstatement, char* condition, char* columnName, int usedBefore)
{
    int i, SQLsPtr, condPtr = 0;
    int* SQLsStrPtr = &SQLsPtr;

    SQLsPtr = fendostr(sqlstatement);

    if ( usedBefore )
        goto notfirstCreate;
	
	
    ptrinnitstrCat( sqlstatement, SQLsStrPtr, "where ");  
    goto firstloop;
    while ( condition[condPtr] != '\0' )
    {
        notfirstCreate:
        ptrinnitstrCat( sqlstatement, SQLsStrPtr, "and ");
        firstloop:
        ptrinnitstrCat( sqlstatement, SQLsStrPtr, columnName);
        ptrinnitstrCat( sqlstatement, SQLsStrPtr, " like '\%");
        condPtr += strCatTillComma( sqlstatement, SQLsStrPtr, &condition[condPtr] );
        ptrinnitstrCat( sqlstatement, SQLsStrPtr, "\%' ");
    }
}

void createSQLStatement(char* sqlstatement, char* condition, char* columnName, int usedBefore)
{
    int i, SQLsPtr, condPtr = 0;
    int* SQLsStrPtr = &SQLsPtr;

    SQLsPtr = fendostr(sqlstatement);

    if ( usedBefore )
	{
        if ( inputfield & ACTION1 )
		{
			ptrinnitstrCat( sqlstatement, SQLsStrPtr, "and ");
			goto firstloop;
		}
		goto notfirstCreate;
	}
	
    ptrinnitstrCat( sqlstatement, SQLsStrPtr, "where ");  
    goto firstloop;
    while ( condition[condPtr] != '\0' )
    {
        notfirstCreate:
        ptrinnitstrCat( sqlstatement, SQLsStrPtr, "or ");
        firstloop:
        ptrinnitstrCat( sqlstatement, SQLsStrPtr, columnName);
        ptrinnitstrCat( sqlstatement, SQLsStrPtr, "='");
        condPtr += strCatTillComma( sqlstatement, SQLsStrPtr, &condition[condPtr] );
        ptrinnitstrCat( sqlstatement, SQLsStrPtr, "' ");
    }
}

int action(sqlite3* DBlink)
{
    char action;

    start:
    printf("\nWould you like to:\n  VideoHub mode {1}\n  SQL statement mode {2} (Requires knowledge of this database)\n Action: ");

    oOf:
    fflush(stdin);
    switch( action = getchar() )
    {
        case '1': getchar(); VideoHubmode(DBlink); break;
        case '2': getchar(); SQLstatementmode(DBlink); break;
        default : printf("Type 1/2: "); fflush(stdin); goto oOf;
    }

    if (shouldistayorshouldigo("\nWould you like to proceed into exploring the depths of VideoHub sum moe bOi? (Y/N): "))
        goto start;
	printf("\nThank you for using VideoHub!");

    return 0;
}

int getLine(char *line)
{
    int i;

    for(i=0; i<SQLSLEN-1; i++)
       line[i] = ' ';
    line[SQLSLEN-1] = '\0';

    while( (line[0] = getchar()) == ' ' || line[0] == '\t' ){}

    for(i=1; line[i-1] != '\n' && i<SQLSLEN; i++)
       line[i] = getchar();
    line[i-1] = '\0';

    if (i < 2)
        return 1;
    else
        return 0;
}

int fendostr(char* source)
{
    int counter=0;
    while(*source++)
        counter++;

    return counter;
}

void zeroinnitstrCat(char* target, char* source) //concatenates source on target's end
{
    while( *target++ = *source++ )
        ;
}

int strCatTillComma(char* target, int* trgtPtr, char* source) //concatenates source on target's end
{
    int counter = 0;

    while( *source )
        if( *source == ',' )
        {
            counter++;
            break;
        }
        else
        {
            target[(*trgtPtr)++] = *source++;
            counter++;
        }
    return counter;
}

void ptrinnitstrCat(char *target, int *trgtptr, char* source)
{
    while( target[(*trgtptr)++] = *source++ )
        ;
    (*trgtptr)--;
}

int tableCounter = 0;
char tableNames[3][16] = {{'\0'}};
sqlite3* linkingtoDB()
{
    sqlite3 *DBlink;
	int i, errorlinkingtoDB = sqlite3_open("VideoHub.db", &DBlink);
	char *errormsg, mustHaveTables[3][16] = {{"Videos\0"}, \
					{"VideoCategAssos\0"},{"Categories\0"}};
	
    if (errorlinkingtoDB)
    {
        fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(DBlink));
        sqlite3_close(DBlink);
        getchar();
        exit(1);
    }
	
	sqlite3_exec(DBlink, "SELECT name FROM sqlite_master WHERE type = 'table'", DBtableExtract, &tableNames, &errormsg);

	if( errormsg != SQLITE_OK )
    {
        fprintf(stderr, "\nSQL error: %s\n\n", errormsg);
        sqlite3_free(errormsg);
		getchar();
        exit(1);
    }

	for(i=0; i < 3; i++)
		if( strcmp(mustHaveTables[i], tableNames[i]) ) //Are not equal
		{//Create all three database tables
			sqlite3_exec(DBlink, "CREATE TABLE Videos(VidID integer primary key,\
 	 				    VidName text not null, datetime not null default current_timestamp,\
 					    unique(VidID,VidName) );\
						\
 						CREATE TABLE VideoCategAssos(VidID integer references\
						Videos(VidID) on delete cascade deferrable initially deferred,\
 						CatID integer references Categories(CatID) on delete restrict\
						deferrable initially deferred, unique(VidID, CatID) );\
						\
 						CREATE TABLE Categories(CatID integer primary key, CatName\
						text not null, unique(CatID, CatName) );\
						", NULL, NULL, &errormsg);
			
			if( errormsg != SQLITE_OK )
			{
				fprintf(stderr, "\nSQL error: %s\n\n", errormsg);
				sqlite3_free(errormsg);
				getchar();
				exit(1);
			}
 			break;
		}
		else	//All good
			continue;

    return DBlink;
}

static int DBtableExtract(void *tableNamesVoid, int columncount, char **data, char **columns)
{
	if(tableCounter < 3)	
		if(data[0] != NULL)
		{
			strncat(tableNames[tableCounter], data[0], 15);
			tableCounter++;
		}
		else
			tableCounter = 3;	//sqlite3_exec() runs this, row, number of times

	return 0;
}

static int SQLprint(void *emptyfornow, int columncount, char **data, char **columns)
{
    int i, j;

    if( printingColumnNamesInSQLprintFunction )
    {
        printf("\n  ");
        for(i=0; i < columncount; i++)
            if( !strcmp(columns[i], "datetime" ) )
                printf("%-19s ", columns[i]);
            else
                printf("%-35s ", columns[i]);

        printf("  \n");
        printingColumnNamesInSQLprintFunction = 0;

        printf("  ");
        for(i=0; i < columncount; i++)
        {
            if( !strcmp(columns[i], "datetime" ) )
                for( j=0; j < 19; j++)
                    printf("-");
            else
                for( j=0; j < 35; j++)
                    printf("-");
            printf(" ");
        }
        printf("  \n");
    }

    printf("  ");
    for(i=0; i < columncount; i++)
        if( !strcmp(columns[i], "datetime" ) )
            printf("%-19s ", data[i]);
        else
            printf("%-35s ", data[i]);
    printf("  \n");


    return 0;
}

int shouldistayorshouldigo(char* question)
{
    char yn=' ';

    printf("%s", question);
    oOf:
    fflush(stdin);
    if( (( yn = getchar()) == 'y' ) || ( yn == 'Y' ) )
        return 1;
    else if ( ( yn != 'n' ) && ( yn != 'N' ) )
    {
        printf("Type Y/N: ");
        fflush(stdin);
        goto oOf;
    }
    else
        return 0;
}

void credits()
{
    printf("Author: VStructions\n");
    printf("Program: VideoHub\n\n\n");
    printf("                     Welcome to VideoHub :D\n");
    getchar();
}
