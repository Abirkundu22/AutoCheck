 
var ss = SpreadsheetApp.openById('YOUR_SHEET_ID_HERE'); // Your Google Sheet ID 
here 
var sheet = ss.getSheetByName('Sheet1'); 
var timezone = "Asia/Kolkata"; // Set timezone 
 
function doGet(e){ 
  Logger.log( JSON.stringify(e) ); 
 
  if (!e.parameter || !e.parameter.name) { 
    return ContentService.createTextOutput("No name provided."); 
  } 
 
  var Curr_Date = new Date(); 
  var Curr_Time = Utilities.formatDate(Curr_Date, timezone, 'HH:mm:ss'); 
  var Date_Str = Utilities.formatDate(Curr_Date, timezone, 'yyyy-MM-dd'); 
 
  var name = e.parameter.name; 
 
  var nextRow = sheet.getLastRow() + 1; 
 
  // Column order → Name | Date | Time 
  sheet.getRange("A" + nextRow).setValue(name); 
  sheet.getRange("B" + nextRow).setValue(Date_Str); 
  sheet.getRange("C" + nextRow).setValue(Curr_Time); 
 
  return ContentService.createTextOutput("Card holder name is stored."); 
} 
 
function doPost(e) { 
  // not used, can be left for future 
}