/**
* \file
* \lib siemens.dll
* \since Release 3.1 SP4
* \env C,ITK
* \ingroup workflow_action_handlers
* \brief    This defines an action handler to import msd data.
*/

/*------------------------------------------------------------------------------
* History
*---------------------------------------------------------------------------------------------------------------------------
* Date          Name                 Description of Change
*02-Aug-2016    Raghav Rajkumar      P5A00142722A-BOMCheck Import
*14-Sep-2016    Renjith George       Task 26798#BOMcheck export 
* 24-Oct-2016      Kalaiselvan          P5A00191887-Fix all tag_t variable with NULL to NULLTAG 
* 19-Jan-2017      Deepachitra         Task : 28652/QualityImprovement : Remove Unused Variables in serverside
* ------------------------------------------------------------------------------
*
* ------------------------------------------------------------------------------ */

#ifdef _WIN32
#include <iostream>
#include <direct.h>
#include <io.h>
#include <tchar.h>
#else
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#endif

#include "siemens_workflow.h"
#include "siemens_trace_handling.h"
#include "siemens_disp_workflow_record_common.h"
#include <stdio.h>
#include <time.h>

/**
\ingroup workflow_action_handlers

\par Description :
\verbatim
This defines an action handler to import msd data..

\endverbatim

\par Placement :
Place on start action of do task.

\par Restrictions :


\par Usage in Workflows :
To do import msd data for substance compliance.

\param[in] msg :        Message of the handler

\par Returns :
int : ITK_ok / error code


*/

/*------------------------------------------------------------------------------
* History
*---------------------------------------------------------------------------------------------------------------------------
* Date          Name                 \par Description of Change
*02-Aug-2016    Raghav Rajkumar      P5A00142722A-BOMCheck Import
*14-Sep-2016    Renjith George       Task 26798#BOMcheck export 
* ------------------------------------------------------------------------------
*
* ------------------------------------------------------------------------------ */
//int simens_mds_create_request(tag_t tPrimaryObj, tag_t tEpmTask,int intPriority, char* pcProvider, char *pcService,char *pAttrDUNSValue,char *pOrderDescAttrValue,char* pAddOrderDescAttrValue,char *pStartDate, char *pEmail,char *pUrl );

extern "C" int
    siemens_import_MSD_data( EPM_action_message_t msg )
{
    /*int iStatus                     = ITK_ok;
    int iRefAttCount                = 0;
    int intPriority                 = 0;

    tag_t* tagRefAtts               = NULL;
    tag_t tagRootTask               = NULLTAG;

    char* pcProvider             = NULL;
    char* pcService             = NULL;
    char* pcPriority             = NULL;

    const char* __function__ = "siemens_import_MSD_data" ;
    SIEMENS_TRACE_ENTER();

    try
    {
    // Read the input arguments
    //read the handler arguments
    siemens_get_handler_opts( msg.arguments,
    "-provider", &pcProvider,
    "-service", &pcService,
    "-priority", &pcPriority,
    NULL );

    intPriority = atoi( pcPriority );
    //get the root task
    SIEMENS_TRACE_CALL( iStatus = EPM_ask_root_task( msg.task, &tagRootTask ) );
    SIEMENS_LOG_ERROR_AND_THROW_STATUS;

    SIEMENS_TRACE_CALL( iStatus = Siemens_ask_attachments_from_task( msg.task, EPM_target_attachment, &iRefAttCount, &tagRefAtts ) );
    SIEMENS_LOG_ERROR_AND_THROW_STATUS;

    //If attachments are not NULL then only proceed.
    if( iRefAttCount > 0 )
    {
    //Get object type
    for( int attCount = 0; attCount < iRefAttCount; attCount++ )
    {
    tag_t 	tRequest = NULLTAG;
    tag_t  tNewDispWFRecord  = NULLTAG;
    tag_t tPrimaryObj = tagRefAtts[attCount];
    char* acDoTaskUID = NULL;
    char* acPrimaryUID = NULL;
    char* acSecUID = NULL;

    SIEMENS_TRACE_CALL( iStatus = DISPATCHER_create_request(pcProvider,pcService,intPriority,NULL,NULL,0,1, &tPrimaryObj, &tPrimaryObj,0, NULL,"ONDEMAND",0,NULL,NULL,&tRequest));
    SIEMENS_LOG_ERROR_AND_THROW_STATUS;

    if(tRequest != NULLTAG)
    {
    SIEMENS_TRACE_CALL( iStatus = POM_tag_to_uid( msg.task, &acDoTaskUID ) );
    SIEMENS_TRACE_CALL( iStatus = POM_tag_to_uid( tPrimaryObj, &acPrimaryUID ) );
    SIEMENS_TRACE_CALL( iStatus = POM_tag_to_uid( tPrimaryObj, &acSecUID ) );

    SIEMENS_TRACE_CALL( iStatus = siemens_create_dispatcher_workflow_record( acDoTaskUID, acPrimaryUID,
    acSecUID, pcService,
    "STARTED", "ACTIVE", &tNewDispWFRecord ) );
    SIEMENS_LOG_ERROR_AND_THROW_STATUS;
    SIEMENS_TRACE_CALL( iStatus = POM_save_instances( 1, &tNewDispWFRecord , true ) );
    SIEMENS_LOG_ERROR_AND_THROW_STATUS;
    }
    }

    }
    }
    catch( ... )
    {
    }

    Custom_free( tagRefAtts );
    SIEMENS_TRACE_LEAVE_RVAL( "%d", iStatus );
    return  ITK_ok ;*/

    int iStatus                     = ITK_ok;
    int iRefAttCount                = 0;
    int iNamedRefCount              = 0;
    FILE* input                     = NULL;
    tag_t* tagNamedRefs             = NULL;
    tag_t* tagRefAtts               = NULL;
    tag_t tagRootTask               = NULLTAG;
    tag_t tagTaskOwner              = NULLTAG;
    tag_t tagPerson                 = NULLTAG;
    tag_t tManufItem                = NULLTAG;

    char* emailID                   = NULL;
    char* pcUrl                     = NULL;
    char* pcBomCheckSharedValue     = NULL;     //Org shared location value
    char* cUserID                   = NULL;
    //char  currentUserName[SA_person_name_size_c + 1];
    std::string szExportFileName("");
    FILE*       fpExportFile        = NULL;     //file pointer
    char cObjecttype[TCTYPE_name_size_c + 1]  = "";
    logical lIsFileFound = false;

    const char* __function__ = "siemens_import_MSD_data" ;
    SIEMENS_TRACE_ENTER();

    try
    {
        //get the root task
        SIEMENS_TRACE_CALL( iStatus = EPM_ask_root_task( msg.task, &tagRootTask ) );
        SIEMENS_LOG_ERROR_AND_THROW_STATUS;

        if( tagRootTask != NULLTAG )
        {
            //get the initiator of workflow
            SIEMENS_TRACE_CALL( iStatus = AOM_ask_owner( tagRootTask, &tagTaskOwner ) );
            SIEMENS_LOG_ERROR_AND_THROW_STATUS;

            if( tagTaskOwner != NULLTAG )
            {
                //get person
                SIEMENS_TRACE_CALL( iStatus = SA_ask_user_person( tagTaskOwner, &tagPerson ) );
                SIEMENS_LOG_ERROR_AND_THROW_STATUS;
                //Get email address
                SIEMENS_TRACE_CALL( iStatus = SA_ask_person_email_address( tagPerson, &emailID ) );
                SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                SIEMENS_TRACE_CALL( iStatus = SA_ask_user_identifier2( tagTaskOwner, &cUserID ) );
                SIEMENS_LOG_ERROR_AND_THROW_STATUS;
            }
        }

        SIEMENS_TRACE_CALL( iStatus = Siemens_ask_attachments_from_task( msg.task, EPM_target_attachment, &iRefAttCount, &tagRefAtts ) );
        SIEMENS_LOG_ERROR_AND_THROW_STATUS;

        //If attachments are not NULL then only proceed.
        if( iRefAttCount > 0 )
        {
            //Get awc URL
            SIEMENS_TRACE_CALL( iStatus = siemens_get_awc_url(msg.task, &pcUrl ));
            SIEMENS_LOG_ERROR_AND_THROW_STATUS;

            //Get object type
            for( int attCount = 0; attCount < iRefAttCount; attCount++ )
            {
                SIEMENS_TRACE_CALL( iStatus = Siemens_ask_object_type( tagRefAtts[attCount], cObjecttype ) );
                SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                if( tc_strcmp( cObjecttype, "S4_XML" ) == 0 )
                {
                    // If target is S4_XML dataset, get the named reference
                    // Export the named reference to BOMCHECK SHARED directory
                    // Call siemens_subscmpl_import.sh giving mfgdir

                    /*get named ref for input file*/
                    SIEMENS_TRACE_CALL( iStatus = AE_ask_all_dataset_named_refs2( tagRefAtts[attCount], "S4XML", &iNamedRefCount, &tagNamedRefs ) );
                    SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                    //Get the preference value for shared location
                    SIEMENS_TRACE_CALL( iStatus = siemens_get_shared_directory("SIEMENS_BOMCHECK_SHARED_LOC_PREF", &pcBomCheckSharedValue ) ); 
                    SIEMENS_LOG_ERROR_AND_THROW_STATUS;
                    std::string szTopFileDir = "";

                    //Get the system time in seconds & convert it into string
                    time_t tt;
                    struct tm* tm;
                    tt = time( NULL );
                    tm = localtime( &tt );
                    char pcRandom[200];
                    sprintf( pcRandom, "%4d%2d%2d_%2d%2d%2d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec );
                    // remove white spaces between
                    string szRandom = string( pcRandom );
                    string::iterator end_pos = std::remove( szRandom.begin(), szRandom.end(), ' ' );
                    szRandom.erase( end_pos, szRandom.end() );

                    /*Create Top Level Directory in Shared Location */
#ifdef _WIN32
                    szTopFileDir.clear();
                    szTopFileDir.append( pcBomCheckSharedValue ).append( "\\" ).append( szRandom ).append("_").append(cUserID);
                    SIEMENS_TRACE_CALL( iStatus = _mkdir( szTopFileDir.c_str() ) );

                    if( iStatus == ITK_ok )
                    {
                        TC_write_syslog( "\n Created temp directory : %s", szTopFileDir.c_str() );
                    }
                    else
                    {
                        TC_write_syslog( "\n Can not create directory" );
                    }
#else
                    szTopFileDir.clear();
                    szTopFileDir.append( pcBomCheckSharedValue ).append( "/" ).append( szRandom ).append("_").append(cUserID);
                    SIEMENS_TRACE_CALL( iStatus = mkdir( szTopFileDir.c_str(), 0777 ) );

                    if( iStatus == ITK_ok )
                    {
                        TC_write_syslog( "\n Created temp directory : %s", szTopFileDir.c_str() );
                    }
                    else
                    {
                        TC_write_syslog( "\n Can not create directory" );
                    }
#endif
                    if(iNamedRefCount > 0)
                    {
                        char* cOrgFileName = NULL;
                        std::string strProcessing(""); 
                        strProcessing.clear();
                        std::string strFilePath("");
                        strFilePath.clear();
                        SIEMENS_TRACE_CALL( iStatus = IMF_ask_original_file_name2( tagNamedRefs[0], &cOrgFileName ) );
                        SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                        if( cOrgFileName != NULL && tc_strlen( szTopFileDir.c_str() ) > 0 )
                        {
                            /* create the complete path with directory for input file as wqell as log file*/
#ifdef _WIN32
                            strFilePath.append( szTopFileDir ).append( "\\" ).append( cOrgFileName );
                            strProcessing.append( szTopFileDir ).append( "\\" ).append( "Processing" );
                            SIEMENS_TRACE_CALL( iStatus = _mkdir( strProcessing.c_str() ) );

                            if( iStatus == ITK_ok )
                            {
                                TC_write_syslog( "\n Created Processing directory : %s", szTopFileDir.c_str() );
                            }
                            else
                            {
                                TC_write_syslog( "\n Can not create directory" );
                            }
#else
                            strFilePath.append( szTopFileDir ).append( "/" ).append( cOrgFileName );
                            strProcessing.append( szTopFileDir ).append( "/" ).append( "Processing" );
                            SIEMENS_TRACE_CALL( iStatus = mkdir( strProcessing.c_str(), 0777 ) );

                            if( iStatus == ITK_ok )
                            {
                                TC_write_syslog( "\n Created Processing directory : %s", szTopFileDir.c_str() );
                            }
                            else
                            {
                                TC_write_syslog( "\n Can not create directory" );
                            }
#endif
                        }
                        

                        //unlink( strFilePath.c_str() );

                        SIEMENS_TRACE_CALL( iStatus = IMF_export_file( tagNamedRefs[0], strFilePath.c_str() ) );
                        SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                        /* user has given an input file, rather then using stdin */
                        if( !( input = fopen( strFilePath.c_str(), "rt" ) ) )
                        {
                            TC_write_syslog( "Cannot open input file %s\n", strFilePath.c_str() );
                            return 0;
                        }

                        lIsFileFound = false;

                        szExportFileName.append( szTopFileDir ).append( SIEMENS_PATH_SEPERATOR ).append( "ProcessedIPCFile.txt" );

                        //Create the file at the shared location specified in the Org preference....
                        fpExportFile = fopen( szExportFileName.c_str(), SIEMENS_WRITE_PLUS_MODE );

                        //If error while opening the file
                        if( fpExportFile == NULL )
                        {
                            iStatus = SIEMENS_NO_ACCESS_PERMISSION;
                            SIEMENS_LOG_ERROR_AND_THROW_STATUS;
                        }
#ifdef _WIN32
                        std::string sDirPathTemp( szTopFileDir.c_str() );
                        sDirPathTemp.append( SIEMENS_PATH_SEPERATOR ).append( "IPC_*" );
                        struct _finddata_t* fileData = NULL;
                        long hFile = 0;
                        fileData = ( struct _finddata_t* )MEM_alloc( sizeof( struct _finddata_t ) );

                        if( ( hFile = ( long )_findfirst( sDirPathTemp.c_str(), fileData ) ) != -1L )
                        {
                            lIsFileFound = true;
                        }

                        if(lIsFileFound) 
                        {
                            char* pcTC_Root = getenv( SIEMENS_TC_ROOT_ENV_VARIABLE_NAME );
                            char command[2048] = "";

                            sprintf( command, "%s\\siemens_cron_jobs\\siemens_subscmpl_import.bat \"%s\" \"%s\" \"%s\" \"%s\" ", pcTC_Root, szTopFileDir.c_str(),cUserID,pcUrl, cOrgFileName);
                            // printf( "\ncommand : %s\n", command );
                            system( command ); 
                        }

#else
                        DIR* d = NULL;
                        d = opendir( szTopFileDir.c_str() );
                        if( d != NULL )
                        {
                            struct dirent* file = NULL;
                            while( ( file = readdir( d ) ) != NULL )
                            {
                                if( tc_strstr( file->d_name, "IPC_" ) != NULL )
                                {
                                    lIsFileFound = true;
                                }

                                if( lIsFileFound )
                                {
                                    break;
                                }
                            }
                        }
                        closedir(d);

                        char* pcTC_Root = getenv( SIEMENS_TC_ROOT_ENV_VARIABLE_NAME );
                        if( pcTC_Root != NULL )
                        {
                            char command[2048]= "";
                            sprintf( command, "%s/siemens_cron_jobs/siemens_subscmpl_import.sh \"%s\" \"%s\" \"%s\" \"%s\" ", pcTC_Root,szTopFileDir.c_str(),cUserID,pcUrl, cOrgFileName);

                            char* command2 = NULL;
                            SIEMENS_TRACE_CALL( iStatus = STRNG_replace_str( const_cast<char*>( command ), "$", "\\$", &command2 ) );
                            SIEMENS_LOG_ERROR_AND_THROW_STATUS;

                            if( command2 != NULL )
                            {
                                system( command2 );
                                TC_write_syslog( "\ncommand2: %s \n", command2 );
                            }

                            Custom_free( command2 );
                        }
#endif
                        fprintf( fpExportFile, cOrgFileName );
                        //close the the file pointer
                        if( fpExportFile != NULL )
                        {
                            fclose( fpExportFile );
                        }

                        SIEMENS_TRACE_CALL( iStatus = siemens_create_mail_content("Import of MSD with IPC xml file is completed.",cUserID,szExportFileName));
                        SIEMENS_LOG_ERROR_AND_THROW_STATUS;
                        Custom_free( cOrgFileName );
                    }
                }
            }
        }
    }
    catch( ... )
    {
    }

    Custom_free( tagRefAtts );
    Custom_free( pcBomCheckSharedValue );
    Custom_free( tagNamedRefs);
    //Custom_free( emailID );
    //Custom_free( pcUrl );
    //Custom_free( cUserID );

    SIEMENS_TRACE_LEAVE_RVAL( "%d", iStatus );
    return  ITK_ok ;
}