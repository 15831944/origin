// Execute_Requery_Clear_Method_Sample.cpp
// compile with: /EHsc
#include <ole2.h>
#include <stdio.h>

#import "c:\Program Files\Common Files\System\ADO\msado15.dll" \
    no_namespace rename("EOF", "EndOfFile")

// Function declarations
inline void TESTHR(HRESULT x) { if FAILED(x) _com_issue_error(x); };
void ExecuteX();
void ExecuteCommand(_CommandPtr pCmdTemp, _RecordsetPtr pRstTemp);
void PrintOutput(_RecordsetPtr pRstTemp);
void PrintProviderError(_ConnectionPtr pConnection);
void PrintComError(_com_error &e);

int main() {
    if ( FAILED(::CoInitialize(NULL)) )
        return -1;

    ExecuteX();
    ::CoUninitialize();
    }

void ExecuteX() {
    // Define string variables.
    //_bstr_t strSQLChange("UPDATE Titles SET Type = 'self_help' WHERE Type = 'psychology'");
    _bstr_t strSQLRestore("UPDATE Titles SET Type = 'psychology' WHERE Type = 'self_help'");
    //_bstr_t strCnn("Provider='sqloledb'; Data Source='My_Data_Source'; Initial Catalog='pubs'; Integrated Security='SSPI';");
    _bstr_t strCnn("Provider='SQLOLEDB.1';Data Source='127.0.0.1,1433';Initial Catalog='msdb'");
    _bstr_t strSQLChange("USE MASTER;IF EXISTS (SELECT * FROM SYS.DATABASES WHERE NAME ='ZY' and state = 0) Begin;EXEC('ALTER DATABASE [ZY] SET SINGLE_USER WITH ROLLBACK IMMEDIATE');EXEC('ALTER DATABASE [ZY] SET MULTI_USER');end;RESTORE DATABASE [ZY] FROM DISK = 'D:\syn_ZY_full.bak' WITH FILE = 1, STATS = 10, NOUNLOAD, BUFFERCOUNT = 6,maxtransfersize = 419304, REPLACE , move 'ZY' to N'G:\\DBdata\\ZY_data.mdf', move 'ZY_log' to N'G:\\DBdata\\ZY_log_0.ldf'");

    // Define ADO object pointers.  Initialize pointers on define.
    // These are in the ADODB::  namespace.
    _ConnectionPtr pConnection = NULL;
    _CommandPtr pCmdChange = NULL;
    _RecordsetPtr pRstTitles = NULL;

    try {
        // Open connection.
        TESTHR(pConnection.CreateInstance(__uuidof(Connection)));
        pConnection->Open (strCnn, "sa", "Root@123", adConnectUnspecified);

        // Create command object.
        TESTHR(pCmdChange.CreateInstance(__uuidof(Command)));
        pCmdChange->ActiveConnection = pConnection;
        pCmdChange->CommandText = strSQLChange;

        // Open titles table, casting Connection pointer to an 
        // IDispatch type so converted to correct type of variant.
        TESTHR(pRstTitles.CreateInstance(__uuidof(Recordset)));
        pRstTitles->Open(strSQLChange,
            _variant_t((IDispatch*)pConnection,true),
            adOpenDynamic,adLockOptimistic,adCmdText);

        // Print report of original data.
        printf("\n\nData in Titles table before executing the query: \n");

        // Call function to print loop recordset contents.
        PrintOutput(pRstTitles);

        // Clear extraneous errors from the Errors collection.
        pConnection->Errors->Clear();

        // Call ExecuteCommand subroutine to execute pCmdChange command.
        ExecuteCommand(pCmdChange, pRstTitles);

        // Print report of new data.
        printf("\n\n\tData in Titles table after executing the query: \n");
        PrintOutput(pRstTitles);

        // Use Connection object's Execute method to execute SQL statement to restore data.
        pConnection->Execute(strSQLRestore, NULL, adExecuteNoRecords);

        // Retrieve the current data by requerying the recordset.
        pRstTitles->Requery(adCmdUnknown);

        // Print report of restored data.
        printf("\n\n\tData after exec. query to restore original info: \n");
        PrintOutput(pRstTitles);
        }
    catch (_com_error &e) {
        PrintProviderError(pConnection);
        PrintComError(e);
        }

    // Clean up objects before exit.
    if (pRstTitles)
        if (pRstTitles->State == adStateOpen)
            pRstTitles->Close();
    if (pConnection)
        if (pConnection->State == adStateOpen)
            pConnection->Close();
    }

void ExecuteCommand(_CommandPtr pCmdTemp, _RecordsetPtr pRstTemp) {
    try {
        // CommandText property already set before function was called.
        pCmdTemp->Execute(NULL, NULL, adCmdText);

        // Retrieve the current data by requerying the recordset.
        pRstTemp->Requery(adCmdUnknown);
        }

    catch(_com_error &e) {
        // Notify user of any errors that result from executing the query.
        // Pass a connection pointer accessed from the Recordset.
        PrintProviderError(pRstTemp->GetActiveConnection());
        PrintComError(e);
        }
    }

void PrintOutput(_RecordsetPtr pRstTemp) {
    // Ensure at top of recordset.
    pRstTemp->MoveFirst();

    // If EOF is true, then no data and skip print loop.
    if ( pRstTemp->EndOfFile )
        printf("\tRecordset empty\n");
    else {
        // Define strings for output conversions.  Initialize to first record's values.
        _bstr_t bstrTitle;
        _bstr_t bstrType;

        // Enumerate Recordset and print from each.
        while ( !(pRstTemp->EndOfFile) ) {
            // Convert variant string to convertable string type.
            bstrTitle = pRstTemp->Fields->GetItem("Title")->Value;
            bstrType  = pRstTemp->Fields->GetItem("Type")->Value;
            printf("\t%s, %s \n", (LPCSTR) bstrTitle, (LPCSTR) bstrType);

            pRstTemp->MoveNext();
            }
        }
    }

void PrintProviderError(_ConnectionPtr pConnection) {
    // Print Provider Errors from Connection object.
    // pErr is a record object in the Connection's Error collection.
    ErrorPtr pErr = NULL;

    if ( (pConnection->Errors->Count) > 0 ) {
        long nCount = pConnection->Errors->Count;
        // Collection ranges from 0 to nCount -1.
        for ( long i = 0 ; i < nCount ; i++ ) {
            pErr = pConnection->Errors->GetItem(i);
            printf("\t Error number: %x\t%s", pErr->Number, pErr->Description);
            }
        }
}

void PrintComError(_com_error &e) {
    _bstr_t bstrSource(e.Source());
    _bstr_t bstrDescription(e.Description());

    // Print Com errors.
    printf("Error\n");
    printf("\tCode = %08lx\n", e.Error());
    printf("\tCode meaning = %s\n", e.ErrorMessage());
    printf("\tSource = %s\n", (LPCSTR) bstrSource);
    printf("\tDescription = %s\n", (LPCSTR) bstrDescription);
}